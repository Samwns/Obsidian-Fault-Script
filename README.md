# Obsidian Fault Script (OFS) 🔥

**Linguagem compilada, self-hosted e determinística**

Linguagem para aprender programação e, ao mesmo tempo, chegar perto do sistema quando precisar.

Você pode:
- escrever um arquivo `.ofs` e rodar direto;
- validar sem gerar binário;
- compilar para executável nativo;
- reutilizar módulos com `attach`;
- integrar com código externo via `rift vein`;
- usar recursos modernos da linguagem, como `impl`, `namespace`, lambdas e tipos inteiros pequenos;

[English](#english) | [Português (Brasil)](#português-brasil)

---

## English

- Function values and inline lambdas
- Low-level blocks: `fracture`, `abyss`, `fractal`, `bedrock`
- Interop with native code via `extern vein` and `rift vein`
- Standard modules and installable packages

### Install

Download the latest release from [Releases](https://github.com/Samwns/Obsidian-Fault-Script/releases/latest):

| Platform | How to install |
|---|---
| Linux (OFS Wizard) | `ofs run packaging/installer_wizard_linux.ofs` |
| Linux (Archive) | `tar xzf ofs-linux-x64-*.tar.gz && sudo mv ofs/bin/ofscc /usr/local/bin/ofs` |
| Windows (Native Wizard) | Run `dist/installer_windows.exe` (native Win32 or via Wine) |
| VS Code | Install `ofs-vscode-extension.vsix` (supports `.ofs`, `.oll`, `.odl`, `.oes`) |
| From Source | `bash ofscc/scripts/bootstrap-minimal.sh` |

### Main commands

```bash
ofs build file.ofs -o program               # compile to native executable (zero Clang)
ofs build file.ofs --target windows -o app  # cross-compile to Windows PE (.exe)
ofs run file.ofs                           # compile and run immediately
ofs check file.ofs                         # type-check and semantic diagnostics
ofs ui file.oll -o preview.ppm             # parse and render native OLL UI layout
ofs clean                                  # clean temporary build files
ofs tokens file.ofs                        # lexer output
ofs ast file.ofs                           # parser output
ofs ir file.ofs                            # LLVM IR output
ofs asm file.ofs                           # native assembly output
ofs version                                # compiler version
ofs help                                   # full help
```

### Example

```ofs
attach {core}

vein double(x: stone) -> stone {
	return x * 2
}

core main() {
	echo(double(21))
}
```

### Documentation & Official Site

- [Official Website](docs/index.html)
- [Getting started](docs/GETTING_STARTED.md)
- [Language reference](docs/LANGUAGE_REFERENCE.md)
- [OLL Visual Guide](docs/oll.html)
- [Beginner guide](OFS_JORNADA_INICIANTE.md)
- [Packages](packages/README.md)
- [Changelog](docs/CHANGELOG.md)

---

## Português (Brasil)

OFS é uma linguagem estaticamente tipada feita para ser simples no começo e poderosa quando o projeto crescer, agora com **compilação nativa sem dependência do Clang** e runtime **Stack Magma 100% puro em OFS**.

### A Família de Linguagens OFS

- `.ofs` — Lógica de programação, tipos, algoritmos, processos e baixo nível.
- `.oll` — Obsidian Layout Language: interface declarativa e layouts de aplicações nativas.
- `.odl` — Obsidian Document Language: estrutura de documentos e conteúdo unificado.
- `.oes` — Obsidian Element Styling: tokens, estilo, motion e visual Web.

### O que a linguagem já faz hoje

- Executa e compila direto via LLVM sem depender do Clang (`llc` + `ld` / `ld.lld`)
- Possui runtime nativo próprio em OFS puro (**Stack Magma**: `magma`, `reservoir`, `facet`, `matrix`, `outlet`, `foundation`, `pulse`, `frame`, `impulse`, `prism`)
- Suporta compilação cruzada para Windows PE (`--target windows`), executável diretamente no Wine
- Inclui instaladores visuais escritos na própria linguagem OFS com interfaces em OLL
- Renderiza e calcula layouts de janelas e componentes nativos com `ofs ui arquivo.oll`
- Extensão atualizada para VS Code com gramática de `.ofs`, `.oll`, `.odl` e `.oes`
- Diagnósticos de compilação claros e explicativos de sintaxe e tipos
- Limpa artefatos temporários de compilação com `ofs clean`

### Instalar

| Plataforma | Como instalar |
|---|---
| Linux (Assistente OFS) | `ofs run packaging/installer_wizard_linux.ofs` |
| Linux (Pacote manual) | `tar xzf ofs-linux-x64-*.tar.gz && sudo mv ofs/bin/ofscc /usr/local/bin/ofs` |
| Windows (Assistente Visual) | Execute `dist/installer_windows.exe` (nativo ou via Wine) |
| VS Code | Instale `ofs-vscode-extension.vsix` |
| Do Fonte | `bash ofscc/scripts/bootstrap-minimal.sh` |

### Comandos principais

```bash
ofs arquivo.ofs                       # executa direto
ofs run arquivo.ofs                   # compila e executa na hora
ofs build arquivo.ofs -o app          # gera executável nativo Linux
ofs build arquivo.ofs --target windows -o app.exe # gera executável nativo Windows
ofs ui layout.oll -o preview.ppm      # calcula e renderiza interface OLL
ofs check arquivo.ofs                 # valida tipos e sintaxe com erros claros
ofs version                           # exibe versão do compilador
```

### Documentação

- [Site Oficial](docs/index.html)
- [Guia de Início](docs/GETTING_STARTED.md)
- [Referência da Linguagem](docs/LANGUAGE_REFERENCE.md)
- [Guia OLL](docs/oll.html)
- [Jornada Iniciante](OFS_JORNADA_INICIANTE.md)
- [Changelog](docs/CHANGELOG.md)