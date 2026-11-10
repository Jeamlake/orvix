// Aplica la directiva del compilador once.
#pragma once

// Incluye las declaraciones de camera de ORVIX.
#include "orvix/capture/camera.hpp"

// Incluye memory para usar punteros inteligentes y gestión de memoria.
#include <memory>

// Agrupa las declaraciones bajo orvix::capture.
namespace orvix::capture {

// Declara el tipo V4l2Camera y las operaciones o datos de su contrato.
class V4l2Camera final : public Camera {
// Expone los miembros de la interfaz.
public:
    // Ejecuta V4l2Camera().
    V4l2Camera();
    // Define ~V4l2Camera para cerrar los recursos al destruir la instancia.
    ~V4l2Camera() override;

    // Prohíbe esta operación en tiempo de compilación.
    V4l2Camera(const V4l2Camera&) = delete;
    // Establece operator a partir de (const V4l2Camera&) = delete.
    V4l2Camera& operator=(const V4l2Camera&) = delete;
    // Prohíbe esta operación en tiempo de compilación.
    V4l2Camera(V4l2Camera&&) = delete;
    // Establece operator a partir de (V4l2Camera&&) = delete.
    V4l2Camera& operator=(V4l2Camera&&) = delete;

    // Ejecuta void open(const CameraDevice& device) override.
    void open(const CameraDevice& device) override;
    // Cierra el recurso mediante close.
    void close() noexcept override;

    // Declara el atributo de compilación [[nodiscard]].
    [[nodiscard]]
    // Ejecuta std::vector<VideoFormat> available_formats() const override.
    std::vector<VideoFormat> available_formats() const override;

    // Declara el atributo de compilación [[nodiscard]].
    [[nodiscard]]
    // Ejecuta VideoFormat configure(const VideoFormatTarget& target) override.
    VideoFormat configure(const VideoFormatTarget& target) override;

    // Declara el atributo de compilación [[nodiscard]].
    [[nodiscard]]
    // Define la operación capture_frames con los parámetros declarados.
    CaptureSummary capture_frames(
        // Declara el parámetro requested_frames.
        std::size_t requested_frames,
        // Establece sink a partir de nullptr.
        FrameSink* sink = nullptr
    // Incorpora ) override en esta operación.
    ) override;

    // Declara el atributo de compilación [[nodiscard]].
    [[nodiscard]]
    // Ejecuta bool is_open() const noexcept override.
    bool is_open() const noexcept override;

// Restringe estos miembros a la implementación.
private:
    // Declara el tipo Impl y las operaciones o datos de su contrato.
    struct Impl;
    // Declara impl_ con el tipo indicado.
    std::unique_ptr<Impl> impl_;
};

// Incorpora } // namespace orvix::capture en esta operación.
}  // namespace orvix::capture
