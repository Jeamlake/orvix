// Incluye las declaraciones de logger de ORVIX.
#include "orvix/observability/logger.hpp"

// Incluye chrono para usar relojes y duraciones.
#include <chrono>
// Incluye cstdlib para usar utilidades de la biblioteca C.
#include <cstdlib>
// Incluye filesystem para usar rutas y operaciones de archivos.
#include <filesystem>
// Incluye fstream para usar flujos para archivos.
#include <fstream>
// Incluye iostream para usar entrada y salida por consola.
#include <iostream>
// Importa las declaraciones de iterator para esta unidad.
#include <iterator>
// Incluye string para usar cadenas de texto.
#include <string>

// Define la operación main con los parámetros declarados.
int main() {
    // Establece unique_value a partir de std::chrono::steady_clock::now().time_since_epoch().count().
    const auto unique_value =
        // Ejecuta std::chrono::steady_clock::now().time_since_epoch().count().
        std::chrono::steady_clock::now().time_since_epoch().count();
    // Establece log_path a partir de std::filesystem::temp_directory_path() /.
    const std::filesystem::path log_path =
        // Ejecuta std::filesystem::temp_directory_path() /.
        std::filesystem::temp_directory_path() /
        // Ejecuta ("orvix_logger_test_" + std::to_string(unique_value) + ".log").
        ("orvix_logger_test_" + std::to_string(unique_value) + ".log");

    {
        // Ejecuta orvix::observability::Logger logger(log_path).
        orvix::observability::Logger logger(log_path);
        // Ejecuta logger.info("ORV-TEST-001", "test_started", "logger test").
        logger.info("ORV-TEST-001", "test_started", "logger test");
        // Ejecuta logger.error(.
        logger.error(
            // Aporta el texto "ORV-TEST-500" a la expresión.
            "ORV-TEST-500",
            // Aporta el texto "test_error" a la expresión.
            "test_error",
            // Aporta el texto "quoted \"message\"" a la expresión.
            "quoted \"message\""
        );
    }

    // Ejecuta std::ifstream input(log_path).
    std::ifstream input(log_path);
    // Incorpora const std::string content{ en esta operación.
    const std::string content{
        // Aporta std::istreambuf_iterator<char>(input) como elemento de la lista.
        std::istreambuf_iterator<char>(input),
        // Incorpora std::istreambuf_iterator<char>() en esta operación.
        std::istreambuf_iterator<char>()
    };

    // Declara remove_error con el tipo indicado.
    std::error_code remove_error;
    // Ejecuta std::filesystem::remove(log_path, remove_error).
    std::filesystem::remove(log_path, remove_error);

    // Evalúa la condición content.find("level=INFO code=ORV-TEST-001 event=test_started") ==.
    if (
        // Establece level a partir de INFO code=ORV-TEST-001 event=test_started") ==.
        content.find("level=INFO code=ORV-TEST-001 event=test_started") ==
            // Combina esta condición con std::string::npos ||.
            std::string::npos ||
        // Establece level a partir de ERROR code=ORV-TEST-500 event=test_error") ==.
        content.find("level=ERROR code=ORV-TEST-500 event=test_error") ==
            // Combina esta condición con std::string::npos ||.
            std::string::npos ||
        // Ejecuta content.find("quoted \\\"message\\\"") == std::string::npos.
        content.find("quoted \\\"message\\\"") == std::string::npos
    ) {
        // Incorpora std::cerr << "structured log assertion failed\n" en esta operación.
        std::cerr << "structured log assertion failed\n";
        // Devuelve EXIT_FAILURE.
        return EXIT_FAILURE;
    }

    // Establece std::cout << "TC-LOG-001 a partir de PASS\n".
    std::cout << "TC-LOG-001=PASS\n";
    // Devuelve EXIT_SUCCESS.
    return EXIT_SUCCESS;
}
