#!/usr/bin/env sh
# Configura las opciones de ejecución del script con set -eu.
set -eu

# Establece ROOT con $(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd).
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)

# Selecciona las opciones del script según "$(uname -s)".
case "$(uname -s)" in
    # Selecciona las plataformas que coinciden con MINGW*|MSYS*|CYGWIN*.
    MINGW*|MSYS*|CYGWIN*)
        # Establece PYTHON con "$ROOT/.venv/Scripts/python.exe".
        PYTHON="$ROOT/.venv/Scripts/python.exe"
        ;;
    # Ejecuta la alternativa cuando no coincide el caso anterior.
    *)
        # Establece PYTHON con "$ROOT/.venv/bin/python".
        PYTHON="$ROOT/.venv/bin/python"
        ;;
esac

# Comprueba if [ ! -f "$PYTHON" ].
if [ ! -f "$PYTHON" ]; then
    # Muestra 'Run tools/scripts/setup.sh first.\n' >&2 en la salida indicada.
    printf 'Run tools/scripts/setup.sh first.\n' >&2
    # Finaliza con el código 1.
    exit 1
fi

# Ejecuta las pruebas de CTest con ctest --test-dir "$ROOT/build" -C Release --output-on-failure.
ctest --test-dir "$ROOT/build" -C Release --output-on-failure
# Ejecuta pytest con "$PYTHON" -m pytest.
"$PYTHON" -m pytest
