# bench/

Benchmarks reproduzíveis do OFS: `fib` (n=35), `loop` (10^8 iterações), `strings` (1 MB), `sort` (quicksort de 100 mil inteiros), `alloc` (1 milhão de alocações) e `empty` (inicialização).

```bash
bash bench/run.sh        # gera bench/RESULTADOS.md e bench/results.json
RUNS=3 INTERP_TIMEOUT=30 MEM_LIMIT_MB=2048 bash bench/run.sh   # rodada mais curta
```

Variáveis: `RUNS=5` (execuções por medida; vale a mediana), `INTERP_RUNS=1`, `INTERP_TIMEOUT=120` (s, OFS interpretado), `NATIVE_TIMEOUT=300` (s, demais), `MEM_LIMIT_MB=2048`.

- O OFS é compilado com o compilador e o runtime do repositório (`ofs/dist/ofscc`, `ofs/dist/magma.o`, `ofs/stdlib`), não com o OFS instalado no sistema.
- `ofs/`, `python/`, `rust/`: programas equivalentes. `expected/`: saída esperada (todos precisam produzir exatamente isso; o script marca "saída difere" se não).
- `c/` (referência em C, compilada com `clang -O2`) fica só local, está no `.gitignore`. Sem `bench/c/`, o script omite a coluna C e avisa "referência em C ausente".
- Go, Rust e Python só entram se estiverem instalados.

## Limite de memória

Cada processo medido roda com `ulimit -v` de `MEM_LIMIT_MB` MB (e com `timeout`). O interpretador do OFS ainda vaza memória (cerca de 3 KB por chamada de função; `fib(25)` já usa uns 750 MB), então sem limite uma rodada do modo interpretado pode travar a máquina por falta de memória. Quando a alocação falha, o runtime do OFS imprime `ofs: sem memoria (falha ao alocar N bytes)` no stderr e sai com código 1; o script reconhece essa mensagem (e as de Python, Rust e libc) e registra o resultado como "memória". Os outros valores de falha são "timeout" (passou do limite de tempo) e "falha" (saiu com erro por outro motivo).

## results.json

`bench/results.json` traz data, máquina (CPU, threads, SO), versões (OFS, clang, rustc, go, python), método (execuções e limites), o tempo de cada linguagem em cada benchmark (`status`: `ok`, `differs`, `timeout`, `memory` ou `failure`; `ms` é `null` quando não há tempo), o RSS máximo em MB (`rss_mb`) e o tamanho do binário vazio (`binary_size_bytes`). O JSON é gerado só com bash; se houver `python3`, o script também confere se ele é válido.

O site lê esse arquivo: `src/site/src/build_site.ofs` copia `bench/results.json` para `benchmark.json` (`docs/`, `docs/assets/`, `src/site/public/`), e a página `benchmark.html` mostra os dados. O runner antigo (`src/site/benchmarks/`, em node) foi removido.
