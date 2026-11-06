#!/usr/bin/env bash
# Teste do oalc: as formas `use "x.ofs"`, `attach {F:x.ofs}` e `attach {x}`
# geram o mesmo app (mesmo manifesto e mesmo OFS, a menos da linha de
# attach/comentario) e o OFS gerado compila (alvo desktop e android).
#   OALC_BIN=<oalc>  OFSCC_BIN=<ofscc com OFSCC_TARGET>   (padrao: cache da CLI)
set -euo pipefail
cd "$(dirname "$0")"
HERE="$(pwd)"
export OFS_STDLIB_PATH="$(cd ../../stdlib && pwd)" NO_COLOR=1 OFS_PROGRESS=plain
CACHE="${XDG_CACHE_HOME:-$HOME/.cache}/ofs/android"
OALC="${OALC_BIN:-$CACHE/oalc}"
OFSCC="${OFSCC_BIN:-$CACHE/ofscc}"
[ -x "$OALC" ] || { echo "oalc nao encontrado ($OALC); rode 'ofs run app.oal' uma vez ou defina OALC_BIN"; exit 2; }
T="$(mktemp -d "${TMPDIR:-/tmp}/ofs-oal-test-XXXXXX")"
trap 'rm -rf "$T"' EXIT
fail() { echo "FAIL: $*"; exit 1; }
for form in use_form attach_form attach_short; do
  for tgt in desktop android; do
    mkdir -p "$T/$form-$tgt"
    OALC_INPUT="$HERE/$form.oal" OALC_OUT_DIR="$T/$form-$tgt" OALC_TARGET=$tgt "$OALC" >/dev/null || fail "oalc $form $tgt"
    g="$T/$form-$tgt/${form}_oal.ofs"
    grep -qF "attach {F:$HERE/logica.ofs}" "$g" || fail "$form: attach da logica ausente"
    trip=""; [ "$tgt" = android ] && trip=android
    OFSCC_TARGET=$trip OFSCC_INPUT="$g" OFSCC_MODE=ir OFSCC_C_OUT="$T/$form-$tgt.ll" "$OFSCC" >"$T/$form-$tgt.log" 2>&1 || { tail "$T/$form-$tgt.log"; fail "$form $tgt nao compila"; }
    grep -q "AVISO SEMANTICO\|ERRO SEMANTICO" "$T/$form-$tgt.log" && { grep "SEMANTICO" "$T/$form-$tgt.log"; fail "$form $tgt: aviso semantico"; }
    echo "ok: $form ($tgt)"
  done
  cmp -s "$T/use_form-android/AndroidManifest.xml" "$T/$form-android/AndroidManifest.xml" || fail "$form: manifesto diferente"
  diff <(grep -v "^// \|^attach" "$T/use_form-desktop/use_form_oal.ofs") <(grep -v "^// \|^attach" "$T/$form-desktop/${form}_oal.ofs") >/dev/null || fail "$form: OFS gerado diferente"
done
m="$T/use_form-android/AndroidManifest.xml"
grep -q 'package="org.ofs.teste_oal"' "$m" || fail "package"
grep -q 'android:versionCode="10203"' "$m" || fail "versionCode"
grep -q 'android.permission.INTERNET' "$m" || fail "INTERNET"
grep -q 'android.permission.VIBRATE' "$m" || fail "VIBRATE"
grep -q 'android:value="teste_oal"' "$m" || fail "lib_name"
g="$T/use_form-desktop/use_form_oal.ofs"
grep -q 'oal.style_def(5, 16746496, 0, 8, 11162880, -1)' "$g" || fail "style"
grep -q 'dados.itens_nome(' "$g" || fail "from"
grep -q 'oal.go(1)' "$g" || fail "@tela"
grep -q 'oal.quit()' "$g" || fail "@quit"
grep -q 'core main()' "$g" || fail "main desktop"
grep -q 'oal.use_icon("settings")' "$g" || fail "button.icon"
grep -q 'oal.use_icon("save")' "$g" || fail "icon="
grep -q 'oal.style_opacity(6, 40)' "$g" || fail "opacity"
grep -q 'oal.fab("", ' "$g" || fail "fab"
grep -q 'oal.dpad(.*, oal.BOTTOM_LEFT)' "$g" || fail "dpad"
grep -q 'oal.set_orientation(oal.ORIENT_LANDSCAPE)' "$g" || fail "@landscape"
# erro: widget desconhecido
printf 'screen x\n  bolinha "a"\n' > "$T/bad.oal"
if OALC_INPUT="$T/bad.oal" OALC_OUT_DIR="$T" "$OALC" > "$T/bad.log" 2>&1; then fail "oalc aceitou widget desconhecido"; fi
grep -q "linha 2: widget desconhecido" "$T/bad.log" || fail "mensagem de erro"
echo "ok: oalc (use, attach {F:}, attach {mod}, manifesto, estilos, icones, fab, dpad, erros)"
