# Matriz de paridade: interpretador x nativo

Gerado por `tests/parity/run.sh` (fase F0 de `plans/INTERPRETADOR_PARIDADE.md`). Não edite à mão.

- Compilador: `ofs/dist/ofscc` (origem: padrão, sha256 2a182231f865) (`OFSCC_MODE=ir` + `clang` + `ofs/dist/magma.o` (origem: padrão, sha256 dc55f59ceab9) x `OFSCC_MODE=run`), `OFS_STDLIB_PATH=ofs/stdlib`, `NO_COLOR=1`.
- Limites: `ulimit -v` 2 GB e `timeout 20s` por execução; cada modo roda num diretório limpo.
- Compara stdout, stderr normalizado, exit code e arquivos gerados no cwd.
- `erro esperado`: programas listados em `EXPECTED_ERR` (`run.sh`) devem falhar (rc ≠ 0) nos dois modos; fora do recurso x modo. Se só um modo falhar, é `difere`.

## Resumo

| Classe | Programas |
|---|---|
| igual | 81 |
| erro esperado | 13 |
| difere | 0 |
| só nativo | 1 |
| só interpretado | 0 |
| nenhum compila | 0 |
| pulado | 14 |
| **Total** | 109 |

## Recurso x modo

"Passa": nativo = compila, liga e termina sem estourar o tempo; interpretado = roda sem erro do interpretador (`Interpreter runtime error`, `undefined function`, erro de sintaxe). "Igual" = saída, exit code e arquivos idênticos nos dois modos. Um programa conta em todos os recursos que usa (heurística por regex no fonte); `pulado` não conta.

| Recurso | Programas | Nativo passa | Interpretado passa | Igual |
|---|---|---|---|---|
| aritmética e controle | 82 | 82/82 | 81/82 | 81/82 |
| strings | 28 | 28/28 | 28/28 | 28/28 |
| arrays | 23 | 23/23 | 23/23 | 23/23 |
| monolith/impl | 14 | 14/14 | 14/14 | 14/14 |
| namespaces | 3 | 3/3 | 3/3 | 3/3 |
| strata/match | 5 | 5/5 | 5/5 | 5/5 |
| `?`/outcome | 2 | 2/2 | 2/2 | 2/2 |
| `extern` libc / `rift vein` | 22 | 22/22 | 21/22 | 21/22 |
| ponteiros `*T` / `*p = v` | 19 | 19/19 | 19/19 | 19/19 |
| `fault_addr`/`fault_call` | 6 | 6/6 | 5/6 | 5/6 |
| `fracture`/`abyss`/`bedrock` | 21 | 21/21 | 21/21 | 21/21 |
| `asm` | 0 | 0/0 | 0/0 | 0/0 |

## Por programa

| Programa | Classe | Nativo (rc) | Interp. (rc) | Primeira diferença / motivo |
|---|---|---|---|---|
| `ofs/examples/attach_file_demo.ofs` | igual | 0 | 0 |  |
| `ofs/examples/baremetal_minimal.ofs` | igual | 0 | 0 |  |
| `ofs/examples/bedrock_gateway.ofs` | igual | 0 | 0 |  |
| `ofs/examples/calculator.ofs` | igual | 0 | 0 |  |
| `ofs/examples/canvas_package_demo.ofs` | pulado | - | - | UI/janela/rede (check-ok) |
| `ofs/examples/canvas_window_demo.ofs` | pulado | - | - | UI/janela/rede (check-ok) |
| `ofs/examples/collections.ofs` | igual | 0 | 0 |  |
| `ofs/examples/extern_func.ofs` | igual | 0 | 0 |  |
| `ofs/examples/factorial.ofs` | igual | 0 | 0 |  |
| `ofs/examples/fizzbuzz.ofs` | igual | 0 | 0 |  |
| `ofs/examples/fmt.ofs` | igual | 0 | 0 |  |
| `ofs/examples/hello.ofs` | igual | 0 | 0 |  |
| `ofs/examples/hybrid_fault_console.ofs` | igual | 0 | 0 |  |
| `ofs/examples/math_ops.ofs` | igual | 0 | 0 |  |
| `ofs/examples/monolith_advanced.ofs` | igual | 0 | 0 |  |
| `ofs/examples/native_site/build.ofs` | igual | 0 | 0 |  |
| `ofs/examples/oll/oll_window_app.ofs` | pulado | - | - | UI/janela/rede (check-ok) |
| `ofs/examples/packages_demo.ofs` | igual | 0 | 0 |  |
| `ofs/examples/pointers.ofs` | igual | 0 | 0 |  |
| `ofs/examples/recursion.ofs` | igual | 0 | 0 |  |
| `ofs/examples/showcase.ofs` | igual | 0 | 0 |  |
| `ofs/examples/small_ints.ofs` | igual | 0 | 0 |  |
| `ofs/examples/string_ops.ofs` | igual | 0 | 0 |  |
| `ofs/examples/structs.ofs` | igual | 0 | 0 |  |
| `ofs/examples/test_webserver_simple.ofs` | pulado | - | - | UI/janela/rede (check-ok) |
| `ofs/examples/type_cast.ofs` | igual | 0 | 0 |  |
| `ofs/examples/ui_window_demo.ofs` | pulado | - | - | UI/janela/rede (check-ok) |
| `ofs/examples/variables.ofs` | igual | 0 | 0 |  |
| `ofs/examples/webserver_demo.ofs` | pulado | - | - | UI/janela/rede (check-ok) |
| `ofs/examples/webui_site.ofs` | pulado | - | - | UI/janela/rede (check-ok) |
| `ofs/examples/while_loop.ofs` | igual | 0 | 0 |  |
| `ofs/tests/interpreter/01_arithmetic.ofs` | igual | 0 | 0 |  |
| `ofs/tests/interpreter/02_control_flow.ofs` | igual | 0 | 0 |  |
| `ofs/tests/interpreter/03_variables_scoping.ofs` | igual | 0 | 0 |  |
| `ofs/tests/interpreter/04_functions_recursion.ofs` | igual | 0 | 0 |  |
| `ofs/tests/interpreter/05_arrays.ofs` | igual | 0 | 0 |  |
| `ofs/tests/interpreter/06_structs_monolith.ofs` | igual | 0 | 0 |  |
| `ofs/tests/interpreter/07_pointers.ofs` | igual | 0 | 0 |  |
| `ofs/tests/interpreter/08_namespaces.ofs` | igual | 0 | 0 |  |
| `ofs/tests/interpreter/09_impl_methods.ofs` | igual | 0 | 0 |  |
| `ofs/tests/interpreter/10_geological_flow.ofs` | igual | 0 | 0 |  |
| `ofs/tests/interpreter/12_outcome_lode.ofs` | igual | 0 | 0 |  |
| `ofs/tests/interpreter/13_question_propagation.ofs` | igual | 0 | 0 |  |
| `ofs/tests/ofsdoc/sample.ofs` | pulado | - | - | precisa de arquivos fornecidos pelo teste do ofsdoc (ofs/tests/ofsdoc/run.sh) (check-falha) |
| `ofs/tests/regress/anchor_reserved_name.ofs` | erro esperado | - | 1 | falha nos dois modos (nativo: não compila, interp: rc=1) |
| `ofs/tests/regress/array_elem_type.ofs` | igual | 0 | 0 |  |
| `ofs/tests/regress/array_get_as_crystal.ofs` | igual | 0 | 0 |  |
| `ofs/tests/regress/array_oob.ofs` | erro esperado | 1 | 1 | falha nos dois modos (nativo: rc=1, interp: rc=1) |
| `ofs/tests/regress/attach_lib_path.ofs` | igual | 0 | 0 |  |
| `ofs/tests/regress/attach_missing.ofs` | erro esperado | - | 1 | falha nos dois modos (nativo: não compila, interp: rc=1) |
| `ofs/tests/regress/bool_cast_stone.ofs` | igual | 0 | 0 |  |
| `ofs/tests/regress/core_reserved_name.ofs` | erro esperado | - | 1 | falha nos dois modos (nativo: não compila, interp: rc=1) |
| `ofs/tests/regress/crystal_text.ofs` | igual | 0 | 0 |  |
| `ofs/tests/regress/fault_intrinsics.ofs` | igual | 0 | 0 |  |
| `ofs/tests/regress/ffi_addr_var.ofs` | igual | 0 | 0 |  |
| `ofs/tests/regress/ffi_callback_crystal.ofs` | só nativo | 0 | 1 | Interpreter runtime error: fault_addr(com_crystal): callback com parametro crystal/f32 ainda nao e suportado n |
| `ofs/tests/regress/ffi_callback_reentry.ofs` | igual | 0 | 0 |  |
| `ofs/tests/regress/ffi_libc.ofs` | igual | 0 | 0 |  |
| `ofs/tests/regress/ffi_missing_symbol.ofs` | erro esperado | - | 1 | falha nos dois modos (nativo: não compila, interp: rc=1) |
| `ofs/tests/regress/ffi_qsort_callback.ofs` | igual | 0 | 0 |  |
| `ofs/tests/regress/ffi_runtime_native.ofs` | igual | 0 | 0 |  |
| `ofs/tests/regress/ffi_struct.ofs` | igual | 0 | 0 |  |
| `ofs/tests/regress/globals_before_decl.ofs` | igual | 0 | 0 |  |
| `ofs/tests/regress/impl_method_call.ofs` | igual | 0 | 0 |  |
| `ofs/tests/regress/index_assign.ofs` | igual | 0 | 0 |  |
| `ofs/tests/regress/interp_memory.ofs` | igual | 0 | 0 |  |
| `ofs/tests/regress/interp_runtime_error.ofs` | erro esperado | - | 1 | falha nos dois modos (nativo: não compila, interp: rc=1) |
| `ofs/tests/regress/monolith_nested.ofs` | igual | 0 | 0 |  |
| `ofs/tests/regress/neg_null_min.ofs` | igual | 0 | 0 |  |
| `ofs/tests/regress/ptr_widths.ofs` | igual | 0 | 0 |  |
| `ofs/tests/regress/question_propagation.ofs` | igual | 0 | 0 |  |
| `ofs/tests/regress/seal_defer.ofs` | igual | 0 | 0 |  |
| `ofs/tests/regress/seal_reserved_name.ofs` | erro esperado | - | 1 | falha nos dois modos (nativo: não compila, interp: rc=1) |
| `ofs/tests/regress/seal_return_inside.ofs` | erro esperado | - | 1 | falha nos dois modos (nativo: não compila, interp: rc=1) |
| `ofs/tests/regress/short_circuit.ofs` | igual | 0 | 0 |  |
| `ofs/tests/regress/str_alias.ofs` | igual | 0 | 0 |  |
| `ofs/tests/regress/strbuf_basic.ofs` | igual | 0 | 0 |  |
| `ofs/tests/regress/str_char_at_1mb.ofs` | igual | 0 | 0 |  |
| `ofs/tests/regress/type_err_bool_arith.ofs` | erro esperado | - | 1 | falha nos dois modos (nativo: não compila, interp: rc=1) |
| `ofs/tests/regress/type_err_crystal_mod.ofs` | erro esperado | - | 1 | falha nos dois modos (nativo: não compila, interp: rc=1) |
| `ofs/tests/regress/type_err_obsidian_mul.ofs` | erro esperado | - | 1 | falha nos dois modos (nativo: não compila, interp: rc=1) |
| `ofs/tests/regress/type_err_ptr_arith.ofs` | erro esperado | - | 1 | falha nos dois modos (nativo: não compila, interp: rc=1) |
| `ofs/tests/regress/type_numeric_mix.ofs` | igual | 0 | 0 |  |
| `ofs/tests/regress/undefined_ident.ofs` | erro esperado | - | 1 | falha nos dois modos (nativo: não compila, interp: rc=1) |
| `ofs/tests/regress/variant_echo.ofs` | igual | 0 | 0 |  |
| `ofs/tests/release_windows_smoke.ofs` | pulado | - | - | UI/janela/rede (check-ok) |
| `ofs/tests/runtime/test_fault_addr_qsort.ofs` | igual | 0 | 0 |  |
| `ofs/tests/runtime/test_magma_foundation.ofs` | igual | 0 | 0 |  |
| `ofs/tests/runtime/test_magma_full.ofs` | igual | 0 | 0 |  |
| `ofs/tests/runtime/test_magma_input.ofs` | pulado | - | - | stdin/rede (check-ok) |
| `ofs/tests/runtime/test_magma_matrix.ofs` | igual | 0 | 0 |  |
| `ofs/tests/runtime/test_magma_memory.ofs` | igual | 0 | 0 |  |
| `ofs/tests/runtime/test_magma_outlet.ofs` | pulado | - | - | stdin/rede (check-ok) |
| `ofs/tests/runtime/test_magma_prism.ofs` | igual | 0 | 0 |  |
| `ofs/tests/runtime/test_magma_pulse.ofs` | igual | 0 | 0 |  |
| `ofs/tests/runtime/test_magma_strings.ofs` | igual | 0 | 0 |  |
| `ofs/tests/selfhost_stdlib_smoke.ofs` | pulado | - | - | UI/janela/rede (check-ok) |
| `ofs/tests/ui_stdlib_smoke.ofs` | pulado | - | - | UI/janela/rede (check-ok) |
| `ofs/tests/window_runtime_smoke.ofs` | pulado | - | - | UI/janela/rede (check-ok) |
| `tests/parity/cases/bedrock_block.ofs` | igual | 0 | 0 |  |
| `tests/parity/cases/fault_call_basic.ofs` | igual | 0 | 0 |  |
| `tests/parity/cases/fault_intrinsics.ofs` | igual | 0 | 0 |  |
| `tests/parity/cases/libc_calloc.ofs` | igual | 0 | 0 |  |
| `tests/parity/cases/libc_file.ofs` | igual | 0 | 0 |  |
| `tests/parity/cases/libc_snprintf.ofs` | igual | 0 | 0 |  |
| `tests/parity/cases/monolith_memcpy.ofs` | igual | 0 | 0 |  |
| `tests/parity/cases/ptr_stone.ofs` | igual | 0 | 0 |  |
| `tests/parity/cases/ptr_u8.ofs` | igual | 0 | 0 |  |
| `tests/parity/cases/qsort_fault_addr.ofs` | igual | 0 | 0 |  |
