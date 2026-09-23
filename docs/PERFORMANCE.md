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
