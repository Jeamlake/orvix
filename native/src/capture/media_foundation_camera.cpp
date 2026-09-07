#include "orvix/capture/media_foundation_camera.hpp"

#include "orvix/capture/hresult_error.hpp"
#include "orvix/capture/media_foundation_runtime.hpp"

#include <Windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <wrl/client.h>

#include <stdexcept>
#include <string>
#include <utility>

namespace orvix::capture {

namespace {

using Microsoft::WRL::ComPtr;

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

}  // namespace

struct MediaFoundationCamera::Impl final {
    ComRuntime com_runtime;
    MediaFoundationRuntime media_foundation_runtime;
    ComPtr<IMFMediaSource> media_source;
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

    impl_->media_source = std::move(media_source);
}

void MediaFoundationCamera::close() noexcept {
    if (impl_ != nullptr && impl_->media_source != nullptr) {
        static_cast<void>(impl_->media_source->Shutdown());
        impl_->media_source.Reset();
    }
}

bool MediaFoundationCamera::is_open() const noexcept {
    return impl_ != nullptr && impl_->media_source != nullptr;
}

}  // namespace orvix::capture
