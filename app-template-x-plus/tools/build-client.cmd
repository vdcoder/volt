@echo off
setlocal EnableExtensions DisableDelayedExpansion
if /i "%~1"=="clean" goto run

rem Find the Emscripten SDK. First match wins:
rem   1. emsdk-path.txt in the app folder (written by create-volt-app -EmsdkPath / --emsdk-path)
rem   2. the EMSDK environment variable
rem   3. em++ already on PATH
rem   4. .tools\emsdk in the app or its parent, or an emsdk folder beside the app
rem Visual Studio only sees the environment it was started with, so (1) is the
rem most reliable choice for IDE builds.
for %%I in ("%~dp0..") do set "VOLT_APP_ROOT=%%~fI"
set "VOLT_EMSDK_FILE=%VOLT_APP_ROOT%\emsdk-path.txt"
set "VOLT_EMSDK_DIR="

if not exist "%VOLT_EMSDK_FILE%" goto try_env
rem First non-blank line; relative paths are resolved from the app folder.
pushd "%VOLT_APP_ROOT%"
for /f "usebackq delims=" %%P in ("%VOLT_EMSDK_FILE%") do if not defined VOLT_EMSDK_DIR set "VOLT_EMSDK_DIR=%%~fP"
popd
if not defined VOLT_EMSDK_DIR goto bad_file
if not exist "%VOLT_EMSDK_DIR%\emsdk_env.bat" goto bad_file
set "VOLT_EMSDK_FROM=emsdk-path.txt"
goto activate

:try_env
if not defined EMSDK goto try_path
if not exist "%EMSDK%\emsdk_env.bat" goto bad_env
set "VOLT_EMSDK_DIR=%EMSDK%"
set "VOLT_EMSDK_FROM=EMSDK"
goto activate

:try_path
where em++ >nul 2>&1
if not errorlevel 1 goto run
call :probe "%VOLT_APP_ROOT%\.tools\emsdk"
call :probe "%VOLT_APP_ROOT%\..\.tools\emsdk"
call :probe "%VOLT_APP_ROOT%\..\emsdk"
if not defined VOLT_EMSDK_DIR goto not_found
set "VOLT_EMSDK_FROM=local folder"

:activate
echo Emscripten SDK: "%VOLT_EMSDK_DIR%" [%VOLT_EMSDK_FROM%]
set "EMSDK=%VOLT_EMSDK_DIR%"
set "EMSDK_QUIET=1"
call "%EMSDK%\emsdk_env.bat"
if errorlevel 1 goto activate_failed
where em++ >nul 2>&1
if errorlevel 1 goto not_active

:run
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0build-client.ps1" %*
exit /b %errorlevel%

:bad_file
if defined VOLT_EMSDK_DIR echo build-client.cmd : error : emsdk-path.txt names "%VOLT_EMSDK_DIR%", which has no emsdk_env.bat. 1>&2
if not defined VOLT_EMSDK_DIR echo build-client.cmd : error : emsdk-path.txt is empty. 1>&2
echo   File: "%VOLT_EMSDK_FILE%" 1>&2
echo   Put the emsdk folder, the one containing emsdk_env.bat, on its first line, saved as plain text without a byte order mark. 1>&2
echo   Or delete the file to use EMSDK, PATH or a local SDK folder instead. 1>&2
exit /b 1

:bad_env
echo build-client.cmd : error : EMSDK is "%EMSDK%", which has no emsdk_env.bat. 1>&2
echo   Point EMSDK at an installed SDK, or save the SDK folder in "%VOLT_EMSDK_FILE%". 1>&2
exit /b 1

:activate_failed
echo build-client.cmd : error : "%VOLT_EMSDK_DIR%\emsdk_env.bat" failed. 1>&2
exit /b 1

:not_active
echo build-client.cmd : error : The Emscripten SDK in "%VOLT_EMSDK_DIR%" has no active version, so em++ is still not on PATH. 1>&2
echo   Run "%VOLT_EMSDK_DIR%\emsdk.bat" activate VERSION once, then build again. 1>&2
exit /b 1

:not_found
echo build-client.cmd : error : Emscripten SDK not found. Visual Studio only sees the environment it was started with. 1>&2
echo   Checked, in order: 1>&2
echo     1. "%VOLT_EMSDK_FILE%" - not present 1>&2
echo     2. EMSDK environment variable - not set 1>&2
echo     3. em++ on PATH - not found 1>&2
echo     4. .tools\emsdk in the app or its parent, and an emsdk folder beside the app - not found 1>&2
echo   Fix one of: 1>&2
echo     - Create emsdk-path.txt in the app folder with the SDK folder on its first line, for example C:\emsdk 1>&2
echo       (create-volt-app -EmsdkPath or --emsdk-path writes this file for you). 1>&2
echo     - Set EMSDK, or start Visual Studio from a terminal where emsdk_env ran, then restart Visual Studio. 1>&2
exit /b 1

:probe
if defined VOLT_EMSDK_DIR exit /b 0
if exist "%~1\emsdk_env.bat" set "VOLT_EMSDK_DIR=%~f1"
exit /b 0
