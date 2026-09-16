param([Parameter(Mandatory)][string]$AppRoot)
$ErrorActionPreference = 'Stop'
$app = (Resolve-Path -LiteralPath $AppRoot).Path
$build = Join-Path $app 'tools\build-client.cmd'
$web = Join-Path $app 'output\Debug\web'
$releaseJs = Join-Path $app 'output\Release\web\volt.js'
$releaseHash = (Get-FileHash -LiteralPath $releaseJs).Hash

& $build clean Debug
if ($LASTEXITCODE -ne 0 -or (Test-Path -LiteralPath $web)) { throw 'Debug Clean failed' }
if ((Get-FileHash -LiteralPath $releaseJs).Hash -ne $releaseHash) { throw 'Debug Clean changed Release' }

# Insert an error only into this generated test app, and always restore it.
$source = Join-Path $app 'client\src\main.x.cpp'
$original = [IO.File]::ReadAllText($source)
try {
    [IO.File]::WriteAllText($source, "#error XPLUS_SOURCE_MAP_PROBE`n" + $original, [Text.UTF8Encoding]::new($false))
    $diagnostics = & $build build Debug 2>&1 | Out-String
    if ($LASTEXITCODE -eq 0) { throw 'Changed DSL source was not rebuilt' }
    if ($diagnostics -notmatch 'client[/\\]src[/\\]main\.x\.cpp\(1,\d+\).*XPLUS_SOURCE_MAP_PROBE') {
        throw "Compiler diagnostic did not map to original source line 1: $diagnostics"
    }
} finally {
    [IO.File]::WriteAllText($source, $original, [Text.UTF8Encoding]::new($false))
}
& $build rebuild Debug
if ($LASTEXITCODE -ne 0) { throw 'Debug Rebuild failed' }
foreach ($file in 'index.html', 'global.css', 'volt.js', 'app.js', 'app.wasm') {
    if (-not (Test-Path -LiteralPath (Join-Path $web $file))) { throw "Rebuild did not restore $file" }
}
if ((Get-FileHash -LiteralPath (Join-Path $web 'volt.js')).Hash -ne
    (Get-FileHash -LiteralPath (Join-Path $app 'dependencies\volt\src\volt.js')).Hash) {
    throw 'volt.js was not copied from the dependency source'
}
Write-Host 'PASS: config-isolated Clean, source-change detection, original-file #line diagnostics, and complete Rebuild.'
