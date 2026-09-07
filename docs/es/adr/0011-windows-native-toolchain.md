# ADR: Toolchain nativo de Windows

[English](../../en/adr/0011-windows-native-toolchain.md) | [Español](0011-windows-native-toolchain.md)

Estado: **Aceptado para Windows**

## Contexto

La validación del entorno confirmó el toolchain de Microsoft instalado.

## Decisión

Utilizar MSVC, Windows SDK y CMake para compilar en Windows.

## Consecuencias

Linux utiliza GCC y macOS utiliza Apple Clang. MinGW no es un toolchain de
Windows validado.
