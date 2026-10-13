$ErrorActionPreference = "Stop"
$Root = Split-Path $PSScriptRoot -Parent
$Build = Join-Path $Root "build"

cmake -S $Root -B $Build -A x64
if ($LASTEXITCODE -ne 0) { throw "CMake configuration failed ($LASTEXITCODE)." }
cmake --build $Build --config Release --parallel
if ($LASTEXITCODE -ne 0) { throw "Native build failed ($LASTEXITCODE)." }

Write-Host "ORVIX build completed."
