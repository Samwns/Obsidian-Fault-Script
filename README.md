# Obsidian Fault Script (OFS)

Language / Idioma:

- [English](#english)
- [Portugues (Brasil)](#portugues-brasil)

---

## English

A hybrid language (interpreted run flow + native compilation), statically typed, designed to help people learn programming with fast feedback.
Write simple code, run it with one command, and move into lower-level concepts when you are ready, without abandoning the language's own identity.

### OFS Direction

OFS is not trying to become C with different keywords or assembly with nicer syntax.

The direction of the language is:

- high-level and low-level OFS must coexist in the same file and program,
- low-level control must remain native to OFS concepts such as `fracture`, `abyss`, `fractal`, and `bedrock`,
- interop with C, C++, C#, Python, and assembly should exist as a capability layer, not as the conceptual foundation of the language,
- the source of truth stays in OFS, even when inspecting LLVM IR or target assembly.

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
ofs asm file.ofs                # show target-native assembly
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
ofs asm arquivo.ofs             # mostra assembly nativo do alvo
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
- [Roadmap de baixo nivel](docs/LOW_LEVEL_ROADMAP.md)
- [Jornada de aprendizado OFS](OFS_JORNADA_INICIANTE.md)
- [Historico de versoes](docs/CHANGELOG.md)
- [Bare-metal/Freestanding Profile (experimental)](ofs/docs/BAREMETAL_PROFILE.md)

---

## License / Licenca

MIT
