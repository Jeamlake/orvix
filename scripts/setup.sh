#!/usr/bin/env sh
set -eu

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
VENV="$ROOT/.venv"

case "$(uname -s)" in
    MINGW*|MSYS*|CYGWIN*)
        VENV_PYTHON="$VENV/Scripts/python.exe"
        if [ -n "${PYTHON:-}" ]; then
            "$PYTHON" -m venv "$VENV"
        elif command -v py >/dev/null 2>&1; then
            py "-${PYTHON_VERSION:-3.11}" -m venv "$VENV"
        elif command -v python >/dev/null 2>&1; then
            python -m venv "$VENV"
        else
            printf 'Python was not found in Git Bash.\n' >&2
            exit 1
        fi
        ;;
    *)
        VENV_PYTHON="$VENV/bin/python"
        "${PYTHON:-python3}" -m venv "$VENV"
        ;;
esac

"$VENV_PYTHON" -m pip install --upgrade pip
(
    cd "$ROOT"
    "$VENV_PYTHON" -m pip install -e ".[dev,vision]"
)

printf 'ORVIX Python environment ready: %s\n' "$VENV"
