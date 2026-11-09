# OFS Compiler Architecture

## High-Level Overview

```
source.ofs
    │
    ├─────────────────────────────────────────┐
    │                                         │
    ▼                                         ▼
┌──────────────┐                    ┌────────────────────┐
│   Lexer      │ → Array<Token>     │  Symbol Table      │
│  (chars,     │   (87 kinds)       │  (scope stack)     │
│   tokens)    │                    └────────────────────┘
└──────┬───────┘
       │
       ▼
┌──────────────┐
│   Parser     │ → AST Root ID (stone)
│  (recursive  │   (48 node kinds)
│   descent +  │   (pool-based allocation)
│   Pratt)     │
└──────┬───────┘
       │
       ▼
┌──────────────┐
│  TypeChecker │ → Annotated AST
│  (with scope │   (type_str on each node)
│   stack)     │
└──────┬───────┘
       │
       ├─────────────────────────────────┐
       │ (ofs run / interpreted)         │ (ofs build / native)
       ▼                                 ▼
┌──────────────┐                  ┌──────────────┐
│ Interpreter  │                  │  LLVM Gen    │ → output.ll (LLVM IR nativo)
│(interpreter  │                  │ (llvmgen.ofs)│   (mapeamento OFS → LLVM IR SSA)
│    .ofs)     │                  └──────┬───────┘
└──────┬───────┘                         │
       │                                 ▼
       ▼                             llc -filetype=obj output.ll -o output.o
 Direct Execution                        │
(instant evaluation)                     ▼
                             ld / ld.lld output.o magma.o → executável nativo
```

---

## Phase 1: Lexer (Tokenization)

**File**: `ofs/ofscc/lexer.ofs` (433 lines)

### Input
```ofs
forge x: stone = 42
```

### Processing

1. **Character classification** (`chars.ofs`):
   ```ofs
   is_digit(c), is_alpha(c), is_alnum(c), is_space(c)
   ```

2. **Token recognition**:
   - Keywords (38): forge, vein, monolith, if, cycle, match/assay, strata, attach, etc.
   - Operators (25+): +, -, *, /, ==, !=, &&, ||, &, |, ^, ~, %, ?, etc.
   - Literals: 42, 3.14, "hello\n", true, false, null
   - Identifiers: x, _var, CamelCase
   - Comments: // and /* */

3. **Multi-character operators**:
   - ++ / -- / += / -= / *= /= / == / != / <= / >= / && / || / << / >> / -> / =>

### Output
```
Token { kind: TOK_FORGE, val: "forge", line: 1, col: 1 }
Token { kind: TOK_IDENT, val: "x", line: 1, col: 7 }
Token { kind: TOK_COLON, val: ":", line: 1, col: 8 }
Token { kind: TOK_IDENT, val: "stone", line: 1, col: 10 }
Token { kind: TOK_EQ, val: "=", line: 1, col: 16 }
Token { kind: TOK_INT, val: "42", line: 1, col: 18 }
Token { kind: TOK_EOF, val: "", line: 1, col: 21 }
```

### Key Design Decisions

- **No lexer actions** — Just tokenize, don't parse yet
- **Line/column tracking** — For error messages
- **Escape sequences** — \n, \t, \\, \", \r, \0 handled
- **Hex/float literals** — 0xFF, 1.0e-5 supported

---

## Phase 2: Parser (AST Construction)

**file**: `ofs/ofscc/parser.ofs` (1,246 lines)

### Strategy

1. **Recursive Descent** for statements + declarations
2. **Pratt Parser** for expressions (operator precedence)
3. **Pool-based AST** — nodes stored in global Array<Node>

### Grammar (Simplified)

```
program     ::= (fn_decl | monolith_decl | impl_decl | namespace_decl
                | strata_decl | extern_decl | attach_decl | var_decl)*

fn_decl     ::= ("vein" | "core") IDENT "(" params? ")" ("->" type)? block
params      ::= IDENT ":" type ("," IDENT ":" type)*
extern_decl ::= ("extern" | "rift") "vein" IDENT "(" params? ")" ("->" type)?
                ("bind" STRING)? ("abi" IDENT)?
monolith_decl ::= "monolith" IDENT "{" (IDENT ":" type (","|";")?)* "}"
impl_decl   ::= "impl" IDENT "{" fn_decl* "}"
namespace_decl ::= "namespace" IDENT "{" (fn_decl | var_decl | monolith_decl
                | extern_decl | impl_decl | strata_decl)* "}"
strata_decl ::= "strata" IDENT "{" (IDENT ("(" type ")")? (","|";")?)* "}"
attach_decl ::= "attach" "{" path "}"          // e.g. attach {core}, attach {F:lib.ofs}
var_decl    ::= ("forge" | "const" | "shard") IDENT (":" type)? ("=" expr)? ";"?

type        ::= IDENT | "Array" "<" type ">" | "*" type

stmt        ::= var_decl | if_stmt | while_stmt | cycle_stmt | mine_stmt
              | match_stmt | return_stmt | throw_stmt | tremor_stmt
              | anchor_stmt | unsafe_block | block | "break" | "continue"
              | "shatter" | "erode" | expr ";"?
if_stmt     ::= "if" "(" expr ")" block ("else" (if_stmt | block))?
while_stmt  ::= "while" "(" expr ")" block
cycle_stmt  ::= "cycle" "(" IDENT "in" expr ")" block          // over a collection
              | "cycle" "(" "forge" IDENT "in" expr ")" block
              | "cycle" "(" init? ";" expr? ";" expr? ")" block // C style
mine_stmt   ::= "mine" "(" IDENT "in" expr ")" block
match_stmt  ::= ("match" | "assay") "(" expr ")" "{" (pattern "=>" (block | stmt))* "}"
pattern     ::= expr | "_"                                      // "_" is the default case
return_stmt ::= "return" expr? ";"?
throw_stmt  ::= "throw" expr ";"?
tremor_stmt ::= "tremor" block ("catch" "(" IDENT (":" type)? ")" block)?
anchor_stmt ::= "anchor" stmt
unsafe_block::= ("fracture" | "bedrock" | "abyss" | "tectonic") ("fracture"|"bedrock"|"abyss")? block
block       ::= "{" stmt* "}"

expr        ::= unary (binop expr)*         // precedence: see table below
unary       ::= ("-" | "!" | "~" | "&" | "*") postfix(primary)
postfix     ::= primary ( "(" args? ")" | "[" expr "]" | "." IDENT
                        | "++" | "--" | "?" | "as" type )*
primary     ::= IDENT | INT | FLOAT | STRING | "true" | "false" | "null"
              | "(" expr ")" | "[" (expr ("," expr)*)? "]" | "echo" "(" args? ")"
```

Points the grammar does not show:

- The parentheses of `if`, `while`, `match`/`assay` and `cycle`/`mine` are required. `if x == 1 { }` fails with `expected '('`. The body must be a `{ }` block.
- The semicolon is optional at the end of statements and declarations. It is only required inside a C-style `cycle`.
- `assay` is just another name for `match` (same token). `match` becomes a chain of `if` comparing with `==`; `_` is the default case and `=>` separates pattern and body. There are no destructuring patterns.
- `shatter` and `erode` are synonyms of `break` and `continue`. `mine` works like `cycle ... in` but only accepts the `item in collection` form.
- Namespace calls (`ns.fn(x)`) become the symbol `ns__fn`. `impl` methods become `Type__method`.
- `attach {path}` is only valid at file level. The text between the braces is glued token by token (`F:` marks a file path).
- There is no struct literal (`P { x: 1 }`) and no `monolith ... layout ...`; the parser expects `{` right after the `monolith` name. Declare the variable (`forge p: P`) and assign the fields.
- `core` is a reserved word: it marks the entry point (`core main()`) and cannot be used as a name.

### AST Node Pool

All nodes stored in global array with numeric IDs:

```ofs
monolith Node {
    kind: stone        // NK_* constant
    val: obsidian      // name or literal value
    type_str: obsidian // from type checker
    line, col: stone   // source position
    c0, c1, c2, c3: stone  // child indices (fixed)
    extra: Array<stone>    // additional children
}

forge _node_pool: Array<Node> = []
```

### Example: Parse `vein add(a: stone) -> stone { return a }`

```
NK_FN_DECL
├─ val: "add"
├─ c0: 2 (params block)
│  └─ extra: [3] (param node)
│     ├─ kind: NK_PARAM
│     ├─ val: "a"
│     └─ c0: 4 (type node)
│        └─ kind: NK_TYPE, val: "stone"
├─ c1: 5 (return type)
│  └─ kind: NK_TYPE, val: "stone"
└─ c2: 6 (body block)
   └─ kind: NK_BLOCK
      └─ extra: [7] (return stmt)
         ├─ kind: NK_RETURN
         └─ c0: 8 (ident node)
            └─ kind: NK_IDENT, val: "a"
```

### Precedence Table (Pratt Parser)

```
Precedence Order (lowest to highest):
1.  = += -= *= /=   (assignment)
2.  ||              (logical OR)
3.  &&              (logical AND)
4.  == !=           (equality)
5.  < <= > >=       (comparison)
6.  |               (bitwise OR)
7.  ^               (bitwise XOR)
8.  &               (bitwise AND)
9.  << >>           (bit shift)
10. + -             (addition)
11. * / %           (multiplication)
12. Unary - ! ~ & *  (prefix)
13. ++ -- ? as    (postfix: increment, propagation, type cast)
14. [] . ()         (postfix: index, field, call)
```

---

## Phase 3: Type Checker

**File**: `ofs/ofscc/typeck.ofs` (825 lines)

### Symbol Table

```ofs
monolith Symbol {
    name: obsidian       // variable/function name
    type_str: obsidian   // "stone", "obsidian", "Array<stone>", etc
    kind: stone          // SYM_VAR, SYM_CONST, SYM_FN, SYM_PARAM
    fn_arity: stone      // number of parameters
    is_const: bool       // immutable?
}
```

### Scope Stack

```ofs
forge _scopes: Array<Array<Symbol>> = []

// Push scope when entering block/function
push_scope()

// Pop scope when exiting
pop_scope()

// Declare symbol in current scope
declare(sym)

// Resolve symbol (searches outer scopes)
resolve(name)
```

### Type Checking Rules

1. **Literals**:
   - `42` → stone
   - `3.14` → crystal
   - `"hello"` → obsidian
   - `true`/`false` → bool

2. **Binary operations**:
   - Arithmetic (+ - * /): both operands must be numeric
   - Comparison (== != < <= > >=): return bool
   - Logical (&& ||): operands and result are bool

3. **Variable declarations**:
   - `forge x: stone = 42` → explicit type
   - `forge y = 42` → inferred as stone
   - `const z = 100` → inferred, immutable

4. **Function calls**:
   - Arguments count must match parameters
   - Argument types should be compatible with parameters

### Error Handling (Current)

- Warnings only (no `tremor`/`catch` yet)
- Line/column reported
- Message: `[typeck warn line:col] description`

---

## Phase 4A: AST Interpreter Backend (Interpretação Direta)

**File**: `ofs/ofscc/interpreter.ofs` (70KB, 2,086 lines)

The OFS interpreter backend runs the typed syntax tree (AST) directly in memory, without generating intermediate files, C code or LLVM IR. It is the default backend used by `ofs <file.ofs>` and `ofs run <file.ofs>`.

### Interpreter architecture

1. **`Value` system**: a universal monolith that unifies scalar values (`stone`, `crystal`, `obsidian`, `bool`, `null`) and virtual heap handles for `Array<T>`, `monolith` objects and pointers.
2. **Environment and call frames**: strict lexical scope based on frame markers (`_current_frame_base` and `_globals_count`), so callers' variables do not leak into callees or globals.
3. **Value semantics**: `forge y: T = x` and `y = x` copy the `monolith` block (`obj_clone()`/`memcpy` into the block `y` already has, like the native `store %T`); vein parameters receive the `monolith` by reference, like the native pointer.
4. **Method dispatch**: methods defined in `impl Type` blocks are resolved automatically, with `self` injected.
5. **Real pointers (F2)**: a pointer is an address (`VAL_INT` with type `*T`). `*p`/`*p = v` use `mem_load`/`mem_store` by the memory class of the pointee type (`MC_*`: width, sign, `f32`, `obsidian`, `*T`, `monolith`, variant), with the `typeck` static type precomputed per node in `annotate_nodes` (the same type `llvmgen` uses); the load/store is the interpreter's own native code (`fracture { *p8 = v as u8 }`). `&x` of a variable turns the entry into `VAL_BOXED`: the value moves to an 8-byte slot (`box_var`), read/written by `env_load`/`env_store` and freed in `env_pop_one` (`_boxed_live` keeps the fast path free when there is none). `as` mirrors `llvmgen`'s `NK_CAST` (`eval_cast`). A `monolith` lives in raw memory with the `llvmgen.struct_layout` layout (the same function computes the native `ofs_alloc` size and the interpreter's offsets; `compute_layouts`), and `VAL_OBJECT` holds the block address; an `Array` field holds the interpreted handle as `-(h+1)`.
6. **Built-ins**: direct handling of I/O (`echo`), strings (`ofs_str_*`), arrays (`ofs_array_*`) and math (`ofs_pow`, `ofs_sqrt`). The list (`is_builtin`) is only a fast path, not a limit on what can be called.
7. **Rift (`extern vein`/`rift vein`)**: at load time (`collect_declarations` → `resolve_calls` → `ffi_check_symbols`), every called extern becomes an address via `dlsym(RTLD_DEFAULT, sym)` (honoring `bind`), after opening `libm` and `OFS_FFI_LIBS`; a missing symbol is the linker's error (`undefined reference`), exit code 1. Veins from `stdlib/runtime/*.ofs` (line ranges marked by `expand_attaches` through `ast_mark_runtime_lines`) use the native version linked into `ofscc`. The call (`call_extern`) converts arguments by the declared type (integers/pointers/`obsidian` as `i64`, `crystal` as `double`), stores them in a buffer and calls `__ofs_ffi_dispatch`, which `llvmgen` defines inside `ofscc` itself (`emit_ffi_dispatch`, when the module declares that extern): a `switch` with one call of exact LLVM type per signature class (`ffi_sig`/`ffi_sig_supported`: up to 4 `i64|double` args with an `i64|double` return, up to 12 integer-only, variadics with 1 to 3 fixed + up to 4 extra; 155 cases). That is why `ofscc` must be linked with `-rdynamic` (it exports the runtime and the dispatcher to `dlsym`). In F2, `&x`, `monolith` (the block) and `Array` go to the native function as addresses: the `Array` is the interpreter's value column, which already is a native `ofs_array` (`arr_native_ptr`), with its length synchronized on return (`ffi_sync_arrays`); an `Array` returned by the native function becomes an interpreted array that uses the same block (`arr_adopt`). native → OFS callbacks: the module that declares `extern vein __ofs_cb_table` gets from `llvmgen` (`emit_cb_trampolines`) 16 functions `__ofs_cb_K(i64 x 6)` that call `__ofs_cb_enter(K, ...)`, defined at the end of `interpreter.ofs`; `fault_addr` of an interpreted vein binds the vein to a slot (`cb_slot_for`) and returns the address of `__ofs_cb_K` (found via `dlsym`, so an `ofscc` built by an older compiler still links); `cb_enter` pushes the arguments by each parameter's type, runs the vein and returns the result as `i64`, preserving the interpreter registers (re-entrant: it works inside a native call made by the interpreter). Exhausted slots, more than 6 parameters or `crystal`/`f32` in the callback are a clear error (exit code 1).

---

## Phase 4B: LLVM IR Generator (Geração Nativa)

**File**: `ofs/ofscc/llvmgen.ofs` (74KB, 2,509 lines)

O backend de compilação da OFS gera diretamente LLVM Intermediate Representation (IR), dispensando transpiladores para C ou dependências do Clang. É utilizado pelo comando `ofs build`.

### Type Mapping (OFS → LLVM IR)

```ofs
stone      → i64
crystal    → double
obsidian   → i8* (ponteiro para sequência UTF-8)
bool       → i1
void       → void
u8, u16, u32, u64 → i8, i16, i32, i64
i8, i32    → i8, i32
Array<T>   → %struct.Array* (vetor dinâmico do Stack Magma)
```

### Estratégia de Emissão LLVM IR

1. **Target Datalayout & Triple** — Configuração da arquitetura de destino (x86_64 ELF ou Windows PE32+)
2. **Declarações Externas** — Protótipos das rotinas do Stack Magma (`ofs_alloc`, `ofs_str_concat`, `ofs_array_push`, etc.)
3. **Definições de Funções** — Blocos básicos rotulados (`entry:`, etc.), alocações de variáveis (`alloca`), instruções SSA (`load`, `store`, `add`, `icmp`, `br`, `call`)
4. **Constantes de String** — Globais imutáveis `@.str.N = private unnamed_addr constant [...]`

### Exemplo: `forge x = 42`

**LLVM IR Gerado**:
```llvm
%x = alloca i64, align 8
store i64 42, i64* %x, align 8
```

### Exemplo: `vein add(a: stone, b: stone) -> stone { return a + b }`

**LLVM IR Gerado**:
```llvm
define i64 @add(i64 %a, i64 %b) {
entry:
  %0 = add i64 %a, %b
  ret i64 %0
}
```

---

### Alvos (`OFSCC_TARGET`)

Sem a variável, o IR sai com o triple do host (x86-64 Linux/Windows/macOS), idêntico ao de antes. `OFSCC_TARGET=android` emite `aarch64-unknown-linux-android26` (ou passe um triple completo). O runtime `magma` usa só a libc, então para o Android basta gerar o IR do `magma.ofs` com o mesmo triple e compilar com o clang do NDK; a `.so` é linkada com `-Wl,-init,__ofs_init_globals` porque não há `main()`. Ver [ANDROID.md](ANDROID.md).

Um `extern vein` que o próprio módulo define não é mais declarado de novo (antes: `invalid redefinition`).

### Endereço de função (`fault_addr`, `fault_call`)

`fault_addr(nome)`: o typeck troca o argumento pelo símbolo mangled (`ns.fn` → `ns__fn`) e exige que seja uma vein (`SYM_FN`); o llvmgen resolve `bind` de `extern`/`rift` e emite `ptrtoint ptr @simbolo to i64`. `fault_call(endereco, args...)` emite `inttoptr` + `call i64 %p(...)` com os tipos LLVM dos argumentos. Ver [LANGUAGE_REFERENCE.md](LANGUAGE_REFERENCE.md).

## Phase 5: Driver do Compilador

**File**: `ofs/ofscc/ofscc.ofs`

### Pipeline de Execução Self-Hosted

```ofs
core main() {
    forge src = read_file(input_file)
    src = expand_attaches(input_file, src)
    
    forge tokens = lexer.lex(src)
    forge root_id = parser.parse(tokens)
    typeck.check(root_id)
    
    llvmgen.generate(root_id, ll_output)
    
    system("llc -filetype=obj " + ll_output + " -o " + obj_output)
    system("ld " + obj_output + " ofs/dist/magma.o -lc -lm -o " + output_file)
}
```

---

## Memory Model

### Static Allocation

- Token array: ~10KB (typical)
- Node pool: ~1MB (large programs)
- Symbol table: ~100KB

### Dynamic Allocation (via Rift)

```ofs
extern vein ofs_alloc(size: stone) -> stone
extern vein ofs_free(ptr: stone) -> void
```

Used for:
- String concatenation (minimal)
- Array growth (Array<stone> etc)

---

## Determinism Guarantees

For bootstrap to work (`ofscc_v2 === ofscc_v3`):

**Deterministic**:
- Token order matches source order
- AST order matches source order  
- Symbol table ordered byInsertion
- No timestamps or random data

**Non-deterministic** (avoid):
- Hash tables (unpredictable iteration)
- Floating-point without rounding
- Embedded timestamps
- Address-dependent output

---

## Known Limitations

### Current

1. **No argv parsing** — Command-line args hardcoded
2. **Single-pass type checking** — No forward declarations
3. **Limited error recovery** — Errors halt compilation
4. **No diagnostics** — Just echo messages

### Planned

1. Add Interpreter backend (`interpreter.ofs`) for instant evaluation via `ofs run`
2. Implement OIR (Obsidian Intermediate Representation) lowering layer
3. Add tremor/catch error handling and unwinding
4. Path-aware error messages and diagnostics
5. Native graphics, windowing, and audio standard modules

---

## Testing Strategy

| Level | Files | Purpose |
|---|---|---|
| **Unit** | `test_lexer_units.ofs` | Char classification, keywords |
| **Integration** | `test_lexer_parser.ofs` | Lexer + Parser together |
| **E2E** | `test_bootstrap.sh` | Full 3-pass bootstrap |
| **Regression** | `examples/` | Real programs |

---

## Performance

| Phase | Time (typical) | Limiting Factor |
|---|---|---|
| Lexer | <1ms | File I/O |
| Parser | ~10ms | Node allocation |
| TypeCheck | ~5ms | Symbol lookup |
| LLVMGen | ~10ms | SSA IR generation |
| LLC (Assembler) | ~35ms | Machine code emission |
| Linker (ld / ld.lld) | ~15ms | Object linking with `magma.o` |
| **Total** | ~70ms | Native machine code generation |

---

## Active Architecture: Direct LLVM Backend (Zero-Clang)

The OFS compiler emits LLVM IR directly without intermediate C code:

```
llvmgen.ofs → output.ll (LLVM IR) → llc -filetype=obj → output.o → ld (with magma.o) → executable
```

Benefits:
- Direct machine code generation (no GCC or Clang dependency)
- Byte-for-byte reproducibility and deterministic IR emission
- Fast compilation (~50-100ms end-to-end)
- 100% pure OFS runtime via Stack Magma (`magma.o`)
- First-class cross-compilation support (Linux ELF and Windows PE32+)

---

**Architecture Status**: Production & Self-Hosted
**Verification**: Verified via `cmp -s v3.ll v4.ll` (zero diff)
