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
5. add a native string builder (`strbuf` already exists in the stdlib);
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

## Measured run-time benchmarks

Equivalent programs in OFS, Rust and Python (plus a local C reference) live in [`bench/`](../../bench/). Reproduce with `bash bench/run.sh` (it measures the repository compiler, `ofs/dist`, and writes [`bench/RESULTADOS.md`](../../bench/RESULTADOS.md)). Measured on an AMD Ryzen 5 5600GT, OFS 1.0.169, median of 5 runs (interpreted: 1 run), 2,048 MB memory limit per process, wall time in ms:

| Benchmark | Native OFS | Interpreted OFS | C -O2 | Rust | Python |
|---|---|---|---|---|---|
| empty (startup) | 3 | 37 | 2 | 2 | 12 |
| fib (n=35) | 41 | 5,997 | 22 | 23 | 906 |
| loop (10^8) | 410 | 18,135 | 276 | 277 | 11,031 |
| strings (1 MB) | 15 | 589 | 196 | 199 | 272 |
| sort (100k) | 15 | 925 | 7 | 7 | 164 |
| alloc (1 million) | 17 | 751 | 7 | 9 | 150 |

Peak memory (RSS, MB) and empty binary:

| Language | Empty binary (bytes) | empty | fib | loop | strings | sort | alloc |
|---|---|---|---|---|---|---|---|
| OFS | 36,624 | 5.0 | 5.2 | 5.4 | 5.8 | 6.3 | 5.1 |
| C | 12,568 | 2.2 | 1.9 | 1.9 | 3.5 | 2.3 | 2.2 |
| Rust | 10,440,304 | 2.0 | 2.2 | 2.2 | 4.1 | 2.7 | 2.0 |
| Python | n/a (interpreter) | 9.2 | 9.4 | 9.5 | 11.6 | 13.4 | 9.5 |

All interpreted-mode outputs match the native ones.

History: before the "A6" memory optimization, `strings` took 223 ms and 520 MB, `alloc` 116 ms and 97 MB, `sort` 86 ms, and `ofs run` hit the 2,048 MB limit on everything except the empty program. An even older run mistakenly measured an old installed compiler, which produced a 10,404 ms `strings` figure; it does not apply to the repository compiler.

Honest reading (machine: AMD Ryzen 5 5600GT, Linux, OFS 1.0.169, measured on 2026-11-08):

- Native OFS **still loses to C and Rust on compute and allocation tests**: about 1.5x in `loop`, 1.9x in `fib`, 2.1x in `sort` and 2.4x in `alloc`.
- **`strings` is a special case, not a comparison with an optimized C.** The C reference (`bench/c/strings.c`) does `malloc` + `memcpy` on every concatenation, copying everything (O(n^2)), and that is what OFS used to do (223 ms, 520 MB). Now OFS grows the string in place (`realloc`) when the compiler proves, by per-function escape analysis, that the old value is seen by nobody else; aliases such as `t = s` remain correct. That is why OFS (15 ms, 5.8 MB) beats C (196 ms, 3.5 MB) here, by about 13x. A C program with a growing buffer would do the same; the stdlib has `strbuf` for explicit control.
- `alloc` dropped from 116 ms / 97 MB to 17 ms / 5.1 MB (C: 7 ms): local arrays are freed at the end of their scope and allocation is inline. `sort` dropped from 86 to 15 ms (C: 7 ms): `a[i]` and `a[i] = v` are inlined in the generated code.
- It beats Python in `fib` (22x), `loop` (27x), `strings` (18x), `sort` (11x) and `alloc` (9x).
- Startup is 3 ms (Python: 12 ms) and the empty binary is 36,624 bytes (Rust: about 10 MB). Base memory is higher than C: 5.0 MB against 2.2 MB.
- Interpreted mode (`ofs run`) now runs every benchmark within the memory limit (the leak of about 3 KB per call was fixed; the interpreter was rewritten with registers and AST tables). It is still much slower than native and generally slower than Python: 6.6x slower in `fib`, 1.6x in `loop`, 2.2x in `strings`, 5.6x in `sort` and 5.0x in `alloc`. Use `ofs run` for scripts and development; use `ofs build` for performance.
- Compiling the compiler itself (self-host) dropped from 197 MB to 45 MB of RAM.

To reproduce: `bash bench/run.sh`. Full numbers are in [`bench/RESULTADOS.md`](../../bench/RESULTADOS.md).

## Consistency between `ofs run` and `ofs build`

The interpreter and the LLVM generator share the lexer, parser and type checker, but execution still diverges: `ofs run` is a subset (no `extern` outside the builtin list, no pointers). The plan is to reach parity; until then `ofs build` is the reference.
