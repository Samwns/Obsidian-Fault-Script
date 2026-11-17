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

### Comments

```ofs
// Single-line comment

/* Multi-line
   comment */

forge x = 42  // end-of-line comment
```

### Statement termination

Statements end with a **line break** or a **semicolon**. Line breaks inside `()`, `[]` and `{}` are ignored (implicit continuation).

```ofs
forge x = 10; forge y = 20

forge resultado = soma(
    10,
    20
)
```

### Reserved words

| Word | Token | Purpose |
|---|---|---|
| `core` | `KW_CORE` | Entry point function |
| `vein` | `KW_VEIN` | Function definition |
| `forge` | `KW_FORGE` | Variable declaration |
| `monolith` | `KW_MONOLITH` | Struct definition |
| `fracture` | `KW_FRACTURE` | Safe pointer block |
| `abyss` | `KW_ABYSS` | Unsafe memory block |
| `cycle` | `KW_CYCLE` | Loop |
| `if` | `KW_IF` | Conditional |
| `else` | `KW_ELSE` | Else branch |
| `return` | `KW_RETURN` | Return from function |
| `echo` | `KW_ECHO` | Print (built-in) |
| `in` | `KW_IN` | Range-based cycle |
| `shard` | `KW_SHARD` | Pointer type |
| `true` | `KW_TRUE` | Boolean true |
| `false` | `KW_FALSE` | Boolean false |
| `null` | `KW_NULL` | Null value |
| `break` | `KW_BREAK` | Exit loop |
| `continue` | `KW_CONTINUE` | Skip iteration |
| `stone` | `KW_STONE` | Integer type (i64) |
| `crystal` | `KW_CRYSTAL` | Float type (f64) |
| `obsidian` | `KW_OBSIDIAN` | String type (i8*) |
| `bool` | `KW_BOOL` | Boolean type (i1) |
| `void` | `KW_VOID` | Void type |
| `attach` | `KW_IMPORT` | Attach module |
| `while` | `KW_WHILE` | Condition-only loop |
| `extern` | `KW_EXTERN` | Native Rift function (system library) |
| `rift` | `KW_RIFT` | Native function declaration via the Rift |
| `as` | `KW_AS` | Type cast |
| `match` | `KW_MATCH` | Pattern matching |
| `case` | `KW_CASE` | Match arm |
| `default` | `KW_DEFAULT` | Match fallback arm |
| `const` | `KW_CONST` | Immutable variable |
| `strata` | `KW_STRATA` | Enum declaration |
| `tremor` | `KW_TREMOR` | Error handling block |
| `catch` | `KW_CATCH` | Error handler |
| `throw` | `KW_THROW` | Raise error value |
| `tectonic` | `KW_TECTONIC` | Mode directive prefix |
| `bedrock` | `KW_BEDROCK` | Typed low-level block; an arena when it allocates (section 12) |
| `anchor` / `seal` | `TOK_ANCHOR` | Action that runs when leaving the scope (both names are the same token) |
| `quartz` | `TOK_QUARTZ` | Inline assembly, requires `abyss` |

Contextual words (not reserved; valid names elsewhere): `mineral`, `grain`, `bearing`, `settle`, `value` and `result` (contracts, section 4) and `geode` (capturing function, section 4: only when `geode (...)` is followed by `->` or `{`).

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
| name of a `vein`/`extern` used as a value (`forge f = dobro`) | `vein(T1, T2) -> R`, its signature (section 4) |
| `f(...)` with `f: vein(...) -> R` | `R` |

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

Reported as `[ERRO DE TIPO arquivo:linha:column]` with a hint, in both modes, before anything runs:

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
| `forge f: vein(stone) -> stone = nome` with `nome(x: stone) -> obsidian` | the signature must match (parameters and return) |
| `f(1, 2)` or `f("dez")` with `f: vein(stone) -> stone` | arity and argument types are checked on a vein value call |
| `f + 1`, `f < g` with `f: vein(...)` | a vein value only allows `==`/`!=` (same signature or `null`) and the call |

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

**Default parameter values:** `vein escala(x: stone, k: stone = 2) -> stone`. The default is a literal (number, text, `bool`, `null` or `-number`) and parameters with defaults come last (`[ERRO SEMANTICO]` otherwise). A call that omits the last arguments gets the default (`escala(5)` = `escala(5, 2)`), the same natively and interpreted; it also works for `impl` methods. A function type `vein(...)` does not keep defaults: a call through a vein value passes every argument. Under Fault Refract, adding a parameter with a default to a vein in use is an accepted swap (old calls get the default).

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

### Function types and vein values

A declared `vein` is also a **value**: its name (without calling it) has the function type `vein(T1, T2) -> R`, written with the same word that declares functions. Without `-> R` the return is `void`. The type works for variables, parameters, returns, `monolith` fields and `Array<...>`, and it needs no low-level block: it is safe and typed.

```ofs
monolith Operacao {
    nome: obsidian
    calc: vein(stone, stone) -> stone
}

vein dobro(x: stone) -> stone { return x * 2 }
vein triplo(x: stone) -> stone { return x * 3 }
vein soma(a: stone, b: stone) -> stone { return a + b }

// vein as argument and as return
vein aplicar(f: vein(stone) -> stone, x: stone) -> stone { return f(x) }
vein escolher(grande: bool) -> vein(stone) -> stone {
    if (grande) { return triplo }
    return dobro
}

core main() {
    forge f: vein(stone) -> stone = dobro   // or: forge f = dobro
    echo(f(21))                  // 42
    echo(aplicar(triplo, 4))     // 12
    echo(escolher(true)(10))     // 30

    forge op: Operacao
    op.calc = soma
    echo(op.calc(3, 4))          // 7

    forge ops: Array<vein(stone, stone) -> stone> = [soma]
    echo(ops[0](6, 7))           // 13

    echo(f == dobro)             // true
    echo(f)                      // dobro
}
```

- **Value:** the name of a module `vein`, of `ns.vein`, of a method (`Rect__area`) or of an `extern`/`rift vein` (with `bind`, the native symbol) is a value of its signature's type. A variadic `extern` (`printf`) cannot become a value (clear error).
- **Call:** `f(x)` with `f` a variable, parameter, global, field (`s.cb(x)`), element (`a[i](x)`) or result (`escolher(true)(10)`) is a typed indirect call. `typeck` checks the arity and the arguments with the conversions of section 2 (`f(5)` with a `crystal` parameter means `f(5.0)`); an argument with no defined conversion (`obsidian` where the type asks for `stone`) is an `[ERRO DE TIPO]`, not a warning. The result has type `R`.
- **Exact signature:** storing a vein in a place of function type (`forge`, `=`, argument, `return`, field, `push`) requires the same signature, parameter by parameter and in the return; the error says what differs (`retorno: obsidian onde o tipo pede stone`). `null` fits any function type and is the initial value of a variable or field without an initializer.
- **Equality:** `==` and `!=` between values of the same signature or with `null`. Two values of the same vein are equal in both modes.
- **`echo` and text:** `echo(f)` and `"x" + f` give the vein's name (`dobro`; `ns.f` for `ns__f`), `null` if null and `<vein>` for an address that came from C and has no name in the program. Same in both modes.
- **Calling `null`:** runtime error `ofs: chamada de vein nula (o valor de vein e null)` on stderr and exit code 1, in both modes.
- **Native:** the value is the function pointer (`ptr @dobro`) and the call is `call R (T1, T2) %f(...)` with the exact LLVM signature of the type, `crystal` included.
- **Interpreter (`ofs run`):** the value of an interpreted vein is the vein itself (direct call, no trip through C). When the value goes to C (argument of an `extern`/`rift vein`) it becomes the callback trampoline of section 11; when it comes from C (an `extern` return, a callback parameter, raw memory) the type's signature picks the Rift trampoline. A trampoline that comes back from C is recognized as its original vein.
- **Raw address:** `f as stone`, `f as *u8`, `address as vein(...)` and `fault_addr(f)` convert between a vein value and an address; only inside `abyss` (section 12). `fault_call(f, args...)` with `f` of function type checks the signature and means `f(args...)`.
- In the tree the type is an `NK_TYPE_FN` whose `val` is the canonical text without spaces (`vein(stone,crystal)->stone`), the same `type_str` as every expression of that type; the indirect call is an `NK_CALL` `__vein_call` with the vein in `c0`.
- Tests: `ofs/tests/regress/vein_type_*.ofs`.

### `geode`: a vein that captures (closure)

`geode (parameters) -> R { body }` is an **expression**: an unnamed vein that reads variables from the enclosing scope. Its value has the function type `vein(T...) -> R` above and is used like any vein value: store it in a variable, field or `Array`, pass it, return it and call it. Without `-> R` the return is `void`. The name comes from the geode, the hollow stone that keeps what it captured.

```ofs
vein somador(base: stone) -> vein(stone) -> stone {
    return geode (x: stone) -> stone { return x + base }
}

core main() {
    forge k = 10
    forge mais_k = geode (x: stone) -> stone { return x + k }
    k = 100
    echo(mais_k(5))            // 15: k was copied when the geode was born
    forge s7 = somador(7)
    echo(s7(1))                // 8
    echo(mais_k)               // <geode>
}
```

- **Capture by value.** Every local variable (or parameter) from outside that the body reads becomes a capture, copied when the geode is created; changing the variable later does not change the geode. Globals are not captured (the geode reads the current global, like a vein). A geode inside a geode captures transitively.
- **What the copy means:** the same as passing the variable as a vein argument. Numbers, `bool` and `strata` tags are copied; `obsidian` is an immutable value (the copy is the text); `Array` and `monolith` are the same object (the geode sees later `push`es and field changes, and what it changes in elements/fields is visible outside). For an independent `monolith`, copy it first (`forge c: P = m`) and capture `c`.
- **Assigning to a capture** inside the geode (`n = n + 1`, `n++`, `&n`) is `[ERRO DE GEODE]`: the capture is a copy and the assignment would not change the outer variable. Use a local (`forge v = n`) or return the value. `shatter`/`erode` in the body outside a loop of the geode itself is also an error (the body is another vein).
- **Equality and text:** each creation of a capturing geode is a distinct value (`==` is `true` only with itself); a geode without captures is always the same value. `echo(g)` and `"x" + g` give `<geode>` in both modes.
- **Rift (C):** a geode **without captures** is an ordinary vein and goes to C as a callback (`qsort(v, n, 8, geode (a: *stone, b: *stone) -> i32 { ... })`). A **capturing** geode cannot go to C (C only calls a function address): passing a capturing geode literal to an `extern` is `[ERRO DE GEODE]` at compile time; if it reaches C through a variable, or becomes an address (`as stone`, `fault_addr`), the program stops with `ofs: geode com captura nao pode ir para o C (Rift) nem virar endereco; ...` on stderr and exit code 1, in both modes.
- **How it works:** the `typeck` finds the captures and lifts the body into a vein `__geode_<vein>_<k>(captures..., parameters...)` at the end of the program. Natively, without captures the value is `ptr @__geode_...`; with captures it is a heap environment `[adapter, mark, capture 0, ...]` (8 bytes each) and the value is that address with bit 0 set; an indirect call tests the bit (only in modules that have a capturing geode, so other programs' IR is unchanged) and calls `adapter(environment, args)`. In the interpreter the value is the body vein plus an environment (an interpreted array with the captures). The environment lives until the program ends.
- **Crystallization and Fault Refract:** in a program with geodes, a vein that handles vein values (or calls one that does) stays interpreted; a geode body is an ordinary vein and can crystallize. In `ofs refract`, editing a geode body (same captures and parameters) swaps live, including geodes already created; if the captures change, geodes already created keep the old body and new ones use the new body.
- Tests: `ofs/tests/regress/geode_*.ofs`, the `geode` case in `ofs/tests/refract/run.sh`.


### Contracts and refined types (`mineral`, `grain`, `bearing`, `settle`)

A **`mineral`** is a type with a rule: a base type plus one or more **`grain`** clauses (inside the rule, `value` is the value being checked). A vein can have **`bearing`** (precondition: the caller guarantees it) and **`settle`** (postcondition: holds for the return; `result` is the returned value), as many as needed, between the signature and the body.

```ofs
mineral UserId = stone grain value > 0
mineral Email = obsidian grain contains("@")
mineral Nota = stone grain value >= 0 grain value <= 10

vein dividir(a: stone, b: stone) -> stone
    bearing b != 0
    settle result * b + a % b == a
{
    return a / b
}
```

- **Compatibility:** a mineral is its base type for the type checker, llvmgen and the interpreter. Base → mineral checks the rule (`forge x: UserId = e`, `x = e`, `x += e`, `x++`, a `monolith` field of mineral type `obj.f = e`, an argument for a mineral parameter, `return` from a `-> UserId` vein); mineral → base is free. `Array<UserId>`, `*UserId`, `as UserId` and `vein(UserId)->T` behave as the base, unchecked.
- **Mineral of a mineral:** `mineral Small = UserId grain value < 100` adds the rules (the base ones first).
- **`grain` shorthand:** a call with no `value` among its arguments gets `value` as the 1st argument; `contains`/`len` with no vein of that name are the text ones: `contains("@")` = `ofs_str_contains(value, "@")`, `len() >= 2` = `ofs_str_len(value) >= 2`.
- **`result`** is only valid in `settle` (in `bearing` it is an error). In `settle`, parameters have their entry values even if the body changes them. In a vein without a return value, `settle` is checked at each `return` and when falling off the end.
- **`mineral`, `grain`, `bearing`, `settle`, `value` and `result` are contextual words**, not reserved: they still work as names elsewhere.

**When the rule is checked**

| Situation | Check |
|---|---|
| Violation proven at compile time: literal, `const` or arithmetic on them (`forge e: Email = "x"`, `dividir(1, 0)`, `return -1` in `-> UserId`, a `settle` that only depends on `result`) | `[ERRO DE CONTRATO arquivo:linha:col]` with the rule and the values, in both modes; also in release |
| At run time (`ofs run` and default `ofs build`) | `ofs: [CONTRATO VIOLADO arquivo:linha:col]: bearing de 'dividir' falhou: b != 0 (b = 0)` on stderr and **exit code 1**; same text in both modes |
| Release: `ofs build --release` or `OFS_CONTRACTS=0` | no run-time check (the static one stays); the IR has no checks |

A value proven correct at compile time gets no run-time check. A contract failure is not a `throw`: `tremor`/`catch` does not catch it. Leaving through `?` (propagation) does not check `settle`.

**Boundary cases: `ofs assay file.ofs`.** For each vein with a contract and only integer parameters (`stone`, `iN`, `uN` or a mineral with an integer base), it generates cases at the rule boundaries: each comparison of a parameter with a constant (`b != 0`, `n <= 10`, grain `value > 0`) yields the boundary value and ±1; a parameter with no bound uses -1, 0 and 1 (up to 64 cases per vein). Cases outside the `bearing`/`grain` are only listed (`--`); the others are called (`ok`) and `settle` checks the result. The first violation stops with the contract message and exit code 1. A rule that depends on anything other than constants and parameters leaves the vein out (`pulada`). `impl` veins, non-integer parameters and loop ranges are future work.

```
$ ofs assay metade.ofs
assay metade(n: stone)
  --   metade(-1): fora do bearing n >= 0
  ok   metade(0)
ofs: [CONTRATO VIOLADO metade.ofs:3:5]: settle de 'metade' falhou: result * 2 == n (result = 0, n = 1)
```

**Fault Refract:** changing a `bearing`/`settle` is an edit of the vein (it is swapped live); changing the `grain` of a mineral used in a signature is a type change (the swap is refused while the vein is in use).

Tests: `ofs/tests/regress/contract_*.ofs`. Pending: mineral checks in `Array<Mineral>`, in `as` and in globals with a non-literal value; loop ranges in the static check; `ofs assay` with non-integer parameters and without stopping at the first violation.

---

## 5. `monolith`, `impl` and layouts

### Structure

```ofs
monolith Rect {
    w: stone
    h: stone
}
```

A field can be another `monolith`. The field holds a **reference** to the sub-object (8 bytes in the system layout); `forge c: Corpo` already creates the zeroed sub-object, so `c.pos.x = 1` works directly. `c.pos = v` makes the field point to `v`; `forge w: Vec3 = c.pos` copies. Same in native code and in `ofs run` (test: `ofs/tests/regress/monolith_nested.ofs`).

```ofs
monolith Vec3 {
    x: stone
    y: stone
    z: stone
}

monolith Corpo {
    id: stone
    pos: Vec3
    massa: stone
}

core main() {
    forge c: Corpo
    c.pos.z = 3
    echo(c.pos.z)   // 3
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

Indexing is **zero-based** and has **bounds checking** at run time:

```ofs
forge nums = [10, 20, 30, 40, 50]
forge primeiro = nums[0]   // 10
nums[2] = 99               // changes an element
```

Out of bounds (`nums[7]`, `ofs_array_get`/`ofs_array_set`), the program stops with exit code 1 and a message on stderr, the same in both modes:

```
ofs: indice 7 fora do limite do array (tamanho 5)
```

The check applies in `ofs run`, in the default `ofs build` and in `ofs build --release`: the inline fast path of arrays (A6) already compares the index with the size before the direct access and falls into the runtime when out of bounds. Measured cost (A5.4, `bench/`): no noticeable difference in the 6 benchmark programs (sort 14 × 13 ms); in a loop that only reads and writes `a[k]` (100 million accesses) 137 × 126 ms (~9%). That is why release keeps the check (the release IR does not change).

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
    404 => { echo("nao encontrado") }
    _ => { echo("erro") }
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

### `break` and `continue`

```ofs
cycle (forge i = 0; i < 100; i++) {
    if (i == 50) {
        break       // leaves the loop
    }
    if (i % 2 == 0) {
        continue    // skips even numbers
    }
    echo(i)
}
```

`break` and `continue` work in `cycle` and in `while`. `while` is syntactic sugar for a `cycle` with only a condition.

### `seal` and `anchor` (run on scope exit)

`seal { … }` and `anchor { … }` are **the same thing** (the same token): they register an action that runs when the current scope ends, on any exit (end of block, `return`, `break`/`shatter`, `continue`/`erode`, `?` propagation). Several actions run in reverse order (the last one registered runs first). `seal` is the thematic name ("seals the resource on exit"); `anchor` remains valid. They accept a block or a single command.

```ofs
vein copiar(origem: obsidian) -> stone {
    forge f = fopen(origem, "r")
    seal { fclose(f) }          // runs on any return below
    anchor echo("fim de copiar") // same as seal; runs before fclose (LIFO)
    if (f == null) { return 1 }
    // ...
    return 0
}
```

- The action is evaluated **on exit**, with the values of that moment.
- `return`, `break`, `continue` and `throw` **inside** `seal`/`anchor` are forbidden (error `E0042`).
- Works the same in native code and in `ofs run`. Test: `ofs/tests/regress/seal_defer.ofs`.

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

### Operator precedence (lowest to highest)

| Level | Operators | Associativity |
|---|---|---|
| 1 | `\|\|` | Left |
| 2 | `&&` | Left |
| 3 | `==` `!=` | Left |
| 4 | `<` `<=` `>` `>=` | Left |
| 5 | `\|` | Left |
| 6 | `^` | Left |
| 7 | `&` | Left |
| 8 | `<<` `>>` | Left |
| 9 | `+` `-` | Left |
| 10 | `*` `/` `%` | Left |
| 11 | Unary: `-` `!` `&` `*` `++` `--` `~` | Right |

### Allowed casts

| From | To | Result |
|---|---|---|
| `stone` | `crystal` | Integer to float |
| `crystal` | `stone` | Float to integer (truncates) |
| `bool` | `stone` | `true` → 1, `false` → 0 |
| `stone` | `bool` | 0 → `false`, non-zero → `true` |
| `bool` | `crystal` | `true` → 1.0, `false` → 0.0 |
| `crystal` | `bool` | 0.0 → `false`, non-zero → `true` |

String (`obsidian`) conversions require runtime functions (`ofs_stone_to_obsidian` etc.).

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

### Where `attach` looks

The first option that exists wins:

| Form | Search order |
|---|---|
| `attach {module}` | 1. `$OFS_STDLIB_PATH/module.ofs` · 2. the folder of the file that does the `attach` · 3. each folder of `$OFS_LIB_PATH` (separated by `:`; on Windows by `;`, like `PATH`, because `:` appears in the drive letter `C:\libs`) · 4. `$OFS_LIB_HOME`, or `~/.ofs/libs` (where `src/packages/install_library.sh` installs) |
| `attach {F:path}` | 1. absolute path, or relative to the file's folder · 2. `$OFS_STDLIB_PATH/path` · 3. and 4. as above |

In `attach {module}`, a hyphen becomes an underscore (`terminal-colors` → `terminal_colors.ofs`). A module that does not exist in any of these places is a **compile error**, with the list of paths tried:

```
[ERRO DE ATTACH app.ofs:2]: modulo 'graficos' nao encontrado
  procurado em (nesta ordem):
    /usr/lib/ofs/stdlib/graficos.ofs
    ./graficos.ofs
    /home/voce/.ofs/libs/graficos.ofs
```

(Before, `attach {x}` of a missing module was silently ignored.)

A syntax error inside an attached file reports that file and its own line: `[ERRO SINTATICO lib/util.ofs:4:11]: expected ')'` (before, only `4:11` was shown, the line of the already expanded source).

### Undefined names

Using a name that was not declared (variable, global, constant) is a **compile error**, in both modes, with the real file and line and a suggestion when there is a similar name:

```
[ERRO SEMANTICO calc.ofs:5:12]: identificador indefinido 'contadr'
  voce quis dizer 'contador'?
```

Top-level globals can be used in a `vein` before the line that declares them. A module checked on its own (`ofs check module.ofs`) does not see the names of whoever attaches it: check the main file. Incompatible types in an operation (`[AVISO DE TIPO]`) are still only a warning.

---

## 11. `extern vein` and `rift vein`

The **Rift** is the boundary through which OFS reaches the native system libraries (libc, libm, X11, Android JNI, kernel32). `extern vein` and `rift vein` declare a function that lives in a native library. There is no C code in OFS. (The system calling convention is technically known as the C ABI; it involves no C code.)

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

The typed way to pass a vein to C is the function type (section 4): declare the `rift vein` parameter with the signature and pass the vein's name, without `abyss`:

```ofs
rift vein c_qsort(base: *stone, n: stone, size: stone, cmp: vein(*stone, *stone) -> stone) -> void bind "qsort" abi c

core main() {
    forge v: Array<stone> = [42, 7, 19]
    c_qsort(ofs_array_data(v), ofs_array_len(v), 8, crescente)   // crescente: below
}
```

The raw case still exists: get the address of a `vein` as a `stone` and call an address using the system calling convention. This is useful for native function tables (JNI) and for older code whose parameter is declared as `stone` (`ANativeActivity`, `ALooper`, `AChoreographer`).

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

- `fault_addr(nome)` → `stone`: address of the vein. Accepts a module `vein`, `ns.vein`, and `extern`/`rift vein` (with `bind`, it returns the native symbol). In IR: `ptrtoint ptr @simbolo to i64`. `fault_addr(f)` with `f` a vein value means `f as stone`. Anything else is a semantic error.
- `fault_call(endereco, args...)` → `stone`: indirect call using the system calling convention (`inttoptr` + `call i64`). Integer, pointer and `obsidian` arguments; the return value is read as `i64` (`void` functions return garbage, ignore it). No `float`/`double`. When the first argument has a function type (`fault_call(f, a, b)` with `f: vein(...)`), the signature is checked (arity and arguments, as in `f(a, b)`) and the call is the typed one, returning `R`.
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
- **Native → OFS callbacks.** `fault_addr(vein)` of an interpreted vein binds the vein to one of 16 native trampolines (`__ofs_cb_0`..`__ofs_cb_15`, generated by `llvmgen` inside `ofscc`); the library calls the trampoline, which re-enters the interpreter, including inside a native call made by the program itself (`qsort` inside `qsort`, `fault_call` of another callback). The same vein reuses its slot. Up to 6 parameters that are integers, pointers, `obsidian`, `bool` or `monolith` (by pointer), with an integer/pointer return. A callback with `crystal`/`f32` (parameter or return) uses another table, `__ofs_cbx_0`..`__ofs_cbx_15`, which also receives the floating-point registers (up to 6 integers and 8 `crystal`/`f32`; wave 4). More than 16 different veins or parameters beyond those limits give a clear error with exit code 1 (use `ofs run --native`); on Windows x64 (positional registers) a `crystal`/`f32` callback keeps the clear error. The same trampoline serves a vein value passed in a `vein(...)` parameter (section 4), and crystallization uses both to call interpreted veins back. `f32` in the Rift (parameter and return, like `sqrtf`/`powf`) works in both modes.
- `fracture` and `abyss` do not change execution in either mode (native code is the same inside them); the `typeck` requires them for raw-memory operations (see section 12, "Rules"). A `bedrock` that allocates is also an arena (section 12); in the interpreter, the `ofs_alloc` written in the block comes from the runtime arena and what may leave it is decided at compile time, as in native code. The `bedrock` cells and regions (stdlib) use the raw memory above.
- Tests: `ofs/tests/regress/ffi_*.ofs`, `ptr_widths.ofs`, `tests/parity/cases/` (`ptr_*`, `libc_*`, `monolith_memcpy`, `qsort_fault_addr`, `fault_call_basic`, `bedrock_block`).


### Direction of interoperability

`extern vein` is the current surface of the Rift, best suited to runtime symbols and native system libraries. The roadmap widens this to: boundaries with native objects and libraries, bridges to other ecosystems, Python embedding and target-specific assembly escapes. These items are roadmap, not features implemented today.

### Hybrid pattern

Mixing high and low level in the same file is valid and recommended:

- business logic in ordinary `vein`s;
- calls to native libraries through the Rift (`rift vein`);
- machine-close work in `bedrock`;
- explicit storage with `bedrock.ofs`.

---

## 12. Low-level blocks

### `fracture`

Typed low-level layer.

### `abyss`

Raw and unrestricted layer.

### `fractal`

Special layer tied to intent/effects.

### `bedrock`

OFS vocabulary for memory, regions, lanes, packets and `fault_*` intrinsics. As a block it unlocks the low-level operations (F4) and, when the block itself allocates, it is an **arena** (below).

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

### `bedrock { }` arena (A5.3)

A `bedrock { ... }` block (also `tectonic bedrock { }` and `bedrock vein f() { }`, which makes the whole body a block) in which **the block's own code allocates** is an arena: everything it allocates comes from a region that is released at once when the block exits, on **any** exit (end, `return`, `shatter`/`erode`, `?`), after the block's `seal`/`anchor`s. It still unlocks the F4 low-level operations. A `bedrock` that allocates nothing is just the low-level block (same code as before).

```ofs
core main() {
    forge total = 0
    forge ultimo = ""
    forge i = 0
    while (i < 100000) {
        bedrock {
            forge partes: Array<obsidian> = []
            ofs_array_push(partes, "k" + ofs_stone_to_obsidian(i))
            forge buf: *stone = ofs_alloc(1024)
            fracture { *buf = ofs_str_len(partes[0]) }
            fracture { total = total + *buf }
            ultimo = partes[0]          // text leaves as a copy
        }                               // the whole arena is released here
        i++
    }
    echo(total)
    echo(ultimo)
}
```

- **What comes from the arena:** what the code written in the block allocates: concatenation and conversions to text, `[...]` and the growth of an `Array` created in the block, `forge m: Monolith`, `ofs_alloc`/`ofs_realloc` and the runtime's text and `Array` builtins. `ofs_free` of an arena block does nothing (it leaves with the region). The arena is **lexical**: a vein called from inside the block allocates on the normal heap (the compiler turns the arena off during the call), and an outer `Array` that grows inside the block stays on the heap. A `bedrock` inside another opens its own arena, which closes before the outer one.
- **Escape (rule):** a value that may live in the arena and goes outside it is handled like this: **`obsidian` text is copied to the heap** automatically (only if it really is in a live arena); **`Array`, `monolith`, pointer and geode** are `[ERRO DE ARENA arquivo:linha:col]` at compile time. "Outside" means: `return` from inside the block, assignment to a variable declared outside the block (local, parameter or global), a field or element of an outer object, `push`/`set` into an outer `Array`, an argument of a vein that keeps the parameter (per-vein analysis, the same idea as A6), of a vein value or of an unknown function, and runtime functions that keep the pointer (the window title). Copying a `monolith` with only numeric fields into an outer variable is allowed (it is a copy by value). C functions (`extern`/`rift`) do not keep their arguments (C convention); if one does, allocate outside the bedrock. Inside `abyss { }` the error is not raised: the programmer takes the risk.

```
[ERRO DE ARENA app.ofs:5:17]: valor alocado na arena do bedrock e guardado fora dele (a arena e liberada na saida do bloco)
  dica: aloque fora do bedrock o que precisa sair dele (so texto obsidian e copiado para fora automaticamente), ou use abyss { } para assumir o risco
```

- **In both modes:** the rule is decided at compile time (`ofscc/sediment.ofs`, after the `typeck`), the same for native code, the interpreter, crystallization and Fault Refract. Natively, `ofs_arena_open()` on entry and `ofs_arena_close()` as a synthetic anchor; the runtime (`stdlib/runtime/reservoir.ofs`) carves blocks out of 64 KB chunks that double (zeroed like `calloc`) and keeps the largest one for the next arena. In the interpreter, the `ofs_alloc` written in the block comes from the same runtime arena and text copied out goes through the same function (`ofs_arena_lift`).
- **Measured** (200 thousand iterations with 16 strings in an `Array`, a `monolith` and 1 KB of `ofs_alloc` per iteration, native `-O2`): with `bedrock` 1.9 MB and 0.38 s; the same loop in a plain block (with `ofs_free` of the buffer) 258 MB and 0.37 s.

#### Leak warning

A `forge p = ofs_alloc(n)` (or `malloc`, `calloc`, `realloc`, `ofs_realloc`) outside an arena whose pointer never reaches a `seal`/`anchor`, `ofs_free`, a call (owner), `return`, assignment, field, `Array` or geode produces a warning (not an error) on stderr, in both modes; a bare `ofs_alloc(n)` whose result is discarded too. `OFS_LEAK_WARN=0` silences it.

```
[AVISO DE VAZAMENTO app.ofs:3:24]: 'p' recebe ofs_alloc e nunca chega a um seal/ofs_free, return ou dono (o bloco vaza)
  dica: libere com seal { ofs_free(p) } logo depois, devolva ou guarde o ponteiro, ou aloque dentro de bedrock { } (a arena libera ao sair); OFS_LEAK_WARN=0 silencia
```

Tests: `ofs/tests/regress/bedrock_arena.ofs`, `arena_memory.ofs` (20 thousand iterations of 64 KB with `ulimit -v` of 256 MB in both modes), `arena_err_*.ofs`, `leak_warn.ofs`.

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
| `as` between a vein value and an address (`f as stone`, `n as vein(...)`) | — | allows |

- Address arithmetic goes through these operations: `p + 1` on a pointer is already a `[ERRO DE TIPO]`; use `fault_step(p, n)` or `((p as stone) + n) as *T`, both inside the block.
- Still free outside a block: calling `extern`/`rift vein` (even with pointer arguments or results, e.g. `forge buf = ofs_alloc(64)`), comparing a pointer with `null`, storing and passing pointers along, the bit intrinsics (`fault_count`, `fault_cut`, `fault_weave`...) and everything about typed vein values (using a vein's name as a value, storing it, passing it to C in a `vein(...)` parameter, calling `f(x)`).
- `tectonic X` counts as `X` (`tectonic abyss { }` = `abyss`); a bare `tectonic { }` counts as `fracture`. Nested blocks add up: inside `abyss`, everything is allowed.
- A whole vein can be marked: `fracture vein read(p: *u8) -> stone { return *p }`, `abyss vein call(f: stone) -> stone { return fault_call(f) }` (also inside `namespace` and `impl`). The body counts as the block. Code that must build with an `ofscc` older than this rule (the compiler itself, because of the seed, and the stdlib) uses a `fracture { }` block inside the vein, which every compiler understands.
- In the tree the block is an `NK_BLOCK` with the name in `val` (`"fracture"`, `"abyss"`...); `llvmgen` and the interpreter run it as a plain block (same IR).

**Migration.** Until **2027-01-09**, an operation outside a block is only a warning, on stderr (in `ofs run` stdout belongs to the program), and the program builds the same:

```
[AVISO DE FALHA app.ofs:12:15]: acesso por ponteiro '*p' (leitura ou escrita) fora de bloco de baixo nivel (libera: fracture, bedrock ou abyss)
  dica: fracture { forge v = *p; *p = v + 1 }  ou marque a vein: 'fracture vein f(...)'
[AVISO DE FALHA] 1 operacao(oes) de baixo nivel fora de bloco: viram erro em 2027-01-09; OFS_STRICT_FAULT=1 ja trata como erro
```

With `OFS_STRICT_FAULT=1` (the mode of the repository tests) the same line becomes `[ERRO DE FALHA arquivo:linha:col]` and the build stops with exit code 1. From 2027-01-09 on, strict mode becomes the default. The stdlib, runtime, compiler, tools, examples and tests already build without any warning.

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

### `fault_*` intrinsics

The bit ones work anywhere; `fault_step`, `fault_memcpy` and `fault_memset` require `fracture`/`bedrock`/`abyss`, and `fault_addr`/`fault_call` require `abyss` (table above):

- `fault_count(stone)` → count of 1 bits (popcount)
- `fault_fence()` → memory barrier
- `fault_prefetch(*stone)` → memory prefetch for the pointer's target
- `fault_trap()` → machine trap
- `fault_lead(stone)` → leading zeros
- `fault_trail(stone)` → trailing zeros
- `fault_swap(stone)` → byte swap
- `fault_spin_left(stone, stone)` / `fault_spin_right(stone, stone)` → bit rotation
- `fault_step(*stone, stone)` → typed pointer advance, in elements
- `fault_addr(vein)` → address of the `vein` as a `stone` (see section 11)
- `fault_call(endereco, args...)` → indirect call using the system calling convention (see section 11)
- `fault_cut(value, shift, width)` → extracts a bit field
- `fault_patch(base, shift, width, insert)` → replaces a bit field
- `fault_weave(mask, left, right)` → mixes the bits of `left` and `right` according to `mask`
- `fault_unreachable()` → marks the path as unreachable (generates `llvm.trap` + `unreachable`)
- `fault_memcpy(dst, src, len)` → block memory copy (`llvm.memcpy`); `dst` and `src` are pointers
- `fault_memset(dst, val, len)` → fills memory with the byte `val` (`llvm.memset`); `dst` is a pointer

Exact semantics (the same in the native build and in the `ofs run` interpreter):

- Everything is 64-bit, two's complement: `fault_count(-1)` = 64; `fault_lead(0)` = `fault_trail(0)` = 64; `fault_lead(-1)` = 0.
- `fault_spin_left`/`fault_spin_right`: 64-bit rotation (`llvm.fshl`/`llvm.fshr`); the distance is taken modulo 64 (`fault_spin_right(6, 65)` = 3).
- `fault_weave(mask, left, right)` = `(left & mask) | (right & ~mask)`: the bit comes from `left` where `mask` has 1 and from `right` where it has 0.
- `fault_trap()` and `fault_unreachable()` end the process with SIGILL (the interpreter does the same with `raise(SIGILL)`).
- `fault_fence()`/`fault_prefetch()` do nothing in the interpreter.
- In the interpreter pointers are real addresses: `fault_memcpy`/`fault_memset` copy/fill any size, and `fault_step`, `fault_addr` and `fault_call` work as in native code (see "In the interpreter (`ofs run`): the Rift").
- Test: `ofs/tests/regress/fault_intrinsics.ofs`.

These intrinsics cover operations that assembly already has and give OFS its own machine-oriented vocabulary.

### `quartz`: inline assembly

`quartz` emits assembly directly into the LLVM IR, with a side effect (`sideeffect`) by default. It is only allowed in `abyss { ... }` or `abyss vein`; `fracture` and `bedrock` alone do not authorize assembly. `asm` remains the CLI command `ofs asm`, which generates a `.s` file.

```ofs
abyss vein somar(a: stone, b: stone) -> stone {
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
    echo(somar(20, 22)) // 42 (x86-64)
}
```

- No output: `quartz { "nop" }`. With output: `quartz { ... } -> variable "=r"`; declare the scalar variable beforehand. The output constraint is optional (`=r` by default) and goes on the same line as the destination. `+r` reads and writes the destination; `=&r` marks an early-clobber output, which cannot share a register with an input.
- Inputs: `in "r" expression`, `in "{rdi}" expression` or `in "m" variable` (memory of a scalar variable). A single scalar output can be bound to a register. `$0` is the output, when there is one; the following inputs are `$1`, `$2`, etc. With no output, the first input is `$0`.
- Clobbers: put `melt "rdx" "memory"` after the instructions and inputs to declare altered registers and memory. LLVM does not discover these effects from the text; declare every additional effect.
- The default dialect is AT&T; `quartz att { ... }` is explicit and `quartz intel { ... }` selects Intel. A literal `$` is written `$$`: `"shlq $$32, %rdx"`.
- The text is architecture-specific. `OFSCC_TARGET` selects the target but does not translate x86-64 instructions to AArch64/Android. Use assembly appropriate to the target.
- In `ofs build`, LLVM compiles the snippet. In `ofs run`, the vein that contains `quartz` is loaded as native code in the same process; see [Execution model](COMPILED_LANGUAGE.md).

### Live editing: `ofs refract` (Fault Refract)

`ofs refract app.ofs` runs the program interpreted and, while it is open, swaps the veins you edit and save. The unit of swapping is the **vein**:

- The swap happens **between calls**: the next call of the vein uses the new version. A **loop** running in an edited vein (including the `while` of `main`) swaps at the next header (**OSR**, wave 4): the new loop continues with the local variables as they are, and the rest of the vein after it is also the new one; a new variable before the loop gets its initial value at swap time. If a variable before the loop changed type (or the loop is inside `cycle`/`match`/`bedrock`), the loop ends on the old version and the new one applies on the next call. Outside a loop, the running call ends on the old version.
- **State** carries on: existing globals keep their value (the initializer does not run again), and live arrays and objects do not change. A new global is created with its initial value.
- A **crystallized** (native) vein or one with `quartz` is recompiled into a new `.so`, together with the native veins that call it; the new version runs interpreted until the `.so` is ready (those with `quartz`, which cannot run interpreted, are compiled before the swap).
- The whole new program is checked first (syntax and types): with an error, nothing changes.
- A **compatible signature** swaps even with the vein in use: a new parameter at the end with a default value; the same return, which was `void`, a narrow integer where it was `stone`, `f32` where it was `crystal`.
- **A `monolith` with live instances migrates**: a new field gets the type's default value, a removed field disappears, the order may change and a numeric field may change type (the value is converted). Instances are rewritten in place.
- **Need a restart** (the swap is refused and the program continues with the old version): changing the signature incompatibly in a vein that a running vein still calls, or of a vein handed to C as a callback or used as a value; removing a vein that something running still calls; changing a live `monolith` field to a type that does not convert (e.g. `stone` → `obsidian`) or growing the `monolith` by more than 64 bytes; changing the type of a global; changing an `extern`; removing or reordering `strata` variants (adding at the end is allowed).

Example: `ofs/examples/refract_demo.ofs`. Command, messages and variables: [TOOLS.md](TOOLS.md). Mechanism: [COMPILER_ARCHITECTURE.md](COMPILER_ARCHITECTURE.md).

### Stratum and per-module permissions (`stratum`, `claim`, `caprock`)

Gradual safety (Idea 4) and per-module permissions (A5.4), as a single rule. Each file (the main one and every `attach`) is a **module** and declares at the top, before the code (comments, blank lines and `attach` may come before or between these lines), **how much** low-level code it uses and **which kind**:

```ofs
// raw_io.ofs
stratum fault          // optional: fault is the default
claim rift, abyss      // what the module uses

extern vein getenv(name: obsidian) -> obsidian
```

| Stratum | Low level | Blocks | `claim` |
|---|---|---|---|
| `stratum crust` | none: `*p`, `&x`, pointer `as`, memory `fault_*`, `extern`/`rift vein`, `quartz` are `[ERRO DE ESTRATO]` | `fracture`/`tectonic`/`abyss` are errors; `bedrock` only works as an arena (it does not unlock pointers) | none (`claim` in a `crust` file is an error) |
| `stratum fault` (default) | only inside the block that unlocks it (F4 rule, table above) | as in F4 | asks for what it uses |
| `stratum abyss` | anywhere in the file, no block needed | optional | counts as `claim rift, abyss, quartz` |

| Permission | Unlocks | Example |
|---|---|---|
| `claim rift` | declaring `extern vein` and `rift vein` (system calling convention, `abi c`) | `extern vein fopen(...)` |
| `claim abyss` | `abyss { }` blocks and `abyss vein` | `abyss { forge r = fault_call(f) }` |
| `claim quartz` | `quartz { }` (which still requires `abyss`, so `claim abyss` comes along) | `claim abyss, quartz` |
| `claim magma` | calling `magma.cast` (compile and load code at runtime; Idea 5). Missing it is always an error | `claim magma` |

- `fracture`/`bedrock`/`tectonic` need no claim: the `fault` stratum already accepts them; the `crust` stratum rejects them.
- `extern`/`rift vein` of an OFS runtime symbol (name starting with `ofs_`, e.g. `ofs_str_len`) needs no `claim rift`: it is the language itself, not foreign C code.
- **Whoever attaches a module sees what it asks for:** `ofs check` lists each module's stratum and claims (`[usa sem claim: ...]` and `[claim sem uso: ...]` when they do not match), and `ofs doc` shows `stratum`/`claim`/`caprock` badges on the module page.
- **The main program restricts attached modules** with `caprock` (main program only; in a module it is an error): `caprock rift, abyss` lets attached modules ask for at most that; `caprock crust` lets them ask for nothing. An attached module that asks for, uses, or is `stratum abyss` beyond the caprock is `[ERRO DE PERMISSAO main:line]` (always an error: caprock is opt-in). The runtime (`stdlib/runtime/`) is outside the caprock; so is the main program itself (its claims apply to itself).
- Relation with blocks: the stratum says **where** low-level code may appear (nowhere / in blocks / anywhere), blocks mark **the span**, and the claim says **which** power the module uses and is what attachers see. `abyss` names the block, the permission and the deepest stratum: the block unlocks a span, `claim abyss` allows the module to contain such a block, `stratum abyss` makes the whole file one block.
- Comment form: `//! claim rift` (and `//! stratum ...`) means the same. It is for code that must also build with an `ofscc` older than wave 5: the compiler itself (`ofs/ofscc/`), because of the `ofs/dist/ofscc` seed.
- The driver reads these lines as text (like `attach`) and does not pass them to the lexer; a directive after code, an unknown word (`claim lava`) or a repeated `stratum` is `[ERRO DE ESTRATO file:line]`.

**Migration (same as F4).** Until **2027-03-16**, use without the claim is only a warning on stderr, one message per module and permission, and the program builds the same:

```
[AVISO DE PERMISSAO app.ofs:3:17]: 'extern vein labs' sem 'claim rift' no topo do arquivo
  dica: acrescente a linha 'claim rift' no topo de app.ofs (quem anexa o modulo ve o que ele pede no ofs check)
[AVISO DE PERMISSAO] 1 uso(s) de baixo nivel sem 'claim' no topo do modulo: viram erro em 2027-03-16; OFS_STRICT_FAULT=1 ja trata como erro
```

With `OFS_STRICT_FAULT=1` (the same switch as F4, the repository test mode) it becomes `[ERRO DE PERMISSAO arquivo:linha:col]`. Stratum (`crust`) and `caprock` violations are always errors, because they only apply to whoever declares them. The repository's stdlib, runtime, compiler, tools, examples and tests already declare what they use and pass strict mode. Tests: `ofs/tests/regress/stratum_*.ofs`, `claim_*.ofs` and `stratum_mods/`.

### Compiling and loading OFS at runtime: `magma.cast` (Idea 5)

A program (under `ofs run` **and** as an `ofs build` binary) compiles a **mod** written in OFS while it runs and calls its veins: plugins, game scripts, mods. A mod is an OFS file with veins only (no `core main`; stdlib `attach` works). Stdlib module: `attach {magma}`.

```ofs
attach {magma}
claim magma                       // loading code at runtime is a permission

core main() {
    forge m = magma.cast("mods/forja.ofs")      // a path, or the source itself (text with \n)
    if (!magma.cooled(m)) {
        echo(magma.crack(m))                    // the compiler message; the program keeps going
        return
    }
    forge forjar: vein(stone) -> stone = magma.vent(m, "forjar")
    echo(forjar(10))
    forge erro: vein(crystal) -> stone = magma.vent(m, "forjar")
    if (erro == null) { echo(magma.crack(m)) }  // the mod's signature differs from the requested one
    magma.release(m)
}
```

- `magma.cast(source)` → a `stone` handle (always ≥ 1). A `source` with a line break is the code itself; otherwise it is a file path. Syntax, type, permission or load errors never take the program down: `magma.cooled(m)` is `false` and `magma.crack(m)` holds the compiler message (text sources show up as `<fonte>`).
- `magma.vent(m, "name")` → a **typed** vein value (F6). The type is the one of the place that stores the value (`forge f: vein(T) -> R = ...`, `=`, argument, `return`): the compiler passes it as the requested signature, and at runtime it is checked against the one declared in the mod (the `.so` manifest). Mismatch or missing vein: the value is `null` and `magma.crack(m)` says what differs. With no destination type (`forge f = magma.vent(...)`) it is a `[ERRO DE TIPO]`. Namespace veins use the program name (`"lava.grau"`).
- `magma.veins(m)` lists `name: signature`; `magma.release(m)` stops the mod from handing out veins (the code **stays loaded**: vein values, strings and addresses already handed out may be stored, the same caution as Refract wave 4); `magma.caprock("crust")` is the ceiling for the next mods (`"rift"`, `"rift, abyss"`…; `""` removes it).
- **Globals:** the mod does not see the program's globals and vice versa; they talk through arguments, return values and vein values (a mod may receive a program vein as a callback). The mod's globals are its own and start at load time. The same source (same generated code) gives the same `.so` and the **same module** (shared state); an edited mod is another `.so`, and the old version stays valid for whoever kept its veins.
- **Boundary:** signature types are the F6 ones (scalars, `obsidian`, `Array<T>`, vein values, `monolith` by name: both sides must declare it the same way). Rift limits under `ofs run`: up to 12 integer parameters, or up to 4 with `crystal`.
- **Security:** calling `magma.cast` requires `claim magma` at the top of the file; it is **always an error** (`[ERRO DE PERMISSAO]`, no migration). In a `stratum crust` file it is `[ERRO DE ESTRATO]`; `stratum abyss` counts as `claim magma`. The `magma` module declares `claim rift, abyss, magma`, so a `caprock` without `magma` forbids attaching it. The mod is compiled in strict mode (`OFS_STRICT_FAULT=1`) with the `magma.caprock` ceiling applied to all of it (including the mod file and its `attach`es).
- **How it works:** the mod is compiled by `ofscc` in a separate process (`OFSCC_MODE=cast`: the `ir` output plus the `@__magma_manifest`), turned into a `.so` by clang (`-shared -fPIC -O2 -Wl,-Bsymbolic`) in a **cache keyed by the hash** of the generated IR (expanded source with its `attach`es, clang and system; `OFS_MAGMA_CACHE`, `$XDG_CACHE_HOME/ofs/magma` or `~/.cache/ofs/magma`) and opened with `dlopen` (`RTLD_LOCAL`); the mod's globals are initialized once (`__ofs_init_globals`). The mod uses the program's runtime (same heap, stdout and strings): under `ofs run` the `ofscc` is already linked with `-rdynamic`; `ofs build` links programs that use `magma.cast` with `--export-dynamic` (linking by hand: `-rdynamic`; without it, `magma.crack` says how).
- **Compiler and clang:** `OFS_MAGMA_OFSCC`; under `ofs run`, the running `ofscc` itself; otherwise `$OFSCC`, the `ofscc` next to the `ofs` on `PATH`, `~/.local/lib/ofs/ofscc`, `ofscc` on `PATH`, `/usr/local/bin`, `/usr/lib/ofs`. The mod's stdlib: `OFS_STDLIB_PATH` or the folder next to `ofscc`. clang: `OFS_MAGMA_CC` (or `OFS_QUARTZ_CC`), else `clang`. If one is missing, the error comes back in `magma.crack`. `OFS_VERBOSE=1` shows the `.so` and whether it came from the cache. Linux and macOS (on Windows, `magma.crack` says it does not exist yet).
- Example: `ofs/examples/magma_cast_demo.ofs` (mod in `ofs/examples/magma_mods/forja.ofs`). Tests: `ofs/tests/regress/magma/` and `magma_err_*.ofs`.

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

### Standard library modules

| Name | Description |
|---|---|
| `core` | Math helpers, predicates, factorial, Fibonacci |
| `math` | Primes, digit utilities, number reversal |
| `string` | String utilities: repeat, starts_with, is_empty |
| `io` | prompt, print_separator, print_header |
| `webserver` | HTTP response builder, MIME types, log helpers |
| `bedrock` | Cells, regions and bit-field helpers on the heap |
| `bedrock-packet` | Packet layout helpers over `bedrock` regions |
| `rift` | Rift wrappers (native library helpers) |
| `terminal-colors` | ANSI color output in the terminal |
| `memory-modes` | Documentation and helpers for the fracture/abyss/fractal modes |
| `test-lib` | Unit test utilities |

### Math functions (`stdlib/core.ofs`)

```ofs
vein abs(x: stone) -> stone
vein max(a: stone, b: stone) -> stone
vein min(a: stone, b: stone) -> stone
vein clamp(val: stone, lo: stone, hi: stone) -> stone
```

### `bedrock.ofs` helpers

- Cells: `bedrock_cell_new`, `bedrock_cell_read`, `bedrock_cell_write`, `bedrock_cell_add`, `bedrock_cell_sub`, `bedrock_cell_drop`.
- Regions: `bedrock_region_new`, `bedrock_region_read`, `bedrock_region_write`, `bedrock_region_add`, `bedrock_region_drop`, `bedrock_prefetch`.
- Views: `bedrock_view_at`, `bedrock_view_read`, `bedrock_view_write`.
- Fields and lanes: `bedrock_field_cut`, `bedrock_field_patch`, `bedrock_lane8_get/set`, `bedrock_lane16_le/be_get/set`, `bedrock_lane32_le/be_get/set`.
- Barrier: `bedrock_barrier`.

The purpose of this module is not to imitate another language's APIs, but to make low-level programming grow in OFS's own vocabulary. The lane helpers give an endianness-aware surface for ABI-oriented bit windows, without requiring manual shifts.

### `rift.ofs` and `bedrock_packet.ofs` helpers

- `rift.ofs`: `rift_text_size`, `rift_line`, `rift_banner`, `rift_report_line`. Lets you consume native system libraries through OFS APIs.
- `bedrock_packet.ofs`: `bedrock_packet_store`, `bedrock_packet_header`, `bedrock_packet_payload`, `bedrock_packet_opcode`, `bedrock_packet_lane`, `bedrock_packet_opcode_be16`, `bedrock_packet_tail_le16`, `bedrock_packet_set_opcode`, `bedrock_packet_set_lane`. Header and payload views over `bedrock` regions.

### OFS native runtime (Stack Magma)

The main implementation of the runtime and the standard library is written in OFS (`stdlib/runtime/`), with no runtime in another language:

| Subsystem | Module | Responsibility |
|---|---|---|
| **Magma** | `magma.ofs` | Bootstrap and orchestration of native runtime execution |
| **Reservoir** | `reservoir.ofs` | Memory allocation and management (`ofs_alloc`, `ofs_free`) |
| **Facet** | `facet.ofs` | Text and numeric conversion (`ofs_str_*`, `ofs_*_to_*`) |
| **Matrix** | `matrix.ofs` | Dynamic collections and vectors (`ofs_array_*`) |
| **Outlet** | `outlet.ofs` | Terminal input and output (`echo_*`, ANSI colors, formatting) |
| **Foundation** | `foundation.ofs` | Operating system and environment (`ofs_getenv`, `ofs_mod`, `ofs_exit`) |
| **Pulse** | `pulse.ofs` | High-precision monotonic clock and timers (`now_ms`, `sleep_ms`) |
| **Frame** | `frame.ofs` | Native window and surface management |
| **Impulse** | `impulse.ofs` | Input events: mouse and keyboard |
| **Prism** | `prism.ofs` | 2D pixel rasterization, geometry and surfaces |

All runtime primitives are compiled by `ofscc` directly to native LLVM IR.

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
