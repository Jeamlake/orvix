# ORVIX 1.0.0

[Spanish final report](../es/release-1.0.md) | [Requirements](requirements/traceability.md)

Extract the full Windows x64 application directory and run `ORVIX.exe`.
Choose the synthetic source and start capture. The desktop interface can
switch between RAW, grayscale, Canny edges and OpenCV motion detection
while the C++ producer keeps running. The Settings page controls the requested
capture profile, ring slot count, sample interval and minimum motion area.

The History page reads source, session, metrics and diagnostic events from
local SQLite. Frames stay in memory. The charts include capture and
processing FPS, processing latency, rolling 60-second motion events and
ring occupancy. Processing latency is measured with a monotonic clock around
frame decoding and filtering.

## Build and verify on Windows x64

From the source directory, with Python 3.11 or 3.13, CMake, MSVC and the Windows SDK:

```powershell
python -m venv .venv
.\.venv\Scripts\python.exe -m pip install -e '.[dev,desktop,package]'
.\tools\scripts\build.ps1
ctest --test-dir build -C Release --output-on-failure
$env:QT_QPA_PLATFORM = 'offscreen'
.\.venv\Scripts\python.exe -m pytest -o addopts=''
.\.venv\Scripts\python.exe tools/scripts/benchmark.py build/native/Release/orvix-capture.exe --output benchmark-win-x64.json
.\tools\scripts\package-desktop.ps1 -Portable -SkipBuild
```

The portable build is written to `out/desktop-package/dist/ORVIX`. With Inno
Setup 6, run the script without `-Portable` to build an installer. Keep the
`_internal` directory with the executable. The release was validated on Windows
with 16 CTest and 20 pytest checks and a synthetic 720p/30 benchmark in all four
modes. The [raw results](../es/evidence/benchmark-win-x64.json) are included.
Physical webcam evidence for 0.2.1 is in the
[prior validation record](evidence/delivery-02/desktop-validation.md).
