$ErrorActionPreference = "Stop"
$Root = Split-Path $PSScriptRoot -Parent
$Python = Join-Path $Root ".venv\Scripts\python.exe"

if (-not (Test-Path $Python)) {
    throw "Run scripts\setup.ps1 first."
}

ctest --test-dir (Join-Path $Root "build") -C Release --output-on-failure
if ($LASTEXITCODE -ne 0) { throw "Native tests failed ($LASTEXITCODE)." }
& $Python -m pytest
if ($LASTEXITCODE -ne 0) { throw "Python tests failed ($LASTEXITCODE)." }
