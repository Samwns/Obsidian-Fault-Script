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
3. **Frontend Unificado**: Tanto o interpretador quanto o gerador LLVM compartilham o mesmo lexer, o mesmo parser e o mesmo verificador estático de tipos, garantindo consistência semântica.
4. **Auto-hospedado (Self-hosted)**: Todo o compilador (`ofscc`), incluindo o parser, type checker, gerador LLVM e interpretador, é escrito em OFS.

---

## Comparativo Tecnológico

| Característica | OFS (Nativo) | OFS (Interpretado) | Python | Node.js |
|---|---|---|---|---|
| **Modelo** | Máquina nativa AOT | AST Walk em memória | Bytecode em VM | JIT Engine (V8) |
| **Execução** | Direto na CPU | Interpretador em OFS | CPython | V8 C++ |
| **Inicialização** | ~2 a 10 ms | ~5 a 15 ms | ~40 a 100 ms | ~50 a 150 ms |
| **Memória Base** | Leve (~1 a 4 MB) | Leve (~4 a 8 MB) | Moderado (~20 MB)| Elevado (~50 MB) |
| **Dependências** | Nenhuma (binário) | Compilador `ofs` | Interpretador | Runtime Node |

---

## Conclusão

OFS é uma linguagem moderna, híbrida e auto-suficiente:

- Execução de scripts imediata com o interpretador AST (`ofs run`).
- Compilação direta para código de máquina nativo de alta performance (`ofs build`).
- Frontend 100% escrito na própria linguagem OFS.
- Runtime nativo integrado de alto desempenho (Stack Magma).
