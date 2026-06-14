# OFS Website Benchmark

Run the complete local suite:

```bash
bash src/site/benchmarks/run.sh
```

The suite builds and measures OFS, C, C++, Java, C#, Python, and Rust when
their toolchains are available. Results are written to
`src/site/benchmarks/results.json`.

Measured dimensions:

- CPU execution time
- peak resident memory
- process startup
- compilation or bytecode generation
- primary artifact size
- checksum correctness

The native `runner.c` helper uses `wait4` and `getrusage`, so time and peak RSS
come from the child process rather than estimates made in browser JavaScript.

The suite uses five fresh processes for CPU, memory, and compilation, plus
fifteen processes for startup. Every implementation must emit the same
checksum before its measurements are accepted.
