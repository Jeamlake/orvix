# Negociación nativa del formato de video

[English](../../../en/evidence/delivery-01/format-negotiation.md) | [Español](format-negotiation.md)

Estado: **Validado**

Issue: **#7**

## Objetivo

Descubrir los tipos multimedia nativos de la cámara seleccionada y configurar
el formato disponible más cercano al objetivo inicial de 1280×720 a 30 FPS.

## Implementación

ORVIX utiliza `IMFSourceReader::GetNativeMediaType` para inspeccionar los
formatos. Cada formato válido registra:

- índice nativo del tipo multimedia;
- ancho y alto;
- numerador y denominador de FPS;
- formato de píxel;
- indicador de compresión.

La política de selección prioriza:

1. resolución objetivo exacta;
2. FPS más cercanos;
3. formatos de píxel eficientes sin compresión;
4. orden estable del tipo nativo.

El tipo elegido se aplica con `IMFSourceReader::SetCurrentMediaType` y se vuelve
a leer para verificarlo.

## CLI

```text
orvix-capture formats --index <N>
```

## Validación en el equipo de desarrollo

`GENERAL WEBCAM` expuso 15 tipos utilizables. Entre ellos:

```text
[0] 1920x1080 @ 30.00 FPS, NV12
[1] 1920x1080 @ 30.00 FPS, MJPG (compressed)
[2] 1280x720 @ 30.00 FPS, NV12
[3] 1280x720 @ 30.00 FPS, MJPG (compressed)
[4] 640x360 @ 30.00 FPS, NV12
[10] 640x360 @ 30.00 FPS, YUY2
```

Resultado:

```text
Target resolution: 1280x720
Target FPS: 30

Negotiated resolution: 1280x720
Negotiated pixel format: NV12
Negotiated FPS: 30.00
Negotiated native type index: 2

Format status: CONFIGURED
```

## Estado de aceptación

- formatos nativos detectados: **PASS**
- resolución objetivo configurada: **PASS**
- FPS objetivo configurados: **PASS**
- formato de píxel informado: **PASS**
- tipo configurado leído nuevamente: **PASS**
- pruebas de selección independientes del hardware: **PASS**

## Validación con GitHub Actions

- estado: completed
- conclusión: success
- commit: e4455f3b07ea7bebfce305a82eb0eb77b8123d3b
- ejecución: https://github.com/Jeamlake/orvix/actions/runs/34082967234

El CI ejecuta una compilación MSVC limpia y la suite de CTest. La selección se
prueba sin hardware y la negociación física se valida en el equipo de
desarrollo.

## Límite del alcance

Los FPS negociados son la configuración del tipo multimedia. La medición de
FPS efectivos pertenece a RF-015.
