#pragma once

#include "orvix/capture/camera_device.hpp"

#include <memory>

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
    bool is_open() const noexcept;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace orvix::capture
