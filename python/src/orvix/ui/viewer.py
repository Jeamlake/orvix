"""Live OpenCV viewer for frames produced by the native ORVIX process."""

from __future__ import annotations

import argparse
import sys
import time
from collections import deque
from typing import Deque, Optional, Sequence

import cv2

from orvix.ipc import DEFAULT_SHARED_MEMORY_NAME, SharedMemoryReader
from orvix.ipc.protocol import PRODUCER_FAILED, PRODUCER_STOPPED
from orvix.metrics import SequenceTracker
from orvix.vision import FrameDecodeError, decode_bgr


def _parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description="Display raw frames produced by the native ORVIX bridge."
    )
    parser.add_argument("--name", default=DEFAULT_SHARED_MEMORY_NAME)
    parser.add_argument("--frames", type=int, default=0)
    parser.add_argument("--attach-timeout", type=float, default=10.0)
    parser.add_argument("--poll-ms", type=float, default=2.0)
    parser.add_argument(
        "--headless",
        action="store_true",
        help="Validate frames without creating a window.",
    )
    return parser


def _consumer_fps(samples: Deque[float]) -> float:
    if len(samples) < 2:
        return 0.0
    elapsed = samples[-1] - samples[0]
    return (len(samples) - 1) / elapsed if elapsed > 0 else 0.0


def run(args: argparse.Namespace) -> int:
    if args.frames < 0 or args.poll_ms < 0:
        raise ValueError("--frames and --poll-ms cannot be negative")

    displayed = 0
    sequences = SequenceTracker()
    samples: Deque[float] = deque(maxlen=120)

    with SharedMemoryReader(args.name, args.attach_timeout) as reader:
        initial = reader.header
        target_fps = (
            initial.fps_numerator / initial.fps_denominator
            if initial.fps_denominator
            else 0.0
        )
        print(
            "ORVIX viewer connected: name={} producer_pid={} "
            "format={} resolution={}x{} target_fps={:.2f}".format(
                args.name,
                initial.producer_pid,
                initial.pixel_format,
                initial.width,
                initial.height,
                target_fps,
            ),
            flush=True,
        )

        while True:
            header = reader.header
            frame = reader.read_latest()
            if frame is not None and frame.sequence != sequences.last_sequence:
                sequences.observe(frame.sequence)
                image = decode_bgr(frame)
                reader.acknowledge(sequences.last_sequence)
                displayed += 1
                samples.append(time.monotonic())

                if not args.headless:
                    overlay = (
                        "seq={} {}x{} {} producer={:.1f}fps "
                        "viewer={:.1f}fps skipped={}".format(
                            frame.sequence,
                            frame.width,
                            frame.height,
                            frame.pixel_format,
                            header.producer_fps,
                            _consumer_fps(samples),
                            sequences.skipped_frames,
                        )
                    )
                    cv2.putText(
                        image,
                        overlay,
                        (12, 28),
                        cv2.FONT_HERSHEY_SIMPLEX,
                        0.6,
                        (0, 255, 0),
                        2,
                        cv2.LINE_AA,
                    )
                    cv2.imshow("ORVIX Live - RAW", image)
                    key = cv2.waitKey(1) & 0xFF
                    if key in (27, ord("q")):
                        break

                if args.frames and displayed >= args.frames:
                    break

            if header.producer_state in (PRODUCER_STOPPED, PRODUCER_FAILED):
                if frame is None or frame.sequence == sequences.last_sequence:
                    break
            if args.poll_ms:
                time.sleep(args.poll_ms / 1000.0)

        final = reader.header

    if not args.headless:
        cv2.destroyAllWindows()
    print(
        "Viewer status: COMPLETED frames={} first_to_last={} skipped={} "
        "producer_overwrites={} buffer_utilization={:.1f}% viewer_fps={:.2f}".format(
            displayed,
            sequences.last_sequence,
            sequences.skipped_frames,
            final.overwritten_frames,
            final.buffer_utilization * 100.0,
            _consumer_fps(samples),
        )
    )
    return 0 if displayed > 0 else 3


def main(argv: Optional[Sequence[str]] = None) -> int:
    args = _parser().parse_args(argv)
    try:
        return run(args)
    except (FrameDecodeError, RuntimeError, TimeoutError, ValueError) as error:
        print("[ORV-IPC-501] {}".format(error), file=sys.stderr)
        return 2
    except KeyboardInterrupt:
        return 130
    finally:
        cv2.destroyAllWindows()


if __name__ == "__main__":
    raise SystemExit(main())
