# Getting Started with OFS

OFS (Obsidian Fault Script) is a hybrid, self-hosted programming language featuring an integrated AST interpreter for instant execution (`ofs run`) and an LLVM backend for high-performance native binary compilation (`ofs build`).

The ecosystem includes the compiler, a standard library, command-line tools, and domain-specific web languages (ODL and OES).

---

## 1. Installation

### Pre-compiled Packages

Official packages and installers are available across major platforms:

- **Debian / Ubuntu**:
  ```bash
  sudo dpkg -i ofs-debian-ubuntu-x64-installer-v*.deb
  ```
- **Fedora / RHEL / openSUSE**:
  ```bash
  sudo rpm -Uvh ofs-fedora-x64-installer-v*.rpm
  ```
- **Arch / Manjaro**:
  ```bash
  sudo pacman -U ofs-arch-x64-installer-v*.pkg.tar.zst
  ```
- **macOS (Apple Silicon / ARM64)**:
  Run the installer script provided in the GitHub release assets.
- **Windows (x64)**:
  Run the official executable installer: `ofs-windows-x64-installer-v*.exe`.
- **VS Code**:
  Install the official extension `ofs-vscode-extension.vsix` for syntax highlighting, diagnostics, and automatic live reload (*Go Live*).

### Building from Source (Bootstrap)

To build the compiler from source within the repository:

```bash
git clone https://github.com/Samwns/Obsidian-Fault-Script.git
cd Obsidian-Fault-Script
bash ofs/bootstrap/scripts/bootstrap-minimal.sh
```

The resulting compiler binary is written to `ofs/dist/ofs` (or `ofs/dist/ofscc`).

---

## 2. Command-Line Interface (CLI)

The `ofs` CLI provides the following subcommands:

| Command | Description |
|---|---|
| `ofs <file.ofs>` | Executes the program directly using the integrated AST interpreter. |
| `ofs run <file.ofs> [--native]` | Executes via the AST interpreter (or with `--native` for temporary compilation). |
| `ofs build <file.ofs> -o <binary>` | Compiles an OFS source file into a native binary executable via LLVM. |
| `ofs check <file.ofs>` | Type-checks and validates source code without emitting output. |
| `ofs tokens <file.ofs>` | Prints tokens produced by the lexical analyzer. |
| `ofs ast <file.ofs>` | Displays the Abstract Syntax Tree (AST). |
| `ofs ir <file.ofs>` | Emits native LLVM IR code (`.ll`). |
| `ofs asm <file.ofs>` | Emits assembly code for the target architecture. |
| `ofs odl <file.odl> -o <output.html>` | Compiles an ODL document into standard HTML5. |
| `ofs oes <file.oes> -o <output.css>` | Compiles OES styling rules into standard CSS. |

---

## 3. First Program

Create a file named `hello.ofs`:

```ofs
core main() {
    echo("Hello from OFS!")
}
```

Direct execution via the AST interpreter:

```bash
ofs hello.ofs
# or: ofs run hello.ofs
```

Or compiling to a standalone native binary:

```bash
ofs build hello.ofs -o hello
./hello
```

Output:
```text
Hello from OFS!
```

---

## 4. Data Types

OFS is statically typed. The compiler infers variable types when initialized with `forge`.

### Primitive Types

- `stone`: 64-bit signed integer.
- `crystal`: 64-bit floating-point number (IEEE 754).
- `obsidian`: UTF-8 encoded string pointer.
- `bool`: Boolean (`true` or `false`).
- `void`: Indicates absence of a return value.

### Fixed-Width Integer Types

- Unsigned: `u8`, `u16`, `u32`, `u64`
- Signed: `i8`, `i32`

Example:

```ofs
core main() {
    forge counter: stone = 10
    forge rate: crystal = 3.14159
    forge active: bool = true
    forge title: obsidian = "Obsidian"

    forge red: u8 = 255
    forge green: u8 = 128
    forge pixel: u32 = (red as u32 << 16) | (green as u32 << 8)

    echo(title)
    echo(pixel)
}
```

---

## 5. Functions

Functions are declared using the `vein` keyword. The entry point of an executable program is `core main()`.

```ofs
vein add(a: stone, b: stone) -> stone {
    return a + b
}

vein factorial(n: stone) -> stone {
    if (n <= 1) {
        return 1
    }
    return n * factorial(n - 1)
}

core main() {
    forge sum = add(15, 27)
    forge fact = factorial(5)
    echo(sum)
    echo(fact)
}
```

Functions can compose calls and invoke other functions:

```ofs
vein square(n: stone) -> stone {
    return n * n
}

vein sum_of_squares(a: stone, b: stone) -> stone {
    return square(a) + square(b)
}

core main() {
    echo(sum_of_squares(3, 4))
}
```

> **Note**: Anonymous function expressions (*lambdas*) are modeled in the AST specification (`NK_LAMBDA`) and scheduled for an upcoming compiler release. Currently, all functions are declared as named items with `vein`.

---

## 6. Data Structures (`monolith`)

Composite types are defined with `monolith`, and methods are attached using `impl`:

```ofs
monolith Rectangle {
    width: stone
    height: stone
}

impl Rectangle {
    vein area(self: Rectangle) -> stone {
        return self.width * self.height
    }

    vein perimeter(self: Rectangle) -> stone {
        return (self.width + self.height) * 2
    }
}

core main() {
    forge r: Rectangle
    r.width = 10
    r.height = 5

    echo(r.area())
    echo(r.perimeter())
}
```

---

## 7. Modules and Imports (`attach`)

The `attach` keyword imports modules from the installed standard library or local files:

```ofs
attach {math}
attach {string}
attach {io}
```

To import local relative or absolute files, prefix the path with `F:`:

```ofs
attach {F:./modules/helper.ofs}
```

---

## 8. Control Flow

### Conditionals (`if` / `else`)

```ofs
core main() {
    forge x: stone = 10
    if (x > 0) {
        echo("positive")
    } else if (x < 0) {
        echo("negative")
    } else {
        echo("zero")
    }
}
```

### Loops (`while`)

```ofs
core main() {
    forge i: stone = 0
    while (i < 5) {
        echo(i)
        i = i + 1
    }
}
```

### Multi-Branch Selection

Multi-branch decision logic is currently written with chained `if / else if / else`:

```ofs
core main() {
    forge status_code = 200
    if (status_code == 200) {
        echo("OK")
    } else if (status_code == 404) {
        echo("Not Found")
    } else {
        echo("Unknown code")
    }
}
```

> **Note**: Pattern matching with `match` is reserved in the AST specification (`NK_MATCH`) and grammar, with dedicated parser and lowering support planned for an upcoming release.

### Error Handling and Abort (`throw`)

The `throw` statement reports a diagnostic error message and cleanly terminates the process with exit code 1:

```ofs
vein divide(dividend: stone, divisor: stone) -> stone {
    if (divisor == 0) {
        throw "Division by zero"
    }
    return dividend / divisor
}

core main() {
    echo(divide(10, 2))
}
```

> **Note**: The `tremor { ... } catch (...) { ... }` block structure is modeled in the grammar (`NK_TREMOR`), with non-terminating stack unwinding planned for the upcoming interpreter and runtime stages.

---

## 9. Native Interoperability (`rift` / `extern vein`)

External C symbols can be bound directly:

```ofs
extern vein puts(s: obsidian) -> stone
rift vein strlen(s: obsidian) -> stone bind "strlen" abi c

core main() {
    puts("Message dispatched via C puts")
}
```

---

## 10. Web Stack: ODL and OES

OFS provides two native DSLs designed for authoring web applications without writing raw HTML or CSS by hand:

### ODL (Obsidian Document Language)

Semantic document structure compiled into HTML5:

```odl
page "OFS Application" "en"
skin "style.css"

flow
  deck.navigation
    title1 "Main Header"
    link.button "Documentation" "#docs"
  stage#content
    band.article
      mark "## Native Content\nFormatted with **Markdown**."
      raw "<div class=\"custom\">Raw HTML when required</div>"
  spark "app.js"
```

Compile ODL:
```bash
ofs odl document.odl -o public/index.html
```

### OES (Obsidian Effect Scripts)

Visual tokens, layout rules, media queries, and keyframe animations compiled into CSS:

```oes
gem primary "#5b34ea"
gem background "#120e24"

paint .button
  fill "$primary"
  curve "6px"
  space "8px 16px"
  edge "none"

veil "(max-width: 768px)"
  paint .grid
    grid-cols "1fr"

pulse "appear"
  rise
    opacity "0"
  rest
    opacity "1"
```

Compile OES:
```bash
ofs oes style.oes -o public/style.css
```

### Native HTTP Web Server (`webserver`)

The standard library includes a native HTTP server built into the runtime:

```ofs
attach {webserver}

core main() {
    forge html = "<h1>OFS Native Server</h1><p>Running without third-party dependencies.</p>"
    webserver.serve_html_forever(8080, html)
}
```

---

## 11. Low-Level Features

OFS provides explicit machine-level constructs:

- `fracture { ... }`: Typed pointer context.
- `abyss { ... }`: Unrestricted low-level operations.
- `bedrock`: Standard library module for explicit memory allocations, regions, and byte-level memory.

```ofs
attach {bedrock}

core main() {
    forge cell = bedrock_cell_new(42)
    echo(bedrock_cell_read(cell))
    bedrock_cell_drop(cell)
}
```

---

## 12. Next Steps

- Consult the [Language Reference](LANGUAGE_REFERENCE.md) for full syntax and type rules.
- Review the [ODL Reference](ODL_REFERENCE.md) and [OES Reference](OES_REFERENCE.md) for web development.
- Explore the [Standard Library Reference](STANDARD_LIBRARY.md).
