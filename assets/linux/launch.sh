#!/usr/bin/env sh
set -eu

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
cd "$SCRIPT_DIR"

export LD_LIBRARY_PATH="$SCRIPT_DIR/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"

if ldd "$SCRIPT_DIR/keeperfx" | grep -q 'not found'; then
  echo "Missing system dependencies were detected:"
  ldd "$SCRIPT_DIR/keeperfx" | grep 'not found' || true
  echo
  echo "Run ./install-deps.sh and then start again."
  exit 1
fi

exec "$SCRIPT_DIR/keeperfx" "$@"