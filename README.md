# Obsidian Fault Script (OFS)

> Linguagem compilada, estaticamente tipada, que gera código de máquina nativo via LLVM.  
> Simplicidade de alto nível + controle de baixo nível, com palavras-chave temáticas de geologia.

---

## ⬇️ Download (sem precisar compilar)

Baixe o executável pronto para o seu sistema na [página de Releases](https://github.com/Samwns/Obsidian-Fault-Script/releases/latest):

| Sistema       | Arquivo                  |
|---------------|--------------------------|
| Linux (x64)   | `ofs-linux-x64`          |
| Windows (x64) | `ofs-windows-x64.exe`    |
| macOS (ARM)   | `ofs-macos-arm64`        |

```bash
# Linux / macOS — tornar executável e mover para o PATH
chmod +x ofs-linux-x64
sudo mv ofs-linux-x64 /usr/local/bin/ofs

# Windows — renomear e adicionar ao PATH manualmente
ren ofs-windows-x64.exe ofs.exe
```

---

## 🔄 Versionamento Automático

A versão segue o padrão **`1.0.x`** — todo push na `main` incrementa o patch automaticamente:

```
v1.0.0  →  v1.0.1  →  v1.0.2  ...
```

Cada [Release](https://github.com/Samwns/Obsidian-Fault-Script/releases) inclui:
- Binários compilados para Linux, Windows e macOS
- Changelog com todos os commits desde a versão anterior

Para ver o que mudou em cada versão, acesse a [página de Releases](https://github.com/Samwns/Obsidian-Fault-Script/releases).

Histórico detalhado de versões e mudanças: [docs/CHANGELOG.md](docs/CHANGELOG.md)

### Novidades (v1.0.x)

- Pattern matching: `match`, `case`, `default`
- Variáveis imutáveis: `const`
- Enumerações: `strata`
- Tratamento de erro: `tremor`, `catch`, `throw`
- Operadores novos: `<<`, `>>`, `%=` e bitwise (`&`, `|`, `^`)

---

## 🚀 Olá, Mundo!

Crie um arquivo `hello.ofs`:

```ofs
core main() {
    echo("Hello, World!")
}
```

Execute:

```bash
ofs hello.ofs
```

---

## 📖 Exemplos

### Alto nível — funções e structs

```ofs
monolith Player {
    name: obsidian
    hp:   stone
}

vein greet(p: Player) -> void {
    echo(p.name)
    echo(p.hp)
}

core main() {
    forge hero: Player
    hero.name = "Obsidian Knight"
    hero.hp   = 100
    greet(hero)
}
```

### Baixo nível — ponteiros e extern C

```ofs
// Chama função C diretamente
extern vein ofs_pow(base: crystal, exp: crystal) -> crystal

core main() {
    // Ponteiro seguro via bloco fracture
    forge x: stone = 10
    fracture {
        shard p: *stone = &x
        *p = 42
    }
    echo(x)   // 42

    // Chamada C: 2^10
    forge result = ofs_pow(2.0, 10.0)
    echo(result)   // 1024
}
```

### Alto nível — loop e coleções

```ofs
core main() {
    forge nums = [10, 20, 30, 40, 50]

    forge sum: stone = 0
    cycle (n in nums) {
        sum += n
    }
    echo(sum)   // 150
}
```

---

## 🔧 Compilar do Código-Fonte

> Só necessário se quiser contribuir ou não tiver release para o seu sistema.

**Dependências:**
- CMake 3.20+
- Compilador C++17 (GCC 9+, Clang 10+, MSVC 2019+)
- LLVM 17+

```bash
# Ubuntu / Debian
sudo apt install llvm-17-dev clang-17 cmake

# macOS
brew install llvm@17 cmake

# Windows
winget install LLVM.LLVM Kitware.CMake
```

```bash
git clone https://github.com/Samwns/Obsidian-Fault-Script.git
cd Obsidian-Fault-Script/ofs
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

Binário gerado em `build/ofs` (Linux/macOS) ou `build\Release\ofs.exe` (Windows).

---

## 🖥️ Comandos

```bash
ofs hello.ofs              # executa diretamente
ofs build hello.ofs -o hello  # compila para binário nativo
ofs check hello.ofs        # verifica tipos sem compilar
ofs ir    hello.ofs        # exibe o LLVM IR gerado
ofs ast   hello.ofs        # exibe a AST
ofs help                   # mostra ajuda
```

---

## 📖 Referência da Linguagem

### Types

| OFS Type    | Description           | LLVM Type |
|-------------|-----------------------|-----------|
| `stone`     | 64-bit integer        | `i64`     |
| `crystal`   | 64-bit float          | `f64`     |
| `obsidian`  | String                | `i8*`     |
| `bool`      | Boolean               | `i1`      |
| `void`      | No value              | `void`    |
| `shard <T>` | Pointer to type T     | `T*`      |

### Variables

```ofs
forge name = "Rex"              // type inferred as obsidian
forge pi: crystal = 3.14        // explicit type
forge count: stone = 0          // explicit integer
forge alive = true              // type inferred as bool
```

### Functions

```ofs
// Regular function
vein add(a: stone, b: stone) -> stone {
    return a + b
}

// Entry point (like main)
core main() {
    forge result = add(10, 20)
    echo(result)  // prints 30
}
```

### Control Flow

```ofs
// If / else
if (x > 10) {
    echo("big")
} else {
    echo("small")
}

// C-style loop
cycle (forge i = 0; i < 10; i++) {
    echo(i)
}

// While loop
while (count > 0) {
    echo(count)
    count -= 1
}

// Range-based loop
forge nums = [1, 2, 3, 4, 5]
cycle (n in nums) {
    echo(n)
}
```

### Type Casting

```ofs
forge x: stone = 42
forge y: crystal = x as crystal   // stone to crystal
forge z: stone = 3.14 as stone    // crystal to stone
forge b: stone = true as stone    // bool to stone
```

### Imports and Extern

```ofs
// Import other OFS modules
import "stdlib/core.ofs"

// Declare external C functions
extern vein ofs_pow(base: crystal, exp: crystal) -> crystal

core main() {
    forge result = ofs_pow(2.0, 10.0)
    echo(result)
}
```

### Structures (Monoliths)

```ofs
monolith Player {
    name: obsidian
    hp:   stone
    speed: crystal
}

core main() {
    forge hero: Player
    hero.name = "Obsidian Knight"
    hero.hp = 100
    hero.speed = 1.5
    echo(hero.name)
}
```

### Pointers (Shards)

```ofs
core main() {
    forge x: stone = 10

    // Safe pointer block
    fracture {
        shard p: *stone = &x
        *p = 42
    }

    echo(x)  // prints 42
}
```

### Collections

```ofs
core main() {
    forge nums = [10, 20, 30, 40, 50]

    cycle (i in nums) {
        echo(i)
    }

    forge sum: stone = 0
    cycle (forge i = 0; i < 5; i++) {
        sum += nums[i]
    }
    echo(sum)  // prints 150
}
```

---

## 🗺️ Keyword Reference

| Keyword      | Purpose                         | Example                        |
|--------------|----------------------------------|---------------------------------|
| `core`       | Entry point / public function   | `core main() { }`             |
| `vein`       | Function definition             | `vein add(a: stone) -> stone` |
| `forge`      | Variable declaration            | `forge x = 10`                |
| `monolith`   | Struct definition               | `monolith Player { }`         |
| `cycle`      | Loop (for / for-each)           | `cycle (i in list) { }`       |
| `fracture`   | Safe pointer block              | `fracture { }`                |
| `abyss`      | Unsafe memory block             | `abyss { }`                   |
| `shard`      | Pointer type                    | `shard p: *stone`             |
| `echo`       | Print (polymorphic)             | `echo("hello")`               |
| `while`      | Condition-only loop             | `while (x > 0) { }`          |
| `import`     | Import OFS module               | `import "stdlib/core.ofs"`   |
| `extern`     | Foreign function declaration    | `extern vein puts(s: obsidian) -> stone` |
| `as`         | Type cast                       | `x as crystal`               |
| `if` / `else`| Conditional                     | `if (x > 0) { }`             |
| `return`     | Return from function            | `return x + y`                |
| `break`      | Exit loop                       | `break`                       |
| `continue`   | Skip to next iteration          | `continue`                    |

---

## 📂 Project Structure

```
ofs/
├── src/
│   ├── main.cpp              ← CLI entry point
│   ├── lexer/                ← Tokenizer
│   ├── parser/               ← Pratt parser → AST
│   ├── ast/                  ← AST node definitions
│   ├── semantic/             ← Type checking & scope resolution
│   ├── codegen/              ← LLVM IR generation
│   └── runtime/              ← C runtime (echo, alloc, arrays)
├── stdlib/                   ← Standard library (OFS source)
├── tests/                    ← Unit tests
├── examples/                 ← Example programs
└── CMakeLists.txt            ← Build configuration
```

---

## 🧪 Running Tests

```bash
cd ofs/build
ctest --output-on-failure
```

---

## 📚 Documentation

- **[Language Reference](docs/LANGUAGE_REFERENCE.md)** — Complete syntax and semantics guide
- **[Getting Started](docs/GETTING_STARTED.md)** — Step-by-step tutorial
- **[Compiler Roadmap](OFS_COMPILER_ROADMAP.md)** — Technical build guide

---

## 🗓️ Roadmap

| Version | Features                                               | Status |
|---------|--------------------------------------------------------|--------|
| v0.1    | Lexer, Parser, AST                                    | ✅ Done |
| v0.2    | Semantic analysis + type inference                     | ✅ Done |
| v0.3    | LLVM codegen: literals, arithmetic, variables, if, cycle | ✅ Done |
| v0.4    | Functions (vein), return, recursion                    | ✅ Done |
| v0.5    | monolith (structs) + field access                      | ✅ Done |
| v0.6    | fracture (safe pointers) + shard type                  | ✅ Done |
| v0.7    | Dynamic arrays + range-based cycle                     | ✅ Done |
| v0.8    | abyss (unsafe memory), raw mem access                  | ✅ Done |
| v0.9    | String operations (concat, compare)                    | ✅ Done |
| v1.0    | Standard library, import, extern, while, type casts   | ✅ Done |

---

## 📄 License

MIT