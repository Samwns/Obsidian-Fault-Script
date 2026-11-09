# Resultados dos benchmarks

- Data: 2026-11-09
- CPU: AMD Ryzen 5 5600GT with Radeon Graphics (12 threads)
- SO: Linux 7.3.1-201.nobara.fc44.x86_64
- OFS: 1.0.169 (ofs/dist/ofscc + ofs/dist/magma.o do repositório)
- clang: clang version 22.1.8 (Fedora 22.1.8-4.fc44)
- rustc: rustc 1.99.0 (b940084d7 2026-09-28) (Fedora 1.99.0-1.fc44)
- python: Python 3.14.8
- Método: mediana de 5 execuções do tempo de parede (ms); OFS interpretado: 1 execução(ões), limite de 120s; nativos: limite de 300s; limite de 2048 MB de memória (ulimit -v) por processo.
- Reproduzir: `bash bench/run.sh`

## Tempo de parede (ms, menor é melhor)

| Benchmark | OFS nativo | OFS interpretado | C -O2 | rust | python |
|---|---|---|---|---|---|
| empty | 3 | 36 | 2 | 2 | 12 |
| fib | 41 | 6035 | 22 | 22 | 908 |
| loop | 405 | 18126 | 275 | 275 | 11300 |
| strings | 17 | 586 | 201 | 196 | 277 |
| sort | 15 | 960 | 7 | 7 | 162 |
| alloc | 17 | 787 | 7 | 9 | 155 |

## Tamanho do binário (bytes, programa vazio) e memória máxima (MB)

| Linguagem | Binário vazio | RSS empty | RSS fib | RSS loop | RSS strings | RSS sort | RSS alloc |
|---|---|---|---|---|---|---|---|
| ofs | 37312 | 5.4 | 5.5 | 4.9 | 5.5 | 6.4 | 5.1 |
| c | 12568 | 2.0 | 2.0 | 2.0 | 3.3 | 2.3 | 2.1 |
| rust | 10440304 | 2.1 | 1.9 | 1.9 | 4.2 | 3.0 | 1.9 |
| python | n/a (interpretador) | 9.5 | 9.3 | 9.2 | 11.6 | 13.4 | 9.5 |

A linha `empty` da tabela de tempo é o tempo de inicialização (programa que só imprime `ok`).
"memória" = o processo estourou o limite de 2048 MB; "timeout" = passou do limite de tempo; "falha" = saiu com erro sem ser por memória.
