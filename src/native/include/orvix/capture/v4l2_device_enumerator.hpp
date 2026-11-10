#pragma once

#include "orvix/capture/device_enumerator.hpp"

namespace orvix::capture {

class V4l2DeviceEnumerator final : public DeviceEnumerator {
public:
    [[nodiscard]]
    std::vector<CameraDevice> enumerate() const override;
};

}  // namespace orvix::capture
