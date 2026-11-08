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

Ordem planejada: reparar e ampliar os testes do lexer self-host; medir lexer, parser, tipos e IR separadamente; testar fontes de 10 KB, 100 KB e 1 MB; introduzir fatias de token ou arena; criar um construtor de strings nativo; medir tabelas de símbolos indexadas; avaliar LTO e flags do LLVM.

### Link e runtime

O runtime distribuído é compilado com `-ffunction-sections` e `-fdata-sections`; o launcher usa `--gc-sections` (ELF), `dead_strip` (Mach-O) e `OPT:REF` (PE/COFF). Na carga de 14/06/2026 isso reduziu o executável de 28.224 para 16.056 bytes sem mudar o checksum, e o cache de caminhos do CRT, `llc` e `ld` reduziu o build mediano de 320,81 ms para 208,78 ms.

## Benchmarks de execução (medidos)

Programas equivalentes em OFS, Rust e Python (e C, referência local) em [`bench/`](../bench/). Para reproduzir: `bash bench/run.sh` (gera [`bench/RESULTADOS.md`](../bench/RESULTADOS.md)). Medido em AMD Ryzen 5 5600GT, mediana de 5 execuções, tempo de parede em ms:

@@TABLE@@

Leitura honesta:

- O OFS nativo **perde para C e Rust em todos os testes**: cerca de 1,5x a 1,8x em `fib` e `loop`, 10x em `sort`, 17x em `alloc` e 50x em `strings` (concatenação e percurso de 1 MB).
- Ganha de Python em `fib`, `loop`, `sort` e `alloc` (de 1,3x a 27x); em `strings` perde até para Python (CPython otimiza a concatenação de strings).
- Inicialização de 3 ms (Python: 12 ms), binário vazio de 28 KB (Rust: ~10 MB), mas a memória é maior que a de C: 5,1 MB vazio, 97 MB em `alloc` (arrays pequenos não liberados no laço) e 520 MB em `strings`.
- `ofs run` (interpretado) é ordens de grandeza mais lento e hoje não completa `fib(35)` e `loop` em 60 s, e falha (código de saída 1) em `strings`, `sort` e `alloc`. Serve para scripts, não para carga pesada.
- Defeito encontrado ao escrever o benchmark: no build nativo, a atribuição `a[i] = v` em `Array<stone>` não tem efeito; o `sort` usa `ofs_array_set(a, i, v)` como contorno.

## Consistência entre `ofs run` e `ofs build`

O interpretador e o gerador LLVM compartilham lexer, parser e verificador de tipos, mas a execução ainda diverge: `ofs run` é um subconjunto (sem `extern` fora da lista de builtins e sem ponteiros). O plano é chegar à paridade; até lá, `ofs build` é a referência.
