"""Frame sequence telemetry shared by viewer modes."""

# Importa dataclass desde dataclasses.
from dataclasses import dataclass


# Aplica el decorador dataclass a la definición siguiente.
@dataclass
# Declara el tipo SequenceTracker para agrupar su estado y operaciones.
class SequenceTracker:
    # Establece last_sequence (última secuencia observada) a partir de 0.
    last_sequence: int = 0
    # Establece skipped_frames (fotogramas omitidos acumulados) a partir de 0.
    skipped_frames: int = 0

    # Define observe con el parámetro sequence.
    def observe(self, sequence: int) -> int:
        """Record a sequence and return newly detected skipped frames."""

        # Entra en esta rama si sequence <= 0.
        if sequence <= 0:
            # Interrumpe la operación con ValueError('Frame sequence must be positive.').
            raise ValueError("Frame sequence must be positive.")
        # Establece skipped (fotogramas omitidos) a partir de 0.
        skipped = 0
        # Entra en esta rama si self.last_sequence and sequence > self.last_sequence + 1.
        if self.last_sequence and sequence > self.last_sequence + 1:
            # Establece skipped (fotogramas omitidos) a partir de sequence - self.last_sequence - 1.
            skipped = sequence - self.last_sequence - 1
            # Suma skipped al valor de self.skipped_frames (fotogramas omitidos acumulados).
            self.skipped_frames += skipped
        # Entra en esta rama si sequence > self.last_sequence.
        if sequence > self.last_sequence:
            # Establece self.last_sequence (última secuencia observada) a partir de sequence.
            self.last_sequence = sequence
        # Devuelve skipped.
        return skipped
