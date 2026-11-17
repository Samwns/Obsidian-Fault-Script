# OFS: A Truly Hybrid and Native Compiled Language

## The Fundamental Question: Is OFS Interpreted or Compiled?

**Answer: OFS is HYBRID — High-Performance Native AOT Compilation via LLVM and Instant AST Interpretation.**

OFS unites the strengths of both execution models:
1. **Native Ahead-Of-Time (AOT) Compilation (`ofs build`)**: Directly generates LLVM IR from its self-hosted compiler frontend (`ofscc`), yielding machine code linked into standalone platform executables without virtual machines or embedded interpreters.
2. **Instant In-Memory AST Interpretation (`ofs run` / `ofs <file.ofs>`)**: Directly evaluates the typed Abstract Syntax Tree (AST) in memory using an integrated, pure-OFS interpreter backend (`interpreter.ofs`), supporting scripting and experimentation; hybrid native code may require compilation on a cache miss.


## Execution model: interpreted and compiled with the same semantics

The golden rule compares `ofs run program.ofs` with `ofs build program.ofs` followed by execution of the binary: output, exit status and observable effects should match. A shared lexer, parser and type checker provide the foundation; tests provide the evidence. Run `bash tests/parity/run.sh` and inspect the [parity matrix](https://github.com/Samwns/Obsidian-Fault-Script/blob/main/tests/parity/MATRIZ.md), which distinguishes matching programs, differences, expected errors and skipped cases. A difference is a bug or a documented limitation, rather than a second language semantics.

`ofs run` evaluates the typed AST. `extern` and `rift vein` call native symbols resolved by the Rift; pointers are real addresses and callbacks let C reenter an interpreted vein. `ofs build` emits LLVM IR and links an executable independent of the interpreter. `ofs run --native` compiles and executes the whole program in a temporary directory.

Programs using `quartz` run in hybrid mode: the interpreter identifies veins containing assembly, asks `llvmgen` for a module containing their dependency closure, and loads a native library into the same process. Other veins start interpreted and may later be promoted by crystallization. Functions called by the closure have native versions in the module; interpreted vein values passed to native code use callback trampolines. Scalar globals are synchronized on native entry, exit and callbacks; strings, arrays and objects share memory according to the representations supported by the Rift. The cache hashes the IR and compilation command (`OFS_QUARTZ_CACHE`, or `$XDG_CACHE_HOME/ofs/quartz`, or `~/.cache/ofs/quartz`). `OFS_VERBOSE=1` reports compilation or cache reuse. A cache miss requires `clang`; `OFS_QUARTZ_CC` selects the compiler.

The hybrid boundary inherits FFI dispatcher limits: up to 12 integer/pointer arguments or up to 4 arguments including `crystal`/`f32`; floating-point callbacks use the `__ofs_cbx_K` trampolines (outside Windows x64). The same native logic can approach build performance once compiled, but AST interpretation, library compilation and boundary crossings have costs. Performance equivalence requires measurements that distinguish warmup from cached execution.

### Crystallization: hot veins become native

By default, `ofs run` counts calls to each eligible vein. At `OFS_CRYSTALLIZE_THRESHOLD` (default `1000`, an integer from 1 to 1000000000), it generates IR for a single module containing veins with supported signatures and their dependencies. `clang -O2` compiles the library in the background while interpretation continues; later calls poll for completion and switch to native dispatch. IR generation is synchronous. Batch promotion maintains one native copy of globals. With `quartz`, the required module is compiled at load time; other eligible veins gain native dispatch when the heat trigger fires.

`OFS_CRYSTALLIZE=0` disables optional promotion while preserving hybrid `quartz`. `OFS_VERBOSE=1` reports promotion and optional failures. Without `clang`, or if optional compilation fails, interpretation continues; a vein requiring `quartz` still requires native compilation. Signatures lacking supported trampolines, such as `f32` at the boundary, are not promoted as roots. A long loop inside one call (the `while` in `main`, for instance) switches mode in the middle (**OSR**, wave 4): after `OFS_CRYSTALLIZE_THRESHOLD` iterations the loop is extracted into a native function, compiled in the background, and continues there with the live variables transferred; `bench/ofs/loop.ofs` drops from 18.6 s to 0.34 s (the native build takes 0.41 s). Only `while` loops without `return`/`throw`/`?`/`&`/geode in the body; `OFS_OSR=0` turns it off. The cache avoids later compilations but does not remove IR-generation and library-loading costs. Compare cold, cached and AOT-binary times before claiming equivalent speed.

### Safe low-level code

Safe code can call typed veins without a low-level block. Use `fracture` (or `bedrock`) for pointer operations such as `&x` and `*p`, and `abyss` for raw function addresses, raw `fault_call` and `quartz`. The type checker enforces the same rule in both modes: `OFS_STRICT_FAULT=1` makes violations errors; the default migration mode warns until 2027-01-09. These blocks authorize operations but do not automatically validate an address or its lifetime. See the [language reference](LANGUAGE_REFERENCE.md) for FFI and callback limits.

---

## Dual-Backend Architecture

```text
┌─────────────────────────────────────────────────────────────────────┐
│ OFS Source Code (.ofs)                                              │
└────────────────────────┬────────────────────────────────────────────┘
                         │
                    ┌────▼─────┐
                    │  Lexer   │ -> Lexical analysis (tokens)
                    └────┬─────┘
                         │ Array<Token>
                         ▼
                    ┌──────────┐
                    │  Parser  │ -> Syntactic parsing (pure OFS AST)
                    └────┬─────┘
                         │ AST
                         ▼
                    ┌──────────┐
                    │ Typeck   │ -> Static type checking & semantic validation
                    └────┬─────┘
                         │ Typed & Annotated AST
         ┌───────────────┴───────────────┐
         │                               │
[ofs run / ofs <file.ofs>]        [ofs build <file.ofs>]
         │                               │
         ▼                               ▼
┌──────────────────┐            ┌──────────────────┐
│ interpreter.ofs  │            │   llvmgen.ofs    │ -> LLVM IR emission (.ll)
│ Pure-OFS AST     │            └────────┬─────────┘
│ Interpreter      │                     │ LLVM IR
└────────┬─────────┘                     ▼
         │                      ┌──────────────────┐
  Immediate run in              │       llc        │ -> Native machine code (.o)
      memory                    └────────┬─────────┘
                                         │ Object file (.o)
                                         ▼
                                ┌──────────────────┐
                                │ ld / ld.lld      │ -> Static link with magma.o
                                └────────┬─────────┘
                                         │
                                ┌────────▼─────────┐
                                │ Native Executable│
                                │ ELF / Mach-O / PE│
                                └──────────────────┘
```

---

## What Does the Hybrid Model Provide?

1. **Instant Feedback**: With `ofs run` or `ofs script.ofs`, ordinary code executes directly in memory; hybrid execution may compile native veins on first use.
2. **Production Performance**: With `ofs build`, the resulting executable runs directly on CPU hardware (x86_64, ARM64) with zero VM overhead.
3. **Shared frontend**: both modes use the same lexer, parser and type checker; the golden rule and matrix above verify execution parity.
4. **100% Self-Hosted**: The entire compiler (`ofscc`), including lexer, parser, type checker, LLVM code generator, and AST interpreter, is written in OFS.

---

## Comparative Matrix

| Dimension | OFS (Native) | OFS (`run`) | Python 3.14 |
|---|---|---|---|
| **Model** | AOT via LLVM | AST + native crystallization | Bytecode in VM |
| **Startup (measured)** | 3 ms | 40 ms | 13 ms |
| **Empty-program RSS (measured)** | 5.0 MB | n/a | 9.5 MB |
| **Empty binary** | 37 KB | n/a | n/a |
| **Dependencies** | System libraries used by the program | `ofs`; `clang` for uncached native modules | Interpreter |

Measured on 2026-11-15 using a Ryzen 5 5600GT (median of three runs). Full results and methodology: [bench/RESULTADOS.md](https://github.com/Samwns/Obsidian-Fault-Script/blob/main/bench/RESULTADOS.md). Reproduce with `bash bench/run.sh`. Node.js was not measured.

---

## Conclusion

OFS is a modern, hybrid, self-contained programming language:

- Instant script and test execution with the AST interpreter (`ofs run`).
- High-performance native binary compilation with LLVM (`ofs build`).
- Frontend 100% self-hosted and authored in OFS.
- Native integrated runtime (Magma Stack).
