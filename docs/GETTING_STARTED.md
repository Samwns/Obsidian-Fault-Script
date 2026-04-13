# Guia de Início do OFS — Versão Nativa (C++-Free)

> **NOVO**: O OFS agora é completamente self-hosted com compilador nativo!
> Sem dependências de C++, CMake ou LLVM. Releases gerados automaticamente em ~5 segundos.

---

## Modo Rápido (2 passos)

```bash
# 1. Fazer bootstrap do compilador OFS nativo  
bash ofscc/scripts/bootstrap-minimal.sh

# 2. Compilar um programa
dist/ofscc build ofs/examples/hello.ofs -o hello
./hello
# Output: Hello, World!
```

**Tempo total: ~5 segundos!**

---

## Instalação Completa (Sem Dependências!)

### 1. Clone do repositório

```bash
git clone https://github.com/Samwns/Obsidian-Fault-Script.git
cd Obsidian-Fault-Script
```

### 2. Bootstrap do compilador nativo

```bash
# Windows, Linux ou macOS - mesmo comando!
bash ofscc/scripts/bootstrap-minimal.sh
```

Isso cria `dist/ofscc` em apenas ~2 segundos!

### 3. Usar o compilador nativo

```bash
# Compilar arquivo OFS para executável nativo
dist/ofscc build seu_programa.ofs -o seu_programa

# Type-check sem gerar binário
dist/ofscc check seu_programa.ofs

# Inspecionar código
dist/ofscc tokens seu_programa.ofs      # Tokens do lexer
dist/ofscc ast seu_programa.ofs         # Abstract syntax tree
dist/ofscc ir seu_programa.ofs          # LLVM IR
dist/ofscc asm seu_programa.ofs         # Assembly nativo
```

### 4. (Opcional) Usar com Make

```bash
# Build, test, create release tudo com Make
make bootstrap       # Build tudo
make compile FILE=seu_programa.ofs
make test           # Rodar testes
make release        # Criar pacote de release
```

Ver todas as opções com `make help`

---

## 2. Primeiro Programa

### Crie e compile um programa simples

Crie `hello.ofs`:

```ofs
core main() {
    echo("Olá do OFS!")
}
```

Compile:

```bash
dist/ofscc build hello.ofs -o hello
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
dist/ofscc build math.ofs -o math
./math
```

### Exemplos prontos

```bash
# FizzBuzz
dist/ofscc build ofs/examples/fizzbuzz.ofs -o fizzbuzz && ./fizzbuzz

# Recursão
dist/ofscc build ofs/examples/recursion.ofs -o rec && ./rec

# Strings
dist/ofscc build ofs/examples/string_ops.ofs -o str && ./str

# Arrays
dist/ofscc build ofs/examples/collections.ofs -o arr && ./arr
```

## 3. Comandos Principais

O novo compilador nativo `dist/ofscc` oferece:

```bash
# Compilar para executável
dist/ofscc build programa.ofs -o programa

# Validar sem gerar saída
dist/ofscc check programa.ofs

# Debug: inspecionar código
dist/ofscc tokens programa.ofs  # Análise léxica
dist/ofscc ast programa.ofs     # Sintaxe
dist/ofscc ir programa.ofs      # LLVM IR
dist/ofscc asm programa.ofs     # Assembly nativo
```

### Exemplos de uso

```bash
# Programa simples
dist/ofscc build hello.ofs -o hello && ./hello

# Com otimização
dist/ofscc build programa.ofs -o programa -O3 && ./programa

# Type-check de biblioteca (sem executar)
dist/ofscc check meu_codigo.ofs
```

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
