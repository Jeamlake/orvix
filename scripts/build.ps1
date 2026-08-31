$ErrorActionPreference = "Stop"
$Root = Split-Path $PSScriptRoot -Parent
$Build = Join-Path $Root "build"

cmake -S $Root -B $Build -A x64
cmake --build $Build --config Release --parallel

Write-Host "ORVIX build completed."