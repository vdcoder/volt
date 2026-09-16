param([Parameter(Mandatory)][string]$AppRoot)
$ErrorActionPreference = 'Stop'
$app = (Resolve-Path -LiteralPath $AppRoot).Path
$clientDir = Join-Path $app 'intermediate\Debug\client'
New-Item -ItemType Directory -Force -Path $clientDir | Out-Null
$logPath = Join-Path $clientDir ('locked-build-' + [guid]::NewGuid().ToString('N') + '.log')
# Model the IDE keeping a file in IntDir open throughout every build action.
$lockedLog = [IO.File]::Open($logPath, [IO.FileMode]::CreateNew, [IO.FileAccess]::ReadWrite, [IO.FileShare]::None)
try {
    foreach ($action in 'build', 'rebuild', 'clean', 'build') {
        & (Join-Path $app 'tools\build-client.cmd') $action Debug
        if ($LASTEXITCODE -ne 0) { throw "$action failed while an IntDir log was locked" }
        if (-not (Test-Path -LiteralPath $logPath)) { throw "$action deleted the IDE log" }
    }
    if (-not (Test-Path -LiteralPath (Join-Path $clientDir 'generated\src\main.x.cpp'))) {
        throw 'Generated source was not written into the dedicated subtree'
    }
    Write-Host 'PASS: Build, Rebuild, Clean and subsequent Build preserve an exclusively locked IntDir log.'
} finally {
    $lockedLog.Dispose()
    Remove-Item -LiteralPath $logPath
}
