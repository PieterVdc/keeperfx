#!/usr/bin/env sh
set -eu

if [ "$(id -u)" -eq 0 ]; then
  SUDO=""
else
  SUDO="sudo"
fi

install_apt() {
  $SUDO apt update
  $SUDO apt install -y \
    libsdl2-2.0-0 \
    libsdl2-image-2.0-0 \
    libsdl2-mixer-2.0-0 \
    libsdl2-net-2.0-0 \
    libopenal1 \
    libavcodec60 \
    libavformat60 \
    libavutil58 \
    libswresample4 \
    libpulse0 \
    libasound2
}

install_dnf() {
  $SUDO dnf install -y \
    SDL2 \
    SDL2_image \
    SDL2_mixer \
    SDL2_net \
    openal-soft \
    ffmpeg-libs \
    pulseaudio-libs \
    alsa-lib
}

install_pacman() {
  $SUDO pacman -Syu --needed --noconfirm \
    sdl2 \
    sdl2_image \
    sdl2_mixer \
    sdl2_net \
    openal \
    ffmpeg \
    libpulse \
    alsa-lib
}

install_zypper() {
  $SUDO zypper --non-interactive install \
    libSDL2-2_0-0 \
    libSDL2_image-2_0-0 \
    libSDL2_mixer-2_0-0 \
    libSDL2_net-2_0-0 \
    openal-soft \
    libavcodec60 \
    libavformat60 \
    libavutil58 \
    libswresample4 \
    libpulse0 \
    libasound2
}

if command -v apt >/dev/null 2>&1; then
  install_apt
elif command -v dnf >/dev/null 2>&1; then
  install_dnf
elif command -v pacman >/dev/null 2>&1; then
  install_pacman
elif command -v zypper >/dev/null 2>&1; then
  install_zypper
else
  echo "No supported package manager found (apt, dnf, pacman, zypper)."
  exit 1
fi

echo "Dependencies installed. Launch with ./launch.sh"