# Entrega 2 — ORVIX Bridge y visualización RAW

[English](../../../en/evidence/delivery-02/README.md) | [Español](README.md)

Estado: **Alcance funcional completo; webcam física validada en Windows**

La Entrega 2 conecta Capture y Vision mediante un protocolo binario versionado.
El proceso C++ conserva la propiedad exclusiva de la cámara; Python solamente
lee memoria compartida, crea arreglos NumPy y presenta el video con OpenCV.

## Alcance verificado

| Requisito | Resultado | Evidencia |
|---|---|---|
| RF-006 — Publicación en memoria compartida | PASS | Header `ORVX`, protocolo v1 y anillo de tres slots |
| RF-007 — Consumo desde Python | PASS | Prueba integrada entre procesos C++ y Python |
| RF-008 — Interpretación con NumPy | PASS | Decodificación NV12 y prueba de geometría BGR |
| RF-009 / RF-010 — Ventana y modo RAW | PASS | Visor `ORVIX Live - RAW` y prueba del flujo de presentación |
| RF-017 — Frames omitidos | PASS | `SequenceTracker` detecta saltos y los muestra en pantalla |
| RF-020 — Utilización del búfer | PASS | Secuencias publicada/consumida, ocupación y sobrescrituras |
| RF-024 — Aislamiento de procesos | PASS | El productor completa aunque el consumidor termine antes |
| RF-025 — Fuente sintética | PASS | Video NV12 reproducible a 1280×720 y 30 FPS |

El diseño binario está definido en el [protocolo IPC v1](../../protocol/ipc-v1.md).

## Validación local

El 8 de septiembre de 2026 se ejecutó el flujo completo con la webcam `GENERAL
WEBCAM` y el backend Windows Media Foundation:

| Medición | Resultado |
|---|---|
| Formato negociado | 1280×720, NV12, 30 FPS nominales |
| Frames recibidos por Python | 30 |
| Primera/última secuencia observada | 1 / 30 |
| Frames omitidos durante el consumo | 0 |
| Frames completados por C++ | 180 |
| FPS físicos medidos | 19.46 |
| Estado final del productor | `COMPLETED` |

Python finalizó después de 30 frames y C++ continuó hasta 180. Las 147
sobrescrituras finales corresponden a frames producidos después de que el
consumidor salió y demuestran la política no bloqueante del anillo.

La validación automática local obtuvo:

- CTest nativo: **16/16 PASS**.
- pytest unitario: **6/6 PASS**.
- integración sintética C++ → memoria compartida → Python: **PASS**.

## Validación con GitHub Actions

- estado: completed
- conclusión: success
- ejecución: https://github.com/Jeamlake/orvix/actions/runs/34273071421
- C++ y puente sintético en Windows: **PASS**
- C++ y puente sintético en Linux: **PASS**
- C++ y puente sintético en macOS: **PASS**
- Python 3.11 en Windows, Linux y macOS: **PASS**
- Compatibilidad con Python 3.8: **PASS**

## Reproducción

La [guía de demostración](demonstration-guide.md) contiene los comandos para
Git Bash, PowerShell, Linux y macOS, junto con la salida y ventana esperadas.

Los frames permanecen en memoria y no se guardan imágenes o videos de forma
automática.
