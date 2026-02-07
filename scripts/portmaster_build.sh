#!/usr/bin/env bash
set -euo pipefail

WORKDIR="/mnt/a/CodeProjects/KeeperFx/keeperfx"
BUILD_DIR="$HOME/keeperfx_build"

ensure_cross_compiler() {
  if ! command -v aarch64-linux-gnu-gcc >/dev/null 2>&1; then
    sudo apt-get update
    sudo apt-get install -y gcc-aarch64-linux-gnu g++-aarch64-linux-gnu
  fi
}

enable_arm64_arch() {
  sudo dpkg --add-architecture arm64
  # Avoid cross-arch conflicts from i386 in multiarch setups
  if dpkg --print-foreign-architectures | grep -q '^i386$'; then
    echo "i386 is enabled; skipping removal to avoid dpkg errors."
  fi
  # Ensure arm64 packages resolve from Ubuntu ports, not archive.ubuntu.com
  sudo tee /etc/apt/sources.list.d/arm64.list >/dev/null <<'EOF'
deb [arch=arm64] http://ports.ubuntu.com/ubuntu-ports noble main restricted universe multiverse
deb [arch=arm64] http://ports.ubuntu.com/ubuntu-ports noble-updates main restricted universe multiverse
deb [arch=arm64] http://ports.ubuntu.com/ubuntu-ports noble-security main restricted universe multiverse
deb [arch=arm64] http://ports.ubuntu.com/ubuntu-ports noble-backports main restricted universe multiverse
EOF
  # Prevent apt from trying to fetch arm64 packages from amd64 sources
  if [ -f /etc/apt/sources.list.d/ubuntu.sources ]; then
    sudo mv /etc/apt/sources.list.d/ubuntu.sources /etc/apt/sources.list.d/ubuntu.sources.bak
  fi
  sudo tee /etc/apt/sources.list >/dev/null <<'EOF'
deb [arch=amd64] http://archive.ubuntu.com/ubuntu noble main restricted universe multiverse
deb [arch=amd64] http://archive.ubuntu.com/ubuntu noble-updates main restricted universe multiverse
deb [arch=amd64] http://archive.ubuntu.com/ubuntu noble-backports main restricted universe multiverse
deb [arch=amd64] http://security.ubuntu.com/ubuntu noble-security main restricted universe multiverse
EOF
  sudo apt-get update
}

ensure_arm64_deps() {
  if [ ! -f /usr/lib/aarch64-linux-gnu/pkgconfig/sdl2.pc ]; then
    echo "Installing ARM64 SDL2 and dependencies..."
    sudo apt-get install -y \
      libsdl2-dev:arm64 \
      libsdl2-mixer-dev:arm64 \
      libsdl2-net-dev:arm64 \
      libjack-jackd2-0:arm64 \
      libjack-jackd2-dev:arm64 \
      libglib2.0-dev-bin:arm64 \
      libglib2.0-dev-bin-linux:arm64 \
      libavcodec-dev:arm64 \
      libavformat-dev:arm64 \
      libavutil-dev:arm64 \
      libswresample-dev:arm64 \
      libswscale-dev:arm64 \
      libopenal-dev:arm64 \
      pkg-config
  fi
}

copy_sources() {
  echo "Copying sources into WSL2 workspace..."
  rm -rf "$BUILD_DIR"
  mkdir -p "$BUILD_DIR"
  tar -C "$WORKDIR" -cf - . | tar -C "$BUILD_DIR" -xf -
}

debug_pkg_config() {
  echo "Debugging pkg-config paths:"
  export PKG_CONFIG_PATH=/usr/lib/aarch64-linux-gnu/pkgconfig:/usr/share/pkgconfig
  export PKG_CONFIG_LIBDIR=/usr/lib/aarch64-linux-gnu/pkgconfig
  echo "PKG_CONFIG_PATH=$PKG_CONFIG_PATH"
  echo "PKG_CONFIG_LIBDIR=$PKG_CONFIG_LIBDIR"
  echo "SDL2 cflags:"
  pkg-config --cflags-only-I sdl2 || echo "SDL2 pkg-config FAILED"
  echo "SDL2 .pc file location:"
  pkg-config --variable=prefix sdl2 || echo "NO PREFIX"
  ls -la /usr/lib/aarch64-linux-gnu/pkgconfig/sdl2.pc 2>&1
  echo "SDL2 header check:"
  ls -la /usr/include/aarch64-linux-gnu/SDL2/SDL_config.h 2>&1 || echo "ARM64 SDL_config.h not found"
  ls -la /usr/include/SDL2/SDL_config.h 2>&1 || echo "x86_64 SDL_config.h not found"
}

build_project() {
  echo "Building..."
  cd "$BUILD_DIR"
  rm -rf obj bin src/ver_defs.h deps/astronomy/libastronomy.a deps/centijson/libjson.a
  export PKG_CONFIG_PATH=/usr/lib/aarch64-linux-gnu/pkgconfig:/usr/share/pkgconfig
  export PKG_CONFIG_LIBDIR=/usr/lib/aarch64-linux-gnu/pkgconfig
  CC=aarch64-linux-gnu-gcc CXX=aarch64-linux-gnu-g++ make -f linux.mk -j4
}

copy_output() {
  cp -f "$BUILD_DIR/bin/keeperfx" "$WORKDIR/keeperfx.aarch64"
}

main() {
  ensure_cross_compiler
  enable_arm64_arch
  ensure_arm64_deps
  copy_sources
  debug_pkg_config
  build_project
  copy_output
}

main "$@"
