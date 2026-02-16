#!/bin/bash
# Collect all required shared libraries for KeeperFX deployment

set -e

BINARY="bin/keeperfx"
TARGET_DIR="libs_complete"

if [ ! -f "$BINARY" ]; then
    echo "Error: $BINARY not found!"
    exit 1
fi

echo "Collecting all shared library dependencies..."
mkdir -p "$TARGET_DIR"

# Get all library dependencies and their paths
ldd "$BINARY" | grep "=>" | awk '{print $3}' | grep -v "^$" | while read -r lib; do
    if [ -f "$lib" ]; then
        # Copy the library (following symlinks)
        cp -Lv "$lib" "$TARGET_DIR/" 2>/dev/null || echo "  (skipped system lib: $lib)"
        
        # Also copy versioned symlinks if they exist
        libname=$(basename "$lib")
        libdir=$(dirname "$lib")
        cp -Lv "$libdir/$libname"* "$TARGET_DIR/" 2>/dev/null || true
    fi
done

echo ""
echo "=== Library Collection Complete ==="
ls -lh "$TARGET_DIR/" | wc -l
echo "Total size:"
du -sh "$TARGET_DIR/"

echo ""
echo "=== Missing libraries check ==="
LD_LIBRARY_PATH="$TARGET_DIR" ldd "$BINARY" | grep "not found" || echo "✓ All dependencies satisfied!"
