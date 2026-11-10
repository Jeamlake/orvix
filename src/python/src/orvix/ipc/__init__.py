"""ORVIX module."""
"""Shared-memory transport for ORVIX video frames."""

from .protocol import DEFAULT_SHARED_MEMORY_NAME, ProtocolError
from .shared_memory_reader import SharedMemoryReader

__all__ = [
    "DEFAULT_SHARED_MEMORY_NAME",
    "ProtocolError",
    "SharedMemoryReader",
]
