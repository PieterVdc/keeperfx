#!/bin/bash
export DEVKITPRO=/mnt/d/devkitPro
export DEVKITPPC=$DEVKITPRO/devkitPPC

# Windows devkitPro: create wrapper scripts so make can find tools without .exe
SHIM_DIR="/tmp/dkp_shims"
if [ ! -f "$SHIM_DIR/powerpc-eabi-gcc" ]; then
    rm -rf "$SHIM_DIR"
    mkdir -p "$SHIM_DIR"
    for exe in "$DEVKITPPC/bin/"*.exe "$DEVKITPRO/tools/bin/"*.exe; do
        [ -f "$exe" ] || continue
        base=$(basename "$exe" .exe)
        cat > "$SHIM_DIR/$base" <<SHIM
#!/bin/bash
exec "$exe" "\$@"
SHIM
        chmod +x "$SHIM_DIR/$base"
    done
fi
export PATH="$SHIM_DIR:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin"

# Verify compiler is reachable
if ! command -v powerpc-eabi-gcc &>/dev/null; then
    echo "ERROR: powerpc-eabi-gcc not found"
    exit 1
fi

cd /mnt/a/CodeProjects/KeeperFx/keeperfx
make -f wii.mk "$@"
