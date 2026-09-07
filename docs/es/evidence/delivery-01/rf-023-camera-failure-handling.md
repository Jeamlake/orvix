# RF-023 — Tratamiento de fallos de cámara

[English](../../../en/evidence/delivery-01/rf-023-camera-failure-handling.md) | [Español](rf-023-camera-failure-handling.md)

Estado: **Validado**

Issue: **#12**

## Objetivo

Convertir los fallos esperados del dispositivo y del stream en diagnósticos
estables sobre los que pueda actuar una persona o proceso supervisor.

## Condiciones cubiertas

| Condición | Diagnóstico | Salida |
|---|---|---:|
| No hay cámaras para un comando que las requiere | ORV-CAP-100 | 65 |
| Índice fuera de la lista | ORV-CAP-404 | 66 |
| No se puede activar la fuente seleccionada | ORV-CAP-501 | 71 |
| No se puede configurar el formato | ORV-CAP-502 | 72 |
| Dispositivo retirado o invalidado durante `ReadSample` | ORV-CAP-503 | 74 |
| Error, final temprano o ausencia prolongada de muestras | ORV-CAP-503 | 74 |
| Cambio del tipo multimedia durante la captura | ORV-CAP-505 | 75 |

Cada error se escribe en stderr y en el log antes de devolver su código de
salida distinto de cero.

## Validación en el equipo de desarrollo

```powershell
.\build\native\Release\orvix-capture.exe capture --index 999999
$LASTEXITCODE
```

Resultado:

```text
[ORV-CAP-404] Camera selection failed: Camera index 999999 is out of range. Available devices: 1
66
```

`TC-CAP-023` simula una cámara que deja de producir frames y verifica el
diagnóstico `ORV-CAP-503` después del límite de lecturas vacías.

El lector de Media Foundation maneja los HRESULT de dispositivo invalidado y
desconectado, las banderas de error y fin de stream, y cambios inesperados del
tipo multimedia. No se forzó una desconexión física; la prueba de stream
detenido cubre ese límite sin hardware en CI.

## Estado de aceptación

- caso sin cámara: **PASS**
- índice inválido en el equipo físico: **PASS**
- fallos de apertura y formato mapeados: **PASS**
- invalidación y terminación mapeadas: **PASS**
- stream detenido rechazado por prueba automática: **PASS**
- códigos de diagnóstico y salida estables: **PASS**
