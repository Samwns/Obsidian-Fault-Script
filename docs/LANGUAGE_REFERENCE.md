# Obsidian Fault Script — Language Reference

> Complete syntax and semantics reference for OFS v1.0

---

## Table of Contents

1. [Program Structure](#program-structure)
2. [Types](#types)
3. [Variables (forge)](#variables-forge)
4. [Functions (vein / core)](#functions-vein--core)
5. [Expressions](#expressions)
6. [Operators](#operators)
7. [Control Flow](#control-flow)
8. [Structures (monolith)](#structures-monolith)
9. [Pointers (shard / fracture)](#pointers-shard--fracture)
10. [Collections (Arrays)](#collections-arrays)
11. [Unsafe Memory (abyss)](#unsafe-memory-abyss)
12. [Type Casting (as)](#type-casting-as)
13. [Imports](#imports)
14. [Extern Functions](#extern-functions)
15. [Built-in Functions](#built-in-functions)
16. [Comments](#comments)
17. [Statement Termination](#statement-termination)
18. [Standard Library](#standard-library)
19. [Keywords](#keywords)

---

## Program Structure

Every OFS program requires a `core main()` function as its entry point:

```ofs
core main() {
    // program starts here
}
```

A program file (`.ofs`) can contain:
- **Attach declarations** (`attach`)
- **Extern function declarations** (`extern vein`)
- **Function declarations** (`vein` or `core`)
- **Structure declarations** (`monolith`)
- **Global variable declarations** (`forge` at top level)

Declarations can appear in any order. The compiler resolves forward references.

---

## Types

OFS is **statically typed** with type inference. Every value has a known type at compile time.

### Primitive Types

| Type        | Description            | Size     | LLVM   | Default Value |
|-------------|------------------------|----------|--------|---------------|
| `stone`     | Signed integer         | 64 bits  | `i64`  | `0`           |
| `crystal`   | Floating point         | 64 bits  | `f64`  | `0.0`         |
| `obsidian`  | String (UTF-8)         | pointer  | `i8*`  | `""`          |
| `bool`      | Boolean                | 1 bit    | `i1`   | `false`       |
| `void`      | No value (return only) | 0 bits   | `void` | —             |

### Compound Types

| Type          | Description              | Example               |
|---------------|--------------------------|-----------------------|
| `shard <T>`   | Pointer to type T        | `shard p: *stone`     |
| `Array<T>`    | Dynamic array of type T  | `[1, 2, 3]`          |
| `monolith`    | User-defined structure   | `monolith Player { }` |

### Type Inference

When you declare a variable without a type annotation, OFS infers the type from the initializer:

```ofs
forge x = 42        // inferred as stone
forge y = 3.14      // inferred as crystal
forge s = "hello"   // inferred as obsidian
forge b = true      // inferred as bool
forge a = [1, 2, 3] // inferred as Array<stone>
```

### Type Promotion

When mixing `stone` and `crystal` in arithmetic, `stone` is promoted to `crystal`:

```ofs
forge x: stone = 10
forge y: crystal = 3.14
forge z = x + y   // z is crystal (10.0 + 3.14 = 13.14)
```

---

## Variables (forge)

Variables are declared with the `forge` keyword:

```ofs
// With type inference
forge name = "Rex"
forge count = 42

// With explicit type annotation
forge name: obsidian = "Rex"
forge count: stone = 42
forge pi: crystal = 3.14

// Without initializer (default-initialized)
forge x: stone       // x = 0
```

### Mutability

All variables are mutable by default:

```ofs
forge x = 10
x = 20        // OK
x += 5        // OK — compound assignment
```

### Assignment Operators

| Operator | Description       | Example     |
|----------|-------------------|-------------|
| `=`      | Assignment        | `x = 10`    |
| `+=`     | Add-assign        | `x += 5`    |
| `-=`     | Subtract-assign   | `x -= 3`    |
| `*=`     | Multiply-assign   | `x *= 2`    |
| `/=`     | Divide-assign     | `x /= 4`    |

### Increment / Decrement

```ofs
x++     // post-increment
x--     // post-decrement
++x     // pre-increment
--x     // pre-decrement
```

---

## Functions (vein / core)

### Regular Functions (vein)

```ofs
vein add(a: stone, b: stone) -> stone {
    return a + b
}

vein greet(name: obsidian) -> void {
    echo("Hello, " + name)
}
```

**Syntax:**
```
vein <name>(<params>) -> <return_type> {
    <body>
}
```

Parameters: `name: type` separated by commas.

Return type: specified after `->`. Use `void` for no return value.

### Entry Point (core)

The `core` keyword marks the program entry point. Every program needs exactly one:

```ofs
core main() {
    echo("Program starts here")
}
```

`core` functions are implicitly `-> void` (return type is void by default).

### Function Calls

```ofs
forge result = add(10, 20)
greet("World")
```

### Recursion

Functions can call themselves:

```ofs
vein factorial(n: stone) -> stone {
    if (n <= 1) {
        return 1
    }
    return n * factorial(n - 1)
}
```

---

## Expressions

### Literals

```ofs
42              // stone (integer)
3.14            // crystal (float)
"hello world"  // obsidian (string)
true            // bool
false           // bool
null            // null
```

### String Escape Sequences

| Escape | Character     |
|--------|---------------|
| `\n`   | Newline       |
| `\t`   | Tab           |
| `\\`   | Backslash     |
| `\"`   | Double quote  |

### Identifiers

Identifiers start with a letter or underscore, followed by letters, digits, or underscores:

```
[a-zA-Z_][a-zA-Z0-9_]*
```

---

## Operators

### Arithmetic Operators

| Operator | Description    | Types              | Result    |
|----------|----------------|--------------------|-----------|
| `+`      | Addition       | stone, crystal     | numeric   |
| `-`      | Subtraction    | stone, crystal     | numeric   |
| `*`      | Multiplication | stone, crystal     | numeric   |
| `/`      | Division       | stone, crystal     | numeric   |
| `%`      | Modulo         | stone              | stone     |
| `<<`     | Shift left     | stone              | stone     |
| `>>`     | Shift right    | stone              | stone     |
| `+`      | Concatenation  | obsidian           | obsidian  |

### Bitwise Operators

| Operator | Description     | Types | Result |
|----------|-----------------|-------|--------|
| `&`      | Bitwise AND     | stone | stone  |
| `|`      | Bitwise OR      | stone | stone  |
| `^`      | Bitwise XOR     | stone | stone  |

### Comparison Operators

| Operator | Description       | Result |
|----------|-------------------|--------|
| `==`     | Equal             | bool   |
| `!=`     | Not equal         | bool   |
| `<`      | Less than         | bool   |
| `<=`     | Less or equal     | bool   |
| `>`      | Greater than      | bool   |
| `>=`     | Greater or equal  | bool   |

### Logical Operators

| Operator | Description | Types | Result |
|----------|-------------|-------|--------|
| `&&`     | Logical AND | bool  | bool   |
| `\|\|`   | Logical OR  | bool  | bool   |
| `!`      | Logical NOT | bool  | bool   |

### Unary Operators

| Operator | Description    | Example    |
|----------|----------------|------------|
| `-`      | Negation       | `-x`       |
| `!`      | Logical NOT    | `!done`    |
| `&`      | Address-of     | `&x`       |
| `*`      | Dereference    | `*ptr`     |
| `++`     | Increment      | `x++`/`++x` |
| `--`     | Decrement      | `x--`/`--x` |

### Operator Precedence (lowest to highest)

| Level | Operators           | Associativity |
|-------|---------------------|---------------|
| 1     | `\|\|`              | Left          |
| 2     | `&&`                | Left          |
| 3     | `==` `!=`           | Left          |
| 4     | `<` `<=` `>` `>=`   | Left          |
| 5     | `|`                 | Left          |
| 6     | `^`                 | Left          |
| 7     | `&`                 | Left          |
| 8     | `<<` `>>`           | Left          |
| 9     | `+` `-`             | Left          |
| 10    | `*` `/` `%`         | Left          |
| 11    | Unary: `-` `!` `&` `*` `++` `--` `~` | Right |

---

## Control Flow

### if / else

```ofs
if (condition) {
    // then block
}

if (condition) {
    // then block
} else {
    // else block
}

if (x > 100) {
    echo("big")
} else if (x > 50) {
    echo("medium")
} else {
    echo("small")
}
```

### cycle (C-style loop)

```ofs
cycle (forge i = 0; i < 10; i++) {
    echo(i)
}
```

**Syntax:**
```
cycle (<init>; <condition>; <step>) {
    <body>
}
```

### cycle (range-based loop)

```ofs
forge nums = [1, 2, 3, 4, 5]
cycle (n in nums) {
    echo(n)
}
```

**Syntax:**
```
cycle (<variable> in <collection>) {
    <body>
}
```

### break / continue

```ofs
cycle (forge i = 0; i < 100; i++) {
    if (i == 50) {
        break       // exit loop
    }
    if (i % 2 == 0) {
        continue    // skip even numbers
    }
    echo(i)
}
```

### while (condition-only loop)

```ofs
forge count: stone = 10
while (count > 0) {
    echo(count)
    count -= 1
}
```

**Syntax:**
```
while (<condition>) {
    <body>
}
```

`while` is syntactic sugar for a `cycle` with only a condition. `break` and `continue` work inside `while` blocks.

---

## Type Casting (as)

Use the `as` keyword for explicit type conversions:

```ofs
forge x: stone = 42
forge y: crystal = x as crystal    // integer to float

forge pi: crystal = 3.14
forge rounded: stone = pi as stone // float to integer (truncates)

forge flag: bool = true
forge num: stone = flag as stone   // bool to integer (0 or 1)
```

### Allowed Casts

| From      | To        | Result                  |
|-----------|-----------|-------------------------|
| `stone`   | `crystal` | Integer to float        |
| `crystal` | `stone`   | Float to integer (truncates) |
| `bool`    | `stone`   | `true` → 1, `false` → 0 |
| `stone`   | `bool`    | 0 → `false`, non-zero → `true` |
| `bool`    | `crystal` | `true` → 1.0, `false` → 0.0 |
| `crystal` | `bool`    | 0.0 → `false`, non-zero → `true` |

Note: String (`obsidian`) casts require runtime functions (`ofs_stone_to_obsidian`, etc.).

---

## Imports

Attach other OFS source files:

```ofs
attach "stdlib/core.ofs"
attach "stdlib/math.ofs"
```

Imports must appear at the top level, before or between other declarations.

---

## Extern Functions

Declare external C functions for FFI (Foreign Function Interface):

```ofs
extern vein ofs_pow(base: crystal, exp: crystal) -> crystal
extern vein ofs_sqrt(val: crystal) -> crystal
extern vein ofs_str_len(s: obsidian) -> stone

core main() {
    forge result = ofs_pow(2.0, 10.0)
    echo(result)  // prints 1024
}
```

All OFS runtime functions can be accessed via `extern` declarations. This enables low-level and high-level programming in the same language.

---

## Structures (monolith)

Structures are defined with the `monolith` keyword:

```ofs
monolith Player {
    name: obsidian
    hp:   stone
    speed: crystal
}
```

### Creating Instances

```ofs
forge hero: Player
hero.name = "Obsidian Knight"
hero.hp = 100
hero.speed = 1.5
```

### Field Access

Use dot notation to access fields:

```ofs
echo(hero.name)     // "Obsidian Knight"
echo(hero.hp)       // 100
hero.hp -= 10       // take damage
```

### Passing to Functions

```ofs
vein greet(p: Player) -> void {
    echo(p.name)
    echo(p.hp)
}

core main() {
    forge hero: Player
    hero.name = "Knight"
    hero.hp = 100
    hero.speed = 1.5
    greet(hero)
}
```

---

## Pointers (shard / fracture)

Pointer operations are only allowed inside `fracture` (safe) or `abyss` (unsafe) blocks.

### fracture (Safe Pointer Block)

```ofs
core main() {
    forge x: stone = 10

    fracture {
        shard p: *stone = &x   // p points to x
        *p = 42                // dereference and assign
    }

    echo(x)  // prints 42
}
```

**Key rules:**
- Pointers can only be created and used inside `fracture` blocks
- The compiler type-checks all pointer operations
- `&x` takes the address of `x`
- `*p` dereferences pointer `p`

### Pointer Types

```ofs
shard p: *stone       // pointer to stone (i64*)
shard q: *crystal     // pointer to crystal (f64*)
shard s: *obsidian    // pointer to obsidian (i8**)
```

---

## Collections (Arrays)

### Array Literals

```ofs
forge nums = [10, 20, 30, 40, 50]
forge names = ["Alice", "Bob", "Charlie"]
```

All elements must be the same type. The array type is inferred from the elements.

### Indexing

```ofs
forge first = nums[0]    // 10
forge last = nums[4]     // 50
nums[2] = 99             // modify element
```

Array indexing is **zero-based** with **bounds checking** at runtime.

### Iteration

```ofs
// Range-based
cycle (n in nums) {
    echo(n)
}

// Index-based
cycle (forge i = 0; i < 5; i++) {
    echo(nums[i])
}
```

---

## Unsafe Memory (abyss)

The `abyss` block allows raw, unsafe memory operations:

```ofs
abyss {
    // Raw memory access — no type checking
    // Use with extreme caution
}
```

**Rules:**
- Type checking is relaxed inside `abyss` blocks
- Direct memory addressing is allowed
- Only use when absolutely necessary
- Prefer `fracture` for pointer operations

---

## Built-in Functions

### echo

Polymorphic print function — works with any type:

```ofs
echo(42)              // prints: 42
echo(3.14)            // prints: 3.14
echo("hello")         // prints: hello
echo(true)            // prints: true
```

`echo` automatically selects the correct runtime print function based on the argument type.

---

## Comments

```ofs
// Single-line comment

/* Multi-line
   comment */

forge x = 42  // inline comment
```

---

## Statement Termination

Statements are terminated by **newline** or **semicolon**. Both are valid:

```ofs
forge x = 10
forge y = 20

// Or with semicolons
forge x = 10; forge y = 20
```

Newlines inside `()`, `[]`, and `{}` are ignored (implicit line continuation):

```ofs
forge result = add(
    10,
    20
)
```

---

## Standard Library

OFS includes a standard library written in OFS itself (`stdlib/core.ofs`):

### Math Functions

```ofs
vein abs(x: stone) -> stone         // absolute value
vein max(a: stone, b: stone) -> stone  // maximum
vein min(a: stone, b: stone) -> stone  // minimum
vein clamp(val: stone, lo: stone, hi: stone) -> stone  // clamp to range
```

### Runtime Functions (built-in)

These are implemented in C and linked automatically:

| Function               | Description              |
|------------------------|--------------------------|
| `echo_stone(i64)`      | Print integer            |
| `echo_crystal(f64)`    | Print float              |
| `echo_obsidian(i8*)`   | Print string             |
| `echo_bool(i1)`        | Print boolean            |
| `ofs_alloc(i64)`       | Allocate memory          |
| `ofs_free(ptr)`        | Free memory              |
| `ofs_array_new()`      | Create dynamic array     |
| `ofs_array_push()`     | Push element to array    |
| `ofs_array_get()`      | Get element (with bounds check) |
| `ofs_array_set()`      | Set element at index     |
| `ofs_array_pop()`      | Remove and return last element |
| `ofs_array_len()`      | Get array length         |
| `ofs_str_concat()`     | Concatenate strings      |
| `ofs_str_eq()`         | Compare strings          |
| `ofs_str_len()`        | Get string length        |
| `ofs_str_char_at()`    | Get character at index   |
| `ofs_str_substr()`     | Get substring            |
| `ofs_str_contains()`   | Check if string contains another |
| `ofs_stone_to_obsidian()` | Integer to string     |
| `ofs_crystal_to_obsidian()` | Float to string     |
| `ofs_obsidian_to_stone()` | String to integer     |
| `ofs_obsidian_to_crystal()` | String to float     |
| `ofs_pow()`            | Power function           |
| `ofs_sqrt()`           | Square root              |
| `ofs_mod()`            | Positive modulo          |
| `ofs_read_line()`      | Read line from stdin     |

---

## Keywords

Complete list of reserved keywords:

| Keyword    | Token            | Purpose                    |
|------------|------------------|----------------------------|
| `core`     | `KW_CORE`        | Entry point function       |
| `vein`     | `KW_VEIN`        | Function definition        |
| `forge`    | `KW_FORGE`       | Variable declaration       |
| `monolith` | `KW_MONOLITH`   | Struct definition          |
| `fracture` | `KW_FRACTURE`   | Safe pointer block         |
| `abyss`    | `KW_ABYSS`      | Unsafe memory block        |
| `cycle`    | `KW_CYCLE`       | Loop                       |
| `if`       | `KW_IF`          | Conditional                |
| `else`     | `KW_ELSE`        | Else branch                |
| `return`   | `KW_RETURN`      | Return from function       |
| `echo`     | `KW_ECHO`        | Print (built-in)           |
| `in`       | `KW_IN`          | Range-based cycle          |
| `shard`    | `KW_SHARD`       | Pointer type               |
| `true`     | `KW_TRUE`        | Boolean true               |
| `false`    | `KW_FALSE`       | Boolean false              |
| `null`     | `KW_NULL`        | Null value                 |
| `break`    | `KW_BREAK`       | Exit loop                  |
| `continue` | `KW_CONTINUE`    | Skip iteration             |
| `stone`    | `KW_STONE`       | Integer type (i64)         |
| `crystal`  | `KW_CRYSTAL`     | Float type (f64)           |
| `obsidian` | `KW_OBSIDIAN`    | String type (i8*)          |
| `bool`     | `KW_BOOL`        | Boolean type (i1)          |
| `void`     | `KW_VOID`        | Void type                  |
| `attach`   | `KW_IMPORT`      | Attach module              |
| `while`    | `KW_WHILE`       | Condition-only loop        |
| `extern`   | `KW_EXTERN`      | Foreign function           |
| `as`       | `KW_AS`          | Type cast                  |
| `match`    | `KW_MATCH`       | Pattern matching           |
| `case`     | `KW_CASE`        | Match arm                  |
| `default`  | `KW_DEFAULT`     | Match fallback arm         |
| `const`    | `KW_CONST`       | Immutable variable         |
| `strata`   | `KW_STRATA`      | Enum declaration           |
| `tremor`   | `KW_TREMOR`      | Error handling block       |
| `catch`    | `KW_CATCH`       | Error handler              |
| `throw`    | `KW_THROW`       | Raise error value          |
