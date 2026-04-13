# Guia de Início do OFS — Versão Self-Hosted

> **NOVO**: O OFS agora compila ele mesmo! O compilador foi reescrito completamente em OFS puro.

---

## Modo Rápido

```bash
# 1. Compilar o compilador de OFS (usando C++)
ofs build ofs/ofscc/ofscc.ofs -o ofscc_v1

# 2. Compilar um programa com OFS
./ofscc_v1 ofs/examples/hello.ofs -o hello

# 3. Rodar
./hello
# Output: Hello, World!
```

**Acabou de usar um programa OFS para compilar outro programa OFS!** 🎉

---

## Instalação Completa

### 1. Instalar dependências

```bash
# Ubuntu / Debian
sudo apt update
sudo apt install cmake g++ llvm-17-dev clang-17 lld-17 git

# macOS
brew install cmake llvm@17 gcc git

# Windows
winget install Kitware.CMake LLVM.LLVM Microsoft.VisualStudio.Community Git.Git
```

### 2. Compilar a partir do código-fonte

```bash
# Clone
git clone https://github.com/Samwns/Obsidian-Fault-Script.git
cd Obsidian-Fault-Script/ofs

# Build com C++ (precisa fazer uma vez)
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build

# Ou usar ofs command se já instalado
ofs build src/main.cpp -o ofs  # (antigo, legado)
```

### 3. Compilar o compilador de OFS

```bash
# Copie o binário principal
./build/ofscc || ofs build ofs/ofscc/ofscc.ofs -o ofscc_v1

# Teste bootstrap
bash ofs/ofscc/test_bootstrap.sh

# Se passou: bootstrap completo ✓
# Agora você tem um compilador OFS que compila OFS
```

---

## 2. Primeiro Programa

### Com o novo compilador OFS

Crie `hello.ofs`:

```ofs
core main() {
    echo("Olá do OFS!")
}
```

Compile:

```bash
./ofscc_v1 hello.ofs -o hello
./hello
```

Output:
```
Olá do OFS!
```

### Com operações matemáticas

Crie `math.ofs`:

```ofs
vein factorial(n: stone) -> stone {
    if (n <= 1) { return 1 }
    return n * factorial(n - 1)
}

core main() {
    forge result = factorial(5)
    echo("5! = " + ofs_stone_to_obsidian(result))
}
```

Compile:

```bash
./ofscc_v1 math.ofs -o math
./math
```

### Exemplos prontos

```bash
# FizzBuzz
./ofscc_v1 ofs/examples/fizzbuzz.ofs -o fizzbuzz && ./fizzbuzz

# Recursão
./ofscc_v1 ofs/examples/recursion.ofs -o rec && ./rec

# Strings
./ofscc_v1 ofs/examples/string_ops.ofs -o str && ./str

# Arrays
./ofscc_v1 ofs/examples/collections.ofs -o arr && ./arr
```

---

## 3. Recursos da Linguagem

```ofs
core main() {
    echo("OFS funcionando")
}
```

Executar:

```bash
ofs hello.ofs
```

Ou, se estiver usando o binário compilado localmente:

```bash
./build/ofs hello.ofs
```

---

## 3. Comandos do dia a dia

```bash
ofs arquivo.ofs              # executa
ofs run arquivo.ofs          # compila e executa
ofs build arquivo.ofs -o app # gera executável
ofs check arquivo.ofs        # valida sem gerar saída
ofs tokens arquivo.ofs       # debug léxico
ofs ast arquivo.ofs          # debug sintático
ofs ir arquivo.ofs           # debug LLVM IR
ofs asm arquivo.ofs          # debug assembly nativo
ofs update                   # atualiza pela release
```

### Comandos do compilador self-hosted (ofscc)

Enquanto o bridge de argv está em transição, o `ofscc` usa variáveis de ambiente:

```bash
# Linux/macOS
export OFSCC_INPUT=ofs/examples/hello.ofs
export OFSCC_OUTPUT=hello
export OFSCC_OPT=-O3
./ofscc_v1
./hello
```

```powershell
# Windows PowerShell
$env:OFSCC_INPUT = "ofs/examples/hello.ofs"
$env:OFSCC_OUTPUT = "hello.exe"
$env:OFSCC_OPT = "-O2"
.\ofscc_v1.exe
.\hello.exe
```

Modos especiais:

- `OFSCC_MODE=check` (somente validação)
- `OFSCC_MODE=tokens` (imprime tokens)
- `OFSCC_MODE=ast` (imprime raiz AST)

---

## 4. Tipos básicos

```ofs
core main() {
    forge nome = "Ana"      // obsidian
    forge idade = 19         // stone
    forge altura = 1.70      // crystal
    forge ativo = true       // bool

    forge r: u8 = 255
    forge g: u8 = 128
    forge pixel: u32 = (r as u32 << 16) | (g as u32 << 8)

    echo(nome)
    echo(idade)
    echo(altura)
    echo(ativo)
    echo(pixel)
}
```

Tipos disponíveis no uso comum:

- `stone`, `crystal`, `obsidian`, `bool`, `void`
- `u8`, `u16`, `u32`, `u64`, `i8`, `i32`
- `Array<T>`
- `monolith` e tipos definidos pelo usuário
- tipos de função: `vein(stone) -> stone`

---

## 5. Funções e lambdas

```ofs
vein dobro(x: stone) -> stone {
    return x * 2
}

vein aplicar(x: stone, fn: vein(stone) -> stone) -> stone {
    return fn(x)
}

core main() {
    forge quadrado = vein(n: stone) -> stone {
        return n * n
    }

    echo(aplicar(10, dobro))
    echo(aplicar(10, quadrado))
}
```

---

## 6. `monolith`, `impl` e `namespace`

```ofs
monolith Rect {
    w: stone
    h: stone
}

impl Rect {
    vein area(self) -> stone {
        return self.w * self.h
    }
}

namespace mathx {
    vein square(x: stone) -> stone {
        return x * x
    }
}

core main() {
    forge r: Rect
    r.w = 10
    r.h = 20

    echo(r.area())
    echo(mathx.square(4))
}
```

---

## 7. Reuso com `attach`

### Anexar stdlib ou pacote instalado

```ofs
attach {core}
attach {terminal-colors}
```

### Anexar arquivo local

```ofs
attach {F:meu_modulo.ofs}
```

No Windows, também funciona com caminho absoluto:

```ofs
attach {F:G:\projeto\modulos\meu_modulo.ofs}
```

Exemplo real no repositório:

- `ofs/examples/attach_local_lib.ofs`
- `ofs/examples/attach_file_demo.ofs`

---

## 8. Pacotes

Buscar, instalar e atualizar:

```bash
uncover fmt
infuse fmt
reinfuse fmt
```

Depois, no código:

```ofs
attach {fmt}
```

Pacotes já publicados no repositório incluem, entre outros:

- `canvas`
- `core`
- `fmt`
- `io`
- `math`
- `string`
- `bedrock`
- `bedrock-packet`
- `terminal-colors`
- `memory-modes`
- `rift`
- `webserver`
- `window`

---

## 9. Interop nativa

Use `extern vein` ou `rift vein` para falar com código externo:

```ofs
rift vein ofs_window_create(title: obsidian, w: stone, h: stone) -> void abi c
```

Quando precisar controlar o nome de link:

```ofs
rift vein text_size(s: obsidian) -> stone bind "strlen" abi c
```

---

## 10. Fluxo e recursos modernos

### `const`

```ofs
const limite: stone = 10
```

### `strata`

```ofs
strata Status { Idle, Running, Failed }
```

### `match`

```ofs
match code {
    case 200: { echo("ok") }
    default: { echo("erro") }
}
```

### `tremor/catch/throw`

```ofs
tremor {
    throw "boom"
} catch (e: obsidian) {
    echo(e)
}
```

---

## 11. Blocos de baixo nível

OFS suporta código de mais baixo nível sem sair da linguagem.

- `fracture`: camada tipada de ponteiros
- `abyss`: camada crua e irrestrita
- `fractal`: ponte de efeito/execução especial
- `bedrock`: vocabulário OFS para estado e memória explícita

Exemplo:

```ofs
attach {bedrock}

core main() {
    forge cell = bedrock_cell_new(0)

    bedrock {
        bedrock_cell_add(cell, 10)
    }

    echo(bedrock_cell_read(cell))
    bedrock_cell_drop(cell)
}
```

---

## 12. Janela e input

OFS já tem ponte nativa para janela/input em `window.ofs`.

```ofs
attach {window}

core main() {
    window.create("Teste", 640, 480)
    while (window.poll()) {
        echo(input.mouse_x())
    }
    window.destroy()
}
```

Se SDL2 estiver disponível no build, a ponte nativa é ativada automaticamente.

### Canvas em OFS puro

Também existe `canvas`, com buffer de pixels e helpers prontos:

```ofs
attach {canvas}

core main() {
    forge cv = canvas.create(64, 48)
    canvas.clear(cv, 0x1A1A2E)
    canvas.set_pixel(cv, 10, 10, 0xFF0000)
    canvas.destroy(cv)
}
```

Exemplos no repositório:

- `ofs/examples/canvas_window_demo.ofs`
- `ofs/examples/canvas_package_demo.ofs`

---

## Próximos passos

1. Leia a [Referência da Linguagem](LANGUAGE_REFERENCE.md)
2. Rode os exemplos em `ofs/examples/`
3. Veja os pacotes em [packages/README.md](../packages/README.md)
4. Use a [Jornada Iniciante](../OFS_JORNADA_INICIANTE.md) se estiver começando

```ofs
core main() {
    cycle (forge i = 0; i < 5; i++) {
        echo(i)
    }
    // Output: 0 1 2 3 4
}
```

**Range-based loop:**

```ofs
core main() {
    forge fruits = ["apple", "banana", "cherry"]
    cycle (fruit in fruits) {
        echo(fruit)
    }
}
```

---

## 6. Structures (Monoliths)

Define data structures with `monolith`:

```ofs
monolith Player {
    name: obsidian
    hp:   stone
    speed: crystal
}

vein introduce(p: Player) -> void {
    echo(p.name)
    echo(p.hp)
}

core main() {
    forge hero: Player
    hero.name = "Obsidian Knight"
    hero.hp = 100
    hero.speed = 1.5

    introduce(hero)
}
```

---

## 7. Collections

Arrays are dynamic and type-safe:

```ofs
core main() {
    forge scores = [95, 87, 92, 78, 88]

    // Iterate
    cycle (s in scores) {
        echo(s)
    }

    // Access by index
    echo(scores[0])    // 95
    echo(scores[4])    // 88

    // Sum with a loop
    forge total: stone = 0
    cycle (forge i = 0; i < 5; i++) {
        total += scores[i]
    }
    echo(total)
}
```

---

## 8. Pointers (Safe and Unsafe)

### Safe Pointers (fracture)

```ofs
core main() {
    forge x: stone = 10
    echo(x)    // 10

    fracture {
        shard p: *stone = &x
        *p = 42
    }

    echo(x)    // 42
}
```

The `fracture` block ensures pointer safety with compile-time checks.

### Unsafe Memory (abyss)

For rare cases when you need raw memory access:

```ofs
abyss {
    // Unchecked memory operations
    // Use sparingly!
}
```

---

## 9. Classic Example: FizzBuzz

```ofs
core main() {
    cycle (forge i = 1; i <= 20; i++) {
        if (i % 15 == 0) {
            echo("FizzBuzz")
        } else if (i % 3 == 0) {
            echo("Fizz")
        } else if (i % 5 == 0) {
            echo("Buzz")
        } else {
            echo(i)
        }
    }
}
```

Save as `fizzbuzz.ofs` and run:

```bash
ofs fizzbuzz.ofs
```

---

## 10. CLI Commands

| Command                    | Description                        |
|----------------------------|------------------------------------|
| `ofs file.ofs`             | Run directly (like Python)         |
| `ofs run file.ofs`         | Compile and run                    |
| `ofs build file.ofs -o app`| Compile to native executable       |
| `ofs check file.ofs`       | Type-check only                    |
| `ofs tokens file.ofs`      | Show lexer tokens (debug)          |
| `ofs ast file.ofs`         | Show parse tree (debug)            |
| `ofs ir file.ofs`          | Show LLVM IR (debug)               |
| `ofs version`              | Show version                       |
| `ofs update`               | Download and install latest release |
| `ofs help`                 | Show help                          |

---

## What's Next?

- Run the complete feature showcase: `ofs examples/showcase.ofs`
- Read the **[Language Reference](LANGUAGE_REFERENCE.md)** for complete syntax documentation
- Check the **[examples/](../ofs/examples/)** directory for more programs
- Explore the **[standard library](../ofs/stdlib/)** for reusable functions
- Continue with the **[OFS Beginner Journey](../OFS_JORNADA_INICIANTE.md)** for the next study steps

### New in v1.0

- **`while` loops**: `while (cond) { }` for condition-only looping
- **Type casting**: `expr as type` for explicit conversions
- **`attach`**: Include other OFS modules
- **`extern`**: Declare C functions for low-level access
- **Expanded runtime**: String ops, math, I/O, type conversions
- **Standard library**: `core.ofs`, `math.ofs`, `string.ofs`, `io.ofs`

Happy coding with OFS! 🪨⚡

- [Bare-metal/Freestanding Profile (experimental)](ofs/docs/BAREMETAL_PROFILE.md)
