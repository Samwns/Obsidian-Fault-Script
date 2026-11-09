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

Programas equivalentes em OFS, Rust e Python (e C, referência local) em [`bench/`](../bench/). Para reproduzir: `bash bench/run.sh` (mede o compilador do repositório, `ofs/dist`, e gera [`bench/RESULTADOS.md`](../bench/RESULTADOS.md)). Medido em AMD Ryzen 5 5600GT, OFS 1.0.169, mediana de 5 execuções (interpretado: 1 execução), limite de 2.048 MB de memória por processo, tempo de parede em ms:

| Benchmark | OFS nativo | OFS interpretado | C -O2 | Rust | Python |
|---|---|---|---|---|---|
| empty (inicialização) | 3 | 37 | 2 | 2 | 12 |
| fib (n=35) | 41 | 5.997 | 22 | 23 | 906 |
| loop (10^8) | 410 | 18.135 | 276 | 277 | 11.031 |
| strings (1 MB) | 15 | 589 | 196 | 199 | 272 |
| sort (100 mil) | 15 | 925 | 7 | 7 | 164 |
| alloc (1 milhão) | 17 | 751 | 7 | 9 | 150 |

Memória máxima (RSS, MB) e binário vazio:

| Linguagem | Binário vazio (bytes) | vazio | fib | loop | strings | sort | alloc |
|---|---|---|---|---|---|---|---|
| OFS | 36.624 | 5,0 | 5,2 | 5,4 | 5,8 | 6,3 | 5,1 |
| C | 12.568 | 2,2 | 1,9 | 1,9 | 3,5 | 2,3 | 2,2 |
| Rust | 10.440.304 | 2,0 | 2,2 | 2,2 | 4,1 | 2,7 | 2,0 |
| Python | n/a (interpretador) | 9,2 | 9,4 | 9,5 | 11,6 | 13,4 | 9,5 |

Todas as saídas do modo interpretado conferem com as do nativo.

Histórico: antes da otimização de memória "A6", `strings` gastava 223 ms e 520 MB, `alloc` 116 ms e 97 MB, `sort` 86 ms, e `ofs run` estourava o limite de 2.048 MB em tudo, exceto no programa vazio. Uma rodada ainda mais antiga mediu por engano um compilador instalado e velho, o que gerou um `strings` de 10.404 ms; isso não vale para o compilador do repositório.

Leitura honesta (máquina: AMD Ryzen 5 5600GT, Linux, OFS 1.0.169, medido em 08/11/2026):

- O OFS nativo **ainda perde para C e Rust em testes de cálculo e alocação**: cerca de 1,5x em `loop`, 1,9x em `fib`, 2,1x em `sort` e 2,4x em `alloc`.
- **`strings` é um caso especial e não uma comparação com um C otimizado.** A referência em C (`bench/c/strings.c`) faz `malloc` + `memcpy` a cada concatenação, copiando tudo (O(n²)), e era isso que o OFS fazia (223 ms, 520 MB). Agora o OFS cresce a string no lugar (`realloc`) quando o compilador prova, por análise de escape por função, que o valor antigo não é visto por mais ninguém; aliases como `t = s` continuam corretos. Por isso o OFS (15 ms, 5,8 MB) passa o C (196 ms, 3,5 MB) nesse teste, cerca de 13x. Um C com buffer que cresce faria o mesmo; o stdlib tem `strbuf` para quem quer esse controle explícito.
- `alloc` caiu de 116 ms / 97 MB para 17 ms / 5,1 MB (C: 7 ms): arrays locais são liberados no fim do escopo e a alocação é inline. `sort` caiu de 86 para 15 ms (C: 7 ms): `a[i]` e `a[i] = v` são inline no código gerado.
- Ganha de Python em `fib` (22x), `loop` (27x), `strings` (18x), `sort` (11x) e `alloc` (9x).
- Inicialização de 3 ms (Python: 12 ms) e binário vazio de 36.624 bytes (Rust: cerca de 10 MB). A memória base é maior que a de C: 5,0 MB contra 2,2 MB.
- O modo interpretado (`ofs run`) agora roda todos os benchmarks dentro do limite de memória (o vazamento de cerca de 3 KB por chamada foi corrigido; o interpretador foi reescrito com registradores e tabelas da AST). Continua bem mais lento que o nativo e, em geral, que Python: 6,6x mais lento em `fib`, 1,6x em `loop`, 2,2x em `strings`, 5,6x em `sort` e 5,0x em `alloc`. Use `ofs run` para scripts e desenvolvimento; use `ofs build` para desempenho.
- Compilar o próprio compilador (self-host) caiu de 197 MB para 45 MB de RAM.

Para reproduzir: `bash bench/run.sh`. Os números completos ficam em [`bench/RESULTADOS.md`](../bench/RESULTADOS.md).

## Consistência entre `ofs run` e `ofs build`

O interpretador e o gerador LLVM compartilham lexer, parser e verificador de tipos, mas a execução ainda diverge: `ofs run` é um subconjunto (sem `extern` fora da lista de builtins e sem ponteiros). O plano é chegar à paridade; até lá, `ofs build` é a referência.
