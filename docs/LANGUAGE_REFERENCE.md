# Obsidian Fault Script — Referência da Linguagem

Referência prática da linguagem suportada hoje pelo compilador OFS.

---

## Índice

1. Estrutura de programa
2. Tipos
3. Variáveis e constantes
4. Funções, intents e lambdas
5. `monolith`, `impl` e layouts
6. `namespace`
7. Arrays e coleções
8. Controle de fluxo
9. Operadores e casts
10. `attach`
11. `extern vein` e `rift vein`
12. Blocos de baixo nível
13. Biblioteca padrão e pacotes
14. Comandos do compilador

---

## 1. Estrutura de programa

Todo programa executável precisa de um `core main()`:

```ofs
core main() {
    echo("inicio")
}
```

Um arquivo `.ofs` pode conter:

- `attach { ... }`
- `extern vein` e `rift vein`
- `vein` e `core`
- `monolith`
- `impl`
- `namespace`
- `strata`
- `forge` global

---

## 2. Tipos

### Tipos primitivos

| Tipo | Uso | Representação geral |
|---|---|---|
| `stone` | inteiro principal | 64 bits |
| `crystal` | ponto flutuante | 64 bits |
| `obsidian` | string | ponteiro |
| `bool` | booleano | lógico |
| `void` | sem retorno | retorno de função |

### Tipos inteiros pequenos

| Tipo | Sinal | Tamanho |
|---|---|---|
| `u8` | sem sinal | 8 bits |
| `u16` | sem sinal | 16 bits |
| `u32` | sem sinal | 32 bits |
| `u64` | sem sinal | 64 bits |
| `i8` | com sinal | 8 bits |
| `i32` | com sinal | 32 bits |

Exemplo:

```ofs
forge r: u8 = 255
forge g: u8 = 128
forge pixel: u32 = (r as u32 << 16) | (g as u32 << 8)
```

### Tipos compostos

| Tipo | Exemplo |
|---|---|
| `Array<T>` | `Array<stone>` |
| tipo de função | `vein(stone) -> stone` |
| `monolith` | `monolith Player { ... }` |

### Inferência

```ofs
forge n = 42
forge nome = "Ana"
forge ativo = true
forge nums = [1, 2, 3]
```

---

## 3. Variáveis e constantes

### `forge`

```ofs
forge nome = "Rex"
forge idade: stone = 20
forge saldo: crystal = 42.5
forge canal: u8 = 255
```

### `const`

```ofs
const limite: stone = 10
```

### Atribuições comuns

```ofs
x = 10
x += 2
x -= 1
x *= 3
x /= 2
x++
x--
```

---

## 4. Funções, intents e lambdas

### Funções normais

```ofs
vein soma(a: stone, b: stone) -> stone {
    return a + b
}
```

### Entrada do programa

```ofs
core main() {
    echo("ok")
}
```

### Intent

```ofs
vein square(x: stone) -> stone intent pure {
    return x * x
}
```

Intents aceitos:

- `pure`
- `impure`
- `fractal`

### Funções como valores

```ofs
vein aplicar(x: stone, fn: vein(stone) -> stone) -> stone {
    return fn(x)
}
```

### Lambda inline

```ofs
forge dobrar = vein(x: stone) -> stone {
    return x * 2
}
```

---

## 5. `monolith`, `impl` e layouts

### Estrutura

```ofs
monolith Rect {
    w: stone
    h: stone
}
```

### Métodos com `impl`

```ofs
impl Rect {
    vein area(self) -> stone {
        return self.w * self.h
    }
}
```

Uso:

```ofs
forge r: Rect
r.w = 10
r.h = 5
echo(r.area())
```

### Layout explícito

```ofs
monolith Header layout packed {
    tag: stone
    flags: stone
}
```

Layouts suportados:

- `native`
- `packed`
- `c`

---

## 6. `namespace`

Use namespace para agrupar funções e evitar colisão de nomes.

```ofs
namespace mathx {
    vein square(x: stone) -> stone {
        return x * x
    }
}

core main() {
    echo(mathx.square(4))
}
```

---

## 7. Arrays e coleções

```ofs
forge nums = [1, 2, 3]
echo(nums[0])
```

Exemplo com passagem de função:

```ofs
vein map_array(arr: Array<stone>, fn: vein(stone) -> stone) -> Array<stone> {
    forge result: Array<stone> = []
    cycle (n in arr) {
        ofs_array_push(result, fn(n))
    }
    return result
}
```

---

## 8. Controle de fluxo

### `if / else`

```ofs
if (x > 10) {
    echo("alto")
} else {
    echo("baixo")
}
```

### `while`

```ofs
while (energia > 0) {
    energia -= 1
}
```

### `cycle`

Forma C-like:

```ofs
cycle (forge i = 0; i < 10; i++) {
    echo(i)
}
```

Forma iterativa:

```ofs
cycle (n in nums) {
    echo(n)
}
```

### `match`

```ofs
match code {
    case 200: { echo("ok") }
    case 404: { echo("nao encontrado") }
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

### `strata`

```ofs
strata Status { Idle, Running, Failed }
```

---

## 9. Operadores e casts

### Operadores principais

- aritméticos: `+`, `-`, `*`, `/`, `%`
- comparação: `==`, `!=`, `<`, `<=`, `>`, `>=`
- lógicos: `&&`, `||`, `!`
- bitwise: `&`, `|`, `^`, `<<`, `>>`

### Cast com `as`

```ofs
forge r: u8 = 255
forge x: u32 = r as u32
forge y: stone = x as stone
forge z: u8 = y as u8
```

Também funciona em conversões entre inteiro e float:

```ofs
forge c: crystal = 10.5
forge n: u8 = c as u8
```

---

## 10. `attach`

### Biblioteca padrão ou pacote instalado

```ofs
attach {core}
attach {window}
attach {terminal-colors}
```

### Arquivo local

```ofs
attach {F:modulos/minha_lib.ofs}
```

No Windows também funciona com caminho absoluto:

```ofs
attach {F:G:\projeto\minha_lib.ofs}
```

O `attach` embute o código OFS do outro arquivo no processo de compilação. Ele não carrega DLL nativa.

---

## 11. `extern vein` e `rift vein`

### `extern vein`

```ofs
extern vein puts(s: obsidian) -> stone
```

### `rift vein`

```ofs
rift vein ofs_window_create(title: obsidian, w: stone, h: stone) -> void abi c
```

### Metadados de link

```ofs
rift vein text_size(s: obsidian) -> stone bind "strlen" abi c
```

Campos importantes:

- `bind "..."` define o símbolo nativo real
- `abi c` define ABI C
- `...` é aceito em função variádica nativa

---

## 12. Blocos de baixo nível

### `fracture`

Camada de baixo nível mais tipada.

### `abyss`

Camada crua e irrestrita.

### `fractal`

Camada especial ligada a intent/efeitos.

### `bedrock`

Vocabulário OFS para memória, regiões, lanes, packets e intrinsics `fault_*`.

Exemplo:

```ofs
attach {bedrock}

core main() {
    forge region = bedrock_region_new(4)
    bedrock_region_write(region, 0, 10)
    bedrock_region_write(region, 1, 20)
    echo(bedrock_region_read(region, 1))
    bedrock_region_drop(region)
}
```

### Alias `tectonic`

O parser aceita aliases textuais como:

- `tectonic fracture`
- `tectonic abyss`
- `tectonic fractal`
- `tectonic safe`
- `tectonic unsafe`
- `tectonic bedrock`

---

## 13. Biblioteca padrão e pacotes

Módulos e pacotes relevantes já presentes no projeto:

- `canvas`
- `core`
- `fmt`
- `io`
- `math`
- `string`
- `rift`
- `bedrock`
- `bedrock-packet`
- `memory-modes`
- `terminal-colors`
- `test-lib`
- `webserver`
- `window`

Módulo `window` expõe:

- `window.create`
- `window.destroy`
- `window.present`
- `window.poll`
- `window.is_open`
- `window.width`
- `window.height`
- `window.set_title`
- `input.mouse_x`
- `input.mouse_y`
- `input.is_down`
- `input.key`

Módulo `canvas` expõe:

- `canvas.create`
- `canvas.destroy`
- `canvas.clear`
- `canvas.set_pixel`
- `canvas.get_pixel`
- `canvas.fill_rect`
- `canvas.present`

---

## 14. Comandos do compilador

### Compilador principal (`ofs`)

```bash
ofs run arquivo.ofs
ofs build arquivo.ofs -o app
ofs check arquivo.ofs
ofs tokens arquivo.ofs
ofs ast arquivo.ofs
ofs ir arquivo.ofs
ofs asm arquivo.ofs
```

### Compilador self-hosted (`ofscc`) no estado atual

`ofscc` já suporta otimização e modos de inspeção via bridge de ambiente:

Variáveis:

- `OFSCC_INPUT`
- `OFSCC_OUTPUT`
- `OFSCC_C_OUT`
- `OFSCC_OPT` (`-O0`, `-O2`, `-O3`)
- `OFSCC_MODE` (`check`, `tokens`, `ast`)

Exemplo:

```bash
export OFSCC_INPUT=programa.ofs
export OFSCC_OUTPUT=programa
export OFSCC_OPT=-O3
./ofscc_v1
```

```bash
ofs arquivo.ofs
ofs run arquivo.ofs
ofs build arquivo.ofs -o app
ofs check arquivo.ofs
ofs tokens arquivo.ofs
ofs ast arquivo.ofs
ofs ir arquivo.ofs
ofs asm arquivo.ofs
ofs version
ofs update
```

Pacotes:

```bash
uncover fmt
infuse fmt
infuse {fmt:stable}
reinfuse fmt
```

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

Use `attach` to bring stdlib modules, packages, or local files into scope.

### Library attach (stdlib / package)

```ofs
attach {core}
attach {math}
attach {webserver}
attach {bedrock}
attach {terminal-colors}
attach {memory-modes}
```

The name inside `{}` maps to a known stdlib module or an installed package. The compiler resolves the file automatically from the stdlib directory, `OFS_STDLIB_PATH`, or `OFS_LIB_PATH`.

### File attach

```ofs
attach {F:helpers.ofs}
attach {F:../shared/utils.ofs}
```

Use the `F:` prefix to reference a specific `.ofs` file by path relative to the current file.

Attach declarations must appear at the top level, before or between other declarations. The preprocessor resolves them before the lexer and parser run, so each module is inlined exactly once (deduplication is automatic).

### Stdlib modules

| Name | Description |
|------|-------------|
| `core` | Math helpers, predicates, factorial, fibonacci |
| `math` | Primes, digit utilities, reverse numbers |
| `string` | String utilities: repeat, starts_with, is_empty |
| `io` | prompt, print_separator, print_header |
| `webserver` | HTTP response builder, MIME types, log helpers |
| `bedrock` | Low-level heap cells, regions, bit-field helpers |
| `bedrock-packet` | Packet layout helpers built on bedrock regions |
| `rift` | Foreign runtime wrappers (C ABI helpers) |
| `terminal-colors` | ANSI terminal color output |
| `memory-modes` | Docs and helpers for fracture/abyss/fractal modes |
| `test-lib` | Unit test utilities |

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

### rift vein

OFS now also supports `rift vein` as a native interop-facing declaration form:

```ofs
rift vein strlen(text: obsidian) -> stone

core main() {
    echo(strlen("fault"))
}
```

`rift vein` lowers to the same foreign-call pipeline as `extern vein`, but it presents interop as an OFS-native boundary instead of framing the feature only as "extern C".

You can also add explicit interop metadata:

```ofs
rift vein text_size(text: obsidian) -> stone bind "strlen" abi c
```

- `bind "symbol"` sets the external link symbol name.
- `abi c` declares the intended ABI.
- `abi system` is reserved for platform/system-boundary calls.

### Interop Direction

`extern vein` is the current FFI surface. Today it is best suited for runtime and C-compatible symbols.

The roadmap extends this toward richer native interop layers for:

- C object and library boundaries,
- C++ bridge layers,
- C# host/runtime bridges,
- Python embedding or host bindings,
- target-specific assembly escape hatches.

These are roadmap items, not all implemented features today.

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

You can also declare an explicit layout mode:

```ofs
monolith Header layout packed {
    tag: stone
    flags: stone
}
```

Supported layout modes today:

- `layout native`
- `layout packed`
- `layout c`

`layout packed` lowers to a packed LLVM struct layout. `layout c` is accepted as an explicit ABI-facing intent and currently lowers like `native` until the full ABI/layout subsystem is expanded.

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

Pointer operations are only allowed inside `fracture` (safe), `bedrock` (typed low-level), or `abyss` (unsafe) blocks.

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

### bedrock (Typed Low-Level Block)

`bedrock` is the OFS-native low-level block for typed machine-adjacent work.

```ofs
core main() {
    forge x: stone = 10

    bedrock {
        shard p: *stone = &x
        *p = *p + fault_spin_left(1, 3)
    }

    echo(x)
}
```

Use `bedrock` when you want low-level control and machine-like intrinsics while still keeping typed OFS rules.

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

### Directive alias: tectonic

You can use `tectonic` as a prefix directive for memory/effect modes.
This is an alias syntax that maps to the same runtime/semantic behavior:

- `tectonic fracture { ... }` -> same as `fracture { ... }`
- `tectonic safe { ... }` -> same as `fracture { ... }`
- `tectonic abyss { ... }` -> same as `abyss { ... }`
- `tectonic unsafe { ... }` -> same as `abyss { ... }`
- `tectonic fractal { ... }` -> same as `fractal { ... }`
- `tectonic bedrock { ... }` -> same as `bedrock { ... }`

Example:

```ofs
core main() {
    forge x: stone = 10

    tectonic fracture {
        shard p: *stone = &x
        *p = 20
    }

    tectonic safe {
        shard q: *stone = &x
        *q = *q + 1
    }

    tectonic abyss {
        shard raw: *stone = &x
        *raw = *raw + 1
    }

    tectonic unsafe {
        shard raw2: *stone = &x
        *raw2 = *raw2 + 1
    }

    tectonic fractal {
        echo("effect-lifted mode")
    }

    tectonic bedrock {
        shard native: *stone = &x
        *native = *native + 1
    }
}
```

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

### fault_* intrinsics

Inside `bedrock`, `fracture`, or `abyss`, OFS exposes machine-like intrinsics with OFS-native names:

```ofs
bedrock {
    forge x: stone = 0xF0F0
    echo(fault_count(x))
    echo(fault_lead(x))
    echo(fault_trail(x))
    echo(fault_swap(x))
    echo(fault_spin_left(x, 4))
    echo(fault_spin_right(x, 4))
    echo(fault_weave(0xFF00, 0xAAAA, 0x5555))
}
```

Available intrinsics:

- `fault_count(stone)` -> population count
- `fault_fence()` -> emit a machine-level memory barrier
- `fault_prefetch(*stone)` -> request a machine-level prefetch for a pointer target
- `fault_trap()` -> emit a machine-level trap
- `fault_lead(stone)` -> count leading zeros
- `fault_trail(stone)` -> count trailing zeros
- `fault_swap(stone)` -> byte swap
- `fault_spin_left(stone, stone)` -> rotate left
- `fault_spin_right(stone, stone)` -> rotate right
- `fault_step(*stone, stone)` -> typed pointer stepping in elements
- `fault_cut(value, shift, width)` -> extract a bit field
- `fault_patch(base, shift, width, insert)` -> replace a bit field
- `fault_weave(mask, left, right)` -> blend bits from `left` and `right` using `mask`
- `fault_unreachable()` -> marks the current path as unreachable, lowers to `llvm.trap` + `unreachable` IR
- `fault_memcpy(dst, src, len)` -> bulk memory copy (lowers to `llvm.memcpy`); `dst` and `src` must be pointers
- `fault_memset(dst, val, len)` -> bulk memory fill with `val` byte (lowers to `llvm.memset`); `dst` must be a pointer

These functions are designed to cover operations assembly already has, while also giving OFS room for its own machine-oriented vocabulary.

### asm (inline assembly escape hatch)

When no `fault_*` intrinsic covers your need, `asm` emits a raw assembly string directly at that point in the code. It can only appear inside a `bedrock`, `fracture`, or `abyss` block.

```ofs
bedrock {
    asm "nop"
    asm "int3"
}
```

**Restrictions:**
- The argument must be a string literal.
- Input/output operand syntax (full AT&T-style constraints) is not yet parseable; use `fault_*` intrinsics when operand binding is needed.
- Generates a void inline-asm call in LLVM IR with the assembly string forwarded verbatim.
- The `asm` expression is lowered to LLVM `InlineAsm`; the compiler correctly dispatches it in both semantic analysis and code generation.

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

The standard library is split between application-facing helpers and emerging low-level OFS-native helpers.

Examples:

- `terminal_colors.ofs` for terminal presentation
- `webserver.ofs` for HTTP helpers
- `memory_modes.ofs` for memory-mode descriptions
- `bedrock.ofs` for explicit heap-backed state helpers in OFS-native style
- `rift.ofs` for OFS-native foreign-boundary wrappers
- `bedrock_packet.ofs` for packet/header views built on low-level regions

### Bedrock Module

`bedrock.ofs` is the current seed for OFS-native low-level programming.

It provides helpers such as:

- `bedrock_cell_new(initial)`
- `bedrock_cell_read(cell)`
- `bedrock_cell_write(cell, value)`
- `bedrock_cell_add(cell, delta)`
- `bedrock_cell_sub(cell, delta)`
- `bedrock_cell_drop(cell)`
- `bedrock_region_new(len)`
- `bedrock_region_read(region, index)`
- `bedrock_region_write(region, index, value)`
- `bedrock_region_add(region, index, delta)`
- `bedrock_prefetch(region, index)`
- `bedrock_region_drop(region)`
- `bedrock_view_at(region, start, offset)`
- `bedrock_view_read(region, start, offset)`
- `bedrock_view_write(region, start, offset, value)`
- `bedrock_field_cut(value, shift, width)`
- `bedrock_field_patch(base, shift, width, insert)`
- `bedrock_lane8_get(value, lane)`
- `bedrock_lane8_set(value, lane, insert)`
- `bedrock_lane16_le_get(value, lane)`
- `bedrock_lane16_be_get(value, lane)`
- `bedrock_lane16_le_set(value, lane, insert)`
- `bedrock_lane16_be_set(value, lane, insert)`
- `bedrock_lane32_le_get(value, lane)`
- `bedrock_lane32_be_get(value, lane)`
- `bedrock_lane32_le_set(value, lane, insert)`
- `bedrock_lane32_be_set(value, lane, insert)`
- `bedrock_barrier()`

The purpose of this module is not to imitate C APIs directly, but to let low-level programming grow in OFS vocabulary.

These lane helpers also give OFS an endianness-aware surface for ABI-facing bit windows without forcing the user to drop into raw shifts everywhere.

### Rift Module

`rift.ofs` is the first standard-library interop surface built on top of `rift vein`.

It provides wrappers such as:

- `rift_text_size(text)`
- `rift_line(text)`
- `rift_banner(title)`
- `rift_report_line(label, value)`

The goal is to let foreign boundaries be consumed through OFS-facing APIs instead of exposing only raw declarations everywhere.

### Bedrock Packet Module

`bedrock_packet.ofs` is the first structured low-level object-style module built on top of bedrock regions.

It provides helpers such as:

- `bedrock_packet_store(region, slot, header, payload)`
- `bedrock_packet_header(region, slot)`
- `bedrock_packet_payload(region, slot)`
- `bedrock_packet_opcode(header)`
- `bedrock_packet_lane(header)`
- `bedrock_packet_opcode_be16(header)`
- `bedrock_packet_tail_le16(header)`
- `bedrock_packet_set_opcode(header, opcode)`
- `bedrock_packet_set_lane(header, lane)`

This is part of the direction where OFS low-level programming gains its own reusable objects/modules instead of exposing only raw operations.

### Hybrid Example Pattern

It is valid and encouraged to mix high-level and low-level OFS in the same file:

- business logic in regular `vein` functions,
- foreign interop at the boundary through `rift vein`,
- machine-adjacent work in `bedrock`,
- explicit storage helpers from `bedrock.ofs`.

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
| `rift`     | `KW_RIFT`        | Native interop declaration |
| `as`       | `KW_AS`          | Type cast                  |
| `match`    | `KW_MATCH`       | Pattern matching           |
| `case`     | `KW_CASE`        | Match arm                  |
| `default`  | `KW_DEFAULT`     | Match fallback arm         |
| `const`    | `KW_CONST`       | Immutable variable         |
| `strata`   | `KW_STRATA`      | Enum declaration           |
| `tremor`   | `KW_TREMOR`      | Error handling block       |
| `catch`    | `KW_CATCH`       | Error handler              |
| `throw`    | `KW_THROW`       | Raise error value          |
| `tectonic` | `KW_TECTONIC`    | Mode directive prefix      |
| `bedrock`  | `KW_BEDROCK`     | Typed low-level block      |
| `asm`      | `KW_ASM`         | Inline assembly escape hatch |
