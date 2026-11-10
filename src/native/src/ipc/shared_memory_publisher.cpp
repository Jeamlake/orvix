// Incluye las declaraciones de shared memory publisher de ORVIX.
#include "orvix/ipc/shared_memory_publisher.hpp"

// Incluye las declaraciones de shared memory protocol de ORVIX.
#include "orvix/ipc/shared_memory_protocol.hpp"

// Incluye algorithm para usar algoritmos sobre rangos.
#include <algorithm>
// Incluye array para usar arreglos de tamaño fijo.
#include <array>
// Incluye atomic para usar operaciones atómicas.
#include <atomic>
// Importa las declaraciones de bit para esta unidad.
#include <bit>
// Incluye chrono para usar relojes y duraciones.
#include <chrono>
// Incluye cmath para usar funciones matemáticas.
#include <cmath>
// Incluye cstring para usar operaciones de memoria y cadenas de C.
#include <cstring>
// Incluye limits para usar límites de tipos numéricos.
#include <limits>
// Incluye memory para usar punteros inteligentes y gestión de memoria.
#include <memory>
// Incluye ratio para usar proporciones en tiempo de compilación.
#include <ratio>
// Incluye stdexcept para usar tipos estándar de excepciones.
#include <stdexcept>
// Importa las declaraciones de type_traits para esta unidad.
#include <type_traits>

// Incluye este bloque si se cumple defined(_WIN32).
#if defined(_WIN32)
// Importa las declaraciones de Windows.h para esta unidad.
#include <Windows.h>
// Abre la alternativa de compilación.
#else
// Incluye unistd.h para usar operaciones POSIX.
#include <unistd.h>
// Cierra la condición de compilación.
#endif

// Agrupa las declaraciones bajo orvix::ipc.
namespace orvix::ipc {

// Limita estas declaraciones a esta unidad de compilación.
namespace {

// Comprueba en compilación std::endian::native == std::endian::little.
static_assert(std::endian::native == std::endian::little);

// Declara los parámetros genéricos template<typename Value>.
template<typename Value>
// Define la operación write_value con los parámetros declarados.
void write_value(
    // Declara el parámetro data.
    std::byte* const data,
    // Declara el parámetro offset.
    const std::size_t offset,
    // Incorpora const Value value en esta operación.
    const Value value
// Incorpora ) noexcept { en esta operación.
) noexcept {
    // Comprueba en compilación std::is_trivially_copyable_v<Value>.
    static_assert(std::is_trivially_copyable_v<Value>);
    // Copia bytes entre dos regiones de memoria mediante std::memcpy.
    std::memcpy(data + offset, &value, sizeof(Value));
}

// Declara los parámetros genéricos template<typename Value>.
template<typename Value>
// Define la operación read_value con los parámetros declarados.
Value read_value(
    // Declara el parámetro data.
    const std::byte* const data,
    // Incorpora const std::size_t offset en esta operación.
    const std::size_t offset
// Incorpora ) noexcept { en esta operación.
) noexcept {
    // Inicializa value con inicialización por valor.
    Value value{};
    // Copia bytes entre dos regiones de memoria mediante std::memcpy.
    std::memcpy(&value, data + offset, sizeof(Value));
    // Devuelve value.
    return value;
}

// Define la operación uint64_at con los parámetros declarados.
std::uint64_t& uint64_at(
    // Declara el parámetro data.
    std::byte* const data,
    // Incorpora const std::size_t offset en esta operación.
    const std::size_t offset
// Incorpora ) noexcept { en esta operación.
) noexcept {
    // Devuelve *reinterpret_cast<std::uint64_t*>(data + offset).
    return *reinterpret_cast<std::uint64_t*>(data + offset);
}

// Define la operación uint32_at con los parámetros declarados.
std::uint32_t& uint32_at(
    // Declara el parámetro data.
    std::byte* const data,
    // Incorpora const std::size_t offset en esta operación.
    const std::size_t offset
// Incorpora ) noexcept { en esta operación.
) noexcept {
    // Devuelve *reinterpret_cast<std::uint32_t*>(data + offset).
    return *reinterpret_cast<std::uint32_t*>(data + offset);
}

// Define la operación atomic_load_u64 con los parámetros declarados.
std::uint64_t atomic_load_u64(
    // Declara el parámetro data.
    const std::byte* const data,
    // Incorpora const std::size_t offset en esta operación.
    const std::size_t offset
// Incorpora ) noexcept { en esta operación.
) noexcept {
    // Establece value a partir de *reinterpret_cast<std::uint64_t*>(.
    auto& value = *reinterpret_cast<std::uint64_t*>(
        // Incorpora const_cast<std::byte*>(data) + offset en esta operación.
        const_cast<std::byte*>(data) + offset
    );
    // Devuelve std::atomic_ref<std::uint64_t>(value).load(.
    return std::atomic_ref<std::uint64_t>(value).load(
        // Incorpora std::memory_order_acquire en esta operación.
        std::memory_order_acquire
    );
}

// Define la operación atomic_store_u64 con los parámetros declarados.
void atomic_store_u64(
    // Declara el parámetro data.
    std::byte* const data,
    // Declara el parámetro offset.
    const std::size_t offset,
    // Incorpora const std::uint64_t value en esta operación.
    const std::uint64_t value
// Incorpora ) noexcept { en esta operación.
) noexcept {
    // Publica el valor indicado mediante uint64_at.
    std::atomic_ref<std::uint64_t>(uint64_at(data, offset)).store(
        // Aporta value como elemento de la lista.
        value,
        // Incorpora std::memory_order_release en esta operación.
        std::memory_order_release
    );
}

// Define la operación atomic_store_u32 con los parámetros declarados.
void atomic_store_u32(
    // Declara el parámetro data.
    std::byte* const data,
    // Declara el parámetro offset.
    const std::size_t offset,
    // Incorpora const std::uint32_t value en esta operación.
    const std::uint32_t value
// Incorpora ) noexcept { en esta operación.
) noexcept {
    // Publica el valor indicado mediante uint32_at.
    std::atomic_ref<std::uint32_t>(uint32_at(data, offset)).store(
        // Aporta value como elemento de la lista.
        value,
        // Incorpora std::memory_order_release en esta operación.
        std::memory_order_release
    );
}

// Define la operación align_up con los parámetros declarados.
std::size_t align_up(
    // Declara el parámetro value.
    const std::size_t value,
    // Incorpora const std::size_t alignment en esta operación.
    const std::size_t alignment
) {
    // Evalúa la condición value > std::numeric_limits<std::size_t>::max() - alignment + 1U.
    if (value > std::numeric_limits<std::size_t>::max() - alignment + 1U) {
        // Interrumpe la operación con std::overflow_error("Shared-memory alignment overflowed.").
        throw std::overflow_error("Shared-memory alignment overflowed.");
    }
    // Devuelve (value + alignment - 1U) / alignment * alignment.
    return (value + alignment - 1U) / alignment * alignment;
}

// Define la operación checked_multiply con los parámetros declarados.
std::size_t checked_multiply(
    // Declara el parámetro left.
    const std::size_t left,
    // Incorpora const std::size_t right en esta operación.
    const std::size_t right
) {
    // Evalúa la condición right != 0 &&.
    if (
        // Combina esta condición con right != 0 &&.
        right != 0 &&
        // Ejecuta left > std::numeric_limits<std::size_t>::max() / right.
        left > std::numeric_limits<std::size_t>::max() / right
    ) {
        // Interrumpe la operación con std::overflow_error("Shared-memory size overflowed.").
        throw std::overflow_error("Shared-memory size overflowed.");
    }
    // Devuelve left * right.
    return left * right;
}

// Define la operación process_id con los parámetros declarados.
std::uint64_t process_id() noexcept {
// Incluye este bloque si se cumple defined(_WIN32).
#if defined(_WIN32)
    // Devuelve static_cast<std::uint64_t>(GetCurrentProcessId()).
    return static_cast<std::uint64_t>(GetCurrentProcessId());
// Abre la alternativa de compilación.
#else
    // Devuelve static_cast<std::uint64_t>(::getpid()).
    return static_cast<std::uint64_t>(::getpid());
// Cierra la condición de compilación.
#endif
}

// Define la operación timestamp_100ns_now con los parámetros declarados.
std::int64_t timestamp_100ns_now() noexcept {
    // Devuelve std::chrono::duration_cast<.
    return std::chrono::duration_cast<
        // Incorpora std::chrono::duration<std::int64_t, std::ratio<1, 10'000'000>> en esta operación.
        std::chrono::duration<std::int64_t, std::ratio<1, 10'000'000>>
    // Ejecuta >(std::chrono::system_clock::now().time_since_epoch()).count().
    >(std::chrono::system_clock::now().time_since_epoch()).count();
}

// Incorpora } // namespace en esta operación.
}  // namespace

// Define la operación pixel_format_code con los parámetros declarados.
PixelFormat pixel_format_code(const std::string_view name) noexcept {
    // Evalúa la condición name == "NV12") return PixelFormat::nv12.
    if (name == "NV12") return PixelFormat::nv12;
    // Evalúa la condición name == "YUY2") return PixelFormat::yuy2.
    if (name == "YUY2") return PixelFormat::yuy2;
    // Evalúa la condición name == "UYVY") return PixelFormat::uyvy.
    if (name == "UYVY") return PixelFormat::uyvy;
    // Evalúa la condición name == "I420" || name == "IYUV") return PixelFormat::i420.
    if (name == "I420" || name == "IYUV") return PixelFormat::i420;
    // Evalúa la condición name == "YV12") return PixelFormat::yv12.
    if (name == "YV12") return PixelFormat::yv12;
    // Evalúa la condición name == "RGB24") return PixelFormat::rgb24.
    if (name == "RGB24") return PixelFormat::rgb24;
    // Evalúa la condición name == "RGB32" || name == "BGRA32") return PixelFormat::bgra32.
    if (name == "RGB32" || name == "BGRA32") return PixelFormat::bgra32;
    // Evalúa la condición name == "ARGB32") return PixelFormat::argb32.
    if (name == "ARGB32") return PixelFormat::argb32;
    // Evalúa la condición name == "MJPG") return PixelFormat::mjpg.
    if (name == "MJPG") return PixelFormat::mjpg;
    // Evalúa la condición name == "H264") return PixelFormat::h264.
    if (name == "H264") return PixelFormat::h264;
    // Devuelve PixelFormat::unknown.
    return PixelFormat::unknown;
}

// Define la operación pixel_format_name con los parámetros declarados.
std::string_view pixel_format_name(const PixelFormat format) noexcept {
    // Selecciona una rama según format.
    switch (format) {
    // Selecciona el caso PixelFormat::nv12: return "NV12";.
    case PixelFormat::nv12: return "NV12";
    // Selecciona el caso PixelFormat::yuy2: return "YUY2";.
    case PixelFormat::yuy2: return "YUY2";
    // Selecciona el caso PixelFormat::uyvy: return "UYVY";.
    case PixelFormat::uyvy: return "UYVY";
    // Selecciona el caso PixelFormat::i420: return "I420";.
    case PixelFormat::i420: return "I420";
    // Selecciona el caso PixelFormat::yv12: return "YV12";.
    case PixelFormat::yv12: return "YV12";
    // Selecciona el caso PixelFormat::rgb24: return "RGB24";.
    case PixelFormat::rgb24: return "RGB24";
    // Selecciona el caso PixelFormat::bgra32: return "BGRA32";.
    case PixelFormat::bgra32: return "BGRA32";
    // Selecciona el caso PixelFormat::argb32: return "ARGB32";.
    case PixelFormat::argb32: return "ARGB32";
    // Selecciona el caso PixelFormat::mjpg: return "MJPG";.
    case PixelFormat::mjpg: return "MJPG";
    // Selecciona el caso PixelFormat::h264: return "H264";.
    case PixelFormat::h264: return "H264";
    // Atiende los valores que no coinciden con los casos anteriores.
    default: return "UNKNOWN";
    }
}

// Define la operación default_payload_capacity con los parámetros declarados.
std::size_t default_payload_capacity(const capture::VideoFormat& format) {
    // Evalúa la condición format.width == 0 || format.height == 0.
    if (format.width == 0 || format.height == 0) {
        // Interrumpe la operación con std::invalid_argument(.
        throw std::invalid_argument(
            // Aporta el texto "Shared memory requires non-zero frame dimensions." a la expresión.
            "Shared memory requires non-zero frame dimensions."
        );
    }
    // Establece pixels a partir de checked_multiply(format.width, format.height).
    const std::size_t pixels = checked_multiply(format.width, format.height);
    // Devuelve checked_multiply(pixels, 4U).
    return checked_multiply(pixels, 4U);
}

// Ejecuta SharedMemoryPublisher::SharedMemoryPublisher(.
SharedMemoryPublisher::SharedMemoryPublisher(
    // Declara el parámetro name.
    std::string name,
    // Declara el parámetro format (formato de video).
    const capture::VideoFormat& format,
    // Declara el parámetro slot_count (cantidad de posiciones del anillo).
    const std::uint32_t slot_count,
    // Incorpora std::size_t payload_capacity en esta operación.
    std::size_t payload_capacity
)
    // Inicializa el miembro indicado por format_(format), antes del cuerpo.
    : format_(format),
      // Define la operación _ con los parámetros declarados.
      slot_count_(slot_count) {
    // Evalúa la condición slot_count < 2 || slot_count > 64.
    if (slot_count < 2 || slot_count > 64) {
        // Interrumpe la operación con std::invalid_argument(.
        throw std::invalid_argument(
            // Aporta el texto "The shared-memory ring requires between 2 and 64 slots." a la expresión.
            "The shared-memory ring requires between 2 and 64 slots."
        );
    }
    // Evalúa la condición format.frame_rate_denominator == 0.
    if (format.frame_rate_denominator == 0) {
        // Interrumpe la operación con std::invalid_argument("Frame-rate denominator cannot be zero.").
        throw std::invalid_argument("Frame-rate denominator cannot be zero.");
    }
    // Evalúa la condición payload_capacity == 0.
    if (payload_capacity == 0) {
        // Establece payload_capacity a partir de default_payload_capacity(format).
        payload_capacity = default_payload_capacity(format);
    }
    // Establece payload_capacity_ a partir de payload_capacity.
    payload_capacity_ = payload_capacity;
    // Establece slot_stride_ a partir de align_up(kSlotHeaderSize + payload_capacity_, 64U).
    slot_stride_ = align_up(kSlotHeaderSize + payload_capacity_, 64U);
    // Establece total_size a partir de kGlobalHeaderSize + checked_multiply(.
    const std::size_t total_size = kGlobalHeaderSize + checked_multiply(
        // Aporta slot_stride_ como elemento de la lista.
        slot_stride_,
        // Incorpora slot_count_ en esta operación.
        slot_count_
    );
    // Establece region_ a partir de std::make_unique<SharedMemoryRegion>(.
    region_ = std::make_unique<SharedMemoryRegion>(
        // Transfiere el valor sin exigir una copia mediante std::move.
        std::move(name),
        // Incorpora total_size en esta operación.
        total_size
    );

    // Establece data a partir de region_->data().
    std::byte* const data = region_->data();
    // Ejecuta std::memset(data, 0, total_size).
    std::memset(data, 0, total_size);
    // Ejecuta std::construct_at(.
    std::construct_at(
        // Incorpora reinterpret_cast<std::uint32_t*>( en esta operación.
        reinterpret_cast<std::uint32_t*>(
            // Incorpora data + header_offset::producer_state en esta operación.
            data + header_offset::producer_state
        ),
        // Incorpora static_cast<std::uint32_t>(ProducerState::initializing) en esta operación.
        static_cast<std::uint32_t>(ProducerState::initializing)
    );
    // Ejecuta std::construct_at(.
    std::construct_at(
        // Incorpora reinterpret_cast<std::uint32_t*>( en esta operación.
        reinterpret_cast<std::uint32_t*>(
            // Incorpora data + header_offset::published_slot en esta operación.
            data + header_offset::published_slot
        ),
        // Incorpora 0U en esta operación.
        0U
    );
    // Recorre según std::uint32_t slot = 0; slot < slot_count_; ++slot.
    for (std::uint32_t slot = 0; slot < slot_count_; ++slot) {
        // Ejecuta std::construct_at(.
        std::construct_at(
            // Incorpora reinterpret_cast<std::uint64_t*>( en esta operación.
            reinterpret_cast<std::uint64_t*>(
                // Incorpora data + kGlobalHeaderSize + en esta operación.
                data + kGlobalHeaderSize +
                // Incorpora static_cast<std::size_t>(slot) * slot_stride_ + en esta operación.
                static_cast<std::size_t>(slot) * slot_stride_ +
                // Incorpora slot_offset::generation en esta operación.
                slot_offset::generation
            ),
            // Incorpora 0ULL en esta operación.
            0ULL
        );
    }

    // Inicializa magic con 'O', 'R', 'V', 'X'.
    const std::array<char, 4> magic{'O', 'R', 'V', 'X'};
    // Copia bytes entre dos regiones de memoria mediante std::memcpy.
    std::memcpy(data + header_offset::magic, magic.data(), magic.size());
    // Incorpora write_value<std::uint16_t>(data, header_offset::version, kProtocolVersion) en esta operación.
    write_value<std::uint16_t>(data, header_offset::version, kProtocolVersion);
    // Incorpora write_value<std::uint16_t>( en esta operación.
    write_value<std::uint16_t>(
        // Aporta data como elemento de la lista.
        data,
        // Aporta header_offset::header_size como elemento de la lista.
        header_offset::header_size,
        // Incorpora static_cast<std::uint16_t>(kGlobalHeaderSize) en esta operación.
        static_cast<std::uint16_t>(kGlobalHeaderSize)
    );
    // Incorpora write_value<std::uint64_t>(data, header_offset::total_size, total_size) en esta operación.
    write_value<std::uint64_t>(data, header_offset::total_size, total_size);
    // Incorpora write_value<std::uint32_t>(data, header_offset::slot_count, slot_count_) en esta operación.
    write_value<std::uint32_t>(data, header_offset::slot_count, slot_count_);
    // Incorpora write_value<std::uint32_t>( en esta operación.
    write_value<std::uint32_t>(
        // Aporta data como elemento de la lista.
        data,
        // Aporta header_offset::slot_header_size como elemento de la lista.
        header_offset::slot_header_size,
        // Incorpora static_cast<std::uint32_t>(kSlotHeaderSize) en esta operación.
        static_cast<std::uint32_t>(kSlotHeaderSize)
    );
    // Incorpora write_value<std::uint64_t>( en esta operación.
    write_value<std::uint64_t>(
        // Aporta data como elemento de la lista.
        data,
        // Aporta header_offset::payload_capacity como elemento de la lista.
        header_offset::payload_capacity,
        // Incorpora payload_capacity_ en esta operación.
        payload_capacity_
    );
    // Incorpora write_value<std::uint64_t>(data, header_offset::slot_stride, slot_stride_) en esta operación.
    write_value<std::uint64_t>(data, header_offset::slot_stride, slot_stride_);
    // Incorpora write_value<std::uint32_t>(data, header_offset::width, format.width) en esta operación.
    write_value<std::uint32_t>(data, header_offset::width, format.width);
    // Incorpora write_value<std::uint32_t>(data, header_offset::height, format.height) en esta operación.
    write_value<std::uint32_t>(data, header_offset::height, format.height);
    // Incorpora write_value<std::uint32_t>( en esta operación.
    write_value<std::uint32_t>(
        // Aporta data como elemento de la lista.
        data,
        // Aporta header_offset::frame_stride como elemento de la lista.
        header_offset::frame_stride,
        // Incorpora format.stride en esta operación.
        format.stride
    );
    // Incorpora write_value<std::uint32_t>( en esta operación.
    write_value<std::uint32_t>(
        // Aporta data como elemento de la lista.
        data,
        // Aporta header_offset::pixel_format como elemento de la lista.
        header_offset::pixel_format,
        // Ejecuta static_cast<std::uint32_t>(pixel_format_code(format.pixel_format)).
        static_cast<std::uint32_t>(pixel_format_code(format.pixel_format))
    );
    // Incorpora write_value<std::uint32_t>( en esta operación.
    write_value<std::uint32_t>(
        // Aporta data como elemento de la lista.
        data,
        // Aporta header_offset::fps_numerator como elemento de la lista.
        header_offset::fps_numerator,
        // Incorpora format.frame_rate_numerator en esta operación.
        format.frame_rate_numerator
    );
    // Incorpora write_value<std::uint32_t>( en esta operación.
    write_value<std::uint32_t>(
        // Aporta data como elemento de la lista.
        data,
        // Aporta header_offset::fps_denominator como elemento de la lista.
        header_offset::fps_denominator,
        // Incorpora format.frame_rate_denominator en esta operación.
        format.frame_rate_denominator
    );
    // Incorpora write_value<std::int64_t>( en esta operación.
    write_value<std::int64_t>(
        // Aporta data como elemento de la lista.
        data,
        // Aporta header_offset::created_timestamp_100ns como elemento de la lista.
        header_offset::created_timestamp_100ns,
        // Ejecuta timestamp_100ns_now().
        timestamp_100ns_now()
    );
    // Incorpora write_value<std::uint64_t>( en esta operación.
    write_value<std::uint64_t>(
        // Aporta data como elemento de la lista.
        data,
        // Aporta header_offset::producer_pid como elemento de la lista.
        header_offset::producer_pid,
        // Ejecuta process_id().
        process_id()
    );
    // Ejecuta std::construct_at(.
    std::construct_at(
        // Incorpora reinterpret_cast<std::uint64_t*>( en esta operación.
        reinterpret_cast<std::uint64_t*>(
            // Incorpora data + header_offset::published_sequence en esta operación.
            data + header_offset::published_sequence
        ),
        // Incorpora 0ULL en esta operación.
        0ULL
    );
    // Ejecuta std::construct_at(.
    std::construct_at(
        // Incorpora reinterpret_cast<std::uint64_t*>( en esta operación.
        reinterpret_cast<std::uint64_t*>(
            // Incorpora data + header_offset::consumed_sequence en esta operación.
            data + header_offset::consumed_sequence
        ),
        // Incorpora 0ULL en esta operación.
        0ULL
    );
    // Ejecuta std::construct_at(.
    std::construct_at(
        // Incorpora reinterpret_cast<std::uint64_t*>( en esta operación.
        reinterpret_cast<std::uint64_t*>(
            // Incorpora data + header_offset::overwritten_frames en esta operación.
            data + header_offset::overwritten_frames
        ),
        // Incorpora 0ULL en esta operación.
        0ULL
    );
    // Ejecuta std::construct_at(.
    std::construct_at(
        // Incorpora reinterpret_cast<std::uint64_t*>( en esta operación.
        reinterpret_cast<std::uint64_t*>(
            // Incorpora data + header_offset::producer_fps_x1000 en esta operación.
            data + header_offset::producer_fps_x1000
        ),
        // Incorpora 0ULL en esta operación.
        0ULL
    );
    // Ejecuta std::construct_at(reinterpret_cast<std::uint32_t*>(.
    std::construct_at(reinterpret_cast<std::uint32_t*>(
        // Incorpora data + header_offset::stop_request), 0U) en esta operación.
        data + header_offset::stop_request), 0U);
    // Ejecuta atomic_store_u32(.
    atomic_store_u32(
        // Aporta data como elemento de la lista.
        data,
        // Aporta header_offset::producer_state como elemento de la lista.
        header_offset::producer_state,
        // Incorpora static_cast<std::uint32_t>(ProducerState::running) en esta operación.
        static_cast<std::uint32_t>(ProducerState::running)
    );
}

// Ejecuta SharedMemoryPublisher::~SharedMemoryPublisher() {.
SharedMemoryPublisher::~SharedMemoryPublisher() {
    // Evalúa la condición !terminal_state_written_.
    if (!terminal_state_written_) {
        // Ejecuta mark_failed().
        mark_failed();
    }
}

// Define la operación SharedMemoryPublisher::publish con los parámetros declarados.
void SharedMemoryPublisher::publish(
    // Declara el parámetro metadata.
    const capture::FrameMetadata& metadata,
    // Incorpora const std::span<const std::byte> payload en esta operación.
    const std::span<const std::byte> payload
) {
    // Evalúa la condición terminal_state_written_.
    if (terminal_state_written_) {
        // Interrumpe la operación con std::logic_error("Cannot publish after the producer stopped.").
        throw std::logic_error("Cannot publish after the producer stopped.");
    }
    // Evalúa la condición metadata.sequence == 0.
    if (metadata.sequence == 0) {
        // Interrumpe la operación con std::invalid_argument("Published frame sequence must be positive.").
        throw std::invalid_argument("Published frame sequence must be positive.");
    }
    // Evalúa la condición payload.size() != metadata.byte_count.
    if (payload.size() != metadata.byte_count) {
        // Interrumpe la operación con std::invalid_argument(.
        throw std::invalid_argument(
            // Aporta el texto "Published payload size does not match frame metadata." a la expresión.
            "Published payload size does not match frame metadata."
        );
    }
    // Evalúa la condición payload.size() > payload_capacity_.
    if (payload.size() > payload_capacity_) {
        // Interrumpe la operación con std::length_error(.
        throw std::length_error(
            // Aporta el texto "Captured frame exceeds the shared-memory slot capacity." a la expresión.
            "Captured frame exceeds the shared-memory slot capacity."
        );
    }

    // Establece data a partir de region_->data().
    std::byte* const data = region_->data();
    // Establece slot_index a partir de static_cast<std::uint32_t>(.
    const std::uint32_t slot_index = static_cast<std::uint32_t>(
        // Incorpora (metadata.sequence - 1U) % slot_count_ en esta operación.
        (metadata.sequence - 1U) % slot_count_
    );
    // Establece slot (posición del anillo) a partir de data + kGlobalHeaderSize +.
    std::byte* const slot = data + kGlobalHeaderSize +
        // Incorpora static_cast<std::size_t>(slot_index) * slot_stride_ en esta operación.
        static_cast<std::size_t>(slot_index) * slot_stride_;
    // Establece generation a partir de std::atomic_ref<std::uint64_t>(.
    auto generation = std::atomic_ref<std::uint64_t>(
        // Ejecuta uint64_at(slot, slot_offset::generation).
        uint64_at(slot, slot_offset::generation)
    );
    // Establece current_generation a partir de generation.load(.
    std::uint64_t current_generation = generation.load(
        // Incorpora std::memory_order_relaxed en esta operación.
        std::memory_order_relaxed
    );
    // Evalúa la condición (current_generation & 1U) != 0U.
    if ((current_generation & 1U) != 0U) {
        // Incrementa current_generation en una unidad.
        ++current_generation;
    }
    // Publica el valor indicado mediante generation.store.
    generation.store(current_generation + 1U, std::memory_order_release);
    // Incorpora write_value<std::uint32_t>( en esta operación.
    write_value<std::uint32_t>(
        // Aporta slot como elemento de la lista.
        slot,
        // Aporta slot_offset::state como elemento de la lista.
        slot_offset::state,
        // Incorpora static_cast<std::uint32_t>(SlotState::writing) en esta operación.
        static_cast<std::uint32_t>(SlotState::writing)
    );
    // Incorpora write_value<std::uint64_t>( en esta operación.
    write_value<std::uint64_t>(
        // Aporta slot como elemento de la lista.
        slot,
        // Aporta slot_offset::sequence como elemento de la lista.
        slot_offset::sequence,
        // Incorpora metadata.sequence en esta operación.
        metadata.sequence
    );
    // Incorpora write_value<std::int64_t>( en esta operación.
    write_value<std::int64_t>(
        // Aporta slot como elemento de la lista.
        slot,
        // Aporta slot_offset::timestamp_100ns como elemento de la lista.
        slot_offset::timestamp_100ns,
        // Incorpora metadata.timestamp_100ns en esta operación.
        metadata.timestamp_100ns
    );
    // Incorpora write_value<std::uint64_t>( en esta operación.
    write_value<std::uint64_t>(
        // Aporta slot como elemento de la lista.
        slot,
        // Aporta slot_offset::payload_size como elemento de la lista.
        slot_offset::payload_size,
        // Ejecuta payload.size().
        payload.size()
    );
    // Incorpora write_value<std::uint32_t>(slot, slot_offset::width, metadata.width) en esta operación.
    write_value<std::uint32_t>(slot, slot_offset::width, metadata.width);
    // Incorpora write_value<std::uint32_t>(slot, slot_offset::height, metadata.height) en esta operación.
    write_value<std::uint32_t>(slot, slot_offset::height, metadata.height);
    // Incorpora write_value<std::uint32_t>( en esta operación.
    write_value<std::uint32_t>(
        // Aporta slot como elemento de la lista.
        slot,
        // Aporta slot_offset::frame_stride como elemento de la lista.
        slot_offset::frame_stride,
        // Incorpora metadata.stride en esta operación.
        metadata.stride
    );
    // Incorpora write_value<std::uint32_t>( en esta operación.
    write_value<std::uint32_t>(
        // Aporta slot como elemento de la lista.
        slot,
        // Aporta slot_offset::pixel_format como elemento de la lista.
        slot_offset::pixel_format,
        // Ejecuta static_cast<std::uint32_t>(pixel_format_code(metadata.pixel_format)).
        static_cast<std::uint32_t>(pixel_format_code(metadata.pixel_format))
    );
    // Evalúa la condición !payload.empty().
    if (!payload.empty()) {
        // Copia bytes entre dos regiones de memoria mediante std::memcpy.
        std::memcpy(slot + slot_offset::payload, payload.data(), payload.size());
    }
    // Incorpora write_value<std::uint32_t>( en esta operación.
    write_value<std::uint32_t>(
        // Aporta slot como elemento de la lista.
        slot,
        // Aporta slot_offset::state como elemento de la lista.
        slot_offset::state,
        // Incorpora static_cast<std::uint32_t>(SlotState::ready) en esta operación.
        static_cast<std::uint32_t>(SlotState::ready)
    );
    // Publica el valor indicado mediante generation.store.
    generation.store(current_generation + 2U, std::memory_order_release);

    // Ejecuta atomic_store_u32(data, header_offset::published_slot, slot_index).
    atomic_store_u32(data, header_offset::published_slot, slot_index);
    // Ejecuta atomic_store_u64(.
    atomic_store_u64(
        // Aporta data como elemento de la lista.
        data,
        // Aporta header_offset::published_sequence como elemento de la lista.
        header_offset::published_sequence,
        // Incorpora metadata.sequence en esta operación.
        metadata.sequence
    );

    // Establece consumed a partir de atomic_load_u64(.
    const std::uint64_t consumed = atomic_load_u64(
        // Aporta data como elemento de la lista.
        data,
        // Incorpora header_offset::consumed_sequence en esta operación.
        header_offset::consumed_sequence
    );
    // Evalúa la condición metadata.sequence > slot_count_ &&.
    if (
        // Combina esta condición con metadata.sequence > slot_count_ &&.
        metadata.sequence > slot_count_ &&
        // Evalúa metadata.sequence - consumed > slot_count_ como parte de la condición.
        metadata.sequence - consumed > slot_count_
    ) {
        // Establece overwritten a partir de uint64_at(.
        auto& overwritten = uint64_at(
            // Aporta data como elemento de la lista.
            data,
            // Incorpora header_offset::overwritten_frames en esta operación.
            header_offset::overwritten_frames
        );
        // Ejecuta std::atomic_ref<std::uint64_t>(overwritten).fetch_add(.
        std::atomic_ref<std::uint64_t>(overwritten).fetch_add(
            // Aporta 1U como elemento de la lista.
            1U,
            // Incorpora std::memory_order_relaxed en esta operación.
            std::memory_order_relaxed
        );
    }

    // Evalúa la condición first_sequence_ == 0.
    if (first_sequence_ == 0) {
        // Establece first_sequence_ a partir de metadata.sequence.
        first_sequence_ = metadata.sequence;
        // Establece first_timestamp_100ns_ a partir de metadata.timestamp_100ns.
        first_timestamp_100ns_ = metadata.timestamp_100ns;
    }
    // Evalúa la alternativa else if (metadata.timestamp_100ns > first_timestamp_100ns_) {.
    else if (metadata.timestamp_100ns > first_timestamp_100ns_) {
        // Establece elapsed (tiempo transcurrido) a partir de static_cast<double>(.
        const double elapsed = static_cast<double>(
            // Incorpora metadata.timestamp_100ns - first_timestamp_100ns_ en esta operación.
            metadata.timestamp_100ns - first_timestamp_100ns_
        // Incorpora ) / 10'000'000.0 en esta operación.
        ) / 10'000'000.0;
        // Establece fps a partir de static_cast<double>(.
        const double fps = static_cast<double>(
            // Incorpora metadata.sequence - first_sequence_ en esta operación.
            metadata.sequence - first_sequence_
        // Incorpora ) / elapsed en esta operación.
        ) / elapsed;
        // Ejecuta atomic_store_u64(.
        atomic_store_u64(
            // Aporta data como elemento de la lista.
            data,
            // Aporta header_offset::producer_fps_x1000 como elemento de la lista.
            header_offset::producer_fps_x1000,
            // Ejecuta static_cast<std::uint64_t>(std::llround(fps * 1000.0)).
            static_cast<std::uint64_t>(std::llround(fps * 1000.0))
        );
    }
}

// Define la operación SharedMemoryPublisher::stop_requested con los parámetros declarados.
bool SharedMemoryPublisher::stop_requested() const noexcept {
    // Establece value a partir de *reinterpret_cast<std::uint32_t*>(.
    auto& value = *reinterpret_cast<std::uint32_t*>(
        // Ejecuta region_->data() + header_offset::stop_request).
        region_->data() + header_offset::stop_request);
    // Devuelve std::atomic_ref<std::uint32_t>(value).load(.
    return std::atomic_ref<std::uint32_t>(value).load(
        // Evalúa std::memory_order_acquire) != 0U como parte de la condición.
        std::memory_order_acquire) != 0U;
}

// Define la operación SharedMemoryPublisher::mark_completed con los parámetros declarados.
void SharedMemoryPublisher::mark_completed() noexcept {
    // Evalúa la condición region_ != nullptr.
    if (region_ != nullptr) {
        // Ejecuta atomic_store_u32(.
        atomic_store_u32(
            // Ejecuta region_->data(),.
            region_->data(),
            // Aporta header_offset::producer_state como elemento de la lista.
            header_offset::producer_state,
            // Incorpora static_cast<std::uint32_t>(ProducerState::stopped) en esta operación.
            static_cast<std::uint32_t>(ProducerState::stopped)
        );
    }
    // Establece terminal_state_written_ a partir de true.
    terminal_state_written_ = true;
}

// Define la operación SharedMemoryPublisher::mark_failed con los parámetros declarados.
void SharedMemoryPublisher::mark_failed() noexcept {
    // Evalúa la condición region_ != nullptr.
    if (region_ != nullptr) {
        // Ejecuta atomic_store_u32(.
        atomic_store_u32(
            // Ejecuta region_->data(),.
            region_->data(),
            // Aporta header_offset::producer_state como elemento de la lista.
            header_offset::producer_state,
            // Incorpora static_cast<std::uint32_t>(ProducerState::failed) en esta operación.
            static_cast<std::uint32_t>(ProducerState::failed)
        );
    }
    // Establece terminal_state_written_ a partir de true.
    terminal_state_written_ = true;
}

// Define la operación SharedMemoryPublisher::stats con los parámetros declarados.
PublisherStats SharedMemoryPublisher::stats() const noexcept {
    // Establece data a partir de region_->data().
    const std::byte* const data = region_->data();
    // Declara result con el tipo indicado.
    PublisherStats result;
    // Establece result.published_sequence (última secuencia publicada) a partir de atomic_load_u64(.
    result.published_sequence = atomic_load_u64(
        // Aporta data como elemento de la lista.
        data,
        // Incorpora header_offset::published_sequence en esta operación.
        header_offset::published_sequence
    );
    // Establece result.consumed_sequence a partir de atomic_load_u64(.
    result.consumed_sequence = atomic_load_u64(
        // Aporta data como elemento de la lista.
        data,
        // Incorpora header_offset::consumed_sequence en esta operación.
        header_offset::consumed_sequence
    );
    // Establece result.overwritten_frames a partir de atomic_load_u64(.
    result.overwritten_frames = atomic_load_u64(
        // Aporta data como elemento de la lista.
        data,
        // Incorpora header_offset::overwritten_frames en esta operación.
        header_offset::overwritten_frames
    );
    // Establece result.producer_fps a partir de static_cast<double>(atomic_load_u64(.
    result.producer_fps = static_cast<double>(atomic_load_u64(
        // Aporta data como elemento de la lista.
        data,
        // Incorpora header_offset::producer_fps_x1000 en esta operación.
        header_offset::producer_fps_x1000
    // Incorpora )) / 1000.0 en esta operación.
    )) / 1000.0;
    // Establece pending a partir de result.published_sequence >.
    const std::uint64_t pending = result.published_sequence >
        // Incorpora result.consumed_sequence en esta operación.
        result.consumed_sequence
            // Incorpora ? result.published_sequence - result.consumed_sequence en esta operación.
            ? result.published_sequence - result.consumed_sequence
            // Inicializa el miembro indicado por 0 antes del cuerpo.
            : 0;
    // Establece result.buffer_utilization a partir de static_cast<double>(.
    result.buffer_utilization = static_cast<double>(
        // Incorpora std::min<std::uint64_t>(pending, slot_count_) en esta operación.
        std::min<std::uint64_t>(pending, slot_count_)
    // Incorpora ) / static_cast<double>(slot_count_) en esta operación.
    ) / static_cast<double>(slot_count_);
    // Devuelve result.
    return result;
}

// Define la operación SharedMemoryPublisher::memory con los parámetros declarados.
std::span<const std::byte> SharedMemoryPublisher::memory() const noexcept {
    // Devuelve {region_->data(), region_->size()}.
    return {region_->data(), region_->size()};
}

// Define la operación SharedMemoryPublisher::name con los parámetros declarados.
const std::string& SharedMemoryPublisher::name() const noexcept {
    // Devuelve region_->name().
    return region_->name();
}

// Incorpora } // namespace orvix::ipc en esta operación.
}  // namespace orvix::ipc
