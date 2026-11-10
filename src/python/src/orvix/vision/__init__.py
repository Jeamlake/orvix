"""ORVIX module."""
"""Video decoding helpers."""

# Importa FrameDecodeError, decode_bgr desde .frame_decoder.
from .frame_decoder import FrameDecodeError, decode_bgr

# Inicializa __all__ con ['FrameDecodeError', 'decode_bgr'].
__all__ = ["FrameDecodeError", "decode_bgr"]
