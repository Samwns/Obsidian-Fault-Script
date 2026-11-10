#!/usr/bin/env bash
# Empacota cada biblioteca de src/<pacote>/ com `ofs quarry publish`:
#   dist/<pacote>-<versao>.tar.gz (reprodutivel: tar ordenado, datas/donos zerados, gzip -n)
#   e a entrada em registry.json (sha256, tamanho, file e url da release do GitHub).
# O manifesto de cada pacote e ofs.claim (ou o ofspkg.json antigo).
# A URL aponta para a release `quarry-<pacote>-<versao>` (criada pelo
# .github/workflows/publish-libraries.yml); o `ofs quarry` tambem tenta
# <pasta do registro>/dist/<file>, que funciona antes da release existir.
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SRC_DIR="$ROOT_DIR/src"
DIST_DIR="$ROOT_DIR/dist"
OFS="${OFS_BIN:-$ROOT_DIR/../packaging/linux/ofs}"
ORIGIN='https://github.com/Samwns/Obsidian-Fault-Script/releases/download/quarry-{name}-{version}/{file}'
[ -n "${QUARRY_ORIGIN:-}" ] && ORIGIN="$QUARRY_ORIGIN"

mkdir -p "$DIST_DIR"

for pkg in "$SRC_DIR"/*; do
  [[ -d "$pkg" ]] || continue
  ( cd "$pkg" && bash "$OFS" quarry publish --out "$DIST_DIR" --origin "$ORIGIN" --deposit "$ROOT_DIR/registry.json" ) \
    | grep -E '^ofs quarry: pacote:|^sha256' || { echo "falhou: $pkg" >&2; exit 1; }
done
