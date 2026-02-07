@echo off
REM Build KeeperFX for ARM64 (PortMaster ArkOS) using Docker
REM Requirements: Docker Desktop with BuildKit support

setlocal enabledelayedexpansion

echo ========================================
echo KeeperFX ARM64 Build Script
echo ========================================
echo.

REM Build the Docker image
echo [1/3] Building Docker image...
docker buildx build --platform linux/arm64 -f dockerfile.eoan -t keeperfx-arm64-eoan:latest .
if errorlevel 1 (
    echo ERROR: Docker image build failed!
    exit /b 1
)
echo Docker image built successfully.
echo.

REM Compile inside the container
echo [2/3] Compiling ARM64 binary...
docker run --rm --platform linux/arm64 --entrypoint bash keeperfx-arm64-eoan:latest -c "cd /build && make -f linux.mk clean && make -f linux.mk -j8"
if errorlevel 1 (
    echo ERROR: Compilation failed!
    exit /b 1
)
echo Compilation completed successfully.
echo.

REM Extract the binary
echo [3/3] Extracting binary...
docker run --rm --platform linux/arm64 --entrypoint bash keeperfx-arm64-eoan:latest -c "cat /build/bin/keeperfx" > keeperfx.aarch64
if errorlevel 1 (
    echo ERROR: Failed to extract binary!
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

endlocal
