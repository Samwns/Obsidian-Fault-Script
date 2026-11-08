# OFS: Linguagem Híbrida e Compilada Nativa

## Pergunta Essencial: OFS é Interpretada ou Compilada?

**Resposta: OFS é HÍBRIDA — Compilação Nativa AOT via LLVM e Interpretação Direta de AST.**

O OFS combina o melhor dos dois paradigmas:
1. **Compilação Nativa AOT (`ofs build`)**: gera representação intermediária LLVM IR diretamente pelo seu próprio compilador auto-hospedado (`ofscc`), emitindo código de máquina e ligando executáveis através de linkers nativos do sistema operacional, sem VM e sem interpretador no binário final.
2. **Execução Instantânea Interpretada (`ofs run` / `ofs <arquivo.ofs>`)**: executa a árvore sintática abstrata (AST) tipada diretamente na memória através de um interpretador AST integrado em OFS puro (`interpreter.ofs`), ideal para scripts rápidos, prototipação e validação imediata.

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

1. **Desenvolvimento Instantâneo**: Em modo `ofs run` ou `ofs arquivo.ofs`, o código é executado diretamente na memória sem o atraso da invocação do `llc` e do `linker`.
2. **Produção Nativa sem VM**: Em modo `ofs build`, o executável final não embute nenhum interpretador nem máquina virtual; roda diretamente como código de máquina da CPU (x86_64, ARM64).
3. **Frontend Compartilhado**: o interpretador e o gerador LLVM compartilham lexer, parser e verificador de tipos. Isso **não** garante paridade de execução: hoje o `ofs run` ainda é um subconjunto do build nativo (sem `extern` fora da lista de builtins e sem ponteiros). O plano é chegar à paridade; até lá, programas com `extern` próprio ou ponteiros só rodam com `ofs build`.
4. **Auto-hospedado (Self-hosted)**: Todo o compilador (`ofscc`), incluindo o parser, type checker, gerador LLVM e interpretador, é escrito em OFS.

---

## Comparativo Tecnológico

| Característica | OFS (Nativo) | OFS (Interpretado) | Python 3.14 |
|---|---|---|---|
| **Modelo** | AOT via LLVM | Interpretador de AST | Bytecode em VM |
| **Inicialização (medida)** | 3 ms | 38 ms | 12 ms |
| **RSS programa vazio (medido)** | 5,1 MB | n/d | 9,3 MB |
| **Binário vazio** | 28 KB | n/a | n/a |
| **Dependências** | Nenhuma (binário) | Compilador `ofs` | Interpretador |

Números medidos em Ryzen 5 5600GT (mediana de 5 execuções). Tabela completa por benchmark, incluindo C e Rust, em [`bench/RESULTADOS.md`](../bench/RESULTADOS.md). Reproduzir: `bash bench/run.sh`. Node.js não foi medido (não faz parte do `bench/`). Resumo honesto: o OFS nativo fica entre 1,5x e 1,8x o tempo do C em `fib` e `loop`, 10x a 17x em `sort` e `alloc`, e **50x em `strings`**; o modo interpretado é lento demais para os benchmarks pesados (veja [PERFORMANCE.md](PERFORMANCE.md)).

---

## Conclusão

OFS é uma linguagem moderna, híbrida e auto-suficiente:

- Execução de scripts imediata com o interpretador AST (`ofs run`).
- Compilação direta para código de máquina nativo de alta performance (`ofs build`).
- Frontend 100% escrito na própria linguagem OFS.
- Runtime nativo integrado de alto desempenho (Stack Magma).
