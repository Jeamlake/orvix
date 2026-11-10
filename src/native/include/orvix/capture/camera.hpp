// Aplica la directiva del compilador once.
#pragma once

// Incluye las declaraciones de camera device de ORVIX.
#include "orvix/capture/camera_device.hpp"
// Incluye las declaraciones de continuous frame capture de ORVIX.
#include "orvix/capture/continuous_frame_capture.hpp"
// Incluye las declaraciones de video format de ORVIX.
#include "orvix/capture/video_format.hpp"

// Incluye cstddef para usar tipos de tamaño y bytes.
#include <cstddef>
// Incluye vector para usar contenedores dinámicos.
#include <vector>

// Agrupa las declaraciones bajo orvix::capture.
namespace orvix::capture {

// Declara el tipo Camera y las operaciones o datos de su contrato.
class Camera {
// Expone los miembros de la interfaz.
public:
    // Solicita la implementación predeterminada del compilador.
    virtual ~Camera() = default;

    // Ejecuta virtual void open(const CameraDevice& device) = 0.
    virtual void open(const CameraDevice& device) = 0;
    // Establece noexcept a partir de 0.
    virtual void close() noexcept = 0;

    // Declara el atributo de compilación [[nodiscard]].
    [[nodiscard]]
    // Ejecuta virtual std::vector<VideoFormat> available_formats() const = 0.
    virtual std::vector<VideoFormat> available_formats() const = 0;

    // Declara el atributo de compilación [[nodiscard]].
    [[nodiscard]]
    // Ejecuta virtual VideoFormat configure(const VideoFormatTarget& target) = 0.
    virtual VideoFormat configure(const VideoFormatTarget& target) = 0;

    // Declara el atributo de compilación [[nodiscard]].
    [[nodiscard]]
    // Define la operación capture_frames con los parámetros declarados.
    virtual CaptureSummary capture_frames(
        // Declara el parámetro requested_frames.
        std::size_t requested_frames,
        // Establece sink a partir de nullptr.
        FrameSink* sink = nullptr
    // Incorpora ) = 0 en esta operación.
    ) = 0;

    // Declara el atributo de compilación [[nodiscard]].
    [[nodiscard]]
    // Establece noexcept a partir de 0.
    virtual bool is_open() const noexcept = 0;
};

// Incorpora } // namespace orvix::capture en esta operación.
}  // namespace orvix::capture
