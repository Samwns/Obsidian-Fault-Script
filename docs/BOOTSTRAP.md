# OFS Self-Hosting Bootstrap Guide

> **Nova Realidade**: O compilador OFS é completamente self-hosted e determinístico.
> Use `bootstrap.sh` para instalar a linguagem inteira em modo rapido e sem C++.
> `bootstrap-minimal.sh` continua existindo por compatibilidade com releases e docs antigas.

---

## ⚡ Quick Start (Recomendado)

```bash
# Tudo em um comando - ~2 segundos!
bash ofscc/scripts/bootstrap.sh

# Pronto! Seu compilador nativo está em dist/ofscc
dist/ofs build seu_programa.ofs -o programa
```

Pronto! Apenas OFS compilando OFS.

`bootstrap-minimal.sh` continua existindo para compatibilidade com docs, scripts e usuarios antigos.
O ponto de entrada recomendado para instalacao completa da linguagem e `bootstrap.sh`.

## O que é Bootstrap?

**Bootstrap** ocorre quando um compilador compila a si mesmo. É a prova final de que uma linguagem é Turing-completa e expressiva o suficiente para implementar sistemas complexos.

- **Lisp** (1962): Primeira linguagem self-hosting
- **C** (1972): Bootstrapped e tornou-se padrão
- **Rust** (2011): Self-hosted em Rust
- **Go** (2015): Self-hosted em Go
- **OFS** (2024): **AGORA COMPLETAMENTE SELF-HOSTED** ✨🚀

---

## Como Funciona?

### Phase 1: Compilador inicial (uma vez)
Quando o repositório é clonado, `dist/ofscc` já contém um compilador OFS pré-compilado.

### Phase 2: Recompilação self-hosted

Por padrao, `bootstrap.sh` reconstrói o compilador a partir do IR versionado `dist/ofscc.ll`.
Isso e o caminho certo para instalacao: rapido, previsivel e sem depender de C++/CMake.

Para validar o ciclo OFS -> OFS explicitamente:

```bash
OFS_BOOTSTRAP_SELFHOST=1 bash ofscc/scripts/bootstrap.sh
```
```bash
# Compila ofscc.ofs (código-fonte do compilador)
# usando dist/ofscc (compilador binário existente)
# -> novo dist/ofscc
```

### Phase 3: Verificação (determinismo)
```bash
# Compila novamente com o novo compilador
# Se ofscc_v2 === ofscc_v3 → SUCESSO!
# Compilador é determinístico ✓
```

---

## Como Usar

### Método 1: Bootstrap oficial (Recomendado) ⚡

```bash
# Navega para o repositório
cd Obsidian-Fault-Script

# Roda bootstrap.sh (não precisa de C++/CMake)
bash ofscc/scripts/bootstrap.sh

# Saída esperada:
# [✓] dist/ofscc criado com sucesso
# [✓] Checksum: a1b2c3d4... (repetível!)
# [✓] Determinismo verificado!
```

**Tempo**: ~2 segundos
**Dependências**: bash, clang e ar
**Saída**: `dist/ofscc` compilador nativo

### Método 2: Bootstrap com Validação (Legacy C++)

Se você quiser executar o bootstrap completo com validação (recompila 3 vezes), ainda pode:

```bash
# Roda validação completa de determinismo
bash ofscc/scripts/bootstrap-minimal.sh --validate

# Isso vai compilar:
# 1. dist/ofscc (v1)
# 2. dist/ofscc (v2, compilado por v1)
# 3. dist/ofscc (v3, compilado por v2)
# Depois verifica: v2 === v3? Se sim → ✓ SUCESSO
```

**Tempo**: ~5-10 segundos
**Validação**: Verifica determinismo byte-a-byte

---

## Troubleshooting

### Erro: "bash: ofscc/scripts/bootstrap-minimal.sh: Arquivo não encontrado"

**Causa**: Você não está no diretório correto.

**Solução**:
```bash
cd Obsidian-Fault-Script
bash ofscc/scripts/bootstrap-minimal.sh
```

### Erro: "dist/ofscc: permission denied"

**Causa**: Arquivo não tem permissão de execução.

**Solução**:
```bash
chmod +x dist/ofscc
# Ou roda bootstrap novamente
bash ofscc/scripts/bootstrap-minimal.sh
```

### Erro: "Failed to compile ofscc.ofs"

**Causa**: Compilador existente está corrompido ou incompatível.

**Solução**:
```bash
# Tente redownloadar o repositório fresco
git clone https://github.com/Samwns/Obsidian-Fault-Script.git
cd Obsidian-Fault-Script
bash ofscc/scripts/bootstrap-minimal.sh
```

### Erro: "Determinism check failed: v2 != v3"

**Causa**: Compilador não é determinístico (timestamps, random seeds, etc).

**Solução**: Este é um bug no compilador. Abra uma issue com:
```bash
bash ofscc/scripts/bootstrap-minimal.sh --debug
# Salva output.log com detalhes da compilação
```

---

## Arquitetura Pós-Bootstrap

```
Toolchain OFS Nativo (POST-BOOTSTRAP):

┌──────────────────────┐
│  dist/ofscc (nativo) │  Compilador OFS self-hosted
└──────────┬───────────┘
           │ lê arquivo .ofs
           │ análise léxica/sintática
           │ type-checking
           │ gera código C
           │ cc/gcc -O2 compila
           ▼
       executável nativo

Não há runtime C++. Apenas:
1. Código-fonte OFS
2. Compilador binário (dist/ofscc)
3. C compiler (gcc/clang) para link final
```

---

## Archivos que Podem Ser Removidos

Após bootstrap bem-sucedido, você pode arquivar (não remover):

**Para deletar (legacy):**
- ❌ `ofs/src/` (código-fonte C++ do compilador antigo)
- ❌ Qualquer CMakeLists.txt referente ao C++
- ❌ Dependências LLVM/Clang para build

**Para manter:**
- ✅ `dist/ofscc` (compilador nativo)
- ✅ `ofscc/` (código-fonte OFS do compilador)
- ✅ `stdlib/` (biblioteca padrão)
- ✅ `examples/` (exemplos)

---

## Próximos Passos

1. ✅ Bootstrap completo (sem C++)
2. 📦 Fazer releases (via GitHub Actions)
3. 🔗 Distribuir para package managers
4. 🧪 Criar test suite completo
5. 📚 Documentar stdlib completamente

---

## Referências

- [Getting Started Guide](./GETTING_STARTED.md) — Como começar
- [Language Reference](./LANGUAGE_REFERENCE.md) — Sintaxe completa
- [Compiler Architecture](./COMPILER_ARCHITECTURE.md) — Internals do compilador
- [Bootstrap Scripts](../ofscc/scripts/README.md) — Scripts disponíveis

---

**Status**: ✅ Compilador self-hosted e determinístico
**Próximo**: Execute releases automáticas via CI/CD
