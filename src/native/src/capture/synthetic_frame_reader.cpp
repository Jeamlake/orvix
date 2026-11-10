// Incluye las declaraciones de synthetic frame reader de ORVIX.
#include "orvix/capture/synthetic_frame_reader.hpp"

// Incluye algorithm para usar algoritmos sobre rangos.
#include <algorithm>
// Incluye chrono para usar relojes y duraciones.
#include <chrono>
// Incluye limits para usar límites de tipos numéricos.
#include <limits>
// Incluye stdexcept para usar tipos estándar de excepciones.
#include <stdexcept>
// Incluye thread para usar hilos de ejecución.
#include <thread>
// Incluye utility para usar utilidades de movimiento e intercambio.
#include <utility>
// Incluye vector para usar contenedores dinámicos.
#include <vector>

// Agrupa las declaraciones bajo orvix::capture.
namespace orvix::capture {

// Limita estas declaraciones a esta unidad de compilación.
namespace {

// Define la operación nv12_size con los parámetros declarados.
std::size_t nv12_size(
    // Declara el parámetro width (ancho en píxeles).
    const std::uint32_t width,
    // Incorpora const std::uint32_t height en esta operación.
    const std::uint32_t height
) {
    // Establece pixels a partir de static_cast<std::size_t>(width) * height.
    const std::size_t pixels =
        // Incorpora static_cast<std::size_t>(width) * height en esta operación.
        static_cast<std::size_t>(width) * height;
    // Evalúa la condición pixels > std::numeric_limits<std::size_t>::max() / 3U.
    if (pixels > std::numeric_limits<std::size_t>::max() / 3U) {
        // Interrumpe la operación con std::overflow_error("Synthetic frame size overflowed.").
        throw std::overflow_error("Synthetic frame size overflowed.");
    }
    // Devuelve pixels + pixels / 2U.
    return pixels + pixels / 2U;
}

// Incorpora } // namespace en esta operación.
}  // namespace

// Ejecuta SyntheticFrameReader::SyntheticFrameReader(.
SyntheticFrameReader::SyntheticFrameReader(
    // Declara el parámetro width (ancho en píxeles).
    const std::uint32_t width,
    // Declara el parámetro height (alto en píxeles).
    const std::uint32_t height,
    // Declara el parámetro frames_per_second.
    const std::uint32_t frames_per_second,
    // Incorpora const bool realtime en esta operación.
    const bool realtime
)
    // Inicializa el miembro indicado por format_{ antes del cuerpo.
    : format_{
          // Aporta 0 como elemento de la lista.
          0,
          // Aporta width como elemento de la lista.
          width,
          // Aporta height como elemento de la lista.
          height,
          // Aporta frames_per_second como elemento de la lista.
          frames_per_second,
          // Aporta 1 como elemento de la lista.
          1,
          // Aporta el texto "NV12" a la expresión.
          "NV12",
          // Aporta false como elemento de la lista.
          false,
          // Incorpora width en esta operación.
          width
      },
      // Ejecuta realtime_(realtime),.
      realtime_(realtime),
      // Define la operación _ con los parámetros declarados.
      next_frame_at_(std::chrono::steady_clock::now()) {
    // Evalúa la condición width == 0 ||.
    if (
        // Combina esta condición con width == 0 ||.
        width == 0 ||
        // Combina esta condición con height == 0 ||.
        height == 0 ||
        // Combina esta condición con width % 2U != 0 ||.
        width % 2U != 0 ||
        // Combina esta condición con height % 2U != 0 ||.
        height % 2U != 0 ||
        // Evalúa frames_per_second == 0 como parte de la condición.
        frames_per_second == 0
    ) {
        // Interrumpe la operación con std::invalid_argument(.
        throw std::invalid_argument(
            // Aporta el texto "Synthetic NV12 frames require non-zero even dimensions and FPS." a la expresión.
            "Synthetic NV12 frames require non-zero even dimensions and FPS."
        );
    }
}

// Define la operación SyntheticFrameReader::read_next con los parámetros declarados.
FrameReadResult SyntheticFrameReader::read_next() {
    // Establece interval a partir de std::chrono::nanoseconds(.
    const auto interval = std::chrono::nanoseconds(
        // Incorpora 1'000'000'000LL / format_.frame_rate_numerator en esta operación.
        1'000'000'000LL / format_.frame_rate_numerator
    );
    // Evalúa la condición realtime_.
    if (realtime_) {
        // Actualiza next_frame_at_ con += y interval.
        next_frame_at_ += interval;
        // Ejecuta std::this_thread::sleep_until(next_frame_at_).
        std::this_thread::sleep_until(next_frame_at_);
    }

    // Incrementa generated_frames_ en una unidad.
    ++generated_frames_;
    // Establece y_plane_size a partir de static_cast<std::size_t>(format_.width) * format_.height.
    const std::size_t y_plane_size =
        // Incorpora static_cast<std::size_t>(format_.width) * format_.height en esta operación.
        static_cast<std::size_t>(format_.width) * format_.height;
    // Ejecuta std::vector<std::byte> payload(nv12_size(format_.width, format_.height)).
    std::vector<std::byte> payload(nv12_size(format_.width, format_.height));
    // Establece phase a partir de static_cast<std::uint32_t>(.
    const std::uint32_t phase = static_cast<std::uint32_t>(
        // Incorpora generated_frames_ % format_.width en esta operación.
        generated_frames_ % format_.width
    );

    // A bright block changes position twice a second at the default FPS.
    // It gives the motion mode a reproducible, visible target.
    // Establece block_size a partir de std::min(format_.width, format_.height) / 5.
    const std::uint32_t block_size = std::min(format_.width, format_.height) / 5;
    // Establece block_x a partir de (format_.width - block_size) *.
    const std::uint32_t block_x = (format_.width - block_size) *
        // Incorpora static_cast<std::uint32_t>((generated_frames_ / 15) % 5) / 4 en esta operación.
        static_cast<std::uint32_t>((generated_frames_ / 15) % 5) / 4;
    // Establece block_y a partir de (format_.height - block_size) / 2.
    const std::uint32_t block_y = (format_.height - block_size) / 2;
    // Recorre según std::uint32_t row = 0; row < format_.height; ++row.
    for (std::uint32_t row = 0; row < format_.height; ++row) {
        // Recorre según std::uint32_t column = 0; column < format_.width; ++column.
        for (std::uint32_t column = 0; column < format_.width; ++column) {
            // Establece value a partir de row >= block_y && row < block_y + block_size &&.
            const std::uint32_t value =
                // Combina esta condición con row >= block_y && row < block_y + block_size &&.
                row >= block_y && row < block_y + block_size &&
                // Evalúa column >= block_x && column < block_x + block_size como parte de la condición.
                column >= block_x && column < block_x + block_size
                    // Incorpora ? 235U : 32U + ((column + phase + row / 4U) % 192U) en esta operación.
                    ? 235U : 32U + ((column + phase + row / 4U) % 192U);
            // Establece payload[static_cast<std::size_t>(row) * format_.width + column] a partir de static_cast<std::byte>(value).
            payload[static_cast<std::size_t>(row) * format_.width + column] =
                // Incorpora static_cast<std::byte>(value) en esta operación.
                static_cast<std::byte>(value);
        }
    }
    // Ejecuta std::fill(.
    std::fill(
        // Ejecuta payload.begin() + static_cast<std::ptrdiff_t>(y_plane_size),.
        payload.begin() + static_cast<std::ptrdiff_t>(y_plane_size),
        // Ejecuta payload.end(),.
        payload.end(),
        // Incorpora static_cast<std::byte>(128U) en esta operación.
        static_cast<std::byte>(128U)
    );

    // Establece timestamp (marca de tiempo) a partir de static_cast<std::int64_t>(.
    const std::int64_t timestamp = static_cast<std::int64_t>(
        // Incorpora generated_frames_ * 10'000'000ULL / en esta operación.
        generated_frames_ * 10'000'000ULL /
        // Incorpora format_.frame_rate_numerator en esta operación.
        format_.frame_rate_numerator
    );
    // Devuelve {.
    return {
        // Aporta true como elemento de la lista.
        true,
        // Aporta timestamp como elemento de la lista.
        timestamp,
        // Ejecuta payload.size(),.
        payload.size(),
        // Aporta format_.stride como elemento de la lista.
        format_.stride,
        // Transfiere el valor sin exigir una copia mediante std::move.
        std::move(payload)
    };
}

// Define la operación SyntheticFrameReader::format con los parámetros declarados.
const VideoFormat& SyntheticFrameReader::format() const noexcept {
    // Devuelve format_.
    return format_;
}

// Incorpora } // namespace orvix::capture en esta operación.
}  // namespace orvix::capture
