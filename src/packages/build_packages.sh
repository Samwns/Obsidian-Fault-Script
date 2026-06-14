#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SRC_DIR="$ROOT_DIR/src"
DIST_DIR="$ROOT_DIR/dist"

mkdir -p "$DIST_DIR"

for pkg in "$SRC_DIR"/*; do
  [[ -d "$pkg" ]] || continue
  name="$(basename "$pkg")"
  out="$DIST_DIR/${name}.tar.gz"
  tar -czf "$out" -C "$pkg" .
  echo "Built package: $out"
done
