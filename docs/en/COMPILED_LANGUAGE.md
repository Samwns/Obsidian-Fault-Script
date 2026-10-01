# OFS: A Truly Hybrid and Native Compiled Language

## The Fundamental Question: Is OFS Interpreted or Compiled?

**Answer: OFS is HYBRID — High-Performance Native AOT Compilation via LLVM and Instant AST Interpretation.**

OFS unites the strengths of both execution models:
1. **Native Ahead-Of-Time (AOT) Compilation (`ofs build`)**: Directly generates LLVM IR from its self-hosted compiler frontend (`ofscc`), yielding machine code linked into standalone platform executables without virtual machines or embedded interpreters.
2. **Instant In-Memory AST Interpretation (`ofs run` / `ofs <file.ofs>`)**: Directly evaluates the typed Abstract Syntax Tree (AST) in memory using an integrated, pure-OFS interpreter backend (`interpreter.ofs`), providing rapid startup and zero compile latency for scripting and experimentation.

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

1. **Instant Feedback**: With `ofs run` or `ofs script.ofs`, code executes immediately in memory without invocation delays from `llc` or linkers.
2. **Production Performance**: With `ofs build`, the resulting executable runs directly on CPU hardware (x86_64, ARM64) with zero VM overhead.
3. **Unified Frontend**: Both the interpreter and LLVM code generator consume the exact same lexer, parser, and static type checker, ensuring semantic consistency.
4. **100% Self-Hosted**: The entire compiler (`ofscc`), including lexer, parser, type checker, LLVM code generator, and AST interpreter, is written in OFS.

---

## Comparative Matrix

| Dimension | OFS (Native) | OFS (Interpreted) | Python | Node.js |
|---|---|---|---|---|
| **Model** | Native AOT Machine Code | AST Walk in memory | Bytecode in VM | JIT Engine (V8) |
| **Execution** | CPU Hardware Direct | Pure-OFS Interpreter | CPython | V8 C++ |
| **Startup Overhead** | ~2 to 10 ms | ~5 to 15 ms | ~40 to 100 ms | ~50 to 150 ms |
| **Base Memory** | Lightweight (~1-4 MB) | Lightweight (~4-8 MB)| Moderate (~20 MB) | High (~50 MB) |
| **Dependencies** | Standalone binary | `ofs` executable | Python interpreter | Node runtime |

---

## Conclusion

OFS is a modern, hybrid, self-contained programming language:

- Instant script and test execution with the AST interpreter (`ofs run`).
- High-performance native binary compilation with LLVM (`ofs build`).
- Frontend 100% self-hosted and authored in OFS.
- Native integrated runtime (Magma Stack).
