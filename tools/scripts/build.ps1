# Establece $ErrorActionPreference con "Stop".
$ErrorActionPreference = "Stop"
# Establece $Root con Split-Path (Split-Path $PSScriptRoot -Parent) -Parent.
$Root = Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
# Establece $Build con Join-Path $Root "build".
$Build = Join-Path $Root "build"

# Configura CMake con cmake -S $Root -B $Build -A x64.
cmake -S $Root -B $Build -A x64
# Comprueba if ($LASTEXITCODE -ne 0) { throw "CMake configuration failed ($LASTEXITCODE)." }.
if ($LASTEXITCODE -ne 0) { throw "CMake configuration failed ($LASTEXITCODE)." }
# Compila el proyecto con cmake --build $Build --config Release --parallel.
cmake --build $Build --config Release --parallel
# Comprueba if ($LASTEXITCODE -ne 0) { throw "Native build failed ($LASTEXITCODE)." }.
if ($LASTEXITCODE -ne 0) { throw "Native build failed ($LASTEXITCODE)." }

# Muestra "ORVIX build completed." en la salida indicada.
Write-Host "ORVIX build completed."
