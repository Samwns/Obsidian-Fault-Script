# Obsidian Fault Script (OFS)

Linguagem híbrida: execute código simples diretamente e evolua para conceitos de baixo nível quando quiser.

[English](#english) | [Português (Brasil)](#português-brasil)

---

## English

A hybrid language (interpreted + compiled), statically typed. Write code, run instantly, then optimize to native binary when needed.

### Install

Download from [Releases](https://github.com/Samwns/Obsidian-Fault-Script/releases/latest):

| Platform | Command |
|---|---|
| Linux | `tar -xzf ofs-linux-x64-installer.tar.gz && ./install.sh` |
| macOS | `sudo installer -pkg ofs-macos-arm64-installer.pkg -target /` |
| Windows | Run `ofs-windows-x64-installer.exe` |
| VS Code | `code --install-extension ofs-vscode-extension.vsix` |

### Commands

```bash
ofs file.ofs              # run directly
ofs check file.ofs        # validate syntax
ofs build file.ofs -o app # compile to binary
ofs ast file.ofs          # syntax tree
ofs ir file.ofs           # LLVM IR
ofs asm file.ofs          # assembly
ofs update                # update to latest
ofs help                  # help
```

### Documentation

- [Getting started](docs/GETTING_STARTED.md)
- [Language reference](docs/LANGUAGE_REFERENCE.md)
- [Low-level roadmap](docs/LOW_LEVEL_ROADMAP.md)
- [OFS beginner journey](OFS_JORNADA_INICIANTE.md)
- [Changelog](docs/CHANGELOG.md)
- [Bare-metal/Freestanding Profile (experimental)](ofs/docs/BAREMETAL_PROFILE.md)

---

## Portugues (Brasil)

Linguagem hibrida (fluxo interpretado de execucao + compilacao nativa), estaticamente tipada e feita para aprender programacao com feedback rapido.
Voce escreve codigo simples, roda com um comando e evolui para conceitos de baixo nivel quando quiser, sem abandonar a identidade da propria linguagem.

### Direcao da OFS

A OFS nao quer virar C com palavras diferentes nem assembly com roupa bonita.

A direcao da linguagem e:

- alto nivel e baixo nivel coexistem no mesmo arquivo e no mesmo programa,
- o controle de baixo nivel continua nativo a conceitos da OFS como `fracture`, `abyss`, `fractal` e `bedrock`,
- interoperabilidade com C, C++, C#, Python e assembly existe como camada de capacidade, nao como base conceitual da linguagem,
- a fonte da verdade continua sendo OFS, mesmo quando voce inspeciona LLVM IR ou assembly nativo.

### Coms

- [Getting Started](docs/GETTING_STARTED.md)
- [Language Reference](docs/LANGUAGE_REFERENCE.md)
- [Beginner Guide](OFS_JORNADA_INICIANTE.md)
- [Packages](packages/README.md)
- [Changelog](docs/CHANGELOG.md)

---

## Português (Brasil)

Linguagem híbrida: execute código simples direto e evolua para conceitos de baixo nível quando precisar.

### Instalar

Baixe em [Releases](https://github.com/Samwns/Obsidian-Fault-Script/releases/latest):

| Plataforma | Comando |
|---|---|
| Linux | `tar -xzf ofs-linux-x64-installer.tar.gz && ./install.sh` |
| macOS | `sudo installer -pkg ofs-macos-arm64-installer.pkg -target /` |
| Windows | Execute `ofs-windows-x64-installer.exe` |
| VS Code | `code --install-extension ofs-vscode-extension.vsix` |

### Comandos

```bash
ofs arquivo.ofs           # executa direto
ofs check arquivo.ofs     # valida sintaxe
ofs build arquivo.ofs     # compila para binário
ofs ast arquivo.ofs       # árvore sintática
ofs ir arquivo.ofs        # LLVM IR
ofs asm arquivo.ofs       # assembly
ofs update                # atualiza
ofs help                  # ajuda
```

### Docs

- [Guia de Início](docs/GETTING_STARTED.md)
- [Referência da Linguagem](docs/LANGUAGE_REFERENCE.md)
- [Guia do Iniciante](OFS_JORNADA_INICIANTE.md)
- [Pacotes](packages/README.md)
- [Changelog](docs/CHANGELOG.md)