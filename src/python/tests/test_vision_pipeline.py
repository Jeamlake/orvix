# Importa numpy como np.
import numpy as np

# Importa VisionPipeline desde orvix.vision.pipeline.
from orvix.vision.pipeline import VisionPipeline


# Comprueba modes and motion detection on known frames en la suite de pruebas.
def test_modes_and_motion_detection_on_known_frames():
    # Obtiene pipeline con VisionPipeline().
    pipeline = VisionPipeline()
    # Obtiene static con np.zeros((96, 96, 3), dtype=np.uint8).
    static = np.zeros((96, 96, 3), dtype=np.uint8)
    # Obtiene moving con static.copy().
    moving = static.copy()
    # Inicializa moving[20:60, 35:75] con (20, 100, 230).
    moving[20:60, 35:75] = (20, 100, 230)
    # Verifica que se cumpla pipeline.process(moving, 'raw') is moving.
    assert pipeline.process(moving, "raw") is moving

    # Obtiene gray con pipeline.process(moving, 'gray').
    gray = pipeline.process(moving, "gray")
    # Verifica que se cumpla gray.shape == moving.shape.
    assert gray.shape == moving.shape
    # Verifica que se cumpla 0 < gray[40, 55, 0] < 230.
    assert 0 < gray[40, 55, 0] < 230
    # Verifica que se cumpla np.array_equal(gray[:, :, 0], gray[:, :, 2]).
    assert np.array_equal(gray[:, :, 0], gray[:, :, 2])
    # Obtiene edges con pipeline.process(moving, 'edges').
    edges = pipeline.process(moving, "edges")
    # Verifica que se cumpla edges.max() == 255.
    assert edges.max() == 255
    # Verifica que se cumpla edges[40, 55].max() == 0.
    assert edges[40, 55].max() == 0

    # Verifica que se cumpla np.array_equal(pipeline.process(static.copy(), 'motion'), static).
    assert np.array_equal(pipeline.process(static.copy(), "motion"), static)
    # Verifica que se cumpla pipeline.total_detections == 0.
    assert pipeline.total_detections == 0
    # Obtiene annotated con pipeline.process(moving.copy(), 'motion').
    annotated = pipeline.process(moving.copy(), "motion")
    # Verifica que se cumpla not np.array_equal(annotated, moving).
    assert not np.array_equal(annotated, moving)
    # Verifica que se cumpla pipeline.total_detections == len(pipeline.detection_times) == 1.
    assert pipeline.total_detections == len(pipeline.detection_times) == 1
    # Ejecuta pipeline.process(static, "raw").
    pipeline.process(static, "raw")
    # Verifica que se cumpla np.array_equal(pipeline.process(static.copy(), 'motion'), static).
    assert np.array_equal(pipeline.process(static.copy(), "motion"), static)
    # Verifica que se cumpla pipeline.total_detections == 1.
    assert pipeline.total_detections == 1
