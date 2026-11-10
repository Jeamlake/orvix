import numpy as np

from orvix.ipc.protocol import SharedFrame
from orvix.vision import decode_bgr


def test_decodes_nv12_to_owned_bgr_array():
    payload = bytes([64] * 8 + [128] * 4)
    frame = SharedFrame(
        sequence=1,
        timestamp_100ns=333_333,
        width=4,
        height=2,
        stride=4,
        pixel_format="NV12",
        payload=payload,
    )

    image = decode_bgr(frame)

    assert image.shape == (2, 4, 3)
    assert image.dtype == np.uint8
    assert image.flags["C_CONTIGUOUS"]
