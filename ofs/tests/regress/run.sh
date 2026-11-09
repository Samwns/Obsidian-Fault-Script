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
# reentrancia; callback com crystal = erro claro com rc 1; F3: identificador
# indefinido e `attach` inexistente sao erro de compilacao, `attach {x}` via
# OFS_LIB_PATH e ~/.ofs/libs, `seal` = `anchor`, monolith aninhado, `(cond) as
# stone`, indice fora do limite com mensagem e rc 1; tipos e conversoes no
# typeck: aritmetica mista inteiro/crystal, `a[i]` com o tipo do elemento,
# i64/i16, conversao em forge/argumento/return/compostos, `echo` de variante
# com o nome nos dois modos; combinacoes sem sentido sao [ERRO DE TIPO]).
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
OFSCC="${OFSCC_BIN:-$ROOT/ofs/dist/ofscc}"
T="$(mktemp -d "${TMPDIR:-/tmp}/ofs-regress-XXXXXX")"
trap 'rm -rf "$T"' EXIT
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
compile_err type_err_ptr_arith "ERRO DE TIPO .*type_err_ptr_arith.ofs:6:25\]: aritmetica de ponteiro com '+' (\*u8 e stone)" "fault_step(p, n)"
compile_err type_err_bool_arith "ERRO DE TIPO .*type_err_bool_arith.ofs:6:13\]: operacao '+' entre stone e bool nao existe" "as stone"
compile_err type_err_obsidian_mul "ERRO DE TIPO .*type_err_obsidian_mul.ofs:5:16\]: operacao '\*' entre obsidian e crystal nao existe" "concatenacao"
compile_err type_err_crystal_mod "ERRO DE TIPO .*type_err_crystal_mod.ofs:4:13\]: '%' so vale para inteiros (crystal e stone)"

# 2. saida esperada (nativo e interpretador)
# (vazio desde a F2: memoria crua funciona no `ofs run`)
NATIVE_ONLY=""
for exp in "$HERE"/*.expected; do
  name="$(basename "$exp" .expected)"; src="$HERE/$name.ofs"
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
ffi_err ffi_callback_crystal "callback com parametro crystal"

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

[ "$FAILED" -eq 0 ] && echo "regress: tudo ok" || { echo "regress: $FAILED falha(s)"; exit 1; }
