# Validación nativa multiplataforma

[English](../../en/evidence/cross-platform-native-validation.md) | [Español](cross-platform-native-validation.md)

Estado: **Validado en CI; hardware validado en Windows**

Issue: **#15**

## Selección del backend

| Runner | Backend compilado | Resultado |
|---|---|---|
| `windows-latest` | Windows Media Foundation | PASS |
| `ubuntu-latest` | Linux V4L2 | PASS |
| `macos-latest` | macOS AVFoundation | PASS |

Los jobs nativos parten de un checkout limpio, compilan las fuentes elegidas
para la plataforma y ejecutan CTest. `TC-CAP-003-PLATFORM` verifica que cada
factoría cree el enumerador y la cámara correctos sin necesitar hardware.

La instalación y pytest con Python 3.11 pasan de manera independiente en
Windows, Ubuntu y macOS. Python 3.8 también pasa como ruta heredada para
Windows 7.

## Evidencia de CI

- estado: completed
- conclusión: success
- commit: 37f6b81d53af30e0d2d278e5f034359c43a4b0f2
- ejecución: https://github.com/Jeamlake/orvix/actions/runs/34119675922

## Regresión física en Windows

El CLI independiente de plataforma conservó el resultado físico existente:

```text
Backend: Windows Media Foundation
Device: GENERAL WEBCAM
Negotiated resolution: 1280x720
Negotiated pixel format: NV12
Negotiated FPS: 30.00
Frames captured: 120
First sequence: 1
Last sequence: 120
Sequence status: STRICTLY_INCREASING
Timestamp status: STRICTLY_INCREASING
Measured capture FPS: 19.47
Capture status: CAPTURE_COMPLETED
```

## Límite de la validación de hardware

Los runners hospedados de Ubuntu y macOS no incluyen webcams. Sus backends se
validaron mediante compilación, enlazado y pruebas independientes del hardware.
La captura física en Linux, macOS y Windows 7 requiere acceso a esos equipos y
se registra como comprobación explícita de release, sin presentarla como una
prueba física de CI.
