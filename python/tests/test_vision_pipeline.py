import numpy as np

from orvix.vision.pipeline import VisionPipeline


def test_modes_and_motion_detection_on_known_frames():
    pipeline = VisionPipeline()
    static = np.zeros((96, 96, 3), dtype=np.uint8)
    moving = static.copy()
    moving[20:60, 35:75] = (20, 100, 230)
    assert pipeline.process(moving, "raw") is moving

    gray = pipeline.process(moving, "gray")
    assert gray.shape == moving.shape
    assert 0 < gray[40, 55, 0] < 230
    assert np.array_equal(gray[:, :, 0], gray[:, :, 2])
    edges = pipeline.process(moving, "edges")
    assert edges.max() == 255
    assert edges[40, 55].max() == 0

    assert np.array_equal(pipeline.process(static.copy(), "motion"), static)
    assert pipeline.total_detections == 0
    annotated = pipeline.process(moving.copy(), "motion")
    assert not np.array_equal(annotated, moving)
    assert pipeline.total_detections == len(pipeline.detection_times) == 1
    pipeline.process(static, "raw")
    assert np.array_equal(pipeline.process(static.copy(), "motion"), static)
    assert pipeline.total_detections == 1
