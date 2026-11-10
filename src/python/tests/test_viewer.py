# Importa Namespace desde argparse.
from argparse import Namespace
# Importa SimpleNamespace desde types.
from types import SimpleNamespace

# Importa PRODUCER_RUNNING, SharedFrame desde orvix.ipc.protocol.
from orvix.ipc.protocol import PRODUCER_RUNNING, SharedFrame
# Importa viewer desde orvix.ui.
from orvix.ui import viewer


# Declara el tipo FakeReader para agrupar su estado y operaciones.
class FakeReader:
    # Inicializa la instancia y prepara su estado.
    def __init__(self, *_args, **_kwargs):
        # Obtiene self._frame (fotograma) con SharedFrame(sequence=1, timestamp_100ns=333333, width=4, height=2, s…
        self._frame = SharedFrame(
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
            # Indica payload (carga útil del fotograma) con valor bytes([64] * 8 + [128] * 4).
            payload=bytes([64] * 8 + [128] * 4),
        )
        # Establece self.acknowledged a partir de 0.
        self.acknowledged = 0

    # Aplica el decorador property a la definición siguiente.
    @property
    # Define header sin parámetros de llamada.
    def header(self):
        # Devuelve SimpleNamespace(fps_numerator=30, fps_denominator=1, producer_pid=42, pixel_forma…
        return SimpleNamespace(
            # Indica fps_numerator con valor 30.
            fps_numerator=30,
            # Indica fps_denominator con valor 1.
            fps_denominator=1,
            # Indica producer_pid con valor 42.
            producer_pid=42,
            # Indica pixel_format (formato de píxeles) con valor "NV12".
            pixel_format="NV12",
            # Indica width (ancho en píxeles) con valor 4.
            width=4,
            # Indica height (alto en píxeles) con valor 2.
            height=2,
            # Indica producer_fps con valor 30.0.
            producer_fps=30.0,
            # Indica producer_state con valor PRODUCER_RUNNING.
            producer_state=PRODUCER_RUNNING,
            # Indica overwritten_frames con valor 0.
            overwritten_frames=0,
            # Indica buffer_utilization con valor 0.0.
            buffer_utilization=0.0,
        )

    # Define read_latest sin parámetros de llamada.
    def read_latest(self):
        # Devuelve self._frame.
        return self._frame

    # Define acknowledge con el parámetro sequence.
    def acknowledge(self, sequence):
        # Establece self.acknowledged a partir de sequence.
        self.acknowledged = sequence

    # Define __enter__ sin parámetros de llamada.
    def __enter__(self):
        # Devuelve self.
        return self

    # Define __exit__ sin parámetros de llamada.
    def __exit__(self, *_args):
        # Devuelve None.
        return None


# Comprueba raw mode displays decoded frame en la suite de pruebas.
def test_raw_mode_displays_decoded_frame(monkeypatch):
    # Inicializa displayed con [].
    displayed = []
    # Ejecuta monkeypatch.setattr(viewer, "SharedMemoryReader", FakeReader).
    monkeypatch.setattr(viewer, "SharedMemoryReader", FakeReader)
    # Agrega un elemento al final mediante monkeypatch.setattr.
    monkeypatch.setattr(viewer.cv2, "imshow", lambda title, image: displayed.append((title, image)))
    # Ejecuta monkeypatch.setattr(viewer.cv2, "waitKey", lambda _delay: ord("q")).
    monkeypatch.setattr(viewer.cv2, "waitKey", lambda _delay: ord("q"))
    # Ejecuta monkeypatch.setattr(viewer.cv2, "destroyAllWindows", lambda: None).
    monkeypatch.setattr(viewer.cv2, "destroyAllWindows", lambda: None)

    # Obtiene result con viewer.run(Namespace(name='orvix-test', frames=0, attach_timeout=0,…
    result = viewer.run(
        # Ejecuta Namespace(.
        Namespace(
            # Indica name con valor "orvix-test".
            name="orvix-test",
            # Indica frames (fotogramas) con valor 0.
            frames=0,
            # Indica attach_timeout con valor 0.
            attach_timeout=0,
            # Indica poll_ms con valor 0.
            poll_ms=0,
            # Indica headless con valor False.
            headless=False,
        )
    )

    # Verifica que se cumpla result == 0.
    assert result == 0
    # Verifica que se cumpla displayed[0][0] == 'ORVIX Live - RAW'.
    assert displayed[0][0] == "ORVIX Live - RAW"
    # Verifica que se cumpla displayed[0][1].shape == (2, 4, 3).
    assert displayed[0][1].shape == (2, 4, 3)


# Comprueba window events are processed during frame stall en la suite de pruebas.
def test_window_events_are_processed_during_frame_stall(monkeypatch):
    # Declara el tipo StalledReader que hereda de FakeReader.
    class StalledReader(FakeReader):
        # Define read_latest sin parámetros de llamada.
        def read_latest(self):
            # Devuelve None.
            return None

    # Ejecuta monkeypatch.setattr(viewer, "SharedMemoryReader", StalledReader).
    monkeypatch.setattr(viewer, "SharedMemoryReader", StalledReader)
    # Ejecuta monkeypatch.setattr(viewer.cv2, "waitKey", lambda _delay: ord("q")).
    monkeypatch.setattr(viewer.cv2, "waitKey", lambda _delay: ord("q"))
    # Ejecuta monkeypatch.setattr(viewer.cv2, "destroyAllWindows", lambda: None).
    monkeypatch.setattr(viewer.cv2, "destroyAllWindows", lambda: None)
    # Verifica que se cumpla viewer.run(Namespace(name='test', frames=0, attach_timeout=0, poll_ms=0, headless=False…
    assert viewer.run(Namespace(name="test", frames=0, attach_timeout=0,
                               # Indica poll_ms con valor 0, headless=False)) == 3.
                               poll_ms=0, headless=False)) == 3


# Comprueba headless stall is bounded en la suite de pruebas.
def test_headless_stall_is_bounded(monkeypatch):
    # Importa pytest.
    import pytest

    # Declara el tipo StalledReader que hereda de FakeReader.
    class StalledReader(FakeReader):
        # Define read_latest sin parámetros de llamada.
        def read_latest(self):
            # Devuelve None.
            return None

    # Ejecuta monkeypatch.setattr(viewer, "SharedMemoryReader", StalledReader).
    monkeypatch.setattr(viewer, "SharedMemoryReader", StalledReader)
    # Acota el uso de pytest.raises(TimeoutError) a este bloque.
    with pytest.raises(TimeoutError):
        # Ejecuta viewer.run(Namespace(name="test", frames=0, attach_timeout=0,.
        viewer.run(Namespace(name="test", frames=0, attach_timeout=0,
                             # Indica poll_ms con valor 1, headless=True, stall_timeout=0.02)).
                             poll_ms=1, headless=True, stall_timeout=0.02))


# Comprueba attach delay does not consume frame stall timeout en la suite de pruebas.
def test_attach_delay_does_not_consume_frame_stall_timeout(monkeypatch):
    # Inicializa clock con [0.0].
    clock = [0.0]

    # Declara el tipo DelayedReader que hereda de FakeReader.
    class DelayedReader(FakeReader):
        # Define __enter__ sin parámetros de llamada.
        def __enter__(self):
            # Establece clock[0] a partir de 10.0.
            clock[0] = 10.0
            # Establece self.first_poll a partir de True.
            self.first_poll = True
            # Devuelve self.
            return self

        # Define read_latest sin parámetros de llamada.
        def read_latest(self):
            # Entra en esta rama si self.first_poll.
            if self.first_poll:
                # Establece self.first_poll a partir de False.
                self.first_poll = False
                # Devuelve None.
                return None
            # Devuelve super().read_latest().
            return super().read_latest()

    # Ejecuta monkeypatch.setattr(viewer, "SharedMemoryReader", DelayedReader).
    monkeypatch.setattr(viewer, "SharedMemoryReader", DelayedReader)
    # Ejecuta monkeypatch.setattr(viewer.time, "monotonic", lambda: clock[0]).
    monkeypatch.setattr(viewer.time, "monotonic", lambda: clock[0])
    # Verifica que se cumpla viewer.run(Namespace(name='test', frames=1, attach_timeout=15, poll_ms=0, headless=True…
    assert viewer.run(Namespace(name="test", frames=1, attach_timeout=15,
                               # Indica poll_ms con valor 0, headless=True, stall_timeout=5)) == 0.
                               poll_ms=0, headless=True, stall_timeout=5)) == 0
