# Getting Started with Obsidian Fault Script

> Learn OFS in 10 minutes — from installation to your first program.

---

## 1. Install OFS

### Prerequisites

You need CMake, a C++ compiler, and LLVM installed:

```bash
# Ubuntu / Debian
sudo apt update
sudo apt install cmake g++ llvm-17-dev clang-17 lld-17

# macOS
brew install cmake llvm@17

# Windows
winget install Kitware.CMake LLVM.LLVM
```

### Build the Compiler

```bash
git clone https://github.com/Samwns/Obsidian-Fault-Script.git
cd Obsidian-Fault-Script/ofs

cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

### Verify

```bash
./build/ofs version
# Output: ofs 0.2.0 — Obsidian Fault Script
```

---

## 2. Hello World

Create a file called `hello.ofs`:

```ofs
core main() {
    echo("Hello, World!")
}
```

Run it:

```bash
./build/ofs hello.ofs
# Output: Hello, World!
```

That's it! OFS works just like Python — point it at a file and it runs.

---

## 3. Variables and Types

OFS has **type inference** — you don't always need to write types:

```ofs
core main() {
    // Type is inferred automatically
    forge name = "Rex"          // obsidian (string)
    forge age = 25              // stone (integer)
    forge height = 1.75         // crystal (float)
    forge alive = true          // bool

    echo(name)
    echo(age)
    echo(height)
    echo(alive)
}
```

You can also be explicit about types:

```ofs
forge count: stone = 0
forge pi: crystal = 3.14159
forge greeting: obsidian = "Hello"
```

---

## 4. Functions

Use `vein` to define functions:

```ofs
vein add(a: stone, b: stone) -> stone {
    return a + b
}

vein greet(name: obsidian) -> void {
    echo("Hello, " + name + "!")
}

core main() {
    forge sum = add(10, 20)
    echo(sum)          // 30
    greet("World")     // Hello, World!
}
```

**Key points:**
- `vein` = regular function
- `core` = program entry point (like `main` in C)
- Parameters need explicit types: `name: type`
- Return type goes after `->` arrow

---

## 5. New in v1.0.x

### Immutable variable (`const`)

```ofs
const retries: stone = 3
```

### Enum (`strata`)

```ofs
strata Status { Idle, Running, Failed }
```

### Pattern matching (`match/case/default`)

```ofs
match code {
    case 200: { echo("ok") }
    default: { echo("unknown") }
}
```

### Error flow (`tremor/catch/throw`)

```ofs
tremor {
    throw "boom"
} catch (e: obsidian) {
    echo(e)
}
```

---

## 5. Control Flow

### If / Else

```ofs
core main() {
    forge score: stone = 85

    if (score >= 90) {
        echo("A")
    } else if (score >= 80) {
        echo("B")
    } else if (score >= 70) {
        echo("C")
    } else {
        echo("F")
    }
}
```

### Loops (cycle)

**C-style loop:**

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
