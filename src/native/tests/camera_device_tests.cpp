#include "orvix/capture/camera_device.hpp"

#include <cstdlib>
#include <iostream>
#include <string>

int main() {
    const orvix::capture::CameraDevice device{
        3,
        "Synthetic Test Camera",
        R"(\\?\usb#vid_0000&pid_0000#test)",
        "Test Backend"
    };

    if (device.index != 3) {
        std::cerr << "index assertion failed\n";
        return EXIT_FAILURE;
    }

    if (device.friendly_name != "Synthetic Test Camera") {
        std::cerr << "friendly_name assertion failed\n";
        return EXIT_FAILURE;
    }

    if (device.symbolic_link.empty()) {
        std::cerr << "symbolic_link assertion failed\n";
        return EXIT_FAILURE;
    }

    if (device.backend != "Test Backend") {
        std::cerr << "backend assertion failed\n";
        return EXIT_FAILURE;
    }

    std::cout << "TC-CAP-001-MODEL=PASS\n";
    return EXIT_SUCCESS;
}