# Obsidian Fault Script - VS Code Extension

Dedicated documentation for the VS Code extension of Obsidian Fault Script (OFS).

Language / Idioma:

- [English](#english)
- [Portugues (Brasil)](#portugues-brasil)

---

## English

### What This Extension Provides

- Syntax highlighting for `.ofs` files
- Snippets for faster authoring
- Hover documentation for OFS keywords and commands
- Hover documentation for low-level OFS surfaces such as `bedrock`, `rift`, and `fault_*`
- Hover/completion support for interop and layout metadata such as `bind`, `abi`, and `monolith ... layout packed`
- Autocomplete for core language symbols
- Autocomplete for attached libraries resolved from `attach` and `OFS_LIB_PATH`
- Diagnostics using `ofs check`
- Native assembly emission using `ofs asm`
- Native VS Code Run and Debug integration (F5 / Run and Debug panel)
- Command palette action: `OFS: Check`
- Command palette action: `OFS: Emit Native Assembly`
- Built-in OFS theme: `Obsidian Fault Purple`
- Embedded compiler support (`bin/<platform>/ofs`) with auto-install fallback

### Requirements

- VS Code `^1.85.0`
- OFS compiler in PATH or configured with `ofs.path`

### Settings

- `ofs.path`
: Path to OFS executable. Default: `ofs`.

- `ofs.diagnosticsOnType`
: Run diagnostics while typing (debounced). Default: `true`.

- `ofs.autoInstallCompiler`
: Automatically installs OFS compiler on extension activation when missing. Default: `true`.

- `ofs.preferEmbeddedCompiler`
: Prefer compiler embedded in extension (`bin/<platform>/ofs`) before workspace/system compiler. Default: `true`.

### Commands

- `OFS: Check` (`ofs.checkFile`)
- `OFS: Emit Native Assembly` (`ofs.emitAssembly`)

The extension follows the current OFS direction: high-level and low-level OFS should coexist in the same codebase, while LLVM IR and target assembly remain inspection tools instead of the language's identity.

That includes:

- `bedrock { ... }` for typed low-level work,
- `rift vein` for OFS-native interop boundaries,
- `bind` and `abi` metadata for explicit external boundaries,
- `monolith ... layout native|packed|c` for ABI-facing layout intent,
- `fault_*` intrinsics for machine-like operations with OFS naming.

Run/Debug now uses native VS Code debug configurations (`ofs-native`) instead of custom extension buttons.

### Install From VSIX

```bash
code --install-extension ofs-vscode-extension.vsix
```

Or in VS Code: Extensions -> `...` -> `Install from VSIX...`

Important: do not use Visual Studio `VSIXInstaller.exe`; this package targets Visual Studio Code only.

### Build Extension Package Locally

```bash
cd vscode-extension
npm ci
npm run package
```

### Troubleshooting

- `ofs: command not found`
: Set `ofs.path` in settings or add OFS to PATH.

- No diagnostics shown
: Ensure `ofs.diagnosticsOnType` is enabled and the file is saved as `.ofs`.

- Attach/autocomplete for libraries not appearing
: Verify `attach` paths and `OFS_LIB_PATH` configuration.

### Repository Links

- Main repository: https://github.com/Samwns/Obsidian-Fault-Script
- Issues: https://github.com/Samwns/Obsidian-Fault-Script/issues

---

## Portugues (Brasil)

### O Que Esta Extensao Oferece

- Highlight de sintaxe para arquivos `.ofs`
- Snippets para acelerar a escrita
- Hover docs para palavras-chave e comandos OFS
- Hover docs para superficies low-level da OFS como `bedrock`, `rift` e `fault_*`
- Suporte de hover/autocomplete para metadados de interop e layout como `bind`, `abi` e `monolith ... layout packed`
- Autocomplete para simbolos da linguagem
- Autocomplete para bibliotecas anexadas via `attach` e `OFS_LIB_PATH`
- Diagnosticos usando `ofs check`
- Emissao de assembly nativo usando `ofs asm`
- Integracao nativa com Executar/Depurar do VS Code (F5 / painel Run and Debug)
- Acao na paleta de comandos: `OFS: Check`
- Acao na paleta de comandos: `OFS: Emit Native Assembly`
- Tema integrado: `Obsidian Fault Purple`
- Suporte a compilador embutido (`bin/<plataforma>/ofs`) com fallback de auto-instalacao

### Requisitos

- VS Code `^1.85.0`
- Compilador OFS no PATH ou configurado em `ofs.path`

### Configuracoes

- `ofs.path`
: Caminho para o executavel OFS. Padrao: `ofs`.

- `ofs.diagnosticsOnType`
: Executa diagnosticos enquanto digita (com debounce). Padrao: `true`.

- `ofs.autoInstallCompiler`
: Instala automaticamente o compilador OFS ao ativar a extensao quando ele nao estiver presente. Padrao: `true`.

- `ofs.preferEmbeddedCompiler`
: Prioriza o compilador embutido na extensao (`bin/<plataforma>/ofs`) antes do compilador do workspace/sistema. Padrao: `true`.

### Comandos

- `OFS: Check` (`ofs.checkFile`)
- `OFS: Emit Native Assembly` (`ofs.emitAssembly`)

A extensao segue a direcao atual da OFS: alto nivel e baixo nivel devem coexistir no mesmo codebase, enquanto LLVM IR e assembly nativo continuam como ferramentas de inspecao, nao como identidade da linguagem.

Isso inclui:

- `bedrock { ... }` para trabalho low-level tipado,
- `rift vein` para fronteiras de interoperabilidade nativas da OFS,
- metadados `bind` e `abi` para fronteiras externas explicitas,
- `monolith ... layout native|packed|c` para intencao de layout em tipos expostos por ABI,
- intrinsics `fault_*` para operacoes machine-like com nomes da propria linguagem.

Executar/Depurar agora usa configuracoes nativas de debug do VS Code (`ofs-native`) no lugar de botoes customizados da extensao.

### Instalar Via VSIX

```bash
code --install-extension ofs-vscode-extension.vsix
```

Ou no VS Code: Extensions -> `...` -> `Install from VSIX...`

Importante: nao use o `VSIXInstaller.exe` do Visual Studio; este pacote e apenas para Visual Studio Code.

### Gerar Pacote da Extensao Localmente

```bash
cd vscode-extension
npm ci
npm run package
```

### Solucao de Problemas

- `ofs: command not found`
: Configure `ofs.path` ou adicione OFS no PATH.

- Sem diagnosticos no editor
: Verifique se `ofs.diagnosticsOnType` esta ativo e se o arquivo e `.ofs`.

- Sem autocomplete para bibliotecas
: Confira caminhos de `attach` e a variavel `OFS_LIB_PATH`.

### Links

- Repositorio principal: https://github.com/Samwns/Obsidian-Fault-Script
- Issues: https://github.com/Samwns/Obsidian-Fault-Script/issues
