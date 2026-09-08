#include "orvix/capture/media_foundation_camera.hpp"

#include "orvix/capture/continuous_frame_capture.hpp"
#include "orvix/capture/diagnostic_error.hpp"
#include "orvix/capture/hresult_error.hpp"
#include "orvix/capture/media_foundation_runtime.hpp"
#include "orvix/capture/video_format_selector.hpp"

#include <Windows.h>
#include <mfapi.h>
#include <mferror.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <wrl/client.h>

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace orvix::capture {

namespace {

using Microsoft::WRL::ComPtr;

constexpr DWORD kAllStreams =
    static_cast<DWORD>(MF_SOURCE_READER_ALL_STREAMS);
constexpr DWORD kFirstVideoStream =
    static_cast<DWORD>(MF_SOURCE_READER_FIRST_VIDEO_STREAM);

std::wstring utf8_to_wide(const std::string& value) {
    if (value.empty()) {
        return {};
    }

    const int required = MultiByteToWideChar(
        CP_UTF8,
        MB_ERR_INVALID_CHARS,
        value.c_str(),
        -1,
        nullptr,
        0
    );

    if (required <= 0) {
        throw HResultError(
            HRESULT_FROM_WIN32(GetLastError()),
            "MultiByteToWideChar(size)"
        );
    }

    std::wstring result(static_cast<std::size_t>(required), L'\0');

    const int written = MultiByteToWideChar(
        CP_UTF8,
        MB_ERR_INVALID_CHARS,
        value.c_str(),
        -1,
        result.data(),
        required
    );

    if (written <= 0) {
        throw HResultError(
            HRESULT_FROM_WIN32(GetLastError()),
            "MultiByteToWideChar(convert)"
        );
    }

    if (!result.empty() && result.back() == L'\0') {
        result.pop_back();
    }

    return result;
}

bool guid_equals(const GUID& left, const GUID& right) noexcept {
    return IsEqualGUID(left, right) != FALSE;
}

std::string pixel_format_name(const GUID& subtype) {
    if (guid_equals(subtype, MFVideoFormat_NV12)) {
        return "NV12";
    }

    if (guid_equals(subtype, MFVideoFormat_YUY2)) {
        return "YUY2";
    }

    if (guid_equals(subtype, MFVideoFormat_UYVY)) {
        return "UYVY";
    }

    if (guid_equals(subtype, MFVideoFormat_I420)) {
        return "I420";
    }

    if (guid_equals(subtype, MFVideoFormat_IYUV)) {
        return "IYUV";
    }

    if (guid_equals(subtype, MFVideoFormat_YV12)) {
        return "YV12";
    }

    if (guid_equals(subtype, MFVideoFormat_RGB24)) {
        return "RGB24";
    }

    if (guid_equals(subtype, MFVideoFormat_RGB32)) {
        return "RGB32";
    }

    if (guid_equals(subtype, MFVideoFormat_ARGB32)) {
        return "ARGB32";
    }

    if (guid_equals(subtype, MFVideoFormat_MJPG)) {
        return "MJPG";
    }

    if (guid_equals(subtype, MFVideoFormat_H264)) {
        return "H264";
    }

    return "UNKNOWN";
}

bool is_compressed_format(const GUID& subtype) noexcept {
    return
        guid_equals(subtype, MFVideoFormat_MJPG) ||
        guid_equals(subtype, MFVideoFormat_H264);
}

std::uint32_t packed_stride(
    const std::string& pixel_format,
    const std::uint32_t width
) noexcept {
    if (
        pixel_format == "YUY2" ||
        pixel_format == "UYVY"
    ) {
        return width * 2U;
    }
    if (pixel_format == "RGB24") {
        return width * 3U;
    }
    if (
        pixel_format == "RGB32" ||
        pixel_format == "ARGB32" ||
        pixel_format == "BGRA32"
    ) {
        return width * 4U;
    }
    return width;
}

bool try_read_video_format(
    IMFMediaType* media_type,
    const std::size_t native_type_index,
    VideoFormat& format
) {
    GUID major_type{};
    if (
        FAILED(media_type->GetGUID(MF_MT_MAJOR_TYPE, &major_type)) ||
        !guid_equals(major_type, MFMediaType_Video)
    ) {
        return false;
    }

    UINT32 width = 0;
    UINT32 height = 0;
    if (FAILED(MFGetAttributeSize(
        media_type,
        MF_MT_FRAME_SIZE,
        &width,
        &height
    ))) {
        return false;
    }

    UINT32 frame_rate_numerator = 0;
    UINT32 frame_rate_denominator = 0;
    if (
        FAILED(MFGetAttributeRatio(
            media_type,
            MF_MT_FRAME_RATE,
            &frame_rate_numerator,
            &frame_rate_denominator
        )) ||
        frame_rate_denominator == 0
    ) {
        return false;
    }

    GUID subtype{};
    if (FAILED(media_type->GetGUID(MF_MT_SUBTYPE, &subtype))) {
        return false;
    }

    format.native_type_index = native_type_index;
    format.width = width;
    format.height = height;
    format.frame_rate_numerator = frame_rate_numerator;
    format.frame_rate_denominator = frame_rate_denominator;
    format.pixel_format = pixel_format_name(subtype);
    format.compressed = is_compressed_format(subtype);
    format.stride = packed_stride(format.pixel_format, width);

    return true;
}

class MediaFoundationFrameReader final : public FrameReader {
public:
    MediaFoundationFrameReader(
        IMFSourceReader* source_reader,
        const std::uint32_t stride
    )
        : source_reader_(source_reader),
          stride_(stride) {}

    FrameReadResult read_next() override {
        DWORD stream_flags = 0;
        LONGLONG timestamp = 0;
        ComPtr<IMFSample> sample;

        const HRESULT result = source_reader_->ReadSample(
            kFirstVideoStream,
            0,
            nullptr,
            &stream_flags,
            &timestamp,
            &sample
        );

        if (
            result == MF_E_VIDEO_RECORDING_DEVICE_INVALIDATED ||
            result == HRESULT_FROM_WIN32(ERROR_DEVICE_NOT_CONNECTED)
        ) {
            throw DiagnosticError(
                "ORV-CAP-503",
                "camera_disconnected",
                "The camera was disconnected or invalidated during capture.",
                74
            );
        }

        if (FAILED(result)) {
            throw HResultError(result, "IMFSourceReader::ReadSample");
        }

        if ((stream_flags & MF_SOURCE_READERF_ERROR) != 0) {
            throw DiagnosticError(
                "ORV-CAP-503",
                "camera_stream_error",
                "The camera stream reported a device error.",
                74
            );
        }

        if ((stream_flags & MF_SOURCE_READERF_ENDOFSTREAM) != 0) {
            throw DiagnosticError(
                "ORV-CAP-503",
                "camera_stream_ended",
                "The camera stream ended before capture completed.",
                74
            );
        }

        if (
            (stream_flags & MF_SOURCE_READERF_NATIVEMEDIATYPECHANGED) != 0 ||
            (stream_flags & MF_SOURCE_READERF_CURRENTMEDIATYPECHANGED) != 0
        ) {
            throw DiagnosticError(
                "ORV-CAP-505",
                "camera_format_changed",
                "The camera format changed during capture.",
                75
            );
        }

        if (sample == nullptr) {
            return {false, timestamp, 0};
        }

        DWORD byte_count = 0;
        const HRESULT length_result = sample->GetTotalLength(&byte_count);

        if (FAILED(length_result)) {
            throw HResultError(length_result, "IMFSample::GetTotalLength");
        }

        ComPtr<IMFMediaBuffer> contiguous_buffer;
        const HRESULT buffer_result = sample->ConvertToContiguousBuffer(
            &contiguous_buffer
        );
        if (FAILED(buffer_result)) {
            throw HResultError(
                buffer_result,
                "IMFSample::ConvertToContiguousBuffer"
            );
        }

        std::vector<std::byte> payload(
            static_cast<std::size_t>(byte_count)
        );
        BYTE* data = nullptr;
        DWORD maximum_length = 0;
        DWORD current_length = 0;
        const HRESULT lock_result = contiguous_buffer->Lock(
            &data,
            &maximum_length,
            &current_length
        );
        if (FAILED(lock_result)) {
            throw HResultError(lock_result, "IMFMediaBuffer::Lock");
        }

        if (current_length != byte_count || current_length > maximum_length) {
            static_cast<void>(contiguous_buffer->Unlock());
            throw std::runtime_error(
                "Media Foundation returned an inconsistent frame length."
            );
        }

        if (byte_count != 0) {
            std::memcpy(payload.data(), data, byte_count);
        }

        const HRESULT unlock_result = contiguous_buffer->Unlock();
        if (FAILED(unlock_result)) {
            throw HResultError(unlock_result, "IMFMediaBuffer::Unlock");
        }

        return {
            true,
            static_cast<std::int64_t>(timestamp),
            static_cast<std::size_t>(byte_count),
            stride_,
            std::move(payload)
        };
    }

private:
    IMFSourceReader* source_reader_;
    std::uint32_t stride_{};
};

}  // namespace

struct MediaFoundationCamera::Impl final {
    ComRuntime com_runtime;
    MediaFoundationRuntime media_foundation_runtime;
    ComPtr<IMFMediaSource> media_source;
    ComPtr<IMFSourceReader> source_reader;
    VideoFormat configured_format;
    bool format_configured{false};
};

MediaFoundationCamera::MediaFoundationCamera()
    : impl_(std::make_unique<Impl>()) {}

MediaFoundationCamera::~MediaFoundationCamera() {
    close();
}

void MediaFoundationCamera::open(const CameraDevice& device) {
    if (is_open()) {
        throw std::logic_error("A camera is already open.");
    }

    if (device.symbolic_link.empty()) {
        throw std::invalid_argument(
            "The selected camera does not provide a symbolic link."
        );
    }

    ComPtr<IMFAttributes> attributes;
    HRESULT result = MFCreateAttributes(&attributes, 2);

    if (FAILED(result)) {
        throw HResultError(result, "MFCreateAttributes(camera source)");
    }

    result = attributes->SetGUID(
        MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
        MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID
    );

    if (FAILED(result)) {
        throw HResultError(
            result,
            "IMFAttributes::SetGUID(video capture)"
        );
    }

    const std::wstring symbolic_link = utf8_to_wide(device.symbolic_link);
    result = attributes->SetString(
        MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK,
        symbolic_link.c_str()
    );

    if (FAILED(result)) {
        throw HResultError(
            result,
            "IMFAttributes::SetString(camera symbolic link)"
        );
    }

    ComPtr<IMFMediaSource> media_source;
    result = MFCreateDeviceSource(attributes.Get(), &media_source);

    if (FAILED(result)) {
        throw HResultError(result, "MFCreateDeviceSource");
    }

    DWORD characteristics = 0;
    result = media_source->GetCharacteristics(&characteristics);

    if (FAILED(result)) {
        static_cast<void>(media_source->Shutdown());
        throw HResultError(result, "IMFMediaSource::GetCharacteristics");
    }

    ComPtr<IMFSourceReader> source_reader;
    result = MFCreateSourceReaderFromMediaSource(
        media_source.Get(),
        nullptr,
        &source_reader
    );

    if (FAILED(result)) {
        static_cast<void>(media_source->Shutdown());
        throw HResultError(result, "MFCreateSourceReaderFromMediaSource");
    }

    result = source_reader->SetStreamSelection(
        kAllStreams,
        FALSE
    );

    if (FAILED(result)) {
        source_reader.Reset();
        static_cast<void>(media_source->Shutdown());
        throw HResultError(result, "IMFSourceReader::SetStreamSelection(all)");
    }

    result = source_reader->SetStreamSelection(
        kFirstVideoStream,
        TRUE
    );

    if (FAILED(result)) {
        source_reader.Reset();
        static_cast<void>(media_source->Shutdown());
        throw HResultError(result, "IMFSourceReader::SetStreamSelection(video)");
    }

    impl_->media_source = std::move(media_source);
    impl_->source_reader = std::move(source_reader);
    impl_->format_configured = false;
}

void MediaFoundationCamera::close() noexcept {
    if (impl_ == nullptr) {
        return;
    }

    impl_->source_reader.Reset();
    impl_->configured_format = {};
    impl_->format_configured = false;

    if (impl_->media_source != nullptr) {
        static_cast<void>(impl_->media_source->Shutdown());
        impl_->media_source.Reset();
    }
}

std::vector<VideoFormat> MediaFoundationCamera::available_formats() const {
    if (!is_open() || impl_->source_reader == nullptr) {
        throw std::logic_error(
            "A camera must be open before enumerating video formats."
        );
    }

    std::vector<VideoFormat> formats;

    for (DWORD index = 0;; ++index) {
        ComPtr<IMFMediaType> media_type;
        const HRESULT result = impl_->source_reader->GetNativeMediaType(
            kFirstVideoStream,
            index,
            &media_type
        );

        if (result == MF_E_NO_MORE_TYPES) {
            break;
        }

        if (FAILED(result)) {
            throw HResultError(
                result,
                "IMFSourceReader::GetNativeMediaType"
            );
        }

        VideoFormat format;
        if (try_read_video_format(media_type.Get(), index, format)) {
            formats.push_back(std::move(format));
        }
    }

    return formats;
}

VideoFormat MediaFoundationCamera::configure(const VideoFormatTarget& target) {
    if (!is_open() || impl_->source_reader == nullptr) {
        throw std::logic_error(
            "A camera must be open before configuring a video format."
        );
    }

    impl_->format_configured = false;

    const std::vector<VideoFormat> formats = available_formats();
    const VideoFormat& selected =
        VideoFormatSelector::select_best(formats, target);

    ComPtr<IMFMediaType> media_type;
    HRESULT result = impl_->source_reader->GetNativeMediaType(
        kFirstVideoStream,
        static_cast<DWORD>(selected.native_type_index),
        &media_type
    );

    if (FAILED(result)) {
        throw HResultError(
            result,
            "IMFSourceReader::GetNativeMediaType(selected)"
        );
    }

    result = impl_->source_reader->SetCurrentMediaType(
        kFirstVideoStream,
        nullptr,
        media_type.Get()
    );

    if (FAILED(result)) {
        throw HResultError(result, "IMFSourceReader::SetCurrentMediaType");
    }

    ComPtr<IMFMediaType> configured_type;
    result = impl_->source_reader->GetCurrentMediaType(
        kFirstVideoStream,
        &configured_type
    );

    if (FAILED(result)) {
        throw HResultError(result, "IMFSourceReader::GetCurrentMediaType");
    }

    VideoFormat configured;
    if (!try_read_video_format(
        configured_type.Get(),
        selected.native_type_index,
        configured
    )) {
        throw std::runtime_error(
            "Media Foundation returned an incomplete configured video format."
        );
    }

    impl_->format_configured = true;
    impl_->configured_format = configured;
    return configured;
}

CaptureSummary MediaFoundationCamera::capture_frames(
    const std::size_t requested_frames,
    FrameSink* const sink
) {
    if (!is_open() || impl_->source_reader == nullptr) {
        throw std::logic_error(
            "A camera must be open before capturing frames."
        );
    }

    if (!impl_->format_configured) {
        throw std::logic_error(
            "A video format must be configured before capturing frames."
        );
    }

    MediaFoundationFrameReader reader(
        impl_->source_reader.Get(),
        impl_->configured_format.stride
    );
    return ContinuousFrameCapture::run(
        reader,
        requested_frames,
        impl_->configured_format,
        1000,
        sink
    );
}

bool MediaFoundationCamera::is_open() const noexcept {
    return impl_ != nullptr && impl_->media_source != nullptr;
}

}  // namespace orvix::capture
