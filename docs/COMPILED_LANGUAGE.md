# OFS: Linguagem Compilada

## Pergunta Essencial: OFS é Interpretada ou Compilada?

**Resposta: OFS é COMPILADA.**

Mais especificamente, OFS é uma linguagem **compilada com front-end em C como linguagem intermediária**.

---

## Arquitetura de Compilação

```
┌─────────────────────────────────────────────────────────────────────┐
│ Código-fonte OFS                                                    │
│ (.ofs files)                                                        │
└────────────────────────┬────────────────────────────────────────────┘
                         │
                    ┌────▼─────┐
                    │  Lexer    │ -> Tokenação
                    └────┬──────┘
                         │ Array<Token>
    ┌────────────────────┐
    │                    │
    ▼  (Fase 1:Lex)      │
┌─────────────────────────┐
│     parser.ofs          │
│ (apenas OFS, sem deps) │
└────────┬────────────────┘
         │ Array<Node> (AST)
         │
    ┌────▼─────────┐
    │ Type Checker │ -> Validação de tipos
    └────┬─────────┘
         │ AST annotado
         │
     ┌────▼───────────┐
     │    LLVM Gen    │ -> Gera LLVM IR (.ll) diretamente
     └────┬───────────┘
          │ IR LLVM otimizado
          │
     ┌────▼──────────────────────────┐
     │  llc -filetype=obj (.ll -> .o)│ -> Código de máquina nativo
     └────┬──────────────────────────┘
          │ Objeto binário (.o)
          │
     ┌────▼───────────────────────────────────┐
     │  ld / ld.lld (.o + magma.o -> binário) │ -> Ligação direta com Stack Magma
     └────┬───────────────────────────────────┘
          │
     ┌────▼──────────────────┐
     │ Executável nativo     │
     │ ELF (Linux)           │
     │ PE32+ (Windows / Wine)│
     └───────────────────────┘
```

---

## O que significa "Compilado"?

Uma linguagem é compilada quando:

1. **Tem um compilador self-hosted** — OFS tem o compilador nativo `ofscc` escrito em OFS.
2. **Produz executável nativo** — Não interpreta em runtime, não usa máquina virtual ou bytecode.
3. **Fase de compilação separada** — `ofs build` gera o executável de máquina, depois roda.
4. **Sem overhead de interpretação** — Executa direto nas instruções do processador (x86_64, ARM).

### Pipeline Nativo sem Dependência do Clang

OFS compila diretamente para código de máquina através do pipeline:

1. **Emissão Direta de LLVM IR** — `llvmgen.ofs` gera representação intermediária sem intermediários em C.
2. **Compilação Estática via LLC** — O compilador estático LLVM gera o arquivo objeto de máquina (`.o`).
3. **Ligador Nativo (LD / LLD)** — O executável é ligado diretamente com os módulos do **Stack Magma** (`magma.o`), gerando o binário executável nativo sem intermediários externos.

---

## Compilação vs Interpretação: Comparação

| Aspecto | OFS | Python | JavaScript |
|---|---|---|---|
| **Compilador?** | Sim (Self-hosted) | Não (bytecode) | Não (JIT) |
| **Execução** | Máquina nativa | VM + bytecode | Motor JS |
| **Performance** | Nativa | ~50x mais lento | ~10x mais lento |
| **Linguagem intermediária** | LLVM IR | Bytecode | Código JS |
| **Auto-hosting** | Sim (ofscc em OFS) | Sim (Python em Python) | Sim (V8 tem partes em JS) |

---

## As 3 Gerações de Compiladores OFS

### Geração v1: Compilador em C++ 

```
┌──────────────────────────────┐
│ ofs.exe (C++ + LLVM)         │
│ ~3,000 linhas C++            │
│ Compila .ofs → .c            │
└──────────────┬───────────────┘
               │
               ▼
        ofs programa.ofs
               │
               ▼
        programa.c (código C)
               │
               ▼
        gcc -O2 → programa.exe
```

**Status**: Funcional, compila codigo OFS

---

### Geração v2: Compilador em OFS (Auto-Hosting com LLVM IR)

```
┌─────────────────────────────────────────┐
│ ofscc (auto-hospedado em OFS)           │
│ ofs/ofscc/*.ofs (~4,500 LOC)            │
│ Emissão direta de LLVM IR nativo (.ll)  │
└──────────────┬──────────────────────────┘
               │
               ▼
        ofscc.ll (LLVM IR)
               │
               ▼
        llc -filetype=obj → ofscc.o
               │
               ▼
        ld ofscc.o magma.o → ofscc_v2
```

**Status**: Completo e determinístico

---

### Geração v3: Verificação de Determinismo Byte-a-Byte

```
        ofscc_v2
               │
        compila ofs/ofscc/ofscc.ofs
               │
        gera ofscc_v3.ll
               │
        llc + ld → ofscc_v3
               │
        ┌──────▼─────────────┐
        │ VERIFICA:           │
        │ v2 === v3 ?         │
        │ (byte-a-byte)       │
        └─────────────────────┘
```

**Esperado**: ofscc_v2 === ofscc_v3 (determinismo 100%)

---

## Mudança Implementada: Compilador e Runtime

Antes:
- Compilador dependia de C++ e runtime em C (`libofs_runtime.a`)
- Build requeria Clang e bibliotecas externas

Depois:
- Compilador 100% em OFS (`ofs/ofscc/`, ~4,500 LOC)
- Runtime nativo puro em OFS (**Stack Magma**: `ofs/stdlib/runtime/*.ofs`)
- Compilação direta via `llc` e ligadores nativos do SO (`ld` / `lld`)
- Auto-hosting comprovado e determinístico

---

## Performance

### Compilação Direta (Zero Clang)

```
Entrada:  programa.ofs
          ↓
        ofscc (emissão direta de LLVM IR .ll)
          ↓
        llc -filetype=obj (código de máquina .o)
          ↓
        ld / lld (+ magma.o do Stack Magma)
          ↓
Final:   executável nativo (ELF / PE32+)
```

**Total**: ~50-100ms para compilação completa nativa sem Clang!

### Execução

```
OFS compilado:  ~= C (100% velocidade nativa)
Python:         ~50x mais lento
Node.js:        ~10x mais lento
```

---

## Status das Fases de Evolução

### Phase B: Otimização e Suporte Cross-Platform
- [x] Melhorar emissão de código OFS com LLVM IR SSA
- [x] Suporte cross-platform (Linux nativo ELF e Windows nativo PE32+)
- [x] CLI completo (`build`, `run`, `check`, `ir`, `asm`, `ui`, `tokens`, `ast`)

### Phase C: LLVM Backend Direto (Zero-Clang)
- [x] Geração de LLVM IR direto (`llvmgen.ofs`), sem transpiladores intermediários
- [x] Compilação ultra-rápida via `llc` e ligadores nativos do sistema (`ld` e `ld.lld`)
- [x] Determinismo byte-a-byte validado (`cmp -s v3.ll v4.ll`)

### Phase D: Auto-Hospedagem e Runtime Puro
- [x] Compilador 100% auto-hospedado em `ofs/ofscc/`
- [x] Stack Magma: runtime 100% puro em OFS (`stdlib/runtime/`), sem bibliotecas estáticas externas em C
- [x] Bootstrap reprodutível via `ofs/bootstrap/scripts/bootstrap-minimal.sh`

### Phase E: Distribuição e Interfaces Nativas
- [x] Instaladores nativos interativos desenvolvidos na própria linguagem OFS (`src/packaging/`)
- [x] Obsidian Layout Language (.oll) com renderização e janelas nativas a 60 FPS
- [x] Extensão oficial para VS Code com diagnósticos e suporte à família OFS

---

## Conclusão

**OFS é uma linguagem genuinamente COMPILADA e AUTO-SUFICIENTE:**

```
Código OFS (.ofs) → [Compilador ofscc] → LLVM IR (.ll) → [llc] → Objeto (.o) → [ld + magma.o] → Executável Nativo
                          ↑
                    100% puro em OFS
                          ↑
                    Stack Magma & stdlib
```

- Compilação direta para código de máquina nativo
- Zero interpretadores, zero máquinas virtuais (sem JVM/CLR)
- Zero dependência de Clang ou compiladores C no pipeline diário
- Runtime puro Stack Magma escrito em OFS
- Interface visual declarativa moderna (OLL) com loop de eventos nativo
