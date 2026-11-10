"""Python consumer for the C++ shared-memory producer."""

# Importa annotations desde __future__.
from __future__ import annotations

# Importa os.
import os
# Importa struct.
import struct
# Importa time.
import time
# Importa resource_tracker, shared_memory desde multiprocessing.
from multiprocessing import resource_tracker, shared_memory
# Importa Optional desde typing.
from typing import Optional

# Importa SharedFrame, SharedHeader, acknowledge_frame, read_header desde .protocol.
from .protocol import SharedFrame, SharedHeader, acknowledge_frame, read_header
# Importa read_latest_frame desde .protocol.
from .protocol import read_latest_frame


# Declara el tipo SharedMemoryReader para agrupar su estado y operaciones.
class SharedMemoryReader:
    """Attach to a producer without opening or controlling the camera."""

    # Inicializa la instancia con name, attach_timeout.
    def __init__(self, name: str, attach_timeout: float = 10.0) -> None:
        # Entra en esta rama si attach_timeout < 0.
        if attach_timeout < 0:
            # Interrumpe la operación con ValueError('attach_timeout cannot be negative').
            raise ValueError("attach_timeout cannot be negative")
        # Establece deadline a partir de time.monotonic() + attach_timeout.
        deadline = time.monotonic() + attach_timeout
        # Repite mientras True.
        while True:
            # Ejecuta el bloque con manejo de excepciones.
            try:
                # Obtiene self._shared con shared_memory.SharedMemory(name=name).
                self._shared = shared_memory.SharedMemory(name=name)
                # Termina el bucle actual.
                break
            # Maneja FileNotFoundError.
            except FileNotFoundError:
                # Entra en esta rama si time.monotonic() >= deadline.
                if time.monotonic() >= deadline:
                    # Interrumpe la operación con TimeoutError("ORVIX producer '{}' was not found within {:.1f} s.".format(name, attach_t…
                    raise TimeoutError(
                        # Ejecuta "ORVIX producer '{}' was not found within {:.1f} s.".format(.
                        "ORVIX producer '{}' was not found within {:.1f} s.".format(
                            # Incorpora name, attach_timeout en esta operación.
                            name, attach_timeout
                        )
                    )
                # Ejecuta time.sleep(0.05).
                time.sleep(0.05)

        # Establece self.name a partir de name.
        self.name = name
        # Establece self._closed a partir de False.
        self._closed = False
        # Entra en esta rama si os.name != 'nt'.
        if os.name != "nt":
            # Ejecuta el bloque con manejo de excepciones.
            try:
                # Ejecuta resource_tracker.unregister(self._shared._name, "shared_memory").
                resource_tracker.unregister(self._shared._name, "shared_memory")
            # Maneja (AttributeError, KeyError).
            except (AttributeError, KeyError):
                # Deja el bloque sin acciones.
                pass
        # Ejecuta el bloque con manejo de excepciones.
        try:
            # Ejecuta read_header(self._shared.buf).
            read_header(self._shared.buf)
        # Maneja Exception.
        except Exception:
            # Cierra el recurso mediante self.close.
            self.close()
            # Vuelve a propagar la excepción en curso.
            raise

    # Aplica el decorador property a la definición siguiente.
    @property
    # Define header sin parámetros de llamada.
    def header(self) -> SharedHeader:
        # Devuelve read_header(self._shared.buf).
        return read_header(self._shared.buf)

    # Define read_latest sin parámetros de llamada.
    def read_latest(self) -> Optional[SharedFrame]:
        # Devuelve read_latest_frame(self._shared.buf).
        return read_latest_frame(self._shared.buf)

    # Define acknowledge con el parámetro sequence.
    def acknowledge(self, sequence: int) -> None:
        # Ejecuta acknowledge_frame(self._shared.buf, sequence).
        acknowledge_frame(self._shared.buf, sequence)

    # Define request_stop sin parámetros de llamada.
    def request_stop(self) -> None:
        """Explicitly stop a producer supporting the IPC v1 reserved extension."""
        # Ejecuta struct.pack_into("<I", self._shared.buf, 120, 1).
        struct.pack_into("<I", self._shared.buf, 120, 1)

    # Define close, que cierra el recurso.
    def close(self) -> None:
        # Entra en esta rama si not self._closed.
        if not self._closed:
            # Cierra el recurso mediante self._shared.close.
            self._shared.close()
            # Establece self._closed a partir de True.
            self._closed = True

    # Define __enter__ sin parámetros de llamada.
    def __enter__(self) -> "SharedMemoryReader":
        # Devuelve self.
        return self

    # Define __exit__ sin parámetros de llamada.
    def __exit__(self, *_args: object) -> None:
        # Cierra el recurso mediante self.close.
        self.close()
