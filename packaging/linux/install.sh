#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BIN="$SCRIPT_DIR/ofs"
TARGET="/usr/local/bin/ofs"
ICON_SRC="$SCRIPT_DIR/ofs.png"
ICON_TARGET="/usr/share/icons/hicolor/256x256/apps/ofs.png"
LICENSE_SRC="$SCRIPT_DIR/LICENSE"
LICENSE_TARGET_DIR="/usr/local/share/doc/ofs"
LICENSE_TARGET="$LICENSE_TARGET_DIR/LICENSE"

if [ ! -f "$BIN" ]; then
  echo "ofs binary not found next to installer script"
  exit 1
fi

echo "[OFS] Installing compiler binary..."
sudo install -m 0755 "$BIN" "$TARGET"

if [ -f "$ICON_SRC" ]; then
  echo "[OFS] Installing icon..."
  sudo install -d /usr/share/icons/hicolor/256x256/apps
  sudo install -m 0644 "$ICON_SRC" "$ICON_TARGET"
fi

if [ -f "$LICENSE_SRC" ]; then
  echo "[OFS] Installing license terms..."
  sudo install -d "$LICENSE_TARGET_DIR"
  sudo install -m 0644 "$LICENSE_SRC" "$LICENSE_TARGET"
fi

echo "[OFS] Installed at $TARGET"
ofs version || true
