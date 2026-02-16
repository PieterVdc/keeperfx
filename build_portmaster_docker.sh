#!/bin/bash
# Build KeeperFX ARM64 binary for PortMaster/ArkOS using Docker

set -e

echo "=== Building KeeperFX for PortMaster/ArkOS (ARM64) ==="
# Check if we're on the portmaster branch (has all the ARM64 fixes)
CURRENT_BRANCH=$(git rev-parse --abbrev-ref HEAD 2>/dev/null || echo "HEAD")
if [[ ! "$CURRENT_BRANCH" =~ portmaster ]]; then
    echo "WARNING: Not on portmaster branch."
    echo "The portmaster branch has ARM64 optimizations and SDL2 fixes."
    echo "Current branch: $CURRENT_BRANCH"
fi

# Create cache directory for built dependencies
CACHE_DIR="$(pwd)/build-cache-docker"
mkdir -p "$CACHE_DIR"
echo "Using build cache at: $CACHE_DIR"

# Pull the PortMaster builder image for ARM64 (if not already present)
echo "Checking for PortMaster builder Docker image..."
if ! sudo docker images | grep -q portmaster-builder; then
    echo "Pulling PortMaster builder Docker image..."
    sudo docker pull --platform=linux/arm64 ghcr.io/monkeyx-net/portmaster-build-templates/portmaster-builder:aarch64-latest
fi

# Build inside the container
echo "Running build in Docker container..."
sudo docker run --rm \
  -v "$(pwd)":/workspace \
  -v "$CACHE_DIR":/cache \
  --platform=linux/arm64 \
  ghcr.io/monkeyx-net/portmaster-build-templates/portmaster-builder:aarch64-latest \
  bash -c '
    set -e
    echo "Installing and building dependencies for ArkOS compatibility..."
    
    # Install available packages from apt
    echo "Installing system packages..."
    apt-get update -qq
    apt-get install -y -qq \
        libavcodec-dev \
        libavformat-dev \
        libavutil-dev \
        libswresample-dev \
        libswscale-dev \
        libpng-dev \
        zlib1g-dev \
        libminizip-dev \
        libminiupnpc-dev \
        libnatpmp-dev \
        cmake \
        ninja-build \
        python3-pip \
        || true
    
    cd /tmp
    
    # Build spng (libpng-based simple PNG library)
    if [ -f /cache/spng-installed.tar.gz ]; then
        echo "Using cached spng..."
        cd /usr/local
        tar -xzf /cache/spng-installed.tar.gz
        ldconfig
    else
        echo "Building spng from source..."
        cd /tmp
        wget -q https://github.com/randy408/libspng/archive/refs/tags/v0.7.2.tar.gz
        tar -xzf v0.7.2.tar.gz
        cd libspng-0.7.2
        # Use CMake instead of meson
        mkdir build && cd build
        cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local -DCMAKE_BUILD_TYPE=Release -DSPNG_STATIC=ON
        make -j$(nproc)
        make install
        # Create pkg-config file
        cat > /usr/local/lib/pkgconfig/spng.pc <<EOF
prefix=/usr/local
exec_prefix=\${prefix}
libdir=\${exec_prefix}/lib
includedir=\${prefix}/include

Name: spng
Description: Simple PNG library
Version: 0.7.2
Libs: -L\${libdir} -lspng
Cflags: -I\${includedir}
EOF
        ldconfig
        # Cache the installation
        cd /usr/local
        tar -czf /cache/spng-installed.tar.gz include/spng.h lib/libspng.* lib/pkgconfig/spng.pc
        echo "spng cached to /cache/spng-installed.tar.gz"
    fi
    cd /tmp
    
    # Build LuaJIT
    if [ -f /cache/luajit-installed.tar.gz ]; then
        echo "Using cached LuaJIT..."
        cd /usr/local
        tar -xzf /cache/luajit-installed.tar.gz
        ln -sf /usr/local/bin/luajit-2.1.0-beta3 /usr/local/bin/luajit 2>/dev/null || true
        ldconfig
    else
        echo "Building LuaJIT from source..."
        cd /tmp
        wget -q https://github.com/LuaJIT/LuaJIT/archive/refs/tags/v2.1.0-beta3.tar.gz
        tar -xzf v2.1.0-beta3.tar.gz
        cd LuaJIT-2.1.0-beta3
        make -j$(nproc) PREFIX=/usr/local
        make install PREFIX=/usr/local
        ln -sf /usr/local/bin/luajit-2.1.0-beta3 /usr/local/bin/luajit
        # Create pkg-config file
        cat > /usr/local/lib/pkgconfig/luajit.pc <<EOF
prefix=/usr/local
exec_prefix=\${prefix}
libdir=\${exec_prefix}/lib
includedir=\${prefix}/include/luajit-2.1

Name: LuaJIT
Description: Just-in-time compiler for Lua
Version: 2.1.0
Libs: -L\${libdir} -lluajit-5.1
Libs.private: -lm -ldl
Cflags: -I\${includedir}
EOF
        ldconfig
        # Cache the installation
        cd /usr/local
        tar -czf /cache/luajit-installed.tar.gz bin/luajit* lib/libluajit* lib/pkgconfig/luajit.pc include/luajit-2.1 share/luajit*
        echo "LuaJIT cached to /cache/luajit-installed.tar.gz"
    fi
    
    # Build SDL2 2.0.22 (compatible with old ArkOS but has newer features)
    if [ -f /cache/sdl2-installed.tar.gz ]; then
        echo "Using cached SDL2..."
        cd /usr/local
        tar -xzf /cache/sdl2-installed.tar.gz
        ldconfig
    else
        echo "Building SDL2 from source..."
        cd /tmp
        wget -q https://github.com/libsdl-org/SDL/releases/download/release-2.0.22/SDL2-2.0.22.tar.gz
        tar -xzf SDL2-2.0.22.tar.gz
        cd SDL2-2.0.22
        ./configure --prefix=/usr/local \
            --disable-input-tslib \
            --disable-video-wayland \
            --disable-video-x11 \
            --disable-video-opengl \
            --disable-video-opengles \
            --disable-pulseaudio \
            --disable-esd \
            --enable-video-kmsdrm \
            --enable-video-fbdev
        make -j$(nproc)
        make install
        ldconfig
        # Cache the installation
        cd /usr/local
        tar -czf /cache/sdl2-installed.tar.gz bin/sdl2-config include/SDL2 lib/libSDL2* lib/pkgconfig/sdl2.pc lib/cmake/SDL2 share/aclocal/sdl2.m4
        echo "SDL2 cached to /cache/sdl2-installed.tar.gz"
    fi
    
    # Build SDL2_mixer
    if [ -f /cache/sdl2_mixer-installed.tar.gz ]; then
        echo "Using cached SDL2_mixer..."
        cd /usr/local
        tar -xzf /cache/sdl2_mixer-installed.tar.gz
        ldconfig
    else
        echo "Building SDL2_mixer from source..."
        cd /tmp
        wget -q https://www.libsdl.org/projects/SDL_mixer/release/SDL2_mixer-2.0.4.tar.gz
        tar -xzf SDL2_mixer-2.0.4.tar.gz
        cd SDL2_mixer-2.0.4
        ./configure --prefix=/usr/local
        make -j$(nproc)
        make install
        ldconfig
        # Cache the installation
        cd /usr/local
        tar -czf /cache/sdl2_mixer-installed.tar.gz include/SDL2/SDL_mixer.h lib/libSDL2_mixer* lib/pkgconfig/SDL2_mixer.pc
        echo "SDL2_mixer cached to /cache/sdl2_mixer-installed.tar.gz"
    fi
    
    # Build SDL2_net
    if [ -f /cache/sdl2_net-installed.tar.gz ]; then
        echo "Using cached SDL2_net..."
        cd /usr/local
        tar -xzf /cache/sdl2_net-installed.tar.gz
        ldconfig
    else
        echo "Building SDL2_net from source..."
        cd /tmp
        wget -q https://github.com/libsdl-org/SDL_net/archive/refs/tags/release-2.0.1.tar.gz -O SDL2_net-2.0.1.tar.gz
        tar -xzf SDL2_net-2.0.1.tar.gz
        cd SDL_net-release-2.0.1
        ./configure --prefix=/usr/local
        make -j$(nproc)
        make install
        ldconfig
        # Cache the installation
        cd /usr/local
        tar -czf /cache/sdl2_net-installed.tar.gz include/SDL2/SDL_net.h lib/libSDL2_net* lib/pkgconfig/SDL2_net.pc
        echo "SDL2_net cached to /cache/sdl2_net-installed.tar.gz"
    fi
    
    echo "Building KeeperFX..."
    cd /workspace
    git config --global --add safe.directory /workspace
    make -f linux.mk clean
    export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig:/usr/lib/aarch64-linux-gnu/pkgconfig:/usr/lib/pkgconfig:$PKG_CONFIG_PATH
    make -f linux.mk -j$(nproc)
    
    # Copy ALL required shared libraries including transitive dependencies
    echo "Collecting all shared library dependencies..."
    mkdir -p /workspace/libs.aarch64
    
    # Get all library dependencies from ldd and copy them
    ldd /workspace/bin/keeperfx | grep "=>" | awk '"'"'{print $3}'"'"' | grep -v -e "^$" | while IFS= read -r lib; do
        if [ -f "$lib" ]; then
            libname=$(basename "$lib")
            
            # Skip system libraries that MUST use target system versions
            # These cause GLIBC version conflicts and ABI issues
            # Also skip SDL2 - use PortMaster device-specific patched SDL2
            case "$libname" in
                libc.so*|libc-*.so*)
                    echo "  Skipping: $libname (system C library)"
                    continue
                    ;;
                libpthread.so*|libpthread-*.so*)
                    echo "  Skipping: $libname (system pthread)"
                    continue
                    ;;
                libm.so*|libm-*.so*)
                    echo "  Skipping: $libname (system math)"
                    continue
                    ;;
                libdl.so*|libdl-*.so*)
                    echo "  Skipping: $libname (system dl)"
                    continue
                    ;;
                librt.so*|librt-*.so*)
                    echo "  Skipping: $libname (system rt)"
                    continue
                    ;;
                libgcc_s.so*|libstdc++.so*)
                    echo "  Skipping: $libname (system C++ runtime)"
                    continue
                    ;;
                ld-linux-*.so*|ld-*.so*)
                    echo "  Skipping: $libname (dynamic linker)"
                    continue
                    ;;
                libSDL2*)
                    echo "  Skipping: $libname (use PortMaster system SDL2)"
                    continue
                    ;;
            esac
            
            # Copy the library and its symlinks
            libdir=$(dirname "$lib")
            echo "  Copying: $libname"
            cp -L "$lib" /workspace/libs.aarch64/ 2>/dev/null || true
            cp -L "$libdir/$libname"* /workspace/libs.aarch64/ 2>/dev/null || true
        fi
    done
    
    echo "Libraries copied to libs/ folder (\$(ls /workspace/libs.aarch64/ | wc -l) files)"
  '

# Copy and rename the output
if [ -f bin/keeperfx ]; then
    echo "Build successful! Copying to keeperfx.aarch64..."
    cp bin/keeperfx keeperfx.aarch64
    chmod +x keeperfx.aarch64
    ls -lh keeperfx.aarch64
    echo "=== Build complete! Output: keeperfx.aarch64 ==="
else
    echo "ERROR: bin/keeperfx not found!"
    exit 1
fi
