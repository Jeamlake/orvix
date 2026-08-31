#include "orvix/capture/camera_selector.hpp"

#include <sstream>
#include <stdexcept>

namespace orvix::capture {

const CameraDevice& CameraSelector::select_by_index(
    const std::vector<CameraDevice>& devices,
    const std::size_t index
) {
    if (index >= devices.size()) {
        std::ostringstream message;
        message
            << "Camera index "
            << index
            << " is out of range. Available devices: "
            << devices.size();

        throw std::out_of_range(message.str());
    }

    return devices[index];
}

}  // namespace orvix::capture
