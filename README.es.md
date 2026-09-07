# ORVIX

[English](README.md) | [Español](README.es.md)

**Optical Real-time Video Ingestion eXchange**

ORVIX es una plataforma modular de ingesta de video en tiempo real y visión por
computadora desarrollada para el curso **Programación de Interfaces y
Dispositivos Periféricos**.

## Problema

Los prototipos de alto nivel suelen abrir la cámara directamente mediante API
como `cv2.VideoCapture(0)`. Ese enfoque oculta gran parte del recorrido de E/S
entre el dispositivo físico, el sistema operativo, la API multimedia nativa,
los búferes de frames y la aplicación de alto nivel.

ORVIX separa esas responsabilidades.

## Arquitectura

```text
Cámara física
      |
      v
Controlador de cámara del sistema operativo
      |
      v
Backend nativo de captura
Media Foundation / V4L2 / AVFoundation
      |
      v
ORVIX Capture (C++20)
      |
      v
ORVIX Bridge (memoria compartida)
      |
      v
ORVIX Vision (Python / NumPy / OpenCV)
      |
      v
ORVIX Insight (telemetría)
```

## Componentes

### ORVIX Capture

Subsistema C++ nativo responsable de enumerar y seleccionar cámaras, negociar
el formato, adquirir frames y producir telemetría. CMake selecciona Media
Foundation en Windows, V4L2 en Linux y AVFoundation en macOS.

### ORVIX Bridge

Subsistema de comunicación entre procesos responsable del transporte de frames
mediante memoria compartida.

### ORVIX Vision

Subsistema Python responsable del procesamiento de alto nivel.

### ORVIX Insight

Subsistema de observabilidad para FPS, frames perdidos, latencia, detecciones y
métricas históricas.

## Hoja de ruta de entregas

### Entrega 1 — Fundamentos y captura nativa

Repositorio, arquitectura, requisitos, C++20, backends nativos para
Windows/Linux/macOS, enumeración, selección, negociación de formato, captura,
metadatos, FPS básico, logging, pruebas y CI multiplataforma.

### Entrega 2 — ORVIX Bridge

Protocolo IPC v1, memoria compartida, búfer circular, sincronización, consumidor
Python y visualización con NumPy y OpenCV.

### Entrega 3 — Visión y observabilidad

Filtros, pipeline de detección, análisis de FPS, frames perdidos, latencia,
detecciones por minuto y telemetría SQLite.

### Entrega 4 — ORVIX 1.0

Dashboard, configuración, recuperación, pruebas automatizadas, benchmarks,
reproducibilidad, documentación y versión académica.

## Tecnologías

- C++20
- MSVC, GCC y Apple Clang
- Windows Media Foundation
- Linux Video4Linux2 (V4L2)
- macOS AVFoundation
- CMake y Ninja
- Python 3.8–3.13
- NumPy y OpenCV
- SQLite
- pytest
- GitHub Actions

## Plataformas

| Sistema operativo | Backend nativo | Estado |
|---|---|---|
| Windows 10/11 | Media Foundation | CI y cámara física validados |
| Windows 7 SP1 | Media Foundation heredado | Ruta de compatibilidad de código y Python 3.8 |
| Linux | V4L2 | Validado en CI |
| macOS | AVFoundation | Validado en CI |

Los requisitos de compilación y permisos de cámara están documentados en
[`docs/es/platform/compatibility.md`](docs/es/platform/compatibility.md).

## CLI de captura nativa

```text
orvix-capture devices
orvix-capture select --index <N>
orvix-capture open --index <N>
orvix-capture formats --index <N>
orvix-capture capture --index <N>
```

El comando `capture` abre la cámara seleccionada, negocia el formato nativo más
cercano al objetivo inicial de 1280×720 a 30 FPS y recibe 120 muestras
consecutivas. Muestra secuencia, timestamp, resolución, formato de píxel y
tamaño de cada frame, además de los FPS medidos.

Cada comando registra diagnósticos estructurados en
`logs/orvix-capture.log`. Los fallos esperados incluyen un código de diagnóstico
estable y un código de salida distinto de cero.

La evidencia de la Entrega 1 está organizada en
[`docs/es/evidence/delivery-01/README.md`](docs/es/evidence/delivery-01/README.md).

La documentación completa está disponible en [inglés](docs/en/README.md) y
[español](docs/es/README.md).

## Privacidad

Los frames son efímeros de manera predeterminada y no se guardan
automáticamente.

## Hito actual

**Entrega 1 — Fundamentos y captura nativa (alcance funcional completo)**

## Licencia

MIT
