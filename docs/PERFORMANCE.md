# Análise de Desempenho do OFS

## Objetivo

O objetivo da otimização é reduzir custos concretos de execução e compilação do compilador e do runtime sem alterar a semântica da linguagem, sem comprometer a integridade de memória e sem introduzir regressões silenciosas. O tempo total de build deve permanecer competitivo com toolchains nativas equivalentes por eliminação de trabalho redundante, e não por remoção de recursos da linguagem.

---

## Fluxo de Compilação

O pipeline de compilação é estruturado nas seguintes etapas:

```text
Código-fonte OFS
  -> Resolução e expansão de módulos (attach)
  -> Análise léxica (lexer)
  -> Análise sintática (parser)
  -> Verificação de tipos (type checker)
  -> Geração de código intermediário LLVM IR (llvmgen.ofs)
  -> Compilador estático LLVM (llc -filetype=obj)
  -> Linkagem nativa com runtime nativo (ld / ld.lld)
  -> Executável binário nativo (ELF / Mach-O / PE)
```

Cada etapa é mensurada de forma isolada: otimizações no binário final gerado não reduzem necessariamente o tempo do lexer, e melhorias no lexer não alteram o consumo de CPU do programa resultante.

---

## Pontos Críticos e Oportunidades de Otimização

### 1. Leitura de Caracteres no Lexer

A função `ofs_str_char_at` realiza validação de limites usando o comprimento da string. Para chamadas genéricas na biblioteca padrão, essa verificação garante segurança de acesso. No entanto, o lexer interno já mantém o controle estrito de `_pos` em relação a `_len`. O uso da variante `ofs_str_char_at_known` evita varreduras redundantes de comprimento (`strlen`) a cada caractere lido, mantendo a complexidade estritamente linear $O(N)$ no processamento de arquivos-fonte extensos.

### 2. Alocação de Substrings

Durante a tokenização, o lexer extrai fatias de texto para representar identificadores, números e literais. A estratégia otimizada consiste em:
- Armazenar tuplas de posição e comprimento `(início, tamanho)` apontando para o buffer original do arquivo durante a análise sintática.
- Materializar strings na memória heap apenas quando um nó da árvore sintática ou a tabela de símbolos exigir persistência explícita.
- Empregar alocadores baseados em arena por arquivo de compilação, permitindo liberação em lote ao término da unidade de tradução.

### 3. Concatenação de Cadeias de Caracteres

Operações sequenciais de concatenação em loops geram cópias repetidas de prefixos na heap. No gerador de código LLVM, a rotina `cg_emit` grava diretamente no arquivo de saída, contornando a criação intermediária de buffers temporários.

### 4. Tabelas de Símbolos e Consultas de Escopo

Para programas pequenos e médios, listas lineares de símbolos oferecem boa localidade de cache e simplicidade determinística. Para bases de código de grande porte, a utilização de tabelas hash ou estruturas indexadas por escopo reduz o custo assintótico de busca de variáveis e tipos.

---

## Metodologia de Medição

1. Conjunto controlado de testes sintáticos e léxicos com fontes variando entre 10 KB e 1 MB.
2. Isolamento de tempos por fase: frontend (lexer, parser, tipos), emissão de LLVM IR, e linkagem nativa.
3. Medição de métricas reais de sistema via chamadas `wait4`: tempo de usuário (CPU), tempo de sistema e pico de memória residente (RSS).
4. Verificação estrita de integridade via checksum da saída gerada para garantir que nenhuma otimização introduza alterações semânticas.

---

## Desempenho de programas gerados (sincronizado com a versão em inglês)

Os trabalhos se dividem entre vazão do compilador e tempo de execução do programa gerado. Pontos já medidos em 14/06/2026 (mediana de sete processos): a geração de IR da carga pequena foi de 8,11 ms para 7,54 ms, e a compilação do `ofscc.ofs` de 4.610,40 ms para 3.694,08 ms; os estágios dois e três do bootstrap emitiram IR idêntico.

Ordem planejada: reparar e ampliar os testes do lexer self-host; medir lexer, parser, tipos e IR separadamente; testar fontes de 10 KB, 100 KB e 1 MB; introduzir fatias de token ou arena; criar um construtor de strings nativo (já existe `strbuf` no stdlib); medir tabelas de símbolos indexadas; avaliar LTO e flags do LLVM.

### Link e runtime

O runtime distribuído é compilado com `-ffunction-sections` e `-fdata-sections`; o launcher usa `--gc-sections` (ELF), `dead_strip` (Mach-O) e `OPT:REF` (PE/COFF). Na carga de 14/06/2026 isso reduziu o executável de 28.224 para 16.056 bytes sem mudar o checksum, e o cache de caminhos do CRT, `llc` e `ld` reduziu o build mediano de 320,81 ms para 208,78 ms.

## Benchmarks de execução (medidos)

Programas equivalentes em OFS, Rust e Python (e C, referência local) em [`bench/`](../bench/). Para reproduzir: `bash bench/run.sh` (mede o compilador do repositório, `ofs/dist`, e gera [`bench/RESULTADOS.md`](../bench/RESULTADOS.md)). Medido em 2026-11-17 em AMD Ryzen 5 5600GT, OFS 1.0.169, mediana de 3 execuções (interpretado: 1 execução, limite de 60 s), limite de 2.048 MB de memória por processo, tempo de parede em ms. A coluna "cristalizado" é o `ofs run` com a cristalização automática ligada (o padrão); "interpretado" é o `ofs run` com `OFS_CRYSTALLIZE=0`:

| Benchmark | OFS nativo | OFS interpretado | OFS cristalizado | C -O2 | Rust | Python |
|---|---|---|---|---|---|---|
| empty (inicialização) | 4 | 43 | 43 | 2 | 2 | 13 |
| fib (n=35) | 43 | 6.435 | 79 | 23 | 24 | 897 |
| loop (10^8) | 404 | 18.295 | 318 | 273 | 273 | 11.664 |
| strings (1 MB) | 16 | 601 | 299 | 196 | 197 | 279 |
| sort (100 mil) | 16 | 989 | 123 | 7 | 8 | 172 |
| alloc (1 milhão) | 18 | 838 | 70 | 7 | 9 | 152 |

Memória máxima (RSS, MB) e binário vazio:

| Linguagem | Binário vazio (bytes) | vazio | fib | loop | strings | sort | alloc |
|---|---|---|---|---|---|---|---|
| OFS | 37.656 | 5,0 | 5,4 | 5,3 | 5,6 | 6,4 | 4,9 |
| C | 12.568 | 2,0 | 1,9 | 1,9 | 3,4 | 2,3 | 2,0 |
| Rust | 10.440.304 | 2,1 | 2,0 | 2,0 | 4,1 | 2,8 | 1,9 |
| Python | n/a (interpretador) | 9,5 | 9,5 | 9,5 | 11,7 | 13,3 | 9,5 |

Todas as saídas do modo interpretado conferem com as do nativo. A cristalização ajuda onde há veins quentes chamadas muitas vezes (`fib`: 6.435 para 79 ms; `sort`: 989 para 123 ms) e, com a OSR (troca de um laço já em execução por código nativo, ver [COMPILED_LANGUAGE.md](COMPILED_LANGUAGE.md)), também dentro de um único laço longo: `loop` 18.295 para 318 ms (mais rápido que o nativo AOT, 404 ms), `alloc` 838 para 70 ms, `strings` 601 para 299 ms.

Histórico: antes da otimização de memória "A6", `strings` gastava 223 ms e 520 MB, `alloc` 116 ms e 97 MB, `sort` 86 ms, e `ofs run` estourava o limite de 2.048 MB em tudo, exceto no programa vazio. Uma rodada ainda mais antiga mediu por engano um compilador instalado e velho, o que gerou um `strings` de 10.404 ms; isso não vale para o compilador do repositório.

Leitura honesta (máquina: AMD Ryzen 5 5600GT, Linux, OFS 1.0.169, medido em 17/11/2026):

- O OFS nativo **ainda perde para C e Rust em testes de cálculo e alocação**: cerca de 1,5x em `loop`, 1,8x em `fib`, 2,3x em `sort` e 2,4x em `alloc`.
- **`strings` é um caso especial e não uma comparação com um C otimizado.** A referência em C (`bench/c/strings.c`) faz `malloc` + `memcpy` a cada concatenação, copiando tudo (O(n²)), e era isso que o OFS fazia (223 ms, 520 MB). Agora o OFS cresce a string no lugar (`realloc`) quando o compilador prova, por análise de escape por função, que o valor antigo não é visto por mais ninguém; aliases como `t = s` continuam corretos. Por isso o OFS (16 ms, 5,6 MB) passa o C (200 ms, 3,4 MB) nesse teste, cerca de 12x. Um C com buffer que cresce faria o mesmo; o stdlib tem `strbuf` para quem quer esse controle explícito.
- `alloc` caiu de 116 ms / 97 MB para 17 ms / 4,9 MB (C: 7 ms): arrays locais são liberados no fim do escopo e a alocação é inline. `sort` caiu de 86 para 16 ms (C: 7 ms): `a[i]` e `a[i] = v` são inline no código gerado.
- Ganha de Python em `fib` (22x), `loop` (27x), `strings` (17x), `sort` (11x) e `alloc` (9x).
- Inicialização de 3 ms (Python: 13 ms) e binário vazio de 37.656 bytes (Rust: cerca de 10 MB). A memória base é maior que a de C: 5,0 MB contra 2,0 MB.
- O modo interpretado (`ofs run`) agora roda todos os benchmarks dentro do limite de memória (o vazamento de cerca de 3 KB por chamada foi corrigido; o interpretador foi reescrito com registradores e tabelas da AST). Continua bem mais lento que o nativo e, em geral, que Python: 7,2x mais lento em `fib`, 1,7x em `loop`, 2,2x em `strings`, 5,7x em `sort` e 5,3x em `alloc` (com a cristalização ligada, `fib` fica 12x mais rápido que o Python). Use `ofs run` para scripts e desenvolvimento; use `ofs build` para desempenho.
- Compilar o próprio compilador (self-host) caiu de 197 MB para 45 MB de RAM.
- O `ofs run` com cristalização e OSR (o padrão) chega à mesma ordem do nativo depois do aquecimento: `fib` 79 ms (nativo 43), `sort` 123 ms (16), `alloc` 70 ms (18), e o `loop` fica **mais rápido que o nativo AOT** (318 × 404 ms), porque o laço quente é recompilado em `-O2` com o estado real.

Para reproduzir: `bash bench/run.sh`. Os números completos ficam em [`bench/RESULTADOS.md`](../bench/RESULTADOS.md).

## Consistência entre `ofs run` e `ofs build`

O interpretador e o gerador LLVM compartilham lexer, parser e verificador de tipos, e o `ofs run` aceita `extern`/`rift vein` (pelo Rift) e ponteiros reais. A regra é a mesma nos dois modos: uma diferença de saída é um bug ou uma limitação documentada. Confira com `ofs twin` e com `bash tests/parity/run.sh` (veja a [matriz de paridade](COMPILED_LANGUAGE.md)); em caso de dúvida, `ofs build` é a referência.
