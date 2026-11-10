"""ORVIX module."""
"""Shared-memory transport for ORVIX video frames."""

# Importa DEFAULT_SHARED_MEMORY_NAME, ProtocolError desde .protocol.
from .protocol import DEFAULT_SHARED_MEMORY_NAME, ProtocolError
# Importa SharedMemoryReader desde .shared_memory_reader.
from .shared_memory_reader import SharedMemoryReader

# Inicializa __all__ con ['DEFAULT_SHARED_MEMORY_NAME', 'ProtocolError', 'SharedMemoryReader'].
__all__ = [
    # Aporta el texto "DEFAULT_SHARED_MEMORY_NAME" a la expresión.
    "DEFAULT_SHARED_MEMORY_NAME",
    # Aporta el texto "ProtocolError" a la expresión.
    "ProtocolError",
    # Aporta el texto "SharedMemoryReader" a la expresión.
    "SharedMemoryReader",
]
