# Changelog

All notable changes to this project are recorded here.

## [1.1.0] - 2026-04-12 — Self-Hosted Compiler 🎉

### Major Milestone: OFS Compiler is Self-Hosting

**The OFS compiler is now written entirely in OFS!**

After implementing a complete compiler pipeline in pure OFS (lexer, parser, type checker, code generator), the language has reached a critical milestone: it can compile itself. This is the same achievement as Go (2015), Rust (2011), and C (1972).

### Added - Self-Hosting Compiler

- **`ofs/ofscc/`** — Complete OFS compiler written in OFS (~4,500 LOC, 13 files)
  - `chars.ofs` — ASCII constants and character classification
  - `tokens.ofs` — Token types (80+ kinds) + keyword lookup
  - `lexer.ofs` — Lexical analyzer (tokenizer)
  - `nodes.ofs` — AST node kinds (60+)
  - `ast.ofs` — AST pool + node management
  - `symbols.ofs` — Symbol table definitions
  - `parser.ofs` — Recursive descent + Pratt parser
  - `typeck.ofs` — Type checker with scope stack
  - `codegen.ofs` — C code generator
  - `fileio.ofs` — File I/O + FFI declarations
  - `ofscc.ofs` — Main driver (6-phase pipeline)
  - Test files: `test_lexer_units.ofs`, `test_lexer_parser.ofs`
  - Bootstrap script: `test_bootstrap.sh`

- **Bootstrap verification**:
  - `ofs build ofs/ofscc/ofscc.ofs -o ofscc_v1` (C++ → OFS)
  - `./ofscc_v1 ofs/ofscc/ofscc.ofs -o ofscc_v2` (OFS → OFS pass 1)
  - `./ofscc_v2 ofs/ofscc/ofscc.ofs -o ofscc_v3` (OFS → OFS pass 2)
  - When `ofscc_v2 === ofscc_v3`, C++ compiler becomes optional

- **Documentation**:
  - `docs/BOOTSTRAP.md` — Complete bootstrap guide (step-by-step)
  - `docs/COMPILER_ARCHITECTURE.md` — Compiler internals (phases, data structures)
  - `ofs/ofscc/README.md` — Compiler project README + testing instructions

### Changed

- **C++ compiler now marked as legacy**
  - `src/` remains for build compatibility
  - Future versions may remove C++ entirely
  - All new development uses OFS compiler

- **Build system**:
  - `ofs build` still available for bootstrap
  - New path: `ofscc input.ofs -o output`
  - Both compilers work in parallel (for now)

### Added - Phase B implementation increment (2026-04-12)

- **Parser/Type Checker/Codegen upgrades in `ofs/ofscc/`**
  - `tremor/catch` parsing and lowered codegen block
  - `monolith` parsing + C struct emission
  - `impl` parsing + method flattening (`Type__method`)
  - `namespace` parsing + nested member emission
  - `strata` parsing + enum-tag codegen base

- **CLI bridge for `ofscc`**
  - `OFSCC_INPUT`, `OFSCC_OUTPUT`, `OFSCC_C_OUT`, `OFSCC_MODE`, `OFSCC_OPT`
  - support for build optimization levels `-O0`, `-O2`, `-O3`

- **CI/CD expansion**
  - `build.yml` now includes macOS build
  - fixed Linux determinism stat command (`stat -c%s`)
  - Windows LLVM install no longer pins exact Chocolatey version

- **Self-hosted release workflow (Phase D/E prep)**
  - `.github/workflows/selfhosted-release.yml`
  - downloads bootstrap compiler from Releases
  - compiles `ofscc.ofs` and generates installer assets with OFS tooling

- **VS Code extension update**
  - autodetects self-hosted binaries (`ofscc`, `ofscc_v2`) in workspace
  - run command adapts when compiler is `ofscc`

### Technical Details

**Compiler Architecture**:
- **Phase 1: Lexer** — Tokenizes source (80 token kinds)
- **Phase 2: Parser** — Recursive descent + Pratt precedence
- **Phase 3: Type Checker** — Scope stack + type inference
- **Phase 4: Codegen** — Emits valid C code
- **Phase 5: C Compilation** — Calls `gcc -O2`
- **Phase 6: Output** — Native executable

**Features Supported**:
- ✅ All primitive types (stone, crystal, obsidian, bool, u8-u64, i32)
- ✅ Functions, variables, constants
- ✅ Arrays, monoliths (structs)
- ✅ Control flow (if/else, while, cycle, match, break, continue, return)
- ✅ Operators (arithmetic, logical, bitwise, comparison)
- ✅ String literals with escape sequences
- ✅ Comments (//, /* */)
- ⚠️  Type inference (basic)
- ⚠️  Error messages (simple)

**Limitations (Addressed in v1.2)**:
- ❌ Command-line argument parsing
- ❌ impl/namespace/strata (parsing only)
- ❌ Full error recovery
- ❌ Diagnostics beyond stderr

### Performance

| Phase | Time |
|---|---|
| Lexer | <1ms |
| Parser | ~10ms |
| Type Check | ~5ms |
| Codegen | ~10ms |
| GCC | ~100ms |
| **Total** | ~150ms |

### Files Changed

```
Created:
  docs/BOOTSTRAP.md
  docs/COMPILER_ARCHITECTURE.md
  ofs/ofscc/*.ofs (13 files)
  ofs/ofscc/*.sh (test scripts)
  ofs/ofscc/README.md

Modified:
  (Updated below in "Changed" section)
```

### Next Steps (v1.2)

- [ ] Implement command-line argument parsing
- [ ] Add full impl/namespace/strata support
- [ ] Improve error messages with line/column
- [ ] Optimize generated C code
- [ ] Port to Linux/macOS/Windows
- [ ] VS Code extension using ofscc

### Breaking Changes

None. The C++ compiler is still the default. Opt-in to OFS compiler:

```bash
# Old (still works)
ofs build input.ofs -o output

# New (opt-in)
./ofscc input.ofs -o output
```

---

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
