#include "orvix/capture/camera_selector.hpp"
#include "orvix/capture/hresult_error.hpp"
#include "orvix/capture/media_foundation_device_enumerator.hpp"

#include <charconv>
#include <cstddef>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

namespace {

constexpr std::string_view kVersion = "0.1.0";

void print_usage() {
    std::cout
        << "ORVIX Capture Core " << kVersion << "\n\n"
        << "Usage:\n"
        << "  orvix-capture devices\n"
        << "  orvix-capture select --index <N>\n"
        << "  orvix-capture --help\n";
}

std::size_t parse_index(const std::string_view value) {
    std::size_t index = 0;

    const char* begin = value.data();
    const char* end = value.data() + value.size();

    const auto result = std::from_chars(begin, end, index);

    if (result.ec != std::errc{} || result.ptr != end) {
        throw std::invalid_argument(
            "Camera index must be a non-negative integer."
        );
    }

    return index;
}

std::vector<orvix::capture::CameraDevice> enumerate_devices() {
    const orvix::capture::MediaFoundationDeviceEnumerator enumerator;
    return enumerator.enumerate();
}

int run_devices() {
    std::cout
        << "ORVIX Capture Core " << kVersion << "\n\n"
        << "Backend: Windows Media Foundation\n\n";

    const auto devices = enumerate_devices();

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

int run_select(const std::string_view index_text) {
    const std::size_t requested_index = parse_index(index_text);
    const auto devices = enumerate_devices();

    try {
        const auto& selected =
            orvix::capture::CameraSelector::select_by_index(
                devices,
                requested_index
            );

        std::cout
            << "ORVIX Capture Core " << kVersion << "\n\n"
            << "Selected video capture device:\n\n"
            << "Index: " << selected.index << "\n"
            << "Name: " << selected.friendly_name << "\n"
            << "Backend: " << selected.backend << "\n"
            << "Symbolic link: " << selected.symbolic_link << "\n\n"
            << "Selection status: READY_FOR_OPEN\n";

        return 0;
    }
    catch (const std::out_of_range& error) {
        std::cerr
            << "[ORV-CAP-404] Camera selection failed: "
            << error.what()
            << "\n";

        return 66;
    }
}

}  // namespace

int main(const int argc, char* argv[]) {
    try {
        if (argc == 2) {
            const std::string command = argv[1];

            if (command == "devices") {
                return run_devices();
            }

            if (command == "--help" || command == "-h") {
                print_usage();
                return 0;
            }
        }

        if (
            argc == 4 &&
            std::string_view(argv[1]) == "select" &&
            std::string_view(argv[2]) == "--index"
        ) {
            return run_select(argv[3]);
        }

        std::cerr << "[ORV-CAP-400] Invalid command or arguments.\n\n";
        print_usage();
        return 64;
    }
    catch (const std::invalid_argument& error) {
        std::cerr
            << "[ORV-CAP-400] Invalid argument: "
            << error.what()
            << "\n";

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
