# ORVIX 1.0.0 · Entrega final

[English](../en/release-1.0.md) | [Requisitos](requirements/traceability.md)

## Uso en Windows x64

1. Extrae el RAR completo. Abre `Aplicacion-Windows-x64/ORVIX.exe`.
2. Deja **Demo sintética** y pulsa **Iniciar**. Cambia entre **RAW**, **Grises**,
   **Bordes** y **Movimiento**. El recuadro del detector marca regiones que
   cambian entre frames.
3. Pulsa **Detener** y abre **Historial**. Selecciona la sesión para ver sus
   datos, gráficos y diagnósticos. Para usar una webcam, pulsa **Buscar** y
   elige la cámara antes de iniciar.
4. En **Ajustes**, selecciona el perfil solicitado (480p, 720p o 1080p a 30
   FPS), slots, intervalo de muestreo y área mínima del detector. Se aplican
   a la siguiente sesión. El filtro se puede cambiar en cualquier momento.

El paquete es portátil y lleva el motor, Python, Qt, NumPy y OpenCV. Conserva
la carpeta `_internal` junto al ejecutable. El paquete no lleva firma digital.

## Arquitectura y métricas

C++ abre el dispositivo mediante Media Foundation, V4L2 o AVFoundation. Bridge
publica frames en el anillo ORVX v1. Un worker Python toma el frame más reciente,
lo decodifica a BGR y aplica el modo con OpenCV. El hilo de la ventana muestra
solo la última imagen disponible. Cambiar el modo conserva la misma secuencia
y proceso nativo.

Grises usa la conversión BGR a luminancia. Bordes usa desenfoque gaussiano y
Canny con umbrales 60/150. Movimiento compara dos imágenes consecutivas
suavizadas, aplica umbral y dilatación, y marca contornos sobre el área mínima
configurable. Se contabiliza hasta un evento por segundo con alguna región
válida. La cifra de detecciones por minuto corresponde a los últimos 60 s.
Un cambio de iluminación o de posición de la cámara puede producir detecciones.

| Medición | Significado |
| --- | --- |
| FPS de captura | Ritmo del productor calculado con timestamps multimedia. |
| FPS de proceso | Ritmo de frames decodificados y procesados por Python en la ventana reciente. |
| Latencia de procesamiento | Tiempo monotónico de decodificación y filtro, en ms. |
| Omitidos | Saltos entre frames consumidos. |
| Sobrescrituras | Slots reemplazados por el productor antes de confirmación. |
| Ocupación | Porcentaje del anillo pendiente de consumo antes de confirmar el frame. |

La latencia medida cubre decodificación y filtro. La negociación del formato y
los FPS de la webcam dependen de su hardware, driver e iluminación.

## Historial y recuperación

La base SQLite `telemetry.sqlite3` se guarda en la carpeta de datos de la app
(en Windows normalmente `%LOCALAPPDATA%/ORVIX/ORVIX`). La base registra fuente,
sesión, configuración, formato negociado, muestras y eventos diagnósticos.
Las fechas se guardan en UTC y se muestran en hora local. Las muestras usan
tiempo relativo monotónico. Los frames permanecen en memoria.

El historial muestra las últimas 100 sesiones. Los gráficos muestran hasta
600 muestras recientes y el resumen usa todas las muestras de la sesión.
La etiqueta **Sin cierre registrado** identifica una sesión cuya terminación
no llegó a persistirse. Tras un fallo del consumidor, **Reconectar vista**
reabre el puente sin reiniciar el motor. Si la cámara o el motor se
desconectan, revisa **Diagnóstico**, vuelve a buscar la fuente e inicia una
sesión nueva. **Detener** libera la cámara y aplica una parada acotada si el
proceso no responde.

## Reproducir la compilación y las pruebas

Desde `Codigo-Fuente`, instala Python x64 3.11 o 3.13, CMake, MSVC con C++20
y Windows SDK. En PowerShell:

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

Si trabajas desde una consola de desarrollador x64 con Ninja, puedes compilar
en un directorio `build` nuevo con:

```powershell
cmake -S . -B build -G 'Ninja Multi-Config'
cmake --build build --config Release --parallel
```

El portátil queda en `out/desktop-package/dist/ORVIX`. Para compilar un
instalador, instala Inno Setup 6 y ejecuta `package-desktop.ps1 -SkipBuild`
con `-InnoCompiler` si el compilador está en otra ruta. Linux y macOS conservan
sus backends y scripts de compilación en [Compatibilidad](platform/compatibility.md).

## Evidencia de validación de la versión

- Compilación Release x64 con MSVC 19.51 y Windows SDK 10.0.26100.
- 16 pruebas CTest y 20 pruebas pytest aprobadas, incluidas Qt y la integración
  entre el proceso nativo y Python. Las pruebas cubren inicio, parada,
  cancelación, recuperación del consumidor, caída del motor, perfil de captura,
  cambio de modo sin cambiar PID y persistencia de sesiones.
- Ejecutable portátil comprobado en modo Movimiento. La sesión 1280×720 se
  cerró correctamente, guardó seis muestras y superó la comprobación de
  integridad de SQLite.
- Benchmark con Windows 10, Python 3.13 y video NV12 sintético de 1280×720,
  180 frames por modo. La siguiente medición es una ejecución, con arranque
  del productor antes del lector.

| Modo | Frames procesados | FPS consumidor | Tiempo medio de proceso |
| --- | ---: | ---: | ---: |
| RAW | 170 | 30.05 | 1.461 ms |
| Grises | 168 | 29.25 | 2.902 ms |
| Bordes | 161 | 27.42 | 8.418 ms |
| Movimiento | 160 | 29.99 | 6.844 ms |

El [JSON del benchmark](evidence/benchmark-win-x64.json) conserva también
las omisiones, sobrescrituras y los seis eventos de movimiento. Las pruebas
de esta versión usaron video sintético. La prueba histórica de cámara física
corresponde a [0.2.1](evidence/delivery-02/desktop-validation.md). La
validación local de 1.0 se realizó en Windows.

## Demostración reproducible

Para probar el ejecutable empaquetado en un directorio de datos aislado:

```powershell
$demo = Start-Process -FilePath .\ORVIX.exe -ArgumentList '--demo-seconds','5','--data-dir','demo-evidencia' -PassThru -Wait
$demo.ExitCode
```

La ventana abre la fuente sintética en modo Movimiento y se cierra al terminar.
El código 0 indica que procesó frames y detuvo el motor. El historial y logs
de esa sesión quedan en `demo-evidencia`. El Word y el PowerPoint del
[Avance 2](https://github.com/Jeamlake/orvix/tree/8e88e67e717c72a161c509cb90a50161f63f7f48/entregables/Avance2_ORVIX_2026-10-11) se conservan en la revisión académica.
