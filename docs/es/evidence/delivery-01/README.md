# Entrega 1 — Fundamentos y captura nativa

[English](../../../en/evidence/delivery-01/README.md) | [Español](README.md)

Estado: **Alcance funcional completo; hardware validado en Windows**

Este directorio reúne evidencia reproducible para la primera entrega calificada
de ORVIX. El comportamiento físico se valida en el equipo de desarrollo; la
lógica independiente del hardware también se comprueba con CTest y GitHub
Actions.

## Índice de evidencia

| Alcance | Requisito o criterio | Resultado | Evidencia |
|---|---|---|---|
| Toolchain | CA-008 | PASS | [Validación del entorno](environment-validation.md) |
| Enumeración | RF-001 / CA-001 | PASS | [RF-001](rf-001-device-enumeration.md) |
| Selección | RF-002 | PASS | [RF-002](rf-002-camera-selection.md) |
| Apertura | RF-003 / CA-002 | PASS | [RF-003](rf-003-open-selected-camera.md) |
| Negociación de formato | RNF-001 | PASS | [Negociación](format-negotiation.md) |
| Captura continua | RF-004 / CA-003 / CA-004 | PASS | [RF-004](rf-004-continuous-frame-capture.md) |
| Metadatos | RF-005 / CA-005 | PASS | [RF-005](rf-005-frame-metadata.md) |
| FPS de captura | RF-015 / CA-006 | PASS | [RF-015](rf-015-capture-fps.md) |
| Diagnósticos | RF-023 / CA-007 | PASS | [RF-023](rf-023-camera-failure-handling.md) |
| Logging estructurado | RF-026 | PASS | [RF-026](rf-026-structured-logging.md) |
| Validación automática | CA-009 | PASS | [GitHub Actions](#validación-con-github-actions) |

## Reproducción

Desde una terminal en la raíz del repositorio:

```bash
./scripts/setup.sh
./scripts/build.sh
./scripts/test.sh
./build/native/Release/orvix-capture.exe devices
./build/native/Release/orvix-capture.exe capture --index 0
```

En Linux y macOS el ejecutable se encuentra en
`./build/native/orvix-capture`. El último comando activa la webcam física. La
evidencia esperada incluye 120 registros, `Frames captured: 120`, secuencia de
1 a 120, `Sequence status: STRICTLY_INCREASING` y
`Capture status: CAPTURE_COMPLETED`.

## Validación con GitHub Actions

- estado: completed
- conclusión: success
- commit: 4e37418f655d65d5dbbb444d615c31e929329982
- ejecución: https://github.com/Jeamlake/orvix/actions/runs/34170322943
- C++ nativo en Windows, Linux y macOS: **PASS**
- Python 3.11 en Windows, Linux y macOS: **PASS**
- Compatibilidad con Python 3.8: **PASS**

El CI realiza compilaciones limpias y ejecuta las pruebas independientes del
hardware. La captura física se documenta en las páginas de evidencia de cada
requisito.

## Límite de la entrega

La Entrega 1 termina con captura nativa, metadatos por frame, FPS y
diagnósticos. El transporte por memoria compartida y el consumo desde Python
comienzan en la Entrega 2. Los frames son efímeros y no se guarda ninguna
imagen de manera predeterminada.

La extensión multiplataforma está registrada en la
[validación nativa multiplataforma](../cross-platform-native-validation.md).
