@echo off
setlocal
if /i "%~1"=="clean" goto run
rem Use EMSDK, an initialized PATH, or an optional local SDK.
if not defined EMSDK (
    where em++ >nul 2>&1
    if errorlevel 1 (
        if exist "%~dp0..\.tools\emsdk\emsdk_env.bat" (
            for %%I in ("%~dp0..\.tools\emsdk") do set "EMSDK=%%~fI"
        ) else if exist "%~dp0..\..\.tools\emsdk\emsdk_env.bat" (
            for %%I in ("%~dp0..\..\.tools\emsdk") do set "EMSDK=%%~fI"
        )
    )
)
if defined EMSDK (
    if not exist "%EMSDK%\emsdk_env.bat" (
        echo ERROR: EMSDK must name an installed and activated SDK. 1>&2
        exit /b 1
    )
    set "EMSDK_QUIET=1"
    call "%EMSDK%\emsdk_env.bat"
    if errorlevel 1 exit /b 1
)
:run
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0build-client.ps1" %*
exit /b %errorlevel%
