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
