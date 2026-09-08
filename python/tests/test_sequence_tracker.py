from orvix.metrics import SequenceTracker


def test_counts_sequence_gaps():
    tracker = SequenceTracker()

    assert tracker.observe(1) == 0
    assert tracker.observe(2) == 0
    assert tracker.observe(5) == 2
    assert tracker.last_sequence == 5
    assert tracker.skipped_frames == 2
