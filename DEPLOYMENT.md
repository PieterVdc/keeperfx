# KeeperFX ArkOS ARM64 Deployment Guide

## Build Summary
- **Build Date**: February 7, 2025
- **Platform**: ARM64 (aarch64) for ArkOS/PortMaster
- **Binary Size**: 29MB
- **Total Package Size**: ~80MB (binary + libraries)

## Files for Deployment

### 1. Binary
- `keeperfx.aarch64` (29MB) - Main game executable

### 2. Launcher Script
- `keeperfx_launcher.sh` - Script that sets up library paths and launches the game

### 3. Shared Libraries (libs/ folder - 50MB total)
All dependencies built specifically for ArkOS compatibility:
- **SDL2 Suite**: libSDL2-2.0.so.0.22.0, libSDL2_mixer-2.0.so.0.2.2, libSDL2_net-2.0.so.0.0.1
- **LuaJIT**: libluajit-5.1.so.2.1.0
- **FFmpeg**: libavcodec.so.58, libavformat.so.58, libavutil.so.56, libswresample.so.3
- **Other**: libspng.so, libminiupnpc.so.17, libnatpmp.so.1

### 4. Game Data
You'll also need to copy the following folders from your KeeperFX installation:
- `campgns/` - Campaign files
- `config/` - Configuration files
- `gfx/` - Graphics data
- `lang/` - Language files
- `levels/` - Level files
- `sdl/` - SDL configuration

## Deployment Steps

### Method 1: Using the Launcher Script (Recommended)

1. Create a directory on your ArkOS device (e.g., `/roms/ports/keeperfx/`)

2. Copy these items to the directory:
   ```
   keeperfx.aarch64
   keeperfx_launcher.sh
   libs/ (entire folder)
   campgns/
   config/
   gfx/
   lang/
   levels/
   sdl/
   ```

3. Run the game:
   ```bash
   ./keeperfx_launcher.sh
   ```

### Method 2: Manual Launch

If you prefer to run directly:
```bash
LD_LIBRARY_PATH=./libs ./keeperfx.aarch64
```

## Quick Packaging Command

To create a deployment package from the build directory:
```bash
mkdir -p keeperfx-arkos-deploy
cp keeperfx.aarch64 keeperfx-arkos-deploy/
cp keeperfx_launcher.sh keeperfx-arkos-deploy/
cp -r libs keeperfx-arkos-deploy/
cp -r campgns config gfx lang levels sdl keeperfx-arkos-deploy/
tar -czf keeperfx-arkos.tar.gz keeperfx-arkos-deploy/
```

Then transfer `keeperfx-arkos.tar.gz` to your device and extract it.

## Troubleshooting

### Missing Libraries Error
If you see "error while loading shared libraries", ensure:
1. The `libs/` folder is in the same directory as the executable
2. You're using the launcher script, or
3. LD_LIBRARY_PATH includes the libs directory

### Permission Errors
Make sure the binary and launcher are executable:
```bash
chmod +x keeperfx.aarch64
chmod +x keeperfx_launcher.sh
```

### Graphics/Sound Issues
- Ensure all game data folders (gfx/, lang/, levels/, etc.) are present
- Check that SDL configuration in `sdl/` folder is appropriate for your device

## Library Information

All libraries were built from source for compatibility with ArkOS:
- **SDL2 2.0.22** with KMS/DRM support for handheld devices
- **SDL2_mixer 2.0.4** with support for OGG, FLAC, Opus audio
- **SDL2_net 2.0.1** for multiplayer networking
- **LuaJIT 2.1.0-beta3** for scripting
- **spng 0.7.2** for PNG loading
- **FFmpeg libraries** (Ubuntu 20.04 versions) for video playback

## Build Environment
- **Docker Container**: ghcr.io/monkeyx-net/portmaster-build-templates/portmaster-builder:aarch64-latest
- **Base OS**: Ubuntu 20.04.6 LTS
- **Compiler**: GCC 9.4.0
- **Target**: ARM64 (armv8-a)

## Notes
- This build uses `-march=armv8-a` for broad ARM64 compatibility
- All dependencies are statically bundled to avoid version conflicts with ArkOS system libraries
- The binary includes debug symbols for troubleshooting (can be stripped to reduce size if needed)

## Next Steps
If the game crashes or has issues on ArkOS:
1. Check the console output for error messages
2. Verify all game data files are present
3. Test with the launcher script to ensure library paths are correct
4. Report issues with device model and full error output
