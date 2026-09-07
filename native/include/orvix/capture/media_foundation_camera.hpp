#pragma once

#include "orvix/capture/camera_device.hpp"
#include "orvix/capture/continuous_frame_capture.hpp"
#include "orvix/capture/video_format.hpp"

#include <cstddef>
#include <memory>
#include <vector>

namespace orvix::capture {

class MediaFoundationCamera final {
public:
    MediaFoundationCamera();
    ~MediaFoundationCamera();

    MediaFoundationCamera(const MediaFoundationCamera&) = delete;
    MediaFoundationCamera& operator=(const MediaFoundationCamera&) = delete;
    MediaFoundationCamera(MediaFoundationCamera&&) = delete;
    MediaFoundationCamera& operator=(MediaFoundationCamera&&) = delete;

    void open(const CameraDevice& device);
    void close() noexcept;

    [[nodiscard]]
    std::vector<VideoFormat> available_formats() const;

    [[nodiscard]]
    VideoFormat configure(const VideoFormatTarget& target);

    [[nodiscard]]
    CaptureSummary capture_frames(std::size_t requested_frames);

    [[nodiscard]]
    bool is_open() const noexcept;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace orvix::capture
