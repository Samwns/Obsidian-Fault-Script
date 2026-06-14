#!/usr/bin/env bash
set -euo pipefail

if [[ $# -lt 1 ]]; then
  echo "Usage: $0 <package.tar.gz>"
  exit 1
fi

PKG_FILE="$1"
if [[ ! -f "$PKG_FILE" ]]; then
  echo "Package not found: $PKG_FILE"
  exit 1
fi

LIB_HOME="${OFS_LIB_HOME:-$HOME/.ofs/libs}"
TMP_DIR="$(mktemp -d)"
trap 'rm -rf "$TMP_DIR"' EXIT

tar -xzf "$PKG_FILE" -C "$TMP_DIR"

mkdir -p "$LIB_HOME"
find "$TMP_DIR" -type f -name "*.ofs" -exec cp {} "$LIB_HOME" \;

echo "Installed OFS libraries to: $LIB_HOME"
echo "Set OFS_LIB_PATH to include this path:"
echo "  export OFS_LIB_PATH=\"$LIB_HOME\""
