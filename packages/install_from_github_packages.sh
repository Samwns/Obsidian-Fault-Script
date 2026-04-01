#!/usr/bin/env bash
set -euo pipefail

if [[ $# -lt 1 ]]; then
  echo "Usage: $0 <library-name> [version]"
  echo "Example: $0 terminal-colors 1.0.0"
  exit 1
fi

LIB_NAME="$1"
LIB_VERSION="${2:-latest}"
OWNER="${OFS_PKG_OWNER:-samwns}"
OWNER="$(echo "$OWNER" | tr '[:upper:]' '[:lower:]')"
PKG="@${OWNER}/ofs-lib-${LIB_NAME}@${LIB_VERSION}"

if [[ -z "${NPM_TOKEN:-}" ]]; then
  echo "NPM_TOKEN is required to download from GitHub Packages"
  exit 1
fi

LIB_HOME="${OFS_LIB_HOME:-$HOME/.ofs/libs}"
TMP_DIR="$(mktemp -d)"
trap 'rm -rf "$TMP_DIR"' EXIT

cat > "$TMP_DIR/.npmrc" <<EOF
@${OWNER}:registry=https://npm.pkg.github.com
//npm.pkg.github.com/:_authToken=${NPM_TOKEN}
always-auth=true
EOF

(
  cd "$TMP_DIR"
  npm pack "$PKG" >/dev/null
  tar -xzf *.tgz
)

mkdir -p "$LIB_HOME"
cp "$TMP_DIR"/package/libs/*.ofs "$LIB_HOME"/

echo "Installed ${PKG} into $LIB_HOME"
