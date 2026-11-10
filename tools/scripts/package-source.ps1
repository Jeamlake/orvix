# Establece $ErrorActionPreference con 'Stop'.
$ErrorActionPreference = 'Stop'
# Establece $OrvixRoot con (Split-Path (Split-Path $PSScriptRoot -Parent) -Parent).
$OrvixRoot = (Split-Path (Split-Path $PSScriptRoot -Parent) -Parent)
# Establece $OrvixArchivePath con Join-Path $OrvixRoot 'out/release/ORVIX-Codigo-1.0.0.zip'.
$OrvixArchivePath = Join-Path $OrvixRoot 'out/release/ORVIX-Codigo-1.0.0.zip'
# Crea la ruta con New-Item -ItemType Directory -Path (Split-Path $OrvixArchivePath -Paren…
New-Item -ItemType Directory -Path (Split-Path $OrvixArchivePath -Parent) -Force | Out-Null
# Establece $OrvixTemporaryArchivePath con Join-Path (Split-Path $OrvixArchivePath -Parent) ('.orvix-source-' + [Guid]::NewGuid().…
$OrvixTemporaryArchivePath = Join-Path (Split-Path $OrvixArchivePath -Parent) ('.orvix-source-' + [Guid]::NewGuid().ToString('N') + '.zip')
# Establece $OrvixCandidates con & git -C $OrvixRoot ls-files --cached --others --exclude-standard.
$OrvixCandidates = & git -C $OrvixRoot ls-files --cached --others --exclude-standard
# Comprueba if ($LASTEXITCODE -ne 0) { throw 'Unable to enumerate repository source.' }.
if ($LASTEXITCODE -ne 0) { throw 'Unable to enumerate repository source.' }
# Include technical docs and exclude academic sources and deliverables.
# Establece $OrvixSourceFiles con $OrvixCandidates | Where-Object {.
$OrvixSourceFiles = $OrvixCandidates | Where-Object {
    # Ejecuta $_ -match '^(src/native/|src/python/src/|src/python/tests/|tools/scripts/|tools/packagi…
    $_ -match '^(src/native/|src/python/src/|src/python/tests/|tools/scripts/|tools/packaging/|docs/(?!(?:[^/]+/)?academic/)|\.github/workflows/|CMakeLists\.txt$|pyproject\.toml$|LICENSE$|README(\.es)?\.md$|\.gitignore$)'
}
# Incorpora Add-Type -AssemblyName System.IO.Compression en esta operación.
Add-Type -AssemblyName System.IO.Compression
# Incorpora Add-Type -AssemblyName System.IO.Compression.FileSystem en esta operación.
Add-Type -AssemblyName System.IO.Compression.FileSystem
# Establece $OrvixArchive con [System.IO.Compression.ZipFile]::Open($OrvixTemporaryArchivePath, [System.IO.Compressio…
$OrvixArchive = [System.IO.Compression.ZipFile]::Open($OrvixTemporaryArchivePath, [System.IO.Compression.ZipArchiveMode]::Create)
# Delimita la operación protegida y su limpieza al terminar.
try {
    # Ejecuta el cuerpo para los elementos o condición indicados por foreach ($relative in $OrvixSourceFiles) {.
    foreach ($relative in $OrvixSourceFiles) {
        # Establece $full con [IO.Path]::GetFullPath((Join-Path $OrvixRoot $relative)).
        $full = [IO.Path]::GetFullPath((Join-Path $OrvixRoot $relative))
        # Comprueba if (-not $full.StartsWith($OrvixRoot + [IO.Path]::DirectorySeparatorChar, [StringCompar…
        if (-not $full.StartsWith($OrvixRoot + [IO.Path]::DirectorySeparatorChar, [StringComparison]::OrdinalIgnoreCase)) { throw 'Source escaped the repository.' }
        # Comprueba if (Test-Path -LiteralPath $full -PathType Leaf).
        if (Test-Path -LiteralPath $full -PathType Leaf) {
            # Declara el tipo y el valor de [void][System.IO.Compression.ZipFileExtensions]::CreateEntryFromFile($OrvixArchive, $fu…
            [void][System.IO.Compression.ZipFileExtensions]::CreateEntryFromFile($OrvixArchive, $full, ('orvix/' + $relative), [System.IO.Compression.CompressionLevel]::Optimal)
        }
    }
# Delimita la operación protegida y su limpieza al terminar.
} finally { $OrvixArchive.Dispose() }
# Replace only after all entries were written successfully. An existing valid
# deliverable survives an interrupted or failed archive build.
# Mueve o reemplaza el archivo indicado con Move-Item -LiteralPath $OrvixTemporaryArchivePath -Destination $OrvixAr…
Move-Item -LiteralPath $OrvixTemporaryArchivePath -Destination $OrvixArchivePath -Force
# Incorpora Get-Item -LiteralPath $OrvixArchivePath | Select-Object FullName,Length en esta operación.
Get-Item -LiteralPath $OrvixArchivePath | Select-Object FullName,Length
