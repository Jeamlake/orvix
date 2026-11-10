// Incluye las declaraciones de platform factory de ORVIX.
#include "orvix/capture/platform_factory.hpp"

// Incluye cstdlib para usar utilidades de la biblioteca C.
#include <cstdlib>
// Incluye iostream para usar entrada y salida por consola.
#include <iostream>
// Incluye string_view para usar vistas de cadenas.
#include <string_view>

// Define la operación main con los parámetros declarados.
int main() {
    // Establece enumerator a partir de orvix::capture::create_platform_device_enumerator().
    const auto enumerator =
        // Ejecuta orvix::capture::create_platform_device_enumerator().
        orvix::capture::create_platform_device_enumerator();
    // Establece camera a partir de orvix::capture::create_platform_camera().
    const auto camera = orvix::capture::create_platform_camera();
    // Establece backend_name a partir de orvix::capture::platform_backend_name().
    const auto backend_name = orvix::capture::platform_backend_name();

// Incluye este bloque si se cumple defined(_WIN32).
#if defined(_WIN32)
    // Establece expected_backend a partir de "Windows Media Foundation".
    constexpr std::string_view expected_backend =
        // Incorpora "Windows Media Foundation" en esta operación.
        "Windows Media Foundation";
// Evalúa la condición de compilación alternativa defined(__APPLE__).
#elif defined(__APPLE__)
    // Establece expected_backend a partir de "macOS AVFoundation".
    constexpr std::string_view expected_backend = "macOS AVFoundation";
// Evalúa la condición de compilación alternativa defined(__linux__).
#elif defined(__linux__)
    // Establece expected_backend a partir de "Linux V4L2".
    constexpr std::string_view expected_backend = "Linux V4L2";
// Cierra la condición de compilación.
#endif

    // Evalúa la condición enumerator == nullptr ||.
    if (
        // Combina esta condición con enumerator == nullptr ||.
        enumerator == nullptr ||
        // Combina esta condición con camera == nullptr ||.
        camera == nullptr ||
        // Combina esta condición con camera->is_open() ||.
        camera->is_open() ||
        // Evalúa backend_name != expected_backend como parte de la condición.
        backend_name != expected_backend
    ) {
        // Incorpora std::cerr << "platform capture factory assertion failed\n" en esta operación.
        std::cerr << "platform capture factory assertion failed\n";
        // Devuelve EXIT_FAILURE.
        return EXIT_FAILURE;
    }

    // Establece PLATFORM a partir de " << backend_name << "\n".
    std::cout << "TC-CAP-003-PLATFORM=" << backend_name << "\n";
    // Devuelve EXIT_SUCCESS.
    return EXIT_SUCCESS;
}
