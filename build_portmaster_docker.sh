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
    echo "Building spng..."
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
    cd /tmp
    
    # Build LuaJIT
    echo "Building LuaJIT..."
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
    
    # Build SDL2 2.0.22 (compatible with old ArkOS but has newer features)
    echo "Building SDL2..."
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
    
    # Build SDL2_mixer
    echo "Building SDL2_mixer..."
    cd /tmp
    wget -q https://www.libsdl.org/projects/SDL_mixer/release/SDL2_mixer-2.0.4.tar.gz
    tar -xzf SDL2_mixer-2.0.4.tar.gz
    cd SDL2_mixer-2.0.4
    ./configure --prefix=/usr/local
    make -j$(nproc)
    make install
    ldconfig
    
    # Build SDL2_net
    echo "Building SDL2_net..."
    cd /tmp
    wget -q https://github.com/libsdl-org/SDL_net/archive/refs/tags/release-2.0.1.tar.gz -O SDL2_net-2.0.1.tar.gz
    tar -xzf SDL2_net-2.0.1.tar.gz
    cd SDL_net-release-2.0.1
    ./configure --prefix=/usr/local
    make -j$(nproc)
    make install
    ldconfig
    
    echo "Building KeeperFX..."
    cd /workspace
    git config --global --add safe.directory /workspace
    make -f linux.mk clean
    export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig:/usr/lib/aarch64-linux-gnu/pkgconfig:/usr/lib/pkgconfig:$PKG_CONFIG_PATH
    make -f linux.mk -j$(nproc)
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
