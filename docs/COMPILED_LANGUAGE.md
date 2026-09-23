# OFS: Linguagem Compilada Nativa

## Pergunta Essencial: OFS é Interpretada ou Compilada?

**Resposta: OFS é COMPILADA.**

OFS é uma linguagem de programação compilada nativa que gera representação intermediária LLVM IR diretamente pelo seu próprio compilador auto-hospedado (`ofscc`), emitindo código de máquina e ligando executáveis através de linkers nativos do sistema operacional.

---

## Arquitetura de Compilação

```text
┌─────────────────────────────────────────────────────────────────────┐
│ Código-fonte OFS                                                    │
│ (.ofs)                                                              │
└────────────────────────┬────────────────────────────────────────────┘
                         │
                    ┌────▼─────┐
                    │  Lexer   │ -> Análise léxica (tokens)
                    └────┬─────┘
                         │ Array<Token>
    ┌────────────────────┘
    │
    ▼
┌─────────────────────────┐
│     parser.ofs          │ -> Análise sintática
│ (escrito em OFS puro)   │
└────────┬────────────────┘
         │ AST (Árvore Sintática)
         │
    ┌────▼─────────┐
    │ Type Checker │ -> Validação e inferência de tipos
    └────┬─────────┘
         │ AST anotada
         │
     ┌───▼────────────┐
     │    LLVM Gen    │ -> Emissão de LLVM IR (.ll) diretamente
     └───┬────────────┘
         │ LLVM IR nativo
         │
     ┌───▼───────────────────────────┐
     │  llc -filetype=obj (.ll -> .o)│ -> Código de máquina objeto
     └───┬───────────────────────────┘
         │ Objeto binário (.o)
         │
     ┌───▼────────────────────────────────────┐
     │  ld / ld.lld (.o + magma.o -> binário) │ -> Linkagem estática com runtime
     └───┬────────────────────────────────────┘
         │
     ┌───▼──────────────────┐
     │ Executável Nativo    │
     │ ELF (Linux)          │
     │ Mach-O (macOS)       │
     │ PE32+ (Windows)      │
     └──────────────────────┘
```

---

## O que Significa "Compilado"?

Uma linguagem é estritamente compilada quando:

1. **Possui compilador auto-hospedado** — O compilador `ofscc` é escrito na própria linguagem OFS.
2. **Produz executáveis nativos** — Não utiliza interpretadores em tempo de execução, máquinas virtuais ou bytecodes intermediários dependentes de VM.
3. **Separação estrita entre tempo de compilação e execução** — `ofs build` emite o binário de máquina final antes do início do programa.
4. **Execução direta pelo processador** — O código roda diretamente nas instruções do hardware (x86_64, ARM64).

---

## Comparativo Tecnológico

| Característica | OFS | Python | JavaScript / Node.js |
|---|---|---|---|
| **Modelo de Execução** | Código de máquina nativo AOT | Bytecode em VM | JIT / V8 Engine |
| **Compilador** | Auto-hospedado (escrito em OFS) | CPython (escrito em C) | V8 (escrito em C++) |
| **Tempo de Inicialização** | ~2 a 10 ms | ~40 a 100 ms | ~50 a 150 ms |
| **Consumo de Memória Base** | Leve (~1 a 4 MB) | Moderado (~15 a 30 MB) | Elevado (~30 a 80 MB) |
| **Dependências de Runtime** | Binário independente | Interpretador Python | Node.js Runtime |

---

## Pipeline Nativo

O pipeline de compilação do OFS opera da seguinte forma:

1. **Emissão Direta de LLVM IR**: `llvmgen.ofs` converte a árvore sintática tipada diretamente em instruções LLVM IR em formato SSA, sem transpiladores intermediários.
2. **Compilação Estática via LLC**: O utilitário `llc` compila o arquivo `.ll` para um arquivo objeto nativo (`.o`).
3. **Linkagem com Runtime Nativo**: O linker do sistema (`ld` no Linux/macOS, `ld.lld` no Windows) combina o objeto gerado com o runtime nativo compilado (`magma.o`), gerando o executável final autocontido.

---

## Conclusão

OFS é uma linguagem compilada e auto-suficiente:

- Compilação direta para código de máquina nativo
- Ausência de máquinas virtuais e interpretadores em produção
- Emissão direta de LLVM IR pelo frontend auto-hospedado
- Runtime nativo integrado (Stack Magma)
