// Incluye las declaraciones de continuous frame capture de ORVIX.
#include "orvix/capture/continuous_frame_capture.hpp"

// Incluye las declaraciones de diagnostic error de ORVIX.
#include "orvix/capture/diagnostic_error.hpp"

// Incluye limits para usar límites de tipos numéricos.
#include <limits>
// Incluye stdexcept para usar tipos estándar de excepciones.
#include <stdexcept>

// Agrupa las declaraciones bajo orvix::capture.
namespace orvix::capture {

// Define la operación CaptureSummary::duration_seconds con los parámetros declarados.
double CaptureSummary::duration_seconds() const noexcept {
    // Evalúa la condición captured_frames < 2 ||.
    if (
        // Combina esta condición con captured_frames < 2 ||.
        captured_frames < 2 ||
        // Evalúa last_timestamp_100ns <= first_timestamp_100ns como parte de la condición.
        last_timestamp_100ns <= first_timestamp_100ns
    ) {
        // Devuelve 0.0.
        return 0.0;
    }

    // Establece kTimestampUnitsPerSecond a partir de 10'000'000.0.
    constexpr double kTimestampUnitsPerSecond = 10'000'000.0;
    // Devuelve static_cast<double>(.
    return static_cast<double>(
        // Incorpora last_timestamp_100ns - first_timestamp_100ns en esta operación.
        last_timestamp_100ns - first_timestamp_100ns
    // Incorpora ) / kTimestampUnitsPerSecond en esta operación.
    ) / kTimestampUnitsPerSecond;
}

// Define la operación CaptureSummary::capture_fps con los parámetros declarados.
double CaptureSummary::capture_fps() const noexcept {
    // Establece duration a partir de duration_seconds().
    const double duration = duration_seconds();

    // Evalúa la condición captured_frames < 2 || duration <= 0.0.
    if (captured_frames < 2 || duration <= 0.0) {
        // Devuelve 0.0.
        return 0.0;
    }

    // Devuelve static_cast<double>(captured_frames - 1) / duration.
    return static_cast<double>(captured_frames - 1) / duration;
}

// Define la operación ContinuousFrameCapture::run con los parámetros declarados.
CaptureSummary ContinuousFrameCapture::run(
    // Declara el parámetro reader (lector de fotogramas).
    FrameReader& reader,
    // Declara el parámetro requested_frames.
    const std::size_t requested_frames,
    // Declara el parámetro format (formato de video).
    const VideoFormat& format,
    // Declara el parámetro maximum_empty_reads.
    const std::size_t maximum_empty_reads,
    // Incorpora FrameSink* const sink en esta operación.
    FrameSink* const sink
) {
    // Evalúa la condición requested_frames == 0 && sink == nullptr.
    if (requested_frames == 0 && sink == nullptr) {
        // Interrumpe la operación con std::invalid_argument(.
        throw std::invalid_argument(
            // Aporta el texto "Continuous capture requires at least one frame." a la expresión.
            "Continuous capture requires at least one frame."
        );
    }

    // Declara summary (resumen de la sesión) con el tipo indicado.
    CaptureSummary summary;
    // Establece summary.requested_frames a partir de requested_frames.
    summary.requested_frames = requested_frames;
    // Inicializa consecutive_empty_reads con inicialización por valor.
    std::size_t consecutive_empty_reads{};
    // Evalúa la condición sink == nullptr.
    if (sink == nullptr) {
        // Reserva capacidad en el contenedor mediante summary.frames.reserve.
        summary.frames.reserve(requested_frames);
    }

    // Repite mientras se cumpla requested_frames == 0 || summary.captured_frames < requested_frames.
    while (requested_frames == 0 || summary.captured_frames < requested_frames) {
        // Evalúa la condición sink != nullptr && sink->stop_requested().
        if (sink != nullptr && sink->stop_requested()) {
            // Termina el bucle o la rama actual.
            break;
        }
        // Establece frame (fotograma) a partir de reader.read_next().
        const FrameReadResult frame = reader.read_next();

        // Evalúa la condición !frame.frame_available.
        if (!frame.frame_available) {
            // Incrementa summary.empty_reads en una unidad.
            ++summary.empty_reads;
            // Incrementa consecutive_empty_reads en una unidad.
            ++consecutive_empty_reads;
            // Evalúa la condición consecutive_empty_reads > maximum_empty_reads.
            if (consecutive_empty_reads > maximum_empty_reads) {
                // Interrumpe la operación con DiagnosticError(.
                throw DiagnosticError(
                    // Aporta el texto "ORV-CAP-503" a la expresión.
                    "ORV-CAP-503",
                    // Aporta el texto "camera_stream_stalled" a la expresión.
                    "camera_stream_stalled",
                    // Aporta el texto "The camera stopped producing video frames." a la expresión.
                    "The camera stopped producing video frames.",
                    // Incorpora 74 en esta operación.
                    74
                );
            }

            // Pasa a la siguiente iteración.
            continue;
        }
        // Establece consecutive_empty_reads a partir de 0.
        consecutive_empty_reads = 0;

        // Establece byte_count a partir de frame.payload.empty().
        const std::size_t byte_count = frame.payload.empty()
            // Incorpora ? frame.byte_count en esta operación.
            ? frame.byte_count
            // Inicializa el miembro indicado por frame.payload.size() antes del cuerpo.
            : frame.payload.size();

        // Evalúa la condición byte_count >.
        if (
            // Incorpora byte_count > en esta operación.
            byte_count >
            // Ejecuta std::numeric_limits<std::size_t>::max() - summary.total_bytes.
            std::numeric_limits<std::size_t>::max() - summary.total_bytes
        ) {
            // Interrumpe la operación con std::overflow_error("Captured byte counter overflowed.").
            throw std::overflow_error("Captured byte counter overflowed.");
        }

        // Incrementa summary.captured_frames en una unidad.
        ++summary.captured_frames;
        // Establece summary.last_sequence (última secuencia observada) a partir de static_cast<std::uint64_t>(summary.captured_frames).
        summary.last_sequence =
            // Incorpora static_cast<std::uint64_t>(summary.captured_frames) en esta operación.
            static_cast<std::uint64_t>(summary.captured_frames);
        // Actualiza summary.total_bytes con += y byte_count.
        summary.total_bytes += byte_count;

        // Evalúa la condición summary.captured_frames == 1.
        if (summary.captured_frames == 1) {
            // Establece summary.first_sequence a partir de summary.last_sequence.
            summary.first_sequence = summary.last_sequence;
            // Establece summary.first_timestamp_100ns a partir de frame.timestamp_100ns.
            summary.first_timestamp_100ns = frame.timestamp_100ns;
        }
        // Evalúa la alternativa else if (frame.timestamp_100ns <= summary.last_timestamp_100ns) {.
        else if (frame.timestamp_100ns <= summary.last_timestamp_100ns) {
            // Establece summary.timestamps_strictly_increasing a partir de false.
            summary.timestamps_strictly_increasing = false;
        }

        // Establece summary.last_timestamp_100ns a partir de frame.timestamp_100ns.
        summary.last_timestamp_100ns = frame.timestamp_100ns;
        // Incorpora FrameMetadata metadata{ en esta operación.
        FrameMetadata metadata{
            // Aporta summary.last_sequence como elemento de la lista.
            summary.last_sequence,
            // Aporta frame.timestamp_100ns como elemento de la lista.
            frame.timestamp_100ns,
            // Aporta format.width como elemento de la lista.
            format.width,
            // Aporta format.height como elemento de la lista.
            format.height,
            // Evalúa frame.stride == 0 ? format.stride : frame.stride, como parte de la condición.
            frame.stride == 0 ? format.stride : frame.stride,
            // Aporta format.pixel_format como elemento de la lista.
            format.pixel_format,
            // Incorpora byte_count en esta operación.
            byte_count
        };
        // Bridge sessions can run indefinitely: never accumulate metadata
        // for every frame in the producer. The CLI capture command retains it.
        // Evalúa la condición sink == nullptr.
        if (sink == nullptr) {
            // Agrega un elemento al final del contenedor mediante summary.frames.push_back.
            summary.frames.push_back(metadata);
        }

        // Evalúa la condición sink != nullptr.
        if (sink != nullptr) {
            // Evalúa la condición frame.payload.empty() && byte_count != 0.
            if (frame.payload.empty() && byte_count != 0) {
                // Interrumpe la operación con std::runtime_error(.
                throw std::runtime_error(
                    // Aporta el texto "A frame sink requires the captured payload bytes." a la expresión.
                    "A frame sink requires the captured payload bytes."
                );
            }
            // Ejecuta sink->publish(metadata, frame.payload).
            sink->publish(metadata, frame.payload);
        }
    }

    // Devuelve summary.
    return summary;
}

// Incorpora } // namespace orvix::capture en esta operación.
}  // namespace orvix::capture
