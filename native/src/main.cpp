#include "orvix/capture/camera_selector.hpp"
#include "orvix/capture/hresult_error.hpp"
#include "orvix/capture/media_foundation_camera.hpp"
#include "orvix/capture/media_foundation_device_enumerator.hpp"

#include <charconv>
#include <cstddef>
#include <exception>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

namespace {

constexpr std::string_view kVersion = "0.1.0";
constexpr std::size_t kDefaultCaptureFrames = 120;

void print_usage() {
    std::cout
        << "ORVIX Capture Core " << kVersion << "\n\n"
        << "Usage:\n"
        << "  orvix-capture devices\n"
        << "  orvix-capture select --index <N>\n"
        << "  orvix-capture open --index <N>\n"
        << "  orvix-capture formats --index <N>\n"
        << "  orvix-capture capture --index <N>\n"
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

int run_open(const std::string_view index_text) {
    const std::size_t requested_index = parse_index(index_text);
    const auto devices = enumerate_devices();

    try {
        const auto& selected =
            orvix::capture::CameraSelector::select_by_index(
                devices,
                requested_index
            );

        orvix::capture::MediaFoundationCamera camera;
        camera.open(selected);

        if (!camera.is_open()) {
            throw std::runtime_error(
                "Media Foundation returned without an active media source."
            );
        }

        std::cout
            << "ORVIX Capture Core " << kVersion << "\n\n"
            << "Opened video capture device:\n\n"
            << "Index: " << selected.index << "\n"
            << "Name: " << selected.friendly_name << "\n"
            << "Backend: " << selected.backend << "\n\n"
            << "Open status: OPEN\n"
            << "Media source: ACTIVE\n";

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

void print_video_format(
    const orvix::capture::VideoFormat& format,
    const std::string_view prefix
) {
    std::cout
        << prefix << " resolution: "
        << format.width << "x" << format.height << "\n"
        << prefix << " pixel format: " << format.pixel_format << "\n"
        << prefix << " FPS: "
        << std::fixed << std::setprecision(2)
        << format.frames_per_second() << "\n"
        << prefix << " native type index: "
        << format.native_type_index << "\n";
}

int run_formats(const std::string_view index_text) {
    const std::size_t requested_index = parse_index(index_text);
    const auto devices = enumerate_devices();

    try {
        const auto& selected =
            orvix::capture::CameraSelector::select_by_index(
                devices,
                requested_index
            );

        orvix::capture::MediaFoundationCamera camera;
        camera.open(selected);

        const auto formats = camera.available_formats();
        const auto configured = camera.configure(
            orvix::capture::VideoFormatTarget{}
        );

        std::cout
            << "ORVIX Capture Core " << kVersion << "\n\n"
            << "Video formats for: " << selected.friendly_name << "\n\n"
            << "Usable native formats: " << formats.size() << "\n";

        for (const auto& format : formats) {
            std::cout
                << "[" << format.native_type_index << "] "
                << format.width << "x" << format.height << " @ "
                << std::fixed << std::setprecision(2)
                << format.frames_per_second() << " FPS, "
                << format.pixel_format;

            if (format.compressed) {
                std::cout << " (compressed)";
            }

            std::cout << "\n";
        }

        std::cout
            << "\nTarget resolution: 1280x720\n"
            << "Target FPS: 30\n\n";
        print_video_format(configured, "Negotiated");
        std::cout << "\nFormat status: CONFIGURED\n";

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

int run_capture(const std::string_view index_text) {
    const std::size_t requested_index = parse_index(index_text);
    const auto devices = enumerate_devices();

    try {
        const auto& selected =
            orvix::capture::CameraSelector::select_by_index(
                devices,
                requested_index
            );

        orvix::capture::MediaFoundationCamera camera;
        camera.open(selected);

        const auto configured = camera.configure(
            orvix::capture::VideoFormatTarget{}
        );

        std::cout
            << "ORVIX Capture Core " << kVersion << "\n\n"
            << "Continuous native frame capture\n\n"
            << "Device: " << selected.friendly_name << "\n"
            << "Backend: " << selected.backend << "\n"
            << "Target resolution: 1280x720\n"
            << "Target FPS: 30\n";

        print_video_format(configured, "Negotiated");

        std::cout
            << "\nCapturing " << kDefaultCaptureFrames
            << " consecutive frames...\n";

        const auto summary = camera.capture_frames(kDefaultCaptureFrames);

        std::cout
            << "\nFrames requested: " << summary.requested_frames << "\n"
            << "Frames captured: " << summary.captured_frames << "\n"
            << "First sequence: " << summary.first_sequence << "\n"
            << "Last sequence: " << summary.last_sequence << "\n"
            << "First timestamp (100 ns): "
            << summary.first_timestamp_100ns << "\n"
            << "Last timestamp (100 ns): "
            << summary.last_timestamp_100ns << "\n"
            << "Captured bytes: " << summary.total_bytes << "\n"
            << "Empty reads: " << summary.empty_reads << "\n"
            << "Sequence status: STRICTLY_INCREASING\n"
            << "Capture status: CAPTURE_COMPLETED\n";

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
            (
                std::string_view(argv[1]) == "select" ||
                std::string_view(argv[1]) == "open" ||
                std::string_view(argv[1]) == "formats" ||
                std::string_view(argv[1]) == "capture"
            ) &&
            std::string_view(argv[2]) == "--index"
        ) {
            if (std::string_view(argv[1]) == "select") {
                return run_select(argv[3]);
            }

            if (std::string_view(argv[1]) == "open") {
                return run_open(argv[3]);
            }

            if (std::string_view(argv[1]) == "formats") {
                return run_formats(argv[3]);
            }

            return run_capture(argv[3]);
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
