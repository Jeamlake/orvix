# Desktop application and installer — ORVIX 0.2.1

Desktop validation: 11 October 2026. Source review: 12 October 2026,
America/Lima. This record covers the Delivery 2 RAW implementation.

## Scope and architecture

The desktop application controls native C++ Capture, receives frames through
the shared-memory Bridge and displays RAW video. Python does not open the
camera or call `cv2.VideoCapture`. The UI provides camera selection, synthetic
demo, start/stop, aspect-fit preview, zoom/pan, frame metadata, capture and
consumer FPS, skipped sequences, overwrites, ring occupancy, diagnostics and
consumer reconnection. Frames are not saved automatically.

Filters, detection, end-to-end latency, SQLite history and the analytical
dashboard remain Delivery 3/4 work. Earlier academic mockups and the database
model describe future designs; the implementation addendum records the
currently available RAW UI.

- `orvix-capture.exe` owns native capture and shared-memory publication.
- `CaptureController` / `QProcess` manage the producer without blocking the UI.
- `FrameWorker` / `QThread` read IPC frames and decode them with NumPy/OpenCV.
- `LatestFrame` retains one frame instead of accumulating queued video.
- Qt owns the displayed image and renders it on the GUI thread.

Continuous capture does not accumulate frame metadata indefinitely. Stop is
an explicit session-owner request at IPC header offset 120. Detaching or
failing the consumer leaves Capture running (RF-024). If an explicit stop
does not complete within three seconds, only the session's owned process is
terminated. The consumer reports five seconds without frames; automatic
recovery of disconnected hardware is not promised.

## Validation

| Check | Result | Scope |
|---|---|---|
| CMake/MSVC Release x64 | Passed | This Windows computer |
| CTest | 16/16 | Explicit stop, bounded metadata and transient empty reads |
| Python without integration | 13/13 | PySide6 and offscreen GUI |
| Integration | 5/5 | Real native producer, synthetic frames |
| Start/stop/restart and native crash | Passed | Visible errors, reusable UI |
| Consumer failure and reconnect | Same native PID | RF-024 |
| Invalid index / startup cancellation | Controlled | Automated lifecycle tests |
| 1024×768, Qt scaling 1.25 and 1.5 | Passed on 11 October | Not every physical monitor/DPI |
| Packaged and installed application | RAW UI opened on 11 October | No repo virtual environment needed |
| GENERAL WEBCAM on 11 October | ~19.46 FPS, 1280×720 NV12 | 877 published, 876 consumed, 0 overwrites at stop |

The physical camera image was inspected without saving it as evidence.
Capture completed and released the camera. A roughly six-minute synthetic
session sustained 30 FPS with zero skips and overwrites in the final observed
metrics. These observations do not certify all cameras or operating systems.

The 12 October review reproduced and corrected accumulated empty-read stalls,
attachment time incorrectly counted as frame stall time, and shutdown when
device enumeration cannot start. Source packaging now creates a temporary ZIP
and replaces the prior archive only after success. CI includes a Windows Qt
desktop job with the real bridge lifecycle alongside the existing native and
Python platform matrices.

## Distribution and reproduction

PyInstaller bundles Python, Qt, NumPy, OpenCV and the native engine in onedir
form. Inno Setup installs per user into `%LOCALAPPDATA%/Programs/ORVIX`, with
an uninstaller and dependency licenses. No drivers or elevated privileges
are required. Logs stay in `%LOCALAPPDATA%/ORVIX/ORVIX/logs` and are not uploaded;
logs and settings are retained on uninstall. The installer is unsigned and
targets Windows 10/11 x64, not Windows 7. Executables are excluded from Git.

```powershell
./scripts/setup.ps1
./.venv/Scripts/python.exe -m pip install -e '.[desktop,package]'
./scripts/build.ps1
./.venv/Scripts/python.exe -m orvix.ui.desktop
./scripts/test.ps1
./.venv/Scripts/python.exe -m pytest -m integration
./scripts/package-desktop.ps1 -InnoCompiler 'C:/Program Files (x86)/Inno Setup 6/ISCC.exe'
./scripts/package-source.ps1
```

The Qt interface needs Python 3.9 or later. Earlier academic Word/PowerPoint
deliverables retain their baseline revision and future designs; read their
implementation addendum for the current desktop scope.
