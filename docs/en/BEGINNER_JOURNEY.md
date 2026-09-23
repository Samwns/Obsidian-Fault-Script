# OFS — Beginner's Journey

A step-by-step practical guide to learning Obsidian Fault Script (OFS) from the ground up, starting with primitives and advancing to structures, modules, and closures.

---

## How to Follow This Guide

1. Read and work through one module at a time.
2. Run each example via the terminal with `ofs run file.ofs` (or compile with `ofs build file.ofs -o app`).
3. Experiment by changing values and verifying compiler feedback.
4. Advance to the next module only after fully understanding the program output.

---

## Module 1: Variables and Primitive Types

**Goal**: Understand `core main()`, variable declarations with `forge`, console output with `echo()`, and fundamental types.

```ofs
core main() {
    forge name: obsidian = "Alice"
    forge age: stone = 24
    forge rating: crystal = 9.8
    forge active: bool = true

    echo(name)
    echo(age)
    echo(rating)
    echo(active)
}
```

**Challenge**: Declare additional variables to hold a city name (`obsidian`) and an exchange rate (`crystal`).

---

## Module 2: Conditionals and Loops

**Goal**: Use conditional branching (`if` / `else`) and loops (`while`).

```ofs
core main() {
    forge energy: stone = 3

    while (energy > 0) {
        echo(energy)
        energy = energy - 1
    }

    if (energy == 0) {
        echo("Recharge required")
    }
}
```

**Challenge**: Write a loop that counts down from 5 to 1 and then prints "Liftoff".

---

## Module 3: Functions (`vein`)

**Goal**: Structure and reuse computational logic using the `vein` keyword.

```ofs
vein double(n: stone) -> stone {
    return n * 2
}

vein add(a: stone, b: stone) -> stone {
    return a + b
}

core main() {
    echo(double(21))
    echo(add(10, 32))
}
```

**Challenge**: Write a function `average(a: stone, b: stone) -> stone` that returns the integer arithmetic mean of two numbers.

---

## Module 4: Fixed-Width Integers

**Goal**: Work with machine-sized integer types (`u8`, `u16`, `u32`, `i8`, `i32`).

```ofs
core main() {
    forge channel_r: u8 = 255
    forge channel_g: u8 = 128
    forge channel_b: u8 = 0

    forge pixel: u32 = (channel_r as u32 << 16) | (channel_g as u32 << 8) | (channel_b as u32)
    echo(pixel)
}
```

**Challenge**: Extract the green channel (`channel_g`) back out from `pixel` using right bit-shifts and masking (`& 0xFF`).

---

## Module 5: Records (`monolith`) and Methods (`impl`)

**Goal**: Model composite structures and attach methods using `monolith` and `impl`.

```ofs
monolith Rectangle {
    width: stone
    height: stone
}

impl Rectangle {
    vein area(self) -> stone {
        return self.width * self.height
    }

    vein perimeter(self) -> stone {
        return (self.width + self.height) * 2
    }
}

core main() {
    forge r: Rectangle
    r.width = 10
    r.height = 20

    echo(r.area())
    echo(r.perimeter())
}
```

**Challenge**: Add an `is_square(self) -> bool` method that returns true when width equals height.

---

## Module 6: Namespaces (`namespace`)

**Goal**: Group functions and constants under scoped identifiers to prevent collisions.

```ofs
namespace math_utils {
    vein square(x: stone) -> stone {
        return x * x
    }

    vein cube(x: stone) -> stone {
        return x * x * x
    }
}

core main() {
    echo(math_utils.square(5))
    echo(math_utils.cube(3))
}
```

---

## Module 7: Modular Code with `attach`

**Goal**: Split code across separate files and import functions.

Helper file `helper.ofs`:
```ofs
vein greet(name: obsidian) -> obsidian {
    return "Hello, " + name + "!"
}
```

Main file `main.ofs`:
```ofs
attach {F:./helper.ofs}

core main() {
    echo(greet("OFS"))
}
```

---

## Module 8: Functions as Values and Closures

**Goal**: Pass functions as arguments to other functions.

```ofs
vein apply(x: stone, op: vein(stone) -> stone) -> stone {
    return op(x)
}

core main() {
    forge triple = vein(n: stone) -> stone {
        return n * 3
    }

    echo(apply(7, triple))
}
```

---

## Module 9: Pattern Matching (`match`) and Constants

**Goal**: Cleanly handle multi-way branching without nested conditionals.

```ofs
const STATUS_OK: stone = 200
const STATUS_NOT_FOUND: stone = 404

core main() {
    forge status = 200

    match status {
        case STATUS_OK: {
            echo("Request succeeded")
        }
        case STATUS_NOT_FOUND: {
            echo("Resource missing")
        }
        default: {
            echo("Unhandled response code")
        }
    }
}
```

---

## Next Steps

1. Read the [Getting Started Guide](../GETTING_STARTED.md)
2. Explore the [Language Reference](../LANGUAGE_REFERENCE.md)
3. Check out the real-world samples in `ofs/examples/`
