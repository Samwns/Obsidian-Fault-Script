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
│  Contracts   │ → mineral/bearing/settle become checks
│ (contracts   │   and minerals become the base type (A5.1)
│    .ofs)     │
└──────┬───────┘
       │
       ▼
┌──────────────┐
│  TypeChecker │ → Annotated AST
│  (with scope │   (type_str on each node)
│   stack)     │   geode: body becomes vein __geode_*
└──────┬───────┘
       │
       ▼
┌──────────────┐
│  Sediment    │ → bedrock arena (escape: copies
│ (sediment    │   text or [ERRO DE ARENA]) and
│    .ofs)     │   [AVISO DE VAZAMENTO] (A5.3)
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
              | "vein" "(" (type ("," type)*)? ")" ("->" type)?   // function type (F6)

stmt        ::= var_decl | if_stmt | while_stmt | cycle_stmt | mine_stmt
              | match_stmt | return_stmt | throw_stmt | tremor_stmt
              | anchor_stmt | quartz_stmt | unsafe_block | block | "break" | "continue"
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
quartz_stmt ::= "quartz" ("att" | "intel")? "{" quartz_item+ "}"
                ("->" IDENT STRING?)? ";"?
quartz_item ::= STRING | "in" STRING expr | "melt" STRING+ | "," | ";"
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
- Function type (F6): `vein(T1, T2) -> R` becomes an `NK_TYPE_FN` with the parameter types in `extra`, the return in `c1` and, in `val`, the canonical text without spaces (`vein(T1,T2)->R`; without `->`, `void`), which is the `type_str` of every expression of that type. The type's text helpers (`vein_type_params`, `vein_type_ret`, `type_list_at`, which splits on commas outside parentheses) live in `ast.ofs` and are shared by the three stages.
- Calling something that is not a name (`a[i](x)`, `f(1)(2)`, `s.a.b(x)`): the parser keeps the called expression in the `NK_CALL`'s `c0`; typeck decides whether it is a vein value or the old call by name.

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

## Phase 2b: Contracts (`ofs/ofscc/contracts.ofs`, A5.1)

Runs between the parser and the type checker, in both modes (llvmgen and the interpreter get the same tree), and on every Fault Refract reload (`refract_reload`):

1. **Registration:** `NK_MINERAL` nodes (`mineral Name = T grain ...`: `c0` = base type, `extra` = `NK_CONTRACT` "grain") leave the tree; each vein's `bearing`/`settle` clauses live in an `NK_BLOCK` at `c3` of the `NK_FN_DECL` (`NK_CONTRACT`, `val` = the word, `c0` = the rule). A mineral of a mineral adds the rules; a repeated name, a cycle or an existing type name is `[ERRO DE CONTRATO]`. The `grain` shorthand (`contains("@")` → `ofs_str_contains(value, "@")`) is rewritten here.
2. **Static check:** a constant evaluator (integers, booleans and text; `+ - * / % & | ^`, comparisons, `&& || !`, `ofs_str_contains`/`ofs_str_eq`/`ofs_str_len`, global `const` with a literal; a name declared in the local scope is unknown) checks calls to veins with `bearing`/mineral parameters, `forge`/assignment of a mineral, `return` from a `-> Mineral` vein and a `settle` that only depends on `result`. Definitely false = `[ERRO DE CONTRATO file:line:col]` + `regra em ...`; the driver stops (as on a syntax error). Definitely true skips the run-time check.
3. **Run-time check** (off with `OFS_CONTRACTS=0`, which `ofs build --release` sets): plain OFS code inserted into the tree. At vein entry, `if (!(grain))` + a call to the `__ofs_contract_fail` builtin with the message per mineral parameter and per `bearing`; after `forge`/`=`/`+=`/`++` on a mineral (local, parameter or `obj.field` with an object of declared type); each `return e` becomes `{ forge __settle_result: R = e; checks; return __settle_result }` (`result` → `__settle_result`; a parameter changed in the body and cited in a `settle` gets a `__settle_in_p` copy at entry). The message is built at run time from the rule text (`expr_text`), the location (`ast_src_loc`) and the values (`ofs_stone_to_obsidian`, `ofs_crystal_to_obsidian`, `__ofs_bool_text`, quoted text).
4. **Erasing minerals:** every `NK_TYPE`/`NK_TYPE_FN`/`NK_CAST` in the program replaces the mineral name by its base (`Array<UserId>` → `Array<stone>`); the type checker, llvmgen and the interpreter never see minerals. Each erased node keeps a note (`contracts.type_note`) with the mineral and its rules, which Fault Refract adds to the signature (`rf_sig`/`rf_mshape`/global type): changing the `grain` of a used type is a type change.

The `__ofs_contract_fail` builtin (one argument: the message) is the only new one: `void` in the type checker; in llvmgen it becomes the runtime's `ofs_array_fail(msg, ofs_str_len(msg))` (already present: `"ofs: " + msg` on stderr and `exit(1)`; declared only when the module uses it); in the interpreter, `BI_CONTRACT_FAIL` writes the same text and exits with 1. A program without `mineral`/contracts goes through none of this (identical IR).

`OFSCC_MODE=assay` (`ofs assay`): before phase 2b, `contracts.assay_source` builds the text of a `core main` with the boundary cases (bounds of comparisons against constants in the rules, ±1; cases outside `bearing`/`grain` only listed), the driver parses it and `assay_splice` replaces the program's `core main` with it; then it runs interpreted with checks on.

---

## Phase 3b: Sediment (`ofs/ofscc/sediment.ofs`, A5.3): `bedrock` arena and leaks

Runs after the typeck (it uses the types), in both modes, in `ofs check` and on every Fault Refract reload. A program without an allocating `bedrock` and without a dangling `ofs_alloc` does not change (identical IR).

1. **Levels.** Each expression has a level: 0 = heap (or outside any arena), d = the arena of the `bedrock` at depth d. An allocation in the block itself (text `+`, `[...]`, `forge m: Monolith`, the runtime's text/`Array`/`ofs_alloc` builtins) is d; reading a variable is its content; field, element, `*p` and `?` take the base's level; a vein call takes the highest level of its arguments (it may return one of them). Variable: "home" level (depth of the declaration) and content (highest level assigned, capped at home), iterated to a fixed point, stored per declaration node.
2. **Kept parameters.** Per vein, the mask of parameters that may be kept (assigned to a global, field, element, `*p`, `push`/`set`, captured by a geode, passed to a kept parameter of another vein or to an unknown vein/C); `return` does not keep. Fixed point across all veins, starting pessimistic (recursion stays kept). `extern`/`rift` (C) do not keep; runtime functions that keep (window title) do.
3. **Escape.** A value whose level is higher than the place it goes to (`return` inside the bedrock, outer variable/global/parameter, field/element of an outer object, `push`/`set`, kept argument): `obsidian` is wrapped in the __bedrock_lift wrapper; text `s += x` going out becomes `s = lift(s + x)` (the __bedrock_lift wrapper); anything else is `[ERRO DE ARENA]` (outside `abyss`). Copying a pointer-free `monolith` into an outer variable is allowed.
4. **Mark.** A `bedrock` `NK_BLOCK` with some allocation in its own code gets `type_str = "arena"`. The llvmgen calls `ofs_arena_open()` (slot on the stack) and registers the close as a synthetic anchor (`register_scope_free`, kind 3: `ofs_arena_close`), run on every exit (`emit_anchors_from_depth`); inside it, every call that is not a runtime text/`Array` builtin (`sediment.arena_builtin`) runs with `__ofs_arena_cur = null` (the arena is lexical). __bedrock_lift becomes `ofs_arena_lift`. In the interpreter, `exec_arena_block` opens the runtime arena through `dlsym`, keeps it inactive for the interpreter's own allocations, and only the `ofs_alloc` written in the block (same vein) uses `__ofs_arena_alloc`.
5. **Runtime** (`stdlib/runtime/reservoir.ofs`): `ofs_alloc` takes from the active arena (bump in 64 KB chunks that double, zeroed); `ofs_free` of a live arena block does nothing; `ofs_realloc` grows in place (last block) or copies inside the same arena; an `Array` grows in the region of the `Array` itself (`__ofs_alloc_like`), so an outer `Array` stays on the heap; `ofs_arena_close` also closes arenas opened above it, keeps the largest chunk and resets the `ofs_str_char_at` cache.
6. **Leaks.** Per vein, outside "arena" blocks: a local variable that receives `ofs_alloc`/`malloc`/`calloc`/`realloc`/`ofs_realloc` and never appears in an owner use (call argument, `return`, right side of an assignment, `Array`, `seal`/`anchor`, geode, `&p`; `*p`, field, index and comparison do not count), or a bare allocation: `[AVISO DE VAZAMENTO]` on stderr. `OFS_LEAK_WARN=0` turns it off.

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

5. **Function types (F6)**: the name of a `vein`/`extern` (`SYM_FN`) used as a value gets the type `vein(params)->ret` of its declaration (`fn_value_ident`; an `extern` of a variadic C symbol is an error). `f(x)` with `f` a variable/parameter of vein type, `s.field(x)` with a vein field (`vein_field_callee`), a `c0` coming from the parser and `fault_call(f, ...)` with a typed `f` become an `NK_CALL` `__vein_call` with the vein in `c0` (`check_vein_call`): the arity is checked and each argument is converted to the parameter type with the F3 rules, but no defined conversion is an `[ERRO DE TIPO]` (not a warning). `coerce_value` requires the same signature when storing a vein value (`fn_store_check`, with a hint of what differs); `==`/`!=` only between the same signature or with `null`; `as` between a vein and an address requires `abyss`. `echo(f)` and `"x" + f` get the `__ofs_vein_name` conversion (the vein's name in both modes).

6. **Low-level needs a block (F4; an allocating `bedrock` is also an arena: Phase 3b)**: the parser keeps `fracture`/`bedrock`/`abyss`/`tectonic [X]` as an `NK_BLOCK` with the name in `val` (the body of `fracture vein`/`abyss vein` too); `llvmgen` and the interpreter run it as a plain block. The typeck tracks the depth (`_fracture_depth`, `_abyss_depth`, modeled on `_anchor_depth`) and, in `require_fault`, requires the block for `*p`, `&x`, `as` with a pointer, `fault_step`/`fault_memcpy`/`fault_memset` (any block) and `fault_call`/`fault_addr` (`abyss` only). Outside the block: `[AVISO DE FALHA file:line:col]` on stderr until 2027-01-09; with `OFS_STRICT_FAULT=1`, `[ERRO DE FALHA ...]` and `_semantic_errors++`. Full rule: [LANGUAGE_REFERENCE.md](LANGUAGE_REFERENCE.md), section 12.
7. **Stratum and per-module permissions (A5.4 + Idea 4)**: the driver (`ofscc.ofs`, `expand_attaches_body`) reads each file's header as text, like `attach` (`stratum`, `claim`, `caprock` and the `//! claim` form), and does not pass those lines to the lexer; every file becomes a module in the `ast.ofs` table (`ast_mod_*`: stratum, declared claims, used claims, whether it is runtime, the main program's `caprock`), and the line map stores the module of each range (`ast_mod_of_line`). The type checker finds each node's module from its line: `require_fault` allows everything in `stratum abyss` and is `[ERRO DE ESTRATO]` in `crust`; block entry (`fault_block_open`) rejects `fracture`/`tectonic`/`abyss` in `crust` (`bedrock` becomes just an arena, not counted in the depth) and requires `claim abyss`; `extern`/`rift vein` (except `ofs_*`) require `claim rift` and `quartz` requires `claim quartz` (`require_claim`, one message per module and permission; `[AVISO DE PERMISSAO]` until 2027-03-16, error with `OFS_STRICT_FAULT=1`). At the end (`check_modules`) come `claim` in a `crust` file and the `caprock` (declared or used claims of each attached module, runtime excluded). `ofs check` prints the module table (`print_module_claims`). None of this changes the IR.

### Error Handling (Current)

- Warnings only (no `tremor`/`catch` yet)
- Line/column reported
- Message: `[typeck warn line:col] description`
- Syntax error: `[ERRO SINTATICO file:line:col]` with the real file and line (also inside an `attach`), from the expanded-source line map (`ast_src_loc`)

---

## Phase 4A: AST Interpreter Backend (Interpretação Direta)

**File**: `ofs/ofscc/interpreter.ofs` (70KB, 2,086 lines)

The OFS interpreter backend evaluates the typed syntax tree (AST) directly in memory. Veins containing `quartz` use the hybrid path described below, which generates LLVM IR and caches a native library. It is the default backend used by `ofs <file.ofs>` and `ofs run <file.ofs>`.

### Interpreter architecture

1. **`Value` system**: a universal monolith that unifies scalar values (`stone`, `crystal`, `obsidian`, `bool`, `null`) and virtual heap handles for `Array<T>`, `monolith` objects and pointers.
2. **Environment and call frames**: strict lexical scope based on frame markers (`_current_frame_base` and `_globals_count`), so callers' variables do not leak into callees or globals.
3. **Value semantics**: `forge y: T = x` and `y = x` copy the `monolith` block (`obj_clone()`/`memcpy` into the block `y` already has, like the native `store %T`); vein parameters receive the `monolith` by reference, like the native pointer.
4. **Method dispatch**: methods defined in `impl Type` blocks are resolved automatically, with `self` injected.
5. **Real pointers (F2)**: a pointer is an address (`VAL_INT` with type `*T`). `*p`/`*p = v` use `mem_load`/`mem_store` by the memory class of the pointee type (`MC_*`: width, sign, `f32`, `obsidian`, `*T`, `monolith`, variant), with the `typeck` static type precomputed per node in `annotate_nodes` (the same type `llvmgen` uses); the load/store is the interpreter's own native code (`fracture { *p8 = v as u8 }`). `&x` of a variable turns the entry into `VAL_BOXED`: the value moves to an 8-byte slot (`box_var`), read/written by `env_load`/`env_store` and freed in `env_pop_one` (`_boxed_live` keeps the fast path free when there is none). `as` mirrors `llvmgen`'s `NK_CAST` (`eval_cast`). A `monolith` lives in raw memory with the `llvmgen.struct_layout` layout (the same function computes the native `ofs_alloc` size and the interpreter's offsets; `compute_layouts`), and `VAL_OBJECT` holds the block address; an `Array` field holds the interpreted handle as `-(h+1)`.
6. **Built-ins**: direct handling of I/O (`echo`), strings (`ofs_str_*`), arrays (`ofs_array_*`) and math (`ofs_pow`, `ofs_sqrt`). The list (`is_builtin`) is only a fast path, not a limit on what can be called.
7. **Rift (`extern vein`/`rift vein`)**: at load time (`collect_declarations` → `resolve_calls` → `ffi_check_symbols`), every called extern becomes an address via `dlsym(RTLD_DEFAULT, sym)` (honoring `bind`), after opening `libm` and `OFS_FFI_LIBS`; a missing symbol is the linker's error (`undefined reference`), exit code 1. Veins from `stdlib/runtime/*.ofs` (line ranges marked by `expand_attaches` through `ast_mark_runtime_lines`) use the native version linked into `ofscc`. The call (`call_extern`) converts arguments by the declared type (integers/pointers/`obsidian` as `i64`, `crystal` as `double`), stores them in a buffer and calls `__ofs_ffi_dispatch`, which `llvmgen` defines inside `ofscc` itself (`emit_ffi_dispatch`, when the module declares that extern): a `switch` with one call of exact LLVM type per signature class (`ffi_sig`/`ffi_sig_supported`: up to 4 `i64|double` args with an `i64|double` return, up to 12 integer-only, variadics with 1 to 3 fixed + up to 4 extra; 155 cases). That is why `ofscc` must be linked with `-rdynamic` (it exports the runtime and the dispatcher to `dlsym`). In F2, `&x`, `monolith` (the block) and `Array` go to the native function as addresses: the `Array` is the interpreter's value column, which already is a native `ofs_array` (`arr_native_ptr`), with its length synchronized on return (`ffi_sync_arrays`); an `Array` returned by the native function becomes an interpreted array that uses the same block (`arr_adopt`). native → OFS callbacks: the module that declares `extern vein __ofs_cb_table` gets from `llvmgen` (`emit_cb_trampolines`) 16 functions `__ofs_cb_K(i64 x 6)` that call `__ofs_cb_enter(K, ...)`, defined at the end of `interpreter.ofs`; `fault_addr` of an interpreted vein binds the vein to a slot (`cb_slot_for`) and returns the address of `__ofs_cb_K` (found via `dlsym`, so an `ofscc` built by an older compiler still links); `cb_enter` pushes the arguments by each parameter's type, runs the vein and returns the result as `i64`, preserving the interpreter registers (re-entrant: it works inside a native call made by the interpreter). Exhausted slots, more than 6 parameters or `crystal`/`f32` in the callback are a clear error (exit code 1).
8. **Vein values (F6)**: `VAL_FN` holds the type (`_rt`), the interpreted vein (`_rr` = id of the declaration node, or -1), the native address (`_ri`, of an `extern` or coming from C) and the name (`_rs`). `resolve_calls` marks each vein `NK_IDENT` in `_naux` (vein `id+1`, extern `-(ei+1)`); `eval_ident` only uses it when no variable has that name. `__vein_call` (`CALL_VEIN`) calls the interpreted vein directly (`call_function_by_id`) or the native address with the type's signature (`call_native_sig` → `ffi_call_addr`, the same path as `call_extern`). Going to C (`ffi_arg_int`, `reg_as_int`, `as stone`), an interpreted vein becomes the `cb_slot_for` trampoline; coming back from C (`fn_from_raw`: extern return, callback parameter), a trampoline address turns back into its original vein and an extern address gets its name. In raw memory (`monolith` field, `*p`) the `MC_FN` class stores the native address or `-(id+1)` for an interpreted vein, without spending a trampoline.

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
vein(T) -> R → ptr (function pointer; the call uses the type R (T))
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

### Function address (`fault_addr`, `fault_call`)

`fault_addr(name)`: typeck replaces the argument with the mangled symbol (`ns.fn` → `ns__fn`) and requires it to be a vein (`SYM_FN`); llvmgen resolves the `bind` of an `extern`/`rift` and emits `ptrtoint ptr @symbol to i64`. `fault_call(address, args...)` emits `inttoptr` + `call i64 %p(...)` with the LLVM types of the arguments. See [LANGUAGE_REFERENCE.md](LANGUAGE_REFERENCE.md).

### Vein values (F6)

A `vein(...) -> R` type is `ptr` in LLVM (8 bytes in a `monolith`, like any pointer). The name of a vein used as a value (an `NK_IDENT` of vein type that is not a variable, `is_vein_ref`) becomes `@symbol` (with the extern's `bind`); a global initialized with a vein's name gets the constant `@symbol`. `__vein_call` (`emit_vein_call`) evaluates the vein, tests `null` (branching to `__ofs_vein_null`, which calls the runtime's `ofs_array_fail`: message and exit code 1) and emits `call R %f(T1 a, T2 b)` with the exact LLVM signature of the type; each argument goes through `coerce_for_store` to the parameter type. `==` between vein values is `icmp ptr`. `__ofs_vein_name` is a chain of `icmp eq ptr %p, @symbol` over the veins the module uses as values. Both helpers are only emitted when the module uses them: the IR of a program without vein values does not change.

### geode (closures)

The parser reads `geode (params) -> R { body }` (a contextual word: `geode` followed by `(` whose list closes before `->` or `{`) as `NK_LAMBDA` (`c0` params, `c1` return, `c2` body). The typeck checks the body in the creator's scope, with a stack of open geodes (`_gd_base`: the depth of each one's parameters); an identifier that is local to the vein (depth ≥ `_fn_base`) below the base of an open geode becomes a capture of that geode (and of the outer ones, transitively). Assigning to a capture, `&capture` and `shatter`/`erode` without a loop in the body are `[ERRO DE GEODE]`. Then the body is lifted into an `NK_FN_DECL` `__geode_<vein>_<k>` appended at the end of the program, with the captures as the first parameters (types rebuilt from the canonical text, `make_type_node`); the `NK_LAMBDA` keeps `val` = that name, `extra` = the captures (`NK_IDENT`) and `c0..c3 = -1`, type `vein(params)->R`. So the rest of the compiler (A6, contracts, crystallization, Refract) sees an ordinary vein.

**Native:** without captures, `@__geode_...` (goes to C like any vein). With captures, `emit_geode` allocates on the heap (arena off) `[@name.env, MAGIC, capture 0..n]` (8 bytes per capture, stored with its LLVM type) and the value is that address `| 1`. The adapter `@name.env(ptr %env, params)` (end of the module, `emit_sediment_helpers`) loads the captures and calls the body vein. If the module has a capturing geode (`scan_geodes`), `emit_vein_call` tests bit 0 and the mark and calls `adapter(env, args)` (or the vein directly, with a `phi` on the result), `__ofs_vein_name` returns `<geode>` for a tagged pointer, and vein values passed to functions outside the module or turned into addresses go through `__ofs_geode_c_check` (tagged = `ofs_array_fail`, exit code 1). Without a capturing geode none of this is emitted. In A6 each capture escapes (`esc_walk(capture, 1)`); the analysis is no longer switched off.

**Interpreter:** `eval_geode` builds a `VAL_FN` with `_rr` = id of the body vein (`_naux` resolved in `resolve_calls_from`) and, with captures, `_ri` = handle + 1 of an interpreted array with the copied captures. `call_vein_value` calls `call_geode` (captures, then the arguments on the stack). In raw memory the value is `-(id + 1 + env·2^24)`. `fn_native_addr` of a capturing geode is the runtime error. Crystallization (`cr_supported`) keeps interpreted, when the program has geodes, a vein that handles vein values or reaches (`rf_walk_calls`) one that does; and a vein with an arena only crystallizes if the arena runtime is in the process. In Refract, a `__geode_*` vein with a new signature (captures changed) is neither refused nor redirected: geodes already created keep the old body.

## Phase 4C: inline assembly and hybrid execution

`TOK_QUARTZ` → `NK_QUARTZ` represents a statement with instruction strings, inputs (`NK_QUARTZ_IN`), an optional destination and options (`NK_QUARTZ_OPT`: constraints, clobbers and dialect). `parser.parse_quartz` parses the syntax; `typeck.check_quartz` validates scalar destinations, operands and constraints, and `require_fault` requires `abyss`. `llvmgen.emit_quartz` emits `call ... asm sideeffect`, optional `inteldialect` and an output store. Memory inputs use `ptr elementtype(T)` with `*m`; `+r` outputs receive a tied input in the same register.

In the interpreter, `hy_prepare` finds veins containing quartz and requests their closure from `llvmgen.hy_generate`. The module contains used functions and globals and declares runtime symbols exported by the `ofscc` process (`-rdynamic`). The IR and compilation command determine the cache key. `clang -shared -fPIC` builds the library; `dlopen`/`dlsym` resolve roots and globals. Typed calls use `ffi_call_addr`/`__ofs_ffi_dispatch`; `hy_sync_in`/`hy_sync_out` synchronize globals, including around reentrant callbacks. Heap and runtime belong to the same process. Assembly lowering and native dependency closure can be reused by crystallization; live editing (Fault Refract, Phase 4D) uses both.

`cr_tick` counts calls and, at `OFS_CRYSTALLIZE_THRESHOLD` (default 1000), requests the same module for all eligible roots. IR generation is synchronous; `clang -O2` compilation runs in the background, and later calls check a completion marker before installing native dispatch. `OFS_CRYSTALLIZE=0` disables optional promotion; missing compilers or optional failures preserve interpretation. Long loops inside one call use OSR (below); replacement triggered by file edits is Fault Refract (Phase 4D).

**Shared globals (wave 4).** Hybrid modules do not define globals: each global in the closure becomes `@__ofsg.<name> = global ptr null` and every access goes through it (`var_ptr` emits `load ptr, ptr @__ofsg.name`). After `dlopen`, `hy_load` stores the address of the global's **single cell** (`hy_global_cell`, 8 bytes) there: the interpreter keeps the global itself in that cell (`VAL_BOXED`, the same mechanism as `&x`), so the interpreter and every `.so` (crystallization, OSR, Refract recrystallizations) read and write the same memory, with no copy at the boundary. Only `Array` globals (the interpreter has its own handle), `vein(...)` globals and not-yet-initialized globals keep a copy (`_hy_gmode` = 1, synchronized by `hy_sync_in/out`). Measured (`gsync.ofs`, 24 globals, 400k interpreted→native calls, `OFS_OSR=0`): 0.43 s → 0.18 s (pure interpretation takes 0.29 s: crystallization used to lose to it in this case).

**OSR (wave 4).** `exec_while` counts iterations of each loop execution; every 1024, `osr_tick` (unless `OFS_OSR=0`): an eligible loop (`osr_prepare`: `while` only; body without `return`/`throw`/`anchor`/`?`/`&`/geode/vein value/`bedrock`; reached veins without geodes and with the arena runtime when they use `bedrock`; the path to it through block/`while`/`if`/`cycle`) that adds up `OFS_CRYSTALLIZE_THRESHOLD` iterations is requested from `llvmgen` (`hy_osr_add`). **Live variables** are the declarations visible at the header (parameters and `forge` of the blocks on the path, `osr_scope_decls`) whose name appears in the loop (`osr_live`). `hy_emit_osr` generates `__ofs_osr_<node>(ptr %st) -> i64` in its own module (the closure of the body; globals are the shared cells): it loads each variable from `st[1..n]` (8 bytes each; `monolith` is the pointer to the interpreter's block, `Array` the native `ofs_array`, text with owner 0), enters through the header (tests the condition), runs the loop like a normal `while` and writes the variables back to `st`, returning 0. `osr_enter` does the transfer (`mem_store`/`mem_load` by class; `Array` through `arr_native_ptr`/`arr_sync`, like globals) and the interpreter continues after the loop. Compilation runs in the background, one job at a time (`_cr_josr`, `_hy_load_loops`); cached by IR hash. A variable whose address was taken (`&x`) or outside the frame keeps the loop interpreted. `bench/ofs/loop.ofs`: 18.6 s → 0.34 s (native 0.41 s).

**Floating-point callbacks and `f32` in the FFI (wave 4).** Besides `__ofs_cb_K(i64 x6)`, the module that declares `__ofs_cb_table` (the `ofscc`) gets `__ofs_cbx_K(i64 x6, double x8) -> { i64, double }` and `__ofs_cbx_table`: SysV x86-64 and AArch64 number integer and float argument registers separately, so the trampoline receives all of them; the `{ i64, double }` return goes out in rax/x0 **and** xmm0/d0 and serves integer, `crystal` or `f32` callbacks (bits in the low half). `__ofs_cbx_enter` (interpreter) receives the doubles as bits and assigns them to parameters in per-class order. On Windows x64 (positional registers) `cbx` is not generated and a float callback keeps the clear error. `f32` in Rift/FFI and in hybrid roots travels as a `double` whose low 32 bits are the `f32` (C reads only those); an `f32` result comes from the low 32 bits of xmm0.

Assembly text remains target specific. LLVM does not translate instructions between x86-64 and AArch64. The hybrid interface keeps the FFI arity limits (up to 12 integers, up to 4 with `crystal`/`f32`; callbacks up to 6 integers and 8 floats). See [LANGUAGE_REFERENCE.md](LANGUAGE_REFERENCE.md) and [COMPILED_LANGUAGE.md](COMPILED_LANGUAGE.md) for syntax and the parity rule.

## Phase 4D: Fault Refract (live editing, L0/L1/L2/L3)

**Files**: `ofs/ofscc/interpreter.ofs` (Fault Refract section), `ofs/ofscc/ofscc.ofs` (`refract_reload`, `refract_paths`), `typeck.reset`, `ast_srcmap_reset`. Command: `ofs refract` (wrapper) = `OFS_REFRACT=1` in `run` mode.

**Dispatch cell.** Every vein call goes through `call_function_by_id(fid)`. With refract enabled, `_rf_cell[fid]` (one entry per AST node) maps the id to the live version: the old version of a swapped vein points to the new one, and identical copies in the new version point to the live one (so native state and crystallization heat stay on the version already running). `rf_redirect` keeps chains short (A→B then B→C becomes A→C). A per-vein counter (`_rf_active`) tells who is on the stack; a running call finishes on the old version, because the body being executed belongs to the old node (no OSR).

**Watching.** `rf_tick` runs at the start of every call; every `OFS_REFRACT_PERIOD` ms (clock via `dlsym("clock_gettime")`, `time` as a fallback) it flushes stdout (live output through pipes/files), checks pending native work and computes an FNV hash of the watched files' contents (`refract_paths`: the main file and attaches outside the stdlib and the runtime). Content that differs and stays stable for one period triggers `rf_refract`. A missing file (atomic save in progress) counts as no change.

**In-process recompilation.** `refract_reload` repeats the startup path (attach expansion, lexer, parser, `typeck.reset` + `typeck.check`) with stdout redirected to stderr (`dup`/`dup2`); new nodes go into the same pool. `build_node_tables_from`, `annotate_nodes_from` and `resolve_calls_from` process only the new nodes; the per-node crystallization tables (`_cr_heat`, `_cr_eligible`, `_cr_quartz`) grow with them. On an error the tables still grow (ids are global), but no cell changes.

**Diff.** `rf_scan` lists veins (key = name with namespace/monolith, as in `collect_declarations`), globals, monoliths and externs. Each vein gets a structural hash of its typed tree (`rf_hash`: kind, text and `type_str` of every node, no line numbers); equal hash = equal vein. Signature (`rf_sig`), monolith shape (fields and types), global type and extern shape feed the rules.

**Safety** (any rule refuses the whole swap and nothing changes): new signature of a vein called by an active vein (`rf_active_user`, from already resolved calls: `CALL_FN`, vein values and `CALL_DYN` methods by suffix), handed to C (`_cb_fn`) or used as a value; removed vein called by an active vein; monolith layout change with instances (`obj_new` counts per monolith); global with a new type; new extern without a symbol or changed extern; variant list that does not start with the old one (numeric tags must stay the same). Calls from unchanged veins were checked by the typeck against the new signature, because the whole program is checked.

**Swap (L0).** New veins are registered by name (as at startup); changed ones are not: the name still resolves to the old id and the cell leads to the new version. New globals take one of `RF_GLOBAL_SLOTS` (64) slots reserved at the end of the globals area (`[0, _globals_count)`, where `find_var` looks), running the initializer without seeing the caller's locals; existing globals are not re-run. A new monolith (or one with a new layout and no instances) gets a new field range and `compute_layouts`. Registers and `_cur_call_id` are preserved around the swap.

**L1 (native).** Loaded `.so` files share the global cells (Phase 4C, wave 4). On a swap, the affected native roots are the changed ones and those that reach (`rf_reaches`, transitively) a changed vein, because calls inside a `.so` are direct. They get address 0 (they run interpreted, in the new version) and go to `_rf_dirty`; `rf_launch` generates the closure from the new root (`llvmgen.hy_generate`) and compiles it in the background. When the `.so` is ready, `hy_set_root` replaces the root entry (`rf_live_fid`). One job at a time; a job that finishes after another swap (generation `_cr_jgen` different from `_rf_gen`) is discarded and redone with the current program. Veins with `quartz` are compiled before the swap in soft mode (`_hy_soft`: a clang/dlopen failure refuses the swap instead of ending the program). The old `.so` stays loaded. Unloading it (`dlclose`) would require proving that nothing points into it anymore: having no frame inside it is not enough, because text literals and vein addresses of the `.so` may have gone into globals, arrays and objects; this is pending (copy literals to the heap or count references).

**OSR in Refract (wave 4).** At every `while` header, if `_rf_gen` changed since the loop started, `rf_osr_target` looks in the live version of the running vein (`_cur_fn_id` → `rf_live_fid`) for the loop at the same position among the `while` loops (pre-order) and compares the variables before it (`osr_scope_decls`, block/`while`/`if` only): same name and type keeps the live value, a new one gets its initial value (`execute_stmt` of the declaration at switch time), a removed one goes out of sight (`_env_sym` = -2), a different type or a new parameter refuses. `rf_osr_switch` runs the new loop and then the rest of the new version along the path (`rf_exec_after` in blocks, re-entry in the `while` loops above, nothing in `if`) and returns `EXEC_RETURN` so the old frame only unwinds; the active-frame count (`rf_frame`) moves to the new version. A loop without calls also sees the edit (`rf_tick` every 1024 iterations). A **native** (OSR) loop under Refract gets the `__ofs_osr_poll` hook in `st[0]` and asks every 65536 iterations; when the file changed (`rf_osr_poll`, at the `OFS_REFRACT_PERIOD` pace) it returns 1 through the header, the interpreter continues interpreted (`_osr_hold` until the edit is handled), switches version and requests OSR again with the new program (`rf_apply` clears `_osr_addr`: native loops call the old veins directly).

**Compatible signature (wave 4).** `rf_sig_compat`: old parameters equal (`rf_tstr`, with the grain) and first, new ones at the end with a default value; return equal, formerly `void`, a narrow integer where it was `stone`/`i64`, `f32` where it was `crystal`. Compatible with an active user = accepted (callbacks and vein values still refuse). The parameter default (`b: stone = 3`, a literal) belongs to typeck: `note_defaults` validates it (literal, trailing only) and `fill_defaults` completes calls with copies of the literal; an old call of a vein that gained a parameter gets the default in `bind_params` (interpreted) or in `rf_pad_defaults` (pushed before a native root).

**Monolith migration (wave 4).** Under Refract every monolith block is registered (`rf_obj_alloc`: `obj_new`, `obj_clone` and, in the native code of hybrid modules, `__ofs_obj_alloc` instead of `ofs_alloc`, except inside a native `bedrock`, whose block dies at its end) and gets `RF_OBJ_SLACK` (64) spare bytes. A new layout with live instances: `rf_migrate_check` refuses a field that changes to a non-convertible type (numeric↔numeric converts) or a size beyond the slack; otherwise `rf_migrate` rewrites each instance **in place** (copy of the old block, zero, field by field by name: same type copies the bytes, numeric converts via `mem_load`/`mem_store`, a new field gets the type default as in `obj_new`), so pointers to it stay valid. Old fields leave the table (empty owner), which invalidates the field cache (`_naux`) of nodes that already ran. Instances created by C or with raw `ofs_alloc` are not known.

**Limits:** an interpreted vein that gets hot after being edited only crystallizes if it was already eligible at startup; up to 64 new globals per run; OSR only for `while` (not `cycle`) and not for loops with `return`/`throw`/`?` in the body; Refract OSR does not follow a loop inside `cycle`/`match`/`bedrock`; the old `.so` is not unloaded.

**L3 (remote/Android), done.** **Files:** `ofs/ofscc/ofscc.ofs` (Fault Refract L3 section: `r3_scan`, `r3_hash`, `r3_build_manifest`, `r3_patch_plan`), `ofs/ofscc/llvmgen.ofs` (`rf3_set`, `rf3_emit_cells`, `rf3_patch_generate`, the call through the cell in `emit_expr` and the `external` globals in `emit_header`), `ofs/stdlib/geophone.ofs` (agent), wrapper (`android_build` with `--refract`, `refract_remote_run`). The app runs **AOT native**, not interpreted. **Build with cells** (`OFSCC_REFRACT=cells`): the driver appends `attach {geophone}` to the source, lists the user veins (those in the `refract_paths` files; `main` and `__*`, such as geodes, get no cell) and `llvmgen` emits `@__ofs_cell.<vein> = global ptr @<vein>` and, for every `NK_CALL` whose target has a cell, a `load ptr` plus an indirect call (variadic calls and vein values stay direct). The manifest (`@__ofs_refract_manifest`, text with one record per line: `V` symbol/hash/signature/cell, `G` global/type, `M` monolith/fields, `X` extern/shape, `S` variants) uses a structural hash of the typed tree like `rf_hash` (kind, text and `type_str`, no lines), computed after typeck and `sediment`. The agent starts through `@llvm.global_ctors` (after `__ofs_init_globals`, which the build links with `-Wl,-init`), on the thread that loads the lib (the main one), and adds the abstract socket `ofs-refract.<package>` to that thread's looper (`ALooper_addFd`): no new thread, because the runtime is not thread-safe, and a patch is applied between two events. `SO_PEERCRED` only accepts uids 2000 (adb/shell), 0 and the app's own. **Patch** (`OFSCC_REFRACT=patch`, `OFSCC_REFRACT_BASE` = the manifest the app sent on connect, already merged with earlier patches' records): `r3_patch_plan` diffs vein by vein and applies the rules (new signature for a vein with a cell, new global or new global type, layout of an existing monolith, changed extern, reordered variants: rc 4 with the message; nothing changed: rc 3). It includes the changed veins, the ones without a cell in the app (new veins from earlier patches, geodes: redefined in every patch, since calls to them are direct) and the cell veins that use one without a cell. `rf3_patch_generate` emits only those veins; globals become `external` (the app's unique address: state carries on, like wave 4's global cells), variants stay `internal` to the patch (appending works), the other veins they use become `declare` (the `hy_walk` closure, non-transitive) and `@__ofs_refract_install` stores the new address of each vein with a cell. On the PC the NDK `clang` links the patch with `-shared -fPIC -Wl,-Bsymbolic -Wl,--no-undefined` against the build's `lib<name>.so` (which got a `-soname`): `dlopen` in the app resolves the `declare`s and globals there through `DT_NEEDED`, and a symbol the app lacks becomes a link error on the PC. The app writes the `.so` to `code_cache/ofs-refract/`, `dlopen`s it, calls the installer, merges the patch's manifest and replies `OK`/`ERR`; then it calls `oal__refresh` (if present) or requests a frame. Measured on a moto g(8) power (Android 11): a 6 KB patch applied in 1 to 3 ms, same PID, counter preserved (`ofs/tests/refract/android.sh`). **Limits:** no native OSR (a running vein finishes in the old version); a stored vein value points at the build's version (the PC warns); new signatures, new globals and new layouts need a reinstall; every patch's `.so` stays loaded; aarch64 only.

## Phase 4E: `magma.cast` (Idea 5): compiling and loading OFS at runtime

**Files**: `ofs/stdlib/magma.ofs` (the API, pure OFS, the same in both modes), `ofs/ofscc/ofscc.ofs` (`OFSCC_MODE=cast`, `magma_manifest`, `OFS_MAGMA_CAPROCK`), `llvmgen.set_magma_manifest`, `typeck` (`claim magma`, `magma_vent_seat`), `ast.ofs` (`CLAIM_MAGMA`, mod ceiling), wrapper (`--export-dynamic`).

**Decision: the compiler runs in a separate process, in both modes.** Under `ofs run` the compiler is in the process, but recompiling another program into the same node pool and `typeck` (the `refract_reload` path) would replace state the host program's interpreter and crystallization still use, and compiler errors call `exit` in several places: a broken mod would take the program down. A separate process isolates both and is the same path natively, where the binary has no compiler inside (the alternative, linking `ofscc` as a library, would put the compiler in every binary). Under `ofs run` the process is the running `ofscc` itself (`/proc/<pid>/exe`, detected through the `interpreter__run` symbol exported with `-rdynamic`); natively, `OFS_MAGMA_OFSCC` or the installed `ofscc`, searched like the wrapper does.

**Pipeline.** `magma.cast` writes an `sh` script into a `mkdtemp` directory (`ulimit -v 3000000`, `timeout 120`): `ofscc` with `OFSCC_MODE=cast`, `OFS_STRICT_FAULT=1` and the ceiling (`OFS_MAGMA_CAPROCK`) → IR; key = sha256 of IR + clang + flags + `uname`; with no cached `.so`, `clang -shared -fPIC -O2 -Wno-override-module -Wl,-Bsymbolic -lm` (plus `-mllvm -opaque-pointers` on clang 14) and atomic publication (`cp` + `mv`). The `.so` is opened with `dlopen(RTLD_NOW)` (local); `__magma_manifest` (text `name\tsymbol\tsignature` per vein of the mod file) is read with `dlsym`, and `__ofs_init_globals` runs once per `.so` (the same source reuses the module). Any failure becomes `magma.crack(m)` (compiler log without progress lines).

**`cast` mode in the driver.** It is `ir` plus: the `OFS_MAGMA_CAPROCK` ceiling applied as a `caprock` (intersected with the mod's own) that also covers module 0 (`ast_mod_set_cast_ceiling`, `check_modules` starts at 0); `[ERRO DE MAGMA]` if the mod has `core main`; the manifest (`magma_manifest`: top-level and namespace veins on module 0 lines, no `__*`, signature from `typeck.param_types_of` + `vein_type_make`, the canonical F6 text) emitted by `llvmgen` at the end of `generate` as `@__magma_manifest`.

**Typed `magma.vent`.** `coerce_value` (typed forge, `=`, argument, `return`) recognizes `magma__vent` (with `m`, the name and `""`) (the 3rd parameter defaults to `""`, filled by `fill_defaults`) stored into a `vein(...)` place: it replaces the literal with the canonical type text and wraps the call in an `NK_CAST` to that type, created by typeck itself (it does not go through the `abyss` rule, like implicit conversions). At runtime the stdlib only returns the address when the manifest signature matches; otherwise 0, which becomes `null`. Native: `inttoptr` and a `call` with the exact signature; interpreter: `fn_from_raw` → a Rift call (`call_native_sig`), and an interpreted vein handed to the mod becomes a `__ofs_cb_K` trampoline.

**Shared runtime.** The `.so` carries no runtime: `ofs_*`/`echo_*` come from the process (same heap, stdout, strings, arena). `ofscc` is already `-rdynamic`; the wrapper links with `--export-dynamic`/`-rdynamic` when the IR defines `@magma__cast`; without the symbols (`dlsym(RTLD_DEFAULT, "ofs_str_concat") == 0`) `magma.cast` returns the error with a hint. `-Wl,-Bsymbolic` keeps the mod's veins and globals bound inside it even when the program exports same-named symbols.

**Security.** `CLAIM_MAGMA` (bit 8; `CLAIM_ALL` = 15): `require_claim` on calls to `magma__cast` and on its definition (the `magma` module declares `claim rift, abyss, magma`), always an error (no migration window); `crust` = `[ERRO DE ESTRATO]`; a `caprock` without `magma` forbids attaching the module.

**Limits:** no `dlclose` (the `.so` stays loaded after `magma.release`: literals and addresses may be stored, as in wave 4); Linux/macOS only; `monolith` crosses the boundary by name (layout not checked); the mod does not see the program's globals.

---

## Phase 4F: `ofs fossil` (Idea 2): record, replay and travel back in time

**Files**: `ofs/ofscc/fossil.ofs` (the `.fossil` format, reading/writing, per-symbol policy table `fz_spec`, source hash), `interpreter.ofs` (section "ofs fossil": `fs_*`; hooks in `execute_stmt`, `call_extern`, `ffi_call_addr`, `call_builtin` (`echo`/`exit`), `fault_runtime_error`, `hy_prepare`, `run`), `ast.ofs` (`ast_src_file`, `ast_src_line`, `ast_exp_line`), `ofscc.ofs` (`OFS_FOSSIL`, source from the header, `fs_enable`/`fs_finish`), wrapper (`ofs fossil`, `--to`, `--back`, `--dig`, `--layer`), VS Code extension (`ofs.fossilReplay/Next/Back`).

**Where to record.** What already existed decided the design: every native call of the interpreter goes through `call_extern` → `ffi_call_addr` → `ffi_invoke` (F1), including the runtime's native veins (`ofs_read_line`, `pulse__now_ms`, registered as extern by `ffi_runtime_native`). `call_extern` looks up the symbol's policy (`fs_spec_of`, cached per extern; 0 = normal call, costing only a test of `_fs_mode`) and leaves it in `_fs_pol`; `ffi_call_addr` consumes it and, with the arguments already in the FFI slots, calls `fs_invoke` instead of `ffi_invoke`. There the integer/pointer arguments are copied before the call (a callback may reuse the buffer). Recording: calls C and writes the event (raw return, float bits from the return slot, returned text, the `localtime` block, bytes C wrote into the effect argument and the FNV-1a hash of output data). Replaying: reads the next event, checks the symbol (otherwise diverges with rc 3), builds the return (text becomes a fresh copy, "the argument itself" becomes the current argument), copies the bytes into the argument's current pointer, checks the output hash and only really calls C for console output when not silenced. The rest (`ffi_set_result`, `ffi_sync_arrays`) is the same path as before, so the registers end up as during recording.

**Policy** (`fz_spec`, one integer per symbol: policy, return kind, effect argument and size, console rule, hash argument and data): world (input, clock, randomness, environment, files, network), output (console really runs, file/network is a checked no-op), process exit (`exit`: closes the fossil first) and live (everything else, deterministic). `OFS_FOSSIL_WORLD` adds symbols.

**Determinism of the rest.** The interpreter is deterministic given what comes from outside: addresses change (C pointers, buffers), but the program only hands them back to C. That is why crystallization and OSR are off (`hy_prepare`: `_cr_enabled = false` when `_fs_mode != 0`): native code would bypass the hook and not count steps. `quartz` stays native, with a warning.

**Steps.** `execute_stmt` calls `fs_step(id)` when `_fs_on` (recording and replaying): counts the statement and, on replay, compares with the target (step, expanded line via `_fs_nline`, precomputed only when there is a line target, or the first statement of the vein body). Without fossil the cost is one global test per statement (measured: interpreted `fib(35)` 6.21 s after × 6.42 s before, within noise).

**Going back: layers via `fork`.** Restoring the interpreter state by copying (`_env_*`, `_arr_*` tables, raw memory of `monolith`s, `ofs_alloc`, C memory) would be large and fragile; `fork` copies everything on demand and is the same idea as `rr`'s checkpoints. Every `_fs_every` steps (`--layer`, default 2000; at most 32 layers, then every other one is dropped and the interval doubles), `fs_layer_make` does `fflush` and `fork`: the child becomes a layer and sleeps reading its own pipe; the parent goes on. Going back to step `t`: kills the layers after the chosen one, sends `go t` to the closest layer ≤ `t` and exits; the layer forks again (it keeps holding the point) and the child runs silenced up to `t`. Since replay is deterministic, all layers are on the same timeline and stay valid. Commands are read byte by byte with `read` (the stdio buffer would be duplicated by `fork`).

**Supervisor.** With navigation, the process the shell waits for forks right at the start and stays out of the run: it receives through a pipe `A <pid>` (new active timeline), `H <pid>` (the active one handed over), `L/K <pid>` (layer created/ended) and `X <code>` (end). On Linux it is a *subreaper* (`prctl(PR_SET_CHILD_SUBREAPER)`): it reaps orphans and, if the active process dies without `X`/`H`, kills the layers and exits with `128 + signal`. POSIX functions via `dlsym` + `fault_call` (the Windows `ofscc` links without them; there only forward `--to`).

**State shown.** `fs_where` (step, `file:line` through the line map, vein, line text) and `fs_show_vars` (locals of the current frame `_current_frame_base..`, globals whose `NK_VAR_DECL` is in a program file, collected by `fs_collect_user_globals`; arrays and `monolith`s through `interp_array_get`/`obj_load_field`; opaque `FILE*` as `<ponteiro do C>`).

**Limits:** `scanf` and structures outside the table; UI (X11/Android events); calls through a vein value coming from C (`call_native_sig`) really run; the language has no `argv` yet. Next step: a DAP adapter with step-back on top of `--dig`.

---

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
4. **Text diagnostics** — errors and warnings come out as `[ERRO ... file:line:col]` on stderr (no structured format, except with `OFS_PROGRESS=json`)

### Planned

1. Implement OIR (Obsidian Intermediate Representation) lowering layer
2. `tremor`/`catch` error handling with stack unwinding (today only `throw`, which ends the program)
3. Command-line arguments (`argv`) in the language
4. `layout` on `monolith` (reserved word; the parser does not accept it yet)

The interpreter (`interpreter.ofs`), crystallization, OSR, Fault Refract (L0 to L3), `quartz` and messages with `file:line:col` already exist.

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

Timing depends on program size, optimization level and the native toolchain. Use `bash bench/run.sh` to measure programs and record the configuration and results. For crystallization, measure IR generation, cold compilation, cached loading and execution after promotion separately; AOT build time does not include program execution.

---

## Active Architecture: Direct LLVM Backend

The OFS compiler emits LLVM IR directly without intermediate C code:

```
llvmgen.ofs → output.ll (LLVM IR) → llc -filetype=obj → output.o → ld (with magma.o) → executable
```

Benefits:
- Direct LLVM IR emission without intermediate C code; the driver can use `llc` and the native linker. The hybrid path compiles its library with `clang`.
- Byte-for-byte reproducibility and deterministic IR emission
- Compilation time depends on the program, optimization level and selected native toolchain
- 100% pure OFS runtime via Stack Magma (`magma.o`)
- First-class cross-compilation support (Linux ELF and Windows PE32+)

---

**Architecture Status**: Production & Self-Hosted
**Verification**: Verified via `cmp -s v3.ll v4.ll` (zero diff)
