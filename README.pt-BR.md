# Obsidian Fault Script (OFS)

Linguagem compilada, estaticamente tipada e feita para aprender programacao com feedback rapido.
Voce escreve codigo simples, roda com um comando e evolui para conceitos de baixo nivel quando quiser.

## Comece em 5 minutos

1. Baixe a versao mais recente em [Releases](https://github.com/Samwns/Obsidian-Fault-Script/releases/latest).
2. Instale o binario do seu sistema.
3. (Opcional) Instale a extensao do VS Code para ter highlight, diagnostico e botao de Run.

Arquivos de release:

| Plataforma | Arquivo |
|---|---|
| Linux (x64) | `ofs-linux-x64-installer.tar.gz` |
| Windows (x64) | `ofs-windows-x64-installer.exe` |
| macOS (ARM) | `ofs-macos-arm64-installer.pkg` |
| VS Code | `ofs-vscode-extension.vsix` |

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

No Windows, execute `ofs-windows-x64-installer.exe`.

## Seu primeiro programa

Crie `hello.ofs`:

```ofs
core main() {
    echo("Ola, OFS!")
}
```

Rode:

```bash
ofs hello.ofs
```

## Trilha para iniciantes (dinamica)

Escolha um nivel e avance no seu ritmo:

1. Nivel 1: Variaveis e tipos
2. Nivel 2: Decisoes e repeticao
3. Nivel 3: Funcoes e organizacao
4. Nivel 4: Memoria e performance

Nivel 1 (15 min):

```ofs
core main() {
    forge nome: obsidian = "Nina"
    forge idade: stone = 17
    forge altura: crystal = 1.68
    echo(nome)
    echo(idade)
    echo(altura)
}
```

Desafio rapido:
- Troque os valores e imprima uma frase com nome e idade.

Nivel 2 (20 min):

```ofs
core main() {
    forge nota: stone = 8

    if (nota >= 7) {
        echo("Aprovado")
    } else {
        echo("Revisar conteudo")
    }

    cycle (forge i = 1; i <= 3; i++) {
        echo(i)
    }
}
```

Desafio rapido:
- Mostre os numeros de 1 a 10 e a soma total.

Nivel 3 (20 min):

```ofs
vein soma(a: stone, b: stone) -> stone {
    return a + b
}

core main() {
    forge resultado = soma(12, 30)
    echo(resultado)
}
```

Desafio rapido:
- Crie uma funcao que receba um nome e retorne uma saudacao.

Nivel 4 (avancado, quando voce quiser):
- `fracture` e `shard` para ponteiros seguros
- `extern` para chamar funcoes C
- `attach` para reutilizar modulos

## Comandos essenciais

```bash
ofs arquivo.ofs                 # executa direto
ofs check arquivo.ofs           # valida tipos e sintaxe
ofs build arquivo.ofs -o app    # gera binario nativo
ofs ast arquivo.ofs             # mostra arvore sintatica
ofs ir arquivo.ofs              # mostra LLVM IR
ofs help                        # ajuda
```

## Exemplos prontos

Os exemplos estao em `ofs/examples/`:

- `hello.ofs`
- `fizzbuzz.ofs`
- `calculator.ofs`
- `showcase.ofs`
- `packages_demo.ofs`

Rodar showcase completo:

```bash
ofs ofs/examples/showcase.ofs
```

## Pacotes e bibliotecas

Bibliotecas de exemplo no projeto:

- `terminal-colors`
- `memory-modes`

Fluxo local:

```bash
./packages/build_packages.sh
./packages/install_library.sh packages/dist/terminal-colors.tar.gz
./packages/install_library.sh packages/dist/memory-modes.tar.gz
export OFS_LIB_PATH="$HOME/.ofs/libs"
ofs ofs/examples/packages_demo.ofs
```

## Documentacao

- [Guia de inicio](docs/GETTING_STARTED.md)
- [Referencia da linguagem](docs/LANGUAGE_REFERENCE.md)
- [Jornada de aprendizado OFS](OFS_JORNADA_INICIANTE.md)
- [Historico de versoes](docs/CHANGELOG.md)

## Para contribuir (build local)

Requisitos:

- CMake 3.20+
- compilador C++17
- LLVM 17+

```bash
git clone https://github.com/Samwns/Obsidian-Fault-Script.git
cd Obsidian-Fault-Script/ofs
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

## Licenca

MIT
