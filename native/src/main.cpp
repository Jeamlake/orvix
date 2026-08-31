#include "orvix/capture/hresult_error.hpp"
#include "orvix/capture/media_foundation_device_enumerator.hpp"

#include <exception>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

namespace {

constexpr std::string_view kVersion = "0.1.0";

void print_usage() {
    std::cout
        << "ORVIX Capture Core " << kVersion << "\n\n"
        << "Usage:\n"
        << "  orvix-capture devices\n"
        << "  orvix-capture --help\n";
}

int run_devices() {
    std::cout
        << "ORVIX Capture Core " << kVersion << "\n\n"
        << "Backend: Windows Media Foundation\n\n";

    const orvix::capture::MediaFoundationDeviceEnumerator enumerator;
    const std::vector<orvix::capture::CameraDevice> devices =
        enumerator.enumerate();

    if (devices.empty()) {
        std::cout
            << "[ORV-CAP-100] No video capture devices were found.\n\n"
            << "Device count: 0\n";

        return 0;
    }

    std::cout << "Available video capture devices:\n\n";

    for (const auto& device : devices) {
        std::cout
            << "[" << device.index << "] "
            << device.friendly_name << "\n"
            << "    Backend: " << device.backend << "\n"
            << "    Symbolic link: " << device.symbolic_link << "\n\n";
    }

    std::cout << "Device count: " << devices.size() << "\n";

    return 0;
}

}  // namespace

int main(const int argc, char* argv[]) {
    try {
        if (argc != 2) {
            print_usage();
            return 64;
        }

        const std::string command = argv[1];

        if (command == "devices") {
            return run_devices();
        }

        if (command == "--help" || command == "-h") {
            print_usage();
            return 0;
        }

        std::cerr
            << "[ORV-CAP-400] Unknown command: "
            << command
            << "\n\n";

        print_usage();

        return 64;
    }
    catch (const orvix::capture::HResultError& error) {
        std::cerr
            << "[ORV-CAP-500] Native capture API failure: "
            << error.what()
            << "\n";

        return 70;
    }
    catch (const std::exception& error) {
        std::cerr
            << "[ORV-CAP-599] Unexpected failure: "
            << error.what()
            << "\n";

        return 70;
    }
}