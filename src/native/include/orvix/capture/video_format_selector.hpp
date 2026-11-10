// Aplica la directiva del compilador once.
#pragma once

// Incluye las declaraciones de video format de ORVIX.
#include "orvix/capture/video_format.hpp"

// Incluye vector para usar contenedores dinámicos.
#include <vector>

// Agrupa las declaraciones bajo orvix::capture.
namespace orvix::capture {

// Declara el tipo VideoFormatSelector y las operaciones o datos de su contrato.
class VideoFormatSelector final {
// Expone los miembros de la interfaz.
public:
    // Declara el atributo de compilación [[nodiscard]].
    [[nodiscard]]
    // Define la operación select_best con los parámetros declarados.
    static const VideoFormat& select_best(
        // Declara el parámetro formats.
        const std::vector<VideoFormat>& formats,
        // Incorpora const VideoFormatTarget& target en esta operación.
        const VideoFormatTarget& target
    );
};

// Incorpora } // namespace orvix::capture en esta operación.
}  // namespace orvix::capture
