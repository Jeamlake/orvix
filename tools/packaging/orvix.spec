# -*- mode: python ; coding: utf-8 -*-
# Importa Path desde pathlib.
from pathlib import Path
# Establece root a partir de Path(SPECPATH).parents[1].
root = Path(SPECPATH).parents[1]
# Obtiene licenses con list((root / 'tools/packaging/licenses').glob('*.txt')).
licenses = list((root / "tools/packaging/licenses").glob("*.txt"))
# Obtiene a con Analysis([str(root / 'tools/scripts/desktop_entry.py')], pathex=[str…
a = Analysis(
    # Ejecuta [str(root / "tools/scripts/desktop_entry.py")],.
    [str(root / "tools/scripts/desktop_entry.py")],
    # Indica pathex con valor [str(root / "src/python/src")].
    pathex=[str(root / "src/python/src")],
    # Indica binaries con valor [(str(root / "build/native/Release/orvix-capture.exe"), "native")].
    binaries=[(str(root / "build/native/Release/orvix-capture.exe"), "native")],
    # Indica datas con valor [(str(root / "src/python/src/orvix/ui/assets"), "orvix/ui/assets").
    datas=[(str(root / "src/python/src/orvix/ui/assets"), "orvix/ui/assets"),
           # Ejecuta (str(root / "out/desktop-package/licenses"), "licenses/Packages"),.
           (str(root / "out/desktop-package/licenses"), "licenses/Packages"),
           # Ejecuta (str(root / "LICENSE"), "licenses/ORVIX"),.
           (str(root / "LICENSE"), "licenses/ORVIX"),
           # Ejecuta (str(root / "tools/packaging/THIRD-PARTY-NOTICES.txt"), "licenses")].
           (str(root / "tools/packaging/THIRD-PARTY-NOTICES.txt"), "licenses")]
          # Ejecuta + [(str(p), "licenses/Qt") for p in licenses],.
          + [(str(p), "licenses/Qt") for p in licenses],
    # Indica hiddenimports con valor [].
    hiddenimports=[],
    # Indica excludes con valor ["tkinter", "matplotlib", "pytest", "PySide6.QtQml", "PySide6.QtQuick".
    excludes=["tkinter", "matplotlib", "pytest", "PySide6.QtQml", "PySide6.QtQuick",
              # Aporta "PySide6.QtNetwork", "PySide6.QtMultimedia"] como elemento de la lista.
              "PySide6.QtNetwork", "PySide6.QtMultimedia"],
    # Indica noarchive con valor False.
    noarchive=False,
)
# Obtiene pyz con PYZ(a.pure).
pyz = PYZ(a.pure)
# Obtiene exe con EXE(pyz, a.scripts, [], exclude_binaries=True, name='ORVIX', debug=F…
exe = EXE(pyz, a.scripts, [], exclude_binaries=True, name="ORVIX", debug=False,
          # Indica strip con valor False, upx=False, console=False.
          strip=False, upx=False, console=False,
          # Indica icon con valor str(root / "out/desktop-package/orvix.ico")).
          icon=str(root / "out/desktop-package/orvix.ico"))
# Obtiene coll con COLLECT(exe, a.binaries, a.datas, strip=False, upx=False, name='ORVI…
coll = COLLECT(exe, a.binaries, a.datas, strip=False, upx=False, name="ORVIX")
