#include "orvix/capture/platform_factory.hpp"

#if defined(_WIN32)
#include "orvix/capture/media_foundation_camera.hpp"
#include "orvix/capture/media_foundation_device_enumerator.hpp"
#elif defined(__APPLE__)
#include "orvix/capture/av_foundation_camera.hpp"
#include "orvix/capture/av_foundation_device_enumerator.hpp"
#elif defined(__linux__)
#include "orvix/capture/v4l2_camera.hpp"
#include "orvix/capture/v4l2_device_enumerator.hpp"
#else
#error "ORVIX does not have a native capture backend for this platform."
#endif

#include <memory>
#include <string_view>

namespace orvix::capture {

std::unique_ptr<DeviceEnumerator> create_platform_device_enumerator() {
#if defined(_WIN32)
    return std::make_unique<MediaFoundationDeviceEnumerator>();
#elif defined(__APPLE__)
    return std::make_unique<AvFoundationDeviceEnumerator>();
#elif defined(__linux__)
    return std::make_unique<V4l2DeviceEnumerator>();
#endif
}

std::unique_ptr<Camera> create_platform_camera() {
#if defined(_WIN32)
    return std::make_unique<MediaFoundationCamera>();
#elif defined(__APPLE__)
    return std::make_unique<AvFoundationCamera>();
#elif defined(__linux__)
    return std::make_unique<V4l2Camera>();
#endif
}

std::string_view platform_backend_name() noexcept {
#if defined(_WIN32)
    return "Windows Media Foundation";
#elif defined(__APPLE__)
    return "macOS AVFoundation";
#elif defined(__linux__)
    return "Linux V4L2";
#endif
}

}  // namespace orvix::capture
