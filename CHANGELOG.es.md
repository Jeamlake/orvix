# Historial de cambios

[English](CHANGELOG.md) | [Español](CHANGELOG.es.md)

## [Sin publicar]

### Agregado

- Secciones paralelas de documentación en inglés y español con registros
  equivalentes de arquitectura, requisitos, plataformas, decisiones y evidencia.
- Contrato nativo de cámara independiente de plataforma y factoría seleccionada
  durante la compilación.
- Enumeración, negociación de formato y captura con memoria mapeada mediante
  Linux V4L2.
- Enumeración, negociación y callbacks de captura con macOS AVFoundation,
  incluida la descripción de privacidad de cámara.
- CI nativo y Python en Windows, Ubuntu y macOS.
- Scripts shell de configuración, compilación y pruebas para Linux, macOS y Git
  Bash en Windows.
- Ruta de compatibilidad con Python 3.8 para el objetivo heredado Windows 7.
- Documentación de instalación, permisos y ejecución multiplataforma.
- Descubrimiento y negociación del formato nativo con objetivo inicial de
  1280×720 a 30 FPS.
- Comando nativo `orvix-capture formats --index <N>`.
- Adquisición continua de muestras para RF-004.
- Comando nativo `orvix-capture capture --index <N>`.
- Pruebas independientes del hardware para selección de formato y secuencia.
- Metadatos de secuencia, timestamp, resolución, formato y bytes para RF-005.
- Medición de FPS efectivos para RF-015.
- Diagnósticos y códigos de salida estables para RF-023.
- Logging estructurado en `logs/orvix-capture.log` para RF-026.
- Índice de evidencia de la Entrega 1 y referencia de diagnósticos.
- Apertura de la cámara seleccionada mediante Media Foundation para RF-003.
- Comando nativo `orvix-capture open --index <N>`.
- Administración RAII de `IMFMediaSource`.
- Enumeración de dispositivos mediante Media Foundation para RF-001.
- Comando nativo `orvix-capture devices`.
- Arquitectura monorepo inicial, requisitos, ADR, estructuras C++ y Python, y
  configuración de integración continua.
