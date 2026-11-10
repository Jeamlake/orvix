// Incluye las declaraciones de video format selector de ORVIX.
#include "orvix/capture/video_format_selector.hpp"

// Incluye cstdlib para usar utilidades de la biblioteca C.
#include <cstdlib>
// Incluye iostream para usar entrada y salida por consola.
#include <iostream>
// Incluye stdexcept para usar tipos estándar de excepciones.
#include <stdexcept>
// Incluye vector para usar contenedores dinámicos.
#include <vector>

// Define la operación main con los parámetros declarados.
int main() {
    // Introduce en este ámbito orvix::capture::VideoFormat.
    using orvix::capture::VideoFormat;
    // Introduce en este ámbito orvix::capture::VideoFormatSelector.
    using orvix::capture::VideoFormatSelector;
    // Introduce en este ámbito orvix::capture::VideoFormatTarget.
    using orvix::capture::VideoFormatTarget;

    // Incorpora const std::vector<VideoFormat> formats{ en esta operación.
    const std::vector<VideoFormat> formats{
        // Aporta {0, 640, 480, 30, 1, "YUY2", false} como elemento de la lista.
        {0, 640, 480, 30, 1, "YUY2", false},
        // Aporta {1, 1280, 720, 15, 1, "YUY2", false} como elemento de la lista.
        {1, 1280, 720, 15, 1, "YUY2", false},
        // Aporta {2, 1280, 720, 30, 1, "MJPG", true} como elemento de la lista.
        {2, 1280, 720, 30, 1, "MJPG", true},
        // Aporta {3, 1280, 720, 30, 1, "NV12", false} como elemento de la lista.
        {3, 1280, 720, 30, 1, "NV12", false},
        // Incorpora {4, 1920, 1080, 30, 1, "MJPG", true} en esta operación.
        {4, 1920, 1080, 30, 1, "MJPG", true}
    };

    // Establece selected a partir de VideoFormatSelector::select_best(.
    const auto& selected = VideoFormatSelector::select_best(
        // Aporta formats como elemento de la lista.
        formats,
        // Incorpora VideoFormatTarget{} en esta operación.
        VideoFormatTarget{}
    );

    // Evalúa la condición selected.native_type_index != 3.
    if (selected.native_type_index != 3) {
        // Incorpora std::cerr << "exact target or pixel-format preference failed\n" en esta operación.
        std::cerr << "exact target or pixel-format preference failed\n";
        // Devuelve EXIT_FAILURE.
        return EXIT_FAILURE;
    }

    // Incorpora const std::vector<VideoFormat> fallback_formats{ en esta operación.
    const std::vector<VideoFormat> fallback_formats{
        // Aporta {0, 640, 480, 30, 1, "YUY2", false} como elemento de la lista.
        {0, 640, 480, 30, 1, "YUY2", false},
        // Incorpora {1, 1920, 1080, 60, 1, "MJPG", true} en esta operación.
        {1, 1920, 1080, 60, 1, "MJPG", true}
    };

    // Establece fallback a partir de VideoFormatSelector::select_best(.
    const auto& fallback = VideoFormatSelector::select_best(
        // Aporta fallback_formats como elemento de la lista.
        fallback_formats,
        // Incorpora VideoFormatTarget{} en esta operación.
        VideoFormatTarget{}
    );

    // Evalúa la condición fallback.native_type_index != 0.
    if (fallback.native_type_index != 0) {
        // Incorpora std::cerr << "nearest-resolution fallback failed\n" en esta operación.
        std::cerr << "nearest-resolution fallback failed\n";
        // Devuelve EXIT_FAILURE.
        return EXIT_FAILURE;
    }

    // Establece empty_list_rejected a partir de false.
    bool empty_list_rejected = false;

    // Ejecuta el bloque protegido contra excepciones.
    try {
        // Declara empty con el tipo indicado.
        const std::vector<VideoFormat> empty;
        // Incorpora static_cast<void>( en esta operación.
        static_cast<void>(
            // Ejecuta VideoFormatSelector::select_best(empty, VideoFormatTarget{}).
            VideoFormatSelector::select_best(empty, VideoFormatTarget{})
        );
    }
    // Maneja la excepción declarada en catch (const std::runtime_error&) {.
    catch (const std::runtime_error&) {
        // Establece empty_list_rejected a partir de true.
        empty_list_rejected = true;
    }

    // Evalúa la condición !empty_list_rejected.
    if (!empty_list_rejected) {
        // Incorpora std::cerr << "empty format list was not rejected\n" en esta operación.
        std::cerr << "empty format list was not rejected\n";
        // Devuelve EXIT_FAILURE.
        return EXIT_FAILURE;
    }

    // Establece FORMAT a partir de PASS\n".
    std::cout << "TC-CAP-FORMAT=PASS\n";
    // Devuelve EXIT_SUCCESS.
    return EXIT_SUCCESS;
}
