param(
    [string]$PythonVersion = "3.11"
)

$ErrorActionPreference = "Stop"
$Root = Split-Path $PSScriptRoot -Parent
$Venv = Join-Path $Root ".venv"

py "-$PythonVersion" -m venv $Venv
if ($LASTEXITCODE -ne 0) { throw "Virtual environment creation failed." }
$Python = Join-Path $Venv "Scripts\python.exe"
& $Python -m pip install --upgrade pip
if ($LASTEXITCODE -ne 0) { throw "pip upgrade failed." }
& $Python -m pip install -e "$Root[dev,vision]"
if ($LASTEXITCODE -ne 0) { throw "Dependency installation failed." }

Write-Host "ORVIX Python environment ready: $Venv"
