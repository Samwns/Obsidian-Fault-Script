#!/usr/bin/env bash
# Regressoes do compilador/runtime (bugs: `core`/`anchor` como nome, `array_get
# as crystal`, `ofs_str_char_at` O(n), metodos de `impl`, `a[i] = v`, tipo do
# elemento de `Array<T>` em parametro/campo, curto-circuito de `&&`/`||`).
#   OFSCC_BIN=<ofscc>  (padrao: ofs/dist/ofscc)
# Cada .ofs com .expected roda no build nativo e no interpretador; o
# str_char_at_1mb usa o runtime compilado de stdlib/runtime/magma.ofs (como
# no Android) e tem limite de tempo.
set -uo pipefail
# limite de memoria: um bug no compilador nao pode derrubar a maquina (OOM)
ulimit -v 4194304
cd "$(dirname "$0")"
HERE="$(pwd)"
ROOT="$(cd ../../.. && pwd)"
export OFS_STDLIB_PATH="$ROOT/ofs/stdlib" NO_COLOR=1 OFS_PROGRESS=plain
OFSCC="${OFSCC_BIN:-$ROOT/ofs/dist/ofscc}"
T="$(mktemp -d "${TMPDIR:-/tmp}/ofs-regress-XXXXXX")"
trap 'rm -rf "$T"' EXIT
FAILED=0
fail() { echo "FAIL: $*"; FAILED=$((FAILED + 1)); }

# runtime a partir do fonte (magma.ofs)
( cd "$ROOT/ofs/stdlib/runtime" && OFSCC_MODE=ir OFSCC_INPUT=magma.ofs OFSCC_C_OUT="$T/magma.ll" "$OFSCC" ) >"$T/magma.log" 2>&1 \
  && clang -Wno-override-module -c "$T/magma.ll" -o "$T/magma.o" 2>>"$T/magma.log" \
  || { tail "$T/magma.log"; echo "FAIL: runtime magma.ofs nao compila"; exit 1; }

native() { # native <src> <bin>
  OFSCC_MODE=ir OFSCC_INPUT="$1" OFSCC_C_OUT="$T/p.ll" "$OFSCC" >"$T/build.log" 2>&1 || return 1
  clang -Wno-override-module -Wl,--allow-multiple-definition -no-pie -O1 "$T/p.ll" "$T/magma.o" -lm -o "$2" 2>>"$T/build.log"
}

# 1. palavra reservada como nome (`core` em funcao, `anchor` em variavel):
#    erro de compilacao claro em vez de seguir em silencio
reserved() { # reserved <teste> <palavra>
  if OFSCC_MODE=check OFSCC_INPUT="$HERE/$1.ofs" "$OFSCC" >"$T/$1.log" 2>&1; then
    fail "$1: compilou (devia dar erro)"
  elif ! grep -q "$2. e palavra reservada" "$T/$1.log"; then
    fail "$1: erro sem a mensagem de palavra reservada"; tail -3 "$T/$1.log"
  else
    echo "ok: $1"
  fi
}
reserved core_reserved_name core
reserved anchor_reserved_name anchor

# 2. saida esperada (nativo e interpretador)
for exp in "$HERE"/*.expected; do
  name="$(basename "$exp" .expected)"; src="$HERE/$name.ofs"
  native "$src" "$T/$name" || { tail -3 "$T/build.log"; fail "$name: nao compila"; continue; }
  out="$(timeout 10 "$T/$name" 2>&1)" || { fail "$name (nativo): rc=$? / tempo"; continue; }
  [ "$out" = "$(cat "$exp")" ] || { fail "$name (nativo): saida"; diff <(echo "$out") "$exp" | head; continue; }
  echo "ok: $name (nativo)"
  [ "$name" = str_char_at_1mb ] && continue   # interpretador e lento demais para 1 MiB
  out="$(OFSCC_MODE=run OFSCC_INPUT="$src" timeout 20 "$OFSCC" 2>/dev/null)" || { fail "$name (interp): rc=$?"; continue; }
  [ "$out" = "$(cat "$exp")" ] || { fail "$name (interp): saida"; diff <(echo "$out") "$exp" | head; continue; }
  echo "ok: $name (interp)"
done

[ "$FAILED" -eq 0 ] && echo "regress: tudo ok" || { echo "regress: $FAILED falha(s)"; exit 1; }
