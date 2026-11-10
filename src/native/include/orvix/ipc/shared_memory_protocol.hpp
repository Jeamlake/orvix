// Aplica la directiva del compilador once.
#pragma once

// Incluye las declaraciones de video format de ORVIX.
#include "orvix/capture/video_format.hpp"

// Incluye cstddef para usar tipos de tamaño y bytes.
#include <cstddef>
// Incluye cstdint para usar enteros de tamaño fijo.
#include <cstdint>
// Incluye string_view para usar vistas de cadenas.
#include <string_view>

// Agrupa las declaraciones bajo orvix::ipc.
namespace orvix::ipc {

// Establece kDefaultSharedMemoryName a partir de "orvix-camera-v1".
inline constexpr std::string_view kDefaultSharedMemoryName =
    // Incorpora "orvix-camera-v1" en esta operación.
    "orvix-camera-v1";
// Establece kProtocolVersion a partir de 1.
inline constexpr std::uint16_t kProtocolVersion = 1;
// Establece kGlobalHeaderSize a partir de 128.
inline constexpr std::size_t kGlobalHeaderSize = 128;
// Establece kSlotHeaderSize a partir de 64.
inline constexpr std::size_t kSlotHeaderSize = 64;
// Establece kDefaultSlotCount a partir de 3.
inline constexpr std::uint32_t kDefaultSlotCount = 3;

// Declara el tipo PixelFormat y las operaciones o datos de su contrato.
enum class PixelFormat : std::uint32_t {
    // Establece unknown a partir de 0.
    unknown = 0,
    // Establece nv12 a partir de 1.
    nv12 = 1,
    // Establece yuy2 a partir de 2.
    yuy2 = 2,
    // Establece uyvy a partir de 3.
    uyvy = 3,
    // Establece i420 a partir de 4.
    i420 = 4,
    // Establece yv12 a partir de 5.
    yv12 = 5,
    // Establece rgb24 a partir de 6.
    rgb24 = 6,
    // Establece bgra32 a partir de 7.
    bgra32 = 7,
    // Establece argb32 a partir de 8.
    argb32 = 8,
    // Establece mjpg a partir de 9.
    mjpg = 9,
    // Establece h264 a partir de 10.
    h264 = 10
};

// Declara el tipo ProducerState y las operaciones o datos de su contrato.
enum class ProducerState : std::uint32_t {
    // Establece initializing a partir de 0.
    initializing = 0,
    // Establece running a partir de 1.
    running = 1,
    // Establece stopped a partir de 2.
    stopped = 2,
    // Establece failed a partir de 3.
    failed = 3
};

// Declara el tipo SlotState y las operaciones o datos de su contrato.
enum class SlotState : std::uint32_t {
    // Establece empty a partir de 0.
    empty = 0,
    // Establece writing a partir de 1.
    writing = 1,
    // Establece ready a partir de 2.
    ready = 2
};

// Agrupa las declaraciones bajo header_offset.
namespace header_offset {
// Establece magic a partir de 0.
inline constexpr std::size_t magic = 0;
// Establece version a partir de 4.
inline constexpr std::size_t version = 4;
// Establece header_size a partir de 6.
inline constexpr std::size_t header_size = 6;
// Establece total_size a partir de 8.
inline constexpr std::size_t total_size = 8;
// Establece slot_count (cantidad de posiciones del anillo) a partir de 16.
inline constexpr std::size_t slot_count = 16;
// Establece slot_header_size a partir de 20.
inline constexpr std::size_t slot_header_size = 20;
// Establece payload_capacity a partir de 24.
inline constexpr std::size_t payload_capacity = 24;
// Establece slot_stride a partir de 32.
inline constexpr std::size_t slot_stride = 32;
// Establece width (ancho en píxeles) a partir de 40.
inline constexpr std::size_t width = 40;
// Establece height (alto en píxeles) a partir de 44.
inline constexpr std::size_t height = 44;
// Establece frame_stride a partir de 48.
inline constexpr std::size_t frame_stride = 48;
// Establece pixel_format (formato de píxeles) a partir de 52.
inline constexpr std::size_t pixel_format = 52;
// Establece fps_numerator a partir de 56.
inline constexpr std::size_t fps_numerator = 56;
// Establece fps_denominator a partir de 60.
inline constexpr std::size_t fps_denominator = 60;
// Establece created_timestamp_100ns a partir de 64.
inline constexpr std::size_t created_timestamp_100ns = 64;
// Establece producer_pid a partir de 72.
inline constexpr std::size_t producer_pid = 72;
// Establece producer_state a partir de 80.
inline constexpr std::size_t producer_state = 80;
// Establece published_slot (slot publicado) a partir de 84.
inline constexpr std::size_t published_slot = 84;
// Establece published_sequence (última secuencia publicada) a partir de 88.
inline constexpr std::size_t published_sequence = 88;
// Establece consumed_sequence a partir de 96.
inline constexpr std::size_t consumed_sequence = 96;
// Establece overwritten_frames a partir de 104.
inline constexpr std::size_t overwritten_frames = 104;
// Establece producer_fps_x1000 a partir de 112.
inline constexpr std::size_t producer_fps_x1000 = 112;
// Establece stop_request a partir de 120.
inline constexpr std::size_t stop_request = 120;
// Incorpora } // namespace header_offset en esta operación.
}  // namespace header_offset

// Agrupa las declaraciones bajo slot_offset.
namespace slot_offset {
// Establece generation a partir de 0.
inline constexpr std::size_t generation = 0;
// Establece sequence (secuencia del fotograma) a partir de 8.
inline constexpr std::size_t sequence = 8;
// Establece timestamp_100ns (marca de tiempo en unidades de 100 ns) a partir de 16.
inline constexpr std::size_t timestamp_100ns = 16;
// Establece payload_size (tamaño de la carga útil) a partir de 24.
inline constexpr std::size_t payload_size = 24;
// Establece width (ancho en píxeles) a partir de 32.
inline constexpr std::size_t width = 32;
// Establece height (alto en píxeles) a partir de 36.
inline constexpr std::size_t height = 36;
// Establece frame_stride a partir de 40.
inline constexpr std::size_t frame_stride = 40;
// Establece pixel_format (formato de píxeles) a partir de 44.
inline constexpr std::size_t pixel_format = 44;
// Establece state (estado) a partir de 48.
inline constexpr std::size_t state = 48;
// Establece payload (carga útil del fotograma) a partir de kSlotHeaderSize.
inline constexpr std::size_t payload = kSlotHeaderSize;
// Incorpora } // namespace slot_offset en esta operación.
}  // namespace slot_offset

// Declara el atributo de compilación [[nodiscard]].
[[nodiscard]]
// Ejecuta PixelFormat pixel_format_code(std::string_view name) noexcept.
PixelFormat pixel_format_code(std::string_view name) noexcept;

// Declara el atributo de compilación [[nodiscard]].
[[nodiscard]]
// Ejecuta std::string_view pixel_format_name(PixelFormat format) noexcept.
std::string_view pixel_format_name(PixelFormat format) noexcept;

// Declara el atributo de compilación [[nodiscard]].
[[nodiscard]]
// Define la operación default_payload_capacity con los parámetros declarados.
std::size_t default_payload_capacity(
    // Incorpora const capture::VideoFormat& format en esta operación.
    const capture::VideoFormat& format
);

// Incorpora } // namespace orvix::ipc en esta operación.
}  // namespace orvix::ipc
