# Especificación de requisitos de software de ORVIX

[English](../../en/requirements/software-requirements.md) | [Español](software-requirements.md)

Versión: 0.2

## Requisitos funcionales

| ID | Requisito | Prioridad | Entrega |
|---|---|---|---|
| RF-001 | Enumerar los dispositivos de captura de video disponibles. | Crítica | 1 |
| RF-002 | Permitir seleccionar un dispositivo de captura. | Crítica | 1 |
| RF-003 | Abrir el dispositivo seleccionado desde C++ mediante la API nativa del sistema operativo. | Crítica | 1 |
| RF-004 | Adquirir frames de manera continua. | Crítica | 1 |
| RF-005 | Exponer metadatos por frame. | Crítica | 1 |
| RF-006 | Publicar frames mediante memoria compartida. | Crítica | 2 |
| RF-007 | Permitir que Python consuma los frames compartidos sin abrir directamente la cámara. | Crítica | 2 |
| RF-008 | Interpretar los bytes del frame mediante NumPy. | Crítica | 2 |
| RF-009 | Mostrar el video recibido. | Crítica | 2 |
| RF-010 | Proporcionar visualización RAW. | Alta | 2 |
| RF-011 | Proporcionar procesamiento en escala de grises. | Alta | 3 |
| RF-012 | Proporcionar detección de bordes. | Alta | 3 |
| RF-013 | Proporcionar un algoritmo de detección con OpenCV. | Crítica | 3 |
| RF-014 | Cambiar el modo de procesamiento sin reiniciar la captura. | Alta | 3 |
| RF-015 | Medir los FPS de captura. | Crítica | 1 |
| RF-016 | Medir los FPS de procesamiento. | Crítica | 3 |
| RF-017 | Detectar frames omitidos. | Crítica | 2 |
| RF-018 | Estimar la latencia de procesamiento. | Alta | 3 |
| RF-019 | Contar detecciones por minuto. | Crítica | 3 |
| RF-020 | Informar la utilización de memoria compartida. | Crítica | 2 |
| RF-021 | Persistir la telemetría histórica. | Alta | 3 |
| RF-022 | Exponer información de la sesión. | Alta | 3 |
| RF-023 | Detectar la desconexión de la cámara. | Alta | 1 |
| RF-024 | Evitar que un fallo de Vision finalice necesariamente Capture. | Alta | 2 |
| RF-025 | Proporcionar una fuente de frames sintéticos. | Alta | 2 |
| RF-026 | Registrar eventos y fallos relevantes. | Alta | 1 |
| RF-027 | Proporcionar backends nativos para Windows, Linux y macOS detrás de una interfaz común. | Crítica | 1 |

## Requisito de arquitectura

### RF-ARCH-001

Python no debe acceder directamente a la cámara durante el flujo normal de
ORVIX.

```text
Cámara
-> Controlador
-> Backend nativo del sistema operativo
-> C++
-> Memoria compartida
-> Python
-> NumPy
-> OpenCV
```

## Requisitos no funcionales

- RNF-001 Rendimiento: objetivo inicial de 1280×720 a 30 FPS.
- RNF-002 Baja latencia: evitar archivos temporales y serialización innecesaria.
- RNF-003 Minimizar copias entre memoria compartida y NumPy cuando sea viable.
- RNF-004 Modularidad entre Capture, Bridge, Vision e Insight.
- RNF-005 Observabilidad estructurada.
- RNF-006 Reproducción desde un checkout limpio.
- RNF-007 Límites de módulos mantenibles.
- RNF-008 Pruebas automatizadas independientes del hardware.
- RNF-009 CI en pushes y pull requests.
- RNF-010 Los frames no se guardan de manera predeterminada.
- RNF-011 Diagnósticos controlados para fallos esperados.
- RNF-012 Versionado explícito del protocolo IPC.
- RNF-013 Portabilidad de código y compilación en Windows, Linux y macOS.

## Criterios de aceptación de la Entrega 1

| ID | Criterio |
|---|---|
| CA-001 | Se puede enumerar al menos una webcam. |
| CA-002 | C++ abre la webcam. |
| CA-003 | Se pueden adquirir frames consecutivos. |
| CA-004 | La secuencia de frames aumenta. |
| CA-005 | Se pueden informar los metadatos del frame. |
| CA-006 | Se pueden medir los FPS de captura. |
| CA-007 | Los fallos esperados de cámara producen diagnósticos. |
| CA-008 | El checkout limpio se puede compilar usando la documentación. |
| CA-009 | El CI independiente del hardware finaliza correctamente. |
| CA-010 | El núcleo nativo compila y pasa sus pruebas en Windows, Linux y macOS. |

## Criterios de aceptación de la Entrega 2

| ID | Criterio |
|---|---|
| CA2-001 | C++ publica payload y metadatos de frames en un protocolo de memoria compartida versionado. |
| CA2-002 | Python consume frames sin abrir la cámara. |
| CA2-003 | NumPy interpreta los bytes según formato, dimensiones y stride. |
| CA2-004 | OpenCV muestra el modo RAW y permite salir con `Q` o `Esc`. |
| CA2-005 | El consumidor detecta saltos en la secuencia. |
| CA2-006 | El sistema informa utilización del búfer y sobrescrituras. |
| CA2-007 | Capture continúa si Vision termina antes que el productor. |
| CA2-008 | Una fuente sintética valida el puente completo sin hardware en Windows, Linux y macOS. |
