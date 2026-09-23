# OFS Self-Hosting Bootstrap Guide

The OFS compiler is fully self-hosted and deterministic. The bootstrap process compiles the compiler using itself, without depending on external C++ compilers or legacy runtimes.

---

## Quick Start

To bootstrap the native compiler in the repository:

```bash
bash ofs/bootstrap/scripts/bootstrap-minimal.sh
```

Once built, the compiler binary will reside in `ofs/dist/ofs` (or `ofs/dist/ofscc`).

Compile a program:

```bash
ofs/dist/ofs build ofs/examples/hello.ofs -o hello
./hello
```

---

## What is Self-Hosting / Bootstrap?

**Bootstrapping** occurs when a programming language's compiler compiles its own source code. This milestone formally proves that the language's grammar, type system, and standard library are expressive and robust enough for systems-level development.

- **Lisp** (1962): First self-hosting programming language.
- **C** (1972): Bootstrapped and established as the systems standard.
- **Rust** (2011): Compiler written in Rust.
- **Go** (2015): Compiler migrated completely to Go.
- **OFS** (2024–2026): Compiler and native runtime self-hosted.

---

## Process Overview

### Phase 1: Seed Compiler
The repository provides a pre-compiled native compiler binary in `ofs/dist/` (`ofs` / `ofscc`) to initiate the build cycle.

### Phase 2: Recompilation
The script compiles all compiler source modules (`ofs/ofscc/*.ofs`) using the existing binary, emitting a fresh compiler executable.

### Phase 3: Determinism Verification
The newly generated compiler compiles the compiler source once more. The binary output from pass 2 is compared byte-for-byte with pass 3 (`v2 === v3`). Identical checksums confirm deterministic build reproducibility.

---

## Usage

### 1. Standard Minimal Bootstrap

```bash
cd Obsidian-Fault-Script
bash ofs/bootstrap/scripts/bootstrap-minimal.sh
```

**Expected output**:
```text
[OK] ofs/dist/ofscc built successfully
[OK] Checksum verified
[OK] Determinism validated
```

### 2. Full Determinism Validation Cycle

To execute a complete triple-stage validation pass:

```bash
bash ofs/bootstrap/scripts/bootstrap-minimal.sh --validate
```

The script builds:
1. `ofs/dist/ofscc` (stage 1)
2. `ofs/dist/ofscc` (stage 2, produced by stage 1)
3. `ofs/dist/ofscc` (stage 3, produced by stage 2)
4. Strict byte comparison: `cmp -s v2 v3`

---

## Troubleshooting

### 1. Permission Denied on Script Execution

```bash
chmod +x ofs/bootstrap/scripts/bootstrap-minimal.sh
bash ofs/bootstrap/scripts/bootstrap-minimal.sh
```

### 2. Execution Permission on Emitted Binary

```bash
chmod +x ofs/dist/ofs ofs/dist/ofscc
```

### 3. Determinism Mismatch

If `v2 != v3`, run with the `--debug` flag to capture the diagnostic log:

```bash
bash ofs/bootstrap/scripts/bootstrap-minimal.sh --debug
```

---

## Pipeline Architecture

```text
OFS Source Code (.ofs)
  │
  ▼
[Native OFS Compiler: ofs/dist/ofscc]
  │ Lexical analysis and parsing (lexer / parser)
  │ Static type checking
  │ LLVM IR emission (.ll)
  ▼
[LLVM Static Compiler llc]
  │ Machine object emission (.o)
  ▼
[System Linker ld / ld.lld]
  │ Static linkage with native runtime (magma.o)
  ▼
Target Executable (ELF on Linux, Mach-O on macOS, PE32+ on Windows)
```

---

## Component Layout

- `ofs/dist/ofs` / `ofs/dist/ofscc`: Compiled compiler binary.
- `ofs/dist/magma.o`: Compiled native runtime object.
- `ofs/ofscc/`: Compiler source written in OFS.
- `ofs/stdlib/`: Language standard library modules.
- `ofs/stdlib/runtime/`: Native runtime source code (Stack Magma).
