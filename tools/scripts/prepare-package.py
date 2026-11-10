"""Build deterministic icon and collect bundled-library license metadata."""
from importlib import metadata
from pathlib import Path
import shutil
import sys

from PySide6.QtCore import QSize
from PySide6.QtGui import QIcon
from PySide6.QtWidgets import QApplication

root = Path(__file__).resolve().parents[2]
output = root / "out/desktop-package"
output.mkdir(parents=True, exist_ok=True)
app = QApplication([])
icon = QIcon(str(root / "src/python/src/orvix/ui/assets/orvix.svg"))
assert icon.pixmap(QSize(256, 256)).save(str(output / "orvix.ico"), "ICO")
python_license = Path(sys.base_prefix) / "LICENSE.txt"
if python_license.is_file():
    license_dir = output / "licenses" / "Python"
    license_dir.mkdir(parents=True, exist_ok=True)
    shutil.copy2(python_license, license_dir / "LICENSE.txt")
for name in ("numpy", "opencv-python", "PySide6-Essentials", "shiboken6", "pyinstaller"):
    dist = metadata.distribution(name)
    for entry in dist.files or []:
        if any(word in str(entry).lower() for word in ("license", "copying")):
            source = Path(dist.locate_file(entry))
            if source.is_file():
                # Preserve relative paths: many dependencies have several
                # different files named LICENSE, which must not overwrite.
                destination = output / "licenses" / name / Path(str(entry))
                destination.parent.mkdir(parents=True, exist_ok=True)
                shutil.copy2(source, destination)
