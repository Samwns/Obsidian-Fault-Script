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
| empty | 3 | 38 | 2 | 2 | 12 |
| fib | 41 | 6081 | 22 | 22 | 867 |
| loop | 404 | 18088 | 273 | 273 | 10671 |
| strings | 15 | 583 | 191 | 197 | 272 |
| sort | 16 | 943 | 7 | 8 | 161 |
| alloc | 16 | 770 | 7 | 11 | 168 |

## Tamanho do binário (bytes, programa vazio) e memória máxima (MB)

| Linguagem | Binário vazio | RSS empty | RSS fib | RSS loop | RSS strings | RSS sort | RSS alloc |
|---|---|---|---|---|---|---|---|
| ofs | 37656 | 5.1 | 5.2 | 5.3 | 5.7 | 6.0 | 5.3 |
| c | 12568 | 2.1 | 2.2 | 2.0 | 3.4 | 2.4 | 2.0 |
| rust | 10440304 | 2.0 | 2.0 | 2.1 | 4.1 | 2.9 | 2.1 |
| python | n/a (interpretador) | 9.4 | 9.4 | 9.3 | 11.6 | 13.2 | 9.2 |

A linha `empty` da tabela de tempo é o tempo de inicialização (programa que só imprime `ok`).
"memória" = o processo estourou o limite de 2048 MB; "timeout" = passou do limite de tempo; "falha" = saiu com erro sem ser por memória.
