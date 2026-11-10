// Aplica la directiva del compilador once.
#pragma once

// Incluye las declaraciones de camera device de ORVIX.
#include "orvix/capture/camera_device.hpp"

// Incluye vector para usar contenedores dinámicos.
#include <vector>

// Agrupa las declaraciones bajo orvix::capture.
namespace orvix::capture {

// Declara el tipo DeviceEnumerator y las operaciones o datos de su contrato.
class DeviceEnumerator {
// Expone los miembros de la interfaz.
public:
    // Solicita la implementación predeterminada del compilador.
    virtual ~DeviceEnumerator() = default;

    // Declara el atributo de compilación [[nodiscard]].
    [[nodiscard]]
    // Ejecuta virtual std::vector<CameraDevice> enumerate() const = 0.
    virtual std::vector<CameraDevice> enumerate() const = 0;
};

// Incorpora } // namespace orvix::capture en esta operación.
}  // namespace orvix::capture