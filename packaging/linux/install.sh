#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BIN="$SCRIPT_DIR/ofs"
TARGET="/usr/local/bin/ofs"
RUNTIME_SRC="$SCRIPT_DIR/libofs_runtime.a"
RUNTIME_TARGET_DIR="/usr/local/lib"
RUNTIME_TARGET="$RUNTIME_TARGET_DIR/libofs_runtime.a"
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

if [ -f "$RUNTIME_SRC" ]; then
  echo "[OFS] Installing native runtime library..."
  sudo install -d "$RUNTIME_TARGET_DIR"
  sudo install -m 0644 "$RUNTIME_SRC" "$RUNTIME_TARGET"
fi

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
if [ -f "$RUNTIME_TARGET" ]; then
  echo "[OFS] Runtime installed at $RUNTIME_TARGET"
fi
ofs version || true
