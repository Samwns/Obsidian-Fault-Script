# OFS Performance Analysis

OFS performance work is split between compiler throughput and generated-program runtime.

The main hotspots currently identified are:

1. substring allocation while materializing tokens;
2. repeated string concatenation in import expansion and escaping;
3. linear lookup tables for symbols, fields, variables, and interned strings;
4. length calls repeated inside loops whose collections do not change.

The public `ofs_str_char_at` function still validates bounds. The lexer now
uses an internal known-length byte read only after checking `_pos` against
`_len`. In seven-process measurements on June 14, 2026, median IR generation
for the small workload moved from 8.11 ms to 7.54 ms. Compiling `ofscc.ofs`
moved from 4,610.40 ms to 3,694.08 ms. Bootstrap stages two and three emitted
identical IR.

The planned order is:

1. repair and expand lexer self-host tests;
2. add detailed lexer, parser, type-checker, and LLVM IR timing;
3. benchmark 10 KB, 100 KB, and 1 MB sources;
4. introduce token slices or a compilation arena;
5. add a native string builder;
6. measure indexed symbol tables;
7. evaluate LTO and LLVM flags.

## Link and runtime optimization

The distributed runtime is built with `-ffunction-sections` and
`-fdata-sections`. The launcher enables `--gc-sections` on ELF, `dead_strip`
on Mach-O, and `OPT:REF` on PE/COFF so unused runtime functions do not remain
in every application.

On the workload published on June 14, 2026, this reduced the OFS executable
from 28,224 to 16,056 bytes without changing its checksum. The launcher caches
CRT paths and native toolchain binaries (llc and ld) instead of slow filesystem probing on
every compilation. Median build time for the same workload moved from
320.81 ms to 208.78 ms on the recorded machine.

These are distribution-pipeline improvements. They do not change syntax,
type checking, or program semantics.

## Goal

The goal is to reduce concrete compile-time and run-time costs of the compiler and runtime without changing language semantics, compromising memory integrity, or introducing silent regressions. Total build time should stay competitive with equivalent native toolchains by removing redundant work, not by removing language features.

## Compilation Flow

```text
OFS source
  -> module resolution and expansion (attach)
  -> lexer
  -> parser
  -> type checker
  -> LLVM IR generation (llvmgen.ofs)
  -> static LLVM compiler (llc -filetype=obj)
  -> native link with the native runtime (ld / ld.lld)
  -> native executable (ELF / Mach-O / PE)
```

Each stage is measured in isolation: optimizing the generated binary does not necessarily reduce lexer time, and lexer improvements do not change the CPU use of the resulting program.

## Hotspots and Optimization Opportunities

### 1. Character reads in the lexer

`ofs_str_char_at` checks bounds against the string length. The lexer already tracks `_pos` against `_len`, so the `ofs_str_char_at_known` variant avoids a redundant `strlen` per character and keeps processing of large sources linear, O(N).

### 2. Substring allocation

The lexer slices text for identifiers, numbers and literals. The optimized strategy is:
- keep `(start, length)` tuples that point into the original file buffer during parsing;
- materialize heap strings only when a syntax node or the symbol table needs them to persist;
- use a per-compilation-unit arena allocator, released in one step at the end.

### 3. String concatenation

Sequential concatenation in loops copies prefixes repeatedly. In the LLVM generator, `cg_emit` writes straight to the output file and avoids temporary buffers.

### 4. Symbol tables and scope lookup

Linear symbol lists have good cache locality for small and medium programs. For large code bases, hash tables or scope-indexed structures reduce the lookup cost for variables and types.

## Measurement Methodology

1. A controlled set of lexical and syntactic tests with sources from 10 KB to 1 MB.
2. Per-phase timing: front end (lexer, parser, types), LLVM IR emission, native link.
3. Real system metrics via `wait4`: user CPU time, system time and peak resident memory (RSS).
4. Strict output checksum verification so no optimization changes semantics.
