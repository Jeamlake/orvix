# Arquitectura de ORVIX

[English](../../en/architecture/overview.md) | [Español](overview.md)

## Arquitectura lógica

```text
Cámara física
      |
      v
Controlador de cámara del sistema operativo
      |
      v
Backend nativo de captura
      |
      v
ORVIX Capture
C++20
      |
      v
ORVIX Bridge
Memoria compartida
      |
      v
ORVIX Vision
Python / NumPy / OpenCV
      |
      +------------------+
      |                  |
      v                  v
Video procesado     ORVIX Insight
                    Métricas / SQLite
```

## Plano de datos

Los frames, que son cargas de gran tamaño, se transportan mediante memoria
compartida. El Bridge implementa un anillo de tres slots con seqlock por slot.
Capture publica sin esperar a Vision; Vision copia solamente un frame estable,
confirma la secuencia consumida y transforma el payload con NumPy según su
formato y stride.

```text
FrameReader nativo o sintético
            |
            v
ContinuousFrameCapture -> SharedMemoryPublisher
                              |
                         Protocolo ORVX v1
                              |
                              v
                    SharedMemoryReader (Python)
                              |
                         NumPy + OpenCV
```

## Plano de control

Los comandos pequeños futuros pueden incluir `START`, `STOP`,
`SET_RESOLUTION`, `SET_FPS` y `GET_STATUS`. El mecanismo exacto se definirá en
una entrega posterior.

## Plataformas compatibles

El contrato común de captura se implementa mediante un backend nativo por
sistema operativo:

```text
Interfaz Camera
      |
      +-- Windows -> Media Foundation
      +-- Linux   -> V4L2
      +-- macOS   -> AVFoundation
```

CMake compila exactamente un backend. Los headers y bibliotecas específicos de
plataforma permanecen fuera de los módulos comunes de CLI, secuencia,
metadatos, FPS y logging.

## Backends de captura

- Windows utiliza `IMFMediaSource` e `IMFSourceReader`.
- Linux utiliza E/S de streaming V4L2 con búferes de memoria del kernel mapeados.
- macOS utiliza callbacks de `AVCaptureSession` y
  `AVCaptureVideoDataOutput`.

Cada flujo nativo administra sus recursos del sistema operativo, conserva el
tipo de medio configurado y asigna una secuencia ORVIX creciente a cada muestra
aceptada. Los timestamps nativos se normalizan a unidades de 100 nanosegundos y
proporcionan el tiempo multimedia utilizado para calcular los FPS efectivos.

## Observabilidad nativa

```text
Resultado del backend / estado del stream
                 |
                 v
Diagnóstico ORV-CAP estable
                 |
          +------+------+
          |             |
          v             v
     stderr del CLI    Log estructurado
                       logs/orvix-capture.log
```

El CLI registra el ciclo de vida de la cámara, el formato negociado, la
finalización de la captura y los fallos controlados. Los frames permanecen
efímeros; el log contiene diagnósticos y mediciones agregadas, sin imágenes.
