#!/usr/bin/env bash
# Teste do `ofs fmt`.
#   (a) idempotencia: formatar duas vezes da o mesmo arquivo;
#   (b) o programa nao muda: o IR (.ll) que o ofscc gera do original e do
#       formatado e identico (o IR nao leva numero de linha/coluna; se o
#       original nao compila sozinho, compara-se a sequencia de tokens do ofscc,
#       OFSCC_MODE=tokens, sem comentarios/espacos);
#   (c) os casos de cases/ (comentarios, linhas em branco, strings com { e //,
#       codigo mal recuado) contra os .expected;
#   (d) CLI: --check (exit 1 / 0 e lista), --diff, --stdin, pasta recursiva.
# Variaveis: OFS_BIN (wrapper), OFSCC (compilador), OFS_RUNTIME (magma.o).
# Memoria limitada a 3 GB e cada comando com timeout.
set -uo pipefail
ulimit -v 3000000
HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../../.." && pwd)"
OFS="${OFS_BIN:-$ROOT/src/packaging/linux/ofs}"
export OFSCC="${OFSCC:-$ROOT/ofs/dist/ofscc}"
export OFS_RUNTIME="${OFS_RUNTIME:-$ROOT/ofs/dist/magma.o}"
export OFS_SOURCE="${OFS_SOURCE:-$ROOT/ofs}"
export NO_COLOR=1 OFS_PROGRESS=plain
T="$(mktemp -d "${TMPDIR:-/tmp}/ofsfmt-test-XXXXXX")"; trap 'rm -rf "$T"' EXIT
export XDG_CACHE_HOME="${XDG_CACHE_HOME:-$T/cache}"
FAILED=0
fail() { echo "FAIL: $*"; FAILED=$((FAILED + 1)); }
fmt() { timeout 120 bash "$OFS" fmt "$@"; }
ir() { # ir <arquivo.ofs> <saida.ll>: 0 se compilou
  ( cd "$(dirname "$1")" && OFSCC_INPUT="$1" OFSCC_MODE=ir OFSCC_C_OUT="$2" timeout 120 "$OFSCC" >/dev/null 2>&1 )
}

fmt --stdin </dev/null >/dev/null 2>&1   # compila a ferramenta uma vez

for src in "$HERE"/cases/*.ofs; do
  n="$(basename "$src" .ofs)"
  fmt --stdin <"$src" >"$T/$n.1" 2>/dev/null || { fail "$n: ofs fmt --stdin falhou"; continue; }
  fmt --stdin <"$T/$n.1" >"$T/$n.2" 2>/dev/null
  cmp -s "$T/$n.1" "$T/$n.2" && echo "ok: $n idempotente" || { fail "$n: nao idempotente"; diff "$T/$n.1" "$T/$n.2" | head; }
  if [ -f "$HERE/cases/$n.expected" ]; then
    cmp -s "$T/$n.1" "$HERE/cases/$n.expected" && echo "ok: $n == .expected" \
      || { fail "$n: saida difere de $n.expected"; diff "$T/$n.1" "$HERE/cases/$n.expected" | head -20; }
  fi
  [ -z "$(tail -c1 "$T/$n.1")" ] && ! grep -q ' $' "$T/$n.1" && echo "ok: $n sem espaco no fim e termina com newline" || fail "$n: espaco no fim / sem newline final"
  mkdir -p "$T/ir/o" "$T/ir/f"; cp "$src" "$T/ir/o/$n.ofs"; cp "$T/$n.1" "$T/ir/f/$n.ofs"
  if ir "$T/ir/o/$n.ofs" "$T/ir/o/$n.ll"; then
    if ir "$T/ir/f/$n.ofs" "$T/ir/f/$n.ll" && cmp -s "$T/ir/o/$n.ll" "$T/ir/f/$n.ll"; then echo "ok: $n IR identico"; else fail "$n: IR mudou depois de formatar"; fi
  else
    # nao compila sozinho (attach de arquivo ausente): compara a sequencia de tokens do ofscc
    for d in o f; do
      grep -av '^attach ' "$T/ir/$d/$n.ofs" >"$T/ir/$d.tok.ofs"
      ( cd "$T/ir" && OFSCC_INPUT="$T/ir/$d.tok.ofs" OFSCC_MODE=tokens timeout 60 "$OFSCC" 2>&1 | tr -d '\033' >"$T/ir/$d.tok" )
    done
    [ -s "$T/ir/o.tok" ] && cmp -s "$T/ir/o.tok" "$T/ir/f.tok" && echo "ok: $n tokens identicos (nao compila sozinho)" || fail "$n: tokens mudaram"
  fi
done

# programa que nao muda: strings e comentarios continuam byte a byte
fmt --stdin <"$HERE/cases/strings_literais.ofs" | grep -qF 'forge a = "chave { aberta // nao e comentario /* nem bloco */"' \
  && echo "ok: string com { // /* intocada" || fail "string alterada"
fmt --stdin <"$HERE/cases/comentarios_e_linhas.ofs" | grep -qF '    forge a = 1   // comentario no fim' \
  && echo "ok: comentario no fim da linha preservado" || fail "comentario no fim alterado"

# CLI
mkdir -p "$T/proj/sub"; cp "$HERE/cases/basico_mal_recuado.ofs" "$T/proj/sub/a.ofs"
printf 'forge  y:stone=1\n' >"$T/proj/b.ofs"; echo "nao e ofs" >"$T/proj/c.txt"
out="$(fmt --check "$T/proj" 2>/dev/null)"; rc=$?
[ "$rc" = 1 ] && echo "$out" | grep -q 'a.ofs' && echo "$out" | grep -q 'b.ofs' && ! echo "$out" | grep -q 'c.txt' \
  && echo "ok: --check sai com 1 e lista os arquivos" || fail "--check (rc=$rc): $out"
fmt --diff "$T/proj/b.ofs" 2>/dev/null | grep -q '^+forge y: stone = 1' && echo "ok: --diff" || fail "--diff"
cmp -s <(echo 'forge  y:stone=1') "$T/proj/b.ofs" || fail "--diff nao pode escrever"
fmt "$T/proj" >/dev/null 2>&1
[ "$(cat "$T/proj/b.ofs")" = "forge y: stone = 1" ] && echo "ok: pasta reescrita no lugar" || fail "pasta nao reescrita"
[ "$(cat "$T/proj/c.txt")" = "nao e ofs" ] && echo "ok: so .ofs" || fail "tocou em arquivo que nao e .ofs"
fmt --check "$T/proj" >/dev/null 2>&1 && echo "ok: --check passa depois de formatar (exit 0)" || fail "--check depois de formatar devia sair com 0"

echo
[ "$FAILED" = 0 ] && echo "ofs fmt: todos os testes passaram" || { echo "ofs fmt: $FAILED falha(s)"; exit 1; }
