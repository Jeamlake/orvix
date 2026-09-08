#pragma once

#include "orvix/capture/camera_device.hpp"
#include "orvix/capture/continuous_frame_capture.hpp"
#include "orvix/capture/video_format.hpp"

#include <cstddef>
#include <vector>

namespace orvix::capture {

class Camera {
public:
    virtual ~Camera() = default;

    virtual void open(const CameraDevice& device) = 0;
    virtual void close() noexcept = 0;

    [[nodiscard]]
    virtual std::vector<VideoFormat> available_formats() const = 0;

    [[nodiscard]]
    virtual VideoFormat configure(const VideoFormatTarget& target) = 0;

    [[nodiscard]]
    virtual CaptureSummary capture_frames(
        std::size_t requested_frames,
        FrameSink* sink = nullptr
    ) = 0;

    [[nodiscard]]
    virtual bool is_open() const noexcept = 0;
};

}  // namespace orvix::capture
