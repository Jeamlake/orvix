# Establece $ErrorActionPreference con "Stop".
$ErrorActionPreference = "Stop"
# Establece $Root con Split-Path (Split-Path $PSScriptRoot -Parent) -Parent.
$Root = Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
# Establece $Python con Join-Path $Root ".venv\Scripts\python.exe".
$Python = Join-Path $Root ".venv\Scripts\python.exe"

# Comprueba if (-not (Test-Path $Python)).
if (-not (Test-Path $Python)) {
    # Detiene el script con el error "Run tools\scripts\setup.ps1 first.".
    throw "Run tools\scripts\setup.ps1 first."
}

# Ejecuta las pruebas de CTest con ctest --test-dir (Join-Path $Root "build") -C Release --output-on-failure.
ctest --test-dir (Join-Path $Root "build") -C Release --output-on-failure
# Comprueba if ($LASTEXITCODE -ne 0) { throw "Native tests failed ($LASTEXITCODE)." }.
if ($LASTEXITCODE -ne 0) { throw "Native tests failed ($LASTEXITCODE)." }
# Ejecuta pytest con & $Python -m pytest.
& $Python -m pytest
# Comprueba if ($LASTEXITCODE -ne 0) { throw "Python tests failed ($LASTEXITCODE)." }.
if ($LASTEXITCODE -ne 0) { throw "Python tests failed ($LASTEXITCODE)." }
