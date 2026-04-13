# Obsidian Fault Script (OFS)

Linguagem para aprender programação e, ao mesmo tempo, chegar perto do sistema quando precisar.

Você pode:
- escrever um arquivo `.ofs` e rodar direto;
- validar sem gerar binário;
- compilar para executável nativo;
- reutilizar módulos com `attach`;
- integrar com código externo via `rift vein`;
- usar recursos modernos da linguagem, como `impl`, `namespace`, lambdas e tipos inteiros pequenos.

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
|---|---|
| Linux | Extract `ofs-linux-x64-installer.tar.gz` and run `./install.sh` |
| macOS | Run `sudo installer -pkg ofs-macos-arm64-installer.pkg -target /` |
| Windows | Run `ofs-windows-x64-installer.exe` |
| VS Code | Install `ofs-vscode-extension.vsix` |

### Main commands

```bash
ofs tokens file.ofs          # lexer output
ofs ast file.ofs             # parser output
ofs ir file.ofs              # LLVM IR output
ofs asm file.ofs             # native assembly output
ofs update                   # update to latest release
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

### Documentation

- [Getting started](docs/GETTING_STARTED.md)
- [Language reference](docs/LANGUAGE_REFERENCE.md)
- [Beginner guide](OFS_JORNADA_INICIANTE.md)
- [Packages](packages/README.md)
- [Changelog](docs/CHANGELOG.md)

---

## Português (Brasil)

OFS é uma linguagem estaticamente tipada feita para ser simples no começo e poderosa quando o projeto crescer.

### O que a linguagem já faz hoje

- Executa arquivos `.ofs` direto com `ofs arquivo.ofs`
- Compila e roda com `ofs run`
- Gera executável nativo com `ofs build`
- Valida sem gerar saída com `ofs check`
- Mostra tokens, AST, LLVM IR e assembly para debug
- Tem tipos pequenos: `u8`, `u16`, `u32`, `u64`, `i8`, `i32`
- Tem `monolith` com métodos via `impl`
- Tem `namespace` para organizar símbolos
- Aceita funções como valores e lambdas inline
- Tem `match`, `tremor/catch/throw`, `const` e `strata`
- Tem blocos de baixo nível: `fracture`, `abyss`, `fractal`, `bedrock`
- Faz integração nativa com `extern vein` e `rift vein`
- Reutiliza módulos por `attach {nome}` ou `attach {F:arquivo.ofs}`
- Já tem `window` e `canvas` para fluxo gráfico básico em OFS

### Instalar

Baixe a release mais recente em [Releases](https://github.com/Samwns/Obsidian-Fault-Script/releases/latest):

| Plataforma | Como instalar |
|---|---|
| Linux | Extraia `ofs-linux-x64-installer.tar.gz` e rode `./install.sh` |
| macOS | Rode `sudo installer -pkg ofs-macos-arm64-installer.pkg -target /` |
| Windows | Execute `ofs-windows-x64-installer.exe` |
| VS Code | Instale `ofs-vscode-extension.vsix` |

### Comandos principais

```bash
ofs arquivo.ofs              # executa direto
ofs run arquivo.ofs          # compila e executa na hora
ofs build arquivo.ofs -o app # gera executável nativo
ofs check arquivo.ofs        # valida tipos e sintaxe
ofs tokens arquivo.ofs       # mostra tokens
ofs ast arquivo.ofs          # mostra a AST
ofs ir arquivo.ofs           # mostra LLVM IR
ofs asm arquivo.ofs          # mostra assembly nativo
ofs update                   # atualiza pela release mais recente
```

### Exemplo rápido

```ofs
attach {core}

vein dobro(x: stone) -> stone {
	return x * 2
}

core main() {
	echo(dobro(21))
}
```

### Documentação

- [Guia de Início](docs/GETTING_STARTED.md)
- [Referência da Linguagem](docs/LANGUAGE_REFERENCE.md)
- [Jornada Iniciante](OFS_JORNADA_INICIANTE.md)
- [Pacotes](packages/README.md)
- [Changelog](docs/CHANGELOG.md)