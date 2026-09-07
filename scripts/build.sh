#!/usr/bin/env sh
set -eu

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
BUILD="$ROOT/build"

case "$(uname -s)" in
    MINGW*|MSYS*|CYGWIN*)
        cmake -S "$ROOT" -B "$BUILD" -A x64
        ;;
    *)
        cmake -S "$ROOT" -B "$BUILD" -DCMAKE_BUILD_TYPE=Release
        ;;
esac

cmake --build "$BUILD" --config Release --parallel

printf 'ORVIX build completed.\n'
