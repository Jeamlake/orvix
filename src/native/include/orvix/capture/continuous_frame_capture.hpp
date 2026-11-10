// Aplica la directiva del compilador once.
#pragma once

// Incluye las declaraciones de video format de ORVIX.
#include "orvix/capture/video_format.hpp"

// Incluye cstddef para usar tipos de tamaño y bytes.
#include <cstddef>
// Incluye cstdint para usar enteros de tamaño fijo.
#include <cstdint>
// Incluye span para usar vistas de memoria contigua.
#include <span>
// Incluye string para usar cadenas de texto.
#include <string>
// Incluye vector para usar contenedores dinámicos.
#include <vector>

// Agrupa las declaraciones bajo orvix::capture.
namespace orvix::capture {

// Declara el tipo FrameReadResult y las operaciones o datos de su contrato.
struct FrameReadResult final {
    // Inicializa frame_available con false.
    bool frame_available{false};
    // Inicializa timestamp_100ns (marca de tiempo en unidades de 100 ns) con inicialización por valor.
    std::int64_t timestamp_100ns{};
    // Inicializa byte_count con inicialización por valor.
    std::size_t byte_count{};
    // Inicializa stride (bytes entre filas de imagen) con inicialización por valor.
    std::uint32_t stride{};
    // Declara payload (carga útil del fotograma) con el tipo indicado.
    std::vector<std::byte> payload;
};

// Declara el tipo FrameReader y las operaciones o datos de su contrato.
class FrameReader {
// Expone los miembros de la interfaz.
public:
    // Solicita la implementación predeterminada del compilador.
    virtual ~FrameReader() = default;

    // Declara el atributo de compilación [[nodiscard]].
    [[nodiscard]]
    // Ejecuta virtual FrameReadResult read_next() = 0.
    virtual FrameReadResult read_next() = 0;
};

// Declara el tipo FrameMetadata y las operaciones o datos de su contrato.
struct FrameMetadata final {
    // Inicializa sequence (secuencia del fotograma) con inicialización por valor.
    std::uint64_t sequence{};
    // Inicializa timestamp_100ns (marca de tiempo en unidades de 100 ns) con inicialización por valor.
    std::int64_t timestamp_100ns{};
    // Inicializa width (ancho en píxeles) con inicialización por valor.
    std::uint32_t width{};
    // Inicializa height (alto en píxeles) con inicialización por valor.
    std::uint32_t height{};
    // Inicializa stride (bytes entre filas de imagen) con inicialización por valor.
    std::uint32_t stride{};
    // Declara pixel_format (formato de píxeles) con el tipo indicado.
    std::string pixel_format;
    // Inicializa byte_count con inicialización por valor.
    std::size_t byte_count{};
};

// Declara el tipo FrameSink y las operaciones o datos de su contrato.
class FrameSink {
// Expone los miembros de la interfaz.
public:
    // Solicita la implementación predeterminada del compilador.
    virtual ~FrameSink() = default;

    // Define la operación publish con los parámetros declarados.
    virtual void publish(
        // Declara el parámetro metadata.
        const FrameMetadata& metadata,
        // Incorpora std::span<const std::byte> payload en esta operación.
        std::span<const std::byte> payload
    // Incorpora ) = 0 en esta operación.
    ) = 0;

    // Consumers never stop Capture merely by disconnecting. This is an
    // explicit lifecycle request made by the owner of a desktop session.
    // Declara el atributo de compilación [[nodiscard]].
    [[nodiscard]]
    // Ejecuta virtual bool stop_requested() const noexcept { return false; }.
    virtual bool stop_requested() const noexcept { return false; }
};

// Declara el tipo CaptureSummary y las operaciones o datos de su contrato.
struct CaptureSummary final {
    // Inicializa requested_frames con inicialización por valor.
    std::size_t requested_frames{};
    // Inicializa captured_frames con inicialización por valor.
    std::size_t captured_frames{};
    // Inicializa first_sequence con inicialización por valor.
    std::uint64_t first_sequence{};
    // Inicializa last_sequence (última secuencia observada) con inicialización por valor.
    std::uint64_t last_sequence{};
    // Inicializa first_timestamp_100ns con inicialización por valor.
    std::int64_t first_timestamp_100ns{};
    // Inicializa last_timestamp_100ns con inicialización por valor.
    std::int64_t last_timestamp_100ns{};
    // Inicializa total_bytes con inicialización por valor.
    std::size_t total_bytes{};
    // Inicializa empty_reads con inicialización por valor.
    std::size_t empty_reads{};
    // Inicializa timestamps_strictly_increasing con true.
    bool timestamps_strictly_increasing{true};
    // Declara frames (fotogramas) con el tipo indicado.
    std::vector<FrameMetadata> frames;

    // Declara el atributo de compilación [[nodiscard]].
    [[nodiscard]]
    // Ejecuta double duration_seconds() const noexcept.
    double duration_seconds() const noexcept;

    // Declara el atributo de compilación [[nodiscard]].
    [[nodiscard]]
    // Ejecuta double capture_fps() const noexcept.
    double capture_fps() const noexcept;
};

// Declara el tipo ContinuousFrameCapture y las operaciones o datos de su contrato.
class ContinuousFrameCapture final {
// Expone los miembros de la interfaz.
public:
    // Declara el atributo de compilación [[nodiscard]].
    [[nodiscard]]
    // Define la operación run con los parámetros declarados.
    static CaptureSummary run(
        // Declara el parámetro reader (lector de fotogramas).
        FrameReader& reader,
        // Declara el parámetro requested_frames.
        std::size_t requested_frames,
        // Declara el parámetro format (formato de video).
        const VideoFormat& format,
        // Establece maximum_empty_reads a partir de 1000.
        std::size_t maximum_empty_reads = 1000,
        // Establece sink a partir de nullptr.
        FrameSink* sink = nullptr
    );
};

// Incorpora } // namespace orvix::capture en esta operación.
}  // namespace orvix::capture
