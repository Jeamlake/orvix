// Incluye las declaraciones de continuous frame capture de ORVIX.
#include "orvix/capture/continuous_frame_capture.hpp"
// Incluye las declaraciones de diagnostic error de ORVIX.
#include "orvix/capture/diagnostic_error.hpp"

// Incluye cmath para usar funciones matemáticas.
#include <cmath>
// Incluye cstdlib para usar utilidades de la biblioteca C.
#include <cstdlib>
// Incluye iostream para usar entrada y salida por consola.
#include <iostream>
// Incluye stdexcept para usar tipos estándar de excepciones.
#include <stdexcept>
// Incluye utility para usar utilidades de movimiento e intercambio.
#include <utility>
// Incluye vector para usar contenedores dinámicos.
#include <vector>

// Limita estas declaraciones a esta unidad de compilación.
namespace {

// Declara el tipo SyntheticFrameReader y las operaciones o datos de su contrato.
class SyntheticFrameReader final : public orvix::capture::FrameReader {
// Expone los miembros de la interfaz.
public:
    // Define la operación r con los parámetros declarados.
    explicit SyntheticFrameReader(
        // Incorpora std::vector<orvix::capture::FrameReadResult> reads en esta operación.
        std::vector<orvix::capture::FrameReadResult> reads
    )
        // Inicializa el miembro indicado por reads_(std::move(reads)) {} antes del cuerpo.
        : reads_(std::move(reads)) {}

    // Define la operación read_next con los parámetros declarados.
    orvix::capture::FrameReadResult read_next() override {
        // Evalúa la condición position_ >= reads_.size().
        if (position_ >= reads_.size()) {
            // Devuelve {}.
            return {};
        }

        // Devuelve reads_[position_++].
        return reads_[position_++];
    }

// Restringe estos miembros a la implementación.
private:
    // Declara reads_ con el tipo indicado.
    std::vector<orvix::capture::FrameReadResult> reads_;
    // Inicializa position_ con inicialización por valor.
    std::size_t position_{};
};

// Incorpora } // namespace en esta operación.
}  // namespace

// Declara el tipo StopAfterTwoSink y las operaciones o datos de su contrato.
class StopAfterTwoSink final : public orvix::capture::FrameSink {
// Expone los miembros de la interfaz.
public:
    // Define la operación publish con los parámetros declarados.
    void publish(const orvix::capture::FrameMetadata&, std::span<const std::byte>) override {
        // Incrementa count en una unidad.
        ++count;
    }
    // Ejecuta bool stop_requested() const noexcept override { return count >= 2; }.
    bool stop_requested() const noexcept override { return count >= 2; }
    // Inicializa count con inicialización por valor.
    int count{};
};

// Define la operación main con los parámetros declarados.
int main() {
    // Define la operación reader con los parámetros declarados.
    SyntheticFrameReader reader({
        // Aporta {false, 0, 0} como elemento de la lista.
        {false, 0, 0},
        // Aporta {true, 1000000, 10} como elemento de la lista.
        {true, 1000000, 10},
        // Aporta {true, 1333333, 20} como elemento de la lista.
        {true, 1333333, 20},
        // Incorpora {true, 1666666, 30} en esta operación.
        {true, 1666666, 30}
    });

    // Establece summary (resumen de la sesión) a partir de orvix::capture::ContinuousFrameCapture::run(.
    const auto summary =
        // Ejecuta orvix::capture::ContinuousFrameCapture::run(.
        orvix::capture::ContinuousFrameCapture::run(
            // Aporta reader como elemento de la lista.
            reader,
            // Aporta 3 como elemento de la lista.
            3,
            // Incorpora {0, 1280, 720, 30, 1, "NV12", false} en esta operación.
            {0, 1280, 720, 30, 1, "NV12", false}
        );

    // Evalúa la condición summary.captured_frames != 3 ||.
    if (
        // Combina esta condición con summary.captured_frames != 3 ||.
        summary.captured_frames != 3 ||
        // Combina esta condición con summary.first_sequence != 1 ||.
        summary.first_sequence != 1 ||
        // Combina esta condición con summary.last_sequence != 3 ||.
        summary.last_sequence != 3 ||
        // Combina esta condición con summary.first_timestamp_100ns != 1000000 ||.
        summary.first_timestamp_100ns != 1000000 ||
        // Combina esta condición con summary.last_timestamp_100ns != 1666666 ||.
        summary.last_timestamp_100ns != 1666666 ||
        // Combina esta condición con summary.total_bytes != 60 ||.
        summary.total_bytes != 60 ||
        // Combina esta condición con summary.empty_reads != 1 ||.
        summary.empty_reads != 1 ||
        // Combina esta condición con !summary.timestamps_strictly_increasing ||.
        !summary.timestamps_strictly_increasing ||
        // Combina esta condición con summary.frames.size() != 3 ||.
        summary.frames.size() != 3 ||
        // Combina esta condición con summary.frames[1].sequence != 2 ||.
        summary.frames[1].sequence != 2 ||
        // Combina esta condición con summary.frames[1].timestamp_100ns != 1333333 ||.
        summary.frames[1].timestamp_100ns != 1333333 ||
        // Combina esta condición con summary.frames[1].width != 1280 ||.
        summary.frames[1].width != 1280 ||
        // Combina esta condición con summary.frames[1].height != 720 ||.
        summary.frames[1].height != 720 ||
        // Combina esta condición con summary.frames[1].pixel_format != "NV12" ||.
        summary.frames[1].pixel_format != "NV12" ||
        // Evalúa summary.frames[1].byte_count != 20 como parte de la condición.
        summary.frames[1].byte_count != 20
    ) {
        // Incorpora std::cerr << "continuous frame summary assertion failed\n" en esta operación.
        std::cerr << "continuous frame summary assertion failed\n";
        // Devuelve EXIT_FAILURE.
        return EXIT_FAILURE;
    }

    // Evalúa la condición std::abs(summary.capture_fps() - 30.0) > 0.01.
    if (std::abs(summary.capture_fps() - 30.0) > 0.01) {
        // Incorpora std::cerr << "capture FPS calculation assertion failed\n" en esta operación.
        std::cerr << "capture FPS calculation assertion failed\n";
        // Devuelve EXIT_FAILURE.
        return EXIT_FAILURE;
    }

    // Establece zero_frames_rejected a partir de false.
    bool zero_frames_rejected = false;

    // Ejecuta el bloque protegido contra excepciones.
    try {
        // Ejecuta SyntheticFrameReader unused({}).
        SyntheticFrameReader unused({});
        // Incorpora static_cast<void>( en esta operación.
        static_cast<void>(
            // Ejecuta orvix::capture::ContinuousFrameCapture::run(.
            orvix::capture::ContinuousFrameCapture::run(
                // Aporta unused como elemento de la lista.
                unused,
                // Aporta 0 como elemento de la lista.
                0,
                // Incorpora {0, 1280, 720, 30, 1, "NV12", false} en esta operación.
                {0, 1280, 720, 30, 1, "NV12", false}
            )
        );
    }
    // Maneja la excepción declarada en catch (const std::invalid_argument&) {.
    catch (const std::invalid_argument&) {
        // Establece zero_frames_rejected a partir de true.
        zero_frames_rejected = true;
    }

    // Evalúa la condición !zero_frames_rejected.
    if (!zero_frames_rejected) {
        // Incorpora std::cerr << "zero-frame request was not rejected\n" en esta operación.
        std::cerr << "zero-frame request was not rejected\n";
        // Devuelve EXIT_FAILURE.
        return EXIT_FAILURE;
    }

    // Establece stalled_stream_rejected a partir de false.
    bool stalled_stream_rejected = false;

    // Ejecuta el bloque protegido contra excepciones.
    try {
        // Ejecuta SyntheticFrameReader stalled({}).
        SyntheticFrameReader stalled({});
        // Incorpora static_cast<void>( en esta operación.
        static_cast<void>(
            // Ejecuta orvix::capture::ContinuousFrameCapture::run(.
            orvix::capture::ContinuousFrameCapture::run(
                // Aporta stalled como elemento de la lista.
                stalled,
                // Aporta 1 como elemento de la lista.
                1,
                // Aporta {0, 1280, 720, 30, 1, "NV12", false} como elemento de la lista.
                {0, 1280, 720, 30, 1, "NV12", false},
                // Incorpora 2 en esta operación.
                2
            )
        );
    }
    // Maneja la excepción declarada en catch (const orvix::capture::DiagnosticError& error) {.
    catch (const orvix::capture::DiagnosticError& error) {
        // Establece stalled_stream_rejected a partir de error.code() == "ORV-CAP-503" &&.
        stalled_stream_rejected =
            // Combina esta condición con error.code() == "ORV-CAP-503" &&.
            error.code() == "ORV-CAP-503" &&
            // Combina esta condición con error.event() == "camera_stream_stalled" &&.
            error.event() == "camera_stream_stalled" &&
            // Ejecuta error.exit_code() == 74.
            error.exit_code() == 74;
    }

    // Evalúa la condición !stalled_stream_rejected.
    if (!stalled_stream_rejected) {
        // Incorpora std::cerr << "stalled camera stream was not diagnosed\n" en esta operación.
        std::cerr << "stalled camera stream was not diagnosed\n";
        // Devuelve EXIT_FAILURE.
        return EXIT_FAILURE;
    }

    // Declara sink con el tipo indicado.
    StopAfterTwoSink sink;
    // Define la operación continuous con los parámetros declarados.
    SyntheticFrameReader continuous({
        // Aporta {false, 0, 0}, {true, 1000000, 0} como elemento de la lista.
        {false, 0, 0}, {true, 1000000, 0},
        // Incorpora {false, 0, 0}, {true, 1333333, 0}, {true, 1666666, 0} en esta operación.
        {false, 0, 0}, {true, 1333333, 0}, {true, 1666666, 0}
    });
    // Establece bounded a partir de orvix::capture::ContinuousFrameCapture::run(.
    const auto bounded = orvix::capture::ContinuousFrameCapture::run(
        // Incorpora continuous, 0, {0, 4, 2, 30, 1, "NV12", false}, 1, &sink) en esta operación.
        continuous, 0, {0, 4, 2, 30, 1, "NV12", false}, 1, &sink);
    // Evalúa la condición bounded.captured_frames != 2 || bounded.empty_reads != 2 || !bounded.frames.empty().
    if (bounded.captured_frames != 2 || bounded.empty_reads != 2 || !bounded.frames.empty()) {
        // Incorpora std::cerr << "continuous bridge stop/bounded-memory assertion failed\n" en esta operación.
        std::cerr << "continuous bridge stop/bounded-memory assertion failed\n";
        // Devuelve EXIT_FAILURE.
        return EXIT_FAILURE;
    }

    // Establece std::cout << "TC-CAP-004 a partir de PASS\n".
    std::cout << "TC-CAP-004=PASS\n";
    // Devuelve EXIT_SUCCESS.
    return EXIT_SUCCESS;
}
