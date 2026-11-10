// Incluye las declaraciones de hresult error de ORVIX.
#include "orvix/capture/hresult_error.hpp"
// Incluye las declaraciones de media foundation camera de ORVIX.
#include "orvix/capture/media_foundation_camera.hpp"

// Incluye cstdlib para usar utilidades de la biblioteca C.
#include <cstdlib>
// Incluye iostream para usar entrada y salida por consola.
#include <iostream>
// Incluye stdexcept para usar tipos estándar de excepciones.
#include <stdexcept>

// Define la operación main con los parámetros declarados.
int main() {
    // Declara camera con el tipo indicado.
    orvix::capture::MediaFoundationCamera camera;

    // Evalúa la condición camera.is_open().
    if (camera.is_open()) {
        // Incorpora std::cerr << "new camera unexpectedly reports OPEN\n" en esta operación.
        std::cerr << "new camera unexpectedly reports OPEN\n";
        // Devuelve EXIT_FAILURE.
        return EXIT_FAILURE;
    }

    // Cierra el recurso mediante camera.close.
    camera.close();
    // Cierra el recurso mediante camera.close.
    camera.close();

    // Incorpora const orvix::capture::CameraDevice incomplete_device{ en esta operación.
    const orvix::capture::CameraDevice incomplete_device{
        // Aporta 0 como elemento de la lista.
        0,
        // Aporta el texto "Incomplete Test Camera" a la expresión.
        "Incomplete Test Camera",
        // Aporta el texto "" a la expresión.
        "",
        // Aporta el texto "Test Backend" a la expresión.
        "Test Backend"
    };

    // Establece empty_symbolic_link_rejected a partir de false.
    bool empty_symbolic_link_rejected = false;

    // Ejecuta el bloque protegido contra excepciones.
    try {
        // Ejecuta camera.open(incomplete_device).
        camera.open(incomplete_device);
    }
    // Maneja la excepción declarada en catch (const std::invalid_argument&) {.
    catch (const std::invalid_argument&) {
        // Establece empty_symbolic_link_rejected a partir de true.
        empty_symbolic_link_rejected = true;
    }

    // Evalúa la condición !empty_symbolic_link_rejected || camera.is_open().
    if (!empty_symbolic_link_rejected || camera.is_open()) {
        // Incorpora std::cerr << "invalid camera descriptor was not rejected safely\n" en esta operación.
        std::cerr << "invalid camera descriptor was not rejected safely\n";
        // Devuelve EXIT_FAILURE.
        return EXIT_FAILURE;
    }

    // Incorpora const orvix::capture::CameraDevice unavailable_device{ en esta operación.
    const orvix::capture::CameraDevice unavailable_device{
        // Aporta 0 como elemento de la lista.
        0,
        // Aporta el texto "Unavailable Test Camera" a la expresión.
        "Unavailable Test Camera",
        // Aporta el texto R"(\\?\orvix#nonexistent_camera#tc_cap_003)" a la expresión.
        R"(\\?\orvix#nonexistent_camera#tc_cap_003)",
        // Aporta el texto "Windows Media Foundation" a la expresión.
        "Windows Media Foundation"
    };

    // Establece unavailable_device_rejected a partir de false.
    bool unavailable_device_rejected = false;

    // Ejecuta el bloque protegido contra excepciones.
    try {
        // Ejecuta camera.open(unavailable_device).
        camera.open(unavailable_device);
    }
    // Maneja la excepción declarada en catch (const orvix::capture::HResultError&) {.
    catch (const orvix::capture::HResultError&) {
        // Establece unavailable_device_rejected a partir de true.
        unavailable_device_rejected = true;
    }

    // Evalúa la condición !unavailable_device_rejected || camera.is_open().
    if (!unavailable_device_rejected || camera.is_open()) {
        // Incorpora std::cerr << "unavailable camera was not rejected safely\n" en esta operación.
        std::cerr << "unavailable camera was not rejected safely\n";
        // Devuelve EXIT_FAILURE.
        return EXIT_FAILURE;
    }

    // Establece std::cout << "TC-CAP-003 a partir de PASS\n".
    std::cout << "TC-CAP-003=PASS\n";
    // Devuelve EXIT_SUCCESS.
    return EXIT_SUCCESS;
}
