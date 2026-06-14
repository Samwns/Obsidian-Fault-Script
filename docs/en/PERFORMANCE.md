# OFS Performance Analysis

OFS performance work is split between compiler throughput and generated-program runtime.

The main hotspots currently identified are:

1. character access in the lexer repeatedly validating C-string length;
2. substring allocation while materializing tokens;
3. repeated string concatenation in import expansion and escaping;
4. linear lookup tables for symbols, fields, variables, and interned strings;
5. length calls repeated inside loops whose collections do not change.

The first lexer fast-path experiment was not published because the current self-hosted lexer test already crashes in the baseline pipeline. Coverage must be repaired before changing the public string contract.

The planned order is:

1. repair lexer self-host tests;
2. add per-phase compiler timing;
3. benchmark 10 KB, 100 KB, and 1 MB sources;
4. add a compiler-internal known-length character API;
5. introduce token slices or a compilation arena;
6. add a native string builder;
7. measure indexed symbol tables;
8. evaluate LTO and LLVM flags.

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
