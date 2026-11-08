#!/usr/bin/env bash
# Teste do `ofs doc`: o extrator (nomes, assinaturas, comentarios, chama/usado por)
# contra expected.txt e o codigo de saida do --check (CI).
#   OFS_BIN=<wrapper ofs>  (padrao: src/packaging/linux/ofs)
set -uo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../../.." && pwd)"
OFS="${OFS_BIN:-$ROOT/src/packaging/linux/ofs}"
export NO_COLOR=1 OFS_PROGRESS=plain
FAILED=0
fail() { echo "FAIL: $*"; FAILED=$((FAILED + 1)); }

out="$(cd "$HERE" && bash "$OFS" doc --dump sample.ofs 2>/dev/null)"
if [ "$out" = "$(cat "$HERE/expected.txt")" ]; then echo "ok: extrator"; else
  fail "extrator: saida diferente de expected.txt"; diff <(echo "$out") "$HERE/expected.txt" | head -20; fi

( cd "$HERE" && bash "$OFS" doc --check sample.ofs --docs docs_ok >/dev/null 2>&1 ) \
  && echo "ok: --check sem funcao inexistente (exit 0)" || fail "--check docs_ok devia sair com 0"
( cd "$HERE" && bash "$OFS" doc --check sample.ofs --docs docs_bad >/tmp/ofsdoc_test_bad.txt 2>&1 ) \
  && fail "--check docs_bad devia sair com 1" || echo "ok: --check com funcao inexistente (exit 1)"
grep -q "shapes.nope" /tmp/ofsdoc_test_bad.txt && grep -q "ghost_fn" /tmp/ofsdoc_test_bad.txt \
  && echo "ok: --check lista shapes.nope e ghost_fn" || fail "--check nao listou as funcoes inexistentes"

T="$(mktemp -d "${TMPDIR:-/tmp}/ofsdoc-test-XXXXXX")"; trap 'rm -rf "$T"' EXIT
( cd "$HERE" && bash "$OFS" doc sample.ofs -o "$T/out" --docs docs_ok >/dev/null 2>&1 )
for f in index.html sample.html cobertura.html search.json ofsdoc.css ofsdoc.js; do
  [ -s "$T/out/$f" ] || fail "saida sem $f"
done
grep -q 'id="shapes.area"' "$T/out/sample.html" && grep -q 'abi c' "$T/out/sample.html" \
  && echo "ok: HTML gerado (ancoras e selo abi c)" || fail "HTML sem ancora/selo esperado"
[ "$FAILED" -eq 0 ] && echo "ofsdoc: tudo ok" || { echo "ofsdoc: $FAILED falha(s)"; exit 1; }
