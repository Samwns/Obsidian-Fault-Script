# Obsidian Fault Script - VS Code Extension

Documentação da extensão VS Code para linguagem Obsidian Fault Script (OFS).

Language / Idioma:

- [English](#english)
- [Portugues (Brasil)](#portugues-brasil)

---

## English

### What This Extension Provides

- Syntax highlighting for `.ofs` files
- Snippets for faster authoring
- Hover documentation for OFS keywords, CLI commands, stdlib helpers, `bedrock`, `rift`, and `fault_*`
- Hover/completion support for interop metadata such as `bind` and `abi`
- Autocomplete for keywords, types, stdlib/package names, builtins, low-level intrinsics, and symbols from attached libraries
- Attach resolution for `attach {name}`, `attach {F:relative.ofs}`, `attach {F:/absolute/path.ofs}`, `ofs/stdlib`, `src/packages/src`, and `OFS_LIB_PATH`
- Diagnostics using quiet `ofs check`
- LLVM IR and native assembly emission using `ofs ir` and `ofs asm`
- Native VS Code Run and Debug integration (F5 / Run and Debug panel)
- Embedded Linux x64 self-hosted compiler (`bin/linux-x64/ofscc`) with an `ofs` wrapper
- Command palette action: `OFS: Check`
- Command palette action: `OFS: Emit Native Assembly`
- Built-in OFS theme: `Obsidian Fault Purple`

This extension should be updated together with language changes. When OFS gains new syntax, stdlib modules, or package workflows, the extension grammar, snippets, hover docs, and README should be updated in the same change.

### Requirements

- VS Code `^1.85.0`
- OFS compiler (`ofs`) in PATH, configured in `ofs.path`, or the embedded Linux x64 compiler shipped with the extension

### Settings

- `ofs.path`
: Path to OFS executable. Default: `ofs`.

- `ofs.diagnosticsOnType`
: Run diagnostics while typing (debounced). Default: `true`.

- `ofs.autoInstallCompiler`
: Automatically installs OFS compiler on extension activation when missing. Default: `true`.

- `ofs.preferEmbeddedCompiler`
: Prefer compiler embedded in extension (`bin/<platform>/ofs`) before workspace/system compiler. Default: `true`.

The Linux x64 package includes the self-hosted compiler and emits LLVM IR directly. The `ofs` wrapper uses LLVM/clang tools to assemble/link native output.

### Commands

- `OFS: Check` (`ofs.checkFile`)
- `OFS: Emit Native Assembly` (`ofs.emitAssembly`)

The extension follows the current OFS direction: high-level and low-level OFS should coexist in the same codebase, while LLVM IR and target assembly remain inspection tools instead of the language's identity.

That includes:

- small integer types such as `u8`, `u16`, `u32`, `u64`, `i8`, `i16`, `i32`,
- `impl` blocks for monolith methods,
- `namespace` declarations,
- `cycle (value in array)` and C-style `cycle (forge i = 0; i < n; i++)`,
- explicit casts with `as`,
- typed pointer work with `shard p: *stone = &x` and `*p = ...`,
- `bedrock { ... }`, `fracture { ... }`, and `abyss { ... }` for low-level regions,
- `rift vein` for OFS-native interop boundaries,
- `bind` and `abi` metadata for explicit external boundaries,
- `fault_*` intrinsics for machine-like operations with OFS naming,
- stdlib/package modules such as `bedrock`, `bedrock-packet`, `fmt`, `terminal-colors`, `memory-modes`, `rift`, `canvas`, and `window`.

The current compiler/runtime test matrix passes all examples in `ofs/examples/*.ofs`, package attach smokes, and CLI smoke checks for `check`, `ir`, `asm`, `build`, and `run`. `canvas`/`window` have headless runtime stubs on Linux x64 so examples compile and run in Codespaces/CI; real interactive presentation remains platform/runtime work.

Run/Debug now uses native VS Code debug configurations (`ofs-native`) instead of custom extension buttons.

### Install From VSIX

```bash
code --install-extension src/vscode-extension.vsix
```

Or in VS Code: Extensions -> `...` -> `Install from VSIX...`

Important: do not use Visual Studio `VSIXInstaller.exe`; this package targets Visual Studio Code only.

### Build Extension Package Locally

```bash
cd src/vscode-extension
npm ci
npm run package
```

### Troubleshooting

- `ofs: command not found`
: Set `ofs.path` in settings or add OFS to PATH.

- No diagnostics shown
: Ensure `ofs.diagnosticsOnType` is enabled and the file is saved as `.ofs`.

- Attach/autocomplete for libraries not appearing
: Verify `attach` paths, `OFS_LIB_PATH`, or that the workspace contains `ofs/stdlib` / `src/packages/src`.

### Repository Links

- Main repository: https://github.com/Samwns/Obsidian-Fault-Script
- Issues: https://github.com/Samwns/Obsidian-Fault-Script/issues

---

## Portugues (Brasil)

### O Que Esta Extensao Oferece

- Highlight de sintaxe para arquivos `.ofs`
- Snippets para acelerar a escrita
- Hover docs para palavras-chave, comandos CLI, helpers de stdlib, `bedrock`, `rift` e `fault_*`
- Suporte de hover/autocomplete para metadados de interop como `bind` e `abi`
- Autocomplete para keywords, tipos, nomes de stdlib/pacotes, builtins, intrinsics low-level e símbolos de bibliotecas anexadas
- Resolução de attach para `attach {name}`, `attach {F:relativo.ofs}`, `attach {F:/absoluto/ofs}`, `ofs/stdlib`, `src/packages/src` e `OFS_LIB_PATH`
- Diagnosticos usando `ofs check` silencioso em sucesso
- Emissao de LLVM IR e assembly nativo usando `ofs ir` e `ofs asm`
- Integracao nativa com Executar/Depurar do VS Code (F5 / painel Run and Debug)
- Compilador self-hosted Linux x64 embutido (`bin/linux-x64/ofscc`) com wrapper `ofs`
- Acao na paleta de comandos: `OFS: Check`
- Acao na paleta de comandos: `OFS: Emit Native Assembly`
- Tema integrado: `Obsidian Fault Purple`
- Suporte a compilador embutido (`bin/<plataforma>/ofs`) com fallback de auto-instalacao
- Descoberta do compilador self-hosted no workspace (`ofs/ofscc/ofscc`, `ofs/ofscc/ofscc_v2`)

Esta extensao deve ser atualizada junto com a linguagem. Quando a OFS ganhar nova sintaxe, novos modulos de stdlib ou novos fluxos de pacote, a gramática, os snippets, os hovers e este README da extensao devem ser atualizados no mesmo conjunto de mudanças.

### Requisitos

- VS Code `^1.85.0`
- Compilador OFS no PATH, configurado em `ofs.path`, ou o compilador Linux x64 embutido na extensao

### Configuracoes

- `ofs.path`
: Caminho para o executavel OFS. Padrao: `ofs`.

Voce tambem pode configurar `ofs.path` para `ofscc`/`ofscc_v2` durante os testes do compilador self-hosted.

- `ofs.diagnosticsOnType`
: Executa diagnosticos enquanto digita (com debounce). Padrao: `true`.

- `ofs.autoInstallCompiler`
: Instala automaticamente o compilador OFS ao ativar a extensao quando ele nao estiver presente. Padrao: `true`.

- `ofs.preferEmbeddedCompiler`
: Prioriza o compilador embutido na extensao (`bin/<plataforma>/ofs`) antes do compilador do workspace/sistema. Padrao: `true`.

O pacote Linux x64 inclui o compilador self-hosted e emite LLVM IR diretamente. O wrapper `ofs` usa ferramentas LLVM/clang para montar/linkar a saida nativa.

### Comandos

- `OFS: Check` (`ofs.checkFile`)
- `OFS: Emit Native Assembly` (`ofs.emitAssembly`)

A extensao segue a direcao atual da OFS: alto nivel e baixo nivel devem coexistir no mesmo codebase, enquanto LLVM IR e assembly nativo continuam como ferramentas de inspecao, nao como identidade da linguagem.

Isso inclui:

- tipos pequenos como `u8`, `u16`, `u32`, `u64`, `i8`, `i16`, `i32`,
- blocos `impl` para métodos de `monolith`,
- declarações `namespace`,
- `cycle (valor in array)` e `cycle` estilo C,
- casts explicitos com `as`,
- ponteiros tipados com `shard p: *stone = &x` e `*p = ...`,
- `bedrock { ... }`, `fracture { ... }` e `abyss { ... }` para regioes low-level,
- `rift vein` para fronteiras de interoperabilidade nativas da OFS,
- metadados `bind` e `abi` para fronteiras externas explicitas,
- intrinsics `fault_*` para operacoes machine-like com nomes da propria linguagem,
- modulos stdlib/pacote como `bedrock`, `bedrock-packet`, `fmt`, `terminal-colors`, `memory-modes`, `rift`, `canvas` e `window`.

A matriz atual do compilador/runtime passa todos os exemplos em `ofs/examples/*.ofs`, smokes de attach/pacotes e comandos `check`, `ir`, `asm`, `build` e `run`. `canvas`/`window` têm stubs headless no runtime Linux x64 para compilar/rodar em Codespaces/CI; apresentacao interativa real ainda e trabalho de runtime/plataforma.

Executar/Depurar agora usa configuracoes nativas de debug do VS Code (`ofs-native`) no lugar de botoes customizados da extensao.

### Instalar Via VSIX

```bash
code --install-extension src/vscode-extension.vsix
```

Ou no VS Code: Extensions -> `...` -> `Install from VSIX...`

Importante: nao use o `VSIXInstaller.exe` do Visual Studio; este pacote e apenas para Visual Studio Code.

### Gerar Pacote da Extensao Localmente

```bash
cd src/vscode-extension
npm ci
npm run package
```

### Solucao de Problemas

- `ofs: command not found`
: Configure `ofs.path` ou adicione OFS no PATH.

- Sem diagnosticos no editor
: Verifique se `ofs.diagnosticsOnType` esta ativo e se o arquivo e `.ofs`.

- Sem autocomplete para bibliotecas
: Confira caminhos de `attach`, `OFS_LIB_PATH`, ou se o workspace contem `ofs/stdlib` / `src/packages/src`.

### Links

- Repositorio principal: https://github.com/Samwns/Obsidian-Fault-Script
- Issues: https://github.com/Samwns/Obsidian-Fault-Script/issues
