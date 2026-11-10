"""Build deterministic icon and collect bundled-library license metadata."""
# Importa metadata desde importlib.
from importlib import metadata
# Importa Path desde pathlib.
from pathlib import Path
# Importa shutil.
import shutil
# Importa sys.
import sys

# Importa QSize desde PySide6.QtCore.
from PySide6.QtCore import QSize
# Importa QIcon desde PySide6.QtGui.
from PySide6.QtGui import QIcon
# Importa QApplication desde PySide6.QtWidgets.
from PySide6.QtWidgets import QApplication

# Establece root a partir de Path(__file__).resolve().parents[2].
root = Path(__file__).resolve().parents[2]
# Establece output a partir de root / 'out/desktop-package'.
output = root / "out/desktop-package"
# Ejecuta output.mkdir(parents=True, exist_ok=True).
output.mkdir(parents=True, exist_ok=True)
# Obtiene app con QApplication([]).
app = QApplication([])
# Obtiene icon con QIcon(str(root / 'src/python/src/orvix/ui/assets/orvix.svg')).
icon = QIcon(str(root / "src/python/src/orvix/ui/assets/orvix.svg"))
# Verifica que se cumpla icon.pixmap(QSize(256, 256)).save(str(output / 'orvix.ico'), 'ICO').
assert icon.pixmap(QSize(256, 256)).save(str(output / "orvix.ico"), "ICO")
# Establece python_license a partir de Path(sys.base_prefix) / 'LICENSE.txt'.
python_license = Path(sys.base_prefix) / "LICENSE.txt"
# Entra en esta rama si python_license.is_file().
if python_license.is_file():
    # Establece license_dir a partir de output / 'licenses' / 'Python'.
    license_dir = output / "licenses" / "Python"
    # Ejecuta license_dir.mkdir(parents=True, exist_ok=True).
    license_dir.mkdir(parents=True, exist_ok=True)
    # Ejecuta shutil.copy2(python_license, license_dir / "LICENSE.txt").
    shutil.copy2(python_license, license_dir / "LICENSE.txt")
# Recorre ('numpy', 'opencv-python', 'PySide6-Essentials', 'shiboken6', 'pyinstaller') y toma cada elemento como name.
for name in ("numpy", "opencv-python", "PySide6-Essentials", "shiboken6", "pyinstaller"):
    # Obtiene dist con metadata.distribution(name).
    dist = metadata.distribution(name)
    # Recorre dist.files or [] y toma cada elemento como entry.
    for entry in dist.files or []:
        # Entra en esta rama si any((word in str(entry).lower() for word in ('license', 'copying'))).
        if any(word in str(entry).lower() for word in ("license", "copying")):
            # Obtiene source (fuente de video) con Path(dist.locate_file(entry)).
            source = Path(dist.locate_file(entry))
            # Entra en esta rama si source.is_file().
            if source.is_file():
                # Preserve relative paths: many dependencies have several
                # different files named LICENSE, which must not overwrite.
                # Establece destination a partir de output / 'licenses' / name / Path(str(entry)).
                destination = output / "licenses" / name / Path(str(entry))
                # Ejecuta destination.parent.mkdir(parents=True, exist_ok=True).
                destination.parent.mkdir(parents=True, exist_ok=True)
                # Ejecuta shutil.copy2(source, destination).
                shutil.copy2(source, destination)
