# OFS: Linguagem Híbrida e Compilada Nativa

## Pergunta Essencial: OFS é Interpretada ou Compilada?

**Resposta: OFS é HÍBRIDA — Compilação Nativa AOT via LLVM e Interpretação Direta de AST.**

O OFS combina o melhor dos dois paradigmas:
1. **Compilação Nativa AOT (`ofs build`)**: gera representação intermediária LLVM IR diretamente pelo seu próprio compilador auto-hospedado (`ofscc`), emitindo código de máquina e ligando executáveis através de linkers nativos do sistema operacional, sem VM e sem interpretador no binário final.
2. **Execução Instantânea Interpretada (`ofs run` / `ofs <arquivo.ofs>`)**: executa a árvore sintática abstrata (AST) tipada diretamente na memória através de um interpretador AST integrado em OFS puro (`interpreter.ofs`), ideal para scripts rápidos, prototipação e validação imediata.


## Modelo de execução: interpretado e compilado com a mesma semântica

A regra de ouro é comparar `ofs run programa.ofs` com `ofs build programa.ofs` e executar o binário: a saída, o código de saída e os efeitos observáveis devem ser iguais. Lexer, parser e typeck compartilhados são a base; a evidência vem dos testes. Execute `bash tests/parity/run.sh` e consulte a [matriz de paridade](https://github.com/Samwns/Obsidian-Fault-Script/blob/main/tests/parity/MATRIZ.md), que separa programas iguais, diferenças, erros esperados e casos pulados. Uma diferença é um bug ou uma limitação documentada, não uma segunda semântica da linguagem.

`ofs run` executa a AST tipada. `extern` e `rift vein` usam símbolos nativos resolvidos pelo Rift; ponteiros são endereços reais e os callbacks permitem que C reentre numa vein interpretada. `ofs build` emite LLVM IR e liga um executável independente do interpretador. `ofs run --native` compila e executa o programa inteiro num temporário.

Com `quartz`, a execução é híbrida: o interpretador identifica as veins que contêm assembly, gera com o próprio `llvmgen` um módulo com o fechamento das dependências e carrega uma biblioteca nativa no mesmo processo. As outras veins começam interpretadas e podem ser promovidas pela cristalização. Funções chamadas pelo fechamento têm versões nativas no módulo; valores de veins interpretadas passados ao nativo usam os trampolins de callback. Globais escalares são sincronizados ao entrar e sair do nativo e em callbacks; strings, arrays e objetos compartilham memória conforme a representação suportada pelo Rift. O cache usa o hash do IR e do comando de compilação (`OFS_QUARTZ_CACHE`, ou `$XDG_CACHE_HOME/ofs/quartz`, ou `~/.cache/ofs/quartz`). `OFS_VERBOSE=1` informa compilação ou cache. Na primeira execução sem cache, é necessário ter `clang`; `OFS_QUARTZ_CC` seleciona o compilador.

A fronteira híbrida herda os limites do dispatcher FFI: até 12 parâmetros inteiros/ponteiros ou até 4 parâmetros com `crystal`/`f32`; callbacks com ponto flutuante usam os trampolins `__ofs_cbx_K` (fora do Windows x64). A mesma lógica nativa pode ter velocidade próxima à do build depois de compilada, mas interpretar a AST, compilar a biblioteca e atravessar a fronteira têm custos. Desempenho equivalente só pode ser afirmado para os casos medidos, distinguindo aquecimento de execução com cache.

### Cristalização: veins quentes viram nativas

Por padrão, `ofs run` conta as chamadas de cada vein elegível. Ao atingir `OFS_CRYSTALLIZE_THRESHOLD` (padrão `1000`, inteiro de 1 a 1000000000), gera o IR de um único módulo com as veins de assinatura suportada e suas dependências. O `clang -O2` compila a biblioteca em segundo plano enquanto a execução interpretada continua; chamadas seguintes verificam a conclusão e passam a usar o despacho nativo. A geração do IR é síncrona. Os globais vivem em células únicas que o interpretador e todas as bibliotecas compartilham (sem cópia na fronteira, exceto `Array` e valores de vein). Com `quartz`, o módulo necessário já é compilado no carregamento; as demais veins elegíveis ganham despacho nativo quando atingem o gatilho de calor.

`OFS_CRYSTALLIZE=0` desativa essa promoção opcional e mantém `quartz` híbrido. `OFS_VERBOSE=1` mostra a promoção e falhas opcionais. Sem `clang`, ou se a compilação opcional falhar, o programa continua interpretado; uma vein que precisa de `quartz` exige compilação nativa. Assinaturas sem trampolim suportado não são promovidas como raízes. Um laço longo dentro de uma única chamada (o `while` do `main`, por exemplo) troca de modo no meio (**OSR**, onda 4): depois de `OFS_CRYSTALLIZE_THRESHOLD` voltas, o laço é extraído para uma função nativa, compilada em segundo plano, e continua nela com as variáveis vivas transferidas; `bench/ofs/loop.ofs` cai de 18,6 s para 0,34 s (o build nativo leva 0,41 s). Só `while` sem `return`/`throw`/`?`/`&`/geode no corpo; `OFS_OSR=0` desliga. O cache reduz compilações posteriores, mas não elimina o custo de gerar IR e carregar a biblioteca. Compare tempos a frio, com cache e do binário AOT antes de afirmar equivalência de velocidade.

### Baixo nível seguro

Código seguro pode chamar uma vein tipada sem bloco de baixo nível. Use `fracture` (ou `bedrock`) para operações de ponteiro, como `&x` e `*p`, e `abyss` para endereços crus de funções, `fault_call` cru e `quartz`. O typeck aplica a mesma regra nos dois modos: `OFS_STRICT_FAULT=1` transforma violações em erros; a migração padrão usa avisos até 2027-01-09. Esses blocos autorizam operações, mas não validam automaticamente a vida útil ou a validade de um endereço. Os limites de FFI e de callbacks estão descritos na [Referência da linguagem](LANGUAGE_REFERENCE.md).

---

## Arquitetura de Execução Dupla

```text
┌─────────────────────────────────────────────────────────────────────┐
│ Código-fonte OFS (.ofs)                                             │
└────────────────────────┬────────────────────────────────────────────┘
                         │
                    ┌────▼─────┐
                    │  Lexer   │ -> Análise léxica (tokens)
                    └────┬─────┘
                         │ Array<Token>
                         ▼
                    ┌──────────┐
                    │  Parser  │ -> Análise sintática (AST pura em OFS)
                    └────┬─────┘
                         │ AST
                         ▼
                    ┌──────────┐
                    │ Typeck   │ -> Verificação e anotação estática de tipos
                    └────┬─────┘
                         │ AST Tipada e Anotada
         ┌───────────────┴───────────────┐
         │                               │
[ofs run / ofs <arquivo>]         [ofs build <arquivo>]
         │                               │
         ▼                               ▼
┌──────────────────┐            ┌──────────────────┐
│ interpreter.ofs  │            │   llvmgen.ofs    │ -> Emissão de LLVM IR (.ll)
│ Interpretador    │            └────────┬─────────┘
│ AST em OFS puro  │                     │ LLVM IR
└────────┬─────────┘                     ▼
         │                      ┌──────────────────┐
  Execução imediata             │       llc        │ -> Código de máquina (.o)
     em memória                 └────────┬─────────┘
                                         │ Objeto binário (.o)
                                         ▼
                                ┌──────────────────┐
                                │ ld / ld.lld      │ -> Linkagem com magma.o
                                └────────┬─────────┘
                                         │
                                ┌────────▼─────────┐
                                │ Executável Nativo│
                                │ ELF / Mach-O / PE│
                                └──────────────────┘
```

---

## O que Significa o Modelo Híbrido?

1. **Desenvolvimento Instantâneo**: Em modo `ofs run` ou `ofs arquivo.ofs`, o código comum roda diretamente em memória; a execução híbrida pode compilar veins nativas no primeiro uso.
2. **Produção Nativa sem VM**: Em modo `ofs build`, o executável final não embute nenhum interpretador nem máquina virtual; roda diretamente como código de máquina da CPU (x86_64, ARM64).
3. **Frontend compartilhado**: lexer, parser e typeck são comuns aos dois modos; a regra de ouro e a matriz acima verificam a paridade de execução.
4. **Auto-hospedado (Self-hosted)**: Todo o compilador (`ofscc`), incluindo o parser, type checker, gerador LLVM e interpretador, é escrito em OFS.

---

## Comparativo Tecnológico

| Característica | OFS (Nativo) | OFS (Interpretado) | Python 3.14 |
|---|---|---|---|
| **Modelo** | AOT via LLVM | AST + cristalização nativa | Bytecode em VM |
| **Inicialização (medida)** | 3 ms | 40 ms | 13 ms |
| **RSS programa vazio (medido)** | 5,0 MB | n/d | 9,5 MB |
| **Binário vazio** | 37 KB | n/a | n/a |
| **Dependências** | Bibliotecas do sistema usadas pelo programa | `ofs`; `clang` para módulos nativos sem cache | Interpretador |

Medido em 2026-11-15 em Ryzen 5 5600GT (mediana de 3 execuções). Tabela completa por benchmark, incluindo C e Rust, em [`bench/RESULTADOS.md`](https://github.com/Samwns/Obsidian-Fault-Script/blob/main/bench/RESULTADOS.md). Reproduzir: `bash bench/run.sh`. Node.js não foi medido (não faz parte do `bench/`). Resumo honesto: o OFS nativo fica entre 1,5x e 1,8x o tempo do C em `loop` e `fib` e entre 2,3x e 2,4x em `sort` e `alloc`; em `strings` ele é mais rápido que o C de referência (16 ms contra 200 ms) porque cresce a string no lugar, enquanto o C de referência copia tudo a cada concatenação (veja a leitura honesta em [PERFORMANCE.md](PERFORMANCE.md)). O modo interpretado roda todos os benchmarks, mas é de 1,7x a 7,2x mais lento que Python; com a cristalização, `fib` cai de 6.573 para 75 ms.

---

## Conclusão

OFS é uma linguagem moderna, híbrida e auto-suficiente:

- Execução de scripts imediata com o interpretador AST (`ofs run`).
- Compilação direta para código de máquina nativo de alta performance (`ofs build`).
- Frontend 100% escrito na própria linguagem OFS.
- Runtime nativo integrado de alto desempenho (Stack Magma).
