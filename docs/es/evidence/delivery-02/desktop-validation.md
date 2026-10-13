# Interfaz e instalador — ORVIX 0.2.1

Fecha local de validación: **11 de octubre de 2026 (America/Lima)**.
Estado: implementación RAW de escritorio, revisada para la versión 0.2.1.

## Alcance exclusivo de la entrega 2

La aplicación de escritorio controla Capture, recibe frames mediante Bridge y
muestra RAW. No abre la cámara con Python ni `cv2.VideoCapture`.

Incluye selección de cámara, fuente sintética, iniciar/detener, vista adaptable,
zoom y desplazamiento, metadatos por frame, FPS de captura y consumidor,
secuencias omitidas, sobrescrituras, ocupación del anillo, diagnóstico y
reconexión del consumidor. Los frames no se guardan automáticamente.

**No implementa** filtros, detección, latencia extremo a extremo, histórico
SQLite ni dashboard analítico: corresponden a las entregas 3 y 4. El modelo
de datos del documento académico sigue siendo un diseño de evolución.

## Arquitectura implementada

- `orvix-capture.exe`: captura nativa C++ y publicación en memoria compartida.
- `CaptureController` / `QProcess`: ciclo de vida no bloqueante del productor.
- `FrameWorker` / `QThread`: lectura IPC, NumPy y decodificación OpenCV.
- `LatestFrame`: buzón de un solo frame; no acumula señales de video si la UI
  se retrasa. Qt dibuja una imagen propia en el hilo gráfico.
- `MainWindow` / `Preview`: controles, métricas y zoom manteniendo proporción.
- Logs locales en `%LOCALAPPDATA%/ORVIX/ORVIX/logs`; no se envían a Internet.

La captura continua no conserva una lista ilimitada de metadatos. Detener
es una solicitud explícita del propietario de la sesión (offset 120 del
header IPC). Si solo se desconecta o falla el consumidor, Capture continúa.
Una parada sin respuesta tiene fallback de 3 segundos que finaliza solamente
el proceso creado para esa captura. El consumidor avisa después de 5 segundos
sin frames; no promete recuperación automática de dispositivos físicamente
desconectados.

## Verificaciones realizadas

| Comprobación | Resultado | Límites |
|---|---|---|
| Compilación CMake/MSVC Release x64 | Correcta | Windows de esta computadora |
| CTest | 16/16 | Incluye parada explícita y memoria de metadatos acotada |
| pytest sin integración | 13/13 | PySide6 disponible; GUI probada con plataforma offscreen |
| pytest de integración | 5/5 | Productor nativo real, fuente sintética |
| Inicio/parada/reinicio de escritorio | Correcto | Mapeo único por sesión y salida nativa 0 |
| Caída del productor | Error visible, UI utilizable | Fallo inyectado solo al proceso de prueba |
| Consumidor interrumpido y reconexión | Mismo PID nativo | RF-024: no mata al productor |
| Índice inválido / cancelación de inicio | Diagnóstico / cierre acotado | Pruebas automáticas |
| Layout 1024×768 | Controles dentro de ventana | No se certifica cada monitor/escala DPI |
| Ejecutable empaquetado | Abre y muestra RAW | No depende de la `.venv` del repositorio |
| Webcam GENERAL WEBCAM | 1280×720 / NV12 / ~19.46 FPS | 877 frames publicados, 876 consumidos al cierre, 0 sobrescrituras; no garantiza 30 FPS |

La webcam se activó desde la interfaz del ejecutable empaquetado. La imagen
se verificó visualmente sin guardarla como evidencia. La prueba terminó con
`shared_memory_bridge_completed` y sin proceso de captura restante.
La fuente sintética sostuvo 30.0 FPS en una sesión de aproximadamente seis
minutos, con cero omisiones y cero sobrescrituras en la observación final.
Esto no equivale a un benchmark de la webcam ni a pruebas en todos los SO.

## Distribución

Instalador: `ORVIX-Avance2-Setup-0.2.1.exe` en
`entregables/Avance2_ORVIX_2026-10-11/Aplicacion`.

PyInstaller empaqueta Python, Qt, NumPy, OpenCV y el motor en una distribución
onedir. Inno Setup instala por usuario en `%LOCALAPPDATA%/Programs/ORVIX`,
sin drivers ni elevación administrativa. Incluye desinstalador y licencias.
El instalador es **sin firma digital**; no se modifican barreras de seguridad
ni permisos de privacidad para ocultar esa condición.

El instalador se ejecutó satisfactoriamente por usuario y se comprobaron
`ORVIX.exe`, el desinstalador, el registro de versión 0.2.1 y el acceso directo
del escritorio. La aplicación instalada abrió su interfaz correctamente.
También pasaron las pruebas offscreen de layout a factores Qt 1.25 y 1.5;
no sustituyen una revisión física de cada monitor y escala de Windows.

## Reproducir

```powershell
./scripts/setup.ps1
./.venv/Scripts/python.exe -m pip install -e '.[desktop,package]'
./scripts/build.ps1
./.venv/Scripts/python.exe -m orvix.ui.desktop
./scripts/test.ps1
./.venv/Scripts/python.exe -m pytest -m integration
./scripts/package-desktop.ps1 -InnoCompiler 'C:/Program Files (x86)/Inno Setup 6/ISCC.exe'
```

La interfaz Qt requiere Python 3.9 o posterior y Windows 10/11 x64 para este
paquete. No se anuncia este instalador como compatible con Windows 7.

## Revisión del 12 de octubre de 2026

La compilación Release y las suites volvieron a pasar: 16 pruebas CTest,
13 pruebas Python sin integración y 5 pruebas de integración con el productor
nativo. Las pruebas nuevas reprodujeron y luego verificaron estas correcciones:

- El diagnóstico de bloqueo cuenta lecturas vacías **consecutivas**, conservando
  el total de lecturas vacías como métrica. Pausas aisladas entre frames válidos
  ya no detienen una captura continua larga.
- La espera inicial del mapeo no consume el timeout de entrega de frames.
- Cerrar durante una búsqueda cuyo ejecutable no puede iniciarse completa el
  cierre, incluso cuando QProcess no emite `finished`.
- El ZIP fuente se construye en un archivo temporal y sustituye el anterior
  solamente al completarse, permitiendo repetir el empaquetado.

CI incluye ahora un job de escritorio Windows con PySide6, pruebas offscreen
y ciclo de vida del Bridge real. Los jobs existentes de núcleo y Python siguen
cubriendo Windows, Linux, macOS y Python 3.8. El instalador se genera con
`scripts/package-desktop.ps1`; los binarios permanecen excluidos de Git.

## Documentación académica anterior

El Word y PowerPoint elaborados antes de esta implementación contienen
mockups de la interfaz y planes futuros. Este registro acredita la interfaz
RAW que ahora está implementada. El mockup de histórico SQLite continúa
siendo una propuesta, no una pantalla disponible en esta entrega.
