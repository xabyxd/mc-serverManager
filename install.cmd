@echo off
setlocal enabledelayedexpansion

REM ==========================================================
REM ServerManager Installer & Updater (Safe Version)
REM ----------------------------------------------------------
REM Version: 1.0.3 (Safe PATH handling)
REM Author: xabyxd (https://github.com/xabyxd)
REM License: Apache License 2.0
REM ==========================================================

echo ==========================================================
echo                 ServerManager Installer
echo ----------------------------------------------------------
echo This script will:
echo   - Check for the latest version online.
echo   - Compare it with your installed version (if any).
echo   - Download and install the latest version if needed.
echo.
echo NOTE: Only your user environment is modified, not system-wide settings.
echo ==========================================================
echo.

:prompt_user
set "CONFIRM="
set /p CONTINUE=Do you want to continue? (y, yes/n, no): 
set "ANSWER=%CONTINUE%"
set "ANSWER=!ANSWER:"=!"
set "ANSWER=!ANSWER: =!"
set "ANSWER=!ANSWER:.=!"
set "ANSWER=!ANSWER:,=!"
set "ANSWER=!ANSWER:;=!"

if /I "!ANSWER!"=="Y" set "CONFIRM=YES"
if /I "!ANSWER!"=="YES" set "CONFIRM=YES"
if /I "!ANSWER!"=="N" set "CONFIRM=NO"
if /I "!ANSWER!"=="NO" set "CONFIRM=NO"

if not defined CONFIRM (
    echo [ERROR] Invalid input. Please type 'y' 'yes' or 'n' 'no'.
    goto :prompt_user
)

if /I "!CONFIRM!"=="NO" (
    echo Installation cancelled.
    exit /b
)

REM CONFIGURATION
set "INSTALL_DIR=C:\ServerManager"
set "FILE_NAME=serverManager.exe"
set "VERSION_FILE=%INSTALL_DIR%\version.txt"
set "VERSION_URL=https://pastebin.com/raw/Ps1yZpZc"
set "EXE_BASE_URL=https://github.com/xabyxd/mc-serverManager/releases/download"

REM Get latest version from online source
for /f "delims=" %%i in ('curl -s "%VERSION_URL%"') do set "LATEST_VERSION=%%i"
if not defined LATEST_VERSION (
    echo [ERROR] Failed to fetch the latest version.
    exit /b 1
)
echo [INFO] Latest available version: %LATEST_VERSION%

REM Check installed version
if exist "%VERSION_FILE%" (
    for /f "usebackq tokens=* delims=" %%v in ("%VERSION_FILE%") do set "INSTALLED_VERSION=%%v"
    if not defined INSTALLED_VERSION (
        echo [WARNING] Installed version file exists but is empty or invalid.
        set "INSTALLED_VERSION=none"
    )
    echo [INFO] Installed version: !INSTALLED_VERSION!
) else (
    set "INSTALLED_VERSION=none"
    echo [INFO] No installed version found.
)

REM Normalize versions to remove trailing whitespace and line endings
for /f %%A in ('echo %LATEST_VERSION%') do set "LATEST_VERSION_CLEAN=%%A"
for /f %%A in ('echo !INSTALLED_VERSION!') do set "INSTALLED_VERSION_CLEAN=%%A"

REM Debug
REM echo [DEBUG] Cleaned installed version: [!INSTALLED_VERSION_CLEAN!]
REM echo [DEBUG] Cleaned latest version:    [%LATEST_VERSION_CLEAN%]

REM Compare versions
if /I "!INSTALLED_VERSION_CLEAN!"=="%LATEST_VERSION_CLEAN%" (
    echo [OK] You already have the latest version installed.
    goto :EOF
)

REM Build download URL
set "URL=%EXE_BASE_URL%/v%LATEST_VERSION_CLEAN%/%FILE_NAME%"

echo [INFO] New version available. Starting update...

REM Download the new executable
curl -L "%URL%" -o "%TEMP%\%FILE_NAME%"
if errorlevel 1 (
    echo [ERROR] Failed to download the file.
    exit /b 1
)

REM Create install directory if it doesn't exist
mkdir "%INSTALL_DIR%" >nul 2>&1

REM Move the file to the install directory
echo [INFO] Installing new version to: %INSTALL_DIR%
move /Y "%TEMP%\%FILE_NAME%" "%INSTALL_DIR%" >nul

REM Save version to version.txt (without BOM, without newline issues)
<nul set /p=".%LATEST_VERSION_CLEAN%" > "%VERSION_FILE%"
echo.>> "%VERSION_FILE%"

REM Read current user PATH from registry
set "USER_PATH="
for /f "tokens=3*" %%A in ('reg query HKCU\Environment /v Path 2^>nul') do set "USER_PATH=%%A %%B"
set "USER_PATH=!USER_PATH:~0,1023!"

REM Check if install dir is already in PATH
echo !USER_PATH! | find /I "%INSTALL_DIR%" >nul
if !errorlevel! EQU 0 (
    echo [INFO] Installation folder already in user PATH.
) else (
    echo [INFO] Adding installation folder to user PATH...
    set "NEW_USER_PATH=!USER_PATH!;%INSTALL_DIR%"
    echo !NEW_USER_PATH! | findstr /R "^.\{1024,\}" >nul
    if !errorlevel! EQU 0 (
        echo [ERROR] PATH would exceed 1024 characters.
        echo [WARNING] Cannot safely modify user PATH.
        echo [SOLUTION] Please manually add "%INSTALL_DIR%" to your user PATH in System Settings.
    ) else (
        setx Path "!NEW_USER_PATH!" >nul
        echo [SUCCESS] Added to user PATH. Please open a NEW terminal to use it.
    )
)

echo [SUCCESS] Version %LATEST_VERSION_CLEAN% installed successfully.
echo You can now run "%FILE_NAME%" from any terminal window.

goto :pause_exit

:pause_exit
echo.
echo Script finished. Press any key to exit...
pause >nul
exit /b