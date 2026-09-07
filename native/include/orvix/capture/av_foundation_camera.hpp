#pragma once

#include "orvix/capture/camera.hpp"

#include <memory>

namespace orvix::capture {

class AvFoundationCamera final : public Camera {
public:
    AvFoundationCamera();
    ~AvFoundationCamera() override;

    AvFoundationCamera(const AvFoundationCamera&) = delete;
    AvFoundationCamera& operator=(const AvFoundationCamera&) = delete;
    AvFoundationCamera(AvFoundationCamera&&) = delete;
    AvFoundationCamera& operator=(AvFoundationCamera&&) = delete;

    void open(const CameraDevice& device) override;
    void close() noexcept override;

    [[nodiscard]]
    std::vector<VideoFormat> available_formats() const override;

    [[nodiscard]]
    VideoFormat configure(const VideoFormatTarget& target) override;

    [[nodiscard]]
    CaptureSummary capture_frames(
        std::size_t requested_frames
    ) override;

    [[nodiscard]]
    bool is_open() const noexcept override;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace orvix::capture
