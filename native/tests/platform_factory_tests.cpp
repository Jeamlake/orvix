#include "orvix/capture/platform_factory.hpp"

#include <cstdlib>
#include <iostream>

int main() {
    const auto enumerator =
        orvix::capture::create_platform_device_enumerator();
    const auto camera = orvix::capture::create_platform_camera();
    const auto backend_name = orvix::capture::platform_backend_name();

    if (
        enumerator == nullptr ||
        camera == nullptr ||
        camera->is_open() ||
        backend_name.empty()
    ) {
        std::cerr << "platform capture factory assertion failed\n";
        return EXIT_FAILURE;
    }

    std::cout << "TC-CAP-003-PLATFORM=" << backend_name << "\n";
    return EXIT_SUCCESS;
}
