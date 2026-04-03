# Obsidian Fault Script (OFS)

Language / Idioma:

- [English](#english)
- [Portugues (Brasil)](#portugues-brasil)

---

## English

A hybrid language (interpreted run flow + native compilation), statically typed, designed to help people learn programming with fast feedback.
Write simple code, run it with one command, and move into lower-level concepts when you are ready.

### Start in 5 minutes

1. Download the latest version from [Releases](https://github.com/Samwns/Obsidian-Fault-Script/releases/latest).
2. Install the binary for your platform.
3. Optional: install the VS Code extension for highlight, diagnostics, and Run button support.

Release files:

| Platform | File |
|---|---|
| Linux (x64) | ofs-linux-x64-installer.tar.gz |
| Windows (x64) | ofs-windows-x64-installer.exe |
| macOS (ARM) | ofs-macos-arm64-installer.pkg |
| VS Code | ofs-vscode-extension.vsix |

Quick install:

```bash
# Linux
tar -xzf ofs-linux-x64-installer.tar.gz
chmod +x install.sh
./install.sh

# macOS
sudo installer -pkg ofs-macos-arm64-installer.pkg -target /

# VS Code
code --install-extension ofs-vscode-extension.vsix
```

On Windows, run ofs-windows-x64-installer.exe.

### Essential commands

```bash
ofs file.ofs                    # run directly
ofs check file.ofs              # validate types and syntax
ofs build file.ofs -o app       # build native binary
ofs ast file.ofs                # show syntax tree
ofs ir file.ofs                 # show LLVM IR
ofs update                      # update to the latest release
ofs help                        # help
```

### Updates

- To update OFS after installation, run `ofs update`.
- The command checks the latest GitHub release, downloads the right installer for your platform, and applies the update.
- To verify after update: `ofs version`.

### Documentation

- [Getting started](docs/GETTING_STARTED.md)
- [Language reference](docs/LANGUAGE_REFERENCE.md)
- [OFS beginner journey](OFS_JORNADA_INICIANTE.md)
- [Changelog](docs/CHANGELOG.md)

---

## Portugues (Brasil)

Linguagem hibrida (fluxo interpretado de execucao + compilacao nativa), estaticamente tipada e feita para aprender programacao com feedback rapido.
Voce escreve codigo simples, roda com um comando e evolui para conceitos de baixo nivel quando quiser.

### Comece em 5 minutos

1. Baixe a versao mais recente em [Releases](https://github.com/Samwns/Obsidian-Fault-Script/releases/latest).
2. Instale o binario do seu sistema.
3. Opcional: instale a extensao do VS Code para ter highlight, diagnostico e botao de Run.

Arquivos de release:

| Plataforma | Arquivo |
|---|---|
| Linux (x64) | ofs-linux-x64-installer.tar.gz |
| Windows (x64) | ofs-windows-x64-installer.exe |
| macOS (ARM) | ofs-macos-arm64-installer.pkg |
| VS Code | ofs-vscode-extension.vsix |

Instalacao rapida:

```bash
# Linux
tar -xzf ofs-linux-x64-installer.tar.gz
chmod +x install.sh
./install.sh

# macOS
sudo installer -pkg ofs-macos-arm64-installer.pkg -target /

# VS Code
code --install-extension ofs-vscode-extension.vsix
```

No Windows, execute ofs-windows-x64-installer.exe.

### Comandos essenciais

```bash
ofs arquivo.ofs                 # executa direto
ofs check arquivo.ofs           # valida tipos e sintaxe
ofs build arquivo.ofs -o app    # gera binario nativo
ofs ast arquivo.ofs             # mostra arvore sintatica
ofs ir arquivo.ofs              # mostra LLVM IR
ofs update                      # atualiza para a release mais recente
ofs help                        # ajuda
```

### Atualizacoes

- Para atualizar o OFS depois de instalado, rode `ofs update`.
- O comando verifica a release mais recente no GitHub, baixa o instalador certo da sua plataforma e aplica a atualizacao.
- Para confirmar depois: `ofs version`.

### Documentacao

- [Guia de inicio](docs/GETTING_STARTED.md)
- [Referencia da linguagem](docs/LANGUAGE_REFERENCE.md)
- [Jornada de aprendizado OFS](OFS_JORNADA_INICIANTE.md)
- [Historico de versoes](docs/CHANGELOG.md)

---

## License / Licenca

MIT
