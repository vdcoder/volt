param(
    [ValidateSet('build', 'rebuild', 'clean')][string]$Action = 'build',
    [ValidateSet('Debug', 'Release')][string]$Configuration = 'Debug'
)
$ErrorActionPreference = 'Stop'
$appRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
# Visual Studio owns the parent IntDir and may hold Client.log open during Build.
# Only this generated subtree belongs to the client script and may be removed.
$work = Join-Path $appRoot "intermediate\$Configuration\client\generated"
$web = Join-Path $appRoot "output\$Configuration\web"

function Remove-BuildDirectory([string]$path) {
    $resolved = [IO.Path]::GetFullPath($path)
    if ($resolved -ne $work -and $resolved -ne $web) { throw "Unexpected clean path: $resolved" }
    if (Test-Path -LiteralPath $resolved) { Remove-Item -LiteralPath $resolved -Recurse -Force }
}

try {
    if ($Action -eq 'clean' -or $Action -eq 'rebuild') {
        Remove-BuildDirectory $work
        Remove-BuildDirectory $web
        if ($Action -eq 'clean') { exit 0 }
    }
    $compiler = Get-Command em++ -CommandType Application -ErrorAction SilentlyContinue | Select-Object -First 1
    if (-not $compiler) { throw 'em++ not found. Set EMSDK, initialize PATH, or install .tools/emsdk in the app or its parent. Invoke tools\build-client.cmd to initialize the SDK.' }
    $python = if ($env:EMSDK_PYTHON) { $env:EMSDK_PYTHON } else { 'python' }
    $settings = Get-Content -LiteralPath (Join-Path $appRoot 'app.json') -Raw | ConvertFrom-Json
    if ($settings.guid -notmatch '^[A-Za-z0-9_-]+$') { throw 'app.json guid must use letters, digits, _ or -' }
    # Regenerate the whole client tree on every requested build, including header changes.
    Remove-BuildDirectory $work
    New-Item -ItemType Directory -Force -Path $work, $web | Out-Null
    $sourceRoot = Join-Path $appRoot 'client\src'
    foreach ($source in Get-ChildItem -LiteralPath $sourceRoot -File -Recurse) {
        $relative = $source.FullName.Substring($sourceRoot.Length + 1)
        $destination = Join-Path $work "src\$relative"
        New-Item -ItemType Directory -Force -Path (Split-Path $destination) | Out-Null
        if ($source.Name -match '\.x\.') {
            & $python (Join-Path $PSScriptRoot 'preprocesor.py') ($source.FullName.Replace('\', '/')) $destination
            if ($LASTEXITCODE -ne 0) { throw "Preprocessing failed: $relative" }
        } else { Copy-Item -LiteralPath $source.FullName -Destination $destination -Force }
    }
    # Clang response-file quoting avoids losing the string quotes through cmd/PowerShell.
    $response = Join-Path $work 'flags.rsp'
    [IO.File]::WriteAllText($response, ('-DVOLT_GUID=\"' + $settings.guid + '\"'), [Text.Encoding]::ASCII)
    $compilerArgs = @(
        (Join-Path $work 'src\main.x.cpp'), "@$response", '-std=c++20',
        '-I', (Join-Path $appRoot 'dependencies\volt\include'),
        '-o', (Join-Path $web 'app.js'), '-lembind', '--bind',
        '-sWASM=1', '-m64', '-sALLOW_MEMORY_GROWTH=1',
        '-sMODULARIZE=1', '-sEXPORT_NAME=VoltApp', '-sEXPORTED_RUNTIME_METHODS=ccall,cwrap',
        '-fdiagnostics-format=msvc'
    )
    if ($Configuration -eq 'Debug') { $compilerArgs += @('-O0', '-g', '-DDEBUG', '-DVOLT_ENABLE_LOG', '-sASSERTIONS=1') }
    else { $compilerArgs += @('-O2', '-DNDEBUG') }
    Write-Host "Building Volt X+ $Configuration (MEMORY64)"
    & $compiler.Source @compilerArgs
    if ($LASTEXITCODE -ne 0) { throw "em++ failed with exit code $LASTEXITCODE" }
    Copy-Item -Path (Join-Path $appRoot 'client\public\*') -Destination $web -Recurse -Force
    Copy-Item -LiteralPath (Join-Path $appRoot 'dependencies\volt\src\volt.js') -Destination $web -Force
    Write-Host "Browser output: $web"
} catch {
    Write-Error $_ -ErrorAction Continue
    exit 1
}
