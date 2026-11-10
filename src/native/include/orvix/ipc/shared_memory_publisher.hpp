// Aplica la directiva del compilador once.
#pragma once

// Incluye las declaraciones de continuous frame capture de ORVIX.
#include "orvix/capture/continuous_frame_capture.hpp"
// Incluye las declaraciones de video format de ORVIX.
#include "orvix/capture/video_format.hpp"
// Incluye las declaraciones de shared memory region de ORVIX.
#include "orvix/ipc/shared_memory_region.hpp"

// Incluye cstddef para usar tipos de tamaño y bytes.
#include <cstddef>
// Incluye cstdint para usar enteros de tamaño fijo.
#include <cstdint>
// Incluye memory para usar punteros inteligentes y gestión de memoria.
#include <memory>
// Incluye span para usar vistas de memoria contigua.
#include <span>
// Incluye string para usar cadenas de texto.
#include <string>

// Agrupa las declaraciones bajo orvix::ipc.
namespace orvix::ipc {

// Declara el tipo PublisherStats y las operaciones o datos de su contrato.
struct PublisherStats final {
    // Inicializa published_sequence (última secuencia publicada) con inicialización por valor.
    std::uint64_t published_sequence{};
    // Inicializa consumed_sequence con inicialización por valor.
    std::uint64_t consumed_sequence{};
    // Inicializa overwritten_frames con inicialización por valor.
    std::uint64_t overwritten_frames{};
    // Inicializa producer_fps con inicialización por valor.
    double producer_fps{};
    // Inicializa buffer_utilization con inicialización por valor.
    double buffer_utilization{};
};

// Declara el tipo SharedMemoryPublisher y las operaciones o datos de su contrato.
class SharedMemoryPublisher final : public capture::FrameSink {
// Expone los miembros de la interfaz.
public:
    // Ejecuta SharedMemoryPublisher(.
    SharedMemoryPublisher(
        // Declara el parámetro name.
        std::string name,
        // Declara el parámetro format (formato de video).
        const capture::VideoFormat& format,
        // Establece slot_count (cantidad de posiciones del anillo) a partir de 3.
        std::uint32_t slot_count = 3,
        // Establece payload_capacity a partir de 0.
        std::size_t payload_capacity = 0
    );
    // Define ~SharedMemoryPublisher para cerrar los recursos al destruir la instancia.
    ~SharedMemoryPublisher() override;

    // Prohíbe esta operación en tiempo de compilación.
    SharedMemoryPublisher(const SharedMemoryPublisher&) = delete;
    // Establece operator a partir de (const SharedMemoryPublisher&) = delete.
    SharedMemoryPublisher& operator=(const SharedMemoryPublisher&) = delete;
    // Prohíbe esta operación en tiempo de compilación.
    SharedMemoryPublisher(SharedMemoryPublisher&&) = delete;
    // Establece operator a partir de (SharedMemoryPublisher&&) = delete.
    SharedMemoryPublisher& operator=(SharedMemoryPublisher&&) = delete;

    // Define la operación publish con los parámetros declarados.
    void publish(
        // Declara el parámetro metadata.
        const capture::FrameMetadata& metadata,
        // Incorpora std::span<const std::byte> payload en esta operación.
        std::span<const std::byte> payload
    // Incorpora ) override en esta operación.
    ) override;

    // Ejecuta void mark_completed() noexcept.
    void mark_completed() noexcept;
    // Ejecuta void mark_failed() noexcept.
    void mark_failed() noexcept;

    // Declara el atributo de compilación [[nodiscard]].
    [[nodiscard]]
    // Ejecuta bool stop_requested() const noexcept override.
    bool stop_requested() const noexcept override;

    // Declara el atributo de compilación [[nodiscard]].
    [[nodiscard]]
    // Ejecuta PublisherStats stats() const noexcept.
    PublisherStats stats() const noexcept;

    // Declara el atributo de compilación [[nodiscard]].
    [[nodiscard]]
    // Ejecuta std::span<const std::byte> memory() const noexcept.
    std::span<const std::byte> memory() const noexcept;

    // Declara el atributo de compilación [[nodiscard]].
    [[nodiscard]]
    // Ejecuta const std::string& name() const noexcept.
    const std::string& name() const noexcept;

// Restringe estos miembros a la implementación.
private:
    // Declara region_ con el tipo indicado.
    std::unique_ptr<SharedMemoryRegion> region_;
    // Declara format_ (formato de video) con el tipo indicado.
    capture::VideoFormat format_;
    // Inicializa slot_count_ (cantidad de posiciones del anillo) con inicialización por valor.
    std::uint32_t slot_count_{};
    // Inicializa payload_capacity_ con inicialización por valor.
    std::size_t payload_capacity_{};
    // Inicializa slot_stride_ con inicialización por valor.
    std::size_t slot_stride_{};
    // Inicializa first_sequence_ con inicialización por valor.
    std::uint64_t first_sequence_{};
    // Inicializa first_timestamp_100ns_ con inicialización por valor.
    std::int64_t first_timestamp_100ns_{};
    // Inicializa terminal_state_written_ con false.
    bool terminal_state_written_{false};
};

// Incorpora } // namespace orvix::ipc en esta operación.
}  // namespace orvix::ipc
