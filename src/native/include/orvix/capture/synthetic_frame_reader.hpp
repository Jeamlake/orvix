// Aplica la directiva del compilador once.
#pragma once

// Incluye las declaraciones de continuous frame capture de ORVIX.
#include "orvix/capture/continuous_frame_capture.hpp"
// Incluye las declaraciones de video format de ORVIX.
#include "orvix/capture/video_format.hpp"

// Incluye chrono para usar relojes y duraciones.
#include <chrono>
// Incluye cstdint para usar enteros de tamaño fijo.
#include <cstdint>

// Agrupa las declaraciones bajo orvix::capture.
namespace orvix::capture {

// Declara el tipo SyntheticFrameReader y las operaciones o datos de su contrato.
class SyntheticFrameReader final : public FrameReader {
// Expone los miembros de la interfaz.
public:
    // Ejecuta SyntheticFrameReader(.
    SyntheticFrameReader(
        // Establece width (ancho en píxeles) a partir de 1280.
        std::uint32_t width = 1280,
        // Establece height (alto en píxeles) a partir de 720.
        std::uint32_t height = 720,
        // Establece frames_per_second a partir de 30.
        std::uint32_t frames_per_second = 30,
        // Establece realtime a partir de true.
        bool realtime = true
    );

    // Declara el atributo de compilación [[nodiscard]].
    [[nodiscard]]
    // Ejecuta FrameReadResult read_next() override.
    FrameReadResult read_next() override;

    // Declara el atributo de compilación [[nodiscard]].
    [[nodiscard]]
    // Ejecuta const VideoFormat& format() const noexcept.
    const VideoFormat& format() const noexcept;

// Restringe estos miembros a la implementación.
private:
    // Declara format_ (formato de video) con el tipo indicado.
    VideoFormat format_;
    // Inicializa generated_frames_ con inicialización por valor.
    std::uint64_t generated_frames_{};
    // Inicializa realtime_ con true.
    bool realtime_{true};
    // Declara next_frame_at_ con el tipo indicado.
    std::chrono::steady_clock::time_point next_frame_at_;
};

// Incorpora } // namespace orvix::capture en esta operación.
}  // namespace orvix::capture
