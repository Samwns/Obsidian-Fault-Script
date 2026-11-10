#!/usr/bin/env bash
# Paridade interpretador x nativo (fase F0 de plans/INTERPRETADOR_PARIDADE.md).
# Roda cada programa com `core main` nos dois modos e compara stdout, stderr
# normalizado, codigo de saida e arquivos gerados no cwd. Gera MATRIZ.md.
#   OFSCC_BIN=<ofscc>   (padrao: ofs/dist/ofscc)
#   MAGMA_O=<magma.o>   (padrao: ofs/dist/magma.o)
#   PARITY_FILTER=<regex> so roda caminhos que casem
#   PARITY_TIMEOUT=<s>  limite por execucao (padrao 20)
#   PARITY_STRICT=1     sai com 1 se algo nao for `igual`/`pulado` (padrao: informativo, sai 0)
# Memoria: ulimit -v 2 GB em todo ofscc/binario executado.
set -uo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
OFSCC="${OFSCC_BIN:-$ROOT/ofs/dist/ofscc}"
MAGMA="${MAGMA_O:-$ROOT/ofs/dist/magma.o}"
OFSCC_SRC="padrão"; [ -n "${OFSCC_BIN:-}" ] && OFSCC_SRC="OFSCC_BIN"
MAGMA_SRC="padrão"; [ -n "${MAGMA_O:-}" ] && MAGMA_SRC="MAGMA_O"
TMO="${PARITY_TIMEOUT:-20}"
# Erro esperado: estes programas devem FALHAR (rc != 0) nos dois modos.
# Criterio: estar nesta lista (caminho relativo a raiz do repo). Se so um modo
# falhar, e diferenca real (classe `difere`).
EXPECTED_ERR=(
  ofs/tests/regress/seal_return_inside.ofs
  ofs/tests/regress/undefined_ident.ofs
  ofs/tests/regress/attach_missing.ofs
  ofs/tests/regress/interp_runtime_error.ofs
  ofs/tests/regress/ffi_missing_symbol.ofs
  ofs/tests/regress/array_oob.ofs
  ofs/tests/regress/core_reserved_name.ofs
  ofs/tests/regress/anchor_reserved_name.ofs
  ofs/tests/regress/seal_reserved_name.ofs
  ofs/tests/regress/type_err_ptr_arith.ofs
  ofs/tests/regress/type_err_bool_arith.ofs
  ofs/tests/regress/type_err_obsidian_mul.ofs
  ofs/tests/regress/type_err_crystal_mod.ofs
  ofs/tests/regress/fault_err_deref.ofs
  ofs/tests/regress/fault_err_call_fracture.ofs
  ofs/tests/regress/fault_err_cast.ofs
  ofs/tests/regress/fault_warn_default.ofs  # so aviso sem OFS_STRICT_FAULT; aqui (estrito) e erro
)
is_expected() { # is_expected <arquivo absoluto>
  local r="${1#$ROOT/}" e
  for e in "${EXPECTED_ERR[@]}"; do [ "$r" = "$e" ] && return 0; done
  return 1
}
export OFS_STDLIB_PATH="$ROOT/ofs/stdlib" OFS_STDLIB="$ROOT/ofs/stdlib" NO_COLOR=1 OFS_PROGRESS=plain
# F4: modo estrito (baixo nivel fora de fracture/abyss = erro nos dois modos)
export OFS_STRICT_FAULT=1
LLVM_FLAGS=()
clang --version 2>/dev/null | grep -Eq 'version 14\.|clang-1400\.' && LLVM_FLAGS=(-mllvm -opaque-pointers)
T="$(mktemp -d "${TMPDIR:-/tmp}/ofs-parity-XXXXXX")"
trap 'rm -rf "$T"' EXIT
RES="$T/results.tsv"; : >"$RES"

[ -x "$OFSCC" ] || { echo "parity: ofscc nao encontrado em $OFSCC" >&2; exit 2; }
[ -f "$MAGMA" ] || { echo "parity: magma.o nao encontrado em $MAGMA" >&2; exit 2; }

# programas: tudo com `core main` em ofs/tests, ofs/examples, testes do stdlib e casos novos
mapfile -t PROGS < <(
  { find "$ROOT/ofs/tests" "$ROOT/ofs/examples" "$HERE/cases" -name '*.ofs'
    find "$ROOT/ofs/stdlib" -name 'test*.ofs'; } 2>/dev/null | sort |
  while read -r f; do grep -qE '(^|[^A-Za-z_])core[[:space:]]+main' "$f" && echo "$f"; done
)
[ -n "${PARITY_FILTER:-}" ] && mapfile -t PROGS < <(printf '%s\n' "${PROGS[@]}" | grep -E "$PARITY_FILTER")

norm() { # remove cores, caminhos temporarios e linhas de progresso do compilador
  sed -E 's/\x1b\[[0-9;]*[A-Za-z]//g' | sed -E "s#$T/[^ :\"']*#<TMP>#g; s#/tmp/[^ :\"']*#<TMP>#g; s#$ROOT/##g" |
  grep -vE '^[[:space:]]*(Parsing|Checking|Forging|Linking|Running|Interpreting)[[:space:]].*(ok|ms)' || true
}
limited() { # limited <timeout> cmd... : 2 GB de memoria virtual
  ( ulimit -v 2000000; ulimit -c 0; exec timeout "$@" )
}
snap() { # lista arquivos do cwd com hash (ordenada)
  ( cd "$1" && find . -type f ! -name '.stdin' | sort | while read -r f; do printf '%s %s\n' "$f" "$(sha256sum "$f" | cut -c1-16)"; done )
}
skip_reason() { # skip_reason <arquivo>
  local f="$1" b; b="$(basename "$f")"
  grep -qE 'attach[[:space:]]*\{(F:[^}]*/)?(window|webui|webserver|ui|canvas|oll|http|net)(\.ofs)?\}|attach[[:space:]]*\{window|webserver_|webui|ofs_window|win_create' "$f" && { echo "UI/janela/rede"; return; }
  case "$b" in *window*|*webserver*|*webui*|*canvas*|*oll_*|*ui_*) echo "UI/janela/rede"; return;; esac
  grep -qE 'ofs_read_line|ofs_input|stdin|read_line|tcp_|socket|listen\(' "$f" && { echo "stdin/rede"; return; }
  case "$b" in *_input*|*outlet*) echo "stdin/rede"; return;; esac
  case "$f" in */ofs/tests/ofsdoc/sample.ofs) echo "precisa de arquivos fornecidos pelo teste do ofsdoc (ofs/tests/ofsdoc/run.sh)"; return;; esac
  return 1
}
tags() { # tags de recurso (separadas por virgula) por regex no fonte
  local f="$1" t="base"
  grep -qE 'obsidian|ofs_str_|ofs_obsidian' "$f" && t+=",strings"
  grep -qE 'Array<|ofs_array|\[[0-9]+(,| *\])|(cycle|mine)[[:space:]]*\(' "$f" && t+=",arrays"
  grep -qE '^[[:space:]]*(monolith|impl)[[:space:]]' "$f" && t+=",monolith"
  grep -qE '^[[:space:]]*namespace[[:space:]]' "$f" && t+=",namespaces"
  grep -qE '^[[:space:]]*strata[[:space:]]|assay[[:space:]]*\(' "$f" && t+=",strata"
  grep -qE '\)\?|[a-z_]\?$|outcome|Solid\(|Crack\(' "$f" && t+=",outcome"
  grep -qE '(extern|rift)[[:space:]]+vein' "$f" && t+=",extern"
  grep -qE '\*(stone|u8|crystal|obsidian|i[0-9]+|u[0-9]+)|fault_step|&[a-z_]+[ ;)]' "$f" && grep -qE 'fracture|abyss|bedrock' "$f" && t+=",ponteiros"
  grep -qE 'fault_addr|fault_call' "$f" && t+=",fault_addr"
  grep -qE 'fracture|abyss|bedrock|tectonic' "$f" && t+=",baixo_nivel"
  grep -qE '(^|[^A-Za-z_])asm[[:space:]]*[({"]' "$f" && t+=",asm"
  echo "$t"
}

case_libpath() { # case_libpath <arquivo>: OFS_LIB_PATH que o caso precisa (vazio = nenhum)
  case "$1" in */ofs/tests/regress/attach_lib_path.ofs) echo "$ROOT/ofs/tests/regress/attach_libdir";; esac
}
run_one() { # run_one <arquivo>  -> linha em $RES
  local f="$1" rel="${1#$ROOT/}" d tag nrc=- irc=- cls first="" reason
  d="$T/w"; rm -rf "$d"; mkdir -p "$d/n" "$d/i" "$d/b"; tag="$(tags "$f")"
  local libp; libp="$(case_libpath "$f")"
  if reason="$(skip_reason "$f")"; then
    local chk="check-falha"
    ( OFSCC_MODE=check OFSCC_INPUT="$f" limited "$TMO" "$OFSCC" ) >/dev/null 2>&1 && chk="check-ok"
    printf '%s\t%s\t%s\t%s\t%s\t%s\n' "$rel" "pulado" "$reason ($chk)" "-" "-" "$tag" >>"$RES"; return
  fi
  # --- nativo
  local nbuild=1 nrun=1 nout="" nerr=""
  if ( cd "$d/b" && { [ -z "$libp" ] || export OFS_LIB_PATH="$libp"; } && OFSCC_MODE=ir OFSCC_INPUT="$f" OFSCC_C_OUT="$d/b/p.ll" limited "$TMO" "$OFSCC" ) >"$d/b/log" 2>&1 \
     && [ -s "$d/b/p.ll" ] \
     && clang -Wno-override-module "${LLVM_FLAGS[@]}" -Wl,--allow-multiple-definition -no-pie -O1 "$d/b/p.ll" "$MAGMA" -lm -o "$d/b/p" >>"$d/b/log" 2>&1; then
    nbuild=0
    ( cd "$d/n" && limited "$TMO" "$d/b/p" </dev/null >"$d/n.out" 2>"$d/n.err" ); nrc=$?
    [ "$nrc" -eq 124 ] && nrun=2 || nrun=0
  fi
  # --- interpretado
  local irun=1
  ( cd "$d/i" && { [ -z "$libp" ] || export OFS_LIB_PATH="$libp"; } && OFSCC_MODE=run OFSCC_INPUT="$f" limited "$TMO" "$OFSCC" </dev/null >"$d/i.out" 2>"$d/i.err" ); irc=$?
  if [ "$irc" -eq 124 ]; then irun=2
  elif grep -qE 'Interpreter runtime error|\[ERRO |error:|erro:|undefined function' "$d/i.err" "$d/i.out"; then irun=1
  else irun=0; fi
  [ -f "$d/n.out" ] || { : >"$d/n.out"; : >"$d/n.err"; }
  if is_expected "$f"; then
    local nf=0 inf=0 ndesc
    { [ "$nbuild" -ne 0 ] || [ "$nrc" != 0 ]; } && nf=1
    [ "$irc" != 0 ] && inf=1
    ndesc="rc=$nrc"; [ "$nbuild" -ne 0 ] && ndesc="não compila"
    if [ "$nf$inf" = 11 ]; then cls="erro esperado"; first="falha nos dois modos (nativo: $ndesc, interp: rc=$irc)"
    else cls="difere"; first="erro esperado: só um modo falha (nativo: $ndesc, interp: rc=$irc)"; fi
  elif [ "$nrun" -eq 2 ] && [ "$irun" -eq 2 ]; then cls="pulado"; first="tempo (> ${TMO}s nos dois)"
  elif [ "$nbuild" -ne 0 ] && [ "$irun" -ne 0 ]; then cls="nenhum compila"; first="$(grep -iE 'error|erro' "$d/b/log" | head -1 | norm | cut -c1-110)"
  elif [ "$nbuild" -eq 0 ] && [ "$nrun" -eq 0 ] && [ "$irun" -ne 0 ]; then cls="só nativo"
    first="$(grep -hE 'Interpreter runtime error|\[ERRO |error:|erro:|undefined function' "$d/i.err" "$d/i.out" | head -1 | norm | cut -c1-110)"
  elif [ "$nbuild" -ne 0 ] && [ "$irun" -eq 0 ]; then cls="só interpretado"; first="$(grep -iE 'error|erro' "$d/b/log" | head -1 | norm | cut -c1-110)"
  else
    norm <"$d/n.out" >"$d/n.o"; norm <"$d/i.out" >"$d/i.o"; norm <"$d/n.err" >"$d/n.e"; norm <"$d/i.err" >"$d/i.e"
    snap "$d/n" >"$d/n.f"; snap "$d/i" >"$d/i.f"
    if [ "$nrc" -ne "$irc" ]; then cls="difere"; first="exit code: nativo=$nrc interp=$irc"
    elif ! cmp -s "$d/n.o" "$d/i.o"; then cls="difere"
      first="stdout: $(diff "$d/n.o" "$d/i.o" | grep -m1 '^[<>]' | cut -c1-100)"
    elif ! cmp -s "$d/n.e" "$d/i.e"; then cls="difere"
      first="stderr: $(diff "$d/n.e" "$d/i.e" | grep -m1 '^[<>]' | cut -c1-100)"
    elif ! cmp -s "$d/n.f" "$d/i.f"; then cls="difere"
      first="arquivos: $(diff "$d/n.f" "$d/i.f" | grep -m1 '^[<>]' | cut -c1-100)"
    else cls="igual"; fi
  fi
  first="${first//$'\t'/ }"; first="${first//|/\\|}"
  printf '%s\t%s\t%s\t%s\t%s\t%s\n' "$rel" "$cls" "$first" "$nrc" "$irc" "$tag" >>"$RES"
  # nativo/interp "passa" para a tabela por recurso
  printf '%s\t%s\t%s\n' "$rel" "$nbuild$nrun" "$irun" >>"$T/pass.tsv"
  # casos com .expected: a saida nativa tem de bater com o esperado
  local exp="${f%.ofs}.expected"
  if [ -f "$exp" ] && [ "$nrun" -eq 0 ] && ! cmp -s <(norm <"$d/n.out") "$exp"; then
    echo "AVISO: $rel: saida nativa difere do .expected" >&2
  fi
}

: >"$T/pass.tsv"
n=0; total=${#PROGS[@]}
for f in "${PROGS[@]}"; do
  n=$((n+1)); printf '[%d/%d] %s\n' "$n" "$total" "${f#$ROOT/}" >&2
  run_one "$f"
done

# ---------- MATRIZ.md
OUTMD="${PARITY_OUT:-$HERE/MATRIZ.md}"
{
  echo "# Matriz de paridade: interpretador x nativo"
  echo
  echo "Gerado por \`tests/parity/run.sh\` (fase F0 de \`plans/INTERPRETADOR_PARIDADE.md\`). Não edite à mão."
  echo
  echo "- Compilador: \`${OFSCC#$ROOT/}\` (origem: $OFSCC_SRC, sha256 $(sha256sum "$OFSCC" | cut -c1-12)) (\`OFSCC_MODE=ir\` + \`clang\` + \`${MAGMA#$ROOT/}\` (origem: $MAGMA_SRC, sha256 $(sha256sum "$MAGMA" | cut -c1-12)) x \`OFSCC_MODE=run\`), \`OFS_STDLIB_PATH=ofs/stdlib\`, \`NO_COLOR=1\`."
  echo "- Limites: \`ulimit -v\` 2 GB e \`timeout ${TMO}s\` por execução; cada modo roda num diretório limpo."
  echo "- Compara stdout, stderr normalizado, exit code e arquivos gerados no cwd."
  echo "- \`erro esperado\`: programas listados em \`EXPECTED_ERR\` (\`run.sh\`) devem falhar (rc ≠ 0) nos dois modos; fora do recurso x modo. Se só um modo falhar, é \`difere\`."
  echo
  echo "## Resumo"
  echo
  echo "| Classe | Programas |"; echo "|---|---|"
  for c in "igual" "erro esperado" "difere" "só nativo" "só interpretado" "nenhum compila" "pulado"; do
    echo "| $c | $(awk -F'\t' -v c="$c" '$2==c' "$RES" | wc -l) |"
  done
  echo "| **Total** | $(wc -l <"$RES") |"
  echo
  echo "## Recurso x modo"
  echo
  echo "\"Passa\": nativo = compila, liga e termina sem estourar o tempo; interpretado = roda sem erro do interpretador (\`Interpreter runtime error\`, \`undefined function\`, erro de sintaxe). \"Igual\" = saída, exit code e arquivos idênticos nos dois modos. Um programa conta em todos os recursos que usa (heurística por regex no fonte); \`pulado\` não conta."
  echo
  echo "| Recurso | Programas | Nativo passa | Interpretado passa | Igual |"; echo "|---|---|---|---|---|"
  for pair in "base:aritmética e controle" "strings:strings" "arrays:arrays" "monolith:monolith/impl" "namespaces:namespaces" "strata:strata/match" "outcome:\`?\`/outcome" "extern:\`extern\` libc / \`rift vein\`" "ponteiros:ponteiros \`*T\` / \`*p = v\`" "fault_addr:\`fault_addr\`/\`fault_call\`" "baixo_nivel:\`fracture\`/\`abyss\`/\`bedrock\`" "asm:\`asm\`"; do
    k="${pair%%:*}"; lbl="${pair#*:}"
    awk -F'\t' -v k="$k" 'BEGIN{FS="\t"} NR==FNR{ if($2!="pulado" && $2!="erro esperado" && ("," $6 ",") ~ ("," k ",")) {tot[$1]=1; if($2=="igual") ig[$1]=1}; next}
      { if(($1 in tot)) { if($2=="00") np++; if($3=="0") ip++ } }
      END{ n=0; for(x in tot) n++; g=0; for(x in ig) g++; printf "| %s | %d | %d/%d | %d/%d | %d/%d |\n", "'"$lbl"'", n, np, n, ip, n, g, n }' "$RES" "$T/pass.tsv"
  done
  echo
  echo "## Por programa"
  echo
  echo "| Programa | Classe | Nativo (rc) | Interp. (rc) | Primeira diferença / motivo |"; echo "|---|---|---|---|---|"
  sort -t$'\t' -k1,1 "$RES" | awk -F'\t' '{printf "| `%s` | %s | %s | %s | %s |\n", $1, $2, $4, $5, ($3==""?"":$3)}'
} >"$OUTMD"
echo "parity: MATRIZ gerada em ${OUTMD#$ROOT/}" >&2
awk -F'\t' '{c[$2]++} END{for(k in c) printf "  %s: %d\n", k, c[k]}' "$RES" | sort >&2
if [ "${PARITY_STRICT:-0}" = 1 ]; then
  awk -F'\t' '$2!="igual" && $2!="pulado" && $2!="erro esperado"{bad=1} END{exit bad}' "$RES" || exit 1
fi
exit 0
