// Incluye las declaraciones de shared memory protocol de ORVIX.
#include "orvix/ipc/shared_memory_protocol.hpp"
// Incluye las declaraciones de shared memory publisher de ORVIX.
#include "orvix/ipc/shared_memory_publisher.hpp"

// Incluye chrono para usar relojes y duraciones.
#include <chrono>
// Incluye cstddef para usar tipos de tamaño y bytes.
#include <cstddef>
// Incluye cstdint para usar enteros de tamaño fijo.
#include <cstdint>
// Incluye cstdlib para usar utilidades de la biblioteca C.
#include <cstdlib>
// Incluye cstring para usar operaciones de memoria y cadenas de C.
#include <cstring>
// Incluye iostream para usar entrada y salida por consola.
#include <iostream>
// Incluye span para usar vistas de memoria contigua.
#include <span>
// Incluye string para usar cadenas de texto.
#include <string>
// Incluye vector para usar contenedores dinámicos.
#include <vector>

// Limita estas declaraciones a esta unidad de compilación.
namespace {

// Declara los parámetros genéricos template<typename Value>.
template<typename Value>
// Define la operación read_value con los parámetros declarados.
Value read_value(
    // Declara el parámetro memory.
    const std::span<const std::byte> memory,
    // Incorpora const std::size_t offset en esta operación.
    const std::size_t offset
) {
    // Inicializa value con inicialización por valor.
    Value value{};
    // Copia bytes entre dos regiones de memoria mediante std::memcpy.
    std::memcpy(&value, memory.data() + offset, sizeof(Value));
    // Devuelve value.
    return value;
}

// Define la operación unique_name con los parámetros declarados.
std::string unique_name() {
    // Establece value a partir de std::chrono::steady_clock::now().
    const auto value = std::chrono::steady_clock::now()
        // Ejecuta .time_since_epoch().count().
        .time_since_epoch().count();
    // Devuelve "orvix-test-" + std::to_string(value % 1'000'000'000LL).
    return "orvix-test-" + std::to_string(value % 1'000'000'000LL);
}

// Incorpora } // namespace en esta operación.
}  // namespace

// Define la operación main con los parámetros declarados.
int main() {
    // Introduce en este ámbito namespace orvix.
    using namespace orvix;
    // Introduce en este ámbito namespace orvix::ipc.
    using namespace orvix::ipc;

    // Incorpora const capture::VideoFormat format{ en esta operación.
    const capture::VideoFormat format{
        // Aporta 0 como elemento de la lista.
        0,
        // Aporta 4 como elemento de la lista.
        4,
        // Aporta 2 como elemento de la lista.
        2,
        // Aporta 30 como elemento de la lista.
        30,
        // Aporta 1 como elemento de la lista.
        1,
        // Aporta el texto "NV12" a la expresión.
        "NV12",
        // Aporta false como elemento de la lista.
        false,
        // Incorpora 4 en esta operación.
        4
    };
    // Ejecuta SharedMemoryPublisher publisher(unique_name(), format, 3, 12).
    SharedMemoryPublisher publisher(unique_name(), format, 3, 12);
    // Ejecuta std::vector<std::byte> payload(12, static_cast<std::byte>(42)).
    std::vector<std::byte> payload(12, static_cast<std::byte>(42));

    // Recorre según std::uint64_t sequence = 1; sequence <= 5; ++sequence.
    for (std::uint64_t sequence = 1; sequence <= 5; ++sequence) {
        // Incorpora const capture::FrameMetadata metadata{ en esta operación.
        const capture::FrameMetadata metadata{
            // Aporta sequence como elemento de la lista.
            sequence,
            // Aporta static_cast<std::int64_t>(sequence * 333'333) como elemento de la lista.
            static_cast<std::int64_t>(sequence * 333'333),
            // Aporta format.width como elemento de la lista.
            format.width,
            // Aporta format.height como elemento de la lista.
            format.height,
            // Aporta format.stride como elemento de la lista.
            format.stride,
            // Aporta format.pixel_format como elemento de la lista.
            format.pixel_format,
            // Ejecuta payload.size().
            payload.size()
        };
        // Ejecuta publisher.publish(metadata, payload).
        publisher.publish(metadata, payload);
    }

    // Establece memory a partir de publisher.memory().
    const auto memory = publisher.memory();
    // Establece published_slot (slot publicado) a partir de read_value<std::uint32_t>(.
    const std::uint32_t published_slot = read_value<std::uint32_t>(
        // Aporta memory como elemento de la lista.
        memory,
        // Incorpora header_offset::published_slot en esta operación.
        header_offset::published_slot
    );
    // Establece slot_base a partir de kGlobalHeaderSize +.
    const std::size_t slot_base = kGlobalHeaderSize +
        // Incorpora static_cast<std::size_t>(published_slot) * en esta operación.
        static_cast<std::size_t>(published_slot) *
        // Incorpora read_value<std::uint64_t>(memory, header_offset::slot_stride) en esta operación.
        read_value<std::uint64_t>(memory, header_offset::slot_stride);
    // Establece statistics a partir de publisher.stats().
    const auto statistics = publisher.stats();

    // Evalúa la condición std::memcmp(memory.data(), "ORVX", 4) != 0 ||.
    if (
        // Combina esta condición con std::memcmp(memory.data(), "ORVX", 4) != 0 ||.
        std::memcmp(memory.data(), "ORVX", 4) != 0 ||
        // Combina esta condición con read_value<std::uint16_t>(memory, header_offset::version) != 1 ||.
        read_value<std::uint16_t>(memory, header_offset::version) != 1 ||
        // Combina esta condición con read_value<std::uint32_t>(memory, header_offset::slot_count) != 3 ||.
        read_value<std::uint32_t>(memory, header_offset::slot_count) != 3 ||
        // Incorpora read_value<std::uint64_t>( en esta operación.
        read_value<std::uint64_t>(
            // Aporta memory como elemento de la lista.
            memory,
            // Incorpora header_offset::published_sequence en esta operación.
            header_offset::published_sequence
        // Combina esta condición con ) != 5 ||.
        ) != 5 ||
        // Incorpora read_value<std::uint64_t>( en esta operación.
        read_value<std::uint64_t>(
            // Aporta memory como elemento de la lista.
            memory,
            // Incorpora slot_base + slot_offset::sequence en esta operación.
            slot_base + slot_offset::sequence
        // Combina esta condición con ) != 5 ||.
        ) != 5 ||
        // Incorpora read_value<std::uint64_t>( en esta operación.
        read_value<std::uint64_t>(
            // Aporta memory como elemento de la lista.
            memory,
            // Incorpora slot_base + slot_offset::payload_size en esta operación.
            slot_base + slot_offset::payload_size
        // Combina esta condición con ) != payload.size() ||.
        ) != payload.size() ||
        // Incorpora read_value<std::uint32_t>( en esta operación.
        read_value<std::uint32_t>(
            // Aporta memory como elemento de la lista.
            memory,
            // Incorpora slot_base + slot_offset::pixel_format en esta operación.
            slot_base + slot_offset::pixel_format
        // Combina esta condición con ) != static_cast<std::uint32_t>(PixelFormat::nv12) ||.
        ) != static_cast<std::uint32_t>(PixelFormat::nv12) ||
        // Ejecuta std::memcmp(.
        std::memcmp(
            // Ejecuta memory.data() + slot_base + slot_offset::payload,.
            memory.data() + slot_base + slot_offset::payload,
            // Ejecuta payload.data(),.
            payload.data(),
            // Ejecuta payload.size().
            payload.size()
        // Combina esta condición con ) != 0 ||.
        ) != 0 ||
        // Combina esta condición con statistics.published_sequence != 5 ||.
        statistics.published_sequence != 5 ||
        // Combina esta condición con statistics.overwritten_frames != 2 ||.
        statistics.overwritten_frames != 2 ||
        // Evalúa statistics.buffer_utilization != 1.0 como parte de la condición.
        statistics.buffer_utilization != 1.0
    ) {
        // Incorpora std::cerr << "shared-memory publication assertion failed\n" en esta operación.
        std::cerr << "shared-memory publication assertion failed\n";
        // Devuelve EXIT_FAILURE.
        return EXIT_FAILURE;
    }

    // Ejecuta publisher.mark_completed().
    publisher.mark_completed();
    // Evalúa la condición read_value<std::uint32_t>(.
    if (
        // Incorpora read_value<std::uint32_t>( en esta operación.
        read_value<std::uint32_t>(
            // Aporta memory como elemento de la lista.
            memory,
            // Incorpora header_offset::producer_state en esta operación.
            header_offset::producer_state
        // Evalúa ) != static_cast<std::uint32_t>(ProducerState::stopped) como parte de la condición.
        ) != static_cast<std::uint32_t>(ProducerState::stopped)
    ) {
        // Incorpora std::cerr << "producer state assertion failed\n" en esta operación.
        std::cerr << "producer state assertion failed\n";
        // Devuelve EXIT_FAILURE.
        return EXIT_FAILURE;
    }

    // Establece std::cout << "TC-IPC-001 a partir de PASS\n".
    std::cout << "TC-IPC-001=PASS\n";
    // Devuelve EXIT_SUCCESS.
    return EXIT_SUCCESS;
}
