#!/usr/bin/env sh
set -eu

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
cd "$SCRIPT_DIR"

export LD_LIBRARY_PATH="$SCRIPT_DIR/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"

# Now we launch KeeperFX - explicitly pass LD_LIBRARY_PATH through sudo using env
env LD_LIBRARY_PATH="$LD_LIBRARY_PATH" "./keeperfx"

&