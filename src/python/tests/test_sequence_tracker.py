# Importa SequenceTracker desde orvix.metrics.
from orvix.metrics import SequenceTracker


# Comprueba counts sequence gaps en la suite de pruebas.
def test_counts_sequence_gaps():
    # Obtiene tracker con SequenceTracker().
    tracker = SequenceTracker()

    # Verifica que se cumpla tracker.observe(1) == 0.
    assert tracker.observe(1) == 0
    # Verifica que se cumpla tracker.observe(2) == 0.
    assert tracker.observe(2) == 0
    # Verifica que se cumpla tracker.observe(5) == 2.
    assert tracker.observe(5) == 2
    # Verifica que se cumpla tracker.last_sequence == 5.
    assert tracker.last_sequence == 5
    # Verifica que se cumpla tracker.skipped_frames == 2.
    assert tracker.skipped_frames == 2
