#include "orvix/capture/v4l2_camera.hpp"

#include "orvix/capture/continuous_frame_capture.hpp"
#include "orvix/capture/diagnostic_error.hpp"
#include "orvix/capture/video_format_selector.hpp"

#include <fcntl.h>
#include <linux/videodev2.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <algorithm>
#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace orvix::capture {

namespace {

struct NativeFormat final {
    VideoFormat format;
    std::uint32_t pixel_format{};
};

struct MappedBuffer final {
    void* address{MAP_FAILED};
    std::size_t length{};
};

int retry_ioctl(const int descriptor, const unsigned long request, void* value) {
    int result = 0;

    do {
        result = ::ioctl(descriptor, request, value);
    } while (result < 0 && errno == EINTR);

    return result;
}

[[noreturn]]
void throw_system_error(const std::string& operation) {
    throw std::runtime_error(
        operation + " failed: " + std::string(std::strerror(errno))
    );
}

std::string pixel_format_name(const std::uint32_t value) {
    switch (value) {
    case V4L2_PIX_FMT_NV12:
        return "NV12";
    case V4L2_PIX_FMT_YUYV:
        return "YUY2";
    case V4L2_PIX_FMT_UYVY:
        return "UYVY";
    case V4L2_PIX_FMT_YUV420:
        return "I420";
    case V4L2_PIX_FMT_RGB24:
        return "RGB24";
    case V4L2_PIX_FMT_BGR24:
        return "BGR24";
    case V4L2_PIX_FMT_MJPEG:
        return "MJPG";
    case V4L2_PIX_FMT_H264:
        return "H264";
    default: {
        std::string fourcc(4, ' ');
        fourcc[0] = static_cast<char>(value & 0xffU);
        fourcc[1] = static_cast<char>((value >> 8U) & 0xffU);
        fourcc[2] = static_cast<char>((value >> 16U) & 0xffU);
        fourcc[3] = static_cast<char>((value >> 24U) & 0xffU);
        return fourcc;
    }
    }
}

bool compressed_format(
    const v4l2_fmtdesc& description,
    const std::uint32_t pixel_format
) noexcept {
    return
        (description.flags & V4L2_FMT_FLAG_COMPRESSED) != 0 ||
        pixel_format == V4L2_PIX_FMT_MJPEG ||
        pixel_format == V4L2_PIX_FMT_H264;
}

void append_intervals(
    const int descriptor,
    const v4l2_fmtdesc& description,
    const std::uint32_t width,
    const std::uint32_t height,
    std::vector<NativeFormat>& formats
) {
    bool interval_added = false;

    for (std::uint32_t index = 0;; ++index) {
        v4l2_frmivalenum interval{};
        interval.index = index;
        interval.pixel_format = description.pixelformat;
        interval.width = width;
        interval.height = height;

        if (retry_ioctl(descriptor, VIDIOC_ENUM_FRAMEINTERVALS, &interval) < 0) {
            if (errno == EINVAL) {
                break;
            }
            throw_system_error("VIDIOC_ENUM_FRAMEINTERVALS");
        }

        std::uint32_t numerator = 30;
        std::uint32_t denominator = 1;

        if (interval.type == V4L2_FRMIVAL_TYPE_DISCRETE) {
            numerator = interval.discrete.denominator;
            denominator = interval.discrete.numerator;
        }
        else {
            const auto& minimum = interval.stepwise.min;
            numerator = minimum.denominator;
            denominator = minimum.numerator;
        }

        if (numerator == 0 || denominator == 0) {
            continue;
        }

        formats.push_back({
            {
                formats.size(),
                width,
                height,
                numerator,
                denominator,
                pixel_format_name(description.pixelformat),
                compressed_format(description, description.pixelformat)
            },
            description.pixelformat
        });
        interval_added = true;

        if (interval.type != V4L2_FRMIVAL_TYPE_DISCRETE) {
            break;
        }
    }

    if (!interval_added) {
        formats.push_back({
            {
                formats.size(),
                width,
                height,
                30,
                1,
                pixel_format_name(description.pixelformat),
                compressed_format(description, description.pixelformat)
            },
            description.pixelformat
        });
    }
}

std::vector<NativeFormat> enumerate_formats(
    const int descriptor,
    const VideoFormatTarget& target
) {
    std::vector<NativeFormat> formats;

    for (std::uint32_t format_index = 0;; ++format_index) {
        v4l2_fmtdesc description{};
        description.index = format_index;
        description.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

        if (retry_ioctl(descriptor, VIDIOC_ENUM_FMT, &description) < 0) {
            if (errno == EINVAL) {
                break;
            }
            throw_system_error("VIDIOC_ENUM_FMT");
        }

        bool size_added = false;

        for (std::uint32_t size_index = 0;; ++size_index) {
            v4l2_frmsizeenum size{};
            size.index = size_index;
            size.pixel_format = description.pixelformat;

            if (retry_ioctl(descriptor, VIDIOC_ENUM_FRAMESIZES, &size) < 0) {
                if (errno == EINVAL) {
                    break;
                }
                throw_system_error("VIDIOC_ENUM_FRAMESIZES");
            }

            std::uint32_t width = target.width;
            std::uint32_t height = target.height;

            if (size.type == V4L2_FRMSIZE_TYPE_DISCRETE) {
                width = size.discrete.width;
                height = size.discrete.height;
            }
            else {
                const auto& range = size.stepwise;
                width = std::clamp(
                    target.width,
                    range.min_width,
                    range.max_width
                );
                height = std::clamp(
                    target.height,
                    range.min_height,
                    range.max_height
                );
            }

            append_intervals(
                descriptor,
                description,
                width,
                height,
                formats
            );
            size_added = true;

            if (size.type != V4L2_FRMSIZE_TYPE_DISCRETE) {
                break;
            }
        }

        if (!size_added) {
            v4l2_format current{};
            current.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

            if (retry_ioctl(descriptor, VIDIOC_G_FMT, &current) == 0) {
                append_intervals(
                    descriptor,
                    description,
                    current.fmt.pix.width,
                    current.fmt.pix.height,
                    formats
                );
            }
        }
    }

    return formats;
}

class V4l2FrameReader final : public FrameReader {
public:
    explicit V4l2FrameReader(const int descriptor)
        : descriptor_(descriptor) {}

    FrameReadResult read_next() override {
        pollfd descriptor_status{};
        descriptor_status.fd = descriptor_;
        descriptor_status.events = POLLIN | POLLPRI;

        const int poll_result = ::poll(&descriptor_status, 1, 250);

        if (poll_result == 0) {
            return {};
        }

        if (poll_result < 0) {
            if (errno == EINTR) {
                return {};
            }
            throw_system_error("poll(camera)");
        }

        if (
            (descriptor_status.revents & (POLLERR | POLLHUP | POLLNVAL)) != 0
        ) {
            throw DiagnosticError(
                "ORV-CAP-503",
                "camera_disconnected",
                "The V4L2 camera was disconnected during capture.",
                74
            );
        }

        v4l2_buffer buffer{};
        buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buffer.memory = V4L2_MEMORY_MMAP;

        if (retry_ioctl(descriptor_, VIDIOC_DQBUF, &buffer) < 0) {
            if (errno == EAGAIN) {
                return {};
            }
            if (errno == ENODEV || errno == EIO) {
                throw DiagnosticError(
                    "ORV-CAP-503",
                    "camera_disconnected",
                    "The V4L2 camera stopped responding during capture.",
                    74
                );
            }
            throw_system_error("VIDIOC_DQBUF");
        }

        const std::int64_t timestamp =
            static_cast<std::int64_t>(buffer.timestamp.tv_sec) * 10'000'000LL +
            static_cast<std::int64_t>(buffer.timestamp.tv_usec) * 10LL;
        const std::size_t byte_count = buffer.bytesused;

        if (retry_ioctl(descriptor_, VIDIOC_QBUF, &buffer) < 0) {
            if (errno == ENODEV || errno == EIO) {
                throw DiagnosticError(
                    "ORV-CAP-503",
                    "camera_disconnected",
                    "The V4L2 camera was removed while returning a frame.",
                    74
                );
            }
            throw_system_error("VIDIOC_QBUF");
        }

        return {true, timestamp, byte_count};
    }

private:
    int descriptor_;
};

}  // namespace

struct V4l2Camera::Impl final {
    int descriptor{-1};
    std::vector<MappedBuffer> buffers;
    VideoFormat configured_format;
    bool streaming{false};
    bool format_configured{false};
};

V4l2Camera::V4l2Camera()
    : impl_(std::make_unique<Impl>()) {}

V4l2Camera::~V4l2Camera() {
    close();
}

void V4l2Camera::open(const CameraDevice& device) {
    if (is_open()) {
        throw std::logic_error("A camera is already open.");
    }

    if (device.symbolic_link.empty()) {
        throw std::invalid_argument("The selected camera path is empty.");
    }

    const int descriptor = ::open(
        device.symbolic_link.c_str(),
        O_RDWR | O_NONBLOCK
    );

    if (descriptor < 0) {
        throw_system_error("open(" + device.symbolic_link + ")");
    }

    v4l2_capability capability{};
    if (retry_ioctl(descriptor, VIDIOC_QUERYCAP, &capability) < 0) {
        const int saved_error = errno;
        static_cast<void>(::close(descriptor));
        errno = saved_error;
        throw_system_error("VIDIOC_QUERYCAP");
    }

    const std::uint32_t capabilities =
        (capability.capabilities & V4L2_CAP_DEVICE_CAPS) != 0
            ? capability.device_caps
            : capability.capabilities;

    if (
        (capabilities & V4L2_CAP_VIDEO_CAPTURE) == 0 ||
        (capabilities & V4L2_CAP_STREAMING) == 0
    ) {
        static_cast<void>(::close(descriptor));
        throw std::runtime_error(
            "The selected V4L2 device does not support streaming capture."
        );
    }

    impl_->descriptor = descriptor;
}

void V4l2Camera::close() noexcept {
    if (impl_ == nullptr) {
        return;
    }

    if (impl_->streaming && impl_->descriptor >= 0) {
        v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        static_cast<void>(retry_ioctl(
            impl_->descriptor,
            VIDIOC_STREAMOFF,
            &type
        ));
    }

    impl_->streaming = false;

    for (const auto& buffer : impl_->buffers) {
        if (buffer.address != MAP_FAILED) {
            static_cast<void>(::munmap(buffer.address, buffer.length));
        }
    }

    impl_->buffers.clear();
    impl_->format_configured = false;
    impl_->configured_format = {};

    if (impl_->descriptor >= 0) {
        static_cast<void>(::close(impl_->descriptor));
        impl_->descriptor = -1;
    }
}

std::vector<VideoFormat> V4l2Camera::available_formats() const {
    if (!is_open()) {
        throw std::logic_error(
            "A camera must be open before enumerating video formats."
        );
    }

    const auto native_formats = enumerate_formats(
        impl_->descriptor,
        VideoFormatTarget{}
    );
    std::vector<VideoFormat> formats;
    formats.reserve(native_formats.size());

    for (const auto& native_format : native_formats) {
        formats.push_back(native_format.format);
    }

    return formats;
}

VideoFormat V4l2Camera::configure(const VideoFormatTarget& target) {
    if (!is_open()) {
        throw std::logic_error(
            "A camera must be open before configuring a video format."
        );
    }

    const auto native_formats = enumerate_formats(impl_->descriptor, target);
    std::vector<VideoFormat> formats;
    formats.reserve(native_formats.size());

    for (const auto& native_format : native_formats) {
        formats.push_back(native_format.format);
    }

    const VideoFormat& selected = VideoFormatSelector::select_best(
        formats,
        target
    );
    const NativeFormat& native = native_formats.at(
        selected.native_type_index
    );

    v4l2_format requested{};
    requested.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    requested.fmt.pix.width = selected.width;
    requested.fmt.pix.height = selected.height;
    requested.fmt.pix.pixelformat = native.pixel_format;
    requested.fmt.pix.field = V4L2_FIELD_ANY;

    if (retry_ioctl(impl_->descriptor, VIDIOC_S_FMT, &requested) < 0) {
        throw_system_error("VIDIOC_S_FMT");
    }

    v4l2_streamparm parameters{};
    parameters.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    parameters.parm.capture.timeperframe.numerator =
        selected.frame_rate_denominator;
    parameters.parm.capture.timeperframe.denominator =
        selected.frame_rate_numerator;

    const int parameter_result = retry_ioctl(
        impl_->descriptor,
        VIDIOC_S_PARM,
        &parameters
    );

    if (parameter_result < 0 && errno != EINVAL) {
        throw_system_error("VIDIOC_S_PARM");
    }

    VideoFormat configured = selected;
    configured.width = requested.fmt.pix.width;
    configured.height = requested.fmt.pix.height;
    configured.pixel_format = pixel_format_name(requested.fmt.pix.pixelformat);

    if (
        parameter_result == 0 &&
        parameters.parm.capture.timeperframe.numerator != 0 &&
        parameters.parm.capture.timeperframe.denominator != 0
    ) {
        configured.frame_rate_numerator =
            parameters.parm.capture.timeperframe.denominator;
        configured.frame_rate_denominator =
            parameters.parm.capture.timeperframe.numerator;
    }

    v4l2_requestbuffers request{};
    request.count = 4;
    request.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    request.memory = V4L2_MEMORY_MMAP;

    if (retry_ioctl(impl_->descriptor, VIDIOC_REQBUFS, &request) < 0) {
        throw_system_error("VIDIOC_REQBUFS");
    }

    if (request.count < 2) {
        throw std::runtime_error(
            "The V4L2 camera did not provide enough capture buffers."
        );
    }

    impl_->buffers.reserve(request.count);

    try {
        for (std::uint32_t index = 0; index < request.count; ++index) {
            v4l2_buffer buffer{};
            buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buffer.memory = V4L2_MEMORY_MMAP;
            buffer.index = index;

            if (retry_ioctl(impl_->descriptor, VIDIOC_QUERYBUF, &buffer) < 0) {
                throw_system_error("VIDIOC_QUERYBUF");
            }

            void* address = ::mmap(
                nullptr,
                buffer.length,
                PROT_READ | PROT_WRITE,
                MAP_SHARED,
                impl_->descriptor,
                static_cast<off_t>(buffer.m.offset)
            );

            if (address == MAP_FAILED) {
                throw_system_error("mmap(camera buffer)");
            }

            impl_->buffers.push_back({address, buffer.length});

            if (retry_ioctl(impl_->descriptor, VIDIOC_QBUF, &buffer) < 0) {
                throw_system_error("VIDIOC_QBUF");
            }
        }

        v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if (retry_ioctl(impl_->descriptor, VIDIOC_STREAMON, &type) < 0) {
            throw_system_error("VIDIOC_STREAMON");
        }
    }
    catch (...) {
        close();
        throw;
    }

    impl_->streaming = true;
    impl_->format_configured = true;
    impl_->configured_format = configured;
    return configured;
}

CaptureSummary V4l2Camera::capture_frames(
    const std::size_t requested_frames
) {
    if (!is_open()) {
        throw std::logic_error("A camera must be open before capturing frames.");
    }

    if (!impl_->format_configured || !impl_->streaming) {
        throw std::logic_error(
            "A video format must be configured before capturing frames."
        );
    }

    V4l2FrameReader reader(impl_->descriptor);
    return ContinuousFrameCapture::run(
        reader,
        requested_frames,
        impl_->configured_format,
        20
    );
}

bool V4l2Camera::is_open() const noexcept {
    return impl_ != nullptr && impl_->descriptor >= 0;
}

}  // namespace orvix::capture
