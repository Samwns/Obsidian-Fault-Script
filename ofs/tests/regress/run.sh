#!/usr/bin/env bash
# Regressoes do compilador/runtime (bugs: `core`/`anchor` como nome, `array_get
# as crystal`, `ofs_str_char_at` O(n), metodos de `impl`, `a[i] = v`, tipo do
# elemento de `Array<T>` em parametro/campo, curto-circuito de `&&`/`||`,
# memoria do interpretador por chamada/volta de laco, aliasing de strings com
# concatenacao no lugar, strbuf, crystal -> texto via snprintf,
# intrinsecos fault_*, propagacao `?`, codigo de saida de erro do interpretador,
# FFI do interpretador: extern/rift da libc com int/string/double/variadica,
# veins do runtime nativas, simbolo ausente com rc 1; F2: ponteiro de cada
# largura, `&x` passado ao C, monolith com layout C, callback qsort/bsearch e
# reentrancia; callback com crystal/f32 e f32 no Rift (onda 4,
# ffi_callback_crystal, igual nos dois modos); F3: identificador
# indefinido e `attach` inexistente sao erro de compilacao, `attach {x}` via
# OFS_LIB_PATH e ~/.ofs/libs, `seal` = `anchor`, monolith aninhado, `(cond) as
# stone`, indice fora do limite com mensagem e rc 1; tipos e conversoes no
# typeck: aritmetica mista inteiro/crystal, `a[i]` com o tipo do elemento,
# i64/i16, conversao em forge/argumento/return/compostos, `echo` de variante
# com o nome nos dois modos; combinacoes sem sentido sao [ERRO DE TIPO]; F4:
# operacoes de baixo nivel so dentro de fracture/bedrock/tectonic/abyss, com
# OFS_STRICT_FAULT=1 e [ERRO DE FALHA]; sem ela, [AVISO DE FALHA] no stderr);
# F6: tipos de funcao `vein(T) -> R` (vein como argumento, variavel, campo,
# Array, retorno, chamada indireta, qsort com comparador tipado, externs como
# valor) e os erros de aridade/argumento/retorno/`as` fora de abyss;
# A5.1: contratos (`mineral ... grain`, `bearing`, `settle`): validos nos dois
# modos, [ERRO DE CONTRATO] estatico, violacao em execucao (stderr + rc 1),
# OFS_CONTRACTS=0 (release) sem checagem e `ofs assay` (casos de borda);
# erro de sintaxe num `attach` com o arquivo e a linha dele; OFS_LIB_PATH com
# ';' no Windows; A5.3: arena do `bedrock { }` (valido nos dois modos, o que
# escapa e [ERRO DE ARENA], memoria limitada com 20 mil voltas) e aviso de
# vazamento (OFS_LEAK_WARN=0 silencia); geode (closures): captura por valor
# nos dois modos, erros de captura/laco/C e geode com captura no C em
# execucao (stderr + rc 1); A5.4 + Ideia 4: estrato e permissoes por modulo
# (`stratum crust|fault|abyss`, `claim rift|abyss|quartz`, `caprock` do
# principal): validos nos dois modos, [ERRO DE ESTRATO]/[ERRO DE PERMISSAO],
# [AVISO DE PERMISSAO] sem OFS_STRICT_FAULT e o resumo do `ofs check`;
# Ideia 5 (magma.cast): mod compilado e carregado em execucao nos dois modos
# (valores de vein tipados, cache, recarga do mod editado), erros do mod como
# valor (sintaxe, tipo, assinatura, core main, magma.caprock, release) e
# `claim magma`/`stratum crust`/`caprock` como erro de compilacao.
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
# F4: o repositorio compila no modo estrito (operacao de baixo nivel fora de
# bloco e erro); a secao 8 testa o aviso do modo padrao
export OFS_STRICT_FAULT=1
OFSCC="${OFSCC_BIN:-$ROOT/ofs/dist/ofscc}"
T="$(mktemp -d "${TMPDIR:-/tmp}/ofs-regress-XXXXXX")"
trap 'rm -rf "$T"' EXIT
# F5: cache dos modulos nativos do quartz (execucao hibrida) so desta rodada
export OFS_QUARTZ_CACHE="$T/quartz"
FAILED=0
fail() { echo "FAIL: $*"; FAILED=$((FAILED + 1)); }

# log de build que falhou: as linhas-chave (erro, crash do clang/LLVM, passo
# que caiu) e o final, para o CI mostrar a causa e nao so "nao compila"
show_build_log() { # show_build_log <log>
  [ -s "$1" ] || { echo "  (log vazio: $1)"; return; }
  echo "  --- linhas-chave de $(basename "$1") ---"
  grep -E -i 'error|erro|LLVM ERROR|Stack dump|Running pass|Program arguments|PLEASE submit|Segmentation|signal|Assertion|^[0-9]+\.[[:space:]]' "$1" | head -40 | sed 's/^/  /'
  echo "  --- ultimas 40 linhas ---"
  tail -40 "$1" | sed 's/^/  /'
}

# runtime a partir do fonte (magma.ofs)
( cd "$ROOT/ofs/stdlib/runtime" && OFSCC_MODE=ir OFSCC_INPUT=magma.ofs OFSCC_C_OUT="$T/magma.ll" "$OFSCC" ) >"$T/magma.log" 2>&1 \
  && clang -Wno-override-module -c "$T/magma.ll" -o "$T/magma.o" 2>>"$T/magma.log" \
  || { show_build_log "$T/magma.log"; echo "FAIL: runtime magma.ofs nao compila"; exit 1; }

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
reserved seal_reserved_name seal

# 1b. erros de compilacao (F3): mensagem clara e rc != 0, nos dois modos
#     (check e run), sem executar nada
compile_err() { # compile_err <teste> <padrao> [<padrao 2>]
  local m out rc ok=1
  for m in check run; do
    out="$(OFSCC_MODE=$m OFSCC_INPUT="$HERE/$1.ofs" timeout 20 "$OFSCC" 2>&1)"; rc=$?
    if [ "$rc" -eq 0 ] || [ "$rc" -eq 124 ]; then
      fail "$1 ($m): rc=$rc (devia dar erro)"; ok=0
    elif ! grep -q -- "$2" <<<"$out" || { [ -n "${3:-}" ] && ! grep -q -- "$3" <<<"$out"; } || grep -q "nao chega aqui" <<<"$out"; then
      fail "$1 ($m): mensagem inesperada"; echo "$out" | tail -4; ok=0
    fi
  done
  [ "$ok" -eq 1 ] && echo "ok: $1 (erro de compilacao)"
}
compile_err undefined_ident "identificador indefinido 'contadr'" "voce quis dizer 'contador'"
compile_err seal_return_inside "E0042.*'return' is not allowed inside anchor/seal"
compile_err attach_missing "ERRO DE ATTACH .*attach_missing.ofs:2.*modulo 'modulo-que-nao-existe' nao encontrado" "procurado em"
# erros de tipo (F3): arquivo:linha:col e a dica
compile_err type_err_ptr_arith "ERRO DE TIPO .*type_err_ptr_arith.ofs:7:29\]: aritmetica de ponteiro com '+' (\*u8 e stone)" "fault_step(p, n)"
compile_err type_err_bool_arith "ERRO DE TIPO .*type_err_bool_arith.ofs:6:13\]: operacao '+' entre stone e bool nao existe" "as stone"
compile_err type_err_obsidian_mul "ERRO DE TIPO .*type_err_obsidian_mul.ofs:5:16\]: operacao '\*' entre obsidian e crystal nao existe" "concatenacao"
compile_err type_err_crystal_mod "ERRO DE TIPO .*type_err_crystal_mod.ofs:4:13\]: '%' so vale para inteiros (crystal e stone)"
# F4: baixo nivel fora do bloco que o libera (modo estrito)
compile_err fault_err_deref "ERRO DE FALHA .*fault_err_deref.ofs:6:16\]: acesso por ponteiro '\*p'.*libera: fracture, bedrock ou abyss" "fracture vein"
compile_err fault_err_call_fracture "ERRO DE FALHA .*fault_err_call_fracture.ofs:8:29\]: 'fault_call'.*libera: so abyss" "abyss {"
compile_err fault_err_cast "ERRO DE FALHA .*fault_err_cast.ofs:6:21\]: 'as' com ponteiro (\*u8 as stone)"
# F6: tipos de funcao `vein(...) -> R`: aridade, argumento e retorno
# conferidos em compilacao; vein <-> endereco cru so em abyss
compile_err vein_type_err_arity "ERRO DE TIPO .*vein_type_err_arity.ofs:6:11\]: chamada de 'f' com 2 argumento(s), mas o tipo vein(stone)->stone pede 1"
compile_err vein_type_err_arg "ERRO DE TIPO .*vein_type_err_arg.ofs:6:17\]: argumento 1 da chamada: obsidian onde o tipo da vein pede stone"
compile_err vein_type_err_ret "ERRO DE TIPO .*vein_type_err_ret.ofs:5:41\]: vein de assinatura vein(stone)->obsidian onde o tipo pede vein(stone)->stone" "retorno: obsidian onde o tipo pede stone"
compile_err vein_type_err_cast "ERRO DE FALHA .*vein_type_err_cast.ofs:6:21\]: 'as' com valor de vein (vein(stone)->stone as stone).*libera: so abyss"

# F5: assembly e erros sao decididos antes de executar nos dois modos.
compile_err quartz_err_abyss "ERRO DE FALHA" "quartz"
compile_err quartz_err_syntax "restricao entre aspas"
compile_err quartz_err_operand "ERRO DE QUARTZO" "operando"
compile_err quartz_err_dest "ERRO DE QUARTZO" "nao e uma variavel"
compile_err quartz_err_const "ERRO DE QUARTZO" "constante"
compile_err quartz_err_memory_out "ERRO DE QUARTZO" "memoria"

# A5.1: violacao de contrato provada em compilacao (literais/constantes)
compile_err contract_err_bearing "ERRO DE CONTRATO .*contract_err_bearing.ofs:7:17\]: a chamada de 'dividir' viola o bearing b != 0 (b = 0)" "regra em .*contract_err_bearing.ofs:2:"
compile_err contract_err_grain "ERRO DE CONTRATO .*contract_err_grain.ofs:5:34\]: o valor \"sem-arroba\" em 'e' nao cumpre a grain de 'Email': contains(\"@\")"
compile_err contract_err_result "ERRO DE CONTRATO .*contract_err_result.ofs:2:34\]: 'result' so vale no settle"
# erro de sintaxe dentro de um `attach`: arquivo e linha do modulo
compile_err syntax_err_attach "ERRO SINTATICO .*syntax_err_attach/quebrado.ofs:4:11\]: expected ')'"

# A5.3: valor da arena do bedrock que escapa (nao e texto)
compile_err arena_err_escape "ERRO DE ARENA .*arena_err_escape.ofs:6:17\]: valor alocado na arena do bedrock e guardado fora dele" "abyss"
compile_err arena_err_kept_param "ERRO DE ARENA .*arena_err_kept_param.ofs:17:18\]: .*passado a 'guardar', que guarda o parametro 1"
compile_err arena_err_return "ERRO DE ARENA .*arena_err_return.ofs:10:17\]: P alocado na arena do bedrock e devolvido (return)"
# geode: captura e copia; laco; C so recebe geode sem captura
compile_err geode_err_capture_assign "ERRO DE GEODE .*geode_err_capture_assign.ofs:5:10\]: 'n' foi capturada por valor pelo geode" "forge v = n"
compile_err geode_err_c_capture "ERRO DE GEODE .*geode_err_c_capture.ofs:7:25\]: geode que captura variaveis nao pode ir para o C ('qsort' e extern)"
compile_err geode_err_break "ERRO DE GEODE .*geode_err_break.ofs:6:20\]: 'shatter/break' fora de laco dentro do geode"
# A5.4 + Ideia 4: estrato do arquivo e permissoes (claim/caprock)
compile_err stratum_err_crust "ERRO DE ESTRATO .*stratum_err_crust.ofs:6:34\]: endereco '&x' num arquivo 'stratum crust'" "stratum fault"
compile_err stratum_err_crust_block "ERRO DE ESTRATO .*stratum_err_crust_block.ofs:6:15\]: bloco 'fracture' num arquivo 'stratum crust'"
compile_err stratum_err_crust_claim "ERRO DE ESTRATO .*stratum_err_crust_claim.ofs\]: 'claim rift' num arquivo 'stratum crust'"
compile_err claim_err_missing "ERRO DE PERMISSAO .*claim_err_missing.ofs:2:17\]: 'extern vein labs' sem 'claim rift' no topo do arquivo" "acrescente a linha 'claim rift'"
compile_err claim_err_caprock "ERRO DE PERMISSAO .*claim_err_caprock.ofs:2\]: o modulo .*stratum_mods/fonte.ofs pede 'rift', mas o caprock do programa principal libera so: nada"
compile_err claim_err_late "ERRO DE ESTRATO .*claim_err_late.ofs:5\]: 'claim' so vale no topo do arquivo"
compile_err claim_err_unknown "ERRO DE ESTRATO .*claim_err_unknown.ofs:2\]: 'lava' nao e permissao de 'claim'"
# Ideia 5: magma.cast pede 'claim magma' (sempre erro), nunca em crust, nem
# anexado alem do caprock; magma.vent precisa do tipo pedido
compile_err magma_err_claim "ERRO DE PERMISSAO .*magma_err_claim.ofs:5:26\]: 'magma.cast' (compilar e carregar codigo em execucao) sem 'claim magma'" "acrescente a linha 'claim magma'"
compile_err magma_err_crust "ERRO DE ESTRATO .*magma_err_crust.ofs:6:26\]: 'magma.cast' (compilar e carregar codigo em execucao) num arquivo 'stratum crust'"
compile_err magma_err_caprock "ERRO DE PERMISSAO .*magma_err_caprock.ofs:3\]: o modulo .*stdlib/magma.ofs pede 'magma', mas o caprock do programa principal libera so: rift, abyss"
compile_err magma_err_vent_untyped "ERRO DE TIPO .*magma_err_vent_untyped.ofs:7:12\]: magma.vent precisa do tipo de vein pedido ao mod" "forge f: vein(stone) -> stone = magma.vent"

# 2. saida esperada (nativo e interpretador)
# (vazio desde a F2: memoria crua funciona no `ofs run`)
NATIVE_ONLY=""
for exp in "$HERE"/*.expected; do
  name="$(basename "$exp" .expected)"; src="$HERE/$name.ofs"
  case "$name" in
    quartz_*aarch64*)
      if [ "$(uname -m)" != aarch64 ] && [ "$(uname -m)" != arm64 ]; then echo "skip: $name (requer AArch64)"; continue; fi;;
    quartz_*x86*)
      if [ "$(uname -m)" != x86_64 ]; then echo "skip: $name (requer x86-64)"; continue; fi;;
  esac
  case "$name" in
    quartz_*linux*)
      if [ "$(uname -s)" != Linux ]; then echo "skip: $name (requer Linux)"; continue; fi;;
  esac
  native "$src" "$T/$name" || { show_build_log "$T/build.log"; fail "$name: nao compila"; continue; }
  out="$(timeout 10 "$T/$name" 2>&1)" || { fail "$name (nativo): rc=$? / tempo"; continue; }
  [ "$out" = "$(cat "$exp")" ] || { fail "$name (nativo): saida"; diff <(echo "$out") "$exp" | head; continue; }
  echo "ok: $name (nativo)"
  # so nativo: casos listados em NATIVE_ONLY
  case " $NATIVE_ONLY " in *" $name "*) continue;; esac
  out="$(OFSCC_MODE=run OFSCC_INPUT="$src" timeout 20 "$OFSCC" 2>/dev/null)" || { fail "$name (interp): rc=$?"; continue; }
  [ "$out" = "$(cat "$exp")" ] || { fail "$name (interp): saida"; diff <(echo "$out") "$exp" | head; continue; }
  echo "ok: $name (interp)"
done

# 3. memoria do interpretador: chamadas e lacos nao podem alocar por
#    execucao (antes ~3 KB por chamada; fib(22) + 300 mil voltas passava de
#    1 GB). Roda com `ulimit -v` de 64 MB.
out="$( ulimit -v 65536; OFSCC_MODE=run OFSCC_INPUT="$HERE/interp_memory.ofs" timeout 20 "$OFSCC" 2>&1 )" \
  && [ "$out" = "$(cat "$HERE/interp_memory.expected")" ] \
  && echo "ok: interp_memory (interp, ulimit -v 64 MB)" \
  || { fail "interp_memory (interp, ulimit -v 64 MB)"; echo "$out" | tail -3; }

# 4. erro de runtime do interpretador (`Interpreter runtime error`) termina
#    com codigo != 0 (antes saia com 0 e seguia executando)
out="$(OFSCC_MODE=run OFSCC_INPUT="$HERE/interp_runtime_error.ofs" timeout 20 "$OFSCC" 2>&1)"; rc=$?
if [ "$rc" -eq 0 ] || [ "$rc" -eq 124 ]; then
  fail "interp_runtime_error (interp): rc=$rc (esperado != 0)"
elif ! grep -q "Interpreter runtime error" <<<"$out" || grep -q "nao chega aqui" <<<"$out"; then
  fail "interp_runtime_error (interp): saida inesperada"; echo "$out" | tail -3
else
  echo "ok: interp_runtime_error (interp, rc=$rc)"
fi

# 5. FFI do interpretador (F1): extern sem simbolo da o erro do linker
#    ("undefined reference") no carregamento, antes de rodar, com rc 1; e
#    o que a F2 ainda nao cobre (callback com crystal) e erro claro (rc 1).
ffi_err() { # ffi_err <teste> <padrao na saida>
  local out rc
  out="$(OFSCC_MODE=run OFSCC_INPUT="$HERE/$1.ofs" timeout 20 "$OFSCC" 2>&1)"; rc=$?
  if [ "$rc" -ne 1 ]; then
    fail "$1 (interp): rc=$rc (esperado 1)"; echo "$out" | tail -3
  elif ! grep -q "$2" <<<"$out" || grep -q "nao chega aqui" <<<"$out"; then
    fail "$1 (interp): saida inesperada"; echo "$out" | tail -3
  else
    echo "ok: $1 (interp, rc=1)"
  fi
}
ffi_err ffi_missing_symbol "undefined reference to .ofs_ffi_simbolo_que_nao_existe'"

# 6. `attach {x}` procura tambem em $OFS_LIB_PATH (pastas separadas por ':')
#    e em ~/.ofs/libs (onde src/packages/install_library.sh instala)
mkdir -p "$T/vazia" "$T/home/.ofs/libs"
cp "$HERE/attach_libdir/pedra_extra.ofs" "$T/home/.ofs/libs/"
for via in OFS_LIB_PATH home; do
  for m in run native; do
    if [ "$via" = OFS_LIB_PATH ]; then envs=(OFS_LIB_PATH="$T/vazia:$HERE/attach_libdir" HOME="$T/vazia")
    else envs=(HOME="$T/home"); fi
    if [ "$m" = run ]; then
      out="$(env -u OFS_LIB_PATH -u OFS_LIB_HOME "${envs[@]}" OFSCC_MODE=run OFSCC_INPUT="$HERE/attach_lib_path.ofs" timeout 20 "$OFSCC" 2>&1)"
    else
      out="$(env -u OFS_LIB_PATH -u OFS_LIB_HOME "${envs[@]}" OFSCC_MODE=ir OFSCC_INPUT="$HERE/attach_lib_path.ofs" OFSCC_C_OUT="$T/alp.ll" "$OFSCC" >"$T/alp.log" 2>&1 \
        && clang -Wno-override-module -no-pie -O1 "$T/alp.ll" "$T/magma.o" -lm -o "$T/alp" 2>>"$T/alp.log" && timeout 10 "$T/alp" 2>&1)" || out="$(tail -3 "$T/alp.log")"
    fi
    [ "$out" = "42" ] && echo "ok: attach_lib_path ($via, $m)" || { fail "attach_lib_path ($via, $m)"; echo "$out" | tail -3; }
  done
done
# no Windows (OS=Windows_NT) o separador e ';' (':' quebra em C:\libs)
out="$(env -u OFS_LIB_PATH -u OFS_LIB_HOME OS=Windows_NT HOME="$T/vazia" OFS_LIB_PATH="$T/vazia;$HERE/attach_libdir" OFSCC_MODE=run OFSCC_INPUT="$HERE/attach_lib_path.ofs" timeout 20 "$OFSCC" 2>&1)"
[ "$out" = "42" ] && echo "ok: attach_lib_path (OFS_LIB_PATH com ';' no Windows)" || { fail "attach_lib_path (';' no Windows)"; echo "$out" | tail -3; }
out="$(env -u OFS_LIB_PATH -u OFS_LIB_HOME OS=Windows_NT HOME="$T/vazia" OFS_LIB_PATH="$T/vazia" OFSCC_MODE=check OFSCC_INPUT="$HERE/attach_lib_path.ofs" timeout 20 "$OFSCC" 2>&1)"
grep -q "separada por ';'" <<<"$out" && echo "ok: dica do attach com ';' no Windows" || { fail "dica do attach no Windows"; echo "$out" | tail -3; }

# 7. indice fora do limite: mensagem no stderr com indice e tamanho e rc 1,
#    nos dois modos (antes: nativo exit(1) mudo, interpretador null e seguia)
oob_check() { # oob_check <modo> <rc> <stdout> <stderr>
  if [ "$2" -ne 1 ]; then fail "array_oob ($1): rc=$2 (esperado 1)"
  elif ! grep -q "ofs: indice 7 fora do limite do array (tamanho 3)" "$4"; then fail "array_oob ($1): stderr sem a mensagem"; tail -3 "$4"
  elif grep -q "nao chega aqui" "$3" || [ "$(head -1 "$3")" != "2" ]; then fail "array_oob ($1): stdout inesperado"; head -3 "$3"
  else echo "ok: array_oob ($1, rc=1)"; fi
}
if native "$HERE/array_oob.ofs" "$T/array_oob"; then
  timeout 10 "$T/array_oob" >"$T/oob.out" 2>"$T/oob.err"; oob_check nativo $? "$T/oob.out" "$T/oob.err"
else fail "array_oob: nao compila"; show_build_log "$T/build.log"; fi
OFSCC_MODE=run OFSCC_INPUT="$HERE/array_oob.ofs" timeout 20 "$OFSCC" >"$T/oob.out" 2>"$T/oob.err"; oob_check interp $? "$T/oob.out" "$T/oob.err"

# 8. F4 sem OFS_STRICT_FAULT: [AVISO DE FALHA] no stderr (com a data em que
#    vira erro), compila e roda nos dois modos com o stdout so do programa
for m in run native; do
  if [ "$m" = run ]; then
    out="$(env -u OFS_STRICT_FAULT OFSCC_MODE=run OFSCC_INPUT="$HERE/fault_warn_default.ofs" timeout 20 "$OFSCC" 2>"$T/fw.err")"; rc=$?
  else
    env -u OFS_STRICT_FAULT OFSCC_MODE=ir OFSCC_INPUT="$HERE/fault_warn_default.ofs" OFSCC_C_OUT="$T/fw.ll" "$OFSCC" >"$T/fw.err" 2>&1 \
      && clang -Wno-override-module -no-pie -O1 "$T/fw.ll" "$T/magma.o" -lm -o "$T/fw" 2>>"$T/fw.log"; rc=$?
    [ "$rc" -eq 0 ] && { out="$(timeout 10 "$T/fw" 2>&1)"; rc=$?; }
  fi
  if [ "$rc" -ne 0 ] || [ "$out" != "4" ]; then fail "fault_warn_default ($m): rc=$rc saida=$out"
  elif ! grep -q "AVISO DE FALHA .*fault_warn_default.ofs:6:11\]: acesso por ponteiro" "$T/fw.err" || ! grep -q "viram erro em 2027-01-09" "$T/fw.err"; then
    fail "fault_warn_default ($m): sem o aviso"; tail -3 "$T/fw.err"
  else echo "ok: fault_warn_default ($m, aviso)"; fi
done

# 9. F6: chamar um valor de vein null: mensagem no stderr e rc 1 nos dois
#    modos (o que veio antes no stdout fica)
vnull_check() { # vnull_check <modo> <rc> <stdout> <stderr>
  if [ "$2" -ne 1 ]; then fail "vein_type_null_call ($1): rc=$2 (esperado 1)"
  elif ! grep -q "ofs: chamada de vein nula" "$4"; then fail "vein_type_null_call ($1): stderr sem a mensagem"; tail -3 "$4"
  elif grep -q "nao chega aqui" "$3" || [ "$(head -1 "$3")" != "2" ]; then fail "vein_type_null_call ($1): stdout inesperado"; head -3 "$3"
  else echo "ok: vein_type_null_call ($1, rc=1)"; fi
}
if native "$HERE/vein_type_null_call.ofs" "$T/vnull"; then
  timeout 10 "$T/vnull" >"$T/vnull.out" 2>"$T/vnull.err"; vnull_check nativo $? "$T/vnull.out" "$T/vnull.err"
else fail "vein_type_null_call: nao compila"; show_build_log "$T/build.log"; fi
OFSCC_MODE=run OFSCC_INPUT="$HERE/vein_type_null_call.ofs" timeout 20 "$OFSCC" >"$T/vnull.out" 2>"$T/vnull.err"; vnull_check interp $? "$T/vnull.out" "$T/vnull.err"

# 10. F5: `ofs run` hibrido do quartz: a 1a execucao compila o modulo
#     nativo, a 2a usa o cache (mesmo hash); sem clang e sem cache, erro
#     claro com rc 1 antes de executar o programa (so x86-64)
if [ "$(uname -m)" = x86_64 ]; then
  qz="$HERE/quartz_main_x86.ofs"
  for want in compilado cache; do
    out="$(OFS_QUARTZ_CACHE="$T/quartz-novo" OFS_VERBOSE=1 OFSCC_MODE=run OFSCC_INPUT="$qz" timeout 60 "$OFSCC" 2>"$T/qz.err")"; rc=$?
    if [ "$rc" -ne 0 ] || [ "$out" != "$(cat "$HERE/quartz_main_x86.expected")" ]; then fail "quartz hibrido ($want): rc=$rc"; tail -3 "$T/qz.err"
    elif ! grep -q "quartz: .*($want)" "$T/qz.err"; then fail "quartz hibrido: esperava '($want)'"; tail -3 "$T/qz.err"
    else echo "ok: quartz hibrido ($want)"; fi
  done
  out="$(OFS_QUARTZ_CACHE="$T/quartz-sem-cc" OFS_QUARTZ_CC="$T/clang-inexistente" OFSCC_MODE=run OFSCC_INPUT="$qz" timeout 60 "$OFSCC" 2>"$T/qz.err")"; rc=$?
  if [ "$rc" -ne 1 ] || [ -n "$out" ]; then fail "quartz sem clang: rc=$rc saida=$out (esperado rc 1, sem saida)"
  elif ! grep -q "precisa do clang" "$T/qz.err" || ! grep -q "ofs build" "$T/qz.err"; then fail "quartz sem clang: mensagem"; tail -3 "$T/qz.err"
  else echo "ok: quartz sem clang (erro claro, rc=1)"; fi
else
  echo "skip: quartz hibrido (requer x86-64)"
fi

# 11. A5.1: contrato violado em execucao: "ofs: [CONTRATO VIOLADO arquivo:l:c]"
#     com a regra e os valores no stderr e rc 1, nos dois modos (o stdout de
#     antes fica); OFS_CONTRACTS=0 (`ofs build --release`) tira a checagem
ctr_check() { # ctr_check <teste> <modo> <rc> <stdout> <stderr> <padrao>
  if [ "$3" -ne 1 ]; then fail "$1 ($2): rc=$3 (esperado 1)"
  elif ! grep -q -- "$6" "$5"; then fail "$1 ($2): stderr sem a mensagem"; tail -3 "$5"
  elif grep -q "nao chega aqui" "$4" || [ "$(head -1 "$4")" != "4" ]; then fail "$1 ($2): stdout inesperado"; head -3 "$4"
  else echo "ok: $1 ($2, rc=1)"; fi
}
for ct in "contract_fail_settle|ofs: \[CONTRATO VIOLADO .*contract_fail_settle.ofs:2:64\]: settle de 'dividir' falhou: result \* b == a (result = 3, b = 2, a = 7)" \
          "contract_fail_grain|ofs: \[CONTRATO VIOLADO .*contract_fail_grain.ofs:4:17\]: grain de 'Pos' falhou no parametro 'p' de 'usar': value > 0 (value = -3)"; do
  cn="${ct%%|*}"; cp="${ct#*|}"
  if native "$HERE/$cn.ofs" "$T/$cn"; then
    timeout 10 "$T/$cn" >"$T/ct.out" 2>"$T/ct.err"; ctr_check "$cn" nativo $? "$T/ct.out" "$T/ct.err" "$cp"
  else fail "$cn: nao compila"; show_build_log "$T/build.log"; fi
  OFSCC_MODE=run OFSCC_INPUT="$HERE/$cn.ofs" timeout 20 "$OFSCC" >"$T/ct.out" 2>"$T/ct.err"; ctr_check "$cn" interp $? "$T/ct.out" "$T/ct.err" "$cp"
done
OFS_CONTRACTS=0 OFSCC_MODE=ir OFSCC_INPUT="$HERE/contract_fail_settle.ofs" OFSCC_C_OUT="$T/rel.ll" "$OFSCC" >"$T/rel.log" 2>&1 \
  && clang -Wno-override-module -no-pie -O1 "$T/rel.ll" "$T/magma.o" -lm -o "$T/rel" 2>>"$T/rel.log" \
  && out="$(timeout 10 "$T/rel" 2>&1)" && [ "$out" = "$(printf '4\n3\nnao chega aqui')" ] \
  && ! grep -q "__ofs_contract_fail\|ofs_array_fail" "$T/rel.ll" \
  && echo "ok: contract release (OFS_CONTRACTS=0, sem checagem)" || { fail "contract release (OFS_CONTRACTS=0)"; tail -3 "$T/rel.log"; }

# 12. A5.1: `ofs assay` (OFSCC_MODE=assay): casos de borda das veins com
#     contrato; tudo certo = rc 0 e resumo; settle errado = mensagem e rc 1
out="$(OFSCC_MODE=assay OFSCC_INPUT="$HERE/contract_ok.ofs" timeout 20 "$OFSCC" 2>&1)"; rc=$?
if [ "$rc" -ne 0 ] || ! grep -q "assay: 16 caso(s) ok em 5 vein(s); 8 fora das regras de entrada" <<<"$out" \
   || ! grep -q -- "--   dividir(1, 0): fora do bearing b != 0" <<<"$out"; then
  fail "assay contract_ok: rc=$rc"; echo "$out" | tail -4
else echo "ok: assay contract_ok (rc=0)"; fi
OFSCC_MODE=assay OFSCC_INPUT="$HERE/contract_assay.ofs" timeout 20 "$OFSCC" >"$T/as.out" 2>"$T/as.err"; rc=$?
if [ "$rc" -ne 1 ] || ! grep -q "settle de 'metade' falhou: result \* 2 == n (result = 0, n = 1)" "$T/as.err" \
   || ! grep -q "ok   metade(0)" "$T/as.out" || grep -q "ok   metade(1)" "$T/as.out"; then
  fail "assay contract_assay: rc=$rc"; tail -3 "$T/as.err"; tail -3 "$T/as.out"
else echo "ok: assay contract_assay (achou o settle errado, rc=1)"; fi

# 13. A5.3: a arena do bedrock libera a cada volta: 20 mil voltas com 64 KB
#     de ofs_alloc (1,3 GB sem arena) com `ulimit -v` de 256 MB nos dois
#     modos; aviso de vazamento no stderr (e silencio com OFS_LEAK_WARN=0);
#     geode com captura que chega ao C em execucao: stderr + rc 1
if native "$HERE/arena_memory.ofs" "$T/arena_memory"; then
  out="$( ulimit -v 262144; timeout 20 "$T/arena_memory" 2>&1 )"
  [ "$out" = "$(cat "$HERE/arena_memory.expected")" ] && echo "ok: arena_memory (nativo, ulimit -v 256 MB)" || { fail "arena_memory (nativo, 256 MB)"; echo "$out" | tail -3; }
else fail "arena_memory: nao compila"; show_build_log "$T/build.log"; fi
out="$( ulimit -v 262144; OFSCC_MODE=run OFSCC_INPUT="$HERE/arena_memory.ofs" timeout 30 "$OFSCC" 2>&1 )"
[ "$out" = "$(cat "$HERE/arena_memory.expected")" ] && echo "ok: arena_memory (interp, ulimit -v 256 MB)" || { fail "arena_memory (interp, 256 MB)"; echo "$out" | tail -3; }
for m in check run; do
  OFSCC_MODE=$m OFSCC_INPUT="$HERE/leak_warn.ofs" timeout 20 "$OFSCC" >"$T/lw.out" 2>"$T/lw.err"; rc=$?
  n="$(grep -c "AVISO DE VAZAMENTO" "$T/lw.err")"
  if [ "$rc" -ne 0 ]; then fail "leak_warn ($m): rc=$rc (aviso nao e erro)"
  elif [ "$n" != "1" ] || ! grep -q "AVISO DE VAZAMENTO .*leak_warn.ofs:19:35\]: 'vaza' recebe ofs_alloc" "$T/lw.err" || ! grep -q "OFS_LEAK_WARN=0" "$T/lw.err"; then
    fail "leak_warn ($m): esperava 1 aviso (vaza)"; cat "$T/lw.err" | head -6
  elif [ "$m" = run ] && [ "$(cat "$T/lw.out")" != "$(printf '5\nfim')" ]; then fail "leak_warn (run): stdout"; cat "$T/lw.out"
  else echo "ok: leak_warn ($m, 1 aviso)"; fi
done
OFS_LEAK_WARN=0 OFSCC_MODE=check OFSCC_INPUT="$HERE/leak_warn.ofs" timeout 20 "$OFSCC" >/dev/null 2>"$T/lw.err"
grep -q "AVISO DE VAZAMENTO" "$T/lw.err" && fail "leak_warn: OFS_LEAK_WARN=0 nao silencia" || echo "ok: leak_warn (OFS_LEAK_WARN=0 silencia)"
gc_check() { # gc_check <modo> <rc> <stdout> <stderr>
  if [ "$2" -ne 1 ]; then fail "geode_c_runtime ($1): rc=$2 (esperado 1)"
  elif ! grep -q "ofs: geode com captura nao pode ir para o C" "$4"; then fail "geode_c_runtime ($1): stderr sem a mensagem"; tail -3 "$4"
  elif grep -q "nao chega aqui" "$3" || [ "$(head -1 "$3")" != "2" ]; then fail "geode_c_runtime ($1): stdout inesperado"; head -3 "$3"
  else echo "ok: geode_c_runtime ($1, rc=1)"; fi
}
if native "$HERE/geode_c_runtime.ofs" "$T/gcr"; then
  timeout 10 "$T/gcr" >"$T/gcr.out" 2>"$T/gcr.err"; gc_check nativo $? "$T/gcr.out" "$T/gcr.err"
else fail "geode_c_runtime: nao compila"; show_build_log "$T/build.log"; fi
OFSCC_MODE=run OFSCC_INPUT="$HERE/geode_c_runtime.ofs" timeout 20 "$OFSCC" >"$T/gcr.out" 2>"$T/gcr.err"; gc_check interp $? "$T/gcr.out" "$T/gcr.err"

# 14. A5.4: sem OFS_STRICT_FAULT, uso sem `claim` e [AVISO DE PERMISSAO] no
#     stderr (com a data em que vira erro) e o programa compila e roda nos
#     dois modos; o `ofs check` lista o estrato e o claim de cada modulo
for m in run native; do
  if [ "$m" = run ]; then
    out="$(env -u OFS_STRICT_FAULT OFSCC_MODE=run OFSCC_INPUT="$HERE/claim_warn_default.ofs" timeout 20 "$OFSCC" 2>"$T/cw.err")"; rc=$?
  else
    env -u OFS_STRICT_FAULT OFSCC_MODE=ir OFSCC_INPUT="$HERE/claim_warn_default.ofs" OFSCC_C_OUT="$T/cw.ll" "$OFSCC" >"$T/cw.err" 2>&1 \
      && clang -Wno-override-module -no-pie -O1 "$T/cw.ll" "$T/magma.o" -lm -o "$T/cw" 2>>"$T/cw.log"; rc=$?
    [ "$rc" -eq 0 ] && { out="$(timeout 10 "$T/cw" 2>&1)"; rc=$?; }
  fi
  if [ "$rc" -ne 0 ] || [ "$out" != "8" ]; then fail "claim_warn_default ($m): rc=$rc saida=$out"
  elif ! grep -q "AVISO DE PERMISSAO .*claim_warn_default.ofs:3:17\]: 'extern vein labs' sem 'claim rift'" "$T/cw.err" || ! grep -q "viram erro em 2027-03-16" "$T/cw.err"; then
    fail "claim_warn_default ($m): sem o aviso"; tail -3 "$T/cw.err"
  else echo "ok: claim_warn_default ($m, aviso)"; fi
done
out="$(OFSCC_MODE=check OFSCC_INPUT="$HERE/stratum_claims.ofs" timeout 20 "$OFSCC" 2>&1)"; rc=$?
if [ "$rc" -ne 0 ] || ! grep -q "stratum_claims.ofs (principal): stratum fault (padrao), claim abyss" <<<"$out" \
   || ! grep -q "stratum_mods/fonte.ofs: stratum fault, claim rift" <<<"$out" || ! grep -q "stratum_mods/puro.ofs: stratum crust, sem claim" <<<"$out" \
   || ! grep -q "caprock do principal: os anexados podem pedir rift, abyss" <<<"$out"; then
  fail "ofs check: resumo de estrato/claim (rc=$rc)"; echo "$out" | tail -6
else echo "ok: ofs check mostra estrato e claim de cada modulo"; fi

# 15. Ideia 5 (magma.cast): o mod (arquivo ou texto) e compilado pelo ofscc
#     testado (OFS_MAGMA_OFSCC no nativo; no `ofs run` o proprio processo),
#     vira .so no cache desta rodada e e chamado nos dois modos; o nativo e
#     ligado com -rdynamic (o `ofs build` faz isso quando o programa usa
#     magma.cast). A 1a carga compila, a 2a usa o cache; sem o runtime
#     exportado e sem clang, o erro volta como valor.
export OFS_MAGMA_CACHE="$T/magma"
for mt in cast_basic cast_errors cast_reload; do
  msrc="$HERE/magma/$mt.ofs"; mexp="$HERE/magma/$mt.expected"
  if OFSCC_MODE=ir OFSCC_INPUT="$msrc" OFSCC_C_OUT="$T/mg.ll" "$OFSCC" >"$T/build.log" 2>&1 \
     && clang -Wno-override-module -rdynamic -no-pie -O1 "$T/mg.ll" "$T/magma.o" -lm -o "$T/mg_$mt" 2>>"$T/build.log"; then
    out="$(cd "$HERE/magma" && OFS_MAGMA_OFSCC="$OFSCC" timeout 120 "$T/mg_$mt" 2>&1)"; rc=$?
    if [ "$rc" -ne 0 ] || [ "$out" != "$(cat "$mexp")" ]; then fail "magma $mt (nativo): rc=$rc"; diff <(echo "$out") "$mexp" | head; else echo "ok: magma $mt (nativo)"; fi
  else fail "magma $mt: nao compila"; show_build_log "$T/build.log"; fi
  out="$(cd "$HERE/magma" && OFSCC_MODE=run OFSCC_INPUT="$msrc" timeout 120 "$OFSCC" 2>&1)"; rc=$?
  if [ "$rc" -ne 0 ] || [ "$out" != "$(cat "$mexp")" ]; then fail "magma $mt (interp): rc=$rc"; diff <(echo "$out") "$mexp" | head; else echo "ok: magma $mt (interp)"; fi
done
for want in compilado cache; do
  out="$(cd "$HERE/magma" && OFS_MAGMA_CACHE="$T/magma-novo" OFS_VERBOSE=1 OFSCC_MODE=run OFSCC_INPUT="$HERE/magma/cast_basic.ofs" timeout 120 "$OFSCC" 2>&1 >/dev/null)"
  grep -q "magma.cast: .*\.so ($want)" <<<"$out" && echo "ok: magma cache ($want)" || { fail "magma cache: esperava '($want)'"; echo "$out" | tail -3; }
done
if OFSCC_MODE=ir OFSCC_INPUT="$HERE/magma/cast_basic.ofs" OFSCC_C_OUT="$T/mg.ll" "$OFSCC" >"$T/build.log" 2>&1 \
   && clang -Wno-override-module -no-pie -O1 "$T/mg.ll" "$T/magma.o" -lm -o "$T/mg_noexp" 2>>"$T/build.log"; then
  out="$(cd "$HERE/magma" && timeout 60 "$T/mg_noexp" 2>&1)"; rc=$?
  [ "$rc" -eq 0 ] && grep -q "nao exporta o runtime do OFS" <<<"$out" && echo "ok: magma sem runtime exportado (erro como valor)" || { fail "magma sem -rdynamic: rc=$rc"; echo "$out" | tail -3; }
else fail "magma sem -rdynamic: nao compila"; fi
out="$(cd "$HERE/magma" && OFS_MAGMA_CACHE="$T/magma-sem-cc" OFS_MAGMA_CC="$T/clang-inexistente" OFSCC_MODE=run OFSCC_INPUT="$HERE/magma/cast_basic.ofs" timeout 60 "$OFSCC" 2>&1)"; rc=$?
[ "$rc" -eq 0 ] && grep -q "clang-inexistente) nao foi encontrado" <<<"$out" && echo "ok: magma sem clang (erro como valor)" || { fail "magma sem clang: rc=$rc"; echo "$out" | tail -3; }
unset OFS_MAGMA_CACHE

[ "$FAILED" -eq 0 ] && echo "regress: tudo ok" || { echo "regress: $FAILED falha(s)"; exit 1; }
