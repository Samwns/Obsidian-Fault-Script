#!/usr/bin/env bash
# Gera ofs-termux-aarch64.tar.gz (roda no PC, Linux x86-64).
# Uso: build_termux.sh [versao] [saida.tar.gz]
# Env: ANDROID_SDK|ANDROID_HOME|ANDROID_SDK_ROOT|ANDROID_NDK (NDK r25+),
#      HOST_OFSCC (ofscc do PC com OFSCC_TARGET; senao e montado do bootstrap).
# Layout do tar (extrair em $PREFIX): bin/ofs, lib/ofs/{ofscc,magma.o,stdlib/,tools/}
set -euo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO="$(cd "$HERE/../../.." && pwd)"
VERSION="${1:-${OFS_VERSION:-}}"
OUT="${2:-$REPO/ofs-termux-aarch64.tar.gz}"
case "$OUT" in /*) ;; *) OUT="$PWD/$OUT" ;; esac
die() { echo "build_termux: $*" >&2; exit 1; }
step() { echo "==> $*" >&2; }

# --- NDK ---
NDK="${ANDROID_NDK:-}"
if [ -z "$NDK" ]; then
  SDK="${ANDROID_SDK:-${ANDROID_HOME:-${ANDROID_SDK_ROOT:-$HOME/Android/Sdk}}}"
  NDK="$(ls -d "$SDK"/ndk/* 2>/dev/null | sort -V | tail -1 || true)"
fi
[ -n "$NDK" ] && [ -d "$NDK" ] || die "NDK nao encontrado (defina ANDROID_NDK ou ANDROID_SDK)"
NDKCC="$NDK/toolchains/llvm/prebuilt/linux-x86_64/bin/clang"
[ -x "$NDKCC" ] || die "clang do NDK nao encontrado: $NDKCC"
TRIPLE="aarch64-linux-android26"

W="$(mktemp -d "${TMPDIR:-/tmp}/ofs-termux-XXXXXX")"
trap 'rm -rf "$W"' EXIT
export OFS_STDLIB_PATH="$REPO/ofs/stdlib" OFS_STDLIB="$REPO/ofs/stdlib"
export NO_COLOR=1

# --- ofscc do PC com suporte a OFSCC_TARGET ---
HOST="${HOST_OFSCC:-}"
if [ -z "$HOST" ]; then
  step "montando ofscc do PC a partir do bootstrap (ofs/dist)"
  hc="${CC:-clang}"
  # O IR usa `ptr` (ponteiros opacos): o clang 14 (Ubuntu 22.04) so aceita
  # com -opaque-pointers; do 15 em diante e o padrao.
  HC_FLAGS=()
  "$hc" --version 2>/dev/null | grep -Eq 'version 14\.|clang-1400\.' && HC_FLAGS=(-mllvm -opaque-pointers)
  "$hc" "${HC_FLAGS[@]}" -Wno-override-module -c "$REPO/ofs/dist/magma.ll" -o "$W/magma-host.o"
  "$hc" "${HC_FLAGS[@]}" -Wno-override-module -Wl,--allow-multiple-definition -no-pie -O2 "$REPO/ofs/dist/ofscc.ll" "$W/magma-host.o" -lm -o "$W/ofscc0"
  OFS_RUNTIME="$W/magma-host.o" OFSCC_INPUT="$REPO/ofs/ofscc/ofscc.ofs" OFSCC_MODE=ir OFSCC_C_OUT="$W/ofscc1.ll" "$W/ofscc0" >"$W/s1.log" 2>&1 \
    || { tail -20 "$W/s1.log" >&2; die "falha ao compilar o ofscc do fonte"; }
  "$hc" "${HC_FLAGS[@]}" -Wno-override-module -Wl,--allow-multiple-definition -no-pie -O2 "$W/ofscc1.ll" "$W/magma-host.o" -lm -o "$W/ofscc1"
  HOST="$W/ofscc1"
fi

ir() { # fonte saida.ll
  OFSCC_TARGET=android OFSCC_INPUT="$1" OFSCC_MODE=ir OFSCC_C_OUT="$2" "$HOST" >"$2.log" 2>&1 \
    || { tail -20 "$2.log" >&2; die "ofscc falhou em $1"; }
  [ -s "$2" ] || die "ofscc nao gerou $2"
  grep -q aarch64 "$2" || die "o ofscc do PC nao suporta OFSCC_TARGET (defina HOST_OFSCC)"
}

step "magma.o (aarch64-android)"
ir "$REPO/ofs/stdlib/runtime/magma.ofs" "$W/magma.ll"
"$NDKCC" --target="$TRIPLE" -O2 -fPIC -Wno-override-module -c "$W/magma.ll" -o "$W/magma.o"

step "ofscc (aarch64-android)"
ir "$REPO/ofs/ofscc/ofscc.ofs" "$W/ofscc.ll"
"$NDKCC" --target="$TRIPLE" -O2 -Wno-override-module -Wl,--allow-multiple-definition -rdynamic "$W/ofscc.ll" "$W/magma.o" -lm -o "$W/ofscc"

step "montando layout"
P="$W/pkg"
mkdir -p "$P/bin" "$P/lib/ofs/stdlib" "$P/lib/ofs/tools"
cp "$W/ofscc" "$P/lib/ofs/ofscc"
cp "$W/magma.o" "$P/lib/ofs/magma.o"
cp -r "$REPO/ofs/stdlib/." "$P/lib/ofs/stdlib/"
cp "$REPO"/ofs/tools/*.ofs "$P/lib/ofs/tools/"
if [ -n "$VERSION" ]; then
  VERSION="${VERSION#v}"
  sed "s/__OFS_VERSION__/$VERSION/g" "$REPO/src/packaging/linux/ofs" > "$P/bin/ofs"
else
  cp "$REPO/src/packaging/linux/ofs" "$P/bin/ofs"
fi
# Termux nao tem /usr/bin/env: o pacote usa o shebang do proprio Termux.
sed -i '1s|.*|#!/data/data/com.termux/files/usr/bin/env bash|' "$P/bin/ofs"
chmod 755 "$P/bin/ofs" "$P/lib/ofs/ofscc"
"$NDK/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-strip" "$P/lib/ofs/ofscc" 2>/dev/null || true

mkdir -p "$(dirname "$OUT")"
tar -C "$P" -czf "$OUT" bin lib
echo "Gerado: $OUT ($(du -h "$OUT" | cut -f1))"
