"""Run a bounded native synthetic capture in each vision mode."""

# Importa argparse.
import argparse
# Importa json.
import json
# Importa platform.
import platform
# Importa subprocess.
import subprocess
# Importa sys.
import sys
# Importa tempfile.
import tempfile
# Importa uuid.
import uuid
# Importa Path desde pathlib.
from pathlib import Path

# Importa cv2.
import cv2
# Importa numpy.
import numpy

# Importa __version__ desde orvix.
from orvix import __version__
# Importa MODES desde orvix.vision.pipeline.
from orvix.vision.pipeline import MODES


# Define run_mode con los parámetros engine, mode, frames.
def run_mode(engine, mode, frames):
    # Establece name a partir de 'orvix-bench-' + uuid.uuid4().hex[:12].
    name = "orvix-bench-" + uuid.uuid4().hex[:12]
    # Establece timeout (tiempo máximo de espera) a partir de frames / 30 + 15.
    timeout = frames / 30 + 15
    # Acota el uso de tempfile.TemporaryDirectory(prefix='orvix-bench-') a este bloque.
    with tempfile.TemporaryDirectory(prefix="orvix-bench-") as workdir:
        # Obtiene producer con subprocess.Popen([str(engine), 'bridge', '--synthetic', '--frames',…
        producer = subprocess.Popen(
            # Ejecuta [str(engine), "bridge", "--synthetic", "--frames", str(frames), "--name", name],.
            [str(engine), "bridge", "--synthetic", "--frames", str(frames), "--name", name],
            # Indica cwd con valor workdir, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True.
            cwd=workdir, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True,
        )
        # Ejecuta el bloque con manejo de excepciones.
        try:
            # Obtiene viewer con subprocess.run([sys.executable, '-m', 'orvix.ui.viewer', '--headless…
            viewer = subprocess.run(
                # Aporta [sys.executable, "-m", "orvix.ui.viewer", "--headless", "--name", name, "--mode",… como elemento de la lista.
                [sys.executable, "-m", "orvix.ui.viewer", "--headless", "--name", name, "--mode", mode],
                # Indica cwd con valor workdir, capture_output=True, text=True, timeout=timeout, check=True.
                cwd=workdir, capture_output=True, text=True, timeout=timeout, check=True,
            )
            # Obtiene (output, _) con producer.communicate(timeout=timeout).
            output, _ = producer.communicate(timeout=timeout)
            # Entra en esta rama si producer.returncode.
            if producer.returncode:
                # Interrumpe la operación con RuntimeError(output).
                raise RuntimeError(output)
        # Ejecuta la limpieza al salir del bloque, incluso si hubo un error.
        finally:
            # Entra en esta rama si producer.poll() is None.
            if producer.poll() is None:
                # Ejecuta producer.kill().
                producer.kill()
                # Ejecuta producer.communicate().
                producer.communicate()
    # Establece prefix a partir de 'Viewer status: COMPLETED '.
    prefix = "Viewer status: COMPLETED "
    # Obtiene line con next((line for line in viewer.stdout.splitlines() if line.startswith…
    line = next(line for line in viewer.stdout.splitlines() if line.startswith(prefix))
    # Devuelve dict((field.split('=', 1) for field in line[len(prefix):].split())).
    return dict(field.split("=", 1) for field in line[len(prefix):].split())


# Define main sin parámetros de llamada.
def main():
    # Obtiene parser con argparse.ArgumentParser(description=__doc__).
    parser = argparse.ArgumentParser(description=__doc__)
    # Ejecuta parser.add_argument("engine", type=Path).
    parser.add_argument("engine", type=Path)
    # Ejecuta parser.add_argument("--frames", default=180, type=int).
    parser.add_argument("--frames", default=180, type=int)
    # Ejecuta parser.add_argument("--output", type=Path).
    parser.add_argument("--output", type=Path)
    # Obtiene args con parser.parse_args().
    args = parser.parse_args()
    # Entra en esta rama si not args.engine.is_file() or args.frames < 60.
    if not args.engine.is_file() or args.frames < 60:
        # Ejecuta parser.error("Provide an existing native executable and at least 60 frames.").
        parser.error("Provide an existing native executable and at least 60 frames.")
    # Construye el mapa de datos report.
    report = {
        # Asocia la clave "orvix" con __version__, "platform": platform.platform().
        "orvix": __version__, "platform": platform.platform(),
        # Asocia la clave "python" con platform.python_version(), "opencv": cv2.__version__, "numpy": numpy.__version__.
        "python": platform.python_version(), "opencv": cv2.__version__, "numpy": numpy.__version__,
        # Asocia la clave "source" con "synthetic NV12 1280x720 30 FPS", "frames_per_run": args.frames.
        "source": "synthetic NV12 1280x720 30 FPS", "frames_per_run": args.frames,
        # Asocia la clave "modes" con [run_mode(args.engine.resolve(), mode, args.frames) for mode in MODES].
        "modes": [run_mode(args.engine.resolve(), mode, args.frames) for mode in MODES],
    }
    # Establece result a partir de json.dumps(report, indent=2) + '\n'.
    result = json.dumps(report, indent=2) + "\n"
    # Entra en esta rama si args.output.
    if args.output:
        # Ejecuta args.output.write_text(result, encoding="utf-8").
        args.output.write_text(result, encoding="utf-8")
    # Muestra información en la salida mediante print con result.
    print(result, end="")


# Entra en esta rama si __name__ == '__main__'.
if __name__ == "__main__":
    # Ejecuta main().
    main()
