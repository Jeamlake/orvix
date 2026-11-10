#include "orvix/capture/camera_selector.hpp"

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <vector>

int main() {
    const std::vector<orvix::capture::CameraDevice> devices{
        {0, "Camera A", "device-a", "Test Backend"},
        {1, "Camera B", "device-b", "Test Backend"}
    };

    const auto& selected =
        orvix::capture::CameraSelector::select_by_index(devices, 1);

    if (selected.index != 1 || selected.friendly_name != "Camera B") {
        std::cerr << "valid selection assertion failed\n";
        return EXIT_FAILURE;
    }

    bool invalid_index_rejected = false;

    try {
        static_cast<void>(
            orvix::capture::CameraSelector::select_by_index(devices, 8)
        );
    }
    catch (const std::out_of_range&) {
        invalid_index_rejected = true;
    }

    if (!invalid_index_rejected) {
        std::cerr << "invalid index was not rejected\n";
        return EXIT_FAILURE;
    }

    bool empty_list_rejected = false;

    try {
        const std::vector<orvix::capture::CameraDevice> empty;
        static_cast<void>(
            orvix::capture::CameraSelector::select_by_index(empty, 0)
        );
    }
    catch (const std::out_of_range&) {
        empty_list_rejected = true;
    }

    if (!empty_list_rejected) {
        std::cerr << "empty list was not rejected\n";
        return EXIT_FAILURE;
    }

    std::cout << "TC-CAP-002=PASS\n";
    return EXIT_SUCCESS;
}
