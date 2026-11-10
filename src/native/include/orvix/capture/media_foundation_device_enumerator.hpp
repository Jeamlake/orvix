// Aplica la directiva del compilador once.
#pragma once

// Incluye las declaraciones de device enumerator de ORVIX.
#include "orvix/capture/device_enumerator.hpp"

// Agrupa las declaraciones bajo orvix::capture.
namespace orvix::capture {

// Declara el tipo MediaFoundationDeviceEnumerator y las operaciones o datos de su contrato.
class MediaFoundationDeviceEnumerator final : public DeviceEnumerator {
// Expone los miembros de la interfaz.
public:
    // Declara el atributo de compilación [[nodiscard]].
    [[nodiscard]]
    // Ejecuta std::vector<CameraDevice> enumerate() const override.
    std::vector<CameraDevice> enumerate() const override;
};

// Incorpora } // namespace orvix::capture en esta operación.
}  // namespace orvix::capture