#pragma once

#include "orvix/capture/camera.hpp"

#include <memory>

namespace orvix::capture {

class V4l2Camera final : public Camera {
public:
    V4l2Camera();
    ~V4l2Camera() override;

    V4l2Camera(const V4l2Camera&) = delete;
    V4l2Camera& operator=(const V4l2Camera&) = delete;
    V4l2Camera(V4l2Camera&&) = delete;
    V4l2Camera& operator=(V4l2Camera&&) = delete;

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
