// Incluye las declaraciones de camera device de ORVIX.
#include "orvix/capture/camera_device.hpp"

// Incluye cstdlib para usar utilidades de la biblioteca C.
#include <cstdlib>
// Incluye iostream para usar entrada y salida por consola.
#include <iostream>
// Incluye string para usar cadenas de texto.
#include <string>

// Define la operación main con los parámetros declarados.
int main() {
    // Incorpora const orvix::capture::CameraDevice device{ en esta operación.
    const orvix::capture::CameraDevice device{
        // Aporta 3 como elemento de la lista.
        3,
        // Aporta el texto "Synthetic Test Camera" a la expresión.
        "Synthetic Test Camera",
        // Aporta el texto R"(\\?\usb#vid_0000&pid_0000#test)" a la expresión.
        R"(\\?\usb#vid_0000&pid_0000#test)",
        // Aporta el texto "Test Backend" a la expresión.
        "Test Backend"
    };

    // Evalúa la condición device.index != 3.
    if (device.index != 3) {
        // Incorpora std::cerr << "index assertion failed\n" en esta operación.
        std::cerr << "index assertion failed\n";
        // Devuelve EXIT_FAILURE.
        return EXIT_FAILURE;
    }

    // Evalúa la condición device.friendly_name != "Synthetic Test Camera".
    if (device.friendly_name != "Synthetic Test Camera") {
        // Incorpora std::cerr << "friendly_name assertion failed\n" en esta operación.
        std::cerr << "friendly_name assertion failed\n";
        // Devuelve EXIT_FAILURE.
        return EXIT_FAILURE;
    }

    // Evalúa la condición device.symbolic_link.empty().
    if (device.symbolic_link.empty()) {
        // Incorpora std::cerr << "symbolic_link assertion failed\n" en esta operación.
        std::cerr << "symbolic_link assertion failed\n";
        // Devuelve EXIT_FAILURE.
        return EXIT_FAILURE;
    }

    // Evalúa la condición device.backend != "Test Backend".
    if (device.backend != "Test Backend") {
        // Incorpora std::cerr << "backend assertion failed\n" en esta operación.
        std::cerr << "backend assertion failed\n";
        // Devuelve EXIT_FAILURE.
        return EXIT_FAILURE;
    }

    // Establece MODEL a partir de PASS\n".
    std::cout << "TC-CAP-001-MODEL=PASS\n";
    // Devuelve EXIT_SUCCESS.
    return EXIT_SUCCESS;
}