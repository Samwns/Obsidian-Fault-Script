#!/usr/bin/env bash
# Golden tests do Reporter OFS (Fases 3-5 + CI).
#
# Não fazemos diff byte-a-byte porque `ms` e alguns tamanhos mudam a cada execução.
# Em vez disso, cada caso verifica padrões (`grep -E`) que são invariantes.
set -euo pipefail
cd "$(dirname "$0")/../.."
ROOT="$(pwd)"

# Binário sob teste: pode ser sobrescrito via OFSCC_BIN.
OFSCC_BIN="${OFSCC_BIN:-}"
if [ -z "$OFSCC_BIN" ]; then
    if [ -x /tmp/ofs-logs/ofscc-f5 ]; then
        OFSCC_BIN=/tmp/ofs-logs/ofscc-f5
    elif [ -x "$HOME/.local/bin/ofscc" ]; then
        OFSCC_BIN="$HOME/.local/bin/ofscc"
    elif command -v ofscc >/dev/null 2>&1; then
        OFSCC_BIN="$(command -v ofscc)"
    else
        echo "erro: ofscc não encontrado (defina OFSCC_BIN)" >&2
        exit 2
    fi
fi

HELLO="ofs/examples/hello.ofs"
[ -f "$HELLO" ] || { echo "erro: $HELLO não existe"; exit 2; }

fail() { echo "FAIL: $*" >&2; exit 1; }
ok()   { echo "ok: $*"; }

run() {
    # run <logname> <env...> -- <cmd...>
    local log="$1"; shift
    local envs=()
    while [ "$1" != "--" ]; do envs+=("$1"); shift; done
    shift
    ( ulimit -v 4000000 2>/dev/null || true
      ulimit -t 30      2>/dev/null || true
      env "${envs[@]}" timeout 30 "$@" ) >"$log" 2>&1
}

# ─── Plain ──────────────────────────────────────────────────────────
LOG=/tmp/ofs-golden-plain-hello.log
run "$LOG" NO_COLOR=1 TERM=dumb OFS_PROGRESS=plain OFSCC_MODE=check OFSCC_INPUT="$HELLO" -- "$OFSCC_BIN"
grep -qE '^  Reading ok \([0-9]+\) · [0-9.eE+-]+ ms$'          "$LOG" || fail "plain Reading"
grep -qE '^  Lexing ok \([0-9]+\) · [0-9.eE+-]+ ms$'           "$LOG" || fail "plain Lexing"
grep -qE '^  Parsing ok \([0-9]+\) · [0-9.eE+-]+ ms$'          "$LOG" || fail "plain Parsing"
grep -qE '^  Checking ok · [0-9.eE+-]+ ms$'                    "$LOG" || fail "plain Checking"
grep -qE "^done $(echo "$HELLO" | sed 's/[][\/.^$*]/\\&/g') · 0 B · " "$LOG" || fail "plain done"
# Sem ANSI em plain
if grep -qP '\x1b\[' "$LOG"; then fail "plain contém ANSI"; fi
ok "plain/hello"

# ─── Fancy degrada para Plain com NO_COLOR ──────────────────────────
LOG=/tmp/ofs-golden-fancy-nocolor.log
run "$LOG" NO_COLOR=1 TERM=xterm-256color OFS_PROGRESS=fancy OFSCC_MODE=check OFSCC_INPUT="$HELLO" -- "$OFSCC_BIN"
grep -qE '^  Reading ok \(' "$LOG" || fail "fancy NO_COLOR deve degradar para plain"
if grep -qP '\x1b\[' "$LOG"; then fail "fancy NO_COLOR contém ANSI"; fi
ok "fancy/NO_COLOR→plain"

# ─── Fancy degrada com TERM=dumb ────────────────────────────────────
LOG=/tmp/ofs-golden-fancy-dumb.log
run "$LOG" TERM=dumb OFS_PROGRESS=fancy OFSCC_MODE=check OFSCC_INPUT="$HELLO" -- "$OFSCC_BIN"
grep -qE '^  Reading ok \(' "$LOG" || fail "fancy TERM=dumb deve degradar"
if grep -qP '\x1b\[' "$LOG"; then fail "fancy TERM=dumb contém ANSI"; fi
ok "fancy/TERM=dumb→plain"

# ─── Fancy com ANSI habilitado ──────────────────────────────────────
LOG=/tmp/ofs-golden-fancy-ansi.log
run "$LOG" TERM=xterm-256color OFS_PROGRESS=fancy OFS_ANIM=0 OFSCC_MODE=check OFSCC_INPUT="$HELLO" -- "$OFSCC_BIN"
grep -qP '\x1b\[32m✓\x1b\[0m Reading' "$LOG" || fail "fancy ✓ verde em Reading"
grep -qP '\x1b\[32mdone\x1b\[0m'      "$LOG" || fail "fancy done verde"
ok "fancy/ansi"

# ─── Json: cada linha é JSON válido + campos-chave ──────────────────
LOG=/tmp/ofs-golden-json-hello.log
run "$LOG" OFS_PROGRESS=json OFSCC_MODE=check OFSCC_INPUT="$HELLO" -- "$OFSCC_BIN"
if command -v jq >/dev/null 2>&1; then
    jq -c . <"$LOG" >/dev/null || fail "json/hello não é NDJSON válido"
else
    # Fallback: cada linha começa com { e termina com }
    awk 'NF && ($0 !~ /^{/ || $0 !~ /}$/) { exit 1 }' "$LOG" || fail "json/hello linha não parece JSON"
fi
grep -q '"kind":"stage_start"' "$LOG"         || fail "json stage_start"
grep -q '"locale":"'           "$LOG"         || fail "json locale"
grep -q '"kind":"finished"'    "$LOG"         || fail "json finished"
grep -q '"duration_ms_total":' "$LOG"         || fail "json duration_ms_total"
ok "json/hello"

# ─── Locale pt ──────────────────────────────────────────────────────
LOG=/tmp/ofs-golden-locale-pt.log
run "$LOG" NO_COLOR=1 TERM=dumb OFS_PROGRESS=plain OFS_LANG=pt OFSCC_MODE=check OFSCC_INPUT="$HELLO" -- "$OFSCC_BIN"
grep -qE '^  Lendo ok \('      "$LOG" || fail "pt Lendo"
grep -qE '^  Verificando ok'   "$LOG" || fail "pt Verificando"
ok "locale/pt"

# ─── Locale es ──────────────────────────────────────────────────────
LOG=/tmp/ofs-golden-locale-es.log
run "$LOG" NO_COLOR=1 TERM=dumb OFS_PROGRESS=plain OFS_LANG=es OFSCC_MODE=check OFSCC_INPUT="$HELLO" -- "$OFSCC_BIN"
grep -qE '^  Leyendo ok \('    "$LOG" || fail "es Leyendo"
ok "locale/es"

# ─── Locale fallback en ─────────────────────────────────────────────
LOG=/tmp/ofs-golden-locale-en.log
run "$LOG" NO_COLOR=1 TERM=dumb OFS_PROGRESS=plain OFS_LANG=xx OFSCC_MODE=check OFSCC_INPUT="$HELLO" -- "$OFSCC_BIN"
grep -qE '^  Reading ok \('    "$LOG" || fail "en fallback Reading"
ok "locale/en_fallback"

# ─── twin stub emite NDJSON no formato Fase 5 ──────────────────────
LOG=/tmp/ofs-golden-json-twin.log
run "$LOG" OFS_PROGRESS=json OFSCC_MODE=check OFSCC_INPUT=twin -- "$OFSCC_BIN" || true
grep -q '"kind":"twin_start"'            "$LOG" || fail "json twin_start"
grep -q '"kind":"twin_result"'           "$LOG" || fail "json twin_result"
grep -q '"msg_key":"stub_command"'       "$LOG" || fail "json msg_key stub_command"
ok "json/twin_stub"

# ─── Atualizar golden files (modo OFS_UPDATE_GOLDEN=1) ──────────────
if [ "${OFS_UPDATE_GOLDEN:-0}" = "1" ]; then
    echo "== atualizando snapshots documentais em tests/golden/ =="
    cp /tmp/ofs-golden-plain-hello.log      tests/golden/plain/hello.txt.current
    cp /tmp/ofs-golden-fancy-ansi.log       tests/golden/fancy/hello_ansi.txt.current
    cp /tmp/ofs-golden-json-hello.log       tests/golden/json/hello.ndjson.current
    cp /tmp/ofs-golden-locale-pt.log        tests/golden/locale/pt_check.txt.current
    cp /tmp/ofs-golden-locale-es.log        tests/golden/locale/es_check.txt.current
    cp /tmp/ofs-golden-locale-en.log        tests/golden/locale/en_fallback.txt.current
    cp /tmp/ofs-golden-json-twin.log        tests/golden/json/twin_ok.ndjson.current
    echo "→ arquivos .current criados; revise e renomeie antes de commit"
fi

echo "all golden checks passed"
