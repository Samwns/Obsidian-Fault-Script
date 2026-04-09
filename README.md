# Obsidian Fault Script (OFS)

Linguagem para aprender programação. Escreva código, execute na hora, compile depois.

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
ofs update                # update to latest
ofs help                  # help
```

### Documentation

- [Getting started](docs/GETTING_STARTED.md)
- [Language reference](docs/LANGUAGE_REFERENCE.md)
- [Beginner guide](OFS_JORNADA_INICIANTE.md)
- [Packages](packages/README.md)
- [Changelog](docs/CHANGELOG.md)

---

## Português (Brasil)

Linguagem para aprender programação. Escreva código, execute na hora, compile depois.

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
ofs update                # atualiza
ofs help                  # ajuda
```

### Documentação

- [Guia de Início](docs/GETTING_STARTED.md)
- [Referência da Linguagem](docs/LANGUAGE_REFERENCE.md)
- [Guia do Iniciante](OFS_JORNADA_INICIANTE.md)
- [Pacotes](packages/README.md)
- [Changelog](docs/CHANGELOG.md)