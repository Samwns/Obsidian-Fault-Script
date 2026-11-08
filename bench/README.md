# bench/

Benchmarks reproduzíveis do OFS: `fib` (n=35), `loop` (10^8 iterações), `strings` (1 MB), `sort` (quicksort de 100 mil inteiros), `alloc` (1 milhão de alocações) e `empty` (inicialização).

```bash
bash bench/run.sh        # gera bench/RESULTADOS.md e bench/results.json
```

Variáveis: `RUNS=5`, `INTERP_RUNS=1`, `INTERP_TIMEOUT=120`.

- `ofs/`, `python/`, `rust/`: programas equivalentes. `expected/`: saída esperada (todos precisam produzir exatamente isso; o script marca "saída difere" se não).
- `c/` (referência em C, compilada com `clang -O2`) fica só local, está no `.gitignore`. Sem `bench/c/`, o script omite a coluna C e avisa "referência em C ausente".
- Go, Rust e Python só entram se estiverem instalados.
- `src/site/benchmarks/` é o runner antigo da página do site (outro formato, `results.json` próprio); não foi alterado.

Notas: o `sort` usa `ofs_array_set(a, i, v)` porque a atribuição `a[i] = v` não tem efeito no build nativo hoje (bug conhecido do compilador, ver relatório).
