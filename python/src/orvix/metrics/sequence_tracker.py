"""Frame sequence telemetry shared by viewer modes."""

from dataclasses import dataclass


@dataclass
class SequenceTracker:
    last_sequence: int = 0
    skipped_frames: int = 0

    def observe(self, sequence: int) -> int:
        """Record a sequence and return newly detected skipped frames."""

        if sequence <= 0:
            raise ValueError("Frame sequence must be positive.")
        skipped = 0
        if self.last_sequence and sequence > self.last_sequence + 1:
            skipped = sequence - self.last_sequence - 1
            self.skipped_frames += skipped
        if sequence > self.last_sequence:
            self.last_sequence = sequence
        return skipped
