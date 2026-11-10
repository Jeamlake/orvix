# Importa os.
import os
# Importa subprocess.
import subprocess
# Importa sys.
import sys
# Importa time.
import time
# Importa uuid.
import uuid
# Importa Path desde pathlib.
from pathlib import Path

# Importa pytest.
import pytest


# Define _executable con el parámetro root.
def _executable(root: Path) -> Path:
    # Entra en esta rama si os.name == 'nt'.
    if os.name == "nt":
        # Devuelve root / 'build' / 'native' / 'Release' / 'orvix-capture.exe'.
        return root / "build" / "native" / "Release" / "orvix-capture.exe"
    # Devuelve root / 'build' / 'native' / 'orvix-capture'.
    return root / "build" / "native" / "orvix-capture"


# Aplica el decorador pytest.mark.integration a la definición siguiente.
@pytest.mark.integration
# Comprueba synthetic native producer reaches python consumer en la suite de pruebas.
def test_synthetic_native_producer_reaches_python_consumer():
    # Establece root a partir de Path(__file__).resolve().parents[4].
    root = Path(__file__).resolve().parents[4]
    # Obtiene executable con _executable(root).
    executable = _executable(root)
    # Entra en esta rama si not executable.exists().
    if not executable.exists():
        # Interrumpe la operación con AssertionError('Native ORVIX executable was not built: {}'.format(executable)).
        raise AssertionError("Native ORVIX executable was not built: {}".format(executable))

    # Obtiene name con 'orvix-it-{}-{}'.format(os.getpid(), int(time.time()) % 100000).
    name = "orvix-it-{}-{}".format(os.getpid(), int(time.time()) % 100000)
    # Obtiene producer con subprocess.Popen([str(executable), 'bridge', '--synthetic', '--frame…
    producer = subprocess.Popen(
        # Ejecuta [str(executable), "bridge", "--synthetic", "--frames", "45", "--name", name],.
        [str(executable), "bridge", "--synthetic", "--frames", "45", "--name", name],
        # Indica cwd con valor str(root).
        cwd=str(root),
        # Indica stdout con valor subprocess.PIPE.
        stdout=subprocess.PIPE,
        # Indica stderr con valor subprocess.STDOUT.
        stderr=subprocess.STDOUT,
        # Indica text con valor True.
        text=True,
    )
    # Ejecuta el bloque con manejo de excepciones.
    try:
        # Obtiene consumer con subprocess.run([sys.executable, '-m', 'orvix.ui.viewer', '--name', n…
        consumer = subprocess.run(
            [
                # Aporta sys.executable como elemento de la lista.
                sys.executable,
                # Aporta el texto "-m" a la expresión.
                "-m",
                # Aporta el texto "orvix.ui.viewer" a la expresión.
                "orvix.ui.viewer",
                # Aporta el texto "--name" a la expresión.
                "--name",
                # Aporta name como elemento de la lista.
                name,
                # Aporta el texto "--headless" a la expresión.
                "--headless",
                # Aporta el texto "--frames" a la expresión.
                "--frames",
                # Aporta el texto "10" a la expresión.
                "10",
                # Aporta el texto "--attach-timeout" a la expresión.
                "--attach-timeout",
                # Aporta el texto "5" a la expresión.
                "5",
            ],
            # Indica cwd con valor str(root).
            cwd=str(root),
            # Indica capture_output con valor True.
            capture_output=True,
            # Indica text con valor True.
            text=True,
            # Indica timeout (tiempo máximo de espera) con valor 10.
            timeout=10,
        )
        # Establece producer_output a partir de producer.communicate(timeout=10)[0].
        producer_output = producer.communicate(timeout=10)[0]
    # Ejecuta la limpieza al salir del bloque, incluso si hubo un error.
    finally:
        # Entra en esta rama si producer.poll() is None.
        if producer.poll() is None:
            # Ejecuta producer.terminate().
            producer.terminate()
            # Ejecuta producer.wait(timeout=5).
            producer.wait(timeout=5)

    # Verifica que se cumpla consumer.returncode == 0.
    assert consumer.returncode == 0, consumer.stdout + consumer.stderr
    # Verifica que se cumpla 'Viewer status: COMPLETED' in consumer.stdout.
    assert "Viewer status: COMPLETED" in consumer.stdout
    # Verifica que se cumpla 'frames=10' in consumer.stdout.
    assert "frames=10" in consumer.stdout
    # Verifica que se cumpla producer.returncode == 0.
    assert producer.returncode == 0, producer_output
    # Verifica que se cumpla 'Bridge status: COMPLETED' in producer_output.
    assert "Bridge status: COMPLETED" in producer_output


# Aplica el decorador pytest.mark.integration a la definición siguiente.
@pytest.mark.integration
# Comprueba continuous bridge stops only on explicit owner request en la suite de pruebas.
def test_continuous_bridge_stops_only_on_explicit_owner_request():
    # Importa SharedMemoryReader desde orvix.ipc.
    from orvix.ipc import SharedMemoryReader

    # Establece root a partir de Path(__file__).resolve().parents[4].
    root = Path(__file__).resolve().parents[4]
    # Establece name a partir de 'orvix-stop-' + uuid.uuid4().hex[:16].
    name = "orvix-stop-" + uuid.uuid4().hex[:16]
    # Obtiene producer con subprocess.Popen([str(_executable(root)), 'bridge', '--synthetic', '…
    producer = subprocess.Popen([str(_executable(root)), "bridge", "--synthetic",
                                 # Ejecuta "--continuous", "--name", name], cwd=str(root),.
                                 "--continuous", "--name", name], cwd=str(root),
                                # Indica stdout con valor subprocess.PIPE, stderr=subprocess.STDOUT, text=True).
                                stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
    # Ejecuta el bloque con manejo de excepciones.
    try:
        # Acota el uso de SharedMemoryReader(name, 5) a este bloque.
        with SharedMemoryReader(name, 5) as reader:
            # Establece deadline a partir de time.monotonic() + 3.
            deadline = time.monotonic() + 3
            # Repite mientras reader.header.published_sequence < 5 and time.monotonic() < deadline.
            while reader.header.published_sequence < 5 and time.monotonic() < deadline:
                # Ejecuta time.sleep(0.01).
                time.sleep(0.01)
            # Verifica que se cumpla reader.header.published_sequence >= 5.
            assert reader.header.published_sequence >= 5
        # Detaching the consumer must not stop Capture (RF-024).
        # Ejecuta time.sleep(0.1).
        time.sleep(0.1)
        # Verifica que se cumpla producer.poll() is None.
        assert producer.poll() is None
        # Acota el uso de SharedMemoryReader(name, 0) a este bloque.
        with SharedMemoryReader(name, 0) as reader:
            # Ejecuta reader.request_stop().
            reader.request_stop()
        # Establece output a partir de producer.communicate(timeout=5)[0].
        output = producer.communicate(timeout=5)[0]
        # Verifica que se cumpla producer.returncode == 0.
        assert producer.returncode == 0, output
        # Verifica que se cumpla 'Bridge status: COMPLETED' in output.
        assert "Bridge status: COMPLETED" in output
    # Ejecuta la limpieza al salir del bloque, incluso si hubo un error.
    finally:
        # Entra en esta rama si producer.poll() is None.
        if producer.poll() is None:
            # Ejecuta producer.terminate().
            producer.terminate()
            # Ejecuta producer.wait(timeout=5).
            producer.wait(timeout=5)
