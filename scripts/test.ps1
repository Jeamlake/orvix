$ErrorActionPreference = "Stop"
$Root = Split-Path $PSScriptRoot -Parent
$Python = Join-Path $Root ".venv\Scripts\python.exe"

if (-not (Test-Path $Python)) {
    throw "Run scripts\setup.ps1 first."
}

ctest --test-dir (Join-Path $Root "build") -C Release --output-on-failure
& $Python -m pytest
