#!/bin/bash
# Below we assign the source of the control folder (which is the PortMaster folder) based on the distro:

XDG_DATA_HOME=${XDG_DATA_HOME:-$HOME/.local/share}

if [ -d "/opt/system/Tools/PortMaster/" ]; then
  controlfolder="/opt/system/Tools/PortMaster"
elif [ -d "/opt/tools/PortMaster/" ]; then
  controlfolder="/opt/tools/PortMaster"
elif [ -d "$XDG_DATA_HOME/PortMaster/" ]; then
  controlfolder="$XDG_DATA_HOME/PortMaster"
else
  controlfolder="/roms/ports/PortMaster"
fi

source $controlfolder/control.txt # We source the control.txt file contents here
# The $ESUDO, $directory, $param_device and necessary sdl configuration controller configurations will be sourced from the control.txt file shown [here]

# We source custom mod files from the portmaster folder example mod_jelos.txt which containts pipewire fixes
[ -f "${controlfolder}/mod_${CFW_NAME}.txt" ] && source "${controlfolder}/mod_${CFW_NAME}.txt"

# We pull the controller configs like the correct SDL2 Gamecontrollerdb GUID from the get_controls function from the control.txt file here
get_controls

# We switch to the port's directory location below
GAMEDIR="/$directory/ports/keeperfx"

# Check if original Dungeon Keeper data files exist
if [ ! -d "$GAMEDIR/keeperfx/data" ] || [ ! -f "$GAMEDIR/keeperfx/data/bluepal.dat" ]; then
    echo "ERROR: Original Dungeon Keeper data files not found!"
    echo "Please place your original game files in: $GAMEDIR/keeperfx/data/"
    echo "You need the 'data' folder from the original Dungeon Keeper game."
    sleep 5
    exit 1
fi

# Switch to the game directory
cd "$GAMEDIR/keeperfx" || { echo "ERROR: Cannot cd to $GAMEDIR/keeperfx"; exit 1; }

# Log the execution of the script, the script overwrites itself on each launch
exec 1> >(tee "$GAMEDIR/log.txt") 2>&1

echo "Script started at $(date)"

# KeeperFX keeps saves in its own directory, so no special folder mapping needed
# Config files are in config/ and saves in save/ subdirectories of GAMEDIR 

# Port specific additional libraries should be included within the port's directory in a separate subfolder named libs.aarch64, libs.armhf or libs.x64
LIBDIR="$GAMEDIR/keeperfx/libs.${DEVICE_ARCH}"

echo "=== DEBUG INFO ==="
echo "GAMEDIR: $GAMEDIR"
echo "DEVICE_ARCH: $DEVICE_ARCH"
echo "LIBDIR: $LIBDIR"
echo "Current directory: $(pwd)"
echo "Directory contents:"
ls -la "$LIBDIR" | head -20
echo ""

# Ensure all required libraries are in the current directory (same as executable)
# This works around ESUDO potentially dropping LD_LIBRARY_PATH
# Skip SDL2 libraries - use system versions for better compatibility
echo "Copying libraries to current directory..."
for lib in $(ls "$LIBDIR"/*.so* 2>/dev/null); do
    filename=$(basename $lib)
    # Skip SDL2 libraries - use system versions
    if [[ "$filename" =~ SDL2 ]]; then
        echo "Skipping SDL2: $filename (using system version)"
        continue
    fi
    if [ ! -e "./$filename" ]; then
        echo "Copying: $filename"
        cp "$lib" ./
    else
        echo "Already exists: $filename"
    fi
done

# Create symlinks for common library version mismatches
# These handle cases where the binary expects a different version than what we have
ln -sf libavcodec.so.58.134.100 ./libavcodec.so.58 2>/dev/null
ln -sf libavformat.so.58.76.100 ./libavformat.so.58 2>/dev/null
ln -sf libavutil.so.56.70.100 ./libavutil.so.56 2>/dev/null
ln -sf libswresample.so.3.9.100 ./libswresample.so.3 2>/dev/null

# Handle potential version mismatches for different ABI versions
ln -sf libavcodec.so.58 ./libavcodec.so.59 2>/dev/null
ln -sf libavcodec.so.58 ./libavcodec.so.60 2>/dev/null
ln -sf libavcodec.so.58 ./libavcodec.so.61 2>/dev/null
ln -sf libavformat.so.58 ./libavformat.so.59 2>/dev/null
ln -sf libavformat.so.58 ./libavformat.so.60 2>/dev/null
ln -sf libavformat.so.58 ./libavformat.so.61 2>/dev/null

echo "Libraries copied. Current directory now contains:"
ls -la *.so* 2>/dev/null | head -20
echo ""

export LD_LIBRARY_PATH=".:$LIBDIR:$LD_LIBRARY_PATH"
echo "LD_LIBRARY_PATH set to: $LD_LIBRARY_PATH"
echo ""
# Detect or map architecture to binary name
BINARY_ARCH="${DEVICE_ARCH}"
case "${DEVICE_ARCH}" in
  aarch64|arm64|armv8)
    BINARY_ARCH="aarch64"
    ;;
  armhf|armv7|arm)
    BINARY_ARCH="armhf"
    ;;
  x86_64|x64)
    BINARY_ARCH="x64"
    ;;
esac

echo "Detected binary architecture: $BINARY_ARCH"

# Check which binary exists
if [ -f "./keeperfx.${BINARY_ARCH}" ]; then
    BINARY="./keeperfx.${BINARY_ARCH}"
    echo "Found: $BINARY"
elif [ -f "./keeperfx-arm64" ]; then
    BINARY="./keeperfx-arm64"
    echo "Found: $BINARY (migrating from docker build)"
elif [ -f "./keeperfx" ]; then
    BINARY="./keeperfx"
    echo "Found: $BINARY"
else
    echo "ERROR: No keeperfx binary found!"
    echo "Searched for:"
    echo "  ./keeperfx.${BINARY_ARCH}"
    echo "  ./keeperfx-arm64"
    echo "  ./keeperfx"
    echo "Available files:"
    ls -la | grep -E 'keeperfx|^-rw|^-rwx' | head -20
    sleep 5
    exit 1
fi

echo "=== END DEBUG INFO ==="
echo ""

# Test if binary is executable
if [ ! -x "$BINARY" ]; then
    echo "Making $BINARY executable..."
    chmod +x "$BINARY"
fi

# Test: try to run binary with --version or --help to check if it works
echo "Testing binary with --help..."
"$BINARY" --help > /tmp/keeperfx_test.txt 2>&1
TEST_EXIT=$?
echo "Binary test exit code: $TEST_EXIT"
if [ $TEST_EXIT -ne 0 ]; then
    head -20 /tmp/keeperfx_test.txt
fi
echo ""

# Provide appropriate controller configuration - KeeperFX has native SDL input support
export SDL_GAMECONTROLLERCONFIG="$sdl_controllerconfig"

echo "Launching: $ESUDO $BINARY"
echo "LD_LIBRARY_PATH=$LD_LIBRARY_PATH"
echo "SDL_GAMECONTROLLERCONFIG=$SDL_GAMECONTROLLERCONFIG"
echo ""

# Now we launch KeeperFX
$ESUDO "$BINARY"