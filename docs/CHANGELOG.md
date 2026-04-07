# Changelog

All notable changes to this project are recorded here.

## [Unreleased]
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
