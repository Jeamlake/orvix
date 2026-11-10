#!/usr/bin/env sh
# Configura las opciones de ejecución del script con set -eu.
set -eu

# Establece ROOT con $(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd).
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
# Establece VENV con "$ROOT/.venv".
VENV="$ROOT/.venv"

# Selecciona las opciones del script según "$(uname -s)".
case "$(uname -s)" in
    # Selecciona las plataformas que coinciden con MINGW*|MSYS*|CYGWIN*.
    MINGW*|MSYS*|CYGWIN*)
        # Establece VENV_PYTHON con "$VENV/Scripts/python.exe".
        VENV_PYTHON="$VENV/Scripts/python.exe"
        # Comprueba if [ -n "${PYTHON:-}" ].
        if [ -n "${PYTHON:-}" ]; then
            # Crea el entorno Python aislado con "$PYTHON" -m venv "$VENV".
            "$PYTHON" -m venv "$VENV"
        # Comprueba elif command -v py >/dev/null 2>&1.
        elif command -v py >/dev/null 2>&1; then
            # Crea el entorno Python aislado con py "-${PYTHON_VERSION:-3.11}" -m venv "$VENV".
            py "-${PYTHON_VERSION:-3.11}" -m venv "$VENV"
        # Comprueba elif command -v python >/dev/null 2>&1.
        elif command -v python >/dev/null 2>&1; then
            # Crea el entorno Python aislado con python -m venv "$VENV".
            python -m venv "$VENV"
        # Ejecuta la alternativa cuando no coincide el caso anterior.
        else
            # Muestra 'Python was not found in Git Bash.\n' >&2 en la salida indicada.
            printf 'Python was not found in Git Bash.\n' >&2
            # Finaliza con el código 1.
            exit 1
        fi
        ;;
    # Ejecuta la alternativa cuando no coincide el caso anterior.
    *)
        # Establece VENV_PYTHON con "$VENV/bin/python".
        VENV_PYTHON="$VENV/bin/python"
        # Crea el entorno Python aislado con "${PYTHON:-python3}" -m venv "$VENV".
        "${PYTHON:-python3}" -m venv "$VENV"
        ;;
esac

# Instala o actualiza dependencias Python con "$VENV_PYTHON" -m pip install --upgrade pip.
"$VENV_PYTHON" -m pip install --upgrade pip
(
    # Cambia el directorio de trabajo a "$ROOT".
    cd "$ROOT"
    # Instala o actualiza dependencias Python con "$VENV_PYTHON" -m pip install -e ".[dev,vision]".
    "$VENV_PYTHON" -m pip install -e ".[dev,vision]"
)

# Muestra 'ORVIX Python environment ready: %s\n' "$VENV" en la salida indicada.
printf 'ORVIX Python environment ready: %s\n' "$VENV"
