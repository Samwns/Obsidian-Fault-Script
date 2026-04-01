# Obsidian Fault Script (OFS)

> A compiled, statically-typed language that generates native machine code via LLVM.  
> High-level simplicity + low-level control, with geology-themed keywords.

---

## ⬇️ Download (installers)

Download the pre-built binary for your platform from the [Releases page](https://github.com/Samwns/Obsidian-Fault-Script/releases/latest):

| Platform      | Installer file                      |
|---------------|-------------------------------------|
| Linux (x64)   | `ofs-linux-x64-installer.tar.gz`    |
| Windows (x64) | `ofs-windows-x64-installer.exe`     |
| macOS (ARM)   | `ofs-macos-arm64-installer.pkg`     |
| VS Code       | `ofs-vscode-extension.vsix`         |

```bash
# Linux
tar -xzf ofs-linux-x64-installer.tar.gz
chmod +x install.sh
./install.sh

# macOS
sudo installer -pkg ofs-macos-arm64-installer.pkg -target /

# VS Code extension (all platforms)
code --install-extension ofs-vscode-extension.vsix
```

Alternative install path in VS Code:
1. Open VS Code
2. Open Extensions panel
3. Click the `...` menu
4. Choose `Install from VSIX...`
5. Select `ofs-vscode-extension.vsix`

Important: do not install `ofs-vscode-extension.vsix` using `VSIXInstaller.exe` from Visual Studio. That installer is for Visual Studio IDE extensions, not VS Code extensions.

Windows: run `ofs-windows-x64-installer.exe` and complete the installer wizard.

## VS Code Extension

The release also includes `ofs-vscode-extension.vsix`.

Features:
- Syntax highlighting for `.ofs`
- Autocomplete for language keywords
- Snippets (main, function, if, attach)
- Diagnostics from `ofs check` (on save and while typing)
- Commands to run/check the current file
- Included purple theme: `Obsidian Fault Purple`

---

## 🔄 Automatic Versioning

Versions follow the **`1.0.x`** scheme — every push to `main` auto-increments the patch:

```
v1.0.0  →  v1.0.1  →  v1.0.2  ...
```

Each [Release](https://github.com/Samwns/Obsidian-Fault-Script/releases) includes:
- Pre-built binaries for Linux, Windows, and macOS
- Changelog with all commits since the previous release

Full version history: [docs/CHANGELOG.md](docs/CHANGELOG.md)

### What's new (v1.0.x)

- Pattern matching: `match`, `case`, `default`
- Immutable variables: `const`
- Enumerations: `strata`
- Error handling: `tremor`, `catch`, `throw`
- New operators: `<<`, `>>`, `%=`, and bitwise (`&`, `|`, `^`)

---

## 🚀 Hello, World!

Create a file `hello.ofs`:

```ofs
core main() {
    echo("Hello, World!")
}
```

Run it:

```bash
ofs hello.ofs
```

---

## 📖 Examples

### High-level — functions and structs

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

### Low-level — pointers and extern C

```ofs
// Call a C function directly
extern vein ofs_pow(base: crystal, exp: crystal) -> crystal

core main() {
    // Safe pointer via fracture block
    forge x: stone = 10
    fracture {
        shard p: *stone = &x
        *p = 42
    }
    echo(x)   // 42

    // C call: 2^10
    forge result = ofs_pow(2.0, 10.0)
    echo(result)   // 1024
}
```

### High-level — loops and collections

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

## 📦 Libraries Via Packages

You can test package unpack/install flow with libraries included in this repo:

- `terminal-colors`
- `memory-modes`

Build and install:

```bash
./packages/build_packages.sh
./packages/install_library.sh packages/dist/terminal-colors.tar.gz
./packages/install_library.sh packages/dist/memory-modes.tar.gz
export OFS_LIB_PATH="$HOME/.ofs/libs"
```

Run package demo:

```bash
ofs ofs/examples/packages_demo.ofs
```

---

## 🔧 Build from Source

> Only needed if you want to contribute or there is no release binary for your platform.

**Requirements:**
- CMake 3.20+
- C++17 compiler (GCC 9+, Clang 10+, MSVC 2019+)
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

Binary output: `build/ofs` (Linux/macOS) or `build\Release\ofs.exe` (Windows).

---

## 🖥️ Commands

```bash
ofs hello.ofs                  # run directly (like python)
ofs build hello.ofs -o hello   # compile to native binary
ofs check hello.ofs            # type-check only, no output
ofs ir    hello.ofs            # print generated LLVM IR
ofs ast   hello.ofs            # print the AST
ofs help                       # show help
```

---

## 📖 Language Reference

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
// Attach other OFS modules
attach "stdlib/core.ofs"

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
| `attach`     | Attach OFS module               | `attach "stdlib/core.ofs"`   |
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
| v1.0    | Standard library, attach, extern, while, type casts   | ✅ Done |

---

## 📄 License

MIT
