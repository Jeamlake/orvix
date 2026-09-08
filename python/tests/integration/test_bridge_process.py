import os
import subprocess
import sys
import time
from pathlib import Path

import pytest


def _executable(root: Path) -> Path:
    if os.name == "nt":
        return root / "build" / "native" / "Release" / "orvix-capture.exe"
    return root / "build" / "native" / "orvix-capture"


@pytest.mark.integration
def test_synthetic_native_producer_reaches_python_consumer():
    root = Path(__file__).resolve().parents[3]
    executable = _executable(root)
    if not executable.exists():
        raise AssertionError("Native ORVIX executable was not built: {}".format(executable))

    name = "orvix-it-{}-{}".format(os.getpid(), int(time.time()) % 100000)
    producer = subprocess.Popen(
        [str(executable), "bridge", "--synthetic", "--frames", "45", "--name", name],
        cwd=str(root),
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
    )
    try:
        consumer = subprocess.run(
            [
                sys.executable,
                "-m",
                "orvix.ui.viewer",
                "--name",
                name,
                "--headless",
                "--frames",
                "10",
                "--attach-timeout",
                "5",
            ],
            cwd=str(root),
            capture_output=True,
            text=True,
            timeout=10,
        )
        producer_output = producer.communicate(timeout=10)[0]
    finally:
        if producer.poll() is None:
            producer.terminate()
            producer.wait(timeout=5)

    assert consumer.returncode == 0, consumer.stdout + consumer.stderr
    assert "Viewer status: COMPLETED" in consumer.stdout
    assert "frames=10" in consumer.stdout
    assert producer.returncode == 0, producer_output
    assert "Bridge status: COMPLETED" in producer_output
