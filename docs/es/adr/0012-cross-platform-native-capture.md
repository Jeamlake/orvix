# ADR: Backends nativos de captura multiplataforma

[English](../../en/adr/0012-cross-platform-native-capture.md) | [Español](0012-cross-platform-native-capture.md)

Estado: **Aceptado**

## Contexto

ORVIX debe ejecutarse en Windows, Linux y macOS y conservar la regla de
arquitectura según la cual C++ administra la cámara. Media Foundation no puede
capturar fuera de Windows y trasladar el flujo normal a Python violaría
RF-ARCH-001.

## Decisión

Definir un contrato C++ `Camera` independiente de la plataforma y seleccionar
un backend nativo durante la compilación:

- Windows: Media Foundation;
- Linux: Video4Linux2;
- macOS: AVFoundation.

La enumeración de dispositivos también utiliza un contrato común. El CLI
obtiene ambos objetos mediante una factoría de plataforma y no incluye headers
específicos de un backend. La secuencia, los metadatos, el cálculo de FPS, los
diagnósticos y el logging permanecen compartidos.

## Consecuencias

CMake compila y enlaza únicamente el backend del sistema anfitrión. El CI debe
compilar y ejecutar pruebas independientes del hardware en las tres
plataformas. La validación con cámara física continúa siendo específica de cada
plataforma porque los runners hospedados no incluyen webcams.

Windows 7 SP1 utiliza Media Foundation, limita el objetivo de API Win32 a la
versión 6.1 y enlaza estáticamente el runtime de MSVC. Su ruta heredada de
Python utiliza Python 3.8.
