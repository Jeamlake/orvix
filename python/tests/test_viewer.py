from argparse import Namespace
from types import SimpleNamespace

from orvix.ipc.protocol import PRODUCER_RUNNING, SharedFrame
from orvix.ui import viewer


class FakeReader:
    def __init__(self, *_args, **_kwargs):
        self._frame = SharedFrame(
            sequence=1,
            timestamp_100ns=333_333,
            width=4,
            height=2,
            stride=4,
            pixel_format="NV12",
            payload=bytes([64] * 8 + [128] * 4),
        )
        self.acknowledged = 0

    @property
    def header(self):
        return SimpleNamespace(
            fps_numerator=30,
            fps_denominator=1,
            producer_pid=42,
            pixel_format="NV12",
            width=4,
            height=2,
            producer_fps=30.0,
            producer_state=PRODUCER_RUNNING,
            overwritten_frames=0,
            buffer_utilization=0.0,
        )

    def read_latest(self):
        return self._frame

    def acknowledge(self, sequence):
        self.acknowledged = sequence

    def __enter__(self):
        return self

    def __exit__(self, *_args):
        return None


def test_raw_mode_displays_decoded_frame(monkeypatch):
    displayed = []
    monkeypatch.setattr(viewer, "SharedMemoryReader", FakeReader)
    monkeypatch.setattr(viewer.cv2, "imshow", lambda title, image: displayed.append((title, image)))
    monkeypatch.setattr(viewer.cv2, "waitKey", lambda _delay: ord("q"))
    monkeypatch.setattr(viewer.cv2, "destroyAllWindows", lambda: None)

    result = viewer.run(
        Namespace(
            name="orvix-test",
            frames=0,
            attach_timeout=0,
            poll_ms=0,
            headless=False,
        )
    )

    assert result == 0
    assert displayed[0][0] == "ORVIX Live - RAW"
    assert displayed[0][1].shape == (2, 4, 3)
