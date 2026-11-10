$ErrorActionPreference = 'Stop'
$OrvixRoot = (Split-Path (Split-Path $PSScriptRoot -Parent) -Parent)
$OrvixArchivePath = Join-Path $OrvixRoot 'out/release/ORVIX-Codigo-1.0.0.zip'
New-Item -ItemType Directory -Path (Split-Path $OrvixArchivePath -Parent) -Force | Out-Null
$OrvixTemporaryArchivePath = Join-Path (Split-Path $OrvixArchivePath -Parent) ('.orvix-source-' + [Guid]::NewGuid().ToString('N') + '.zip')
$OrvixCandidates = & git -C $OrvixRoot ls-files --cached --others --exclude-standard
if ($LASTEXITCODE -ne 0) { throw 'Unable to enumerate repository source.' }
# Include technical docs and exclude academic sources and deliverables.
$OrvixSourceFiles = $OrvixCandidates | Where-Object {
    $_ -match '^(src/native/|src/python/src/|src/python/tests/|tools/scripts/|tools/packaging/|docs/(?!(?:[^/]+/)?academic/)|\.github/workflows/|CMakeLists\.txt$|pyproject\.toml$|LICENSE$|README(\.es)?\.md$|\.gitignore$)'
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
