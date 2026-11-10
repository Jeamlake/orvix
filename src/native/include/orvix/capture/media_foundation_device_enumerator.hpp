#pragma once

#include "orvix/capture/device_enumerator.hpp"

namespace orvix::capture {

class MediaFoundationDeviceEnumerator final : public DeviceEnumerator {
public:
    [[nodiscard]]
    std::vector<CameraDevice> enumerate() const override;
};

}  // namespace orvix::capture