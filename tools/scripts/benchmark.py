"""Run a bounded native synthetic capture in each vision mode."""

import argparse
import json
import platform
import subprocess
import sys
import tempfile
import uuid
from pathlib import Path

import cv2
import numpy

from orvix import __version__
from orvix.vision.pipeline import MODES


def run_mode(engine, mode, frames):
    name = "orvix-bench-" + uuid.uuid4().hex[:12]
    timeout = frames / 30 + 15
    with tempfile.TemporaryDirectory(prefix="orvix-bench-") as workdir:
        producer = subprocess.Popen(
            [str(engine), "bridge", "--synthetic", "--frames", str(frames), "--name", name],
            cwd=workdir, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True,
        )
        try:
            viewer = subprocess.run(
                [sys.executable, "-m", "orvix.ui.viewer", "--headless", "--name", name, "--mode", mode],
                cwd=workdir, capture_output=True, text=True, timeout=timeout, check=True,
            )
            output, _ = producer.communicate(timeout=timeout)
            if producer.returncode:
                raise RuntimeError(output)
        finally:
            if producer.poll() is None:
                producer.kill()
                producer.communicate()
    prefix = "Viewer status: COMPLETED "
    line = next(line for line in viewer.stdout.splitlines() if line.startswith(prefix))
    return dict(field.split("=", 1) for field in line[len(prefix):].split())


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("engine", type=Path)
    parser.add_argument("--frames", default=180, type=int)
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()
    if not args.engine.is_file() or args.frames < 60:
        parser.error("Provide an existing native executable and at least 60 frames.")
    report = {
        "orvix": __version__, "platform": platform.platform(),
        "python": platform.python_version(), "opencv": cv2.__version__, "numpy": numpy.__version__,
        "source": "synthetic NV12 1280x720 30 FPS", "frames_per_run": args.frames,
        "modes": [run_mode(args.engine.resolve(), mode, args.frames) for mode in MODES],
    }
    result = json.dumps(report, indent=2) + "\n"
    if args.output:
        args.output.write_text(result, encoding="utf-8")
    print(result, end="")


if __name__ == "__main__":
    main()
