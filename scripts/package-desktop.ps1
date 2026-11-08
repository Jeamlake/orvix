param(
    [string]$InnoCompiler,
    [string]$Python,
    [switch]$Portable,
    [switch]$SkipBuild
)

$ErrorActionPreference = "Stop"
$OrvixRoot = Split-Path $PSScriptRoot -Parent
if (-not $Python) { $Python = Join-Path $OrvixRoot '.venv/Scripts/python.exe' }
if (-not (Test-Path -LiteralPath $Python)) { throw 'Run setup.ps1 first or pass -Python.' }
$Python = (Resolve-Path -LiteralPath $Python).Path
$OrvixOutput = Join-Path $OrvixRoot 'out/desktop-package'
$OrvixRelease = Join-Path $OrvixRoot 'out/release'

if (-not $Portable) {
    if (-not $InnoCompiler) { $InnoCompiler = 'C:/Program Files (x86)/Inno Setup 6/ISCC.exe' }
    if (-not (Test-Path -LiteralPath $InnoCompiler)) { throw 'Pass -InnoCompiler or use -Portable.' }
}
if (-not $SkipBuild) { & (Join-Path $PSScriptRoot 'build.ps1') }
if (-not (Test-Path -LiteralPath (Join-Path $OrvixRoot 'build/native/Release/orvix-capture.exe'))) {
    throw 'Build the x64 Release engine before packaging.'
}

New-Item -ItemType Directory -Path $OrvixOutput -Force | Out-Null
& $Python (Join-Path $PSScriptRoot 'prepare-package.py')
if ($LASTEXITCODE -ne 0) { throw 'Package assets failed. Install .[desktop,package] first.' }
& $Python -m PyInstaller --noconfirm --distpath (Join-Path $OrvixOutput 'dist') --workpath (Join-Path $OrvixOutput 'build') (Join-Path $OrvixRoot 'packaging/orvix.spec')
if ($LASTEXITCODE -ne 0) { throw 'PyInstaller failed.' }
Write-Host "ORVIX portable: $OrvixOutput/dist/ORVIX"
if (-not $Portable) {
    New-Item -ItemType Directory -Path $OrvixRelease -Force | Out-Null
    & $InnoCompiler "/DAppSource=$OrvixOutput/dist/ORVIX" "/DOutputPath=$OrvixRelease" "/DIconPath=$OrvixOutput/orvix.ico" (Join-Path $OrvixRoot 'packaging/installer.iss')
    if ($LASTEXITCODE -ne 0) { throw 'Installer compilation failed.' }
    Write-Host "ORVIX installer: $OrvixRelease"
}
