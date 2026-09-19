$ErrorActionPreference = 'Stop'
$version = '1.0.3351.48'
$expectedHash = 'AD98E9D951361E62B6EDC137273EC1787242C5C28539C1A101F98A1E07D060F9'
$cache = Join-Path $PSScriptRoot '../.tools/webview2'
$destination = Join-Path $cache $version
if (Test-Path (Join-Path $destination '.complete')) { exit 0 }
New-Item -ItemType Directory -Force -Path $cache | Out-Null
$archive = Join-Path $cache "$version.zip"
function Read-Hash($path) {
    $hash = [Security.Cryptography.SHA256]::Create()
    $stream = [IO.File]::OpenRead($path)
    try { return [BitConverter]::ToString($hash.ComputeHash($stream)).Replace('-', '') }
    finally { $stream.Dispose(); $hash.Dispose() }
}
try {
    if (!(Test-Path $archive) -or (Read-Hash $archive) -ne $expectedHash) {
        Write-Host "Restoring Microsoft WebView2 SDK $version..."
        [Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12
        Invoke-WebRequest -UseBasicParsing -Uri "https://api.nuget.org/v3-flatcontainer/microsoft.web.webview2/$version/microsoft.web.webview2.$version.nupkg" -OutFile $archive
    }
    if ((Read-Hash $archive) -ne $expectedHash) {
        throw 'WebView2 SDK checksum mismatch.'
    }
    Add-Type -AssemblyName System.IO.Compression.FileSystem
    $zip = [IO.Compression.ZipFile]::OpenRead($archive)
    try {
        $root = [IO.Path]::GetFullPath($destination) + [IO.Path]::DirectorySeparatorChar
        foreach ($entry in $zip.Entries) {
            $target = [IO.Path]::GetFullPath((Join-Path $root $entry.FullName))
            if (!$target.StartsWith($root, [StringComparison]::OrdinalIgnoreCase)) { throw 'Invalid SDK archive path.' }
            if (!$entry.Name) { [IO.Directory]::CreateDirectory($target) | Out-Null; continue }
            [IO.Directory]::CreateDirectory([IO.Path]::GetDirectoryName($target)) | Out-Null
            [IO.Compression.ZipFileExtensions]::ExtractToFile($entry, $target, $true)
        }
    } finally { $zip.Dispose() }
    Set-Content -LiteralPath (Join-Path $destination '.complete') -Value $expectedHash
} catch {
    Write-Error "Cannot restore WebView2 SDK: $_"
    exit 1
}
