# Guía de demostración de la Entrega 2

[English](../../../en/evidence/delivery-02/demonstration-guide.md) | [Español](demonstration-guide.md)

## Preparación única

En Git Bash, desde la raíz de ORVIX:

```bash
./scripts/setup.sh
./scripts/build.sh
./scripts/test.sh
```

En PowerShell se pueden usar `scripts\setup.ps1`, `scripts\build.ps1` y
`scripts\test.ps1`.

## Demostración con webcam

Abrir dos terminales en la raíz del repositorio. En la primera terminal:

```bash
./build/native/Release/orvix-capture.exe bridge --index 0 --frames 900
```

En la segunda terminal Git Bash:

```bash
source .venv/Scripts/activate
python -m orvix.ui.viewer
```

En PowerShell:

```powershell
.\.venv\Scripts\Activate.ps1
python -m orvix.ui.viewer
```

En Linux o macOS, el productor se ejecuta con
`./build/native/orvix-capture` y el entorno se activa con
`source .venv/bin/activate`.

Se abre una ventana titulada **ORVIX Live - RAW** con el video físico. La franja
superior muestra secuencia, resolución, formato, FPS del productor, FPS del
visor y frames omitidos. Presionar `Q` o `Esc` cierra solamente Vision; Capture
continúa hasta completar la cantidad solicitada.

## Demostración sin webcam

El mismo flujo se puede probar con frames sintéticos. En la primera terminal:

```bash
./build/native/Release/orvix-capture.exe bridge --synthetic --frames 900
```

En la segunda terminal se utiliza el mismo comando del visor. La ventana debe
mostrar un gradiente animado. Esta ruta comprueba el protocolo, la
sincronización, NumPy y OpenCV sin depender de un dispositivo físico.

## Comprobación sin ventana

Para una verificación rápida o un entorno CI:

```bash
python -m orvix.ui.viewer --headless --frames 30
```

La salida correcta incluye `Viewer status: COMPLETED`, un número de frames
mayor que cero y las métricas de secuencia, sobrescritura y utilización.
