#!/usr/bin/env bash
set -euo pipefail

if [[ $# -lt 1 ]]; then
  echo "Usage: $0 <library-name>"
  echo "Example: $0 terminal-colors"
  exit 1
fi

LIB_NAME="$1"
LIB_HOME="${OFS_LIB_HOME:-$HOME/.ofs/libs}"

if [[ ! -d "$LIB_HOME" ]]; then
  echo "Library directory not found: $LIB_HOME"
  exit 1
fi

# Remove all .ofs files matching the library name pattern
found=0
while IFS= read -r -d '' file; do
  rm -f "$file"
  echo "Removed: $file"
  found=1
done < <(find "$LIB_HOME" -type f -name "*${LIB_NAME}*" -print0 2>/dev/null || true)

if [[ $found -eq 0 ]]; then
  echo "No libraries found matching: $LIB_NAME"
  exit 1
fi

echo "Library '$LIB_NAME' uninstalled successfully"
