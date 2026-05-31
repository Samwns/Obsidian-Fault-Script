#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BIN="$SCRIPT_DIR/ofs"
TARGET="/usr/local/bin/ofs"
OFSCC_SRC="$SCRIPT_DIR/ofscc"
OFSCC_TARGET="/usr/local/bin/ofscc"
RUNTIME_SRC="$SCRIPT_DIR/libofs_runtime.a"
RUNTIME_TARGET_DIR="/usr/local/lib"
RUNTIME_TARGET="$RUNTIME_TARGET_DIR/libofs_runtime.a"
ICON_SRC="$SCRIPT_DIR/ofs.png"
ICON_TARGET="/usr/share/icons/hicolor/256x256/apps/ofs.png"
LICENSE_SRC="$SCRIPT_DIR/LICENSE"
LICENSE_TARGET_DIR="/usr/local/share/doc/ofs"
LICENSE_TARGET="$LICENSE_TARGET_DIR/LICENSE"
STDLIB_SRC="$SCRIPT_DIR/stdlib"
STDLIB_TARGET="/usr/local/share/ofs/stdlib"
ACCEPT_LICENSE="${OFS_ACCEPT_LICENSE:-}"

for arg in "$@"; do
  case "$arg" in
    --accept-license|--yes|-y)
      ACCEPT_LICENSE=1
      ;;
    --help|-h)
      cat <<'EOF'
OFS installer

Usage:
  ./install.sh
  ./install.sh --accept-license

Options:
  --accept-license, --yes, -y   Accept the bundled license non-interactively.
  --help, -h                    Show this help.
EOF
      exit 0
      ;;
  esac
done

banner() {
  cat <<'EOF'

========================================
 Obsidian Fault Script Installer
========================================

EOF
}

require_license_acceptance() {
  banner
  if [ -f "$LICENSE_SRC" ]; then
    echo "License: Boost Software License 1.0"
    echo "License file: $LICENSE_SRC"
  else
    echo "License file was not found in this package."
  fi
  echo
  echo "This installer will install OFS command-line tools, the self-hosted"
  echo "compiler, standard library files, and native runtime files."
  echo

  if [ "$ACCEPT_LICENSE" = "1" ] || [ "$ACCEPT_LICENSE" = "true" ]; then
    echo "[OFS] License accepted non-interactively."
    return
  fi

  if [ ! -t 0 ]; then
    echo "[OFS] License acceptance is required. Re-run with --accept-license." >&2
    exit 2
  fi

  printf "Do you accept the license terms and continue? [y/N] "
  read -r answer
  case "$answer" in
    y|Y|yes|YES|Yes)
      echo "[OFS] License accepted."
      ;;
    *)
      echo "[OFS] Installation cancelled."
      exit 2
      ;;
  esac
}

if [ ! -f "$BIN" ]; then
  echo "ofs wrapper not found next to installer script"
  exit 1
fi

if [ ! -f "$OFSCC_SRC" ]; then
  echo "ofscc compiler not found next to installer script"
  exit 1
fi

require_license_acceptance

echo "[OFS] Installing command wrapper..."
sudo install -m 0755 "$BIN" "$TARGET"

echo "[OFS] Installing self-hosted compiler..."
sudo install -m 0755 "$OFSCC_SRC" "$OFSCC_TARGET"

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

if [ -d "$STDLIB_SRC" ]; then
  echo "[OFS] Installing standard library files..."
  sudo install -d "$STDLIB_TARGET"
  sudo find "$STDLIB_SRC" -maxdepth 1 -name "*.ofs" -exec install -m 0644 {} "$STDLIB_TARGET/" \;
fi

echo "[OFS] Installed at $TARGET"
if [ -f "$RUNTIME_TARGET" ]; then
  echo "[OFS] Runtime installed at $RUNTIME_TARGET"
fi

# Install package manager wrapper scripts (infuse, uncover, reinfuse)
for cmd in infuse uncover reinfuse; do
  if [ -f "$SCRIPT_DIR/$cmd" ]; then
    echo "[OFS] Installing '$cmd' wrapper..."
    sudo install -m 0755 "$SCRIPT_DIR/$cmd" "/usr/local/bin/$cmd"
  fi
done

ofs version || true
