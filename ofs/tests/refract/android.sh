#!/usr/bin/env bash
# Fault Refract L3 no aparelho: `ofs refract --remote` com o exemplo
# ofs/examples/refract_android (copiado para uma pasta temporaria). Instala e
# abre o app, edita painel.ofs por script e confere pelo logcat (tag oal) e
# pela saida do `ofs refract --remote` que cada troca entrou sem reinstalar
# nem reiniciar o processo (mesmo PID: `adb shell pidof`) e que o estado (a
# contagem, um global do app) continuou. Casos: vein alterada; vein nova (sem
# celula) chamada pela alterada; a vein nova editada de novo (remendo
# seguinte, quem a chama e religado); assinatura mudada (recusa com
# mensagem, o app segue); erro de compilacao (o app segue).
# Sem aparelho (ou sem SDK/NDK) o teste pula com aviso (rc 0).
#   OFSCC_BIN=<ofscc>        (padrao: o do wrapper)
#   ANDROID_SDK=<sdk>        (ou ANDROID_HOME/ANDROID_SDK_ROOT; padrao ~/Android/Sdk)
#   ANDROID_SERIAL=<serial>  (aparelho; padrao: o unico conectado)
#   REFRACT_KEEP=1           mantem a pasta temporaria e o app instalado
# Memoria: ulimit -v e timeout em tudo. Ao final desinstala so o pacote de
# teste (org.ofs.refract.demo).
set -uo pipefail
ulimit -v 4194304
R="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)"
WRAP="$R/src/packaging/linux/ofs"
PKG="org.ofs.refract.demo"
skip() { echo "refract android: PULADO: $*"; exit 0; }

ADB="$(command -v adb || true)"
SDK="${ANDROID_SDK:-${ANDROID_HOME:-${ANDROID_SDK_ROOT:-$HOME/Android/Sdk}}}"
[ -n "$ADB" ] || { [ -x "$SDK/platform-tools/adb" ] && ADB="$SDK/platform-tools/adb"; }
[ -n "$ADB" ] || skip "adb nao encontrado"
[ -d "$SDK/ndk" ] || skip "SDK/NDK nao encontrado em $SDK (defina ANDROID_SDK)"
SERIAL="${ANDROID_SERIAL:-}"
if [ -z "$SERIAL" ]; then
    SERIAL="$(timeout 10 "$ADB" devices | awk 'NR > 1 && $2 == "device" { print $1; exit }')"
fi
[ -n "$SERIAL" ] || skip "nenhum aparelho conectado (adb devices)"
timeout 10 "$ADB" -s "$SERIAL" get-state 2>/dev/null | grep -q device || skip "aparelho $SERIAL indisponivel"
A=("$ADB" -s "$SERIAL")

T="$(mktemp -d "${TMPDIR:-/tmp}/ofs-refract-android-XXXXXX")"
RPID=""
PORT="${OFS_REFRACT_PORT:-7341}"
cleanup() {
    [ -n "$RPID" ] && kill "$RPID" 2>/dev/null
    wait 2>/dev/null
    timeout 10 "${A[@]}" forward --remove "tcp:$PORT" >/dev/null 2>&1
    if [ -n "${REFRACT_KEEP:-}" ]; then
        echo "refract android: arquivos em $T (app instalado)"
    else
        timeout 30 "${A[@]}" uninstall "$PKG" >/dev/null 2>&1
        rm -rf "$T"
    fi
}
trap cleanup EXIT
export ANDROID_SDK="$SDK" XDG_CACHE_HOME="$T/cache" OFS_SOURCE="$R/ofs" NO_COLOR=1 OFS_PROGRESS=plain
export OFS_STDLIB_PATH="$R/ofs/stdlib" OFS_REFRACT_PORT="$PORT"
[ -n "${OFSCC_BIN:-}" ] && export OFSCC="$OFSCC_BIN"
PASS=0
FAIL=0
ok() { echo "ok: $*"; PASS=$((PASS + 1)); }
falha() { echo "FAIL: $*"; FAIL=$((FAIL + 1)); }

cp -r "$R/ofs/examples/refract_android" "$T/app"
APP="$T/app"
LOG="$T/remote.log"
LC="$T/logcat.txt"

# espera_log <arquivo> <regex> <segundos>
espera_log() {
    local i=0
    while [ "$i" -lt "$(($3 * 4))" ]; do
        grep -Eq "$2" "$1" 2>/dev/null && return 0
        sleep 0.25
        i=$((i + 1))
    done
    return 1
}
# logcat da tag oal (o app escreve "painel: <texto>" a cada segundo)
# (so do processo do teste: a reinstalacao mata o anterior, que ainda pode
# ter linhas no buffer)
logcat_dump() { timeout 10 "${A[@]}" logcat -d --pid="$P1" -s oal ofs-refract > "$LC" 2>/dev/null; }
# espera uma linha "painel: <prefixo>N" e devolve N (a maior vista)
espera_painel() {
    local pre="$1" i=0 n
    while [ "$i" -lt 40 ]; do
        logcat_dump
        n="$(sed -n "s/.*painel: ${pre}\([0-9][0-9]*\)\$/\1/p" "$LC" | tail -1)"
        if [ -n "$n" ]; then echo "$n"; return 0; fi
        sleep 0.5
        i=$((i + 1))
    done
    return 1
}
pid_app() { timeout 10 "${A[@]}" shell pidof "$PKG" 2>/dev/null | tr -d '\r'; }
edita() { python3 -I - "$APP/painel.ofs" "$1" "$2" <<'EOF'
import sys
p, a, b = sys.argv[1], sys.argv[2], sys.argv[3]
s = open(p).read()
if a not in s:
    sys.exit("trecho nao encontrado: " + a)
open(p, "w").write(s.replace(a, b, 1))
EOF
}

timeout 30 "${A[@]}" logcat -c
echo "refract android: aparelho $SERIAL ($(timeout 10 "${A[@]}" shell getprop ro.product.model | tr -d '\r'), Android $(timeout 10 "${A[@]}" shell getprop ro.build.version.release | tr -d '\r'))"
(cd "$APP" && exec timeout 900 bash "$WRAP" refract --remote refract_android.oal --serial "$SERIAL") > "$LOG" 2>&1 &
RPID=$!
if ! espera_log "$LOG" "salve um arquivo" 300; then
    cat "$LOG"
    falha "o ofs refract --remote nao conectou ao app"
    exit 1
fi
ok "build --refract, instalado, aberto e conectado ($(grep -o 'pid [0-9]*, geracao 0' "$LOG"))"
P1="$(pid_app)"
N0="$(espera_painel "contagem ")" || { falha "o app nao escreveu a contagem no logcat"; exit 1; }
echo "    pid $P1, contagem $N0 antes da primeira troca"

# 1. vein alterada (painel.texto)
edita 'return "contagem " + ofs_stone_to_obsidian(contagem)' 'return "refratado: " + ofs_stone_to_obsidian(contagem)' || exit 1
if espera_log "$LOG" "geracao 1 aplicada" 60; then
    N1="$(espera_painel "refratado: ")"
    if [ -n "$N1" ] && [ "$N1" -gt "$N0" ]; then ok "1: painel.texto trocada (contagem $N0 -> $N1: o estado continuou)"; else falha "1: texto novo ausente ou contagem reiniciada ($N0 -> ${N1:-?})"; fi
else
    falha "1: troca nao aplicada"; tail -5 "$LOG"
fi

# 2. vein nova (sem celula no app) chamada pela alterada
edita '    vein texto() -> obsidian {' '    vein rotulo() -> obsidian { return "camada 2: " }

    vein texto() -> obsidian {' || exit 1
edita 'return "refratado: " + ofs_stone_to_obsidian(contagem)' 'return rotulo() + ofs_stone_to_obsidian(contagem)' || exit 1
if espera_log "$LOG" "geracao 2 aplicada" 60 && grep -q "novas: painel.rotulo" "$LOG"; then
    N2="$(espera_painel "camada 2: ")"
    if [ -n "$N2" ] && [ "$N2" -gt "${N1:-0}" ]; then ok "2: vein nova painel.rotulo (contagem $N2)"; else falha "2: texto da vein nova ausente"; fi
else
    falha "2: remendo com vein nova nao aplicado"; tail -5 "$LOG"
fi

# 3. a vein nova editada de novo: entra outra vez e quem a chama e religado
edita 'return "camada 2: "' 'return "camada 3: "' || exit 1
if espera_log "$LOG" "geracao 3 aplicada" 60 && grep -q "religadas.*painel.texto" "$LOG"; then
    N3="$(espera_painel "camada 3: ")"
    if [ -n "$N3" ] && [ "$N3" -gt "${N2:-0}" ]; then ok "3: vein sem celula redefinida, painel.texto religada (contagem $N3)"; else falha "3: texto novo ausente"; fi
else
    falha "3: segundo remendo da vein nova nao aplicado"; tail -5 "$LOG"
fi

# 4. assinatura mudada: recusa com mensagem, o app segue com a versao anterior
edita '    vein somar() -> void { contagem = contagem + 10 }' '    vein somar() -> stone {
        contagem = contagem + 10
        return contagem
    }' || exit 1
if espera_log "$LOG" "recusada: a assinatura de 'painel.somar' mudou" 60; then
    ok "4: assinatura nova recusada com mensagem"
else
    falha "4: assinatura nova nao foi recusada"; tail -5 "$LOG"
fi
edita '    vein somar() -> stone {
        contagem = contagem + 10
        return contagem
    }' '    vein somar() -> void { contagem = contagem + 10 }' || exit 1
sleep 2

# 5. erro de compilacao: nada e enviado, o app segue
edita 'return "camada 3: "' 'return "camada 3: " +' || exit 1
if espera_log "$LOG" "o programa nao compila" 60; then ok "5: erro de compilacao mostrado, nada enviado"; else falha "5: erro de compilacao nao mostrado"; tail -5 "$LOG"; fi

P2="$(pid_app)"
N4="$(espera_painel "camada 3: ")"
if [ -n "$P1" ] && [ "$P1" = "$P2" ]; then ok "mesmo processo do inicio ao fim (pid $P1 -> $P2), sem reinstalar"; else falha "o processo mudou (pid $P1 -> $P2)"; fi
if [ -n "$N4" ] && [ "$N4" -gt "${N3:-0}" ]; then ok "o app segue vivo depois da recusa e do erro (contagem $N4)"; else falha "o app parou de contar"; fi
if timeout 10 "${A[@]}" logcat -d -b crash 2>/dev/null | grep -q "$PKG"; then falha "o app caiu (logcat -b crash)"; fi
grep "^refract: " "$LOG" | sed 's/^/    /'

echo "refract android: $PASS ok, $FAIL falha(s)"
[ "$FAIL" -eq 0 ]
