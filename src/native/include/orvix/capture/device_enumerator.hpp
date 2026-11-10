#pragma once

#include "orvix/capture/camera_device.hpp"

#include <vector>

namespace orvix::capture {

class DeviceEnumerator {
public:
    virtual ~DeviceEnumerator() = default;

    [[nodiscard]]
    virtual std::vector<CameraDevice> enumerate() const = 0;
};

}  // namespace orvix::capture