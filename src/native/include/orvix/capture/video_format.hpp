// Aplica la directiva del compilador once.
#pragma once

// Incluye cstddef para usar tipos de tamaño y bytes.
#include <cstddef>
// Incluye cstdint para usar enteros de tamaño fijo.
#include <cstdint>
// Incluye string para usar cadenas de texto.
#include <string>

// Agrupa las declaraciones bajo orvix::capture.
namespace orvix::capture {

// Declara el tipo VideoFormat y las operaciones o datos de su contrato.
struct VideoFormat final {
    // Inicializa native_type_index con inicialización por valor.
    std::size_t native_type_index{};
    // Inicializa width (ancho en píxeles) con inicialización por valor.
    std::uint32_t width{};
    // Inicializa height (alto en píxeles) con inicialización por valor.
    std::uint32_t height{};
    // Inicializa frame_rate_numerator con inicialización por valor.
    std::uint32_t frame_rate_numerator{};
    // Inicializa frame_rate_denominator con 1.
    std::uint32_t frame_rate_denominator{1};
    // Declara pixel_format (formato de píxeles) con el tipo indicado.
    std::string pixel_format;
    // Inicializa compressed con false.
    bool compressed{false};
    // Inicializa stride (bytes entre filas de imagen) con inicialización por valor.
    std::uint32_t stride{};

    // Declara el atributo de compilación [[nodiscard]].
    [[nodiscard]]
    // Define la operación frames_per_second con los parámetros declarados.
    double frames_per_second() const noexcept {
        // Evalúa la condición frame_rate_denominator == 0.
        if (frame_rate_denominator == 0) {
            // Devuelve 0.0.
            return 0.0;
        }

        // Devuelve static_cast<double>(frame_rate_numerator) /.
        return static_cast<double>(frame_rate_numerator) /
               // Incorpora static_cast<double>(frame_rate_denominator) en esta operación.
               static_cast<double>(frame_rate_denominator);
    }
};

// Declara el tipo VideoFormatTarget y las operaciones o datos de su contrato.
struct VideoFormatTarget final {
    // Inicializa width (ancho en píxeles) con 1280.
    std::uint32_t width{1280};
    // Inicializa height (alto en píxeles) con 720.
    std::uint32_t height{720};
    // Inicializa frames_per_second con 30.
    std::uint32_t frames_per_second{30};
};

// Incorpora } // namespace orvix::capture en esta operación.
}  // namespace orvix::capture
