// Aplica la directiva del compilador once.
#pragma once

// Incluye cstddef para usar tipos de tamaño y bytes.
#include <cstddef>
// Incluye string para usar cadenas de texto.
#include <string>

// Agrupa las declaraciones bajo orvix::capture.
namespace orvix::capture {

// Declara el tipo CameraDevice y las operaciones o datos de su contrato.
struct CameraDevice final {
    // Inicializa index con inicialización por valor.
    std::size_t index{};
    // Declara friendly_name con el tipo indicado.
    std::string friendly_name;
    // Declara symbolic_link con el tipo indicado.
    std::string symbolic_link;
    // Declara backend (implementación nativa) con el tipo indicado.
    std::string backend;
};

// Incorpora } // namespace orvix::capture en esta operación.
}  // namespace orvix::capture