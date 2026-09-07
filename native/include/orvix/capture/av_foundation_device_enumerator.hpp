#pragma once

#include "orvix/capture/device_enumerator.hpp"

namespace orvix::capture {

class AvFoundationDeviceEnumerator final : public DeviceEnumerator {
public:
    [[nodiscard]]
    std::vector<CameraDevice> enumerate() const override;
};

}  // namespace orvix::capture
