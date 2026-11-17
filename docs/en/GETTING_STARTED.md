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

### Termux (Android)

OFS runs on the phone itself, inside Termux (aarch64).

**Install** (in Termux):

```bash
curl -fsSL https://raw.githubusercontent.com/Samwns/Obsidian-Fault-Script/main/src/packaging/termux/install_termux.sh | bash
# or, with a local package:  bash install_termux.sh ofs-termux-aarch64.tar.gz
```

The installer runs `pkg install -y clang`, downloads the latest `ofs-termux-aarch64.tar.gz` release, extracts it into `$PREFIX` (`bin/ofs` and `lib/ofs/`) and tests `ofs --version`.

**Use:** `ofs run hello.ofs` (interpreted), `ofs build hello.ofs -o hello && ./hello` (native aarch64, linked with Termux's `clang`). Tools live in `$PREFIX/lib/ofs/tools/` (`ofs run $PREFIX/lib/ofs/tools/odlc.ofs`).

**Limitations:** no X11 windows for now (`.oll`/`.oal` windows); building an APK on the phone (`ofs build --target android`) comes later.

## 2. Command-Line Interface (CLI)

The `ofs` CLI provides the following subcommands:

| Command | Description |
|---|---|
| `ofs <file.ofs>` | Runs the program directly (same as `ofs run`). |
| `ofs run <file.ofs> [--native]` | Runs via the interpreter, which crystallizes hot veins to native code by itself (or, with `--native`, builds the whole program in a temp dir). Chapter 12. |
| `ofs build <file.ofs> -o <binary>` | Compiles the file to a native binary via LLVM (`--release` drops run-time contract checks). |
| `ofs check <file.ofs>` | Type-checks without emitting code. |
| `ofs refract <file.ofs>` | Runs and live-swaps the veins you edit and save; `--remote` does it on an Android app. Chapter 13. |
| `ofs assay <file.ofs>` | Boundary cases for veins with contracts (`bearing`/`settle`). Chapter 14. |
| `ofs fossil <file.ofs>` | Records a run so it can be replayed and stepped back in time (`ofs fossil replay`). Chapter 19. |
| `ofs quarry <add\|list\|install\|...>` | Package manager. Chapter 19. |
| `ofs fmt` / `ofs polish` | Formats the code (`--check` and `--diff` for CI). |
| `ofs test [folder]` | Runs the `.ofs` files that have a `.expected` beside them and compares the output. |
| `ofs twin <file.ofs>` | Runs interpreted and native, comparing outputs. |
| `ofs doc [target]` | Generates HTML documentation from the code comments. |
| `ofs language [en\|pt\|es]` | Shows or changes the CLI language. |
| `ofs ir` / `ofs asm` / `ofs tokens` / `ofs ast` | Compiler debug artifacts (LLVM IR, assembly, tokens, AST). |
| `ofs ui <file.oll> [-o output.ppm]` | Renders a native OLL layout to an image. |
| `ofs clean` | Cleans temporary build artifacts. |
| `ofs version` / `ofs update` / `ofs help` | Version, update to the latest release, and help. |

Environment variables recognized by the launcher: `OFS_LANG` (en/pt/es), `OFS_PROGRESS` (fancy/plain/json), `OFS_VERBOSE` (verbose link/build logs), `OFS_RUNTIME` and `OFS_STDLIB_PATH` (runtime and standard library paths), `OFS_CRYSTALLIZE=0` (turns off the automatic promotion to native in `ofs run`) and `OFS_CONTRACTS=0` (removes run-time contract checks). Full tool reference: [TOOLS.md](TOOLS.md).

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

> **Note**: Anonymous functions that capture variables are `geode (x: stone) -> stone { return x + base }` (closures; see the "`geode`" section of the [Reference](LANGUAGE_REFERENCE.md)). The value has the type `vein(stone) -> stone` and is passed, stored and called like any vein.

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

Multi-branch decision logic is written with chained `if / else if / else`:

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

When all the conditions compare the same value, `match` (synonym: `assay`) is shorter; `_` is the default case:

```ofs
core main() {
    forge status_code = 404
    match (status_code) {
        200 => { echo("OK") }
        404 => { echo("Not Found") }
        _ => { echo("Unknown code") }
    }
}
```

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

## 9. The Rift: native system libraries (`rift` / `extern vein`)

The **Rift** is the boundary through which OFS reaches native system libraries (libc, libm, X11, Android JNI, kernel32). `rift vein` and `extern vein` declare a function that lives in a native library, called through the system calling convention. There is no C code in OFS: the compiler and the runtime are written in OFS.

> The system calling convention is known technically as the C ABI (`abi c`); it does not involve C code.

```ofs
extern vein puts(s: obsidian) -> stone
rift vein strlen(s: obsidian) -> stone bind "strlen" abi c

core main() {
    puts("Message dispatched via libc puts")
}
```

It works the same in `ofs run` and `ofs build`. A module that declares `extern`/`rift vein` asks for `claim rift` at the top (see chapter 17, which also shows callbacks and `f32`/`crystal` through the Rift).

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

Compile ODL (the `odlc` tool ships with the standard library and runs on `ofs` itself):

```bash
# inside the OFS repository (or with tools installed at /usr/share/ofs/tools)
ODLC_INPUT=document.odl ODLC_OUTPUT=public/index.html ofs run ofs/tools/odlc.ofs
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

Compile OES (same pattern, with the `oesc` tool):

```bash
OESC_INPUT=style.oes OESC_OUTPUT=public/style.css ofs run ofs/tools/oesc.ofs
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
- `bedrock`: Standard library module for explicit memory allocations, regions, and byte-level memory (and, as a block, the arena of chapter 15).

```ofs
attach {bedrock}

core main() {
    forge cell = bedrock_cell_new(42)
    echo(bedrock_cell_read(cell))
    bedrock_cell_drop(cell)
}
```

Chapters 15 and 17 show how to do this safely (`seal`, the `bedrock { }` arena, `fracture`, `abyss`, `stratum`/`claim`/`caprock`).

---

## 12. `ofs run` vs `ofs build`: quick to write, quick to run

Both commands take the same program and give the same output. The difference is when the work happens:

- **`ofs run app.ofs`** interprets the program right away: no binary, great for writing and testing.
- **`ofs build app.ofs -o app`** produces a native executable (LLVM) that you ship and run without `ofs`.

You do not have to pick `ofs build` just for speed: `ofs run` **speeds itself up**. It counts the calls of each `vein`; when one gets "hot" (1000 calls by default) it is compiled to native code in the background (**crystallization**) and later calls use it. A long loop inside `main` also turns native in the middle of the run. Without `clang`, the program simply stays interpreted, only slower.

```ofs
vein fib(n: stone) -> stone {
    if (n < 2) { return n }
    return fib(n - 1) + fib(n - 2)
}

core main() {
    echo(fib(32))
}
```

```bash
ofs run fib.ofs                       # 2178309, crystallizes by itself (about 0.06 s here)
OFS_CRYSTALLIZE=0 ofs run fib.ofs     # 2178309, interpreted only (about 1.5 s here)
ofs build fib.ofs -o fib && ./fib     # 2178309, native
```

Use `OFS_CRYSTALLIZE=0` to turn the promotion off (handy for debugging) and `OFS_VERBOSE=1` to see when it happens. Times depend on the machine; comparable measurements are in [PERFORMANCE.md](PERFORMANCE.md). The full model is in [COMPILED_LANGUAGE.md](COMPILED_LANGUAGE.md).

---

## 13. Live editing: `ofs refract`

`ofs refract app.ofs` runs the program and watches the file. Each time you edit a `vein` and save, it is swapped **while the program keeps running**, with no restart and without losing global variables.

```ofs
claim rift
extern vein usleep(us: stone) -> stone

vein msg() -> obsidian { return "hi" }

core main() {
    forge i = 0
    while (i < 20) {
        echo(msg() + " " + ofs_stone_to_obsidian(i))
        usleep(250000)
        i++
    }
}
```

```bash
ofs refract live.ofs
# ofs refract: observando live.ofs; salve para refratar
# hi 0
# hi 1
# (change "hi" to "yo" in the editor and save)
# ofs refract: 1 vein refratada (msg)
# yo 2
```

The `main` loop continues where it was: only the edited `vein` changes. If the file has an error, the diagnostic is printed and the running version carries on. Changes that cannot be made live (for example changing the type of a global) are refused with a message saying what needs a restart.

**On the phone (Android):** `ofs refract --remote app.oal` keeps the app **running natively on the device** and sends only the veins you edited (over `adb`). See [ANDROID.md](ANDROID.md) to prepare the app and [TOOLS.md](TOOLS.md) for the rules and variables. Language reference: [Live editing](LANGUAGE_REFERENCE.md).

---

## 14. Contracts: `mineral`, `grain`, `bearing`, `settle`

Contracts remove the repeated validation `if` from your code. A **`mineral`** is a type with a rule (`grain`); a `vein` can require something from its caller (`bearing`) and promise something about its return (`settle`, where `result` is the returned value).

```ofs
mineral Age = stone grain value >= 0 grain value <= 150

vein divide(a: stone, b: stone) -> stone
    bearing b != 0
    settle result * b + a % b == a
{
    return a / b
}

core main() {
    forge i: Age = 30          // checks the rule
    echo(divide(17, 5))        // 3
    forge zero = 0
    echo(divide(1, zero))      // violates the bearing
}
```

Output: `3`, then on stderr, with exit code 1:

```text
ofs: [CONTRATO VIOLADO contract.ofs:4:12]: bearing de 'divide' falhou: b != 0 (b = 0)
```

When the violation is provable at compile time (`divide(1, 0)`, `forge i: Age = 200`), the compiler reports `[ERRO DE CONTRATO]` before running.

- **`ofs assay file.ofs`** calls the veins that have contracts at the boundary values of their rules and shows what passes:

```text
$ ofs assay half.ofs
assay half(n: stone)
  --   half(-1): fora do bearing n >= 0
  ok   half(0)
  ok   half(1)
assay: 2 caso(s) ok em 1 vein(s); 1 fora das regras de entrada
```

- **`ofs build --release`** (or `OFS_CONTRACTS=0`) removes the **run-time** checks from the binary; compile-time checks stay.

Reference: [Contracts and refined types](LANGUAGE_REFERENCE.md) and [`ofs assay`](TOOLS.md).

---

## 15. Safe cleanup: `seal` and the `bedrock { }` arena

### `seal`: runs on scope exit

`seal { ... }` (or `anchor`, the same thing) registers an action that runs **when the scope ends, by any path**: end of block, `return`, `shatter`/`erode`. Use it to close files, free memory and undo state without repeating the cleanup at every `return`.

```ofs
vein process(name: obsidian) -> stone {
    echo("open " + name)
    seal { echo("close " + name) }
    if (name == "x") { return 1 }
    echo("working on " + name)
    return 0
}

core main() {
    echo(process("a"))
    echo(process("x"))
}
```

```text
open a
working on a
close a
0
open x
close x
1
```

Several `seal`s run in reverse order (the last registered runs first). Reference: [`seal` and `anchor`](LANGUAGE_REFERENCE.md).

### `bedrock { }`: a memory arena

A `bedrock { ... }` block that **allocates** (text, `Array`, `ofs_alloc`) uses an **arena**: everything it allocates is released at once when the block ends. It is the simplest way to process lots of temporary data without leaking memory.

```ofs
core main() {
    forge total = 0
    forge last = ""
    forge i = 0
    while (i < 1000) {
        bedrock {
            forge parts: Array<obsidian> = []
            ofs_array_push(parts, "item" + ofs_stone_to_obsidian(i))
            total = total + ofs_str_len(parts[0])
            last = parts[0]           // the text is copied out
        }                             // the whole arena is freed here
        i++
    }
    echo(total)    // 6890
    echo(last)     // item999
}
```

Golden rule: **`obsidian` text is copied** out of the arena automatically; an `Array`, `monolith` or pointer that "escapes" the block is an `[ERRO DE ARENA]` at compile time, with a hint to allocate outside the block. There is also an `[AVISO DE VAZAMENTO]` (leak warning) for an `ofs_alloc` that is never freed. Reference: [`bedrock { }` arena](LANGUAGE_REFERENCE.md).

---

## 16. Veins as values: `vein(T) -> R` and `geode`

The name of a `vein` is a **value** with a function type, written with the same word: `vein(stone) -> stone`. You can store it in a variable, `Array` or field, pass it as an argument and return it.

```ofs
vein twice(x: stone) -> stone { return x * 2 }
vein thrice(x: stone) -> stone { return x * 3 }

vein apply(f: vein(stone) -> stone, x: stone) -> stone { return f(x) }

core main() {
    forge f: vein(stone) -> stone = twice
    echo(f(21))                 // 42
    echo(apply(thrice, 4))      // 12
    forge ops: Array<vein(stone) -> stone> = [twice, thrice]
    echo(ops[1](10))            // 30
}
```

A **`geode`** is an unnamed vein that **captures** variables from the outer scope (what other languages call a closure or lambda). The capture is a copy of the value at the moment the geode is created.

```ofs
vein adder(base: stone) -> vein(stone) -> stone {
    return geode (x: stone) -> stone { return x + base }
}

core main() {
    forge plus10 = adder(10)
    forge plus1 = adder(1)
    echo(plus10(5))    // 15
    echo(plus1(5))     // 6
}
```

Reference: [Function types and vein values](LANGUAGE_REFERENCE.md) and [`geode`](LANGUAGE_REFERENCE.md).

---

## 17. Low level, with guard rails

OFS lets you touch raw memory and assembly, but it separates **where** and **what**, and each module declares what it uses.

### `fracture` and `abyss` blocks

`*p` (read/write through a pointer) and `&x` (address) are only valid inside a low-level block. `fracture { }` is the typed pointer block; `abyss { }` is the raw, unrestricted one (function addresses, `fault_call`, `quartz`).

```ofs
core main() {
    forge x = 5
    fracture {
        forge p: *stone = &x
        *p = *p + 1
    }
    echo(x)    // 6
}
```

Outside the block, the same code is a compile error. Full rules: [Rules for `fracture`, `bedrock`, `tectonic` and `abyss`](LANGUAGE_REFERENCE.md).

### `stratum`, `claim` and `caprock`: per-module permissions

At the top of a file, the module declares its **stratum** (how much low level it uses) and its **claims** (which). Whoever attaches the module sees what it asks for, and the main program can impose a ceiling with `caprock`.

```ofs
// raw_io.ofs
claim rift                 // this module declares extern vein

extern vein getenv(name: obsidian) -> obsidian
vein home() -> obsidian { return getenv("HOME") }
```

```ofs
// main.ofs
caprock crust              // no attached module may ask for low level
attach {F:raw_io.ofs}

core main() { echo(1) }
```

Running main: `[ERRO DE PERMISSAO ...]: o modulo raw_io.ofs pede 'rift', mas o caprock do programa principal libera so: nada (caprock crust)`. And a file with `stratum crust` accepts neither `&x` nor `extern`:

```text
[ERRO DE ESTRATO stratum.ofs:5:24]: endereco '&x' num arquivo 'stratum crust' (sem baixo nivel)
```

Reference: [Stratum and per-module permissions](LANGUAGE_REFERENCE.md).

### The Rift: native system functions

The **Rift** is the crack through which OFS calls the system's native libraries (libc, libm, X11, Android JNI). There is no C code in OFS: the compiler and the runtime are written in OFS. Declare the function with `rift vein` (or `extern vein`) and ask for `claim rift`. It works the same in `ofs run` and `ofs build`.

```ofs
claim rift

rift vein strlen(s: obsidian) -> stone bind "strlen" abi c
extern vein puts(s: obsidian) -> stone

core main() {
    echo(strlen("obsidiana"))    // 9
    puts("written by libc")
}
```

A `vein` or a `geode` **without captures** can be handed to the system as a callback (for example the comparator of `qsort`); `f32` and `crystal` also cross the Rift. Reference: [`extern vein` and `rift vein`](LANGUAGE_REFERENCE.md).

### `quartz`: inline assembly

`quartz { ... }` writes machine instructions inside the `vein`, AT&T syntax by default. It is only valid in `abyss` and is architecture-specific (the example is x86-64).

```ofs
claim abyss, quartz

abyss vein add(a: stone, b: stone) -> stone {
    forge total: stone = 0
    quartz {
        "movq $1, $0"
        "addq $2, $0"
        in "r" a
        in "r" b
    } -> total "=&r"
    return total
}

core main() {
    echo(add(20, 22))    // 42
}
```

In `ofs run`, a vein with `quartz` becomes native automatically (it needs `clang` on the first run). Reference: [`quartz`](LANGUAGE_REFERENCE.md).

---

## 18. Loading OFS at run time: `magma.cast`

A program can compile a **mod** written in OFS while it runs and call its veins: plugins, game scripts. Since new code enters the process, the file needs `claim magma`.

```ofs
// mods/forge.ofs
vein forge_it(n: stone) -> stone { return n * n + 1 }
```

```ofs
attach {magma}
claim magma

core main() {
    forge m = magma.cast("mods/forge.ofs")
    if (!magma.cooled(m)) {
        echo(magma.crack(m))       // the compiler's message
        return
    }
    forge forge_it: vein(stone) -> stone = magma.vent(m, "forge_it")
    echo(forge_it(10))             // 101
    magma.release(m)
}
```

A mod error does not bring the program down: `magma.cooled` gives `false` and `magma.crack` returns the message. Reference: [`magma.cast`](LANGUAGE_REFERENCE.md). Repository example: `ofs/examples/magma_cast_demo.ofs`.

---

## 19. Everyday tools

All the tools below are programs written in OFS that `ofs` compiles the first time and caches. The full list is in [TOOLS.md](TOOLS.md).

### `ofs fmt` and `ofs polish`: format

Fixes indentation and spacing without changing the program. `ofs polish` is the same command under another name.

```bash
ofs fmt messy.ofs          # rewrites in place
ofs fmt --diff messy.ofs   # only shows the difference
ofs fmt --check src/       # in CI: lists what would change and exits with 1
```

```ofs
// before                          // after
vein sum(n:stone)->stone {         vein sum(n: stone) -> stone {
  forge t=0                            forge t = 0
  ...                                  ...
```

### `ofs test` and `ofs twin`: verify

`ofs test [folder]` runs every `x.ofs` that has an `x.expected` beside it and compares the output. `ofs twin x.ofs` runs interpreted **and** native and confirms they give the same result.

```bash
printf '55\n' > count.expected     # the expected output of count.ofs
ofs test .        # ok    count.ofs  /  resumo: 1 ok, 0 falha(s), 1 teste(s)
ofs twin count.ofs                 # iguais: count.ofs
```

### `ofs doc`: documentation from the code

Comment each declaration with `//` right above it and `ofs doc` generates a reference site (one page per module, search and coverage).

```ofs
// Sums the first n numbers.
// @param n how many numbers to sum
// @returns the sum from 1 to n
vein sum(n: stone) -> stone { ... }
```

```bash
ofs doc count.ofs -o docs_api   # generates docs_api/index.html
ofs doc --check                 # only the coverage and the functions cited in .md files that do not exist
```

### `ofs fossil`: record a run and travel back in time

`ofs fossil` runs the program and records, in a `.fossil` file, everything that comes from outside and changes each time (clock, random, input, files, network). `replay` repeats the run with the recorded values, without touching the world, and lets you **stop at any point**.

```ofs
claim rift
extern vein rand() -> stone
extern vein time(t: stone) -> stone

core main() {
    echo(rand() % 1000)
    echo(time(0) > 0)
}
```

```bash
ofs fossil rec.ofs                      # 383, true  (records rec.fossil)
ofs fossil replay rec.fossil            # 383, true  (the same, always)
ofs fossil replay rec.fossil --to L7    # stops at line 7 and shows locals and globals
ofs fossil replay rec.fossil --dig      # interactive mode, with step back
```

(The random number changes on every recording; replay always returns the recorded one.)

### `ofs quarry`: packages

`ofs quarry` installs community libraries so that `attach` finds them with no configuration.

```bash
ofs quarry search colors             # searches the registry
ofs quarry add terminal-colors       # installs and records in ofs.claim / ofs.lode
ofs quarry list                      # what is installed
ofs quarry install                   # reproduces the project from ofs.lode (checks sha256)
```

```ofs
attach {terminal_colors}    // uses the installed package (hyphen becomes underscore)
```

The `ofs.claim` manifest and publishing (`ofs quarry publish`) are in [TOOLS.md](TOOLS.md).

---

## 20. Next Steps

- Consult the [Language Reference](LANGUAGE_REFERENCE.md) for full syntax and type rules.
- Review the [ODL Reference](ODL_REFERENCE.md) and [OES Reference](OES_REFERENCE.md) for web development.
- Explore the [Standard Library Reference](STANDARD_LIBRARY.md).
- See all the tools (`fmt`, `doc`, `refract`, `fossil`, `quarry`, ...) in [TOOLS.md](TOOLS.md), the `ofs run`/`ofs build` model in [COMPILED_LANGUAGE.md](COMPILED_LANGUAGE.md) and the Android app in [ANDROID.md](ANDROID.md).
- The tested examples for each feature live in `ofs/tests/regress/` and `ofs/examples/`.
