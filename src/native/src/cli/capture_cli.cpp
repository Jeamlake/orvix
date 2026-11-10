// Incluye las declaraciones de capture cli de ORVIX.
#include "orvix/cli/capture_cli.hpp"

// Incluye las declaraciones de camera selector de ORVIX.
#include "orvix/capture/camera_selector.hpp"
// Incluye las declaraciones de diagnostic error de ORVIX.
#include "orvix/capture/diagnostic_error.hpp"
// Incluye las declaraciones de platform factory de ORVIX.
#include "orvix/capture/platform_factory.hpp"
// Incluye las declaraciones de synthetic frame reader de ORVIX.
#include "orvix/capture/synthetic_frame_reader.hpp"
// Incluye las declaraciones de shared memory protocol de ORVIX.
#include "orvix/ipc/shared_memory_protocol.hpp"
// Incluye las declaraciones de shared memory publisher de ORVIX.
#include "orvix/ipc/shared_memory_publisher.hpp"
// Incluye las declaraciones de logger de ORVIX.
#include "orvix/observability/logger.hpp"

// Importa las declaraciones de charconv para esta unidad.
#include <charconv>
// Incluye cstddef para usar tipos de tamaño y bytes.
#include <cstddef>
// Importa las declaraciones de exception para esta unidad.
#include <exception>
// Incluye iomanip para usar formato de flujos.
#include <iomanip>
// Incluye iostream para usar entrada y salida por consola.
#include <iostream>
// Incluye memory para usar punteros inteligentes y gestión de memoria.
#include <memory>
// Incluye sstream para usar flujos de texto en memoria.
#include <sstream>
// Incluye stdexcept para usar tipos estándar de excepciones.
#include <stdexcept>
// Incluye string para usar cadenas de texto.
#include <string>
// Incluye string_view para usar vistas de cadenas.
#include <string_view>
// Importa las declaraciones de system_error para esta unidad.
#include <system_error>
// Incluye utility para usar utilidades de movimiento e intercambio.
#include <utility>
// Incluye vector para usar contenedores dinámicos.
#include <vector>

// Agrupa las declaraciones bajo orvix::cli.
namespace orvix::cli {

// Limita estas declaraciones a esta unidad de compilación.
namespace {

// Introduce en este ámbito capture::CameraDevice.
using capture::CameraDevice;
// Introduce en este ámbito capture::Camera.
using capture::Camera;
// Introduce en este ámbito capture::CaptureSummary.
using capture::CaptureSummary;
// Introduce en este ámbito capture::DiagnosticError.
using capture::DiagnosticError;
// Introduce en este ámbito capture::VideoFormat.
using capture::VideoFormat;
// Introduce en este ámbito observability::Logger.
using observability::Logger;

// Establece kVersion a partir de "1.0.0".
constexpr std::string_view kVersion = "1.0.0";
// Establece kDefaultCaptureFrames a partir de 120.
constexpr std::size_t kDefaultCaptureFrames = 120;
// Establece kDefaultBridgeFrames a partir de 900.
constexpr std::size_t kDefaultBridgeFrames = 900;

// Declara el tipo BridgeOptions y las operaciones o datos de su contrato.
struct BridgeOptions final {
    // Inicializa synthetic con false.
    bool synthetic{false};
    // Inicializa continuous con false.
    bool continuous{false};
    // Inicializa has_frames con false.
    bool has_frames{false};
    // Inicializa has_index con false.
    bool has_index{false};
    // Inicializa index con inicialización por valor.
    std::size_t index{};
    // Inicializa frames (fotogramas) con kDefaultBridgeFrames.
    std::size_t frames{kDefaultBridgeFrames};
    // Declara target con el tipo indicado.
    capture::VideoFormatTarget target;
    // Inicializa slots (posiciones del anillo) con 3.
    std::uint32_t slots{3};
    // Inicializa name con ipc::kDefaultSharedMemoryName.
    std::string name{ipc::kDefaultSharedMemoryName};
};

// Ejecuta void log_camera_opened(Logger& logger, const CameraDevice& selected).
void log_camera_opened(Logger& logger, const CameraDevice& selected);
// Define la operación print_video_format con los parámetros declarados.
void print_video_format(
    // Declara el parámetro format (formato de video).
    const VideoFormat& format,
    // Incorpora std::string_view prefix en esta operación.
    std::string_view prefix
);

// Define la operación print_usage con los parámetros declarados.
void print_usage() {
    // Incorpora std::cout en esta operación.
    std::cout
        // Incorpora << "ORVIX Capture Core " << kVersion << "\n\n" en esta operación.
        << "ORVIX Capture Core " << kVersion << "\n\n"
        // Incorpora << "Usage:\n" en esta operación.
        << "Usage:\n"
        // Incorpora << " orvix-capture devices\n" en esta operación.
        << "  orvix-capture devices\n"
        // Incorpora << " orvix-capture select --index <N>\n" en esta operación.
        << "  orvix-capture select --index <N>\n"
        // Incorpora << " orvix-capture open --index <N>\n" en esta operación.
        << "  orvix-capture open --index <N>\n"
        // Incorpora << " orvix-capture formats --index <N>\n" en esta operación.
        << "  orvix-capture formats --index <N>\n"
        // Incorpora << " orvix-capture capture --index <N>\n" en esta operación.
        << "  orvix-capture capture --index <N>\n"
        // Incorpora << " orvix-capture bridge --index <N> [--frames <N>] [--name <NAME>]\n" en esta operación.
        << "  orvix-capture bridge --index <N> [--frames <N>] [--name <NAME>]\n"
        // Incorpora << " orvix-capture bridge --synthetic [--frames <N>] [--name <NAME>]\n" en esta operación.
        << "  orvix-capture bridge --synthetic [--frames <N>] [--name <NAME>]\n"
        // Incorpora << " Bridge options: --width <even> --height <even> --fps <1..120> --slots <2..8>\n" en esta operación.
        << "  Bridge options: --width <even> --height <even> --fps <1..120> --slots <2..8>\n"
        // Incorpora << " Add --continuous to run until an explicit IPC stop request.\n" en esta operación.
        << "  Add --continuous to run until an explicit IPC stop request.\n"
        // Incorpora << " orvix-capture --help\n" en esta operación.
        << "  orvix-capture --help\n";
}

// Define la operación parse_index con los parámetros declarados.
std::size_t parse_index(const std::string_view value) {
    // Establece index a partir de 0.
    std::size_t index = 0;
    // Establece begin a partir de value.data().
    const char* begin = value.data();
    // Establece end a partir de value.data() + value.size().
    const char* end = value.data() + value.size();
    // Establece result a partir de std::from_chars(begin, end, index).
    const auto result = std::from_chars(begin, end, index);

    // Evalúa la condición result.ec != std::errc{} || result.ptr != end.
    if (result.ec != std::errc{} || result.ptr != end) {
        // Interrumpe la operación con std::invalid_argument(.
        throw std::invalid_argument(
            // Aporta el texto "Camera index must be a non-negative integer." a la expresión.
            "Camera index must be a non-negative integer."
        );
    }

    // Devuelve index.
    return index;
}

// Define la operación parse_positive_size con los parámetros declarados.
std::size_t parse_positive_size(
    // Declara el parámetro value.
    const std::string_view value,
    // Incorpora const std::string_view label en esta operación.
    const std::string_view label
) {
    // Establece parsed a partir de parse_index(value).
    const std::size_t parsed = parse_index(value);
    // Evalúa la condición parsed == 0.
    if (parsed == 0) {
        // Interrumpe la operación con std::invalid_argument(.
        throw std::invalid_argument(
            // Ejecuta std::string(label) + " must be greater than zero.".
            std::string(label) + " must be greater than zero."
        );
    }
    // Devuelve parsed.
    return parsed;
}

// Define la operación parse_bridge_options con los parámetros declarados.
BridgeOptions parse_bridge_options(const int argc, char* argv[]) {
    // Declara options (opciones solicitadas) con el tipo indicado.
    BridgeOptions options;

    // Recorre según int position = 2; position < argc; ++position.
    for (int position = 2; position < argc; ++position) {
        // Establece argument a partir de argv[position].
        const std::string_view argument = argv[position];
        // Evalúa la condición argument == "--continuous".
        if (argument == "--continuous") {
            // Evalúa la condición options.continuous.
            if (options.continuous) {
                // Interrumpe la operación con std::invalid_argument("--continuous was specified twice.").
                throw std::invalid_argument("--continuous was specified twice.");
            }
            // Establece options.continuous a partir de true.
            options.continuous = true;
            // Pasa a la siguiente iteración.
            continue;
        }
        // Evalúa la condición argument == "--synthetic".
        if (argument == "--synthetic") {
            // Evalúa la condición options.synthetic.
            if (options.synthetic) {
                // Interrumpe la operación con std::invalid_argument("--synthetic was specified twice.").
                throw std::invalid_argument("--synthetic was specified twice.");
            }
            // Establece options.synthetic a partir de true.
            options.synthetic = true;
            // Pasa a la siguiente iteración.
            continue;
        }

        // Evalúa la condición argument != "--index" &&.
        if (
            // Combina esta condición con argument != "--index" &&.
            argument != "--index" &&
            // Combina esta condición con argument != "--frames" &&.
            argument != "--frames" &&
            // Combina esta condición con argument != "--name" && argument != "--width" &&.
            argument != "--name" && argument != "--width" &&
            // Evalúa argument != "--height" && argument != "--fps" && argument != "--slots" como parte de la condición.
            argument != "--height" && argument != "--fps" && argument != "--slots"
        ) {
            // Interrumpe la operación con std::invalid_argument(.
            throw std::invalid_argument(
                // Ejecuta "Unknown bridge argument: " + std::string(argument).
                "Unknown bridge argument: " + std::string(argument)
            );
        }
        // Evalúa la condición position + 1 >= argc.
        if (position + 1 >= argc) {
            // Interrumpe la operación con std::invalid_argument(.
            throw std::invalid_argument(
                // Ejecuta "Missing value after " + std::string(argument) + ".".
                "Missing value after " + std::string(argument) + "."
            );
        }

        // Establece value a partir de argv[++position].
        const std::string_view value = argv[++position];
        // Evalúa la condición argument == "--index".
        if (argument == "--index") {
            // Evalúa la condición options.has_index.
            if (options.has_index) {
                // Interrumpe la operación con std::invalid_argument("--index was specified twice.").
                throw std::invalid_argument("--index was specified twice.");
            }
            // Establece options.index a partir de parse_index(value).
            options.index = parse_index(value);
            // Establece options.has_index a partir de true.
            options.has_index = true;
        }
        // Evalúa la alternativa else if (argument == "--frames") {.
        else if (argument == "--frames") {
            // Evalúa la condición options.has_frames.
            if (options.has_frames) {
                // Interrumpe la operación con std::invalid_argument("--frames was specified twice.").
                throw std::invalid_argument("--frames was specified twice.");
            }
            // Establece options.has_frames a partir de true.
            options.has_frames = true;
            // Establece options.frames (fotogramas) a partir de parse_positive_size(value, "Frame count").
            options.frames = parse_positive_size(value, "Frame count");
        }
        // Evalúa la alternativa else if (argument == "--name") {.
        else if (argument == "--name") {
            // Establece options.name a partir de value.
            options.name = value;
        }
        // Ejecuta la rama alternativa si la condición anterior fue falsa.
        else {
            // Establece parsed a partir de parse_positive_size(value, argument).
            const auto parsed = parse_positive_size(value, argument);
            // Evalúa la condición argument == "--width" || argument == "--height".
            if (argument == "--width" || argument == "--height") {
                // Evalúa la condición parsed < 2 || parsed > 1920 || parsed % 2 != 0.
                if (parsed < 2 || parsed > 1920 || parsed % 2 != 0) {
                    // Interrumpe la operación con std::invalid_argument("Frame dimensions must be even and between 2 and 1920.").
                    throw std::invalid_argument("Frame dimensions must be even and between 2 and 1920.");
                }
                // Establece dimension a partir de argument == "--width" ? options.target.width : options.target.height.
                auto& dimension = argument == "--width" ? options.target.width : options.target.height;
                // Establece dimension a partir de static_cast<std::uint32_t>(parsed).
                dimension = static_cast<std::uint32_t>(parsed);
            }
            // Evalúa la alternativa else if (argument == "--fps") {.
            else if (argument == "--fps") {
                // Evalúa la condición parsed > 120.
                if (parsed > 120) {
                    // Interrumpe la operación con std::invalid_argument("FPS must be between 1 and 120.").
                    throw std::invalid_argument("FPS must be between 1 and 120.");
                }
                // Establece options.target.frames_per_second a partir de static_cast<std::uint32_t>(parsed).
                options.target.frames_per_second = static_cast<std::uint32_t>(parsed);
            }
            // Ejecuta la rama alternativa si la condición anterior fue falsa.
            else {
                // Evalúa la condición parsed < 2 || parsed > 8.
                if (parsed < 2 || parsed > 8) {
                    // Interrumpe la operación con std::invalid_argument("Slot count must be between 2 and 8.").
                    throw std::invalid_argument("Slot count must be between 2 and 8.");
                }
                // Establece options.slots (posiciones del anillo) a partir de static_cast<std::uint32_t>(parsed).
                options.slots = static_cast<std::uint32_t>(parsed);
            }
        }
    }
    // Evalúa la condición options.continuous && options.has_frames.
    if (options.continuous && options.has_frames) {
        // Interrumpe la operación con std::invalid_argument("Use --continuous or --frames, not both.").
        throw std::invalid_argument("Use --continuous or --frames, not both.");
    }
    // Evalúa la condición options.continuous.
    if (options.continuous) {
        // Establece options.frames (fotogramas) a partir de 0.
        options.frames = 0;
    }

    // Evalúa la condición options.synthetic == options.has_index.
    if (options.synthetic == options.has_index) {
        // Interrumpe la operación con std::invalid_argument(.
        throw std::invalid_argument(
            // Aporta el texto "Bridge requires exactly one source: --index <N> or --synthetic." a la expresión.
            "Bridge requires exactly one source: --index <N> or --synthetic."
        );
    }
    // Devuelve options.
    return options;
}

// Define la operación enumerate_devices, que enumera las cámaras disponibles.
std::vector<CameraDevice> enumerate_devices() {
    // Ejecuta el bloque protegido contra excepciones.
    try {
        // Establece enumerator a partir de capture::create_platform_device_enumerator().
        const auto enumerator =
            // Ejecuta capture::create_platform_device_enumerator().
            capture::create_platform_device_enumerator();
        // Devuelve enumerator->enumerate().
        return enumerator->enumerate();
    }
    // Maneja la excepción declarada en catch (const DiagnosticError&) {.
    catch (const DiagnosticError&) {
        // Vuelve a propagar la excepción.
        throw;
    }
    // Maneja la excepción declarada en catch (const std::runtime_error& error) {.
    catch (const std::runtime_error& error) {
        // Interrumpe la operación con DiagnosticError(.
        throw DiagnosticError(
            // Aporta el texto "ORV-CAP-500" a la expresión.
            "ORV-CAP-500",
            // Aporta el texto "camera_enumeration_failed" a la expresión.
            "camera_enumeration_failed",
            // Incorpora "Unable to enumerate video devices: " + en esta operación.
            "Unable to enumerate video devices: " +
                // Ejecuta std::string(error.what()),.
                std::string(error.what()),
            // Incorpora 70 en esta operación.
            70
        );
    }
}

// Define la operación select_device, que elige una cámara por índice.
const CameraDevice& select_device(
    // Declara el parámetro devices (dispositivos detectados).
    const std::vector<CameraDevice>& devices,
    // Incorpora const std::size_t requested_index en esta operación.
    const std::size_t requested_index
) {
    // Evalúa la condición devices.empty().
    if (devices.empty()) {
        // Interrumpe la operación con DiagnosticError(.
        throw DiagnosticError(
            // Aporta el texto "ORV-CAP-100" a la expresión.
            "ORV-CAP-100",
            // Aporta el texto "camera_absent" a la expresión.
            "camera_absent",
            // Aporta el texto "No video capture devices were found." a la expresión.
            "No video capture devices were found.",
            // Incorpora 65 en esta operación.
            65
        );
    }

    // Devuelve capture::CameraSelector::select_by_index(.
    return capture::CameraSelector::select_by_index(
        // Aporta devices como elemento de la lista.
        devices,
        // Incorpora requested_index en esta operación.
        requested_index
    );
}

// Define la operación open_camera, que abre la cámara seleccionada.
std::unique_ptr<Camera> open_camera(
    // Incorpora const CameraDevice& selected en esta operación.
    const CameraDevice& selected
) {
    // Ejecuta el bloque protegido contra excepciones.
    try {
        // Establece camera a partir de capture::create_platform_camera().
        auto camera = capture::create_platform_camera();
        // Ejecuta camera->open(selected).
        camera->open(selected);

        // Evalúa la condición !camera->is_open().
        if (!camera->is_open()) {
            // Interrumpe la operación con DiagnosticError(.
            throw DiagnosticError(
                // Aporta el texto "ORV-CAP-501" a la expresión.
                "ORV-CAP-501",
                // Aporta el texto "camera_open_failed" a la expresión.
                "camera_open_failed",
                // Aporta el texto "The native backend returned without an active camera." a la expresión.
                "The native backend returned without an active camera.",
                // Incorpora 71 en esta operación.
                71
            );
        }

        // Devuelve camera.
        return camera;
    }
    // Maneja la excepción declarada en catch (const DiagnosticError&) {.
    catch (const DiagnosticError&) {
        // Vuelve a propagar la excepción.
        throw;
    }
    // Maneja la excepción declarada en catch (const std::runtime_error& error) {.
    catch (const std::runtime_error& error) {
        // Interrumpe la operación con DiagnosticError(.
        throw DiagnosticError(
            // Aporta el texto "ORV-CAP-501" a la expresión.
            "ORV-CAP-501",
            // Aporta el texto "camera_open_failed" a la expresión.
            "camera_open_failed",
            // Incorpora "Unable to open the selected camera: " + en esta operación.
            "Unable to open the selected camera: " +
                // Ejecuta std::string(error.what()),.
                std::string(error.what()),
            // Incorpora 71 en esta operación.
            71
        );
    }
}

// Establece target a partir de {}) {.
VideoFormat configure_camera(Camera& camera, const capture::VideoFormatTarget& target = {}) {
    // Ejecuta el bloque protegido contra excepciones.
    try {
        // Devuelve camera.configure(target).
        return camera.configure(target);
    }
    // Maneja la excepción declarada en catch (const DiagnosticError&) {.
    catch (const DiagnosticError&) {
        // Vuelve a propagar la excepción.
        throw;
    }
    // Maneja la excepción declarada en catch (const std::runtime_error& error) {.
    catch (const std::runtime_error& error) {
        // Interrumpe la operación con DiagnosticError(.
        throw DiagnosticError(
            // Aporta el texto "ORV-CAP-502" a la expresión.
            "ORV-CAP-502",
            // Aporta el texto "format_negotiation_failed" a la expresión.
            "format_negotiation_failed",
            // Incorpora "Unable to configure a camera video format: " + en esta operación.
            "Unable to configure a camera video format: " +
                // Ejecuta std::string(error.what()),.
                std::string(error.what()),
            // Incorpora 72 en esta operación.
            72
        );
    }
}

// Define la operación capture_frames con los parámetros declarados.
CaptureSummary capture_frames(Camera& camera) {
    // Ejecuta el bloque protegido contra excepciones.
    try {
        // Devuelve camera.capture_frames(kDefaultCaptureFrames).
        return camera.capture_frames(kDefaultCaptureFrames);
    }
    // Maneja la excepción declarada en catch (const DiagnosticError&) {.
    catch (const DiagnosticError&) {
        // Vuelve a propagar la excepción.
        throw;
    }
    // Maneja la excepción declarada en catch (const std::runtime_error& error) {.
    catch (const std::runtime_error& error) {
        // Interrumpe la operación con DiagnosticError(.
        throw DiagnosticError(
            // Aporta el texto "ORV-CAP-504" a la expresión.
            "ORV-CAP-504",
            // Aporta el texto "frame_capture_failed" a la expresión.
            "frame_capture_failed",
            // Ejecuta error.what(),.
            error.what(),
            // Incorpora 75 en esta operación.
            75
        );
    }
}

// Define la operación print_bridge_started con los parámetros declarados.
void print_bridge_started(
    // Declara el parámetro options (opciones solicitadas).
    const BridgeOptions& options,
    // Declara el parámetro format (formato de video).
    const VideoFormat& format,
    // Incorpora const std::string_view source en esta operación.
    const std::string_view source
) {
    // Incorpora std::cout en esta operación.
    std::cout
        // Incorpora << "ORVIX Capture Core " << kVersion << "\n\n" en esta operación.
        << "ORVIX Capture Core " << kVersion << "\n\n"
        // Incorpora << "Shared-memory video bridge\n\n" en esta operación.
        << "Shared-memory video bridge\n\n"
        // Incorpora << "Source: " << source << "\n" en esta operación.
        << "Source: " << source << "\n"
        // Incorpora << "Shared memory: " << options.name << "\n" en esta operación.
        << "Shared memory: " << options.name << "\n"
        // Incorpora << "Ring slots: " << ipc::kDefaultSlotCount << "\n" en esta operación.
        << "Ring slots: " << ipc::kDefaultSlotCount << "\n"
        // Incorpora << "Frames requested: " << options.frames << "\n" en esta operación.
        << "Frames requested: " << options.frames << "\n";
    // Ejecuta print_video_format(format, "Published").
    print_video_format(format, "Published");
    // Incorpora std::cout en esta operación.
    std::cout
        // Incorpora << "\nBridge status: RUNNING\n" en esta operación.
        << "\nBridge status: RUNNING\n"
        // Incorpora << "Open another terminal and run:\n" en esta operación.
        << "Open another terminal and run:\n"
        // Incorpora << " python -m orvix.ui.viewer --name " en esta operación.
        << "  python -m orvix.ui.viewer --name "
        // Incorpora << options.name << "\n\n" en esta operación.
        << options.name << "\n\n";
}

// Define la operación print_bridge_completed con los parámetros declarados.
void print_bridge_completed(
    // Declara el parámetro summary (resumen de la sesión).
    const CaptureSummary& summary,
    // Incorpora const ipc::PublisherStats& statistics en esta operación.
    const ipc::PublisherStats& statistics
) {
    // Incorpora std::cout en esta operación.
    std::cout
        // Incorpora << "\nFrames published: " << statistics.published_sequence << "\n" en esta operación.
        << "\nFrames published: " << statistics.published_sequence << "\n"
        // Incorpora << "Last sequence consumed: " << statistics.consumed_sequence << "\n" en esta operación.
        << "Last sequence consumed: " << statistics.consumed_sequence << "\n"
        // Incorpora << "Frames overwritten before consumption: " en esta operación.
        << "Frames overwritten before consumption: "
        // Incorpora << statistics.overwritten_frames << "\n" en esta operación.
        << statistics.overwritten_frames << "\n"
        // Incorpora << "Final buffer utilization: " en esta operación.
        << "Final buffer utilization: "
        // Ejecuta << std::fixed << std::setprecision(1).
        << std::fixed << std::setprecision(1)
        // Incorpora << statistics.buffer_utilization * 100.0 << "%\n" en esta operación.
        << statistics.buffer_utilization * 100.0 << "%\n"
        // Incorpora << "Measured producer FPS: " en esta operación.
        << "Measured producer FPS: "
        // Ejecuta << std::fixed << std::setprecision(2).
        << std::fixed << std::setprecision(2)
        // Incorpora << statistics.producer_fps << "\n" en esta operación.
        << statistics.producer_fps << "\n"
        // Ejecuta << "Capture FPS: " << summary.capture_fps() << "\n".
        << "Capture FPS: " << summary.capture_fps() << "\n"
        // Incorpora << "Bridge status: COMPLETED\n" en esta operación.
        << "Bridge status: COMPLETED\n";
}

// Define la operación publish_reader con los parámetros declarados.
CaptureSummary publish_reader(
    // Declara el parámetro reader (lector de fotogramas).
    capture::FrameReader& reader,
    // Declara el parámetro options (opciones solicitadas).
    const BridgeOptions& options,
    // Declara el parámetro format (formato de video).
    const VideoFormat& format,
    // Declara el parámetro logger (registrador de diagnóstico).
    Logger& logger,
    // Incorpora const std::string_view source en esta operación.
    const std::string_view source
) {
    // Ejecuta ipc::SharedMemoryPublisher publisher(options.name, format, options.slots).
    ipc::SharedMemoryPublisher publisher(options.name, format, options.slots);
    // Ejecuta print_bridge_started(options, format, source).
    print_bridge_started(options, format, source);
    // Ejecuta logger.info(.
    logger.info(
        // Aporta el texto "ORV-IPC-200" a la expresión.
        "ORV-IPC-200",
        // Aporta el texto "shared_memory_bridge_started" a la expresión.
        "shared_memory_bridge_started",
        // Establece name a partir de " + options.name +.
        "name=" + options.name +
            // Establece source (fuente de video) a partir de " + std::string(source) +.
            " source=" + std::string(source) +
            // Establece requested_frames a partir de " + std::to_string(options.frames).
            " requested_frames=" + std::to_string(options.frames)
    );

    // Ejecuta el bloque protegido contra excepciones.
    try {
        // Establece summary (resumen de la sesión) a partir de capture::ContinuousFrameCapture::run(.
        CaptureSummary summary = capture::ContinuousFrameCapture::run(
            // Aporta reader como elemento de la lista.
            reader,
            // Aporta options.frames como elemento de la lista.
            options.frames,
            // Aporta format como elemento de la lista.
            format,
            // Aporta 1000 como elemento de la lista.
            1000,
            // Incorpora &publisher en esta operación.
            &publisher
        );
        // Ejecuta publisher.mark_completed().
        publisher.mark_completed();
        // Establece statistics a partir de publisher.stats().
        const auto statistics = publisher.stats();
        // Ejecuta print_bridge_completed(summary, statistics).
        print_bridge_completed(summary, statistics);
        // Ejecuta logger.info(.
        logger.info(
            // Aporta el texto "ORV-IPC-201" a la expresión.
            "ORV-IPC-201",
            // Aporta el texto "shared_memory_bridge_completed" a la expresión.
            "shared_memory_bridge_completed",
            // Establece published_sequence (última secuencia publicada) a partir de " +.
            "published_sequence=" +
                // Ejecuta std::to_string(statistics.published_sequence) +.
                std::to_string(statistics.published_sequence) +
                // Establece consumed_sequence a partir de " +.
                " consumed_sequence=" +
                // Ejecuta std::to_string(statistics.consumed_sequence) +.
                std::to_string(statistics.consumed_sequence) +
                // Establece overwritten_frames a partir de " +.
                " overwritten_frames=" +
                // Ejecuta std::to_string(statistics.overwritten_frames).
                std::to_string(statistics.overwritten_frames)
        );
        // Devuelve summary.
        return summary;
    }
    // Maneja la excepción declarada en catch (...) {.
    catch (...) {
        // Ejecuta publisher.mark_failed().
        publisher.mark_failed();
        // Ejecuta logger.error(.
        logger.error(
            // Aporta el texto "ORV-IPC-500" a la expresión.
            "ORV-IPC-500",
            // Aporta el texto "shared_memory_bridge_failed" a la expresión.
            "shared_memory_bridge_failed",
            // Aporta el texto "The producer stopped after a bridge failure." a la expresión.
            "The producer stopped after a bridge failure."
        );
        // Vuelve a propagar la excepción.
        throw;
    }
}

// Define la operación run_bridge, que publica fotogramas desde la cámara o la fuente sintética.
int run_bridge(Logger& logger, const BridgeOptions& options) {
    // Evalúa la condición options.synthetic.
    if (options.synthetic) {
        // Define la operación reader con los parámetros declarados.
        capture::SyntheticFrameReader reader(
            // Incorpora options.target.width, options.target.height, options.target.frames_per_second en esta operación.
            options.target.width, options.target.height, options.target.frames_per_second
        );
        // Ejecuta static_cast<void>(publish_reader(.
        static_cast<void>(publish_reader(
            // Aporta reader como elemento de la lista.
            reader,
            // Aporta options como elemento de la lista.
            options,
            // Ejecuta reader.format(),.
            reader.format(),
            // Aporta logger como elemento de la lista.
            logger,
            // Aporta el texto "synthetic NV12" a la expresión.
            "synthetic NV12"
        ));
        // Devuelve 0.
        return 0;
    }

    // Establece devices (dispositivos detectados) a partir de enumerate_devices().
    const auto devices = enumerate_devices();
    // Establece selected a partir de select_device(devices, options.index).
    const auto& selected = select_device(devices, options.index);
    // Establece camera a partir de open_camera(selected).
    auto camera = open_camera(selected);
    // Ejecuta log_camera_opened(logger, selected).
    log_camera_opened(logger, selected);
    // Establece configured a partir de configure_camera(*camera, options.target).
    const auto configured = configure_camera(*camera, options.target);
    // Ejecuta ipc::SharedMemoryPublisher publisher(options.name, configured, options.slots).
    ipc::SharedMemoryPublisher publisher(options.name, configured, options.slots);
    // Ejecuta print_bridge_started(.
    print_bridge_started(
        // Aporta options como elemento de la lista.
        options,
        // Aporta configured como elemento de la lista.
        configured,
        // Incorpora selected.friendly_name en esta operación.
        selected.friendly_name
    );
    // Ejecuta logger.info(.
    logger.info(
        // Aporta el texto "ORV-IPC-200" a la expresión.
        "ORV-IPC-200",
        // Aporta el texto "shared_memory_bridge_started" a la expresión.
        "shared_memory_bridge_started",
        // Establece name a partir de " + options.name +.
        "name=" + options.name +
            // Establece source (fuente de video) a partir de " + selected.friendly_name +.
            " source=" + selected.friendly_name +
            // Establece requested_frames a partir de " + std::to_string(options.frames).
            " requested_frames=" + std::to_string(options.frames)
    );

    // Ejecuta el bloque protegido contra excepciones.
    try {
        // Establece summary (resumen de la sesión) a partir de camera->capture_frames(.
        const CaptureSummary summary = camera->capture_frames(
            // Aporta options.frames como elemento de la lista.
            options.frames,
            // Incorpora &publisher en esta operación.
            &publisher
        );
        // Ejecuta publisher.mark_completed().
        publisher.mark_completed();
        // Establece statistics a partir de publisher.stats().
        const auto statistics = publisher.stats();
        // Ejecuta print_bridge_completed(summary, statistics).
        print_bridge_completed(summary, statistics);
        // Ejecuta logger.info(.
        logger.info(
            // Aporta el texto "ORV-IPC-201" a la expresión.
            "ORV-IPC-201",
            // Aporta el texto "shared_memory_bridge_completed" a la expresión.
            "shared_memory_bridge_completed",
            // Establece published_sequence (última secuencia publicada) a partir de " +.
            "published_sequence=" +
                // Ejecuta std::to_string(statistics.published_sequence) +.
                std::to_string(statistics.published_sequence) +
                // Establece consumed_sequence a partir de " +.
                " consumed_sequence=" +
                // Ejecuta std::to_string(statistics.consumed_sequence) +.
                std::to_string(statistics.consumed_sequence) +
                // Establece overwritten_frames a partir de " +.
                " overwritten_frames=" +
                // Ejecuta std::to_string(statistics.overwritten_frames).
                std::to_string(statistics.overwritten_frames)
        );
    }
    // Maneja la excepción declarada en catch (...) {.
    catch (...) {
        // Ejecuta publisher.mark_failed().
        publisher.mark_failed();
        // Vuelve a propagar la excepción.
        throw;
    }

    // Cierra el recurso mediante camera->close.
    camera->close();
    // Devuelve 0.
    return 0;
}

// Define la operación format_description con los parámetros declarados.
std::string format_description(const VideoFormat& format) {
    // Declara message con el tipo indicado.
    std::ostringstream message;
    // Incorpora message en esta operación.
    message
        // Establece resolution a partir de " << format.width << "x" << format.height.
        << "resolution=" << format.width << "x" << format.height
        // Establece pixel_format (formato de píxeles) a partir de " << format.pixel_format.
        << " pixel_format=" << format.pixel_format
        // Establece fps a partir de " << std::fixed << std::setprecision(2).
        << " fps=" << std::fixed << std::setprecision(2)
        // Ejecuta << format.frames_per_second().
        << format.frames_per_second()
        // Establece native_type_index a partir de " << format.native_type_index.
        << " native_type_index=" << format.native_type_index;
    // Devuelve message.str().
    return message.str();
}

// Define la operación log_camera_opened con los parámetros declarados.
void log_camera_opened(Logger& logger, const CameraDevice& selected) {
    // Ejecuta logger.info(.
    logger.info(
        // Aporta el texto "ORV-CAP-200" a la expresión.
        "ORV-CAP-200",
        // Aporta el texto "camera_opened" a la expresión.
        "camera_opened",
        // Establece index a partir de " + std::to_string(selected.index) +.
        "index=" + std::to_string(selected.index) +
            // Establece name a partir de " + selected.friendly_name +.
            " name=" + selected.friendly_name +
            // Establece backend (implementación nativa) a partir de " + selected.backend.
            " backend=" + selected.backend
    );
}

// Define la operación print_video_format con los parámetros declarados.
void print_video_format(
    // Declara el parámetro format (formato de video).
    const VideoFormat& format,
    // Incorpora const std::string_view prefix en esta operación.
    const std::string_view prefix
) {
    // Incorpora std::cout en esta operación.
    std::cout
        // Incorpora << prefix << " resolution: " en esta operación.
        << prefix << " resolution: "
        // Incorpora << format.width << "x" << format.height << "\n" en esta operación.
        << format.width << "x" << format.height << "\n"
        // Incorpora << prefix << " pixel format: " << format.pixel_format << "\n" en esta operación.
        << prefix << " pixel format: " << format.pixel_format << "\n"
        // Incorpora << prefix << " FPS: " en esta operación.
        << prefix << " FPS: "
        // Ejecuta << std::fixed << std::setprecision(2).
        << std::fixed << std::setprecision(2)
        // Ejecuta << format.frames_per_second() << "\n".
        << format.frames_per_second() << "\n"
        // Incorpora << prefix << " native type index: " en esta operación.
        << prefix << " native type index: "
        // Incorpora << format.native_type_index << "\n" en esta operación.
        << format.native_type_index << "\n";
}

// Define la operación run_devices con los parámetros declarados.
int run_devices(Logger& logger) {
    // Incorpora std::cout en esta operación.
    std::cout
        // Incorpora << "ORVIX Capture Core " << kVersion << "\n\n" en esta operación.
        << "ORVIX Capture Core " << kVersion << "\n\n"
        // Ejecuta << "Backend: " << capture::platform_backend_name() << "\n\n".
        << "Backend: " << capture::platform_backend_name() << "\n\n";

    // Establece devices (dispositivos detectados) a partir de enumerate_devices().
    const auto devices = enumerate_devices();

    // Evalúa la condición devices.empty().
    if (devices.empty()) {
        // Ejecuta logger.warning(.
        logger.warning(
            // Aporta el texto "ORV-CAP-100" a la expresión.
            "ORV-CAP-100",
            // Aporta el texto "camera_absent" a la expresión.
            "camera_absent",
            // Aporta el texto "Device enumeration returned zero video capture devices." a la expresión.
            "Device enumeration returned zero video capture devices."
        );

        // Incorpora std::cout en esta operación.
        std::cout
            // Incorpora << "[ORV-CAP-100] No video capture devices were found.\n\n" en esta operación.
            << "[ORV-CAP-100] No video capture devices were found.\n\n"
            // Incorpora << "Device count: 0\n" en esta operación.
            << "Device count: 0\n"
            // Ejecuta << "Log file: " << logger.path().string() << "\n".
            << "Log file: " << logger.path().string() << "\n";

        // Devuelve 0.
        return 0;
    }

    // Ejecuta logger.info(.
    logger.info(
        // Aporta el texto "ORV-CAP-101" a la expresión.
        "ORV-CAP-101",
        // Aporta el texto "camera_enumeration_completed" a la expresión.
        "camera_enumeration_completed",
        // Establece device_count a partir de " + std::to_string(devices.size()).
        "device_count=" + std::to_string(devices.size())
    );

    // Incorpora std::cout << "Available video capture devices:\n\n" en esta operación.
    std::cout << "Available video capture devices:\n\n";

    // Recorre según const auto& device : devices.
    for (const auto& device : devices) {
        // Incorpora std::cout en esta operación.
        std::cout
            // Incorpora << "[" << device.index << "] " en esta operación.
            << "[" << device.index << "] "
            // Incorpora << device.friendly_name << "\n" en esta operación.
            << device.friendly_name << "\n"
            // Incorpora << " Backend: " << device.backend << "\n" en esta operación.
            << "    Backend: " << device.backend << "\n"
            // Incorpora << " Symbolic link: " << device.symbolic_link << "\n\n" en esta operación.
            << "    Symbolic link: " << device.symbolic_link << "\n\n";
    }

    // Incorpora std::cout en esta operación.
    std::cout
        // Ejecuta << "Device count: " << devices.size() << "\n".
        << "Device count: " << devices.size() << "\n"
        // Ejecuta << "Log file: " << logger.path().string() << "\n".
        << "Log file: " << logger.path().string() << "\n";

    // Devuelve 0.
    return 0;
}

// Define la operación run_select con los parámetros declarados.
int run_select(Logger& logger, const std::string_view index_text) {
    // Establece requested_index a partir de parse_index(index_text).
    const std::size_t requested_index = parse_index(index_text);
    // Establece devices (dispositivos detectados) a partir de enumerate_devices().
    const auto devices = enumerate_devices();
    // Establece selected a partir de select_device(devices, requested_index).
    const auto& selected = select_device(devices, requested_index);

    // Ejecuta logger.info(.
    logger.info(
        // Aporta el texto "ORV-CAP-201" a la expresión.
        "ORV-CAP-201",
        // Aporta el texto "camera_selected" a la expresión.
        "camera_selected",
        // Establece index a partir de " + std::to_string(selected.index) +.
        "index=" + std::to_string(selected.index) +
            // Establece name a partir de " + selected.friendly_name.
            " name=" + selected.friendly_name
    );

    // Incorpora std::cout en esta operación.
    std::cout
        // Incorpora << "ORVIX Capture Core " << kVersion << "\n\n" en esta operación.
        << "ORVIX Capture Core " << kVersion << "\n\n"
        // Incorpora << "Selected video capture device:\n\n" en esta operación.
        << "Selected video capture device:\n\n"
        // Incorpora << "Index: " << selected.index << "\n" en esta operación.
        << "Index: " << selected.index << "\n"
        // Incorpora << "Name: " << selected.friendly_name << "\n" en esta operación.
        << "Name: " << selected.friendly_name << "\n"
        // Incorpora << "Backend: " << selected.backend << "\n" en esta operación.
        << "Backend: " << selected.backend << "\n"
        // Incorpora << "Symbolic link: " << selected.symbolic_link << "\n\n" en esta operación.
        << "Symbolic link: " << selected.symbolic_link << "\n\n"
        // Incorpora << "Selection status: READY_FOR_OPEN\n" en esta operación.
        << "Selection status: READY_FOR_OPEN\n"
        // Ejecuta << "Log file: " << logger.path().string() << "\n".
        << "Log file: " << logger.path().string() << "\n";

    // Devuelve 0.
    return 0;
}

// Define la operación run_open con los parámetros declarados.
int run_open(Logger& logger, const std::string_view index_text) {
    // Establece requested_index a partir de parse_index(index_text).
    const std::size_t requested_index = parse_index(index_text);
    // Establece devices (dispositivos detectados) a partir de enumerate_devices().
    const auto devices = enumerate_devices();
    // Establece selected a partir de select_device(devices, requested_index).
    const auto& selected = select_device(devices, requested_index);
    // Establece camera a partir de open_camera(selected).
    auto camera = open_camera(selected);

    // Ejecuta log_camera_opened(logger, selected).
    log_camera_opened(logger, selected);

    // Incorpora std::cout en esta operación.
    std::cout
        // Incorpora << "ORVIX Capture Core " << kVersion << "\n\n" en esta operación.
        << "ORVIX Capture Core " << kVersion << "\n\n"
        // Incorpora << "Opened video capture device:\n\n" en esta operación.
        << "Opened video capture device:\n\n"
        // Incorpora << "Index: " << selected.index << "\n" en esta operación.
        << "Index: " << selected.index << "\n"
        // Incorpora << "Name: " << selected.friendly_name << "\n" en esta operación.
        << "Name: " << selected.friendly_name << "\n"
        // Incorpora << "Backend: " << selected.backend << "\n\n" en esta operación.
        << "Backend: " << selected.backend << "\n\n"
        // Incorpora << "Open status: OPEN\n" en esta operación.
        << "Open status: OPEN\n"
        // Incorpora << "Media source: ACTIVE\n" en esta operación.
        << "Media source: ACTIVE\n"
        // Ejecuta << "Log file: " << logger.path().string() << "\n".
        << "Log file: " << logger.path().string() << "\n";

    // Cierra el recurso mediante camera->close.
    camera->close();
    // Ejecuta logger.info(.
    logger.info(
        // Aporta el texto "ORV-CAP-202" a la expresión.
        "ORV-CAP-202",
        // Aporta el texto "camera_closed" a la expresión.
        "camera_closed",
        // Aporta el texto "Camera resources released after open validation." a la expresión.
        "Camera resources released after open validation."
    );

    // Devuelve 0.
    return 0;
}

// Define la operación run_formats con los parámetros declarados.
int run_formats(Logger& logger, const std::string_view index_text) {
    // Establece requested_index a partir de parse_index(index_text).
    const std::size_t requested_index = parse_index(index_text);
    // Establece devices (dispositivos detectados) a partir de enumerate_devices().
    const auto devices = enumerate_devices();
    // Establece selected a partir de select_device(devices, requested_index).
    const auto& selected = select_device(devices, requested_index);
    // Establece camera a partir de open_camera(selected).
    auto camera = open_camera(selected);

    // Ejecuta log_camera_opened(logger, selected).
    log_camera_opened(logger, selected);

    // Establece formats a partir de camera->available_formats().
    const auto formats = camera->available_formats();
    // Establece configured a partir de configure_camera(*camera).
    const auto configured = configure_camera(*camera);

    // Ejecuta logger.info(.
    logger.info(
        // Aporta el texto "ORV-CAP-210" a la expresión.
        "ORV-CAP-210",
        // Aporta el texto "camera_formats_discovered" a la expresión.
        "camera_formats_discovered",
        // Establece usable_native_formats a partir de " + std::to_string(formats.size()).
        "usable_native_formats=" + std::to_string(formats.size())
    );
    // Ejecuta logger.info(.
    logger.info(
        // Aporta el texto "ORV-CAP-211" a la expresión.
        "ORV-CAP-211",
        // Aporta el texto "camera_format_configured" a la expresión.
        "camera_format_configured",
        // Ejecuta format_description(configured).
        format_description(configured)
    );

    // Incorpora std::cout en esta operación.
    std::cout
        // Incorpora << "ORVIX Capture Core " << kVersion << "\n\n" en esta operación.
        << "ORVIX Capture Core " << kVersion << "\n\n"
        // Incorpora << "Video formats for: " << selected.friendly_name << "\n\n" en esta operación.
        << "Video formats for: " << selected.friendly_name << "\n\n"
        // Ejecuta << "Usable native formats: " << formats.size() << "\n".
        << "Usable native formats: " << formats.size() << "\n";

    // Recorre según const auto& format : formats.
    for (const auto& format : formats) {
        // Incorpora std::cout en esta operación.
        std::cout
            // Incorpora << "[" << format.native_type_index << "] " en esta operación.
            << "[" << format.native_type_index << "] "
            // Incorpora << format.width << "x" << format.height << " @ " en esta operación.
            << format.width << "x" << format.height << " @ "
            // Ejecuta << std::fixed << std::setprecision(2).
            << std::fixed << std::setprecision(2)
            // Ejecuta << format.frames_per_second() << " FPS, ".
            << format.frames_per_second() << " FPS, "
            // Incorpora << format.pixel_format en esta operación.
            << format.pixel_format;

        // Evalúa la condición format.compressed.
        if (format.compressed) {
            // Incorpora std::cout << " (compressed)" en esta operación.
            std::cout << " (compressed)";
        }

        // Incorpora std::cout << "\n" en esta operación.
        std::cout << "\n";
    }

    // Incorpora std::cout en esta operación.
    std::cout
        // Incorpora << "\nTarget resolution: 1280x720\n" en esta operación.
        << "\nTarget resolution: 1280x720\n"
        // Incorpora << "Target FPS: 30\n\n" en esta operación.
        << "Target FPS: 30\n\n";
    // Ejecuta print_video_format(configured, "Negotiated").
    print_video_format(configured, "Negotiated");
    // Incorpora std::cout en esta operación.
    std::cout
        // Incorpora << "\nFormat status: CONFIGURED\n" en esta operación.
        << "\nFormat status: CONFIGURED\n"
        // Ejecuta << "Log file: " << logger.path().string() << "\n".
        << "Log file: " << logger.path().string() << "\n";

    // Cierra el recurso mediante camera->close.
    camera->close();
    // Ejecuta logger.info(.
    logger.info(
        // Aporta el texto "ORV-CAP-202" a la expresión.
        "ORV-CAP-202",
        // Aporta el texto "camera_closed" a la expresión.
        "camera_closed",
        // Aporta el texto "Camera resources released after format validation." a la expresión.
        "Camera resources released after format validation."
    );

    // Devuelve 0.
    return 0;
}

// Define la operación run_capture con los parámetros declarados.
int run_capture(Logger& logger, const std::string_view index_text) {
    // Establece requested_index a partir de parse_index(index_text).
    const std::size_t requested_index = parse_index(index_text);
    // Establece devices (dispositivos detectados) a partir de enumerate_devices().
    const auto devices = enumerate_devices();
    // Establece selected a partir de select_device(devices, requested_index).
    const auto& selected = select_device(devices, requested_index);
    // Establece camera a partir de open_camera(selected).
    auto camera = open_camera(selected);

    // Ejecuta log_camera_opened(logger, selected).
    log_camera_opened(logger, selected);

    // Establece configured a partir de configure_camera(*camera).
    const auto configured = configure_camera(*camera);
    // Ejecuta logger.info(.
    logger.info(
        // Aporta el texto "ORV-CAP-211" a la expresión.
        "ORV-CAP-211",
        // Aporta el texto "camera_format_configured" a la expresión.
        "camera_format_configured",
        // Ejecuta format_description(configured).
        format_description(configured)
    );
    // Ejecuta logger.info(.
    logger.info(
        // Aporta el texto "ORV-CAP-220" a la expresión.
        "ORV-CAP-220",
        // Aporta el texto "capture_started" a la expresión.
        "capture_started",
        // Establece requested_frames a partir de " + std::to_string(kDefaultCaptureFrames).
        "requested_frames=" + std::to_string(kDefaultCaptureFrames)
    );

    // Incorpora std::cout en esta operación.
    std::cout
        // Incorpora << "ORVIX Capture Core " << kVersion << "\n\n" en esta operación.
        << "ORVIX Capture Core " << kVersion << "\n\n"
        // Incorpora << "Continuous native frame capture\n\n" en esta operación.
        << "Continuous native frame capture\n\n"
        // Incorpora << "Device: " << selected.friendly_name << "\n" en esta operación.
        << "Device: " << selected.friendly_name << "\n"
        // Incorpora << "Backend: " << selected.backend << "\n" en esta operación.
        << "Backend: " << selected.backend << "\n"
        // Incorpora << "Target resolution: 1280x720\n" en esta operación.
        << "Target resolution: 1280x720\n"
        // Incorpora << "Target FPS: 30\n" en esta operación.
        << "Target FPS: 30\n";

    // Ejecuta print_video_format(configured, "Negotiated").
    print_video_format(configured, "Negotiated");

    // Incorpora std::cout en esta operación.
    std::cout
        // Incorpora << "\nCapturing " << kDefaultCaptureFrames en esta operación.
        << "\nCapturing " << kDefaultCaptureFrames
        // Incorpora << " consecutive frames...\n" en esta operación.
        << " consecutive frames...\n";

    // Establece summary (resumen de la sesión) a partir de capture_frames(*camera).
    const CaptureSummary summary = capture_frames(*camera);

    // Incorpora std::cout << "\nPer-frame metadata:\n" en esta operación.
    std::cout << "\nPer-frame metadata:\n";

    // Recorre según const auto& frame : summary.frames.
    for (const auto& frame : summary.frames) {
        // Incorpora std::cout en esta operación.
        std::cout
            // Incorpora << "Frame #" << frame.sequence en esta operación.
            << "Frame #" << frame.sequence
            // Establece timestamp (marca de tiempo) a partir de " << frame.timestamp_100ns.
            << " | timestamp=" << frame.timestamp_100ns
            // Incorpora << " (100 ns)" en esta operación.
            << " (100 ns)"
            // Establece resolution a partir de " << frame.width << "x" << frame.height.
            << " | resolution=" << frame.width << "x" << frame.height
            // Establece format (formato de video) a partir de " << frame.pixel_format.
            << " | format=" << frame.pixel_format
            // Establece bytes (bytes de la muestra) a partir de " << frame.byte_count.
            << " | bytes=" << frame.byte_count
            // Incorpora << "\n" en esta operación.
            << "\n";
    }

    // Incorpora std::cout en esta operación.
    std::cout
        // Incorpora << "\nFrames requested: " << summary.requested_frames << "\n" en esta operación.
        << "\nFrames requested: " << summary.requested_frames << "\n"
        // Incorpora << "Frames captured: " << summary.captured_frames << "\n" en esta operación.
        << "Frames captured: " << summary.captured_frames << "\n"
        // Incorpora << "First sequence: " << summary.first_sequence << "\n" en esta operación.
        << "First sequence: " << summary.first_sequence << "\n"
        // Incorpora << "Last sequence: " << summary.last_sequence << "\n" en esta operación.
        << "Last sequence: " << summary.last_sequence << "\n"
        // Ejecuta << "First timestamp (100 ns): ".
        << "First timestamp (100 ns): "
        // Incorpora << summary.first_timestamp_100ns << "\n" en esta operación.
        << summary.first_timestamp_100ns << "\n"
        // Ejecuta << "Last timestamp (100 ns): ".
        << "Last timestamp (100 ns): "
        // Incorpora << summary.last_timestamp_100ns << "\n" en esta operación.
        << summary.last_timestamp_100ns << "\n"
        // Incorpora << "Captured bytes: " << summary.total_bytes << "\n" en esta operación.
        << "Captured bytes: " << summary.total_bytes << "\n"
        // Incorpora << "Empty reads: " << summary.empty_reads << "\n" en esta operación.
        << "Empty reads: " << summary.empty_reads << "\n"
        // Incorpora << "Sequence status: STRICTLY_INCREASING\n" en esta operación.
        << "Sequence status: STRICTLY_INCREASING\n"
        // Incorpora << "Timestamp status: " en esta operación.
        << "Timestamp status: "
        // Incorpora << ( en esta operación.
        << (
            // Incorpora summary.timestamps_strictly_increasing en esta operación.
            summary.timestamps_strictly_increasing
                // Incorpora ? "STRICTLY_INCREASING" en esta operación.
                ? "STRICTLY_INCREASING"
                // Inicializa el miembro indicado por "NON_MONOTONIC" antes del cuerpo.
                : "NON_MONOTONIC"
        )
        // Incorpora << "\n" en esta operación.
        << "\n"
        // Incorpora << "Capture duration: " en esta operación.
        << "Capture duration: "
        // Ejecuta << std::fixed << std::setprecision(3).
        << std::fixed << std::setprecision(3)
        // Ejecuta << summary.duration_seconds() << " s\n".
        << summary.duration_seconds() << " s\n"
        // Incorpora << "Measured capture FPS: " en esta operación.
        << "Measured capture FPS: "
        // Ejecuta << std::fixed << std::setprecision(2).
        << std::fixed << std::setprecision(2)
        // Ejecuta << summary.capture_fps() << "\n".
        << summary.capture_fps() << "\n"
        // Incorpora << "Capture status: CAPTURE_COMPLETED\n" en esta operación.
        << "Capture status: CAPTURE_COMPLETED\n"
        // Ejecuta << "Log file: " << logger.path().string() << "\n".
        << "Log file: " << logger.path().string() << "\n";

    // Declara completion_message con el tipo indicado.
    std::ostringstream completion_message;
    // Incorpora completion_message en esta operación.
    completion_message
        // Establece captured_frames a partir de " << summary.captured_frames.
        << "captured_frames=" << summary.captured_frames
        // Establece first_sequence a partir de " << summary.first_sequence.
        << " first_sequence=" << summary.first_sequence
        // Establece last_sequence (última secuencia observada) a partir de " << summary.last_sequence.
        << " last_sequence=" << summary.last_sequence
        // Establece total_bytes a partir de " << summary.total_bytes.
        << " total_bytes=" << summary.total_bytes
        // Establece duration_seconds a partir de " << std::fixed << std::setprecision(3).
        << " duration_seconds=" << std::fixed << std::setprecision(3)
        // Ejecuta << summary.duration_seconds().
        << summary.duration_seconds()
        // Establece measured_fps a partir de " << std::setprecision(2).
        << " measured_fps=" << std::setprecision(2)
        // Ejecuta << summary.capture_fps().
        << summary.capture_fps();
    // Ejecuta logger.info(.
    logger.info(
        // Aporta el texto "ORV-CAP-221" a la expresión.
        "ORV-CAP-221",
        // Aporta el texto "capture_completed" a la expresión.
        "capture_completed",
        // Ejecuta completion_message.str().
        completion_message.str()
    );

    // Cierra el recurso mediante camera->close.
    camera->close();
    // Ejecuta logger.info(.
    logger.info(
        // Aporta el texto "ORV-CAP-202" a la expresión.
        "ORV-CAP-202",
        // Aporta el texto "camera_closed" a la expresión.
        "camera_closed",
        // Aporta el texto "Camera resources released after capture." a la expresión.
        "Camera resources released after capture."
    );

    // Devuelve 0.
    return 0;
}

// Define la operación report_error con los parámetros declarados.
int report_error(
    // Declara el parámetro logger (registrador de diagnóstico).
    Logger& logger,
    // Declara el parámetro code.
    const std::string_view code,
    // Declara el parámetro event.
    const std::string_view event,
    // Declara el parámetro message.
    const std::string_view message,
    // Incorpora const int exit_code en esta operación.
    const int exit_code
// Incorpora ) noexcept { en esta operación.
) noexcept {
    // Ejecuta el bloque protegido contra excepciones.
    try {
        // Ejecuta logger.error(code, event, message).
        logger.error(code, event, message);
    }
    // Maneja la excepción declarada en catch (const std::exception& logging_error) {.
    catch (const std::exception& logging_error) {
        // Incorpora std::cerr en esta operación.
        std::cerr
            // Incorpora << "[ORV-LOG-501] Unable to persist diagnostic: " en esta operación.
            << "[ORV-LOG-501] Unable to persist diagnostic: "
            // Ejecuta << logging_error.what() << "\n".
            << logging_error.what() << "\n";
    }

    // Incorpora std::cerr << "[" << code << "] " << message << "\n" en esta operación.
    std::cerr << "[" << code << "] " << message << "\n";
    // Devuelve exit_code.
    return exit_code;
}

// Define la operación dispatch con los parámetros declarados.
int dispatch(Logger& logger, const int argc, char* argv[]) {
    // Evalúa la condición argc >= 3 && std::string_view(argv[1]) == "bridge".
    if (argc >= 3 && std::string_view(argv[1]) == "bridge") {
        // Devuelve run_bridge(logger, parse_bridge_options(argc, argv)).
        return run_bridge(logger, parse_bridge_options(argc, argv));
    }

    // Evalúa la condición argc == 2.
    if (argc == 2) {
        // Establece command a partir de argv[1].
        const std::string command = argv[1];

        // Evalúa la condición command == "devices".
        if (command == "devices") {
            // Devuelve run_devices(logger).
            return run_devices(logger);
        }

        // Evalúa la condición command == "--help" || command == "-h".
        if (command == "--help" || command == "-h") {
            // Ejecuta print_usage().
            print_usage();
            // Devuelve 0.
            return 0;
        }
    }

    // Evalúa la condición argc == 4 &&.
    if (
        // Combina esta condición con argc == 4 &&.
        argc == 4 &&
        (
            // Combina esta condición con std::string_view(argv[1]) == "select" ||.
            std::string_view(argv[1]) == "select" ||
            // Combina esta condición con std::string_view(argv[1]) == "open" ||.
            std::string_view(argv[1]) == "open" ||
            // Combina esta condición con std::string_view(argv[1]) == "formats" ||.
            std::string_view(argv[1]) == "formats" ||
            // Ejecuta std::string_view(argv[1]) == "capture".
            std::string_view(argv[1]) == "capture"
        // Combina esta condición con ) &&.
        ) &&
        // Ejecuta std::string_view(argv[2]) == "--index".
        std::string_view(argv[2]) == "--index"
    ) {
        // Evalúa la condición std::string_view(argv[1]) == "select".
        if (std::string_view(argv[1]) == "select") {
            // Devuelve run_select(logger, argv[3]).
            return run_select(logger, argv[3]);
        }

        // Evalúa la condición std::string_view(argv[1]) == "open".
        if (std::string_view(argv[1]) == "open") {
            // Devuelve run_open(logger, argv[3]).
            return run_open(logger, argv[3]);
        }

        // Evalúa la condición std::string_view(argv[1]) == "formats".
        if (std::string_view(argv[1]) == "formats") {
            // Devuelve run_formats(logger, argv[3]).
            return run_formats(logger, argv[3]);
        }

        // Devuelve run_capture(logger, argv[3]).
        return run_capture(logger, argv[3]);
    }

    // Ejecuta print_usage().
    print_usage();
    // Devuelve report_error(.
    return report_error(
        // Aporta logger como elemento de la lista.
        logger,
        // Aporta el texto "ORV-CAP-400" a la expresión.
        "ORV-CAP-400",
        // Aporta el texto "invalid_command" a la expresión.
        "invalid_command",
        // Aporta el texto "Invalid command or arguments." a la expresión.
        "Invalid command or arguments.",
        // Incorpora 64 en esta operación.
        64
    );
}

// Incorpora } // namespace en esta operación.
}  // namespace

// Define la operación CaptureCli::run con los parámetros declarados.
int CaptureCli::run(const int argc, char* argv[]) const {
    // Declara logger (registrador de diagnóstico) con el tipo indicado.
    std::unique_ptr<Logger> logger;

    // Ejecuta el bloque protegido contra excepciones.
    try {
        // Establece logger (registrador de diagnóstico) a partir de std::make_unique<Logger>().
        logger = std::make_unique<Logger>();
    }
    // Maneja la excepción declarada en catch (const std::exception& error) {.
    catch (const std::exception& error) {
        // Incorpora std::cerr en esta operación.
        std::cerr
            // Incorpora << "[ORV-LOG-500] Unable to initialize capture logging: " en esta operación.
            << "[ORV-LOG-500] Unable to initialize capture logging: "
            // Ejecuta << error.what() << "\n".
            << error.what() << "\n";
        // Devuelve 78.
        return 78;
    }

    // Ejecuta el bloque protegido contra excepciones.
    try {
        // Devuelve dispatch(*logger, argc, argv).
        return dispatch(*logger, argc, argv);
    }
    // Maneja la excepción declarada en catch (const DiagnosticError& error) {.
    catch (const DiagnosticError& error) {
        // Devuelve report_error(.
        return report_error(
            // Aporta *logger como elemento de la lista.
            *logger,
            // Ejecuta error.code(),.
            error.code(),
            // Ejecuta error.event(),.
            error.event(),
            // Ejecuta error.what(),.
            error.what(),
            // Ejecuta error.exit_code().
            error.exit_code()
        );
    }
    // Maneja la excepción declarada en catch (const std::out_of_range& error) {.
    catch (const std::out_of_range& error) {
        // Devuelve report_error(.
        return report_error(
            // Aporta *logger como elemento de la lista.
            *logger,
            // Aporta el texto "ORV-CAP-404" a la expresión.
            "ORV-CAP-404",
            // Aporta el texto "invalid_camera_index" a la expresión.
            "invalid_camera_index",
            // Ejecuta "Camera selection failed: " + std::string(error.what()),.
            "Camera selection failed: " + std::string(error.what()),
            // Incorpora 66 en esta operación.
            66
        );
    }
    // Maneja la excepción declarada en catch (const std::invalid_argument& error) {.
    catch (const std::invalid_argument& error) {
        // Devuelve report_error(.
        return report_error(
            // Aporta *logger como elemento de la lista.
            *logger,
            // Aporta el texto "ORV-CAP-400" a la expresión.
            "ORV-CAP-400",
            // Aporta el texto "invalid_argument" a la expresión.
            "invalid_argument",
            // Ejecuta "Invalid argument: " + std::string(error.what()),.
            "Invalid argument: " + std::string(error.what()),
            // Incorpora 64 en esta operación.
            64
        );
    }
    // Maneja la excepción declarada en catch (const std::exception& error) {.
    catch (const std::exception& error) {
        // Devuelve report_error(.
        return report_error(
            // Aporta *logger como elemento de la lista.
            *logger,
            // Aporta el texto "ORV-CAP-599" a la expresión.
            "ORV-CAP-599",
            // Aporta el texto "unexpected_failure" a la expresión.
            "unexpected_failure",
            // Ejecuta "Unexpected failure: " + std::string(error.what()),.
            "Unexpected failure: " + std::string(error.what()),
            // Incorpora 70 en esta operación.
            70
        );
    }
}

// Incorpora } // namespace orvix::cli en esta operación.
}  // namespace orvix::cli
