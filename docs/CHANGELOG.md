# Changelog

All notable changes to this project are recorded here.

## [Unreleased]
### Added
- Track 1 language update documented and available in the compiler: small integers (`u8`, `u16`, `u32`, `u64`, `i8`, `i32`), `impl`, namespaces, function values/lambdas, and `window` runtime bridge.
- `window.ofs` standard-library module and `window` package entry for native window/input access.
- `canvas.ofs` standard-library module and `canvas` package entry for pixel-buffer drawing on top of `window`.
- VS Code extension grammar, snippets, hover docs, and README updated to cover Track 1 syntax and the canvas/window workflow.
- New `fault_unreachable()` intrinsic: marks the current code path as unreachable; lowers to `llvm.trap` followed by an LLVM `unreachable` instruction, letting the optimizer eliminate dead branches.
- New `fault_memcpy(dst, src, len)` intrinsic: lowers to `llvm.memcpy` for bulk memory copies inside `fracture`/`abyss`/`bedrock` blocks.
- New `fault_memset(dst, val, len)` intrinsic: lowers to `llvm.memset` for bulk memory initialisation inside low-level blocks.
- Variadic `...` syntax on `extern vein` declarations: `extern vein printf(fmt: obsidian, ...) -> stone` now compiles correctly. The `ELLIPSIS` token (`...`) was added to the lexer and parser.
- Implemented `ofs_webserver_serve_once` and `ofs_webserver_serve_forever` in the runtime (POSIX sockets on Linux/macOS; Winsock on Windows). These were previously declared in the header but had no implementation, causing linker errors in any program that called them.

### Changed
- `match` statement codegen is now fully implemented. Previously only the `default` arm was generated; all `case` arms now compile to a chain of compare-and-branch instructions. String arms use `ofs_str_eq`, numeric arms use integer/float comparison.
- Inline assembly (`asm "..."`) is now correctly lowered to LLVM `InlineAsm`. Previously the `gen_inline_asm` handler existed but was never called from `gen_expr`, so all `asm` statements were silently discarded. Input operands are now forwarded with default register constraints.
- `fault_step` symbol type in the built-in symbol table corrected from `stone` to `*stone` (shard of stone), matching what `check_call` already returned at call sites.
- `register_builtin_symbols` rewritten for clarity: each intrinsic now has its exact return type recorded in a table rather than a catch-all `stone` fallback.

### Fixed
- Windows updater now resolves the latest release tag more robustly by following redirects and falling back to the GitHub API when needed.
- **Critical**: String equality (`==`) and inequality (`!=`) on `obsidian` values now use `ofs_str_eq` (content comparison). Previously they compared raw pointer addresses, so two string literals with equal content could compare as unequal.
- **Critical**: `asm "..."` expressions were never lowered to IR (the dispatch case was missing in `gen_expr`). Fixed by adding the `InlineAsmExpr` branch to `gen_expr` and to `check_expr` in the semantic analyser.
- Semantic check for `void` function returning a value now raises a proper error (`void function cannot return a value`) instead of silently producing invalid LLVM IR that fails verifier checks.
- `gen_return` now has a codegen-level safety guard: if a void function somehow passes semantic analysis with a return value expression, codegen evaluates the expression for side effects and emits `ret void`, preventing an LLVM verifier crash.
- `examples/baremetal_minimal.ofs` corrected: `vein main()` (void, no core entry) with `return 42` was replaced with a proper `core main()` + helper function structure.
- `gen_inline_asm` previously ignored `inputs` entirely (empty constraint string) and used only the first output name as the constraint. Now builds the constraint string from outputs, per-input `"r"` constraints, and clobber sections.

## [Unreleased — previous]
### Added
- New `ofs asm <file.ofs>` command to emit target-native assembly from the current LLVM backend.
- Low-level roadmap document focused on OFS-native coexistence between high-level and low-level code in the same program.
- `bedrock` standard-library module with OFS-first heap cell helpers for manual state handling.
- VS Code extension command to emit native assembly from the active OFS file.
- Expanded language direction docs covering OFS-native low-level goals and foreign interop roadmap.
- New `bedrock { ... }` typed low-level block in the language.
- New `rift vein` declaration form for OFS-native foreign interop boundaries.
- New `fault_*` intrinsics for machine-like low-level operations with OFS-native naming.
- New `fault_step` intrinsic for typed pointer stepping.
- New `bedrock_region_*` helpers for explicit low-level storage regions.
- New `rift.ofs` module for OFS-native wrappers around foreign runtime boundaries.
- New `fault_cut` and `fault_patch` intrinsics for field extraction and patching.
- New `bedrock_view_*` helpers for OFS-native low-level views over regions.
- New `fault_fence` and `fault_trap` intrinsics for machine-level barrier/trap hooks.
- New `fault_prefetch` intrinsic and `bedrock_prefetch` helper for cache-aware pointer staging in low-level OFS code.
- New lane helpers such as `bedrock_lane8_get` and `bedrock_lane8_set`.
- New endian-aware lane helpers such as `bedrock_lane16_le_get`, `bedrock_lane16_be_get`, `bedrock_lane32_le_set`, and `bedrock_lane32_be_set`.
- New `bedrock_packet.ofs` module for packet/header views over low-level regions.
- New packet helpers for endian-aware header windows such as `bedrock_packet_opcode_be16` and `bedrock_packet_tail_le16`.
- New `bind`/`abi` metadata on `rift vein` and `extern vein` declarations for more explicit interop boundaries.
- New `layout native|packed|c` support on `monolith` declarations.

- Low-level roadmap document focused on OFS-native coexistence between high-level and low-level code in the same program.
- `bedrock` standard-library module with OFS-first heap cell helpers for manual state handling.
- VS Code extension command to emit native assembly from the active OFS file.
- Expanded language direction docs covering OFS-native low-level goals and foreign interop roadmap.
- New `bedrock { ... }` typed low-level block in the language.
- New `rift vein` declaration form for OFS-native foreign interop boundaries.
- New `fault_*` intrinsics for machine-like low-level operations with OFS-native naming.
- New `fault_step` intrinsic for typed pointer stepping.
- New `bedrock_region_*` helpers for explicit low-level storage regions.
- New `rift.ofs` module for OFS-native wrappers around foreign runtime boundaries.
- New `fault_cut` and `fault_patch` intrinsics for field extraction and patching.
- New `bedrock_view_*` helpers for OFS-native low-level views over regions.
- New `fault_fence` and `fault_trap` intrinsics for machine-level barrier/trap hooks.
- New `fault_prefetch` intrinsic and `bedrock_prefetch` helper for cache-aware pointer staging in low-level OFS code.
- New lane helpers such as `bedrock_lane8_get` and `bedrock_lane8_set`.
- New endian-aware lane helpers such as `bedrock_lane16_le_get`, `bedrock_lane16_be_get`, `bedrock_lane32_le_set`, and `bedrock_lane32_be_set`.
- New `bedrock_packet.ofs` module for packet/header views over low-level regions.
- New packet helpers for endian-aware header windows such as `bedrock_packet_opcode_be16` and `bedrock_packet_tail_le16`.
- New `bind`/`abi` metadata on `rift vein` and `extern vein` declarations for more explicit interop boundaries.
- New `layout native|packed|c` support on `monolith` declarations.

## [1.0.3] - 2026-04-01
### Added
- New keywords: `match`, `case`, `default`, `const`, `strata`, `tremor`, `catch`, `throw`.
- New operators: `<<`, `>>`, `%=`.
- Bitwise operator support in binary expressions (`&`, `|`, `^`, `<<`, `>>`).
- New AST nodes for:
  - Pattern matching (`MatchStmt`, `MatchArm`)
  - Immutable variables (`ConstStmt`)
  - Enumerations (`StrataDecl`)
  - Error handling (`TremorStmt`, `ThrowStmt`)

### Changed
- Parser updated to recognise `strata`, `const`, `match/case/default`, `tremor/catch`, `throw`.
- Semantic analyser with stubs and basic validation for new constructs.
- Codegen stubs for new constructs and bitwise/shift expression support.
- Lexer and parser tests extended for new tokens and syntax.
- Release CI/CD improved for Linux/Windows/macOS with better LLVM detection.

### Fixed
- Windows workflow: no longer fails when a newer LLVM is already installed and a downgrade is attempted.
- Linux workflow: `LLVM_DIR` now set explicitly to avoid picking up an older LLVM on the runner.
- CMake: LLVM backend chosen by architecture (x86_64 vs arm64).
- Codegen: compatibility with LLVM 15/17+ for `Reloc::Model` (`llvm::Optional` vs `std::optional`).

## [1.0.2] - 2026-04-01
### Added
- Automated release pipeline with binaries for Linux, Windows, and macOS.

## [1.0.1] - 2026-04-01
### Changed
- README simplified for quick installation and usage.

## [1.0.0] - 2026-04-01
### Added
- First stable release of the OFS language (lexer, parser, semantic analysis, LLVM codegen, runtime, and examples).
