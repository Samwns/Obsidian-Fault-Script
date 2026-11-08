#!/data/data/com.termux/files/usr/bin/bash
# Instala o OFS no Termux (aarch64).
# Uso: install_termux.sh [ofs-termux-aarch64.tar.gz]
#   sem argumento: baixa a ultima release de Samwns/Obsidian-Fault-Script.
set -euo pipefail
REPO="Samwns/Obsidian-Fault-Script"
ASSET="ofs-termux-aarch64.tar.gz"
PREFIX="${PREFIX:-/data/data/com.termux/files/usr}"
die() { echo "install_termux: $*" >&2; exit 1; }

[ -d "$PREFIX" ] || die "PREFIX nao encontrado: $PREFIX (rode dentro do Termux)"
[ "$(uname -m)" = "aarch64" ] || echo "aviso: arquitetura $(uname -m); o pacote e aarch64" >&2

if ! command -v clang >/dev/null 2>&1; then
  echo "==> pkg install -y clang"
  pkg install -y clang
fi
command -v curl >/dev/null 2>&1 || pkg install -y curl

TMP="$(mktemp -d "${TMPDIR:-$PREFIX/tmp}/ofs-install-XXXXXX")"
trap 'rm -rf "$TMP"' EXIT

if [ $# -ge 1 ]; then
  [ -f "$1" ] || die "arquivo nao encontrado: $1"
  TAR="$1"
else
  echo "==> procurando a ultima release de $REPO"
  URL="$(curl -fsSL "https://api.github.com/repos/$REPO/releases/latest" \
    | grep -o "\"browser_download_url\": *\"[^\"]*$ASSET\"" | head -1 | sed 's/.*: *"\(.*\)"/\1/')"
  [ -n "$URL" ] || die "a ultima release nao tem $ASSET"
  echo "==> baixando $URL"
  curl -fL --progress-bar -o "$TMP/$ASSET" "$URL"
  TAR="$TMP/$ASSET"
fi

echo "==> extraindo em $PREFIX"
tar -xzf "$TAR" -C "$PREFIX"
chmod 755 "$PREFIX/bin/ofs" "$PREFIX/lib/ofs/ofscc"

echo "==> testando"
ofs --version
echo "OFS instalado. Experimente:  echo 'core main() { echo(\"oi\") }' > oi.ofs && ofs run oi.ofs"
