#!/usr/bin/env bash
# Teste do `ofs quarry` com um registro local temporario (sem rede).
# HOME, OFS_LIB_HOME e XDG_CACHE_HOME temporarios: nao toca no ~/.ofs real.
# O binario da ferramenta fica no cache real de ferramentas (TOOLS_CACHE) para
# nao recompilar a cada execucao.
#   OFS_BIN=<wrapper ofs>  (padrao: src/packaging/linux/ofs)
set -uo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../../.." && pwd)"
OFS="${OFS_BIN:-$ROOT/src/packaging/linux/ofs}"
export NO_COLOR=1 OFS_PROGRESS=plain OFS_LANG=pt
FAILED=0
fail() { echo "FAIL: $*"; FAILED=$((FAILED + 1)); }
ok() { echo "ok: $*"; }

T="$(mktemp -d "${TMPDIR:-/tmp}/quarry-test-XXXXXX")"; trap 'rm -rf "$T"' EXIT
REAL_CACHE="${XDG_CACHE_HOME:-$HOME/.cache}"
mkdir -p "$T/home" "$T/reg/dist" "$T/src" "$T/app" "$REAL_CACHE/ofs/tools"
export HOME="$T/home" OFS_LIB_HOME="$T/home/.ofs/libs"
# ferramenta compilada no cache real; downloads no cache temporario
q() { XDG_CACHE_HOME="$REAL_CACHE" QUARRY_CACHE="$T/home/.cache/ofs/quarry" bash "$OFS" quarry "$@"; }
export OFS_QUARRY_REGISTRY="$T/reg/registry.json"
REG="$T/reg/registry.json"

# --- pacotes de exemplo: basalto 1.0.0/1.1.0/2.0.0, granito 1.0.0 (basalto ^1.0), xisto 1.0.0 (basalto ^2.0)
mkpkg() { # nome versao corpo [deps...]
  local n="$1" v="$2" body="$3"; shift 3
  local d="$T/src/$n-$v"; mkdir -p "$d/libs"
  { echo "name = $n"; echo "version = $v"; echo "description = pacote de teste $n"; echo "main = libs/$n.ofs"
    echo "tags = teste, rocha"; echo; echo "[deps]"; for x in "$@"; do echo "$x"; done; } > "$d/ofs.claim"
  printf '%s\n' "$body" > "$d/libs/$n.ofs"
  ( cd "$d" && q publish --out "$T/reg/dist" --deposit "$REG" ) > "$T/pub.log" 2>&1 || { cat "$T/pub.log"; fail "publish $n $v"; }
}
mkpkg basalto 1.0.0 'vein basalto_v() -> obsidian { return "basalto 1.0.0" }'
mkpkg basalto 1.1.0 'vein basalto_v() -> obsidian { return "basalto 1.1.0" }'
mkpkg basalto 2.0.0 'vein basalto_v() -> obsidian { return "basalto 2.0.0" }'
mkpkg granito 1.0.0 'attach {basalto}
vein granito_v() -> obsidian { return "granito 1.0.0 sobre " + basalto_v() }' 'basalto = ^1.0'
mkpkg xisto 1.0.0 'vein xisto_v() -> obsidian { return "xisto" }' 'basalto = ^2.0'
[ -f "$T/reg/dist/granito-1.0.0.tar.gz" ] && grep -q '"sha256"' "$REG" && ok "publish gera .tar.gz e deposita no registro" || fail "publish/deposit"
grep -q '"basalto": "^1.0"' "$REG" && ok "publish leva [deps] para o registro" || fail "deps no registro"
# reprodutivel: publicar de novo da o mesmo sha256
h1="$(sha256sum "$T/reg/dist/basalto-1.0.0.tar.gz" | cut -d' ' -f1)"
( cd "$T/src/basalto-1.0.0" && q publish --out "$T/reg/dist" ) >/dev/null 2>&1
h2="$(sha256sum "$T/reg/dist/basalto-1.0.0.tar.gz" | cut -d' ' -f1)"
[ "$h1" = "$h2" ] && ok "tar.gz reprodutivel (mesmo sha256)" || fail "tar.gz nao reprodutivel"

# para a parte de update: o registro so tem basalto ate 1.0.0 no inicio
cp "$REG" "$T/reg/full.json"
python3 -I - "$REG" <<'EOF' 2>/dev/null || sed -i 's/"1.1.0": {[^}]*},\?//' "$REG"
import json,sys
p=sys.argv[1]; d=json.load(open(p))
for k in d["packages"]:
    if k["name"]=="basalto":
        del k["versions"]["1.1.0"]
        k["channels"]={"stable":"2.0.0","latest":"2.0.0"}
json.dump(d,open(p,"w"),indent=2)
EOF

cd "$T/app"
out="$(q search rocha 2>&1)"
echo "$out" | grep -q "granito" && echo "$out" | grep -q "xisto" && ok "search acha por tag" || { echo "$out"; fail "search"; }
q search nada-disso 2>&1 | grep -q "nada encontrado" && ok "search sem resultado" || fail "search vazio"

# add com dependencia transitiva
out="$(q add granito 2>&1)"; rc=$?
[ $rc -eq 0 ] && ok "add granito" || { echo "$out"; fail "add granito (rc=$rc)"; }
grep -q '^granito = \^1.0.0' ofs.claim && ok "ofs.claim criado com granito ^1.0.0" || { cat ofs.claim; fail "ofs.claim"; }
grep -q '^\[basalto\]' ofs.lode && grep -A1 '^\[basalto\]' ofs.lode | grep -q 'version = 1.0.0' \
  && ok "lock tem a dependencia transitiva basalto 1.0.0" || { cat ofs.lode; fail "transitiva no lock"; }
lsha="$(awk '/^\[granito\]/{f=1} f&&/^sha256/{print $3; exit}' ofs.lode)"
[ "$lsha" = "$(sha256sum "$T/reg/dist/granito-1.0.0.tar.gz" | cut -d' ' -f1)" ] && ok "lock guarda o sha256 do .tar.gz" || fail "sha256 do lock"
[ -f "$OFS_LIB_HOME/granito/libs/granito.ofs" ] && [ -f "$OFS_LIB_HOME/granito.ofs" ] && [ -f "$OFS_LIB_HOME/basalto.ofs" ] \
  && ok "instalado em OFS_LIB_HOME/<pacote>/ + veio <modulo>.ofs" || fail "arquivos instalados"

# programa com attach {granito}: nativo e ofs run, sem configuracao extra
cat > main.ofs <<'EOF'
attach {granito}
core main() {
    echo(granito_v())
}
EOF
( bash "$OFS" build main.ofs -o main ) >"$T/build.log" 2>&1
[ "$(./main 2>&1)" = "granito 1.0.0 sobre basalto 1.0.0" ] && ok "attach {granito} compila e roda (nativo)" || { tail -5 "$T/build.log"; fail "nativo"; }
[ "$(bash "$OFS" run main.ofs 2>/dev/null | tail -1)" = "granito 1.0.0 sobre basalto 1.0.0" ] && ok "attach {granito} roda com ofs run" || fail "ofs run"

out="$(q list 2>&1)"
echo "$out" | grep -q "granito *1.0.0" && echo "$out" | grep -q "basalto *1.0.0 *(transitivo)" && ok "list mostra versoes" || { echo "$out"; fail "list"; }

# conflito: xisto pede basalto ^2.0, granito pede ^1.0
cp ofs.claim "$T/claim.bak"; cp ofs.lode "$T/lode.bak"
out="$(q add xisto 2>&1)"; rc=$?
[ $rc -ne 0 ] && echo "$out" | grep -q "conflito: nenhuma versao de basalto" && ok "conflito detectado" || { echo "$out"; fail "conflito"; }
cmp -s ofs.claim "$T/claim.bak" && cmp -s ofs.lode "$T/lode.bak" && ok "conflito nao altera ofs.claim/ofs.lode" || fail "conflito alterou arquivos"

# update respeita a faixa: aparece basalto 1.1.0 (e 2.0.0, fora de ^1.0)
cp "$T/reg/full.json" "$REG"
out="$(q update 2>&1)"
echo "$out" | grep -q "basalto 1.0.0 -> 1.1.0" && ok "update sobe basalto para 1.1.0 (nao 2.0.0)" || { echo "$out"; fail "update"; }
grep -A1 '^\[basalto\]' ofs.lode | grep -q 'version = 1.1.0' && ok "lock atualizado" || fail "lock apos update"
( bash "$OFS" build main.ofs -o main ) >/dev/null 2>&1
[ "$(./main)" = "granito 1.0.0 sobre basalto 1.1.0" ] && ok "programa usa basalto 1.1.0" || fail "programa apos update"

# install reproduz o projeto do lock em uma pedreira vazia
rm -rf "$OFS_LIB_HOME" "$T/home/.cache"
out="$(q install 2>&1)"; rc=$?
[ $rc -eq 0 ] && echo "$out" | grep -q "instalado conforme ofs.lode: 2" && ok "install a partir do lock" || { echo "$out"; fail "install"; }
[ "$(cat "$OFS_LIB_HOME/basalto/.assay" | grep '^version')" = "version = 1.1.0" ] && ok "install pega a versao exata do lock" || fail "versao do install"

# hash adulterado: o .tar.gz muda depois do lock -> install recusa
rm -rf "$OFS_LIB_HOME" "$T/home/.cache"
cp "$T/reg/dist/granito-1.0.0.tar.gz" "$T/granito.bak"
( cd "$T/src/granito-1.0.0" && echo '// adulterado' >> libs/granito.ofs && q publish --out "$T/reg/dist" ) >/dev/null 2>&1
out="$(q install 2>&1)"; rc=$?
[ $rc -ne 0 ] && echo "$out" | grep -q "sha256 nao confere para granito@1.0.0" && ok "hash adulterado recusado" || { echo "$out"; fail "hash adulterado"; }
[ ! -d "$OFS_LIB_HOME/granito" ] && ok "pacote adulterado nao foi instalado" || fail "adulterado instalado"
cp "$T/granito.bak" "$T/reg/dist/granito-1.0.0.tar.gz"
q install >/dev/null 2>&1 && ok "install volta a funcionar com o arquivo original" || fail "install apos restaurar"

# remove: tira granito e o basalto orfao
out="$(q remove granito 2>&1)"
! grep -q '^granito' ofs.claim && ! grep -q '^\[basalto\]' ofs.lode && [ ! -e "$OFS_LIB_HOME/granito" ] && [ ! -e "$OFS_LIB_HOME/basalto.ofs" ] \
  && ok "remove apaga pacote e dependencia orfa" || { echo "$out"; cat ofs.lode; fail "remove"; }
q list 2>&1 | grep -q "nenhum pacote instalado" && ok "list vazio depois do remove" || fail "list vazio"

# canal e versao exata
q add basalto@1.0.0 >/dev/null 2>&1 && grep -q '^basalto = 1.0.0' ofs.claim && grep -A1 '^\[basalto\]' ofs.lode | grep -q 'version = 1.0.0' \
  && ok "add pacote@versao exata" || fail "add @versao"
q add basalto@stable >/dev/null 2>&1 && grep -q '^basalto = \^2.0.0' ofs.claim && ok "add pacote@canal" || fail "add @canal"
q add inexistente >/dev/null 2>&1 && fail "add de pacote inexistente devia falhar" || ok "add de pacote inexistente falha"

[ "$FAILED" -eq 0 ] && echo "quarry: tudo ok" || { echo "quarry: $FAILED falha(s)"; exit 1; }
