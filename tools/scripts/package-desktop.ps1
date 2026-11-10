# Declara las opciones que acepta el script de PowerShell.
param(
    # Declara el tipo y el valor de [string]$InnoCompiler,.
    [string]$InnoCompiler,
    # Declara el tipo y el valor de [string]$Python,.
    [string]$Python,
    # Declara el tipo y el valor de [switch]$Portable,.
    [switch]$Portable,
    # Declara el tipo y el valor de [switch]$SkipBuild.
    [switch]$SkipBuild
)

# Establece $ErrorActionPreference con "Stop".
$ErrorActionPreference = "Stop"
# Establece $OrvixRoot con Split-Path (Split-Path $PSScriptRoot -Parent) -Parent.
$OrvixRoot = Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
# Comprueba if (-not $Python) { $Python = Join-Path $OrvixRoot '.venv/Scripts/python.exe' }.
if (-not $Python) { $Python = Join-Path $OrvixRoot '.venv/Scripts/python.exe' }
# Comprueba if (-not (Test-Path -LiteralPath $Python)) { throw 'Run tools/scripts/setup.ps1 first o…
if (-not (Test-Path -LiteralPath $Python)) { throw 'Run tools/scripts/setup.ps1 first or pass -Python.' }
# Establece $Python con (Resolve-Path -LiteralPath $Python).Path.
$Python = (Resolve-Path -LiteralPath $Python).Path
# Establece $OrvixOutput con Join-Path $OrvixRoot 'out/desktop-package'.
$OrvixOutput = Join-Path $OrvixRoot 'out/desktop-package'
# Establece $OrvixRelease con Join-Path $OrvixRoot 'out/release'.
$OrvixRelease = Join-Path $OrvixRoot 'out/release'

# Comprueba if (-not $Portable).
if (-not $Portable) {
    # Comprueba if (-not $InnoCompiler) { $InnoCompiler = 'C:/Program Files (x86)/Inno Setup 6/ISCC.exe…
    if (-not $InnoCompiler) { $InnoCompiler = 'C:/Program Files (x86)/Inno Setup 6/ISCC.exe' }
    # Comprueba if (-not (Test-Path -LiteralPath $InnoCompiler)) { throw 'Pass -InnoCompiler or use -Po…
    if (-not (Test-Path -LiteralPath $InnoCompiler)) { throw 'Pass -InnoCompiler or use -Portable.' }
}
# Comprueba if (-not $SkipBuild) { & (Join-Path $PSScriptRoot 'build.ps1') }.
if (-not $SkipBuild) { & (Join-Path $PSScriptRoot 'build.ps1') }
# Comprueba if (-not (Test-Path -LiteralPath (Join-Path $OrvixRoot 'build/native/Release/orvix-capt…
if (-not (Test-Path -LiteralPath (Join-Path $OrvixRoot 'build/native/Release/orvix-capture.exe'))) {
    # Detiene el script con el error 'Build the x64 Release engine before packaging.'.
    throw 'Build the x64 Release engine before packaging.'
}

# Crea la ruta con New-Item -ItemType Directory -Path $OrvixOutput -Force | Out-Null.
New-Item -ItemType Directory -Path $OrvixOutput -Force | Out-Null
# Ejecuta el comando indicado por $Python (Join-Path $PSScriptRoot 'prepare-package.py').
& $Python (Join-Path $PSScriptRoot 'prepare-package.py')
# Comprueba if ($LASTEXITCODE -ne 0) { throw 'Package assets failed. Install .[desktop,package] fir…
if ($LASTEXITCODE -ne 0) { throw 'Package assets failed. Install .[desktop,package] first.' }
# Ejecuta el comando indicado por $Python -m PyInstaller --noconfirm --distpath (Join-Path $OrvixOutput 'dist') --workpat…
& $Python -m PyInstaller --noconfirm --distpath (Join-Path $OrvixOutput 'dist') --workpath (Join-Path $OrvixOutput 'build') (Join-Path $OrvixRoot 'tools/packaging/orvix.spec')
# Comprueba if ($LASTEXITCODE -ne 0) { throw 'PyInstaller failed.' }.
if ($LASTEXITCODE -ne 0) { throw 'PyInstaller failed.' }
# Muestra "ORVIX portable: $OrvixOutput/dist/ORVIX" en la salida indicada.
Write-Host "ORVIX portable: $OrvixOutput/dist/ORVIX"
# Comprueba if (-not $Portable).
if (-not $Portable) {
    # Crea la ruta con New-Item -ItemType Directory -Path $OrvixRelease -Force | Out-Null.
    New-Item -ItemType Directory -Path $OrvixRelease -Force | Out-Null
    # Ejecuta el comando indicado por $InnoCompiler "/DAppSource=$OrvixOutput/dist/ORVIX" "/DOutputPath=$OrvixRelease" "/DIco…
    & $InnoCompiler "/DAppSource=$OrvixOutput/dist/ORVIX" "/DOutputPath=$OrvixRelease" "/DIconPath=$OrvixOutput/orvix.ico" (Join-Path $OrvixRoot 'tools/packaging/installer.iss')
    # Comprueba if ($LASTEXITCODE -ne 0) { throw 'Installer compilation failed.' }.
    if ($LASTEXITCODE -ne 0) { throw 'Installer compilation failed.' }
    # Muestra "ORVIX installer: $OrvixRelease" en la salida indicada.
    Write-Host "ORVIX installer: $OrvixRelease"
}
