# Declara las opciones que acepta el script de PowerShell.
param(
    # Declara el tipo y el valor de [string]$PythonVersion = "3.11".
    [string]$PythonVersion = "3.11"
)

# Establece $ErrorActionPreference con "Stop".
$ErrorActionPreference = "Stop"
# Establece $Root con Split-Path (Split-Path $PSScriptRoot -Parent) -Parent.
$Root = Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
# Establece $Venv con Join-Path $Root ".venv".
$Venv = Join-Path $Root ".venv"

# Crea el entorno Python aislado con py "-$PythonVersion" -m venv $Venv.
py "-$PythonVersion" -m venv $Venv
# Comprueba if ($LASTEXITCODE -ne 0) { throw "Virtual environment creation failed." }.
if ($LASTEXITCODE -ne 0) { throw "Virtual environment creation failed." }
# Establece $Python con Join-Path $Venv "Scripts\python.exe".
$Python = Join-Path $Venv "Scripts\python.exe"
# Instala o actualiza dependencias Python con & $Python -m pip install --upgrade pip.
& $Python -m pip install --upgrade pip
# Comprueba if ($LASTEXITCODE -ne 0) { throw "pip upgrade failed." }.
if ($LASTEXITCODE -ne 0) { throw "pip upgrade failed." }
# Instala o actualiza dependencias Python con & $Python -m pip install -e "$Root[dev,vision]".
& $Python -m pip install -e "$Root[dev,vision]"
# Comprueba if ($LASTEXITCODE -ne 0) { throw "Dependency installation failed." }.
if ($LASTEXITCODE -ne 0) { throw "Dependency installation failed." }

# Muestra "ORVIX Python environment ready: $Venv" en la salida indicada.
Write-Host "ORVIX Python environment ready: $Venv"
