$ErrorActionPreference = 'Stop'
$OrvixRoot = (Split-Path $PSScriptRoot -Parent)
$OrvixArchivePath = Join-Path $OrvixRoot 'entregables/Avance2_ORVIX_2026-10-11/Aplicacion/ORVIX-Avance2-Codigo-0.2.1.zip'
$OrvixTemporaryArchivePath = Join-Path (Split-Path $OrvixArchivePath -Parent) ('.orvix-source-' + [Guid]::NewGuid().ToString('N') + '.zip')
$OrvixCandidates = & git -C $OrvixRoot ls-files --cached --others --exclude-standard
if ($LASTEXITCODE -ne 0) { throw 'Unable to enumerate repository source.' }
# Explicit scope: no .git, environments, logs, databases or user attachments.
$OrvixSourceFiles = $OrvixCandidates | Where-Object {
    $_ -match '^(native/|python/src/|python/tests/|scripts/|packaging/|docs/(es|en)/(requirements/|protocol/|evidence/delivery-02/)|CMakeLists\.txt$|pyproject\.toml$|LICENSE$|README(\.es)?\.md$|\.gitignore$)'
}
Add-Type -AssemblyName System.IO.Compression
Add-Type -AssemblyName System.IO.Compression.FileSystem
$OrvixArchive = [System.IO.Compression.ZipFile]::Open($OrvixTemporaryArchivePath, [System.IO.Compression.ZipArchiveMode]::Create)
try {
    foreach ($relative in $OrvixSourceFiles) {
        $full = [IO.Path]::GetFullPath((Join-Path $OrvixRoot $relative))
        if (-not $full.StartsWith($OrvixRoot + [IO.Path]::DirectorySeparatorChar, [StringComparison]::OrdinalIgnoreCase)) { throw 'Source escaped the repository.' }
        if (Test-Path -LiteralPath $full -PathType Leaf) {
            [void][System.IO.Compression.ZipFileExtensions]::CreateEntryFromFile($OrvixArchive, $full, ('orvix/' + $relative), [System.IO.Compression.CompressionLevel]::Optimal)
        }
    }
} finally { $OrvixArchive.Dispose() }
# Replace only after all entries were written successfully. An existing valid
# deliverable survives an interrupted or failed archive build.
Move-Item -LiteralPath $OrvixTemporaryArchivePath -Destination $OrvixArchivePath -Force
Get-Item -LiteralPath $OrvixArchivePath | Select-Object FullName,Length
