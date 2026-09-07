#!/usr/bin/env sh
set -eu

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
PYTHON="$ROOT/.venv/bin/python"

if [ ! -x "$PYTHON" ]; then
    printf 'Run scripts/setup.sh first.\n' >&2
    exit 1
fi

ctest --test-dir "$ROOT/build" -C Release --output-on-failure
"$PYTHON" -m pytest
