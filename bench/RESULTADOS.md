# Resultados dos benchmarks

- Data: 2026-11-17
- CPU: AMD Ryzen 5 5600GT with Radeon Graphics (12 threads)
- SO: Linux 7.3.4-201.nobara.fc44.x86_64
- OFS: 1.0.169 (ofs/dist/ofscc + ofs/dist/magma.o do repositório)
- clang: clang version 22.1.8 (Fedora 22.1.8-4.fc44)
- rustc: rustc 1.99.0 (b940084d7 2026-09-28) (Fedora 1.99.0-1.fc44)
- python: Python 3.14.8
- Método: mediana de 5 execuções do tempo de parede (ms); OFS interpretado: 1 execução(ões), limite de 120s; nativos: limite de 300s; limite de 2048 MB de memória (ulimit -v) por processo.
- Reproduzir: `bash bench/run.sh`

## Tempo de parede (ms, menor é melhor)

| Benchmark | OFS nativo | OFS interpretado | OFS cristalizado | C -O2 | rust | python |
|---|---|---|---|---|---|---|
| empty | 4 | 43 | 43 | 2 | 2 | 13 |
| fib | 43 | 6435 | 79 | 23 | 24 | 897 |
| loop | 404 | 18295 | 318 | 273 | 273 | 11664 |
| strings | 16 | 601 | 299 | 196 | 197 | 279 |
| sort | 16 | 989 | 123 | 7 | 8 | 172 |
| alloc | 18 | 838 | 70 | 7 | 9 | 152 |

## Tamanho do binário (bytes, programa vazio) e memória máxima (MB)

| Linguagem | Binário vazio | RSS empty | RSS fib | RSS loop | RSS strings | RSS sort | RSS alloc |
|---|---|---|---|---|---|---|---|
| ofs | 42192 | 5.2 | 5.1 | 5.1 | 5.9 | 6.1 | 5.2 |
| c | 12568 | 2.1 | 2.0 | 2.1 | 3.2 | 2.4 | 2.1 |
| rust | 10440304 | 2.1 | 2.1 | 2.1 | 4.0 | 2.5 | 2.1 |
| python | n/a (interpretador) | 9.5 | 9.4 | 9.5 | 11.3 | 13.5 | 9.4 |

A linha `empty` da tabela de tempo é o tempo de inicialização (programa que só imprime `ok`).
"memória" = o processo estourou o limite de 2048 MB; "timeout" = passou do limite de tempo; "falha" = saiu com erro sem ser por memória.
