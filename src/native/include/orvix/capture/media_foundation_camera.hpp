#pragma once

#include "orvix/capture/camera.hpp"

#include <cstddef>
#include <memory>
#include <vector>

namespace orvix::capture {

class MediaFoundationCamera final : public Camera {
public:
    MediaFoundationCamera();
    ~MediaFoundationCamera() override;

    MediaFoundationCamera(const MediaFoundationCamera&) = delete;
    MediaFoundationCamera& operator=(const MediaFoundationCamera&) = delete;
    MediaFoundationCamera(MediaFoundationCamera&&) = delete;
    MediaFoundationCamera& operator=(MediaFoundationCamera&&) = delete;

    void open(const CameraDevice& device) override;
    void close() noexcept override;

    [[nodiscard]]
    std::vector<VideoFormat> available_formats() const override;

    [[nodiscard]]
    VideoFormat configure(const VideoFormatTarget& target) override;

    [[nodiscard]]
    CaptureSummary capture_frames(
        std::size_t requested_frames,
        FrameSink* sink = nullptr
    ) override;

    [[nodiscard]]
    bool is_open() const noexcept override;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace orvix::capture
