#include "orvix/cli/capture_cli.hpp"

#include "orvix/capture/camera_selector.hpp"
#include "orvix/capture/diagnostic_error.hpp"
#include "orvix/capture/platform_factory.hpp"
#include "orvix/capture/synthetic_frame_reader.hpp"
#include "orvix/ipc/shared_memory_protocol.hpp"
#include "orvix/ipc/shared_memory_publisher.hpp"
#include "orvix/observability/logger.hpp"

#include <charconv>
#include <cstddef>
#include <exception>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

namespace orvix::cli {

namespace {

using capture::CameraDevice;
using capture::Camera;
using capture::CaptureSummary;
using capture::DiagnosticError;
using capture::VideoFormat;
using observability::Logger;

constexpr std::string_view kVersion = "0.2.0";
constexpr std::size_t kDefaultCaptureFrames = 120;
constexpr std::size_t kDefaultBridgeFrames = 900;

struct BridgeOptions final {
    bool synthetic{false};
    bool has_index{false};
    std::size_t index{};
    std::size_t frames{kDefaultBridgeFrames};
    std::string name{ipc::kDefaultSharedMemoryName};
};

void log_camera_opened(Logger& logger, const CameraDevice& selected);
void print_video_format(
    const VideoFormat& format,
    std::string_view prefix
);

void print_usage() {
    std::cout
        << "ORVIX Capture Core " << kVersion << "\n\n"
        << "Usage:\n"
        << "  orvix-capture devices\n"
        << "  orvix-capture select --index <N>\n"
        << "  orvix-capture open --index <N>\n"
        << "  orvix-capture formats --index <N>\n"
        << "  orvix-capture capture --index <N>\n"
        << "  orvix-capture bridge --index <N> [--frames <N>] [--name <NAME>]\n"
        << "  orvix-capture bridge --synthetic [--frames <N>] [--name <NAME>]\n"
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

std::size_t parse_positive_size(
    const std::string_view value,
    const std::string_view label
) {
    const std::size_t parsed = parse_index(value);
    if (parsed == 0) {
        throw std::invalid_argument(
            std::string(label) + " must be greater than zero."
        );
    }
    return parsed;
}

BridgeOptions parse_bridge_options(const int argc, char* argv[]) {
    BridgeOptions options;

    for (int position = 2; position < argc; ++position) {
        const std::string_view argument = argv[position];
        if (argument == "--synthetic") {
            if (options.synthetic) {
                throw std::invalid_argument("--synthetic was specified twice.");
            }
            options.synthetic = true;
            continue;
        }

        if (
            argument != "--index" &&
            argument != "--frames" &&
            argument != "--name"
        ) {
            throw std::invalid_argument(
                "Unknown bridge argument: " + std::string(argument)
            );
        }
        if (position + 1 >= argc) {
            throw std::invalid_argument(
                "Missing value after " + std::string(argument) + "."
            );
        }

        const std::string_view value = argv[++position];
        if (argument == "--index") {
            if (options.has_index) {
                throw std::invalid_argument("--index was specified twice.");
            }
            options.index = parse_index(value);
            options.has_index = true;
        }
        else if (argument == "--frames") {
            options.frames = parse_positive_size(value, "Frame count");
        }
        else {
            options.name = value;
        }
    }

    if (options.synthetic == options.has_index) {
        throw std::invalid_argument(
            "Bridge requires exactly one source: --index <N> or --synthetic."
        );
    }
    return options;
}

std::vector<CameraDevice> enumerate_devices() {
    try {
        const auto enumerator =
            capture::create_platform_device_enumerator();
        return enumerator->enumerate();
    }
    catch (const DiagnosticError&) {
        throw;
    }
    catch (const std::runtime_error& error) {
        throw DiagnosticError(
            "ORV-CAP-500",
            "camera_enumeration_failed",
            "Unable to enumerate video devices: " +
                std::string(error.what()),
            70
        );
    }
}

const CameraDevice& select_device(
    const std::vector<CameraDevice>& devices,
    const std::size_t requested_index
) {
    if (devices.empty()) {
        throw DiagnosticError(
            "ORV-CAP-100",
            "camera_absent",
            "No video capture devices were found.",
            65
        );
    }

    return capture::CameraSelector::select_by_index(
        devices,
        requested_index
    );
}

std::unique_ptr<Camera> open_camera(
    const CameraDevice& selected
) {
    try {
        auto camera = capture::create_platform_camera();
        camera->open(selected);

        if (!camera->is_open()) {
            throw DiagnosticError(
                "ORV-CAP-501",
                "camera_open_failed",
                "The native backend returned without an active camera.",
                71
            );
        }

        return camera;
    }
    catch (const DiagnosticError&) {
        throw;
    }
    catch (const std::runtime_error& error) {
        throw DiagnosticError(
            "ORV-CAP-501",
            "camera_open_failed",
            "Unable to open the selected camera: " +
                std::string(error.what()),
            71
        );
    }
}

VideoFormat configure_camera(Camera& camera) {
    try {
        return camera.configure(capture::VideoFormatTarget{});
    }
    catch (const DiagnosticError&) {
        throw;
    }
    catch (const std::runtime_error& error) {
        throw DiagnosticError(
            "ORV-CAP-502",
            "format_negotiation_failed",
            "Unable to configure a camera video format: " +
                std::string(error.what()),
            72
        );
    }
}

CaptureSummary capture_frames(Camera& camera) {
    try {
        return camera.capture_frames(kDefaultCaptureFrames);
    }
    catch (const DiagnosticError&) {
        throw;
    }
    catch (const std::runtime_error& error) {
        throw DiagnosticError(
            "ORV-CAP-504",
            "frame_capture_failed",
            error.what(),
            75
        );
    }
}

void print_bridge_started(
    const BridgeOptions& options,
    const VideoFormat& format,
    const std::string_view source
) {
    std::cout
        << "ORVIX Capture Core " << kVersion << "\n\n"
        << "Shared-memory video bridge\n\n"
        << "Source: " << source << "\n"
        << "Shared memory: " << options.name << "\n"
        << "Ring slots: " << ipc::kDefaultSlotCount << "\n"
        << "Frames requested: " << options.frames << "\n";
    print_video_format(format, "Published");
    std::cout
        << "\nBridge status: RUNNING\n"
        << "Open another terminal and run:\n"
        << "  python -m orvix.ui.viewer --name "
        << options.name << "\n\n";
}

void print_bridge_completed(
    const CaptureSummary& summary,
    const ipc::PublisherStats& statistics
) {
    std::cout
        << "\nFrames published: " << statistics.published_sequence << "\n"
        << "Last sequence consumed: " << statistics.consumed_sequence << "\n"
        << "Frames overwritten before consumption: "
        << statistics.overwritten_frames << "\n"
        << "Final buffer utilization: "
        << std::fixed << std::setprecision(1)
        << statistics.buffer_utilization * 100.0 << "%\n"
        << "Measured producer FPS: "
        << std::fixed << std::setprecision(2)
        << statistics.producer_fps << "\n"
        << "Capture FPS: " << summary.capture_fps() << "\n"
        << "Bridge status: COMPLETED\n";
}

CaptureSummary publish_reader(
    capture::FrameReader& reader,
    const BridgeOptions& options,
    const VideoFormat& format,
    Logger& logger,
    const std::string_view source
) {
    ipc::SharedMemoryPublisher publisher(options.name, format);
    print_bridge_started(options, format, source);
    logger.info(
        "ORV-IPC-200",
        "shared_memory_bridge_started",
        "name=" + options.name +
            " source=" + std::string(source) +
            " requested_frames=" + std::to_string(options.frames)
    );

    try {
        CaptureSummary summary = capture::ContinuousFrameCapture::run(
            reader,
            options.frames,
            format,
            1000,
            &publisher
        );
        publisher.mark_completed();
        const auto statistics = publisher.stats();
        print_bridge_completed(summary, statistics);
        logger.info(
            "ORV-IPC-201",
            "shared_memory_bridge_completed",
            "published_sequence=" +
                std::to_string(statistics.published_sequence) +
                " consumed_sequence=" +
                std::to_string(statistics.consumed_sequence) +
                " overwritten_frames=" +
                std::to_string(statistics.overwritten_frames)
        );
        return summary;
    }
    catch (...) {
        publisher.mark_failed();
        logger.error(
            "ORV-IPC-500",
            "shared_memory_bridge_failed",
            "The producer stopped after a bridge failure."
        );
        throw;
    }
}

int run_bridge(Logger& logger, const BridgeOptions& options) {
    if (options.synthetic) {
        capture::SyntheticFrameReader reader;
        static_cast<void>(publish_reader(
            reader,
            options,
            reader.format(),
            logger,
            "synthetic NV12"
        ));
        return 0;
    }

    const auto devices = enumerate_devices();
    const auto& selected = select_device(devices, options.index);
    auto camera = open_camera(selected);
    log_camera_opened(logger, selected);
    const auto configured = configure_camera(*camera);
    ipc::SharedMemoryPublisher publisher(options.name, configured);
    print_bridge_started(
        options,
        configured,
        selected.friendly_name
    );
    logger.info(
        "ORV-IPC-200",
        "shared_memory_bridge_started",
        "name=" + options.name +
            " source=" + selected.friendly_name +
            " requested_frames=" + std::to_string(options.frames)
    );

    try {
        const CaptureSummary summary = camera->capture_frames(
            options.frames,
            &publisher
        );
        publisher.mark_completed();
        const auto statistics = publisher.stats();
        print_bridge_completed(summary, statistics);
        logger.info(
            "ORV-IPC-201",
            "shared_memory_bridge_completed",
            "published_sequence=" +
                std::to_string(statistics.published_sequence) +
                " consumed_sequence=" +
                std::to_string(statistics.consumed_sequence) +
                " overwritten_frames=" +
                std::to_string(statistics.overwritten_frames)
        );
    }
    catch (...) {
        publisher.mark_failed();
        throw;
    }

    camera->close();
    return 0;
}

std::string format_description(const VideoFormat& format) {
    std::ostringstream message;
    message
        << "resolution=" << format.width << "x" << format.height
        << " pixel_format=" << format.pixel_format
        << " fps=" << std::fixed << std::setprecision(2)
        << format.frames_per_second()
        << " native_type_index=" << format.native_type_index;
    return message.str();
}

void log_camera_opened(Logger& logger, const CameraDevice& selected) {
    logger.info(
        "ORV-CAP-200",
        "camera_opened",
        "index=" + std::to_string(selected.index) +
            " name=" + selected.friendly_name +
            " backend=" + selected.backend
    );
}

void print_video_format(
    const VideoFormat& format,
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

int run_devices(Logger& logger) {
    std::cout
        << "ORVIX Capture Core " << kVersion << "\n\n"
        << "Backend: " << capture::platform_backend_name() << "\n\n";

    const auto devices = enumerate_devices();

    if (devices.empty()) {
        logger.warning(
            "ORV-CAP-100",
            "camera_absent",
            "Device enumeration returned zero video capture devices."
        );

        std::cout
            << "[ORV-CAP-100] No video capture devices were found.\n\n"
            << "Device count: 0\n"
            << "Log file: " << logger.path().string() << "\n";

        return 0;
    }

    logger.info(
        "ORV-CAP-101",
        "camera_enumeration_completed",
        "device_count=" + std::to_string(devices.size())
    );

    std::cout << "Available video capture devices:\n\n";

    for (const auto& device : devices) {
        std::cout
            << "[" << device.index << "] "
            << device.friendly_name << "\n"
            << "    Backend: " << device.backend << "\n"
            << "    Symbolic link: " << device.symbolic_link << "\n\n";
    }

    std::cout
        << "Device count: " << devices.size() << "\n"
        << "Log file: " << logger.path().string() << "\n";

    return 0;
}

int run_select(Logger& logger, const std::string_view index_text) {
    const std::size_t requested_index = parse_index(index_text);
    const auto devices = enumerate_devices();
    const auto& selected = select_device(devices, requested_index);

    logger.info(
        "ORV-CAP-201",
        "camera_selected",
        "index=" + std::to_string(selected.index) +
            " name=" + selected.friendly_name
    );

    std::cout
        << "ORVIX Capture Core " << kVersion << "\n\n"
        << "Selected video capture device:\n\n"
        << "Index: " << selected.index << "\n"
        << "Name: " << selected.friendly_name << "\n"
        << "Backend: " << selected.backend << "\n"
        << "Symbolic link: " << selected.symbolic_link << "\n\n"
        << "Selection status: READY_FOR_OPEN\n"
        << "Log file: " << logger.path().string() << "\n";

    return 0;
}

int run_open(Logger& logger, const std::string_view index_text) {
    const std::size_t requested_index = parse_index(index_text);
    const auto devices = enumerate_devices();
    const auto& selected = select_device(devices, requested_index);
    auto camera = open_camera(selected);

    log_camera_opened(logger, selected);

    std::cout
        << "ORVIX Capture Core " << kVersion << "\n\n"
        << "Opened video capture device:\n\n"
        << "Index: " << selected.index << "\n"
        << "Name: " << selected.friendly_name << "\n"
        << "Backend: " << selected.backend << "\n\n"
        << "Open status: OPEN\n"
        << "Media source: ACTIVE\n"
        << "Log file: " << logger.path().string() << "\n";

    camera->close();
    logger.info(
        "ORV-CAP-202",
        "camera_closed",
        "Camera resources released after open validation."
    );

    return 0;
}

int run_formats(Logger& logger, const std::string_view index_text) {
    const std::size_t requested_index = parse_index(index_text);
    const auto devices = enumerate_devices();
    const auto& selected = select_device(devices, requested_index);
    auto camera = open_camera(selected);

    log_camera_opened(logger, selected);

    const auto formats = camera->available_formats();
    const auto configured = configure_camera(*camera);

    logger.info(
        "ORV-CAP-210",
        "camera_formats_discovered",
        "usable_native_formats=" + std::to_string(formats.size())
    );
    logger.info(
        "ORV-CAP-211",
        "camera_format_configured",
        format_description(configured)
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
    std::cout
        << "\nFormat status: CONFIGURED\n"
        << "Log file: " << logger.path().string() << "\n";

    camera->close();
    logger.info(
        "ORV-CAP-202",
        "camera_closed",
        "Camera resources released after format validation."
    );

    return 0;
}

int run_capture(Logger& logger, const std::string_view index_text) {
    const std::size_t requested_index = parse_index(index_text);
    const auto devices = enumerate_devices();
    const auto& selected = select_device(devices, requested_index);
    auto camera = open_camera(selected);

    log_camera_opened(logger, selected);

    const auto configured = configure_camera(*camera);
    logger.info(
        "ORV-CAP-211",
        "camera_format_configured",
        format_description(configured)
    );
    logger.info(
        "ORV-CAP-220",
        "capture_started",
        "requested_frames=" + std::to_string(kDefaultCaptureFrames)
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

    const CaptureSummary summary = capture_frames(*camera);

    std::cout << "\nPer-frame metadata:\n";

    for (const auto& frame : summary.frames) {
        std::cout
            << "Frame #" << frame.sequence
            << " | timestamp=" << frame.timestamp_100ns
            << " (100 ns)"
            << " | resolution=" << frame.width << "x" << frame.height
            << " | format=" << frame.pixel_format
            << " | bytes=" << frame.byte_count
            << "\n";
    }

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
        << "Timestamp status: "
        << (
            summary.timestamps_strictly_increasing
                ? "STRICTLY_INCREASING"
                : "NON_MONOTONIC"
        )
        << "\n"
        << "Capture duration: "
        << std::fixed << std::setprecision(3)
        << summary.duration_seconds() << " s\n"
        << "Measured capture FPS: "
        << std::fixed << std::setprecision(2)
        << summary.capture_fps() << "\n"
        << "Capture status: CAPTURE_COMPLETED\n"
        << "Log file: " << logger.path().string() << "\n";

    std::ostringstream completion_message;
    completion_message
        << "captured_frames=" << summary.captured_frames
        << " first_sequence=" << summary.first_sequence
        << " last_sequence=" << summary.last_sequence
        << " total_bytes=" << summary.total_bytes
        << " duration_seconds=" << std::fixed << std::setprecision(3)
        << summary.duration_seconds()
        << " measured_fps=" << std::setprecision(2)
        << summary.capture_fps();
    logger.info(
        "ORV-CAP-221",
        "capture_completed",
        completion_message.str()
    );

    camera->close();
    logger.info(
        "ORV-CAP-202",
        "camera_closed",
        "Camera resources released after capture."
    );

    return 0;
}

int report_error(
    Logger& logger,
    const std::string_view code,
    const std::string_view event,
    const std::string_view message,
    const int exit_code
) noexcept {
    try {
        logger.error(code, event, message);
    }
    catch (const std::exception& logging_error) {
        std::cerr
            << "[ORV-LOG-501] Unable to persist diagnostic: "
            << logging_error.what() << "\n";
    }

    std::cerr << "[" << code << "] " << message << "\n";
    return exit_code;
}

int dispatch(Logger& logger, const int argc, char* argv[]) {
    if (argc >= 3 && std::string_view(argv[1]) == "bridge") {
        return run_bridge(logger, parse_bridge_options(argc, argv));
    }

    if (argc == 2) {
        const std::string command = argv[1];

        if (command == "devices") {
            return run_devices(logger);
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
            return run_select(logger, argv[3]);
        }

        if (std::string_view(argv[1]) == "open") {
            return run_open(logger, argv[3]);
        }

        if (std::string_view(argv[1]) == "formats") {
            return run_formats(logger, argv[3]);
        }

        return run_capture(logger, argv[3]);
    }

    print_usage();
    return report_error(
        logger,
        "ORV-CAP-400",
        "invalid_command",
        "Invalid command or arguments.",
        64
    );
}

}  // namespace

int CaptureCli::run(const int argc, char* argv[]) const {
    std::unique_ptr<Logger> logger;

    try {
        logger = std::make_unique<Logger>();
    }
    catch (const std::exception& error) {
        std::cerr
            << "[ORV-LOG-500] Unable to initialize capture logging: "
            << error.what() << "\n";
        return 78;
    }

    try {
        return dispatch(*logger, argc, argv);
    }
    catch (const DiagnosticError& error) {
        return report_error(
            *logger,
            error.code(),
            error.event(),
            error.what(),
            error.exit_code()
        );
    }
    catch (const std::out_of_range& error) {
        return report_error(
            *logger,
            "ORV-CAP-404",
            "invalid_camera_index",
            "Camera selection failed: " + std::string(error.what()),
            66
        );
    }
    catch (const std::invalid_argument& error) {
        return report_error(
            *logger,
            "ORV-CAP-400",
            "invalid_argument",
            "Invalid argument: " + std::string(error.what()),
            64
        );
    }
    catch (const std::exception& error) {
        return report_error(
            *logger,
            "ORV-CAP-599",
            "unexpected_failure",
            "Unexpected failure: " + std::string(error.what()),
            70
        );
    }
}

}  // namespace orvix::cli
