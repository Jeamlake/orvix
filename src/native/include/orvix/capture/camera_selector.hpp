#pragma once

#include "orvix/capture/camera_device.hpp"

#include <cstddef>
#include <vector>

namespace orvix::capture {

class CameraSelector final {
public:
    [[nodiscard]]
    static const CameraDevice& select_by_index(
        const std::vector<CameraDevice>& devices,
        std::size_t index
    );
};

}  // namespace orvix::capture
