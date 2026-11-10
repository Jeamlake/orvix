// Aplica la directiva del compilador once.
#pragma once

// Incluye las declaraciones de camera device de ORVIX.
#include "orvix/capture/camera_device.hpp"

// Incluye cstddef para usar tipos de tamaño y bytes.
#include <cstddef>
// Incluye vector para usar contenedores dinámicos.
#include <vector>

// Agrupa las declaraciones bajo orvix::capture.
namespace orvix::capture {

// Declara el tipo CameraSelector y las operaciones o datos de su contrato.
class CameraSelector final {
// Expone los miembros de la interfaz.
public:
    // Declara el atributo de compilación [[nodiscard]].
    [[nodiscard]]
    // Define la operación select_by_index con los parámetros declarados.
    static const CameraDevice& select_by_index(
        // Declara el parámetro devices (dispositivos detectados).
        const std::vector<CameraDevice>& devices,
        // Incorpora std::size_t index en esta operación.
        std::size_t index
    );
};

// Incorpora } // namespace orvix::capture en esta operación.
}  // namespace orvix::capture
