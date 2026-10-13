param([string]$InnoCompiler)
$ErrorActionPreference = "Stop"
$OrvixRoot = Split-Path $PSScriptRoot -Parent
$OrvixPython = Join-Path $OrvixRoot '.venv/Scripts/python.exe'
$OrvixOutput = Join-Path $OrvixRoot 'out/desktop-package'
$OrvixRelease = Join-Path $OrvixRoot 'entregables/Avance2_ORVIX_2026-10-11/Aplicacion'
if (-not (Test-Path -LiteralPath $OrvixPython)) { throw 'Run setup.ps1 first.' }
if (-not $InnoCompiler) {
    $InnoCompiler = Join-Path $OrvixOutput 'tools/InnoSetup/ISCC.exe'
    if (-not (Test-Path -LiteralPath $InnoCompiler)) {
        $InnoCompiler = 'C:/Program Files (x86)/Inno Setup 6/ISCC.exe'
    }
}
if (-not (Test-Path -LiteralPath $InnoCompiler)) { throw 'Provide the path to Inno Setup 6 ISCC.exe.' }
& (Join-Path $PSScriptRoot 'build.ps1')
& $OrvixPython -m pip install -e "$OrvixRoot[desktop,package]"
if ($LASTEXITCODE -ne 0) { throw 'Desktop dependencies failed.' }
New-Item -ItemType Directory -Path $OrvixOutput,$OrvixRelease -Force | Out-Null
& $OrvixPython (Join-Path $PSScriptRoot 'prepare-package.py')
if ($LASTEXITCODE -ne 0) { throw 'Package assets failed.' }
& $OrvixPython -m PyInstaller --noconfirm --distpath (Join-Path $OrvixOutput 'dist') --workpath (Join-Path $OrvixOutput 'build') (Join-Path $OrvixRoot 'packaging/orvix.spec')
if ($LASTEXITCODE -ne 0) { throw 'PyInstaller failed.' }
& $InnoCompiler "/DAppSource=$OrvixOutput/dist/ORVIX" "/DOutputPath=$OrvixRelease" "/DIconPath=$OrvixOutput/orvix.ico" (Join-Path $OrvixRoot 'packaging/installer.iss')
if ($LASTEXITCODE -ne 0) { throw 'Installer compilation failed.' }
Write-Host "ORVIX installer: $OrvixRelease"
