// Incluye las declaraciones de camera selector de ORVIX.
#include "orvix/capture/camera_selector.hpp"

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
    // Incorpora const std::vector<orvix::capture::CameraDevice> devices{ en esta operación.
    const std::vector<orvix::capture::CameraDevice> devices{
        // Aporta {0, "Camera A", "device-a", "Test Backend"} como elemento de la lista.
        {0, "Camera A", "device-a", "Test Backend"},
        // Incorpora {1, "Camera B", "device-b", "Test Backend"} en esta operación.
        {1, "Camera B", "device-b", "Test Backend"}
    };

    // Establece selected a partir de orvix::capture::CameraSelector::select_by_index(devices, 1).
    const auto& selected =
        // Ejecuta orvix::capture::CameraSelector::select_by_index(devices, 1).
        orvix::capture::CameraSelector::select_by_index(devices, 1);

    // Evalúa la condición selected.index != 1 || selected.friendly_name != "Camera B".
    if (selected.index != 1 || selected.friendly_name != "Camera B") {
        // Incorpora std::cerr << "valid selection assertion failed\n" en esta operación.
        std::cerr << "valid selection assertion failed\n";
        // Devuelve EXIT_FAILURE.
        return EXIT_FAILURE;
    }

    // Establece invalid_index_rejected a partir de false.
    bool invalid_index_rejected = false;

    // Ejecuta el bloque protegido contra excepciones.
    try {
        // Incorpora static_cast<void>( en esta operación.
        static_cast<void>(
            // Ejecuta orvix::capture::CameraSelector::select_by_index(devices, 8).
            orvix::capture::CameraSelector::select_by_index(devices, 8)
        );
    }
    // Maneja la excepción declarada en catch (const std::out_of_range&) {.
    catch (const std::out_of_range&) {
        // Establece invalid_index_rejected a partir de true.
        invalid_index_rejected = true;
    }

    // Evalúa la condición !invalid_index_rejected.
    if (!invalid_index_rejected) {
        // Incorpora std::cerr << "invalid index was not rejected\n" en esta operación.
        std::cerr << "invalid index was not rejected\n";
        // Devuelve EXIT_FAILURE.
        return EXIT_FAILURE;
    }

    // Establece empty_list_rejected a partir de false.
    bool empty_list_rejected = false;

    // Ejecuta el bloque protegido contra excepciones.
    try {
        // Declara empty con el tipo indicado.
        const std::vector<orvix::capture::CameraDevice> empty;
        // Incorpora static_cast<void>( en esta operación.
        static_cast<void>(
            // Ejecuta orvix::capture::CameraSelector::select_by_index(empty, 0).
            orvix::capture::CameraSelector::select_by_index(empty, 0)
        );
    }
    // Maneja la excepción declarada en catch (const std::out_of_range&) {.
    catch (const std::out_of_range&) {
        // Establece empty_list_rejected a partir de true.
        empty_list_rejected = true;
    }

    // Evalúa la condición !empty_list_rejected.
    if (!empty_list_rejected) {
        // Incorpora std::cerr << "empty list was not rejected\n" en esta operación.
        std::cerr << "empty list was not rejected\n";
        // Devuelve EXIT_FAILURE.
        return EXIT_FAILURE;
    }

    // Establece std::cout << "TC-CAP-002 a partir de PASS\n".
    std::cout << "TC-CAP-002=PASS\n";
    // Devuelve EXIT_SUCCESS.
    return EXIT_SUCCESS;
}
