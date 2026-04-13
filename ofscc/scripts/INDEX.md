# OFS Native Compiler Scripts - Index

**Última Atualização:** 13 de Abril de 2026

## 📂 Conteúdo da Pasta

### 🔨 Build e Bootstrap

#### `bootstrap-windows.ps1` (275 linhas)
**Descrição:** Script principal de bootstrap para Windows PowerShell  
**Uso:** `.\bootstrap-windows.ps1 [opções]`  
**Fases:**
1. Compila compilador C++ (CMake)
2. Bootstrap OFS (v1→v2→v3)
3. Gera artifacts (dll, stdlib, version.json)

**Opções:**
- `-SkipCppBuild` — Pula compilação C++
- `-SkipBootstrap` — Pula bootstrap OFS
- `-ReleaseOnly` — Apenas artifacts
- `-BuildDir DIR` — Customizar dir build
- `-InstallDir DIR` — Customizar dir install

---

#### `bootstrap-linux.sh` (355 linhas)
**Descrição:** Script principal de bootstrap para Linux/macOS  
**Uso:** `bash bootstrap-linux.sh [opções]`  
**Fases:** Idênticas ao Windows  

**Opções:**
- `--skip-cpp-build` — Pula compilação C++
- `--skip-bootstrap` — Pula bootstrap OFS
- `--release-only` — Apenas artifacts
- `--debug` — Build em debug mode
- `--jobs N` — Paralelismo (default: nproc)
- `--build-dir DIR` — Customizar dir build
- `--install-dir DIR` — Customizar dir install

---

### 🛠️ Utilitários

#### `utils.ps1` (270 linhas)
**Descrição:** Utilitários de desenvolvimento para Windows  
**Uso:** `.\utils.ps1 <comando> [args]`  

**Comandos:**
```
compile <file.ofs>        Compilar arquivo OFS
check <file.ofs>          Type-check sem codegen
tokens <file.ofs>         Mostrar tokens do lexer
ast <file.ofs>            Mostrar AST
ir <file.ofs>             Mostrar LLVM IR
asm <file.ofs>            Mostrar assembly

test [pattern]            Rodar testes
test-lexer                Rodar testes do lexer
test-parser               Rodar testes do parser
test-semantic             Rodar testes semântico

benchmark                 Performance benchmarks
clean                     Limpar artifacts
rebuild                   Clean + full rebuild
version                   Mostrar versão
```

---

#### `utils.sh` (330 linhas)
**Descrição:** Utilitários de desenvolvimento para Linux/macOS  
**Uso:** `bash utils.sh <comando> [args]`  
**Comandos:** Idênticos ao utils.ps1

---

### 📦 Release e Packaging

#### `create-release.sh` (180 linhas)
**Descrição:** Gera pacotes de release multi-plataforma  
**Uso:** `bash create-release.sh [VERSION]`  

**Saída:**
- `releases/ofs-linux-x64-{VERSION}.tar.gz`
- `releases/ofs-macos-arm64-{VERSION}.dmg` (se em macOS)
- `releases/ofs-windows-x64-portable-{VERSION}.zip`
- `releases/CHECKSUMS.sha256`

**Inclusão:**
- Compilador ofscc
- Runtime library (libofs_runtime.a)
- Stdlib completa
- Documentação
- version.json com metadata

---

### ✅ Validação

#### `validate.sh` (200 linhas)
**Descrição:** Valida build do compilador  
**Uso:** `bash validate.sh [-v|--verbose]`  

**Checks:**
1. Compilador existe e é executável
2. Version e metadata corretos
3. Stdlib presente e completa
4. Bootstrap determinístico (v2 == v3)
5. Compilação básica funciona
6. Comandos estão disponíveis
7. Runtime library presente

---

### 🤖 CI/CD

#### `ci-build.sh` (180 linhas)
**Descrição:** Automação para CI/CD pipelines  
**Uso:** `bash ci-build.sh`  

**Variáveis de Ambiente:**
- `VERSION` — Versão do release
- `BUILD_TYPE` — Release ou Debug
- `ENABLE_LTO` — Link-Time Optimization
- `PLATFORMS` — Plataformas alvo

**Saída:** `release-artifacts/` com metadata JSON

---

### 📋 Make (Unix)

#### `Makefile` (280 linhas)
**Descrição:** Makefile com targets convenientes  
**Uso:** `make <target> [options]`  

**Targets principais:**
```
bootstrap               Full build (default)
cpp-build              Only C++ compiler
test/test-lexer/etc    Run tests
compile/check/tokens   Compilation utilities
examples               Compile example programs
release                Generate release packages
install/uninstall      System installation
clean/rebuild          Maintenance
help                   Show all targets
```

**Exemplo:**
```bash
make bootstrap
make compile FILE=hello.ofs
make test
make release VERSION=1.1.0
make install DESTDIR=/opt
```

---

### 📚 Documentação

#### `README.md` (500+ linhas)
**Descrição:** Guia completo de uso dos scripts  
**Conteúdo:**
- Setup rápido por plataforma
- Fluxo de compilação explicado
- Referência completa de commands
- Troubleshooting
- Configuração avançada

---

#### `QUICKSTART.md` (200+ linhas)
**Descrição:** Quick start 5 minutos  
**Conteúdo:**
- Comandos essenciais
- Features suportados
- Próximos passos
- Troubleshooting rápido

---

#### `MIGRATION_SUMMARY.md` (este arquivo +400 linhas)
**Descrição:** Summary técnico da migração  
**Conteúdo:**
- O que foi implementado
- Arquitetura de build
- Features suportados
- Estatísticas
- Changelog

---

#### `INDEX.md` (este arquivo)
**Descrição:** Índice de todos os scripts  
**Conteúdo:**
- Descrição de cada arquivo
- Opções e commands
- Exemplos de uso

---

## 🎯 Fluxo Recomendado

### Primeira Execução (Bootstrap Completo)

```bash
# Windows
.\bootstrap-windows.ps1

# Linux/macOS  
bash bootstrap-linux.sh
```

### Desenvolvimento Contínuo

```bash
# Compilar programa
bash scripts/utils.sh compile my_program.ofs -o my_program

# Verificar tipos
bash scripts/utils.sh check my_program.ofs

# Rodar testes
bash scripts/utils.sh test

# Com Make
make compile FILE=my_program.ofs
make test
```

### Releases Automáticas

```bash
# Criar release
bash scripts/create-release.sh 1.0.0

# Validar
bash scripts/validate.sh

# CI/CD
bash scripts/ci-build.sh
```

---

## 🔗 Relações entre Scripts

```
bootstrap-windows.ps1/linux.sh  ← Scripts principais
    ↓ ↓
    ├→ cria dist/ofscc
    ├→ cria dist/stdlib/
    └→ cria dist/version.json

utils.ps1/sh  ← Usa dist/ofscc para operações
    ├→ compile
    ├→ check
    ├→ tokens/ast/ir/asm
    └→ test

validate.sh  ← Valida output de bootstrap
    ├→ Verifica dist/ofscc
    ├→ Verifica stdlib
    └→ Compila exemplos

create-release.sh  ← Packing após bootstrap
    ├→ Copia dist/ofscc
    ├→ Copia dist/stdlib/
    └→ Gera tar.gz/zip/dmg

ci-build.sh  ← Orquestra tudo para CI
    ├→ bootstrap-linux.sh
    ├→ validate.sh
    ├→ create-release.sh
    └→ Gera metadata JSON

Makefile  ← Interface Make
    ├→ Chama bootstrap-linux.sh
    ├→ Chama utils.sh
    ├→ Chama create-release.sh
    └→ Targets para development
```

---

## 📊 Estatísticas

### Tamanho dos Scripts
| Arquivo | Linhas | Tipo |
|---|---|---|
| bootstrap-windows.ps1 | 275 | PowerShell |
| bootstrap-linux.sh | 355 | Bash |
| create-release.sh | 180 | Bash |
| utils.ps1 | 270 | PowerShell |
| utils.sh | 330 | Bash |
| validate.sh | 200 | Bash |
| ci-build.sh | 180 | Bash |
| Makefile | 280 | Makefile |
| **TOTAL** | **~2080** | **Produção** |

### Documentação
| Arquivo | Linhas | Propósito |
|---|---|---|
| README.md | 500+ | Guia completo |
| QUICKSTART.md | 200+ | Quick start |
| MIGRATION_SUMMARY.md | 400+ | Technical summary |
| INDEX.md (este) | 300+ | Índice |

---

## ✨ Features Suportados

Todos os features da linguagem OFS estão 100% suportados:

- ✅ Type system completo (stone, float, bool, obsidian, small ints)
- ✅ Pattern matching
- ✅ Error handling (throw/catch/tremor)
- ✅ Monolith + impl
- ✅ Namespace
- ✅ Lambda e function values
- ✅ Package system (attach)
- ✅ External C bindings (extern vein, rift vein)
- ✅ Low-level blocks (fracture, abyss, bedrock, fractal)
- ✅ Complete stdlib (core, io, math, canvas, bedrock)

---

## 🚀 Próximos Passos

1. **Ler README.md** para visão geral completa
2. **Executar bootstrap** com script apropriado
3. **Rodar validate.sh** para verificar tudo
4. **Explorar utils.sh** para operações diárias
5. **Criar releases** com create-release.sh

---

## 📞 Perguntas Frequentes

**P: Qual script devo usar?**  
R: Primeira vez? Use `bootstrap-windows.ps1` (Windows) ou `bootstrap-linux.sh` (Unix)

**P: Como compilar programas?**  
R: Use `utils.ps1 compile file.ofs` ou `bash utils.sh compile file.ofs`

**P: Como criar releases?**  
R: `bash scripts/create-release.sh 1.0.0`

**P: Qual é a diferença entre v2 e v3?**  
R: v2 = OFS compilado por C++; v3 = OFS compilado por OFS. Devem ser idênticos.

**P: Posso usar Make?**  
R: Sim! `make bootstrap`, `make test`, `make release`, etc.

---

**Status:** ✅ Compilador nativo completo e funcional  
**Data:** 13 de Abril de 2026  
**Versão:** 1.0.0-native
