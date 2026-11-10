// Incluye las declaraciones de video format selector de ORVIX.
#include "orvix/capture/video_format_selector.hpp"

// Incluye algorithm para usar algoritmos sobre rangos.
#include <algorithm>
// Incluye cmath para usar funciones matemáticas.
#include <cmath>
// Incluye cstdint para usar enteros de tamaño fijo.
#include <cstdint>
// Incluye stdexcept para usar tipos estándar de excepciones.
#include <stdexcept>
// Incluye string_view para usar vistas de cadenas.
#include <string_view>
// Importa las declaraciones de tuple para esta unidad.
#include <tuple>

// Agrupa las declaraciones bajo orvix::capture.
namespace orvix::capture {

// Limita estas declaraciones a esta unidad de compilación.
namespace {

// Define la operación absolute_difference con los parámetros declarados.
std::uint32_t absolute_difference(
    // Declara el parámetro left.
    const std::uint32_t left,
    // Incorpora const std::uint32_t right en esta operación.
    const std::uint32_t right
// Incorpora ) noexcept { en esta operación.
) noexcept {
    // Devuelve left > right ? left - right : right - left.
    return left > right ? left - right : right - left;
}

// Define la operación pixel_format_rank con los parámetros declarados.
int pixel_format_rank(const std::string_view pixel_format) noexcept {
    // Evalúa la condición pixel_format == "NV12".
    if (pixel_format == "NV12") {
        // Devuelve 0.
        return 0;
    }

    // Evalúa la condición pixel_format == "YUY2".
    if (pixel_format == "YUY2") {
        // Devuelve 1.
        return 1;
    }

    // Evalúa la condición pixel_format == "RGB32" || pixel_format == "BGRA32".
    if (pixel_format == "RGB32" || pixel_format == "BGRA32") {
        // Devuelve 2.
        return 2;
    }

    // Evalúa la condición pixel_format == "RGB24".
    if (pixel_format == "RGB24") {
        // Devuelve 3.
        return 3;
    }

    // Evalúa la condición pixel_format == "MJPG".
    if (pixel_format == "MJPG") {
        // Devuelve 4.
        return 4;
    }

    // Devuelve 5.
    return 5;
}

// Define la operación score con los parámetros declarados.
auto score(
    // Declara el parámetro format (formato de video).
    const VideoFormat& format,
    // Incorpora const VideoFormatTarget& target en esta operación.
    const VideoFormatTarget& target
// Incorpora ) noexcept { en esta operación.
) noexcept {
    // Establece exact_resolution a partir de format.width == target.width && format.height == target.height.
    const bool exact_resolution =
        // Evalúa format.width == target.width && format.height == target.height como parte de la condición.
        format.width == target.width && format.height == target.height;

    // Establece resolution_distance a partir de static_cast<std::uint64_t>(.
    const std::uint64_t resolution_distance =
        // Incorpora static_cast<std::uint64_t>( en esta operación.
        static_cast<std::uint64_t>(
            // Ejecuta absolute_difference(format.width, target.width).
            absolute_difference(format.width, target.width)
        // Incorpora ) + en esta operación.
        ) +
        // Incorpora static_cast<std::uint64_t>( en esta operación.
        static_cast<std::uint64_t>(
            // Ejecuta absolute_difference(format.height, target.height).
            absolute_difference(format.height, target.height)
        );

    // Establece frame_rate_distance a partir de std::abs(.
    const double frame_rate_distance = std::abs(
        // Ejecuta format.frames_per_second() -.
        format.frames_per_second() -
        // Incorpora static_cast<double>(target.frames_per_second) en esta operación.
        static_cast<double>(target.frames_per_second)
    );

    // Devuelve std::tuple{.
    return std::tuple{
        // Aporta !exact_resolution como elemento de la lista.
        !exact_resolution,
        // Aporta resolution_distance como elemento de la lista.
        resolution_distance,
        // Aporta frame_rate_distance como elemento de la lista.
        frame_rate_distance,
        // Ejecuta pixel_format_rank(format.pixel_format),.
        pixel_format_rank(format.pixel_format),
        // Incorpora format.native_type_index en esta operación.
        format.native_type_index
    };
}

// Incorpora } // namespace en esta operación.
}  // namespace

// Define la operación VideoFormatSelector::select_best con los parámetros declarados.
const VideoFormat& VideoFormatSelector::select_best(
    // Declara el parámetro formats.
    const std::vector<VideoFormat>& formats,
    // Incorpora const VideoFormatTarget& target en esta operación.
    const VideoFormatTarget& target
) {
    // Evalúa la condición formats.empty().
    if (formats.empty()) {
        // Interrumpe la operación con std::runtime_error(.
        throw std::runtime_error(
            // Aporta el texto "The selected camera does not expose a usable video format." a la expresión.
            "The selected camera does not expose a usable video format."
        );
    }

    // Devuelve *std::min_element(.
    return *std::min_element(
        // Ejecuta formats.begin(),.
        formats.begin(),
        // Ejecuta formats.end(),.
        formats.end(),
        // Envía el mensaje de Objective-C indicado por [&target](const VideoFormat& left, const VideoFormat& right) {.
        [&target](const VideoFormat& left, const VideoFormat& right) {
            // Devuelve score(left, target) < score(right, target).
            return score(left, target) < score(right, target);
        }
    );
}

// Incorpora } // namespace orvix::capture en esta operación.
}  // namespace orvix::capture
