# Importa numpy como np.
import numpy as np

# Importa SharedFrame desde orvix.ipc.protocol.
from orvix.ipc.protocol import SharedFrame
# Importa decode_bgr desde orvix.vision.
from orvix.vision import decode_bgr


# Comprueba decodes nv12 to owned bgr array en la suite de pruebas.
def test_decodes_nv12_to_owned_bgr_array():
    # Obtiene payload (carga útil del fotograma) con bytes([64] * 8 + [128] * 4).
    payload = bytes([64] * 8 + [128] * 4)
    # Obtiene frame (fotograma) con SharedFrame(sequence=1, timestamp_100ns=333333, width=4, height=2, s…
    frame = SharedFrame(
        # Indica sequence (secuencia del fotograma) con valor 1.
        sequence=1,
        # Indica timestamp_100ns (marca de tiempo en unidades de 100 ns) con valor 333_333.
        timestamp_100ns=333_333,
        # Indica width (ancho en píxeles) con valor 4.
        width=4,
        # Indica height (alto en píxeles) con valor 2.
        height=2,
        # Indica stride (bytes entre filas de imagen) con valor 4.
        stride=4,
        # Indica pixel_format (formato de píxeles) con valor "NV12".
        pixel_format="NV12",
        # Indica payload (carga útil del fotograma) con valor payload.
        payload=payload,
    )

    # Guarda en image (imagen) el resultado de decode_bgr, que decodifica el fotograma como imagen BGR.
    image = decode_bgr(frame)

    # Verifica que se cumpla image.shape == (2, 4, 3).
    assert image.shape == (2, 4, 3)
    # Verifica que se cumpla image.dtype == np.uint8.
    assert image.dtype == np.uint8
    # Verifica que se cumpla image.flags['C_CONTIGUOUS'].
    assert image.flags["C_CONTIGUOUS"]
