# OFS Self-Hosting Bootstrap Guide

> **Nova Realidade**: O compilador OFS é completamente self-hosted e determinístico.
> Use `ofs/bootstrap/scripts/bootstrap-minimal.sh` para compilar em ~2 segundos.

---

## Quick Start (Recomendado)

```bash
# Tudo em um comando - ~2 segundos!
bash ofs/bootstrap/scripts/bootstrap-minimal.sh

# Pronto! Seu compilador nativo está em ofs/dist/ofscc
ofs/dist/ofscc build seu_programa.ofs -o programa
```

Pronto! Apenas OFS compilando OFS.

## O que é Bootstrap?

**Bootstrap** ocorre quando um compilador compila a si mesmo. É a prova final de que uma linguagem é Turing-completa e expressiva o suficiente para implementar sistemas complexos.

- **Lisp** (1962): Primeira linguagem self-hosting
- **C** (1972): Bootstrapped e tornou-se padrão
- **Rust** (2011): Self-hosted em Rust
- **Go** (2015): Self-hosted em Go
- **OFS** (2024-2026): **COMPLETAMENTE SELF-HOSTED COM STACK MAGMA**

---

## Como Funciona?

### Phase 1: Compilador inicial (uma vez)
Quando o repositório é clonado, `ofs/dist/ofscc` já contém um compilador OFS pré-compilado.

### Phase 2: Recompilação (bootstrap-minimal)
```bash
# Compila ofs/ofscc/ofscc.ofs (código-fonte do compilador)
# usando ofs/dist/ofscc (compilador binário existente)
# -> novo ofs/dist/ofscc
```

### Phase 3: Verificação (determinismo)
```bash
# Compila novamente com o novo compilador
# Se ofscc_v2 === ofscc_v3 -> SUCESSO!
# Compilador e deterministico [OK]
```

---

## Como Usar

### Método 1: Bootstrap Minimal (Recomendado)

```bash
# Navega para o repositório
cd Obsidian-Fault-Script

# Roda bootstrap-minimal.sh
bash ofs/bootstrap/scripts/bootstrap-minimal.sh

# Saída esperada:
# [OK] ofs/dist/ofscc criado com sucesso
# [OK] Checksum: a1b2c3d4... (repetivel!)
# [OK] Determinismo verificado!
```

**Tempo**: ~2 segundos
**Dependências**: Nenhuma (apenas bash e binários nativos)
**Saída**: `ofs/dist/ofscc` compilador nativo

### Método 2: Bootstrap com Validação de Determinismo

Se você quiser executar o bootstrap completo com validação (recompila 3 vezes):

```bash
# Roda validação completa de determinismo
bash ofs/bootstrap/scripts/bootstrap-minimal.sh --validate

# Isso vai compilar:
# 1. ofs/dist/ofscc (v1)
# 2. ofs/dist/ofscc (v2, compilado por v1)
# 3. ofs/dist/ofscc (v3, compilado por v2)
# Depois verifica: v2 === v3? Se sim -> [OK] SUCESSO
```

**Tempo**: ~5-10 segundos
**Validação**: Verifica determinismo byte-a-byte

---

## Troubleshooting

### Erro: "bash: ofs/bootstrap/scripts/bootstrap-minimal.sh: Arquivo não encontrado"

**Causa**: Você não está no diretório raiz do repositório.

**Solução**:
```bash
cd Obsidian-Fault-Script
bash ofs/bootstrap/scripts/bootstrap-minimal.sh
```

### Erro: "ofs/dist/ofscc: permission denied"

**Causa**: Arquivo não tem permissão de execução.

**Solução**:
```bash
chmod +x ofs/dist/ofscc
# Ou roda bootstrap novamente
bash ofs/bootstrap/scripts/bootstrap-minimal.sh
```

### Erro: "Failed to compile ofscc.ofs"

**Causa**: Compilador existente está corrompido ou incompatível.

**Solução**:
```bash
# Clone o repositório fresco
git clone https://github.com/Samwns/Obsidian-Fault-Script.git
cd Obsidian-Fault-Script
bash ofs/bootstrap/scripts/bootstrap-minimal.sh
```

### Erro: "Determinism check failed: v2 != v3"

**Causa**: Compilador não é determinístico (timestamps, random seeds, etc).

**Solução**: Abra uma issue com:
```bash
bash ofs/bootstrap/scripts/bootstrap-minimal.sh --debug
# Salva output.log com detalhes da compilação
```

---

## Arquitetura Pós-Bootstrap

```
Toolchain OFS Nativo (POST-BOOTSTRAP):

┌──────────────────────────┐
│  ofs/dist/ofscc (nativo) │  Compilador OFS self-hosted
└────────────┬─────────────┘
             │ lê arquivo .ofs
             │ análise léxica/sintática
             │ type-checking
             │ gera LLVM IR nativo (.ll)
             │ llc -filetype=obj compila para objeto de máquina (.o)
             │ ld / ld.lld liga nativamente com Stack Magma (magma.o)
             ▼
         executável nativo (ELF no Linux, PE32+ no Windows)

Sem dependência de compilador C ou runtime externo em C:
1. Código-fonte OFS (.ofs, .oll, .odl, .oes)
2. Compilador binário self-hosted (ofs/dist/ofscc)
3. Runtime nativo 100% puro em OFS (Stack Magma: ofs/dist/magma.o)
4. Ligador nativo do sistema (ld / ld.lld)
```

---

## Estrutura Canônica Mantida

Após bootstrap bem-sucedido, o layout canônico é:

- `ofs/dist/ofscc` (compilador nativo)
- `ofs/dist/magma.o` (runtime Stack Magma nativo)
- `ofs/ofscc/` (código-fonte OFS do compilador)
- `ofs/stdlib/` (biblioteca padrão)
- `ofs/examples/` (exemplos de código)
- `src/packaging/` (instaladores visuais OLL para Linux e Windows)
- `src/vscode-extension/` (extensão com realce e suporte a .ofs, .oll, .odl, .oes)

---

## Próximos Passos

1. Bootstrap completo (sem C++)
2. Fazer releases (via GitHub Actions)
3. Distribuir para package managers
4. Rodar test suite nativo com Stack Magma
5. Documentar stdlib completamente

---

## Referências

- [Getting Started Guide](./GETTING_STARTED.md) — Como começar
- [Language Reference](./LANGUAGE_REFERENCE.md) — Sintaxe completa
- [Compiler Architecture](./COMPILER_ARCHITECTURE.md) — Internals do compilador
- [Bootstrap Scripts](../ofs/bootstrap/scripts/INDEX.md) — Scripts disponíveis

---

**Status**: Compilador self-hosted e determinístico com Stack Magma
**Pipeline**: 100% nativo (zero Clang, zero runtime em C)

