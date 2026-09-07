#include "orvix/capture/hresult_error.hpp"
#include "orvix/capture/media_foundation_camera.hpp"

#include <cstdlib>
#include <iostream>
#include <stdexcept>

int main() {
    orvix::capture::MediaFoundationCamera camera;

    if (camera.is_open()) {
        std::cerr << "new camera unexpectedly reports OPEN\n";
        return EXIT_FAILURE;
    }

    camera.close();
    camera.close();

    const orvix::capture::CameraDevice incomplete_device{
        0,
        "Incomplete Test Camera",
        "",
        "Test Backend"
    };

    bool empty_symbolic_link_rejected = false;

    try {
        camera.open(incomplete_device);
    }
    catch (const std::invalid_argument&) {
        empty_symbolic_link_rejected = true;
    }

    if (!empty_symbolic_link_rejected || camera.is_open()) {
        std::cerr << "invalid camera descriptor was not rejected safely\n";
        return EXIT_FAILURE;
    }

    const orvix::capture::CameraDevice unavailable_device{
        0,
        "Unavailable Test Camera",
        R"(\\?\orvix#nonexistent_camera#tc_cap_003)",
        "Windows Media Foundation"
    };

    bool unavailable_device_rejected = false;

    try {
        camera.open(unavailable_device);
    }
    catch (const orvix::capture::HResultError&) {
        unavailable_device_rejected = true;
    }

    if (!unavailable_device_rejected || camera.is_open()) {
        std::cerr << "unavailable camera was not rejected safely\n";
        return EXIT_FAILURE;
    }

    std::cout << "TC-CAP-003=PASS\n";
    return EXIT_SUCCESS;
}
