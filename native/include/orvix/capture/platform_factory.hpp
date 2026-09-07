#pragma once

#include "orvix/capture/camera.hpp"
#include "orvix/capture/device_enumerator.hpp"

#include <memory>
#include <string_view>

namespace orvix::capture {

[[nodiscard]]
std::unique_ptr<DeviceEnumerator> create_platform_device_enumerator();

[[nodiscard]]
std::unique_ptr<Camera> create_platform_camera();

[[nodiscard]]
std::string_view platform_backend_name() noexcept;

}  // namespace orvix::capture
