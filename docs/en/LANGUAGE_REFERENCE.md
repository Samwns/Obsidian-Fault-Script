# Obsidian Fault Script — Language Reference

Comprehensive technical reference for the Obsidian Fault Script (OFS) programming language.

---

## Table of Contents

1. Program Structure
2. Type System
3. Variables and Constants
4. Functions and Closures
5. `monolith`, `impl`, and Methods
6. `namespace`
7. Arrays and Collections
8. Control Flow
9. Operators and Type Casts
10. Modules and `attach`
11. Foreign Function Interface (`extern vein`, `rift vein`)
12. Low-Level Execution Contexts
13. Standard Library Modules
14. Compiler Toolchain and Commands

---

## 1. Program Structure

Every standalone executable requires a `core main()` entry point:

```ofs
core main() {
    echo("Program started")
}
```

An OFS source file (`.ofs`) can contain the following top-level declarations:

- `attach { ... }`: Module imports.
- `extern vein` / `rift vein`: External symbol declarations.
- `vein`: Function declarations.
- `core main()`: Program entry point.
- `monolith`: Struct / composite type definitions.
- `impl`: Method implementations for a `monolith`.
- `namespace`: Symbol grouping.
- `strata`: Enumerations / tagged categories.
- `const`: Compile-time constants.
- `forge`: Global variable bindings.

---

## 2. Type System

OFS is a statically typed language with strict type validation.

### Primitive Types

| Type | Description | Machine Representation |
|---|---|---|
| `stone` | Default signed integer | 64-bit signed integer |
| `crystal` | Double-precision float | 64-bit IEEE 754 float |
| `obsidian` | String pointer | Pointer to UTF-8 buffer |
| `bool` | Boolean value | 1-bit / 8-bit logical |
| `void` | Empty return type | Zero-sized value |

### Fixed-Width Integer Types

| Type | Signedness | Width |
|---|---|---|
| `u8` | Unsigned | 8 bits |
| `u16` | Unsigned | 16 bits |
| `u32` | Unsigned | 32 bits |
| `u64` | Unsigned | 64 bits |
| `i8` | Signed | 8 bits |
| `i32` | Signed | 32 bits |

### Composite and Function Types

- **Dynamic Array**: `Array<T>`, e.g., `Array<stone>`
- **Function Pointer**: `vein(T1, T2) -> R`, e.g., `vein(stone) -> stone`
- **Composite Monolith**: `monolith Point { x: stone, y: stone }`

### Type Inference

When variables are initialized with `forge`, the compiler automatically infers their types:

```ofs
forge count = 42              // stone
forge ratio = 1.618           // crystal
forge label = "OFS"           // obsidian
forge enabled = true          // bool
forge items = [10, 20, 30]    // Array<stone>
```

---

## 3. Variables and Constants

### Mutable Bindings (`forge`)

Local variables are declared using `forge`:

```ofs
forge counter: stone = 0
counter = counter + 1
```

### Immutable Constants (`const`)

Constants are declared at top-level or block scope:

```ofs
const MAX_BUFFER: stone = 4096
const PI: crystal = 3.141592653589793
```

---

## 4. Functions and Closures

### Function Declaration (`vein`)

Functions specify typed parameters and an explicit return type:

```ofs
vein multiply(a: stone, b: stone) -> stone {
    return a * b
}
```

Functions returning no value use `-> void` (or omit the return arrow):

```ofs
vein log_info(msg: obsidian) -> void {
    echo("[INFO] " + msg)
}
```

### Anonymous Functions (*Lambdas*)

Anonymous function literals can be assigned to variables or passed as arguments:

```ofs
vein apply_op(val: stone, op: vein(stone) -> stone) -> stone {
    return op(val)
}

core main() {
    forge square = vein(n: stone) -> stone {
        return n * n
    }
    echo(apply_op(8, square))
}
```

---

## 5. `monolith`, `impl`, and Methods

Composite records are declared with `monolith`. Methods are defined within an `impl` block:

```ofs
monolith Vector2 {
    x: stone
    y: stone
}

impl Vector2 {
    vein length_squared(self) -> stone {
        return self.x * self.x + self.y * self.y
    }

    vein scale(self, factor: stone) -> void {
        self.x = self.x * factor
        self.y = self.y * factor
    }
}

core main() {
    forge v: Vector2
    v.x = 3
    v.y = 4
    echo(v.length_squared())
    v.scale(2)
    echo(v.x)
}
```

---

## 6. `namespace`

Namespaces group functions, monoliths, and constants to prevent identifier collisions:

```ofs
namespace geometry {
    const EPSILON: crystal = 0.0001

    vein dot_product(x1: stone, y1: stone, x2: stone, y2: stone) -> stone {
        return x1 * x2 + y1 * y2
    }
}

core main() {
    echo(geometry.dot_product(1, 2, 3, 4))
}
```

---

## 7. Arrays and Collections

Arrays are homogeneous and dynamic:

```ofs
core main() {
    forge numbers = [10, 20, 30, 40]
    echo(numbers[0])

    numbers[1] = 99
    echo(numbers[1])
}
```

Array builtins provided by the runtime include indexing and length retrieval.

---

## 8. Control Flow

### `if` / `else if` / `else`

Conditionals do not require parentheses around the expression, but block braces `{}` are mandatory:

```ofs
if score >= 90 {
    echo("Grade: A")
} else if score >= 80 {
    echo("Grade: B")
} else {
    echo("Grade: C")
}
```

### `while`

Standard pre-condition iteration loop:

```ofs
forge idx = 0
while idx < 10 {
    echo(idx)
    idx = idx + 1
}
```

### Pattern Matching (`match`)

```ofs
match status {
    case 200: { echo("Success") }
    case 404: { echo("Resource Not Found") }
    default:  { echo("Unhandled code") }
}
```

### Error Handling (`tremor` / `catch` / `throw`)

```ofs
tremor {
    if denominator == 0 {
        throw "Division by zero is undefined"
    }
    forge result = numerator / denominator
    echo(result)
} catch (err: obsidian) {
    echo("Runtime error: " + err)
}
```

---

## 9. Operators and Type Casts

### Arithmetic & Bitwise
- Addition, subtraction, multiplication, division, modulo: `+`, `-`, `*`, `/`, `%`
- Bitwise: `&`, `|`, `^`, `<<`, `>>`, `~`

### Comparison & Logical
- Equality and comparison: `==`, `!=`, `<`, `<=`, `>`, `>=`
- Logical: `&&`, `||`, `!`

### Explicit Casts (`as`)
```ofs
forge byte_val: u8 = 200
forge wide_val: u32 = byte_val as u32
forge float_val: crystal = 42 as crystal
```

---

## 10. Modules and `attach`

OFS uses `attach` to import modules:

```ofs
// Import from standard library
attach {math}
attach {string}
attach {io}

// Import local relative or absolute files
attach {F:./utils/formatting.ofs}
```

---

## 11. Foreign Function Interface (FFI)

### `extern vein`
Declares external C ABI symbols linked by the system toolchain:

```ofs
extern vein exit(code: stone) -> void
extern vein puts(s: obsidian) -> stone
```

### `rift vein`
Explicit FFI boundary with customizable symbol binding:

```ofs
rift vein c_strlen(s: obsidian) -> stone bind "strlen" abi c
```

---

## 12. Low-Level Execution Contexts

OFS provides language constructs for systems-level programming without breaking the type system:

- **`fracture { ... }`**: Context with raw pointer operations and checked pointer arithmetic.
- **`abyss { ... }`**: Unchecked low-level execution context.
- **`bedrock`**: Standard module for manual heap cells, buffers, and memory regions.

```ofs
attach {bedrock}

core main() {
    forge cell = bedrock_cell_new(100)
    bedrock {
        bedrock_cell_add(cell, 25)
    }
    echo(bedrock_cell_read(cell))
    bedrock_cell_drop(cell)
}
```

---

## 13. Standard Library Modules

The standard library includes:

- `math.ofs`: Mathematical algorithms, prime checks, digit counters, and ranges.
- `string.ofs`: Text manipulation, repetitions, substrings, and comparisons.
- `io.ofs`: Console I/O, prompts, separators, and formatted headers.
- `terminal_colors.ofs`: ANSI terminal color escape sequences.
- `odl.ofs`: Programmatic ODL document generation.
- `oes.ofs`: Programmatic OES style rule generation.
- `webserver.ofs`: Native HTTP server and response builders.
- `webui.ofs`: High-level UI components for web interfaces.
- `canvas.ofs` / `window.ofs`: Native 2D raster canvas and native windowing.
- `bedrock.ofs`: Low-level memory buffers, cells, and arenas.

---

## 14. Compiler Toolchain and Commands

The unified `ofs` CLI provides:

```bash
# Compilation
ofs build app.ofs -o app       # Produces a native binary
ofs check app.ofs              # Performs lexical, syntax, and type validation
ofs run app.ofs                # Compiles and runs in temporary space

# Debugging and Introspection
ofs tokens app.ofs             # Emits token stream
ofs ast app.ofs                # Emits abstract syntax tree
ofs ir app.ofs                 # Emits LLVM IR (.ll)
ofs asm app.ofs                # Emits target assembly

# Web DSLs
ofs odl page.odl -o index.html # Compiles ODL to HTML5
ofs oes theme.oes -o theme.css # Compiles OES to CSS
```
