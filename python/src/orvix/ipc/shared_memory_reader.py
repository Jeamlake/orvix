"""Python consumer for the C++ shared-memory producer."""

from __future__ import annotations

import os
import time
from multiprocessing import resource_tracker, shared_memory
from typing import Optional

from .protocol import SharedFrame, SharedHeader, acknowledge_frame, read_header
from .protocol import read_latest_frame


class SharedMemoryReader:
    """Attach to a producer without opening or controlling the camera."""

    def __init__(self, name: str, attach_timeout: float = 10.0) -> None:
        if attach_timeout < 0:
            raise ValueError("attach_timeout cannot be negative")
        deadline = time.monotonic() + attach_timeout
        while True:
            try:
                self._shared = shared_memory.SharedMemory(name=name)
                break
            except FileNotFoundError:
                if time.monotonic() >= deadline:
                    raise TimeoutError(
                        "ORVIX producer '{}' was not found within {:.1f} s.".format(
                            name, attach_timeout
                        )
                    )
                time.sleep(0.05)

        self.name = name
        self._closed = False
        if os.name != "nt":
            try:
                resource_tracker.unregister(self._shared._name, "shared_memory")
            except (AttributeError, KeyError):
                pass
        read_header(self._shared.buf)

    @property
    def header(self) -> SharedHeader:
        return read_header(self._shared.buf)

    def read_latest(self) -> Optional[SharedFrame]:
        return read_latest_frame(self._shared.buf)

    def acknowledge(self, sequence: int) -> None:
        acknowledge_frame(self._shared.buf, sequence)

    def close(self) -> None:
        if not self._closed:
            self._shared.close()
            self._closed = True

    def __enter__(self) -> "SharedMemoryReader":
        return self

    def __exit__(self, *_args: object) -> None:
        self.close()
