# Obsidian Fault Script — Language Reference

Practical reference for the language currently supported by the OFS compiler.

---

## Table of Contents

1. Program structure
2. Types
3. Variables and constants
4. Functions and intents
5. `monolith`, `impl` and layouts
6. `namespace`
7. Arrays and collections
8. Control flow
9. Operators and casts
10. `attach`
11. `extern vein` and `rift vein`
12. Low-level blocks
13. Standard library and packages
14. Compiler commands

---

## 1. Program structure

Every executable program needs a `core main()`:

```ofs
core main() {
    echo("inicio")
}
```

`core` is a reserved word: it only marks the entry point. Using it as a name (`vein core()`, `core(1)`, also inside a `namespace`) is a **compile error** ("`core` e palavra reservada…"); rename the function (e.g. `core_fn`). Any syntax error stops compilation with exit code 1.

A `.ofs` file can contain:

- `attach { ... }`
- `extern vein` and `rift vein`
- `vein` and `core`
- `monolith`
- `impl`
- `namespace`
- `strata`
- global `forge`

---

## 2. Types

### Primitive types

| Type | Use | General representation |
|---|---|---|
| `stone` | main integer | 64 bits |
| `crystal` | floating point | 64 bits |
| `obsidian` | string | pointer |
| `bool` | boolean | logical |
| `void` | no return value | function return |

### Small integer types

| Type | Sign | Size |
|---|---|---|
| `u8` | unsigned | 8 bits |
| `u16` | unsigned | 16 bits |
| `u32` | unsigned | 32 bits |
| `u64` | unsigned | 64 bits |
| `i8` | signed | 8 bits |
| `i32` | signed | 32 bits |
| `i16` | signed | 16 bits |
| `i64` | signed | 64 bits |

Example:

```ofs
forge r: u8 = 255
forge g: u8 = 128
forge pixel: u32 = (r as u32 << 16) | (g as u32 << 8)
```

### Composite types

| Type | Example |
|---|---|
| `Array<T>` | `Array<stone>` |
| function type | `vein(stone) -> stone` |
| `monolith` | `monolith Player { ... }` |

### Inference

```ofs
forge n = 42
forge nome = "Ana"
forge ativo = true
forge nums = [1, 2, 3]
```

### Implicit conversions and operations (single rule)

The type checker (`typeck`) computes the final type of every expression and writes the implicit conversions into the program tree itself. Native code (`ofs build`) and the interpreter (`ofs run`) only execute those conversions, so the same program gives the same result in both modes.

**Type of each expression**

| Expression | Type |
|---|---|
| `a[i]` with `a: Array<T>` | `T` (no declared `T`: `stone`) |
| `p.field` | the field's declared type |
| `f(...)`, `x.method(...)` | the `vein`'s declared return type |
| `ofs_array_get(a, i)` / `ofs_array_pop(a)` | the element of `Array<T>` |
| `cycle (x in a)` with scalar `a: Array<T>` | `x: T` |
| `a op b` integer with integer | `stone` |

**Implicit conversions (accepted)**

| Where | From → to | How |
|---|---|---|
| mixed arithmetic and comparison | integer → `crystal` | `c + 1` means `c + 1.0`; `2 * c` is `crystal` |
| integer arithmetic | `u8`…`i64` → `stone` | computed in 64 bits (`iN` sign-extends, `uN` zero-extends), result is `stone` |
| `forge x: T = v`, `x = v`, argument, `return`, `Array<T>` element | integer → `crystal` | `forge c: crystal = 1` is `1.0` |
| same | `crystal` → integer | truncates toward zero, like `as` |
| same | `bool` → integer | `true` → 1, `false` → 0 |
| same | integer → integer of another width | truncates or extends on store (`forge u: u8 = 300` is 44) |
| comparison with a number | `bool`/variant → `stone` | `ok == 1`; `Blue == 2` compares the tag |
| `stone == null` | `null` → 0 | address held in a `stone` |
| `"text" + v` | `stone`, `crystal`, `bool`, variant → `obsidian` | `"c=" + 1.5` gives `c=1.5`; `"ok=" + true` gives `ok=true`; a variant gives its name |
| `x op= v` | same as `x = x op v` | `c += 1` with `c: crystal` adds `1.0` |

**Meaningless combinations: compile error**

Reported as `[ERRO DE TIPO file:line:column]` with a hint, in both modes, before anything runs:

| Example | Why |
|---|---|
| `p + 1`, `p * 2` with `p: *u8` | pointer arithmetic: use `fault_step(p, n)` or `(p as stone) + n` |
| `s + b` with `b: bool` | `bool` is not a number: `(b as stone)` |
| `name * c`, `name - 1` with `name: obsidian` | `obsidian` only accepts `+` (concatenation) |
| `c % 2` with `c: crystal` | `%` is integer-only |
| `c & 1`, `b \| b` | bits only exist on integers (for `bool` use `&&`/`\|\|`) |
| `"a" < "b"`, `p < q` (pointers) | `<`, `<=`, `>`, `>=` are for numbers only |
| `s == 0` with `s: obsidian` | comparison between different types: convert with `as` |
| `-text`, `~c` | sign only on numbers, `~` only on integers |
| `s += 0.5` with `s: stone` | write `s = (s + 0.5) as stone` |

**Warnings (the program still builds)**

`[AVISO DE TIPO]` shows up when a value is stored into a type with no defined conversion, e.g. `forge x: stone = "abc"` or `forge c: crystal = true`. There native code reinterprets the bits (or the address) and the interpreter keeps the value as is. It stays a warning because low-level code sometimes wants exactly that reinterpretation: use `as T` to make the intent explicit.

When `typeck` does not know the type of one side (a call without a known signature, for instance), it neither converts nor reports an error.

**Variants in `echo`**: `echo(v)` of a variant (a `strata` type or a literal like `Green`) prints the **name** in both modes. The numeric tag is `v as stone`.

---

## 3. Variables and constants

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

### Common assignments

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

## 4. Functions and intents

### Regular functions

```ofs
vein soma(a: stone, b: stone) -> stone {
    return a + b
}
```

### Program entry

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

Accepted intents:

- `pure`
- `impure`
- `fractal`

### Function composition

Functions can call other functions and support direct recursion:

```ofs
vein quadrado(n: stone) -> stone {
    return n * n
}

vein soma_quadrados(a: stone, b: stone) -> stone {
    return quadrado(a) + quadrado(b)
}

core main() {
    echo(soma_quadrados(3, 4))
}
```

> **Note**: Anonymous function expressions (inline `vein(...) -> ...`) are specified in the syntax tree (`NK_LAMBDA`) and are planned for the language's evolution roadmap. Currently, all functions must be declared with a name via `vein`.

---

## 5. `monolith`, `impl` and layouts

### Structure

```ofs
monolith Rect {
    w: stone
    h: stone
}
```

### Methods with `impl`

```ofs
impl Rect {
    vein area(self: Rect) -> stone {
        return self.w * self.h
    }
}
```

Usage:

```ofs
forge r: Rect
r.w = 10
r.h = 5
echo(r.area())
```

### Explicit layout

> **Planned:** `layout` is a reserved word, but the parser does not accept `layout` on a `monolith` yet (it reports `expected '{' after monolith name`). Every `monolith` uses the native layout today. The syntax below is the intended one.

```ofs
monolith Header layout packed {
    tag: stone
    flags: stone
}
```

Supported layouts:

- `native`
- `packed`
- `c`

---

## 6. `namespace`

Use namespace to group functions and avoid name collisions.

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

## 7. Arrays and collections

```ofs
forge nums = [1, 2, 3]
echo(nums[0])
```

Example passing a function:

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

## 8. Control flow

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

C-like form:

```ofs
cycle (forge i = 0; i < 10; i++) {
    echo(i)
}
```

Iterative form:

```ofs
cycle (n in nums) {
    echo(n)
}
```

### Multi-branch selection

Multi-branch selection is done with chains of `if / else if`:

```ofs
if (code == 200) {
    echo("ok")
} else if (code == 404) {
    echo("nao encontrado")
} else {
    echo("erro")
}
```

Or with `match` (synonym: `assay`), which compares the value with each case using `==`; `_` is the default case:

```ofs
match (code) {
    200 => { echo("ok") }
    404 => { echo("not found") }
    _ => { echo("error") }
}
```

### Error handling and interruption (`throw`)

The `throw` statement displays a diagnostic message and ends execution with exit code 1:

```ofs
vein dividir(dividendo: stone, divisor: stone) -> stone {
    if (divisor == 0) {
        throw "divisao por zero indefinida"
    }
    return dividendo / divisor
}

core main() {
    echo(dividir(10, 2))
}
```

> **Note**: The syntax `tremor { ... } catch (e: obsidian) { ... }` is recognized by the grammar (`NK_TREMOR`), with structured stack unwinding and runtime capture planned for the interpreter and runtime evolution layers.

### `strata` (variant enumerations)

```ofs
strata Status { Idle, Running, Failed }
```

> **Note**: a variant is a numeric tag (`v as stone`) and can be stored in a `strata` type (`forge c: Status = Idle`, parameters, return values, `Array<Status>`). `echo(c)` prints the name (`Idle`) in both modes. The payload of `Ok(7)` is not stored by native code yet (see `?` propagation).

### `?` propagation

`expr?` unwraps a result variant:

- `Solid(v)?` and `Ore(v)?` evaluate to `v`.
- `Crack(e)?` and `Barren?` leave the current `vein` immediately (pending `anchor`s run), propagating the failure. Failure variants are the ones named `Crack` and `Barren`; any other variant counts as success.

The propagated value is the same in both modes:

- In the native build variants are payload-less `i64` tags: the tag is the variant's position in declaration order across all `strata` of the program (with `strata Outcome { Solid, Crack }` declared first, `Crack` = 1). A `vein ... -> stone` that propagates `Crack` returns that tag, and `echo` prints `1` in both modes. With an `obsidian` return you get the tag as text; with `crystal`, the number; in `core main` it becomes the exit code.
- In the interpreter, if the declared return type is the `strata` type itself (`-> Outcome`), the whole variant (with payload) is returned.
- Since native code keeps no payload, `x?` over an expression that is not a direct constructor (e.g. `f()?`) evaluates to the success variant's tag, not its payload.

```ofs
strata Outcome { Solid, Crack }

vein ok() -> stone {
    forge v = Solid(7)?      // 7
    return v
}

vein bad() -> stone {
    forge v = Crack("boom")? // returns 1 right here
    return v
}

core main() {
    echo(ok())   // 7
    echo(bad())  // 1
}
```

Test: `ofs/tests/regress/question_propagation.ofs`.

---

## 9. Operators and casts

### Main operators

- arithmetic: `+`, `-`, `*`, `/`, `%`
- comparison: `==`, `!=`, `<`, `<=`, `>`, `>=`
- logical: `&&`, `||`, `!`
- bitwise: `&`, `|`, `^`, `<<`, `>>`

### Cast with `as`

```ofs
forge r: u8 = 255
forge x: u32 = r as u32
forge y: stone = x as stone
forge z: u8 = y as u8
```

It also works for conversions between integer and float:

```ofs
forge c: crystal = 10.5
forge n: u8 = c as u8
```

---

## 10. `attach`

### Standard library or installed package

```ofs
attach {core}
attach {window}
attach {terminal-colors}
```

### Local file

```ofs
attach {F:modulos/minha_lib.ofs}
```

On Windows an absolute path also works:

```ofs
attach {F:G:\projeto\minha_lib.ofs}
```

`attach` embeds the OFS code of the other file into the build process. It does not load a native DLL.

---

## 11. `extern vein` and `rift vein`

The **Rift** is the boundary through which OFS reaches the native system libraries (libc, libm, X11, Android JNI, kernel32). `extern vein` and `rift vein` declare a function that lives in a native library.

### `extern vein`

```ofs
extern vein puts(s: obsidian) -> stone
```

### `rift vein`

```ofs
rift vein ofs_window_create(title: obsidian, w: stone, h: stone) -> void abi c
```

### Link metadata

```ofs
rift vein text_size(s: obsidian) -> stone bind "strlen" abi c
```

### Function address and indirect call (`fault_addr`, `fault_call`)

OFS has no "function pointer" type, but it can get the address of a `vein` as a `stone` and call an address using the system calling convention. This is useful for native library callbacks (`qsort`, `ANativeActivity`, `ALooper`, `AChoreographer`) and for native function tables (JNI).

```ofs
rift vein c_qsort(base: *stone, n: stone, size: stone, cmp: stone) -> void bind "qsort" abi c

vein crescente(a: *stone, b: *stone) -> stone {
    forge x = 0
    forge y = 0
    fracture { x = *a; y = *b; }
    if (x < y) { return -1 }
    if (x > y) { return 1 }
    return 0
}

core main() {
    forge buf = ofs_alloc(64)
    // ...fills 8 values...
    abyss {
        c_qsort(buf, 8, 8, fault_addr(crescente))
        forge r = fault_call(fault_addr(crescente), buf, fault_step(buf, 1))
    }
}
```

- `fault_addr(nome)` → `stone`: address of the vein. Accepts a module `vein`, `ns.vein`, and `extern`/`rift vein` (with `bind`, it returns the native symbol). In IR: `ptrtoint ptr @simbolo to i64`. Outside a vein, it is a semantic error.
- `fault_call(endereco, args...)` → `stone`: indirect call using the system calling convention (`inttoptr` + `call i64`). Integer, pointer and `obsidian` arguments; the return value is read as `i64` (`void` functions return garbage, ignore it). No `float`/`double`.
- The vein used as a callback must have parameters compatible with the system calling convention: `stone` for integers (32-bit integers arrive with undefined upper bits; mask them with `& 4294967295`) and `*stone` for pointers.
- In the interpreter (`ofs run`), `fault_addr` of an `extern`/`rift vein` returns the native address and `fault_addr` of an interpreted vein returns a native trampoline that re-enters the interpreter (the `qsort` above works the same). See "In the interpreter (`ofs run`): the Rift".
- Test: `ofs/tests/runtime/test_fault_addr_qsort.ofs`.


Important fields:

- `bind "..."` sets the actual native symbol
- `abi c` selects the system calling convention
- `...` is accepted in a native variadic function

### In the interpreter (`ofs run`): the Rift

`ofs run` calls the same native system library functions the native build would link, with no C code and no libffi:

- **Resolved at load time.** Every `extern vein`/`rift vein` called by the program (or passed to `fault_addr`) becomes an address via `dlsym(RTLD_DEFAULT, "symbol")`, using `bind` when present. The interpreter first opens `libm` (the build links `-lm`) and the libraries in `OFS_FFI_LIBS` (`libfoo.so:/path/libbar.so`); if a symbol is missing it also tries `libX11`/`libXft` (`ofs build` links both when present). A missing symbol gives the linker's error (`undefined reference to 'x'`) before running, with exit code 1. An `extern` that is declared but never called does not need to exist (as in the link).
- **OFS runtime.** `ofscc` is linked with `-rdynamic`, so `ofs_alloc`, `ofs_echo_color` etc. are found too. Veins defined in `stdlib/runtime/*.ofs` (pulled in with `attach`) run the native version linked into `ofscc`, like the builtins, when the signature fits the Rift; `OFS_RUN_RUNTIME=interp` turns this off.
- **Conversion.** `stone`, `bool`, integers (`i8`..`u64`, narrowed to width and sign), pointers and `obsidian` (the value's own `char*`) go as `i64`; `crystal` goes in an FP register. The return follows the declared type: `char*` becomes `obsidian` without a copy (as in native code, where `obsidian` is the pointer; `NULL` becomes `null`), `*T` becomes the address (it can go back to the native function, be compared with `null`, converted with `as` and dereferenced), `monolith` the block and `Array` the `ofs_array` itself.
- **Trampolines.** The call goes through `__ofs_ffi_dispatch`, generated by `llvmgen` inside `ofscc`: one case with the exact LLVM type per signature class (up to 4 arguments with any `stone`/`crystal` mix and an integer or `crystal` return; up to 12 integer-only; variadics with 1 to 3 fixed and up to 4 extra). Any other class is a clear error.
- **Variadics** (`printf`, `snprintf`, `open`...): the ones `llvmgen` knows by name; extras use the declared type (or the value's type), with the right convention (`%al` on x86-64).
- `fault_addr(extern)` returns the native address and `fault_call(address, ...)` calls it with arguments in the value's type (as in native code). `fault_step` adds to a native pointer.
- **Raw memory (F2).** The interpreter runs natively, so a pointer is a real address. `*p` and `*p = v` read and write with the width of the pointee type (`u8`/`i8`/`u16`/`i16`/`u32`/`i32`/`u64`/`i64`/`stone`/`f32`/`f64`/`crystal`/`bool` (4 bytes)/`obsidian`/`*T`), using the `typeck` static type (the same one `llvmgen` uses); narrow integers are sign-extended (`iN`) or zero-extended (`uN`), as in native code. A null `p` faults like native code (address 0). `fault_memcpy`/`fault_memset` use real `memcpy`/`memset` of any size; `fault_step(p, i)` adds `i * sizeof(*p)` with the static type of `p`.
- **`as`** follows the `llvmgen` rule: same LLVM type only retags; `ptrtoint`/`inttoptr` (including `obsidian`, which is the `char*` itself); `fptosi`; `sitofp` from `iN`/`stone` and `uitofp` from `uN` (`(255 as u8) as crystal` is 255.0); `fpext`/`fptrunc` (`f32` rounds); `trunc` if the source is wider, otherwise `sext` from `iN` and `zext` from `uN`/`bool` (`(0 - 1) as i8 as stone` is -1). `forge x: u8 = 300` and `x = v` into a `u8`/`i32`/`f32` truncate like the native `store`.
- **`&x`.** The address of an interpreter variable is real: on the first `&x` the variable moves into an 8-byte raw-memory slot (the native function can read and write it: `sscanf(s, "%d", &n)`, `strtol(s, &end, 10)`, `modf(x, &whole)`), freed when the variable goes out of scope, like the native `alloca`. `&x` of a `monolith` is the block itself. Only variables have an address (native code has no `&s.field`/`&a[i]` either: use `(&s) as stone + offset`); `&x` of an `Array` variable is a clear error.
- **`monolith` with system layout.** Instances live in raw memory with the `llvmgen.struct_layout` layout (the same as native code: fields in order, each aligned to its own size, up to 8; `bool` takes 4 bytes; `Array` and nested `monolith` take 8). `s` or `&s` passed to a native function (`memcpy`, `qsort` of structs...) sees the same bytes as the native build. As in native code, a `monolith` passed to a vein is by reference and `forge y: T = x`/`y = x` copy. An `Array` field holds the interpreter's array (the native function should not read it).
- **`Array` and strings.** An `Array` passed to a native function is the native `ofs_array` itself (`[data, len, cap, elem_size]` header + data) of the interpreter's value column; the native function writes elements in place and the length is synchronized on return. Elements of a single type (integers/pointers, `crystal`, `bool`, `obsidian` or `monolith`); mixed types are a clear error. An `Array` returned by a native function uses the same block, without copying. `obsidian` already is the `char*`. Use `ofs_array_data(a)` to pass only the data (e.g. `qsort`).
- **Native → OFS callbacks.** `fault_addr(vein)` of an interpreted vein binds the vein to one of 16 native trampolines (`__ofs_cb_0`..`__ofs_cb_15`, generated by `llvmgen` inside `ofscc`); the library calls the trampoline, which re-enters the interpreter, including inside a native call made by the program itself (`qsort` inside `qsort`, `fault_call` of another callback). The same vein reuses its slot. Up to 6 parameters that are integers, pointers, `obsidian`, `bool` or `monolith` (by pointer), with an integer/pointer return. More than 16 different veins, more than 6 parameters or `crystal`/`f32` in the callback give a clear error with exit code 1 (use `ofs run --native`). `f32` as a Rift argument is not supported yet either.
- `bedrock`, `fracture` and `abyss` do not change execution in either mode (native code is the same inside them); the `typeck` requires them for raw-memory operations (see section 12, "Rules"). The `bedrock` cells and regions (stdlib) use the raw memory above.
- Tests: `ofs/tests/regress/ffi_*.ofs`, `tests/parity/cases/libc_*`.

---

## 12. Low-level blocks

### `fracture`

Typed low-level layer.

### `abyss`

Raw and unrestricted layer.

### `fractal`

Special layer tied to intent/effects.

### `bedrock`

OFS vocabulary for memory, regions, lanes, packets and `fault_*` intrinsics.

Example:

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

### `tectonic` alias

The parser accepts textual aliases such as:

- `tectonic fracture`
- `tectonic abyss`
- `tectonic fractal`
- `tectonic safe`
- `tectonic unsafe`
- `tectonic bedrock`

### Rules for `fracture`, `bedrock`, `tectonic` and `abyss` (F4)

The `typeck` requires a low-level block for operations that touch raw memory. The rule is the same in `ofs build` and `ofs run` (it comes from the `typeck`):

| Operation | `fracture` / `bedrock` / `tectonic` | `abyss` |
|---|---|---|
| `*p` (read) and `*p = v` (write) | allows | allows |
| `&x` | allows | allows |
| `as` with a pointer (`n as *u8`, `p as stone`, `s as *u8`, `p as *T`) | allows | allows |
| `fault_step`, `fault_memcpy`, `fault_memset` | allows | allows |
| `fault_call`, `fault_addr` | — | allows |

- Address arithmetic goes through these operations: `p + 1` on a pointer is already a `[ERRO DE TIPO]`; use `fault_step(p, n)` or `((p as stone) + n) as *T`, both inside the block.
- Still free outside a block: calling `extern`/`rift vein` (even with pointer arguments or results, e.g. `forge buf = ofs_alloc(64)`), comparing a pointer with `null`, storing and passing pointers along, and the bit intrinsics (`fault_count`, `fault_cut`, `fault_weave`...).
- `tectonic X` counts as `X` (`tectonic abyss { }` = `abyss`); a bare `tectonic { }` counts as `fracture`. Nested blocks add up: inside `abyss`, everything is allowed.
- A whole vein can be marked: `fracture vein read(p: *u8) -> stone { return *p }`, `abyss vein call(f: stone) -> stone { return fault_call(f) }` (also inside `namespace` and `impl`). The body counts as the block. Code that must build with an `ofscc` older than this rule (the compiler itself, because of the seed, and the stdlib) uses a `fracture { }` block inside the vein, which every compiler understands.
- In the tree the block is an `NK_BLOCK` with the name in `val` (`"fracture"`, `"abyss"`...); `llvmgen` and the interpreter run it as a plain block (same IR).

**Migration.** Until **2027-01-09**, an operation outside a block is only a warning, on stderr (in `ofs run` stdout belongs to the program), and the program builds the same:

```
[AVISO DE FALHA app.ofs:12:15]: acesso por ponteiro '*p' (leitura ou escrita) fora de bloco de baixo nivel (libera: fracture, bedrock ou abyss)
  dica: fracture { forge v = *p; *p = v + 1 }  ou marque a vein: 'fracture vein f(...)'
[AVISO DE FALHA] 1 operacao(oes) de baixo nivel fora de bloco: viram erro em 2027-01-09; OFS_STRICT_FAULT=1 ja trata como erro
```

With `OFS_STRICT_FAULT=1` (the mode of the repository tests) the same line becomes `[ERRO DE FALHA file:line:col]` and the build stops with exit code 1. From 2027-01-09 on, strict mode becomes the default. The stdlib, runtime, compiler, tools, examples and tests already build without any warning.

```ofs
core main() {
    forge x: stone = 10
    fracture {
        shard p: *stone = &x   // p points to x
        *p = 42
    }
    echo(x)  // prints 42
}
```

Tests: `ofs/tests/regress/fault_blocks.ofs`, `fault_err_*.ofs`, `fault_warn_default.ofs`.

---

## 13. Standard library and packages

Relevant modules and packages already present in the project:

- `fs`
- `net`
- `http`
- `odl`
- `oes`
- `oll`
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

The `fs` module exposes:

- `fs.exists(path: obsidian) -> bool`
- `fs.size(path: obsidian) -> stone`
- `fs.read(path: obsidian) -> obsidian`
- `fs.write(path: obsidian, content: obsidian) -> bool`
- `fs.append(path: obsidian, content: obsidian) -> bool`

The `net` module exposes:

- `net.tcp_connect(host: obsidian, port: stone) -> stone`
- `net.tcp_send(fd: stone, data: obsidian) -> stone`
- `net.tcp_recv(fd: stone, max_len: stone) -> obsidian`
- `net.tcp_close(fd: stone) -> void`
- `net.tcp_listen(port: stone, backlog: stone) -> stone`
- `net.tcp_accept(server_fd: stone) -> stone`

The `http` module exposes:

- `http.get(ip: obsidian, port: stone, host: obsidian, path: obsidian) -> HttpResponse`
- `http.parse_status_code(response_text: obsidian) -> stone`
- `http.extract_body(response_text: obsidian) -> obsidian`

The `odl` module exposes HTML builders: each function returns the fragment as `obsidian`, and fragments nest by passing one as another's `children`. HTML is only the generated artifact; the source stays in OFS.

- `odl.attr(name: obsidian, value: obsidian) -> obsidian`
- `odl.empty_attrs() -> obsidian`
- `odl.tag(name: obsidian, attrs: obsidian, children: obsidian) -> obsidian`
- `odl.void_tag(name: obsidian, attrs: obsidian) -> obsidian`
- `odl.text(value: obsidian) -> obsidian`
- `odl.document(lang: obsidian, title: obsidian, description: obsidian, head_extra: obsidian, body: obsidian) -> obsidian`
- `odl.css(href: obsidian) -> obsidian`
- `odl.icon(href: obsidian) -> obsidian`
- `odl.script(src: obsidian) -> obsidian`
- `odl.script_module(src: obsidian) -> obsidian`
- `odl.meta(name: obsidian, content: obsidian) -> obsidian`
- `odl.component(name: obsidian, class_name: obsidian, children: obsidian) -> obsidian`
- `odl.div(class_name: obsidian, children: obsidian) -> obsidian`
- `odl.section(id: obsidian, class_name: obsidian, children: obsidian) -> obsidian`
- `odl.article(class_name: obsidian, children: obsidian) -> obsidian`
- `odl.nav(class_name: obsidian, children: obsidian) -> obsidian`
- `odl.header(class_name: obsidian, children: obsidian) -> obsidian`
- `odl.main(id: obsidian, children: obsidian) -> obsidian`
- `odl.footer(class_name: obsidian, children: obsidian) -> obsidian`
- `odl.a(class_name: obsidian, href: obsidian, children: obsidian) -> obsidian`
- `odl.img(src: obsidian, alt: obsidian) -> obsidian`
- `odl.p(class_name: obsidian, children: obsidian) -> obsidian`
- `odl.h1(children: obsidian) -> obsidian`
- `odl.h2(children: obsidian) -> obsidian`
- `odl.h3(children: obsidian) -> obsidian`
- `odl.strong(children: obsidian) -> obsidian`
- `odl.span(class_name: obsidian, children: obsidian) -> obsidian`
- `odl.ul(class_name: obsidian, children: obsidian) -> obsidian`
- `odl.ol(class_name: obsidian, children: obsidian) -> obsidian`
- `odl.li(children: obsidian) -> obsidian`
- `odl.form(action: obsidian, method: obsidian, children: obsidian) -> obsidian`
- `odl.label(for_id: obsidian, children: obsidian) -> obsidian`
- `odl.input(input_type: obsidian, name: obsidian, value: obsidian) -> obsidian`
- `odl.button(button_type: obsidian, class_name: obsidian, children: obsidian) -> obsidian`
- `odl.code_block(children: obsidian) -> obsidian`
- `odl.table(head: obsidian, body: obsidian) -> obsidian`
- `odl.table_id(id: obsidian, head: obsidian, body: obsidian) -> obsidian`

The `oes` module exposes CSS builders in the same style: each function returns the text of the rule or declaration.

- `oes.prop(name: obsidian, value: obsidian) -> obsidian`
- `oes.rule(selector: obsidian, body: obsidian) -> obsidian`
- `oes.media(query: obsidian, body: obsidian) -> obsidian`
- `oes.supports(query: obsidian, body: obsidian) -> obsidian`
- `oes.layer(name: obsidian, body: obsidian) -> obsidian`
- `oes.font_face(body: obsidian) -> obsidian`
- `oes.keyframes(name: obsidian, frames: obsidian) -> obsidian`
- `oes.frame(step: obsidian, body: obsidian) -> obsidian`
- `oes.var(name: obsidian, value: obsidian) -> obsidian`
- `oes.use(name: obsidian) -> obsidian`
- `oes.transition(property: obsidian, duration: obsidian, easing: obsidian) -> obsidian`
- `oes.animation(name: obsidian, duration: obsidian, easing: obsidian) -> obsidian`

```ofs
attach {odl}
attach {oes}

forge corpo = odl.section("inicio", "hero", odl.h1("OFS") + odl.p("lead", "Uma linguagem só."))
forge estilo = oes.rule(".hero", oes.prop("padding", "2rem") + oes.prop("color", oes.use("texto")))
```

The `oll` module exposes:

- `oll.parse(src: obsidian) -> stone`
- `oll.load(path: obsidian) -> stone`
- `oll.node_count() -> stone`
- `oll.get_node(id: stone) -> OllNode`
- `oll.set_node(id: stone, n: OllNode) -> void`
- `oll.find_by_name(name: obsidian) -> stone`
- `oll.compute_layout(root_id: stone) -> void`
- `oll.render(cv: Canvas) -> void`
- `oll.hit_test(x: stone, y: stone) -> stone`
- `oll.click(id: stone) -> obsidian`
- `oll.reset() -> void`

The `canvas` module exposes:

- `canvas.create(w: stone, h: stone) -> Canvas`
- `canvas.destroy(cv: Canvas) -> void`
- `canvas.clear(cv: Canvas, color: stone) -> void`
- `canvas.set_pixel(cv: Canvas, x: stone, y: stone, color: stone) -> void`
- `canvas.get_pixel(cv: Canvas, x: stone, y: stone) -> stone`
- `canvas.fill_rect(cv: Canvas, x: stone, y: stone, w: stone, h: stone, color: stone) -> void`
- `canvas.draw_rect(cv: Canvas, x: stone, y: stone, w: stone, h: stone, color: stone) -> void`
- `canvas.draw_char(cv: Canvas, x: stone, y: stone, ch: stone, color: stone) -> void`
- `canvas.draw_text(cv: Canvas, x: stone, y: stone, text: obsidian, color: stone) -> void`
- `canvas.save_ppm(cv: Canvas, path: obsidian) -> bool`
- `canvas.present(cv: Canvas) -> void`

The `window` module exposes:

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

---

## 14. Compiler commands

### Main compiler (`ofs`)

```bash
ofs run arquivo.ofs
ofs build arquivo.ofs -o app
ofs check arquivo.ofs
ofs clean
ofs tokens arquivo.ofs
ofs ast arquivo.ofs
ofs ir arquivo.ofs
ofs asm arquivo.ofs
```

### Self-hosted compiler (`ofscc`) in its current state

`ofscc` already supports optimization and inspection modes through an environment bridge:

Variables:

- `OFSCC_INPUT`
- `OFSCC_OUTPUT`
- `OFSCC_C_OUT`
- `OFSCC_OPT` (`-O0`, `-O2`, `-O3`)
- `OFSCC_MODE` (`check`, `tokens`, `ast`)

Example:

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

Packages:

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

### seal / anchor (run on scope exit)

`seal { … }` and `anchor { … }` are **the same thing** (the same token): they register an action that runs when the current scope ends, on any exit (end of block, `return`, `break`/`shatter`, `continue`/`erode`, `?` propagation). Several actions run in reverse order (the last registered runs first). `seal` is the thematic name ("seals the resource on the way out"); `anchor` keeps working. Both take a block or a single statement.

```ofs
vein copy(src: obsidian) -> stone {
    forge f = fopen(src, "r")
    seal { fclose(f) }          // runs on any return below
    anchor echo("copy done")    // same as seal; runs before fclose (LIFO)
    if (f == null) { return 1 }
    // ...
    return 0
}
```

- The action is evaluated **on exit**, with the values of that moment.
- `return`, `break`, `continue` and `throw` **inside** `seal`/`anchor` are forbidden (error `E0042`).
- Same behavior in native code and in `ofs run`. Test: `ofs/tests/regress/seal_defer.ofs`.

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

### Where `attach` looks

The first match wins:

| Form | Search order |
|---|---|
| `attach {module}` | 1. `$OFS_STDLIB_PATH/module.ofs` · 2. the attaching file's folder · 3. each folder in `$OFS_LIB_PATH` (`:`-separated) · 4. `$OFS_LIB_HOME`, or `~/.ofs/libs` (where `src/packages/install_library.sh` installs) |
| `attach {F:path}` | 1. absolute path, or relative to the file's folder · 2. `$OFS_STDLIB_PATH/path` · 3. and 4. as above |

In `attach {module}`, a hyphen becomes an underscore (`terminal-colors` → `terminal_colors.ofs`). A module that exists in none of these places is a **compile error** that lists every path tried (`[ERRO DE ATTACH file:line]: modulo 'x' nao encontrado`). Previously `attach {x}` of a missing module was silently ignored.

### Undefined names

Using a name that was never declared (variable, global, constant) is a **compile error** in both modes, with the real file and line and a suggestion when a similar name exists (`[ERRO SEMANTICO calc.ofs:5:12]: identificador indefinido 'contadr'` / `voce quis dizer 'contador'?`). Top-level globals may be used in a `vein` above the line that declares them. A module checked alone (`ofs check module.ofs`) does not see the names of the file that attaches it: check the main file. Incompatible types in an operation (`[AVISO DE TIPO]`) are still only a warning.

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
| `rift` | Rift wrappers (native library helpers) |
| `terminal-colors` | ANSI terminal color output |
| `memory-modes` | Docs and helpers for fracture/abyss/fractal modes |
| `test-lib` | Unit test utilities |

---

## Extern Functions

The **Rift** is the boundary through which OFS reaches the native system libraries (libc, libm, X11, Android JNI, kernel32). `extern vein` and `rift vein` declare a function that lives in a native library. There is no C code in OFS. (The system calling convention is technically known as the C ABI; it does not involve C code.)

Declare native system library functions through the Rift:

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

OFS also supports `rift vein` as a declaration form for native functions through the Rift:

```ofs
rift vein strlen(text: obsidian) -> stone

core main() {
    echo(strlen("fault"))
}
```

`rift vein` lowers to the same native-call pipeline as `extern vein`, but it presents interop as an OFS-native boundary instead of framing the feature only as an "extern" declaration.

You can also add explicit interop metadata:

```ofs
rift vein text_size(text: obsidian) -> stone bind "strlen" abi c
```

- `bind "symbol"` sets the external link symbol name.
- `abi c` declares the intended ABI.
- `abi system` is reserved for platform/system-boundary calls.

### Interop Direction

`extern vein` is the current Rift surface. Today it is best suited for runtime symbols and native system libraries.

The roadmap extends this toward richer native interop layers for:

- native object and library boundaries,
- bridge layers to other ecosystems,
- host/runtime bridges,
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

A field can be another `monolith`. The field holds a **reference** to the sub-object (8 bytes in the system layout); `forge c: Body` already creates the zeroed sub-object, so `c.pos.x = 1` works directly. `c.pos = v` makes the field point to `v`; `forge w: Vec3 = c.pos` copies. Same in native code and in `ofs run` (test: `ofs/tests/regress/monolith_nested.ofs`).

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

Array indexing is **zero-based** with **bounds checking** at runtime. Out of bounds (`a[7]`, `ofs_array_get`/`ofs_array_set`), the program stops with exit code 1 and `ofs: indice 7 fora do limite do array (tamanho 5)` on stderr, in both modes.

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
- `fault_addr(vein)` -> address of the vein as `stone` (see the Rift)
- `fault_call(endereco, args...)` -> indirect call using the system calling convention (see the Rift)
- `fault_cut(value, shift, width)` -> extract a bit field
- `fault_patch(base, shift, width, insert)` -> replace a bit field
- `fault_weave(mask, left, right)` -> blend bits from `left` and `right` using `mask`
- `fault_unreachable()` -> marks the current path as unreachable, lowers to `llvm.trap` + `unreachable` IR
- `fault_memcpy(dst, src, len)` -> bulk memory copy (lowers to `llvm.memcpy`); `dst` and `src` must be pointers
- `fault_memset(dst, val, len)` -> bulk memory fill with `val` byte (lowers to `llvm.memset`); `dst` must be a pointer

Exact semantics (identical in the native build and in the `ofs run` interpreter):

- Everything is 64-bit two's complement: `fault_count(-1)` = 64; `fault_lead(0)` = `fault_trail(0)` = 64; `fault_lead(-1)` = 0.
- `fault_spin_left`/`fault_spin_right`: 64-bit rotation (`llvm.fshl`/`llvm.fshr`); the distance is taken modulo 64 (`fault_spin_right(6, 65)` = 3).
- `fault_weave(mask, left, right)` = `(left & mask) | (right & ~mask)`: each bit comes from `left` where `mask` is 1 and from `right` where it is 0.
- `fault_trap()` and `fault_unreachable()` terminate the process with SIGILL (the interpreter does the same via `raise(SIGILL)`).
- `fault_fence()`/`fault_prefetch()` are no-ops in the interpreter.
- In the interpreter pointers are real addresses: `fault_memcpy`/`fault_memset` copy/fill any size, and `fault_step`, `fault_addr` and `fault_call` work as in native code (see "In the interpreter (`ofs run`): the Rift").
- Test: `ofs/tests/regress/fault_intrinsics.ofs`.

These functions are designed to cover operations assembly already has, while also giving OFS room for its own machine-oriented vocabulary.

### asm (inline assembly escape hatch) — planned

> **Status: planned.** Inline `asm` is not available yet: the lexer does not recognize the `asm` keyword and the LLVM backend does not emit `InlineAsm`. The example below shows the planned design and does not compile today. Use `fault_*` intrinsics for now.

When no `fault_*` intrinsic covers your need, `asm` will emit a raw assembly string directly at that point in the code. It will only be allowed inside a `bedrock`, `fracture`, or `abyss` block.

```ofs
bedrock {
    asm "nop"     // planned
    asm "int3"    // planned
}
```

**Restrictions:**
- Planned: the argument must be a string literal.
- Input/output operand syntax (full AT&T-style constraints) is not yet parseable; use `fault_*` intrinsics when operand binding is needed.
- Planned: a void inline-asm call in LLVM IR with the assembly string forwarded verbatim.
- Planned: the `asm` expression will be lowered to LLVM `InlineAsm`, dispatched in semantic analysis and code generation. Not implemented yet.

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
- `rift.ofs` for OFS-native Rift wrappers

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

The purpose of this module is not to imitate another language's APIs directly, but to let low-level programming grow in OFS vocabulary.

These lane helpers also give OFS an endianness-aware surface for ABI-facing bit windows without forcing the user to drop into raw shifts everywhere.

### Rift Module

`rift.ofs` is the first standard-library interop surface built on top of `rift vein`.

It provides wrappers such as:

- `rift_text_size(text)`
- `rift_line(text)`
- `rift_banner(title)`
- `rift_report_line(label, value)`

The goal is to let native system libraries be consumed through OFS-facing APIs instead of exposing only raw declarations everywhere.

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
- calling native libraries through the Rift (`rift vein`),
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

### Native OFS Runtime (Stack Magma)

The main implementation of the OFS runtime and standard library is written in OFS (`stdlib/runtime/`), with no runtime implementation in another language:

| Subsystem | Module | Responsibility |
|---|---|---|
| **Magma** | `magma.ofs` | Bootstrap and execution orchestrator of the native runtime |
| **Reservoir** | `reservoir.ofs` | Memory allocation and management (`ofs_alloc`, `ofs_free`) |
| **Facet** | `facet.ofs` | Text operations and numeric conversion (`ofs_str_*`, `ofs_*_to_*`) |
| **Matrix** | `matrix.ofs` | Dynamic collections and vectors (`ofs_array_*`) |
| **Outlet** | `outlet.ofs` | Terminal input and output (`echo_*`, ANSI colors, formatting) |
| **Foundation** | `foundation.ofs` | Operating system and environment abstractions (`ofs_getenv`, `ofs_mod`, `ofs_exit`) |
| **Pulse** | `pulse.ofs` | High-precision monotonic clock and timers (`now_ms`, `sleep_ms`) |
| **Frame** | `frame.ofs` | Management of windows and native surfaces |
| **Impulse** | `impulse.ofs` | Input events: mouse and keyboard |
| **Prism** | `prism.ofs` | 2D pixel rasterization, geometry and surfaces |

All runtime primitives are compiled directly by `ofscc` to native LLVM IR, ensuring that OFS programs run on the native runtime itself without depending on C implementations.

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
| `extern`   | `KW_EXTERN`      | Native Rift function (system library) |
| `rift`     | `KW_RIFT`        | Native function declaration via the Rift |
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
| `asm`      | `KW_ASM`         | Inline assembly escape hatch (planned, not yet available) |
