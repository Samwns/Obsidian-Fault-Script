#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BIN="$SCRIPT_DIR/ofs"
TARGET="/usr/local/bin/ofs"

if [ ! -f "$BIN" ]; then
  echo "ofs binary not found next to installer script"
  exit 1
fi

sudo install -m 0755 "$BIN" "$TARGET"
echo "OFS installed at $TARGET"
ofs version || true
