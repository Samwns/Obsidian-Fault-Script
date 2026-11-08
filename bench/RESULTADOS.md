# Resultados dos benchmarks

- Data: 2026-11-08
- CPU: AMD Ryzen 5 5600GT with Radeon Graphics (12 threads)
- SO: Linux 7.3.1-201.nobara.fc44.x86_64
- OFS: 1.0.169
- clang: clang version 22.1.8 (Fedora 22.1.8-4.fc44)
- rustc: rustc 1.99.0 (b940084d7 2026-09-28) (Fedora 1.99.0-1.fc44)
- python: Python 3.14.8
- Método: mediana de 5 execuções do tempo de parede (ms); OFS interpretado: 1 execução(ões), limite de 60s.
- Reproduzir: `bash bench/run.sh`

## Tempo de parede (ms, menor é melhor)

| Benchmark | OFS nativo | OFS interpretado | C -O2 | rust | python |
|---|---|---|---|---|---|
| empty | 3 | 40 | 2 | 2 | 13 |
| fib | 43 | falha (saída 1, sem resultado) | 23 | 23 | 918 |
| loop | 406 | falha (saída 1, sem resultado) | 282 | 281 | 11027 |
| strings | 10404 | falha (saída 1, sem resultado) | 201 | 200 | 283 |
| sort | 86 | falha (saída 1, sem resultado) | 8 | 8 | 176 |
| alloc | 117 | falha (saída 1, sem resultado) | 7 | 9 | 154 |

## Tamanho do binário (bytes, programa vazio) e memória máxima (MB)

| Linguagem | Binário vazio | RSS vazio | RSS fib | RSS strings | RSS sort | RSS alloc |
|---|---|---|---|---|---|---|
| ofs | 28480 | 5.1 | 5.5 | 520.1 | 6.2 | 96.9 |
| c | 12568 | 2.1 | 1.9 | 3.4 | 2.3 | 2.0 |
| rust | 10440304 | 2.1 | 1.9 | 3.6 | 3.0 | 2.0 |
| python | n/a (interpretador) | 9.3 | 9.5 | 11.7 | 13.6 | 9.5 |

A linha `empty` da tabela de tempo é o tempo de inicialização (programa que só imprime `ok`).
