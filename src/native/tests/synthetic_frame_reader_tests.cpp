// Incluye las declaraciones de synthetic frame reader de ORVIX.
#include "orvix/capture/synthetic_frame_reader.hpp"

// Incluye cstdlib para usar utilidades de la biblioteca C.
#include <cstdlib>
// Incluye iostream para usar entrada y salida por consola.
#include <iostream>

// Define la operación main con los parámetros declarados.
int main() {
    // Ejecuta orvix::capture::SyntheticFrameReader reader(4, 2, 30, false).
    orvix::capture::SyntheticFrameReader reader(4, 2, 30, false);
    // Establece first a partir de reader.read_next().
    const auto first = reader.read_next();
    // Establece second a partir de reader.read_next().
    const auto second = reader.read_next();

    // Evalúa la condición !first.frame_available ||.
    if (
        // Combina esta condición con !first.frame_available ||.
        !first.frame_available ||
        // Combina esta condición con first.payload.size() != 12 ||.
        first.payload.size() != 12 ||
        // Combina esta condición con first.byte_count != first.payload.size() ||.
        first.byte_count != first.payload.size() ||
        // Combina esta condición con first.stride != 4 ||.
        first.stride != 4 ||
        // Combina esta condición con second.timestamp_100ns <= first.timestamp_100ns ||.
        second.timestamp_100ns <= first.timestamp_100ns ||
        // Combina esta condición con first.payload == second.payload ||.
        first.payload == second.payload ||
        // Ejecuta reader.format().pixel_format != "NV12".
        reader.format().pixel_format != "NV12"
    ) {
        // Incorpora std::cerr << "synthetic frame source assertion failed\n" en esta operación.
        std::cerr << "synthetic frame source assertion failed\n";
        // Devuelve EXIT_FAILURE.
        return EXIT_FAILURE;
    }

    // Establece std::cout << "TC-SYN-001 a partir de PASS\n".
    std::cout << "TC-SYN-001=PASS\n";
    // Devuelve EXIT_SUCCESS.
    return EXIT_SUCCESS;
}
