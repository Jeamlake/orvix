"""Live OpenCV viewer for frames produced by the native ORVIX process."""

# Importa annotations desde __future__.
from __future__ import annotations

# Importa argparse.
import argparse
# Importa sys.
import sys
# Importa time.
import time
# Importa deque desde collections.
from collections import deque
# Importa Deque, Optional, Sequence desde typing.
from typing import Deque, Optional, Sequence

# Importa cv2.
import cv2

# Importa DEFAULT_SHARED_MEMORY_NAME, SharedMemoryReader desde orvix.ipc.
from orvix.ipc import DEFAULT_SHARED_MEMORY_NAME, SharedMemoryReader
# Importa PRODUCER_FAILED, PRODUCER_STOPPED desde orvix.ipc.protocol.
from orvix.ipc.protocol import PRODUCER_FAILED, PRODUCER_STOPPED
# Importa SequenceTracker desde orvix.metrics.
from orvix.metrics import SequenceTracker
# Importa FrameDecodeError, decode_bgr desde orvix.vision.
from orvix.vision import FrameDecodeError, decode_bgr
# Importa MODES, VisionPipeline desde orvix.vision.pipeline.
from orvix.vision.pipeline import MODES, VisionPipeline


# Define _parser sin parámetros de llamada.
def _parser() -> argparse.ArgumentParser:
    # Obtiene parser con argparse.ArgumentParser(description='Display raw frames produced by…
    parser = argparse.ArgumentParser(
        # Indica description con valor "Display raw frames produced by the native ORVIX bridge.".
        description="Display raw frames produced by the native ORVIX bridge."
    )
    # Ejecuta parser.add_argument("--name", default=DEFAULT_SHARED_MEMORY_NAME).
    parser.add_argument("--name", default=DEFAULT_SHARED_MEMORY_NAME)
    # Ejecuta parser.add_argument("--frames", type=int, default=0).
    parser.add_argument("--frames", type=int, default=0)
    # Ejecuta parser.add_argument("--mode", choices=MODES, default="raw").
    parser.add_argument("--mode", choices=MODES, default="raw")
    # Ejecuta parser.add_argument("--attach-timeout", type=float, default=10.0).
    parser.add_argument("--attach-timeout", type=float, default=10.0)
    # Ejecuta parser.add_argument("--poll-ms", type=float, default=2.0).
    parser.add_argument("--poll-ms", type=float, default=2.0)
    # Ejecuta parser.add_argument("--stall-timeout", type=float, default=5.0).
    parser.add_argument("--stall-timeout", type=float, default=5.0)
    # Ejecuta parser.add_argument(.
    parser.add_argument(
        # Aporta el texto "--headless" a la expresión.
        "--headless",
        # Indica action con valor "store_true".
        action="store_true",
        # Indica help con valor "Validate frames without creating a window.".
        help="Validate frames without creating a window.",
    )
    # Devuelve parser.
    return parser


# Define _consumer_fps con el parámetro samples.
def _consumer_fps(samples: Deque[float]) -> float:
    # Entra en esta rama si len(samples) < 2.
    if len(samples) < 2:
        # Devuelve 0.0.
        return 0.0
    # Establece elapsed (tiempo transcurrido) a partir de samples[-1] - samples[0].
    elapsed = samples[-1] - samples[0]
    # Devuelve (len(samples) - 1) / elapsed if elapsed > 0 else 0.0.
    return (len(samples) - 1) / elapsed if elapsed > 0 else 0.0


# Define run con el parámetro args.
def run(args: argparse.Namespace) -> int:
    # Entra en esta rama si args.frames < 0 or args.poll_ms < 0.
    if args.frames < 0 or args.poll_ms < 0:
        # Interrumpe la operación con ValueError('--frames and --poll-ms cannot be negative').
        raise ValueError("--frames and --poll-ms cannot be negative")
    # Entra en esta rama si getattr(args, 'stall_timeout', 5.0) <= 0.
    if getattr(args, "stall_timeout", 5.0) <= 0:
        # Interrumpe la operación con ValueError('--stall-timeout must be positive').
        raise ValueError("--stall-timeout must be positive")

    # Establece displayed a partir de 0.
    displayed = 0
    # Obtiene mode (modo de procesamiento) con getattr(args, 'mode', 'raw').
    mode = getattr(args, "mode", "raw")
    # Obtiene pipeline con VisionPipeline().
    pipeline = VisionPipeline()
    # Establece latency_total a partir de 0.0.
    latency_total = 0.0
    # Obtiene sequences con SequenceTracker().
    sequences = SequenceTracker()
    # Obtiene samples con deque(maxlen=120).
    samples: Deque[float] = deque(maxlen=120)

    # Acota el uso de SharedMemoryReader(args.name, args.attach_timeout) a este bloque.
    with SharedMemoryReader(args.name, args.attach_timeout) as reader:
        # Waiting for the mapping is distinct from waiting for live frames.
        # Guarda en last_frame_at el resultado de time.monotonic, que lee un reloj monotónico.
        last_frame_at = time.monotonic()
        # Establece initial a partir de reader.header.
        initial = reader.header
        # Establece target_fps a partir de initial.fps_numerator / initial.fps_denominator if initial.fps_denominator else 0…
        target_fps = (
            # Incorpora initial.fps_numerator / initial.fps_denominator en esta operación.
            initial.fps_numerator / initial.fps_denominator
            # Incorpora if initial.fps_denominator como condición o recorrido de la expresión.
            if initial.fps_denominator
            # Incorpora else 0.0 en esta operación.
            else 0.0
        )
        # Muestra información en la salida mediante print con 'ORVIX viewer connected: name={} producer_pid={} format={} r…
        print(
            # Aporta el texto "ORVIX viewer connected: name={} producer_pid={} " a la expresión.
            "ORVIX viewer connected: name={} producer_pid={} "
            # Ejecuta "format={} resolution={}x{} target_fps={:.2f}".format(.
            "format={} resolution={}x{} target_fps={:.2f}".format(
                # Aporta args.name como elemento de la lista.
                args.name,
                # Aporta initial.producer_pid como elemento de la lista.
                initial.producer_pid,
                # Aporta initial.pixel_format como elemento de la lista.
                initial.pixel_format,
                # Aporta initial.width como elemento de la lista.
                initial.width,
                # Aporta initial.height como elemento de la lista.
                initial.height,
                # Aporta target_fps como elemento de la lista.
                target_fps,
            ),
            # Indica flush con valor True.
            flush=True,
        )

        # Repite mientras True.
        while True:
            # Establece header a partir de reader.header.
            header = reader.header
            # Obtiene frame (fotograma) con reader.read_latest().
            frame = reader.read_latest()
            # Entra en esta rama si frame is not None and frame.sequence != sequences.last_sequence.
            if frame is not None and frame.sequence != sequences.last_sequence:
                # Ejecuta sequences.observe(frame.sequence).
                sequences.observe(frame.sequence)
                # Obtiene started con time.perf_counter().
                started = time.perf_counter()
                # Obtiene image (imagen) con pipeline.process(decode_bgr(frame), mode).
                image = pipeline.process(decode_bgr(frame), mode)
                # Suma (time.perf_counter() - started) * 1000 al valor de latency_total.
                latency_total += (time.perf_counter() - started) * 1000
                # Ejecuta reader.acknowledge(sequences.last_sequence).
                reader.acknowledge(sequences.last_sequence)
                # Suma 1 al valor de displayed.
                displayed += 1
                # Agrega un elemento al final mediante samples.append con time.monotonic().
                samples.append(time.monotonic())
                # Guarda en last_frame_at el resultado de time.monotonic, que lee un reloj monotónico.
                last_frame_at = time.monotonic()

                # Entra en esta rama si not args.headless.
                if not args.headless:
                    # Obtiene overlay con 'seq={} {}x{} {} producer={:.1f}fps viewer={:.1f}fps skipped={}'.for…
                    overlay = (
                        # Aporta el texto "seq={} {}x{} {} producer={:.1f}fps " a la expresión.
                        "seq={} {}x{} {} producer={:.1f}fps "
                        # Ejecuta "viewer={:.1f}fps skipped={}".format(.
                        "viewer={:.1f}fps skipped={}".format(
                            # Aporta frame.sequence como elemento de la lista.
                            frame.sequence,
                            # Aporta frame.width como elemento de la lista.
                            frame.width,
                            # Aporta frame.height como elemento de la lista.
                            frame.height,
                            # Aporta frame.pixel_format como elemento de la lista.
                            frame.pixel_format,
                            # Aporta header.producer_fps como elemento de la lista.
                            header.producer_fps,
                            # Ejecuta _consumer_fps(samples),.
                            _consumer_fps(samples),
                            # Aporta sequences.skipped_frames como elemento de la lista.
                            sequences.skipped_frames,
                        )
                    )
                    # Ejecuta cv2.putText(.
                    cv2.putText(
                        # Aporta image como elemento de la lista.
                        image,
                        # Aporta overlay como elemento de la lista.
                        overlay,
                        # Aporta (12, 28) como elemento de la lista.
                        (12, 28),
                        # Aporta cv2.FONT_HERSHEY_SIMPLEX como elemento de la lista.
                        cv2.FONT_HERSHEY_SIMPLEX,
                        # Aporta 0.6 como elemento de la lista.
                        0.6,
                        # Aporta (0, 255, 0) como elemento de la lista.
                        (0, 255, 0),
                        # Aporta 2 como elemento de la lista.
                        2,
                        # Aporta cv2.LINE_AA como elemento de la lista.
                        cv2.LINE_AA,
                    )
                    # Ejecuta cv2.imshow("ORVIX Live - " + mode.upper(), image).
                    cv2.imshow("ORVIX Live - " + mode.upper(), image)

                # Entra en esta rama si args.frames and displayed >= args.frames.
                if args.frames and displayed >= args.frames:
                    # Termina el bucle actual.
                    break

            # Entra en esta rama si header.producer_state in (PRODUCER_STOPPED, PRODUCER_FAILED).
            if header.producer_state in (PRODUCER_STOPPED, PRODUCER_FAILED):
                # Entra en esta rama si frame is None or frame.sequence == sequences.last_sequence.
                if frame is None or frame.sequence == sequences.last_sequence:
                    # Termina el bucle actual.
                    break
            # Entra en esta rama si not args.headless.
            if not args.headless:
                # Pump window events even if the producer stops emitting frames.
                # Establece key a partir de cv2.waitKey(1) & 255.
                key = cv2.waitKey(1) & 0xFF
                # Entra en esta rama si key in (27, ord('q')).
                if key in (27, ord("q")):
                    # Termina el bucle actual.
                    break
            # Entra en esta rama si time.monotonic() - last_frame_at > getattr(args, 'stall_timeout', 5.0).
            if time.monotonic() - last_frame_at > getattr(args, "stall_timeout", 5.0):
                # Interrumpe la operación con TimeoutError('Producer stopped delivering frames; consumer detached.').
                raise TimeoutError("Producer stopped delivering frames; consumer detached.")
            # Entra en esta rama si args.poll_ms.
            if args.poll_ms:
                # Ejecuta time.sleep(args.poll_ms / 1000.0).
                time.sleep(args.poll_ms / 1000.0)

        # Establece final a partir de reader.header.
        final = reader.header

    # Entra en esta rama si not args.headless.
    if not args.headless:
        # Ejecuta cv2.destroyAllWindows().
        cv2.destroyAllWindows()
    # Muestra información en la salida mediante print con 'Viewer status: COMPLETED frames={} first_to_last={} skipped…
    print(
        # Aporta el texto "Viewer status: COMPLETED frames={} first_to_last={} skipped={} " a la expresión.
        "Viewer status: COMPLETED frames={} first_to_last={} skipped={} "
        # Aporta el texto "producer_overwrites={} buffer_utilization={:.1f}% viewer_fps={:.2f} " a la expresión.
        "producer_overwrites={} buffer_utilization={:.1f}% viewer_fps={:.2f} "
        # Ejecuta "mode={} capture_fps={:.2f} mean_processing_ms={:.3f} detections={}".format(.
        "mode={} capture_fps={:.2f} mean_processing_ms={:.3f} detections={}".format(
            # Aporta displayed como elemento de la lista.
            displayed,
            # Aporta sequences.last_sequence como elemento de la lista.
            sequences.last_sequence,
            # Aporta sequences.skipped_frames como elemento de la lista.
            sequences.skipped_frames,
            # Aporta final.overwritten_frames como elemento de la lista.
            final.overwritten_frames,
            # Aporta final.buffer_utilization * 100.0 como elemento de la lista.
            final.buffer_utilization * 100.0,
            # Ejecuta _consumer_fps(samples),.
            _consumer_fps(samples),
            # Aporta mode como elemento de la lista.
            mode,
            # Aporta final.producer_fps como elemento de la lista.
            final.producer_fps,
            # Ejecuta latency_total / max(1, displayed),.
            latency_total / max(1, displayed),
            # Aporta pipeline.total_detections como elemento de la lista.
            pipeline.total_detections,
        )
    )
    # Devuelve 0 if displayed > 0 else 3.
    return 0 if displayed > 0 else 3


# Define main con el parámetro argv.
def main(argv: Optional[Sequence[str]] = None) -> int:
    # Obtiene args con _parser().parse_args(argv).
    args = _parser().parse_args(argv)
    # Ejecuta el bloque con manejo de excepciones.
    try:
        # Devuelve run(args).
        return run(args)
    # Maneja (FrameDecodeError, RuntimeError, TimeoutError, ValueError).
    except (FrameDecodeError, RuntimeError, TimeoutError, ValueError) as error:
        # Muestra información en la salida mediante print con '[ORV-IPC-501] {}'.format(error).
        print("[ORV-IPC-501] {}".format(error), file=sys.stderr)
        # Devuelve 2.
        return 2
    # Maneja KeyboardInterrupt.
    except KeyboardInterrupt:
        # Devuelve 130.
        return 130
    # Ejecuta la limpieza al salir del bloque, incluso si hubo un error.
    finally:
        # Ejecuta cv2.destroyAllWindows().
        cv2.destroyAllWindows()


# Entra en esta rama si __name__ == '__main__'.
if __name__ == "__main__":
    # Interrumpe la operación con SystemExit(main()).
    raise SystemExit(main())
