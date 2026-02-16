@echo off
REM Build KeeperFX for PortMaster ArkOS using official builder
REM See: https://portmaster.games/docker.html

setlocal enabledelayedexpansion

echo ========================================
echo KeeperFX PortMaster Build Script
echo Using official PortMaster builder
echo ========================================
echo.

REM Download required library sources if not present
echo [0/4] Checking dependencies...
if not exist "deps\centijson\src\json.c" (
    echo Downloading centijson...
    if exist "deps\centijson-master" rmdir /s /q "deps\centijson-master"
    powershell -Command "Invoke-WebRequest -Uri 'https://github.com/mity/centijson/archive/refs/heads/master.tar.gz' -OutFile deps\centijson.tar.gz -ErrorAction Stop"
    if errorlevel 1 (
        echo ERROR: Failed to download centijson!
        exit /b 1
    )
    cd deps
    tar -xzf centijson.tar.gz
    if exist "centijson" rmdir /s /q "centijson"
    move centijson-master centijson
    del centijson.tar.gz
    cd ..
)

if not exist "deps\astronomy\source\c\astronomy.c" (
    echo Downloading astronomy...
    if exist "deps\astronomy-2.1.19" rmdir /s /q "deps\astronomy-2.1.19"
    powershell -Command "Invoke-WebRequest -Uri 'https://github.com/cosinekitty/astronomy/archive/refs/tags/v2.1.19.tar.gz' -OutFile deps\astronomy.tar.gz -ErrorAction Stop"
    if errorlevel 1 (
        echo ERROR: Failed to download astronomy!
        exit /b 1
    )
    cd deps
    tar -xzf astronomy.tar.gz
    if exist "astronomy" rmdir /s /q "astronomy"
    move astronomy-2.1.19 astronomy
    del astronomy.tar.gz
    cd ..
)

if not exist "deps\enet6\src\include\enet6\enet.h" (
    echo Downloading enet6...
    if exist "deps\enet6\src" rmdir /s /q "deps\enet6\src"
    cd deps\enet6
    git clone https://github.com/SirLynix/enet6.git src
    if errorlevel 1 (
        echo ERROR: Failed to clone enet6!
        cd ..\..
        exit /b 1
    )
    cd ..\..
)
echo Dependencies ready.
echo.


REM Build using WSL2 native cross-compiler (much faster and more reliable than Docker emulation)
echo.
echo [1/4] Using WSL2 cross-compilation...

REM Build directly using WSL2 Linux with cross-compiler
echo [2/4] Compiling ARM64 binary in WSL2...
wsl bash -c "chmod +x /mnt/a/CodeProjects/KeeperFx/keeperfx/scripts/portmaster_build.sh && /mnt/a/CodeProjects/KeeperFx/keeperfx/scripts/portmaster_build.sh"
if errorlevel 1 (
    echo ERROR: Compilation failed!
    exit /b 1
)
echo Compilation completed successfully.
echo.

REM No Docker extraction needed - binary copied back to workspace by WSL2 build
echo [3/4] Verifying binary...
if not exist "keeperfx.aarch64" (
    echo ERROR: Binary not created!
    exit /b 1
)

echo.
echo ========================================
echo Build Complete!
echo ========================================
echo Binary saved to: keeperfx.aarch64
echo File size:
for %%A in (keeperfx.aarch64) do echo   %%~zA bytes
echo.
echo You can now deploy this binary to your PortMaster ArkOS device.
echo ========================================
