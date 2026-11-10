#!/usr/bin/env sh
# Configura las opciones de ejecución del script con set -eu.
set -eu

# Establece ROOT con $(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd).
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
# Establece BUILD con "$ROOT/build".
BUILD="$ROOT/build"

# Selecciona las opciones del script según "$(uname -s)".
case "$(uname -s)" in
    # Selecciona las plataformas que coinciden con MINGW*|MSYS*|CYGWIN*.
    MINGW*|MSYS*|CYGWIN*)
        # Configura CMake con cmake -S "$ROOT" -B "$BUILD" -A x64.
        cmake -S "$ROOT" -B "$BUILD" -A x64
        ;;
    # Ejecuta la alternativa cuando no coincide el caso anterior.
    *)
        # Configura CMake con cmake -S "$ROOT" -B "$BUILD" -DCMAKE_BUILD_TYPE=Release.
        cmake -S "$ROOT" -B "$BUILD" -DCMAKE_BUILD_TYPE=Release
        ;;
esac

# Compila el proyecto con cmake --build "$BUILD" --config Release --parallel.
cmake --build "$BUILD" --config Release --parallel

# Muestra 'ORVIX build completed.\n' en la salida indicada.
printf 'ORVIX build completed.\n'
