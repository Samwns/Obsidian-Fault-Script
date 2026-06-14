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
from 28,224 to 16,056 bytes without changing its checksum. The launcher also
caches the detected Clang major version instead of probing the toolchain on
every compilation. Median build time for the same workload moved from
320.81 ms to 208.78 ms on the recorded machine.

These are distribution-pipeline improvements. They do not change syntax,
type checking, or program semantics.
