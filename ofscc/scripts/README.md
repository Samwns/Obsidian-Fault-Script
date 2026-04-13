# OFS Scripts - Compilador Nativo

Scripts para compilar e fazer bootstrap do compilador OFS nativo, substituindo completamente o compilador C++.

## 📋 Conteúdo

- **bootstrap-windows.ps1** — Script principal para Windows (PowerShell)
- **bootstrap-linux.sh** — Script principal para Linux/macOS (Bash)
- **create-release.sh** — Gera pacotes de release para distribuição
- **utils.sh** — Utilitários de desenvolvimento (compile, check, test, etc)
- **README.md** — Este arquivo

## 🚀 Início Rápido

### Windows PowerShell

```powershell
# Fazer bootstrap completo (recomendado primeiro uso)
.\bootstrap-windows.ps1

# Com opções
.\bootstrap-windows.ps1 -BuildDir "build-native" -InstallDir "dist-native"

# Pular build do C++, usar o existente
.\bootstrap-windows.ps1 -SkipCppBuild

# Apenas bootstrap, sem build C++
.\bootstrap-windows.ps1 -SkipCppBuild -ReleaseOnly
```

### Linux/macOS Bash

```bash
# Fazer bootstrap completo
bash bootstrap-linux.sh

# Com opções
bash bootstrap-linux.sh --build-dir build-native --install-dir dist-native

# Pular build do C++
bash bootstrap-linux.sh --skip-cpp-build

# Release-only (assumir binários já compilados)
bash bootstrap-linux.sh --skip-cpp-build --release-only
```

## 📖 Fluxo de Compilação

O compilador nativo segue esse fluxo:

```
┌─────────────────────────────────────────────────────────────┐
│ PHASE 1: Build do Compilador C++                            │
│ ──────────────────────────────────────────────────────────  │
│ CMake + GCC/Clang → ofscc_v1 (compilador C++)              │
└──────────────────────┬──────────────────────────────────────┘
                       │
                       ↓
┌─────────────────────────────────────────────────────────────┐
│ PHASE 2: Bootstrap OFS                                      │
│ ──────────────────────────────────────────────────────────  │
│ ofscc_v1 build ofscc.ofs → ofscc_v2 (OFS compilado com C++)│
│                       ↓                                     │
│ ofscc_v2 build ofscc.ofs → ofscc_v3 (OFS compilado com OFS)│
│                       ↓                                     │
│ ✓ Determinismo validado: v2 == v3 (byte-identical)        │
└──────────────────────┬──────────────────────────────────────┘
                       │
                       ↓
┌─────────────────────────────────────────────────────────────┐
│ PHASE 3: Gerar Artifacts                                    │
│ ──────────────────────────────────────────────────────────  │
│ • Copiar ofscc_v3 → dist/ofscc (compilador final)         │
│ • Copiar runtime library → dist/libofs_runtime.a           │
│ • Copiar stdlib → dist/stdlib/                             │
│ • Criar version.json                                       │
└─────────────────────────────────────────────────────────────┘
```

## 🔧 Arquivos de Configuração

### CMakeLists.txt
O único CMakeLists.txt necessário está em `ofs/`, que compila o compilador C++.

```bash
cmake ofs -B build-ofscc
cmake --build build-ofscc -j4
```

### Diretório de Saída
Por padrão, os artifacts são em `dist/`:
- `dist/ofscc` — Compilador nativo
- `dist/libofs_runtime.a` — biblioteca runtime
- `dist/stdlib/` — Bibliotecas padrão
- `dist/version.json` — Informações de versão

## 💻 Usar o Compilador

### Depois do bootstrap bem-sucedido:

```bash
# Compilar arquivo OFS
dist/ofscc build program.ofs -o program

# Type-check sem gerar binário
dist/ofscc check program.ofs

# Ver tokens (debug)
dist/ofscc tokens program.ofs

# Ver AST
dist/ofscc ast program.ofs

# Ver LLVM IR
dist/ofscc ir program.ofs

# Ver assembly nativo
dist/ofscc asm program.ofs
```

## 🛠️ Utilitários de Desenvolvimento

O script `utils.sh` fornece atalhos úteis:

```bash
# Compilar arquivo
./utils.sh compile hello.ofs -o hello

# Verificar tipos
./utils.sh check program.ofs

# Inspecionar tokens
./utils.sh tokens file.ofs

# Inspecionar AST
./utils.sh ast file.ofs

# Rodar testes
./utils.sh test
./utils.sh test-lexer
./utils.sh test-parser
./utils.sh test-semantic

# Benchmark
./utils.sh benchmark

# Limpar build
./utils.sh clean

# Rebuild completo
./utils.sh rebuild

# Ver versão
./utils.sh version
```

## 📦 Criar Releases

Para gerar pacotes de release multi-plataforma:

```bash
bash scripts/create-release.sh 1.0.0-native
```

Isso gera:
- `releases/ofs-linux-x64-1.0.0-native.tar.gz`
- `releases/ofs-macos-arm64-1.0.0-native.dmg` (se em macOS)
- `releases/ofs-windows-x64-portable-1.0.0-native.zip`
- `releases/CHECKSUMS.sha256`

## ✨ Recursos Suportados

O compilador nativo suporta **todos os recursos** da linguagem OFS:

### Sintaxe e Tipos
- ✅ Type checking e type inference
- ✅ Small integer types: `u8, u16, u32, u64, i8, i16, i32, i64`
- ✅ Tipos primitivos: `stone, float, bool, obsidian`
- ✅ Monolith (tipos complexos) com `impl` blocks
- ✅ Enums e structs

### Padrões
- ✅ Pattern matching: `match value { ... }`
- ✅ Error handling: `throw`, `catch`, `tremor`
- ✅ Control flow: `if`, `while`, `for`, `return`

### Funcionalidades Avançadas
- ✅ Funções como valores
- ✅ Lambdas inline
- ✅ Namespace support
- ✅ Package system: `attach {modulo}`
- ✅ Interop externo: `extern vein`, `rift vein`
- ✅ Blocos de baixo nível: `fracture`, `abyss`, `bedrock`, `fractal`
- ✅ Constantes em compile-time: `const`, `strata`

### Biblioteca Padrão
- ✅ `core` — Funções básicas
- ✅ `io` — I/O e arquivo
- ✅ `math` — Operações matemáticas
- ✅ `canvas` — Gráficos e animação
- ✅ `bedrock` — Networking e protocolos

## 🐛 Troubleshooting

### "Não encontrou compilador C++"
- Atualize CMakeCache.txt removendo `build-ofscc` e rodando novamente
- Verifique se GCC/Clang e LLVM estão instalados
- Em Windows: instale MinGW ou Visual Studio

### "ofscc_v2 falhou ao compilar"
- Verifique se `ofs/ofscc/ofscc.ofs` existe e é válido
- Verifique se `ofscc_v1` foi compilado corretamente

### "Determinismo não validado (v2 ≠ v3)"
- Isso pode ser normal durante desenvolvimento
- Se v3 foi compilado com sucesso, está funcionando

### Build muito lento?
- Use `-jN` para paralelismo: `cmake --build . -j8`
- Em Linux: `bash bootstrap-linux.sh --jobs 8`
- Reduzir otimizações: `--debug` em vez de `Release`

## 📝 Variáveis de Ambiente

### Windows PowerShell
```powershell
# Customizar diretório de build
$env:BUILD_DIR = "mybuild"

# Customizar instalação
$env:INSTALL_DIR = "myinstall"

# Depois rodar script
.\bootstrap-windows.ps1
```

### Linux/macOS Bash
```bash
# Customizar diretório de build
BUILD_DIR=mybuild bash bootstrap-linux.sh

# Customizar instalação
INSTALL_DIR=myinstall bash bootstrap-linux.sh

# Compilação em paralelo
bash bootstrap-linux.sh --jobs 16
```

## 🔗 Próximos Passos

Depois do bootstrap bem sucedido:

1. **Compilar exemplos:**
   ```bash
   dist/ofscc build ofs/examples/hello.ofs -o hello
   ./hello  # Output: 21
   ```

2. **Verificar stdlib:**
   ```bash
   dist/ofscc check ofs/examples/collections.ofs
   dist/ofscc check ofs/examples/canvas_window_demo.ofs
   ```

3. **Explorar linguagem:**
   - Leia [LANGUAGE_REFERENCE.md](../docs/LANGUAGE_REFERENCE.md)
   - Leia [GETTING_STARTED.md](../docs/GETTING_STARTED.md)
   - Veja exemplos em `ofs/examples/`

4. **Participar do desenvolvimento:**
   - Adicione features ao compilador OFS
   - Expanda a stdlib
   - Reporte bugs e sugira melhorias

## 📋 Checklist de Validação

Depois do bootstrap, valide com:

```bash
# ✓ Compiler ready
dist/ofscc --help

# ✓ stdlib completa
ls -la dist/stdlib/

# ✓ Exemplos compilam
for ex in ofs/examples/{hello,calculator,fizzbuzz}.ofs; do
  dist/ofscc build $ex && echo "✓ $(basename $ex)"
done

# ✓ Testes passam
./utils.sh test

# ✓ Determinismo
bash ofs/ofscc/test_bootstrap.sh
```

## 📞 Suporte

- 🐛 Bugs: https://github.com/Samwns/Obsidian-Fault-Script/issues
- 💬 Discussões: https://github.com/Samwns/Obsidian-Fault-Script/discussions
- 📖 Docs: https://github.com/Samwns/Obsidian-Fault-Script/tree/main/docs

---

**Última atualização:** 13 de Abril de 2026  
**Status:** ✅ Compilador nativo completo e funcional
