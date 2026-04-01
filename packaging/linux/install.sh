#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BIN="$SCRIPT_DIR/ofs"
TARGET="/usr/local/bin/ofs"
ICON_SRC="$SCRIPT_DIR/ofs.png"
ICON_TARGET="/usr/share/icons/hicolor/256x256/apps/ofs.png"

if [ ! -f "$BIN" ]; then
  echo "ofs binary not found next to installer script"
  exit 1
fi

sudo install -m 0755 "$BIN" "$TARGET"

if [ -f "$ICON_SRC" ]; then
  sudo install -d /usr/share/icons/hicolor/256x256/apps
  sudo install -m 0644 "$ICON_SRC" "$ICON_TARGET"
fi

echo "OFS installed at $TARGET"
ofs version || true
