#!/bin/bash
# KeeperFX ArkOS Launcher
# This script sets up the library path and launches the game

# Get the directory where this script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Set library path to include our bundled libraries
# Try arch-specific folder first, fall back to generic
if [ -d "${SCRIPT_DIR}/libs.aarch64" ]; then
    export LD_LIBRARY_PATH="${SCRIPT_DIR}/libs.aarch64:${LD_LIBRARY_PATH}"
elif [ -d "${SCRIPT_DIR}/libs" ]; then
    export LD_LIBRARY_PATH="${SCRIPT_DIR}/libs:${LD_LIBRARY_PATH}"
fi

# SDL2 configuration for ArkOS/embedded devices
export SDL_VIDEODRIVER=kmsdrm           # Use kernel mode setting DRM
export SDL_AUDIODRIVER=alsa              # Use ALSA for audio
export SDL_JOYSTICK_DEVICE=/dev/input/js0  # Joystick device

# Disable unnecessary features
export SDL_GAMECONTROLLERCONFIG_FILE="${SCRIPT_DIR}/gamecontrollerdb.txt"

# Debug output (comment out after testing)
# export SDL_DEBUG=1

# Change to the game directory
cd "${SCRIPT_DIR}"

# Launch the game
exec ./keeperfx.aarch64 "$@"
