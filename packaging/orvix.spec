# -*- mode: python ; coding: utf-8 -*-
from pathlib import Path
root = Path(SPECPATH).parent
licenses = list((root / "packaging/licenses").glob("*.txt"))
a = Analysis(
    [str(root / "scripts/desktop_entry.py")],
    pathex=[str(root / "python/src")],
    binaries=[(str(root / "build/native/Release/orvix-capture.exe"), "native")],
    datas=[(str(root / "python/src/orvix/ui/assets"), "orvix/ui/assets"),
           (str(root / "out/desktop-package/licenses"), "licenses/Packages"),
           (str(root / "LICENSE"), "licenses/ORVIX"),
           (str(root / "packaging/THIRD-PARTY-NOTICES.txt"), "licenses")]
          + [(str(p), "licenses/Qt") for p in licenses],
    hiddenimports=[],
    excludes=["tkinter", "matplotlib", "pytest", "PySide6.QtQml", "PySide6.QtQuick",
              "PySide6.QtNetwork", "PySide6.QtMultimedia"],
    noarchive=False,
)
pyz = PYZ(a.pure)
exe = EXE(pyz, a.scripts, [], exclude_binaries=True, name="ORVIX", debug=False,
          strip=False, upx=False, console=False,
          icon=str(root / "out/desktop-package/orvix.ico"))
coll = COLLECT(exe, a.binaries, a.datas, strip=False, upx=False, name="ORVIX")
