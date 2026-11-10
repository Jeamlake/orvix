#include "orvix/capture/platform_factory.hpp"

#include <cstdlib>
#include <iostream>
#include <string_view>

int main() {
    const auto enumerator =
        orvix::capture::create_platform_device_enumerator();
    const auto camera = orvix::capture::create_platform_camera();
    const auto backend_name = orvix::capture::platform_backend_name();

#if defined(_WIN32)
    constexpr std::string_view expected_backend =
        "Windows Media Foundation";
#elif defined(__APPLE__)
    constexpr std::string_view expected_backend = "macOS AVFoundation";
#elif defined(__linux__)
    constexpr std::string_view expected_backend = "Linux V4L2";
#endif

    if (
        enumerator == nullptr ||
        camera == nullptr ||
        camera->is_open() ||
        backend_name != expected_backend
    ) {
        std::cerr << "platform capture factory assertion failed\n";
        return EXIT_FAILURE;
    }

    std::cout << "TC-CAP-003-PLATFORM=" << backend_name << "\n";
    return EXIT_SUCCESS;
}
