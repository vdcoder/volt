<#
.SYNOPSIS
    Create a new Volt app from a template (Windows PowerShell version).

.DESCRIPTION
    Mirrors create-volt-app.sh for Windows.
    Copies the chosen template, substitutes placeholder tokens,
    copies the framework headers, applies the two known Windows fixes
    to preprocesor.py, and writes a Windows build.ps1.

.PARAMETER AppName
    Name of the new app (required).

.PARAMETER Guid
    VOLT_GUID for the app (defaults to AppName).

.PARAMETER OutputDir
    Where to create the app (defaults to <repo-root>/../<AppName>).

.PARAMETER Template
    "x" (default) or "raw".

.PARAMETER NoGit
    Skip git init.

.EXAMPLE
    .\create-volt-app.ps1 x2
    .\create-volt-app.ps1 my-app --Template x --Guid my-app-v1
#>

param(
    [Parameter(Mandatory, Position = 0)]
    [string]$AppName,

    [string]$Guid        = "",
    [string]$OutputDir   = "",
    [ValidateSet("x","raw")]
    [string]$Template    = "x",
    [switch]$NoGit
)

$ErrorActionPreference = "Stop"

# ---------------------------------------------------------------------------
#  Helpers
# ---------------------------------------------------------------------------
function Info    { param($m) Write-Host "[INFO]  $m" -ForegroundColor Cyan }
function Ok      { param($m) Write-Host "[ OK ]  $m" -ForegroundColor Green }
function Warn    { param($m) Write-Host "[WARN]  $m" -ForegroundColor Yellow }
function Err     { param($m) Write-Host "[ERR ]  $m" -ForegroundColor Red; exit 1 }

# ---------------------------------------------------------------------------
#  Resolve paths
# ---------------------------------------------------------------------------
$ScriptDir   = Split-Path -Parent $MyInvocation.MyCommand.Path
$RepoRoot    = (Resolve-Path (Join-Path $ScriptDir "..\..")).Path

if ($Guid -eq "")      { $Guid      = $AppName }
if ($OutputDir -eq "") { $OutputDir = Join-Path $RepoRoot "..\$AppName" }
$OutputDir = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($OutputDir)

$TemplateDir = switch ($Template) {
    "x"   { Join-Path $RepoRoot "app-template-x" }
    "raw" { Join-Path $RepoRoot "app-template"   }
}

# ---------------------------------------------------------------------------
#  Derive class name tokens from app name
#  e.g.  "my-cool-app"  ->  AppNameCamel="MyCoolApp"  AppNameUnderscore="my_cool_app"
# ---------------------------------------------------------------------------
function ConvertTo-CamelCase {
    param([string]$name)
    (($name -split '[-_]') | ForEach-Object {
        if ($_.Length -gt 0) { $_.Substring(0,1).ToUpper() + $_.Substring(1).ToLower() }
    }) -join ""
}

$AppNameCamel      = ConvertTo-CamelCase $AppName
$AppNameUnderscore = $AppName -replace '[-]','_'

# ---------------------------------------------------------------------------
#  Pre-flight checks
# ---------------------------------------------------------------------------
Write-Host ""
Write-Host "=================================================================" -ForegroundColor Cyan
Write-Host "  Volt - Create New App (Windows)" -ForegroundColor Cyan
Write-Host "=================================================================" -ForegroundColor Cyan
Write-Host ""

if (-not (Test-Path $TemplateDir)) { Err "Template not found: $TemplateDir" }
if (Test-Path $OutputDir)          { Err "Output directory already exists: $OutputDir`nRemove it first or choose a different name." }

Info "App name   : $AppName"
Info "GUID       : $Guid"
Info "Template   : $Template  ($TemplateDir)"
Info "Output dir : $OutputDir"
Info "Class name : $AppNameCamel"
Info "Namespace  : $AppNameUnderscore"
Write-Host ""

# ---------------------------------------------------------------------------
#  1. Copy template
# ---------------------------------------------------------------------------
Info "Copying template..."
Copy-Item -Path $TemplateDir -Destination $OutputDir -Recurse
Ok "Template copied"

# ---------------------------------------------------------------------------
#  2. Copy framework headers
# ---------------------------------------------------------------------------
Info "Copying framework headers..."
# Bash equivalent: cp -r framework/include <dest>/dependencies/volt
# That produces dependencies/volt/include/  (not a nested include/include/)
$FwIncludeSrc = Join-Path $RepoRoot "framework\include"
$FwDepsVolt   = Join-Path $OutputDir "dependencies\volt"
New-Item -ItemType Directory -Path $FwDepsVolt -Force | Out-Null
Copy-Item -Path $FwIncludeSrc -Destination $FwDepsVolt -Recurse -Force
Ok "Framework headers copied  -> dependencies\volt\include"

# ---------------------------------------------------------------------------
#  3. Copy volt.js (JS glue) to output/
# ---------------------------------------------------------------------------
Info "Copying volt.js to output/..."
$VoltJsSrc = Join-Path $RepoRoot "framework\src\volt.js"
$OutDir    = Join-Path $OutputDir "output"
New-Item -ItemType Directory -Path $OutDir -Force | Out-Null
if (Test-Path $VoltJsSrc) {
    Copy-Item -Path $VoltJsSrc -Destination $OutDir -Force
    Ok "volt.js copied"
} else {
    Warn "volt.js not found at $VoltJsSrc - skipping"
}

# ---------------------------------------------------------------------------
#  4. Substitute placeholder tokens in source files
# ---------------------------------------------------------------------------
Info "Substituting tokens in source files..."

$AppHeaderFile = if ($Template -eq "x") { "src\App.x.hpp" } else { "src\App.hpp" }
$MainCppFile   = if ($Template -eq "x") { "src\main.x.cpp" } else { "src\main.cpp" }

foreach ($rel in @($AppHeaderFile, $MainCppFile)) {
    $fullPath = Join-Path $OutputDir $rel
    if (Test-Path $fullPath) {
        $content = Get-Content $fullPath -Raw -Encoding UTF8
        $content = $content -replace 'VOLT_APP_NAME_CAMEL',      $AppNameCamel
        $content = $content -replace 'VOLT_APP_NAME_UNDERSCORE',  $AppNameUnderscore
        $content = $content -replace 'VOLT_APP_NAME',             $AppName
        [System.IO.File]::WriteAllText($fullPath, $content, [System.Text.UTF8Encoding]::new($false))
        Ok "  Tokens replaced: $rel"
    } else {
        Warn "  File not found, skipping: $rel"
    }
}

# Also substitute in components directory
$ComponentFiles = Get-ChildItem -Path (Join-Path $OutputDir "src") -Recurse -File -ErrorAction SilentlyContinue
foreach ($f in $ComponentFiles) {
    if ($f.Name -ne ($AppHeaderFile | Split-Path -Leaf) -and $f.Name -ne ($MainCppFile | Split-Path -Leaf)) {
        $content = Get-Content $f.FullName -Raw -Encoding UTF8
        $newContent = $content -replace 'VOLT_APP_NAME_CAMEL', $AppNameCamel `
                                -replace 'VOLT_APP_NAME_UNDERSCORE', $AppNameUnderscore `
                                -replace 'VOLT_APP_NAME', $AppName
        if ($newContent -ne $content) {
            [System.IO.File]::WriteAllText($f.FullName, $newContent, [System.Text.UTF8Encoding]::new($false))
            Ok "  Tokens replaced: $($f.Name)"
        }
    }
}

# Update index.html title
$indexHtml = Join-Path $OutputDir "index.html"
if (Test-Path $indexHtml) {
    $content = (Get-Content $indexHtml -Raw -Encoding UTF8) -replace 'Volt App', $AppName
    [System.IO.File]::WriteAllText($indexHtml, $content, [System.Text.UTF8Encoding]::new($false))
}

# Also patch the default GUID in build.sh (keeps parity with bash script behaviour)
$buildSh = Join-Path $OutputDir "build.sh"
if (Test-Path $buildSh) {
    $content = (Get-Content $buildSh -Raw -Encoding UTF8) -replace 'GUID="\$\{VOLT_GUID:-demo\}"', ("GUID=`"`${VOLT_GUID:-" + $Guid + "}`"")
    [System.IO.File]::WriteAllText($buildSh, $content, [System.Text.UTF8Encoding]::new($false))
}

Ok "Token substitution done"

# ---------------------------------------------------------------------------
#  5. Patch preprocesor.py (X template only) - copy the Windows-fixed version
# ---------------------------------------------------------------------------
if ($Template -eq "x") {
    Info "Patching preprocesor.py for Windows..."
    $preprocSrc  = Join-Path $ScriptDir "..\..\app-template-x\preprocesor.py"
    $preprocDest = Join-Path $OutputDir "preprocesor.py"
    if (Test-Path $preprocSrc) {
        # The repo's preprocesor.py already has our Windows fixes applied.
        # Copy it directly - no fragile text-patching needed.
        Copy-Item $preprocSrc $preprocDest -Force
        Ok "  preprocesor.py copied (Windows-fixed version from repo)"
    } else {
        Warn "  preprocesor.py not found at $preprocSrc - skipping"
    }
}

# ---------------------------------------------------------------------------
#  6. Write Windows build.ps1 (from build-template.ps1 in this directory)
# ---------------------------------------------------------------------------
Info "Writing build.ps1..."
$templateFile = Join-Path $ScriptDir "build-template.ps1"
if (-not (Test-Path $templateFile)) {
    Err "build-template.ps1 not found at $templateFile"
}
$buildContent = Get-Content $templateFile -Raw -Encoding UTF8
$buildContent = $buildContent -replace 'APP_GUID_PLACEHOLDER', $Guid
$buildContent = $buildContent -replace 'APP_NAME_PLACEHOLDER', $AppName
$buildPath = Join-Path $OutputDir "build.ps1"
[System.IO.File]::WriteAllText($buildPath, $buildContent, [System.Text.UTF8Encoding]::new($false))
Ok "build.ps1 written"
# ---------------------------------------------------------------------------
#  7. Optional git init
# ---------------------------------------------------------------------------
if (-not $NoGit) {
    Info "Initialising git repository..."
    Push-Location $OutputDir
    git init -q
    git add .
    git commit -q -m "Initial commit - Created with Volt Framework (Windows)"
    Pop-Location
    Ok "Git repo initialised"
}

# ---------------------------------------------------------------------------
#  Done
# ---------------------------------------------------------------------------
Write-Host ""
Write-Host "=================================================================" -ForegroundColor Green
Write-Host "  App created successfully!" -ForegroundColor Green
Write-Host "=================================================================" -ForegroundColor Green
Write-Host ""
Ok "Location : $OutputDir"
Write-Host ""
Write-Host "  Next steps:" -ForegroundColor White
Write-Host "    cd `"$OutputDir`""
Write-Host "    . <emsdk_dir>\emsdk_env.ps1   # activate Emscripten"
Write-Host "    .\build.ps1                   # build"
Write-Host "    cd output"
Write-Host "    python -m http.server 8001    # serve"
Write-Host "    # open http://localhost:8001"
Write-Host ""
