// Incluye las declaraciones de camera selector de ORVIX.
#include "orvix/capture/camera_selector.hpp"

// Incluye sstream para usar flujos de texto en memoria.
#include <sstream>
// Incluye stdexcept para usar tipos estándar de excepciones.
#include <stdexcept>

// Agrupa las declaraciones bajo orvix::capture.
namespace orvix::capture {

// Define la operación CameraSelector::select_by_index con los parámetros declarados.
const CameraDevice& CameraSelector::select_by_index(
    // Declara el parámetro devices (dispositivos detectados).
    const std::vector<CameraDevice>& devices,
    // Incorpora const std::size_t index en esta operación.
    const std::size_t index
) {
    // Evalúa la condición index >= devices.size().
    if (index >= devices.size()) {
        // Declara message con el tipo indicado.
        std::ostringstream message;
        // Incorpora message en esta operación.
        message
            // Incorpora << "Camera index " en esta operación.
            << "Camera index "
            // Incorpora << index en esta operación.
            << index
            // Incorpora << " is out of range. Available devices: " en esta operación.
            << " is out of range. Available devices: "
            // Ejecuta << devices.size().
            << devices.size();

        // Interrumpe la operación con std::out_of_range(message.str()).
        throw std::out_of_range(message.str());
    }

    // Devuelve devices[index].
    return devices[index];
}

// Incorpora } // namespace orvix::capture en esta operación.
}  // namespace orvix::capture
