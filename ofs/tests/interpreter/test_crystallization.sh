#!/usr/bin/env bash
# Real execution checks: background compilation, cache, fallback and state.
set -euo pipefail
ulimit -v 3000000
R="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)"
CC="${OFSCC_BIN:-${OFSCC:-$R/ofs/dist/ofscc}}"
T="$(mktemp -d)"
trap 'rm -rf "$T"' EXIT
export OFS_STDLIB_PATH="$R/ofs/stdlib" NO_COLOR=1 OFS_PROGRESS=plain OFS_STRICT_FAULT=1
export OFS_QUARTZ_CACHE="$T/cache" OFS_VERBOSE=1
unset OFS_CRYSTALLIZE_THRESHOLD OFS_QUARTZ_CC
export OFS_CRYSTALLIZE=1
run() { timeout 45 env OFSCC_MODE=run OFSCC_INPUT="$1" "$CC" >"$2.out" 2>"$2.err"; }
# Recursive work continues interpreting while clang runs, then dispatches native.
run "$R/bench/ofs/fib.ofs" "$T/cold"
printf '9227465\n' > "$T/expected"
cmp "$T/expected" "$T/cold.out"
grep -qE 'cristalizacao:.*segundo plano' "$T/cold.err"
run "$R/bench/ofs/fib.ofs" "$T/warm"
cmp "$T/expected" "$T/warm.out"
grep -qE 'cristalizacao:.*cache' "$T/warm.err"
OFS_CRYSTALLIZE=0 run "$R/bench/ofs/fib.ofs" "$T/off"
cmp "$T/expected" "$T/off.out"
if grep -qE 'vein\(s\) nativa' "$T/off.err"; then exit 1; fi
# A missing compiler affects optional acceleration only, even with a cold cache.
OFS_QUARTZ_CACHE="$T/no-cc" OFS_QUARTZ_CC="$T/missing-clang" run "$R/bench/ofs/fib.ofs" "$T/fallback"
cmp "$T/expected" "$T/fallback.out"
grep -qE 'clang nao encontrado' "$T/fallback.err"
if OFS_CRYSTALLIZE_THRESHOLD=0 run "$R/bench/ofs/fib.ofs" "$T/invalid"; then
    echo 'FAIL: zero heat threshold accepted'; exit 1
fi
grep -qE 'OFS_CRYSTALLIZE_THRESHOLD precisa ser inteiro positivo' "$T/invalid.err"
# Float callbacks (onda 4): trampolins __ofs_cbx_K com registradores float;
# a cristalizacao nao e mais abandonada e o resultado e o mesmo.
OFS_CRYSTALLIZE_THRESHOLD=1 run "$R/ofs/tests/interpreter/crystallization_float_callback.ofs" "$T/float-callback"
printf '2000\n' > "$T/float-expected"
cmp "$T/float-expected" "$T/float-callback.out"
if grep -q 'callback de ponto flutuante' "$T/float-callback.err"; then
    echo 'FAIL: float callback still disables crystallization'; exit 1
fi
# Short programs need not wait for the compiler; the worker cleans its temp.
mkdir "$T/jobs"
printf 'core main() { echo(42) }\n' > "$T/short.ofs"
OFS_CRYSTALLIZE_THRESHOLD=1 OFS_QUARTZ_CACHE="$T/short-cache" TMPDIR="$T/jobs" run "$T/short.ofs" "$T/short"
printf '42\n' > "$T/short-expected"
cmp "$T/short-expected" "$T/short.out"
for ((i=0; i<100; i++)); do
    if ! compgen -G "$T/jobs/ofs-quartz-*" >/dev/null; then break; fi
    sleep 0.1
done
if compgen -G "$T/jobs/ofs-quartz-*" >/dev/null; then
    echo 'FAIL: background compiler left temporary files'; exit 1
fi

# Preserve globals and arrays when native code calls back to the interpreter.
if [[ "$(uname -m)" == x86_64 ]]; then
    OFS_CRYSTALLIZE=0 run "$R/ofs/tests/regress/quartz_hybrid_x86.ofs" "$T/state"
    cmp "$R/ofs/tests/regress/quartz_hybrid_x86.expected" "$T/state.out"
    OFS_CRYSTALLIZE_THRESHOLD=1 run "$R/ofs/tests/regress/quartz_hybrid_x86.ofs" "$T/hot-state"
    cmp "$T/state.out" "$T/hot-state.out"
    OFS_CRYSTALLIZE=0 run "$R/ofs/tests/regress/quartz_types_x86.ofs" "$T/veins"
    cmp "$R/ofs/tests/regress/quartz_types_x86.expected" "$T/veins.out"
fi
# OSR (onda 4): lacos longos dentro de uma unica chamada (main) viram
# nativos no meio, com o estado (variaveis de cada tipo, global, monolith,
# Array, texto, break, laco aninhado); OFS_OSR=0 da o mesmo resultado.
OSR="$R/ofs/tests/interpreter/crystallization_osr.ofs"
run "$OSR" "$T/osr"
cmp "$R/ofs/tests/interpreter/crystallization_osr.expected" "$T/osr.out"
grep -qE 'osr \(1 laco\)' "$T/osr.err"
grep -q 'terminou nativo' "$T/osr.err"
OFS_OSR=0 run "$OSR" "$T/osr-off"
cmp "$R/ofs/tests/interpreter/crystallization_osr.expected" "$T/osr-off.out"
if grep -q 'osr' "$T/osr-off.err"; then echo 'FAIL: OFS_OSR=0 still used OSR'; exit 1; fi
OFS_CRYSTALLIZE=0 run "$OSR" "$T/osr-nocr"
cmp "$R/ofs/tests/interpreter/crystallization_osr.expected" "$T/osr-nocr.out"
# o laco de 10^8 voltas do bench (18 s interpretado) fica perto do nativo
start=$(date +%s)
run "$R/bench/ofs/loop.ofs" "$T/osr-loop"
printf '68000000\n' > "$T/loop-expected"
cmp "$T/loop-expected" "$T/osr-loop.out"
if [ $(( $(date +%s) - start )) -gt 8 ]; then echo 'FAIL: OSR nao acelerou bench/ofs/loop.ofs'; exit 1; fi
echo 'crystallization: background, cache, off, fallback, threshold, globals/callbacks, vein identity, OSR OK'
