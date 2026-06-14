# Analise de desempenho da OFS

## Objetivo

O objetivo da otimizacao nao e produzir um unico numero menor que C ou Rust. O objetivo e reduzir custos concretos do compilador e do runtime sem mudar a semantica da linguagem, corromper memoria ou esconder regressao.

## Caminho de compilacao

O fluxo atual e:

```text
fonte OFS
  -> expansao de attach
  -> lexer
  -> parser
  -> verificacao de tipos
  -> geracao LLVM IR
  -> clang/LLVM
  -> executavel + libofs_runtime.a
```

Cada etapa deve ser medida separadamente. Melhorar o executavel gerado nao reduz necessariamente o tempo do lexer. Melhorar o lexer nao altera necessariamente CPU do programa final.

## Gargalos encontrados

### 1. Leitura de caracteres

`ofs_str_char_at` usa `strlen` para validar o indice. Essa interface e segura para chamadas gerais, mas o lexer ja conhece o tamanho total da fonte e chama a funcao para quase todo caractere.

Em uma string longa, repetir `strlen` pode transformar uma varredura que deveria ser linear em trabalho repetido.

Uma API sem verificacao foi testada localmente apenas no lexer. Ela nao foi mantida porque o teste self-hosted `test_lexer_basic.ofs` atualmente sofre segmentation fault mesmo com o caminho original. A cobertura precisa ser consertada antes de publicar esse tipo de mudanca.

### 2. Substrings

`ofs_str_substr` calcula o tamanho da string, aloca memoria e copia bytes. O lexer usa substrings para materializar identificadores, numeros e literais.

Possiveis evolucoes:

- token guardar inicio e tamanho na fonte;
- materializar texto somente quando necessario;
- usar arena por compilacao;
- liberar a arena ao terminar o arquivo.

### 3. Concatenacao

`ofs_str_concat` calcula os dois tamanhos, aloca um novo bloco e copia os dois lados.

Concatenacao repetida em loops pode copiar o mesmo prefixo muitas vezes. A geracao LLVM principal ja usa `cg_emit` para escrever diretamente em arquivo, mas escapes de string e expansao de imports ainda concatenam.

Possiveis evolucoes:

- string builder nativo;
- buffer com capacidade e crescimento geometrico;
- API `append` para ODL/OES e ferramentas;
- medicao de bytes alocados por compilacao.

### 4. Buscas lineares

Simbolos, nomes de variaveis, strings internadas e campos usam arrays e buscas lineares em varios pontos.

Para projetos pequenos isso e simples e previsivel. Para projetos grandes, uma tabela hash ou indices por escopo podem reduzir consultas repetidas.

### 5. Tamanhos dentro de loops

O compilador possui loops que chamam `ofs_array_len`, `node_extra_len` ou `ofs_str_len` na condicao. Quando a colecao nao muda, o tamanho pode ser guardado antes do loop.

Essa e uma otimizacao de baixo risco, mas cada alteracao precisa manter os testes de parser, type checker e LLVM.

## Ordem proposta

1. Reparar o teste self-hosted do lexer.
2. Adicionar medicao por fase do compilador.
3. Medir leitura de caracteres com fontes de 10 KB, 100 KB e 1 MB.
4. Introduzir uma API de leitura conhecida pelo tamanho apenas dentro do compilador.
5. Adicionar arena ou slices para tokens.
6. Criar string builder para geradores e DSLs.
7. Medir tabelas de simbolos com projetos grandes.
8. Avaliar LTO e flags LLVM depois que o frontend estiver medido.

## Otimizacao de link e runtime

A runtime distribuida e compilada com `-ffunction-sections` e
`-fdata-sections`. O launcher usa `--gc-sections` no ELF, `dead_strip` no
Mach-O e `OPT:REF` no PE/COFF para retirar funcoes da runtime que o programa
nao referencia.

No workload publicado em 14 de junho de 2026, isso reduziu o executavel OFS
de 28.224 para 16.056 bytes sem alterar o checksum. O launcher tambem mantem
em cache a versao principal do Clang, evitando consultar o toolchain em cada
compilacao. A mediana de build do mesmo workload caiu de 320,81 ms para
208,78 ms nesta maquina.

Esses ganhos pertencem ao pipeline real de distribuicao. Eles nao mudam a
sintaxe, o type checker ou a semantica do programa.

## Gates

Uma otimizacao so deve entrar quando:

- os testes existentes passam;
- o bootstrap continua deterministico;
- ODL e OES continuam compilando;
- instaladores continuam construindo;
- o benchmark mostra ganho repetivel;
- a API publica nao perde seguranca silenciosamente.
