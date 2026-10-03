#!/usr/bin/env bash
# Golden de render: desenha a cena scene.ofs e compara o PPM com o snapshot.
# Deterministico: sqrt IEEE + aritmetica inteira — mesma saida em qualquer host.
set -euo pipefail
cd "$(dirname "$0")/../.."
OFS="${OFS_BIN:-ofs}"
GOLDEN=tests/render/scene.ppm.golden

# o interpretador nao tem fopen; o golden e gerado/verificado via build nativo
render_scene() {
    forge_tmp="$(mktemp -d "${TMPDIR:-/tmp}/ofs-render-XXXXXX")"
    "$OFS" build tests/render/scene.ofs -o "$forge_tmp/scene" >/dev/null
    (cd "$(dirname "$0")/../.." && "$forge_tmp/scene" >/dev/null)
    rm -rf "$forge_tmp"
}

if [ "${OFS_UPDATE_GOLDEN:-0}" = "1" ]; then
    render_scene
    mv tests/render/scene.ppm "$GOLDEN"
    echo "golden atualizado: $GOLDEN"
    exit 0
fi

render_scene
cmp "$GOLDEN" tests/render/scene.ppm || { echo "FAIL: render divergiu do golden"; exit 1; }
rm -f tests/render/scene.ppm
echo "ok: render golden"
