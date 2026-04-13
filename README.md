# Obsidian Fault Script (OFS) 🔥

**Linguagem compilada, self-hosted e 100% determinística**

Linguagem para aprender programação e, ao mesmo tempo, chegar perto do sistema quando precisar.

✨ **Agora com compilador nativo (C++-free) e releases automáticas em ~5 segundos!**

Você pode:
- escrever um arquivo `.ofs` e rodar direto;
- validar sem gerar binário;
- compilar para executável nativo;
- reutilizar módulos com `attach`;
- integrar com código externo via `rift vein`;
- usar recursos modernos da linguagem, como `impl`, `namespace`, lambdas e tipos inteiros pequenos;
- compilar com o compilador nativo (sem dependências de C++)

[English](#english) | [Português (Brasil)](#português-brasil)

---

## English

- Function values and inline lambdas
- Low-level blocks: `fracture`, `abyss`, `fractal`, `bedrock`
- Interop with native code via `extern vein` and `rift vein`
- Standard modules and installable packages

### Install

**Built automatically** - Releases created in ~5 seconds with zero C++ dependencies!

Download the latest **native release** from [Releases](https://github.com/Samwns/Obsidian-Fault-Script/releases/latest):

| Platform | How to install |
|---|---
| Linux | `tar xzf ofs-linux-x64-*-native.tar.gz && cd ofs-* && ./bin/ofscc --help` |
| macOS | `tar xzf ofs-macos-arm64-*-native.tar.gz && cd ofs-* && ./bin/ofscc --help` |
| Windows | Extract `.zip` and run `bin\ofscc.exe --help` |
| VS Code | Install `ofs-vscode-extension.vsix` |
| From Source | `bash ofscc/scripts/bootstrap-minimal.sh` (2 sec) |

### Main commands

```bash
ofscc build file.ofs -o program    # compile to executable  
ofscc check file.ofs               # type-check (no codegen)
ofscc tokens file.ofs              # lexer output
ofscc ast file.ofs                 # parser output
ofscc ir file.ofs                  # LLVM IR output
ofscc asm file.ofs                 # native assembly output
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

**Criada automaticamente** - Releases gerados em ~5 segundos sem dependências de C++!

Baixe a **release nativa** mais recente em [Releases](https://github.com/Samwns/Obsidian-Fault-Script/releases/latest):

| Plataforma | Como instalar |
|---|---
| Linux | `tar xzf ofs-linux-x64-*-native.tar.gz && cd ofs-* && ./bin/ofscc --help` |
| macOS | `tar xzf ofs-macos-arm64-*-native.tar.gz && cd ofs-* && ./bin/ofscc --help` |
| Windows | Extraia `.zip` e execute `bin\ofscc.exe --help` |
| VS Code | Instale `ofs-vscode-extension.vsix` |
| Do Fonte | `bash ofscc/scripts/bootstrap-minimal.sh` (2 seg) |

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