# OFS: A Truly Compiled Language

## The Fundamental Question: Is OFS Interpreted or Compiled?

**Answer: OFS is COMPILED.**

OFS is an Ahead-Of-Time (AOT) compiled programming language that directly emits LLVM IR through its self-hosted compiler frontend (`ofscc`), yielding native machine code linked into standalone platform executables.

---

## Compilation Pipeline

```text
┌─────────────────────────────────────────────────────────────────────┐
│ OFS Source Code                                                     │
│ (.ofs)                                                              │
└────────────────────────┬────────────────────────────────────────────┘
                         │
                    ┌────▼─────┐
                    │  Lexer   │ -> Lexical analysis (tokens)
                    └────┬─────┘
                         │ Array<Token>
    ┌────────────────────┘
    │
    ▼
┌─────────────────────────┐
│     parser.ofs          │ -> Syntax parsing
│ (pure OFS, no C++ deps) │
└────────┬────────────────┘
         │ AST (Abstract Syntax Tree)
         │
    ┌────▼─────────┐
    │ Type Checker │ -> Static type validation and inference
    └────┬─────────┘
         │ Annotated AST
         │
     ┌───▼────────────┐
     │    LLVM Gen    │ -> Direct LLVM IR emission (.ll)
     └───┬────────────┘
         │ Native LLVM IR
         │
     ┌───▼───────────────────────────┐
     │  llc -filetype=obj (.ll -> .o)│ -> Native machine object code
     └───┬───────────────────────────┘
         │ Binary object (.o)
         │
     ┌───▼────────────────────────────────────┐
     │  ld / ld.lld (.o + magma.o -> binary)  │ -> Static linkage with runtime
     └───┬────────────────────────────────────┘
         │
     ┌───▼──────────────────┐
     │ Target Executable    │
     │ ELF (Linux)          │
     │ Mach-O (macOS)       │
     │ PE32+ (Windows)      │
     └──────────────────────┘
```

---

## What Defines a Compiled Language?

OFS adheres to the strict criteria of compiled systems languages:

1. **Self-hosted compiler**: The `ofscc` compiler is authored entirely in OFS.
2. **Native machine output**: Execution does not require an interpreter, virtual machine, or runtime bytecode processor.
3. **Discrete compile step**: `ofs build` produces an architecture-specific executable binary prior to program invocation.
4. **Direct hardware execution**: Executables dispatch CPU instructions natively on x86_64 and ARM64.

---

## Comparative Analysis

| Dimension | OFS | Python | JavaScript / Node.js |
|---|---|---|---|
| **Execution Model** | Native machine code (AOT) | Bytecode VM | JIT Engine (V8) |
| **Compiler** | Self-hosted (written in OFS) | CPython (written in C) | V8 (written in C++) |
| **Startup Overhead** | ~2 to 10 ms | ~40 to 100 ms | ~50 to 150 ms |
| **Base Memory Footprint** | Low (~1 to 4 MB) | Moderate (~15 to 30 MB) | High (~30 to 80 MB) |
| **Runtime Dependency** | Standalone executable | Python interpreter | Node.js runtime |

---

## The Compilation Pipeline

The OFS build pipeline executes deterministically:

1. **Direct LLVM IR Emission**: `llvmgen.ofs` translates the typed AST directly into SSA-form LLVM IR, eliminating intermediate transpilations.
2. **LLC Static Compilation**: The LLVM static compiler `llc` converts `.ll` intermediate code into native `.o` object files.
3. **Native Linkage**: The platform linker (`ld` on Linux/macOS, `ld.lld` on Windows) statically links the program object with the pre-compiled native runtime (`magma.o`), outputting a self-contained executable.

---

## Conclusion

OFS is a compiled, self-contained language:

- Direct compilation to native machine code.
- Zero virtual machines or production interpreters.
- Direct LLVM IR generation via the self-hosted frontend.
- Native integrated runtime (Stack Magma).
