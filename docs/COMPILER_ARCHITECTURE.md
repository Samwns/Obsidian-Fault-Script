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
│  (chars,     │   (80 kinds)       │  (scope stack)     │
│   tokens)    │                    └────────────────────┘
└──────┬───────┘
       │
       ▼
┌──────────────┐
│   Parser     │ → AST Root ID (stone)
│  (recursive  │   (60+ node kinds)
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
       ▼
┌──────────────┐
│  Codegen     │ → output.c
│  (emit C)    │   (type mapping OFS→C)
└──────┬───────┘
       │
       ▼
   output.c → gcc -O2 → executable
```

---

## Phase 1: Lexer (Tokenization)

**File**: `ofs/ofscc/lexer.ofs` (660 lines)

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
   - Keywords (34): forge, vein, if, while, match, etc.
   - Operators (25+): +, -, *, /, ==, !=, &&, ||, [], etc.
   - Literals: 42, 3.14, "hello\n", true, false, null
   - Identifiers: x, _var, CamelCase
   - Comments: // and /* */

3. **Multi-character operators**:
   - ++ / -- / += / -= / *= /= / == / != / <= / >= / && / || / << / >> / ->

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
- **Escape sequences** — \n, \t, \\, \", \0 handled
- **Hex/float literals** — 0xFF, 1.0e-5 supported

---

## Phase 2: Parser (AST Construction)

**file**: `ofs/ofscc/parser.ofs` (650+ lines)

### Strategy

1. **Recursive Descent** for statements + declarations
2. **Pratt Parser** for expressions (operator precedence)
3. **Pool-based AST** — nodes stored in global Array<Node>

### Grammar (Simplified)

```
program     ::= (decl | stmt)*
decl        ::= fn_decl | var_decl | monolith_decl
fn_decl     ::= ("vein" | "core") IDENT "(" params ")" ("->" type)? block
var_decl    ::= ("forge" | "const") IDENT (":" type)? ("=" expr)? ";"

stmt        ::= expr_stmt | if_stmt | while_stmt | return_stmt | block
if_stmt     ::= "if" "(" expr ")" block ("else" block)?
while_stmt  ::= "while" "(" expr ")" block
return_stmt ::= "return" expr? ";"

expr        ::= assign_expr
assign_expr ::= or_expr (("=" | "+=" | "-=") or_expr)*
or_expr     ::= and_expr ("||" and_expr)*
and_expr    ::= eq_expr ("&&" eq_expr)*
eq_expr     ::= lt_expr (("==" | "!=") lt_expr)*
lt_expr     ::= add_expr (("<" | "<=" | ">" | ">=") add_expr)*
add_expr    ::= mul_expr (("+" | "-") mul_expr)*
mul_expr    ::= unary_expr (("*" | "/" | "%") unary_expr)*
unary_expr  ::= ("!" | "-" | "&" | "*") unary_expr | postfix_expr
postfix_expr::= primary_expr ("[" expr "]" | "." IDENT | "++" | "--")*

primary_expr::= IDENT | INT | FLOAT | STRING | "true" | "false" | "null"
             | "(" expr ")" | "[" expr_list "]" | call_expr
```

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
12. Unary - ! ~     (prefix)
13. ++ --           (postfix)
14. [] . ()         (postfix: index, field, call)
```

---

## Phase 3: Type Checker

**File**: `ofs/ofscc/typeck.ofs` (350+ lines)

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

## Phase 4: Code Generator

**File**: `ofs/ofscc/codegen.ofs` (400+ lines)

### Type Mapping (OFS → C)

```ofs
stone      → int64_t
crystal    → double
obsidian   → char*
bool       → int
void       → void
u8, u16, u32, u64 → uint8_t, uint16_t, uint32_t, uint64_t
i8, i32    → int8_t, int32_t
Array<T>   → (handled separately)
```

### Code Emission Strategy

1. **Header** — #include, typedefs
2. **Forward declarations** — Function prototypes
3. **Function definitions** — In declaration order
4. **Main logic** — Program body

### Example: `forge x = 42`

**AST**:
```
NK_VAR_DECL
├─ val: "x"
├─ c0: -1  (no type annotation)
└─ c1: 2   (init expr)
   └─ NK_LIT_INT, val: "42"
```

**Generated C**:
```c
int64_t x = 42;
```

### Example: `vein add(a: stone, b: stone) -> stone { return a + b }`

**Generated C**:
```c
int64_t add(int64_t a, int64_t b) {
    return (a + b);
}
```

### String Output (No Buffering)

```ofs
forge _out_file: stone = 0

vein cg_open(path: obsidian) -> void {
    _out_file = fopen(path, "w")
}

vein cg_emit(line: obsidian) -> void {
    fputs(line, _out_file)
    fputs("\n", _out_file)
}
```

**Why**: Avoids O(n²) concatenation — writes directly to file line-by-line.

---

## Phase 5: Driver

**File**: `ofs/ofscc/ofscc.ofs`

### Pipeline

```ofs
core main() {
    // 1. Read source
    forge src = read_file(input_file)
    
    // 2. Lex
    forge tokens = lexer.lex(src)
    
    // 3. Parse
    forge root_id = parser.parse(tokens)
    
    // 4. Type check
    typeck.check(root_id)
    
    // 5. Codegen
    cg_open(c_output)
    codegen.generate(root_id)
    cg_close()
    
    // 6. Compile with GCC
    system("gcc -O2 -o " + output_file + " " + c_output)
}
```

---

## Memory Model

### Static Allocation

- Token array: ~10KB (typical)
- Node pool: ~1MB (large programs)
- Symbol table: ~100KB

### Dynamic Allocation (via FFI)

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

✓ **Deterministic**:
- Token order matches source order
- AST order matches source order  
- Symbol table ordered byInsertion
- No timestamps or random data

✗ **Non-deterministic** (avoid):
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

1. Add support for impl/namespace/strata
2. Add tremor/catch error handling
3. Path-aware error messages
4. Optimize generated C

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
| Codegen | ~10ms | File I/O |
| GCC | ~100ms | C compiler |
| **Total** | ~150ms | C compiler |

---

## Future: LLVM Backend

Instead of generating C:

```
codegen.ofs → output.ll (LLVM IR) → llc → output.o → ld → executable
```

Benefits:
- Direct compilation (no GCC dependency)
- Better optimization opportunities
- Faster compilation
- Direct to machine code

---

**Architecture Status**: ✅ Complete and working
**Next**: Verify bootstrap + production hardening
