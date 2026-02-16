#!/bin/bash
# KeeperFX Debug Launcher for ArkOS
# This version provides detailed debugging information

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "${SCRIPT_DIR}"

echo "=== KeeperFX Debug Launcher ==="
echo "Script directory: ${SCRIPT_DIR}"
echo ""

# Check binary exists
if [ ! -f "./keeperfx.aarch64" ]; then
    echo "ERROR: keeperfx.aarch64 not found!"
    exit 1
fi
echo "✓ Binary found: keeperfx.aarch64"

# Check libs directory
if [ -d "./libs.aarch64" ]; then
    LIBDIR="./libs.aarch64"
    echo "✓ libs.aarch64/ directory found ($(ls libs.aarch64/ | wc -l) files)"
elif [ -d "./libs" ]; then
    LIBDIR="./libs"
    echo "✓ libs/ directory found ($(ls libs/ | wc -l) files)"
else
    echo "ERROR: No libs directory found (tried libs.aarch64 and libs)!"
    exit 1
fi

# Set library path
export LD_LIBRARY_PATH="${SCRIPT_DIR}/${LIBDIR}:${LD_LIBRARY_PATH}"
echo "✓ LD_LIBRARY_PATH set to: ${SCRIPT_DIR}/${LIBDIR}"

# SDL2 configuration
export SDL_VIDEODRIVER=kmsdrm
export SDL_AUDIODRIVER=alsa
echo "✓ SDL environment configured (kmsdrm, alsa)"

echo ""
echo "=== Checking library dependencies ==="
missing=$(ldd ./keeperfx.aarch64 2>&1 | grep "not found" | wc -l)
if [ "$missing" -gt 0 ]; then
    echo "WARNING: Missing libraries detected:"
    ldd ./keeperfx.aarch64 | grep "not found"
else
    echo "✓ All libraries found"
fi

echo ""
echo "=== Launching KeeperFX ==="
echo "If it crashes, check dmesg or journalctl for kernel messages"
echo ""

# Run with core dump enabled
ulimit -c unlimited

# Launch
exec ./keeperfx.aarch64 "$@"
