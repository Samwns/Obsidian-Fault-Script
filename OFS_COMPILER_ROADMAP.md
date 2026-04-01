# Obsidian Fault Script (OFS) — Compiler Roadmap & Build Instructions

> Complete guide to build OFS from scratch using LLVM as backend.
> Send this file to your Copilot agent (Sonnet or Opus).

---

## LANGUAGE OVERVIEW

OFS is a statically-typed, compiled language that generates native machine code via LLVM.
It combines high-level simplicity with low-level control, using geology-themed keywords.

### Syntax at a glance

```ofs
// Entry point
core main() {
    forge x = 10
    forge y: stone = 20
    echo(x + y)
}

// Types
// stone     -> integer (i64)
// crystal   -> float   (f64)
// obsidian  -> string  (i8*)
// shard     -> pointer (*T)
// void      -> void
// bool      -> bool (i1)

// Variables
forge name: obsidian = "Rex"
forge pi: crystal    = 3.14
forge count: stone   = 0

// Collections (dynamic array)
forge nums = [1, 2, 3, 4, 5]

// Control flow
if (x > 10) {
    echo("strong")
} else {
    echo("weak")
}

cycle (i in nums) {
    echo(i)
}

cycle (forge i = 0; i < 10; i++) {
    echo(i)
}

// Structures
monolith Player {
    name: obsidian
    hp:   stone
    speed: crystal
}

// Functions
vein greet(name: obsidian) -> obsidian {
    return "Hello, " + name
}

// Safe pointer block
fracture {
    shard p: *stone = &x
    *p = 50
}

// Unsafe memory block
abyss {
    mem[0x00FF] = 1
}
```

---

## PROJECT STRUCTURE

```
ofs/
├── src/
│   ├── main.cpp                  <- Entry point (CLI)
│   ├── lexer/
│   │   ├── token.hpp             <- Token types and Token struct
│   │   ├── lexer.hpp             <- Lexer class declaration
│   │   └── lexer.cpp             <- Lexer implementation
│   ├── parser/
│   │   ├── parser.hpp            <- Parser class declaration
│   │   └── parser.cpp            <- Parser implementation (produces AST)
│   ├── ast/
│   │   ├── ast.hpp               <- All AST node types
│   │   └── ast_printer.hpp       <- Debug printer for AST
│   ├── semantic/
│   │   ├── semantic.hpp          <- Semantic analyzer declaration
│   │   ├── semantic.cpp          <- Type checking, scope resolution
│   │   └── type_system.hpp       <- OFS type system definitions
│   ├── codegen/
│   │   ├── codegen.hpp           <- LLVM code generator declaration
│   │   └── codegen.cpp           <- LLVM IR generation from AST
│   └── runtime/
│       ├── ofs_runtime.c         <- Runtime functions (echo, alloc, etc.)
│       └── ofs_runtime.h         <- Runtime declarations
├── stdlib/
│   └── core.ofs                  <- Standard library in OFS itself
├── tests/
│   ├── test_lexer.cpp
│   ├── test_parser.cpp
│   ├── test_semantic.cpp
│   └── test_codegen.cpp
├── examples/
│   ├── hello.ofs
│   ├── fizzbuzz.ofs
│   ├── structs.ofs
│   └── pointers.ofs
├── CMakeLists.txt                <- Build system
└── README.md
```

---

## PHASE 1 — LEXER

### File: `src/lexer/token.hpp`

```cpp
#pragma once
#include <string>
#include <unordered_map>

namespace ofs {

enum class TokenKind {
    // ── Literals ──────────────────────────────────────────────────────────
    INT_LIT,        // 42
    FLOAT_LIT,      // 3.14
    STRING_LIT,     // "hello"
    BOOL_LIT,       // true, false
    NULL_LIT,       // null

    // ── Identifiers ───────────────────────────────────────────────────────
    IDENT,          // any user-defined name

    // ── OFS Keywords ──────────────────────────────────────────────────────
    KW_CORE,        // core     (entry point / public function)
    KW_VEIN,        // vein     (function definition)
    KW_FORGE,       // forge    (variable declaration)
    KW_MONOLITH,    // monolith (struct / record)
    KW_FRACTURE,    // fracture (safe pointer block)
    KW_ABYSS,       // abyss    (unsafe memory block)
    KW_CYCLE,       // cycle    (loop)
    KW_IF,          // if
    KW_ELSE,        // else
    KW_RETURN,      // return
    KW_ECHO,        // echo     (print)
    KW_IN,          // in       (for range-based cycle)
    KW_SHARD,       // shard    (pointer type)
    KW_TRUE,        // true
    KW_FALSE,       // false
    KW_NULL,        // null
    KW_BREAK,       // break
    KW_CONTINUE,    // continue

    // ── Type Keywords ─────────────────────────────────────────────────────
    KW_STONE,       // stone    -> i64
    KW_CRYSTAL,     // crystal  -> f64
    KW_OBSIDIAN,    // obsidian -> i8* (string)
    KW_BOOL,        // bool     -> i1
    KW_VOID,        // void

    // ── Operators ─────────────────────────────────────────────────────────
    PLUS,           // +
    MINUS,          // -
    STAR,           // *
    SLASH,          // /
    PERCENT,        // %
    AMP,            // &   (address-of)
    PIPE,           // |
    CARET,          // ^
    TILDE,          // ~
    BANG,           // !
    EQ,             // =
    EQ_EQ,          // ==
    BANG_EQ,        // !=
    LT,             // <
    LT_EQ,          // <=
    GT,             // >
    GT_EQ,          // >=
    AMP_AMP,        // &&
    PIPE_PIPE,      // ||
    PLUS_EQ,        // +=
    MINUS_EQ,       // -=
    STAR_EQ,        // *=
    SLASH_EQ,       // /=
    PLUS_PLUS,      // ++
    MINUS_MINUS,    // --
    ARROW,          // ->
    COLON,          // :
    DOT,            // .

    // ── Delimiters ────────────────────────────────────────────────────────
    LPAREN,         // (
    RPAREN,         // )
    LBRACE,         // {
    RBRACE,         // }
    LBRACKET,       // [
    RBRACKET,       // ]
    COMMA,          // ,
    SEMICOLON,      // ; (optional, used as separator)
    NEWLINE,        // \n (significant for statement separation)

    // ── Special ───────────────────────────────────────────────────────────
    END_OF_FILE,
    ERROR
};

struct Token {
    TokenKind   kind;
    std::string lexeme;  // raw text from source
    int         line;
    int         col;

    // Value accessors (populated during lexing)
    int64_t     int_val   = 0;
    double      float_val = 0.0;

    std::string to_string() const;
};

// Map keyword strings to TokenKind
const std::unordered_map<std::string, TokenKind> KEYWORDS = {
    {"core",     TokenKind::KW_CORE},
    {"vein",     TokenKind::KW_VEIN},
    {"forge",    TokenKind::KW_FORGE},
    {"monolith", TokenKind::KW_MONOLITH},
    {"fracture", TokenKind::KW_FRACTURE},
    {"abyss",    TokenKind::KW_ABYSS},
    {"cycle",    TokenKind::KW_CYCLE},
    {"if",       TokenKind::KW_IF},
    {"else",     TokenKind::KW_ELSE},
    {"return",   TokenKind::KW_RETURN},
    {"echo",     TokenKind::KW_ECHO},
    {"in",       TokenKind::KW_IN},
    {"shard",    TokenKind::KW_SHARD},
    {"true",     TokenKind::KW_TRUE},
    {"false",    TokenKind::KW_FALSE},
    {"null",     TokenKind::KW_NULL},
    {"break",    TokenKind::KW_BREAK},
    {"continue", TokenKind::KW_CONTINUE},
    {"stone",    TokenKind::KW_STONE},
    {"crystal",  TokenKind::KW_CRYSTAL},
    {"obsidian", TokenKind::KW_OBSIDIAN},
    {"bool",     TokenKind::KW_BOOL},
    {"void",     TokenKind::KW_VOID},
};

} // namespace ofs
```

### File: `src/lexer/lexer.hpp` and `lexer.cpp`

```cpp
// lexer.hpp
#pragma once
#include "token.hpp"
#include <string>
#include <vector>

namespace ofs {

class Lexer {
public:
    explicit Lexer(const std::string& source, const std::string& filename = "<source>");
    std::vector<Token> tokenize();

private:
    Token next_token();

    // Character helpers
    char peek(int offset = 0) const;
    char advance();
    bool match(char expected);
    bool is_at_end() const;
    bool is_digit(char c) const;
    bool is_alpha(char c) const;
    bool is_alnum(char c) const;

    // Token builders
    Token make_token(TokenKind kind);
    Token make_error(const std::string& msg);
    Token lex_string();      // "..."
    Token lex_number();      // 42 or 3.14
    Token lex_ident();       // identifier or keyword
    void  skip_whitespace(); // spaces, tabs, comments (//)
    void  skip_block_comment(); // /* ... */

    std::string source_;
    std::string filename_;
    size_t      pos_  = 0;
    int         line_ = 1;
    int         col_  = 1;
    size_t      tok_start_ = 0;
    int         tok_line_  = 1;
    int         tok_col_   = 1;
};

} // namespace ofs
```

**Lexer implementation rules:**
- Comments: `//` single line, `/* */` block
- Strings: double-quoted `"..."` with escape sequences `\n \t \\ \"`
- Numbers: integers `42`, `-5`, floats `3.14`, hex `0xFF`
- Identifiers: `[a-zA-Z_][a-zA-Z0-9_]*`
- Statement termination: newline OR semicolon (both valid)
- Whitespace (except newline) is ignored
- Newlines inside `()`, `[]`, `{}` are ignored (implicit line continuation)

---

## PHASE 2 — AST NODES

### File: `src/ast/ast.hpp`

```cpp
#pragma once
#include <string>
#include <vector>
#include <memory>
#include <optional>

namespace ofs {

// Forward declarations
struct Expr;
struct Stmt;
struct Decl;
using ExprPtr = std::unique_ptr<Expr>;
using StmtPtr = std::unique_ptr<Stmt>;
using DeclPtr = std::unique_ptr<Decl>;

// ── Type representation ───────────────────────────────────────────────────

enum class BaseType {
    Stone,      // i64
    Crystal,    // f64
    Obsidian,   // i8* (string)
    Bool,       // i1
    Void,       // void
    Shard,      // pointer (wraps another type)
    Array,      // dynamic array
    Named,      // user-defined struct name
    Infer,      // type to be inferred
};

struct OFSType {
    BaseType              base;
    std::string           name;           // for Named types
    std::shared_ptr<OFSType> inner;       // for Shard<T> and Array<T>
    bool                  is_mut = true;  // mutability

    static OFSType stone()    { return {BaseType::Stone}; }
    static OFSType crystal()  { return {BaseType::Crystal}; }
    static OFSType obsidian() { return {BaseType::Obsidian}; }
    static OFSType boolean()  { return {BaseType::Bool}; }
    static OFSType void_t()   { return {BaseType::Void}; }
    static OFSType infer()    { return {BaseType::Infer}; }
    static OFSType shard_of(OFSType inner) {
        OFSType t; t.base = BaseType::Shard;
        t.inner = std::make_shared<OFSType>(inner);
        return t;
    }
    static OFSType array_of(OFSType inner) {
        OFSType t; t.base = BaseType::Array;
        t.inner = std::make_shared<OFSType>(inner);
        return t;
    }
    static OFSType named(const std::string& n) {
        OFSType t; t.base = BaseType::Named; t.name = n;
        return t;
    }

    std::string to_string() const;
    bool operator==(const OFSType& o) const;
    bool is_numeric() const;
    bool is_pointer() const;
};

// ── Expressions ───────────────────────────────────────────────────────────

struct Expr {
    int      line = 0, col = 0;
    OFSType  resolved_type; // filled by semantic analyzer
    virtual ~Expr() = default;
};

struct IntLitExpr    : Expr { int64_t value; };
struct FloatLitExpr  : Expr { double value; };
struct StringLitExpr : Expr { std::string value; };
struct BoolLitExpr   : Expr { bool value; };
struct NullLitExpr   : Expr {};

struct IdentExpr     : Expr { std::string name; };

struct BinaryExpr : Expr {
    std::string op; // "+", "-", "*", "/", "%", "==", "!=", "<", "<=", ">", ">=", "&&", "||"
    ExprPtr     left, right;
};

struct UnaryExpr : Expr {
    std::string op;  // "-", "!", "~", "*" (deref), "&" (addr-of), "++", "--"
    ExprPtr     operand;
    bool        prefix = true;
};

struct AssignExpr : Expr {
    std::string op;     // "=", "+=", "-=", "*=", "/="
    ExprPtr     target; // IdentExpr, IndexExpr, or UnaryExpr(* deref)
    ExprPtr     value;
};

struct CallExpr : Expr {
    ExprPtr              callee; // IdentExpr or MemberExpr
    std::vector<ExprPtr> args;
};

struct EchoExpr : Expr {
    // echo(val1, val2, ...) — built-in print
    std::vector<ExprPtr> args;
};

struct IndexExpr : Expr {
    ExprPtr object;
    ExprPtr index;
};

struct MemberExpr : Expr {
    ExprPtr     object;
    std::string field;
};

struct ArrayLitExpr : Expr {
    std::vector<ExprPtr> elements;
};

struct TernaryExpr : Expr {
    ExprPtr cond, then_expr, else_expr;
};

struct CastExpr : Expr {
    OFSType target_type;
    ExprPtr expr;
};

// ── Statements ────────────────────────────────────────────────────────────

struct Stmt {
    int line = 0, col = 0;
    virtual ~Stmt() = default;
};

struct ExprStmt     : Stmt { ExprPtr expr; };
struct BlockStmt    : Stmt { std::vector<StmtPtr> stmts; };
struct ReturnStmt   : Stmt { ExprPtr value; /* nullable for void */ };
struct BreakStmt    : Stmt {};
struct ContinueStmt : Stmt {};

struct ForgeStmt : Stmt {  // forge x = ... or forge x: stone = ...
    std::string         name;
    std::optional<OFSType> type_ann;   // optional explicit type annotation
    ExprPtr             initializer;   // nullable (default-initialized)
};

struct IfStmt : Stmt {
    ExprPtr  cond;
    StmtPtr  then_block;
    StmtPtr  else_block; // nullable
};

// cycle (i in list) { ... }        -- range-based
// cycle (forge i=0; i<n; i++) { }  -- C-style
struct CycleStmt : Stmt {
    // Range-based
    bool    is_range = false;
    std::string range_var;   // "i"
    ExprPtr     range_expr;  // the collection

    // C-style
    StmtPtr init;   // forge i=0
    ExprPtr cond;   // i<n
    ExprPtr step;   // i++
    StmtPtr body;
};

struct FractureStmt : Stmt {
    // fracture { ... } — safe pointer block
    StmtPtr body;
};

struct AbyssStmt : Stmt {
    // abyss { ... } — unsafe memory block
    StmtPtr body;
};

// ── Declarations ──────────────────────────────────────────────────────────

struct Param {
    std::string name;
    OFSType     type;
};

struct Decl {
    int line = 0, col = 0;
    virtual ~Decl() = default;
};

// vein name(params) -> ret_type { body }
// core main() { body }    (is_core = true)
struct FuncDecl : Decl {
    std::string        name;
    std::vector<Param> params;
    OFSType            return_type;
    StmtPtr            body;
    bool               is_core = false; // core main()
};

// monolith Player { name: obsidian, hp: stone }
struct MonolithDecl : Decl {
    std::string              name;
    std::vector<Param>       fields;  // Param reused: {name, type}
    std::vector<std::unique_ptr<FuncDecl>> methods; // optional methods
};

// Top-level forge (global variable)
struct GlobalForgeDecl : Decl {
    std::string         name;
    std::optional<OFSType> type_ann;
    ExprPtr             initializer;
};

// ── Translation unit ──────────────────────────────────────────────────────

struct Module {
    std::string         filename;
    std::vector<DeclPtr> decls;
};

} // namespace ofs
```

---

## PHASE 3 — PARSER

### File: `src/parser/parser.hpp`

```cpp
#pragma once
#include "../lexer/token.hpp"
#include "../ast/ast.hpp"
#include <vector>
#include <stdexcept>

namespace ofs {

struct ParseError : std::runtime_error {
    int line, col;
    ParseError(const std::string& msg, int l, int c)
        : std::runtime_error(msg), line(l), col(c) {}
};

class Parser {
public:
    explicit Parser(std::vector<Token> tokens);
    Module parse(); // parse entire module

private:
    // ── Declarations ─────────────────────────────────────────────────────
    DeclPtr parse_decl();
    std::unique_ptr<FuncDecl>     parse_func(bool is_core = false);
    std::unique_ptr<MonolithDecl> parse_monolith();
    std::unique_ptr<GlobalForgeDecl> parse_global_forge();

    // ── Statements ────────────────────────────────────────────────────────
    StmtPtr parse_stmt();
    StmtPtr parse_block();           // { stmts... }
    StmtPtr parse_forge_stmt();      // forge x = ...
    StmtPtr parse_if_stmt();         // if (cond) { } else { }
    StmtPtr parse_cycle_stmt();      // cycle (...) { }
    StmtPtr parse_return_stmt();     // return expr
    StmtPtr parse_fracture_stmt();   // fracture { }
    StmtPtr parse_abyss_stmt();      // abyss { }

    // ── Expressions (Pratt parser with precedence climbing) ───────────────
    ExprPtr parse_expr(int min_prec = 0);
    ExprPtr parse_primary();
    ExprPtr parse_call(ExprPtr callee);
    ExprPtr parse_index(ExprPtr object);
    ExprPtr parse_member(ExprPtr object);
    ExprPtr parse_unary();
    ExprPtr parse_array_literal();

    // ── Types ─────────────────────────────────────────────────────────────
    OFSType parse_type();
    OFSType parse_shard_type();      // *stone, *crystal, etc.

    // ── Helpers ───────────────────────────────────────────────────────────
    Token& peek(int offset = 0);
    Token& advance();
    bool   check(TokenKind kind) const;
    bool   match(TokenKind kind);
    bool   match_any(std::initializer_list<TokenKind> kinds);
    Token  expect(TokenKind kind, const std::string& msg);
    void   skip_newlines();
    bool   is_at_end() const;
    ParseError error(const std::string& msg);

    // Operator precedence table
    int    precedence(TokenKind op) const;
    bool   is_binary_op(TokenKind kind) const;
    bool   is_right_assoc(TokenKind op) const;
    std::string token_to_op(TokenKind kind) const;

    std::vector<Token> tokens_;
    size_t             pos_ = 0;
};

} // namespace ofs
```

**Operator precedence (lowest to highest):**

```
Level 1  (lowest):  ||
Level 2:            &&
Level 3:            == !=
Level 4:            < <= > >=
Level 5:            + -
Level 6:            * / %
Level 7  (highest): unary: - ! ~ * & ++ --
```

**Parsing rules:**
- `forge` starts a variable declaration
- `vein` starts a function
- `core` starts the main entry function
- `monolith` starts a struct
- `cycle` starts a loop — detect `in` keyword to distinguish range-based vs C-style
- Blocks are `{ stmts... }`, no indentation rules
- Statements are separated by newline or `;`
- `echo(...)` is parsed as `EchoExpr` (built-in, not a regular function call)

---

## PHASE 4 — SEMANTIC ANALYSIS

### File: `src/semantic/type_system.hpp`

```cpp
#pragma once
#include "../ast/ast.hpp"
#include <string>
#include <unordered_map>
#include <vector>
#include <stdexcept>

namespace ofs {

struct SemanticError : std::runtime_error {
    int line, col;
    SemanticError(const std::string& msg, int l=0, int c=0)
        : std::runtime_error(msg), line(l), col(c) {}
};

// Symbol kinds
enum class SymbolKind { Variable, Function, Monolith, Param };

struct Symbol {
    std::string name;
    OFSType     type;
    SymbolKind  kind;
    bool        is_mut   = true;
    bool        is_const = false;
    int         scope_depth = 0;
};

// Scope stack
class Scope {
public:
    Scope();
    void push();                          // enter new scope
    void pop();                           // exit scope
    void define(const std::string& name, Symbol sym);
    const Symbol* lookup(const std::string& name) const;
    bool exists_in_current(const std::string& name) const;
    int  depth() const;

private:
    std::vector<std::unordered_map<std::string, Symbol>> stack_;
};

} // namespace ofs
```

### File: `src/semantic/semantic.hpp`

```cpp
#pragma once
#include "type_system.hpp"

namespace ofs {

class SemanticAnalyzer {
public:
    void analyze(Module& mod); // mutates AST: fills resolved_type fields

private:
    // ── Declarations ─────────────────────────────────────────────────────
    void check_func(FuncDecl& fn);
    void check_monolith(MonolithDecl& m);
    void check_global_forge(GlobalForgeDecl& g);

    // ── Statements ────────────────────────────────────────────────────────
    void check_stmt(Stmt& s);
    void check_block(BlockStmt& s);
    void check_forge(ForgeStmt& s);
    void check_if(IfStmt& s);
    void check_cycle(CycleStmt& s);
    void check_return(ReturnStmt& s);
    void check_fracture(FractureStmt& s);
    void check_abyss(AbyssStmt& s);

    // ── Expressions ───────────────────────────────────────────────────────
    OFSType check_expr(Expr& e);
    OFSType check_binary(BinaryExpr& e);
    OFSType check_unary(UnaryExpr& e);
    OFSType check_call(CallExpr& e);
    OFSType check_echo(EchoExpr& e);
    OFSType check_assign(AssignExpr& e);
    OFSType check_index(IndexExpr& e);
    OFSType check_member(MemberExpr& e);
    OFSType check_array_lit(ArrayLitExpr& e);

    // ── Type helpers ──────────────────────────────────────────────────────
    OFSType infer_type(const Expr& init); // infer type from initializer
    bool    is_assignable(const OFSType& to, const OFSType& from) const;
    bool    is_numeric(const OFSType& t)  const;
    OFSType promote(const OFSType& a, const OFSType& b) const; // int+float -> float
    void    expect_type(const OFSType& got, const OFSType& want, int line, int col);

    // ── State ─────────────────────────────────────────────────────────────
    Scope       scope_;
    OFSType     current_return_type_;    // for return checking
    bool        inside_fracture_ = false;
    bool        inside_abyss_    = false;

    // Monolith registry (for field access checks)
    std::unordered_map<std::string, MonolithDecl*> monoliths_;
};

} // namespace ofs
```

**Type rules:**
- `stone + stone` → `stone`
- `stone + crystal` → `crystal` (widening)
- `crystal + crystal` → `crystal`
- `obsidian + obsidian` → `obsidian` (concatenation)
- `shard` arithmetic only allowed inside `fracture` or `abyss`
- `abyss` allows raw memory access — skip type checking inside it
- `fracture` allows pointer ops but still type-checks
- `forge x = 10` → infer as `stone`
- `forge x = 3.14` → infer as `crystal`
- `forge x = "hi"` → infer as `obsidian`
- `forge x = true` → infer as `bool`

---

## PHASE 5 — LLVM CODE GENERATION

### File: `src/codegen/codegen.hpp`

```cpp
#pragma once
#include "../ast/ast.hpp"
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Type.h>
#include <memory>
#include <unordered_map>

namespace ofs {

class CodeGen {
public:
    CodeGen();

    // Generate LLVM IR from AST module
    // Returns the LLVM Module (ready to compile to object file)
    std::unique_ptr<llvm::Module> generate(const Module& mod);

    // Emit LLVM IR as text to a file (for debugging: ofs build --emit-llvm)
    void emit_ir(const std::string& filepath);

    // Compile to native object file
    void emit_object(const std::string& filepath);

    // Link and produce final executable
    void link(const std::string& obj_file, const std::string& out_file);

private:
    // ── LLVM IR Generation ────────────────────────────────────────────────
    void       gen_func(const FuncDecl& fn);
    void       gen_monolith(const MonolithDecl& m);
    void       gen_global_forge(const GlobalForgeDecl& g);

    void       gen_stmt(const Stmt& s);
    void       gen_block(const BlockStmt& s);
    void       gen_forge(const ForgeStmt& s);
    void       gen_if(const IfStmt& s);
    void       gen_cycle(const CycleStmt& s);
    void       gen_return(const ReturnStmt& s);
    void       gen_fracture(const FractureStmt& s);
    void       gen_abyss(const AbyssStmt& s);
    void       gen_echo(const EchoExpr& e);

    llvm::Value* gen_expr(const Expr& e);
    llvm::Value* gen_binary(const BinaryExpr& e);
    llvm::Value* gen_unary(const UnaryExpr& e);
    llvm::Value* gen_call(const CallExpr& e);
    llvm::Value* gen_assign(const AssignExpr& e);
    llvm::Value* gen_index(const IndexExpr& e);
    llvm::Value* gen_member(const MemberExpr& e);
    llvm::Value* gen_array_lit(const ArrayLitExpr& e);

    // ── LLVM Type Mapping ─────────────────────────────────────────────────
    // stone    -> llvm::Type::getInt64Ty(ctx)
    // crystal  -> llvm::Type::getDoubleTy(ctx)
    // obsidian -> llvm::Type::getInt8PtrTy(ctx) (or ptr in LLVM 15+)
    // bool     -> llvm::Type::getInt1Ty(ctx)
    // void     -> llvm::Type::getVoidTy(ctx)
    // shard<T> -> llvm::PointerType::get(llvm_type(T), 0)
    // array<T> -> struct { T* data, i64 len, i64 cap }
    llvm::Type*     llvm_type(const OFSType& t);
    llvm::Type*     llvm_array_struct(const OFSType& element_type);

    // ── Runtime function declarations ─────────────────────────────────────
    // These are declared in LLVM IR and linked from ofs_runtime.c
    void declare_runtime();
    llvm::Function* get_runtime_fn(const std::string& name);

    // echo_stone(i64)       -- print integer
    // echo_crystal(f64)     -- print float
    // echo_obsidian(i8*)    -- print string
    // echo_bool(i1)         -- print bool
    // echo_newline()        -- print \n
    // ofs_alloc(i64) -> i8* -- allocate memory
    // ofs_free(i8*)         -- free memory
    // ofs_array_new(i64, i64) -> ptr  -- create array(len, elem_size)
    // ofs_array_push(ptr, i8*, i64)   -- push element

    // ── Helpers ───────────────────────────────────────────────────────────
    llvm::Value*    get_var(const std::string& name);
    void            set_var(const std::string& name, llvm::Value* alloca);
    llvm::AllocaInst* create_alloca(llvm::Function* fn,
                                     const std::string& name,
                                     llvm::Type* ty);

    // ── State ─────────────────────────────────────────────────────────────
    llvm::LLVMContext                 ctx_;
    llvm::IRBuilder<>                 builder_;
    std::unique_ptr<llvm::Module>     mod_;

    // Variable name -> alloca (stack slot)
    // Stack of scopes: each scope is a map
    std::vector<std::unordered_map<std::string, llvm::Value*>> var_stack_;

    // Monolith (struct) type cache
    std::unordered_map<std::string, llvm::StructType*> struct_types_;

    // Current function being generated
    llvm::Function*  cur_fn_ = nullptr;

    // Labels for break/continue in cycle
    llvm::BasicBlock* cycle_exit_bb_    = nullptr;
    llvm::BasicBlock* cycle_header_bb_  = nullptr;

    // Runtime function cache
    std::unordered_map<std::string, llvm::Function*> runtime_fns_;
};

} // namespace ofs
```

### LLVM IR Examples

**Hello World — what the codegen must produce:**

```llvm
; OFS "Hello World" → LLVM IR

@.str = private unnamed_addr constant [13 x i8] c"Hello world\0A\00"

declare i32 @puts(i8*)

define i32 @main() {
entry:
  %0 = getelementptr [13 x i8], [13 x i8]* @.str, i64 0, i64 0
  call i32 @puts(i8* %0)
  ret i32 0
}
```

**Variable + arithmetic:**

```ofs
core main() {
    forge x: stone = 10
    forge y: stone = 20
    forge z = x + y
    echo(z)
}
```
→ LLVM IR:
```llvm
define i32 @main() {
entry:
  %x = alloca i64
  store i64 10, i64* %x
  %y = alloca i64
  store i64 20, i64* %y
  %z = alloca i64
  %x_val = load i64, i64* %x
  %y_val = load i64, i64* %y
  %sum   = add i64 %x_val, %y_val
  store i64 %sum, i64* %z
  %z_val = load i64, i64* %z
  call void @echo_stone(i64 %z_val)
  ret i32 0
}
```

**if statement:**

```ofs
if (x > 10) { echo("strong") } else { echo("weak") }
```
→ LLVM IR:
```llvm
  %x_val  = load i64, i64* %x
  %cond   = icmp sgt i64 %x_val, 10
  br i1 %cond, label %then, label %else

then:
  call void @echo_obsidian(i8* @.str_strong)
  br label %merge

else:
  call void @echo_obsidian(i8* @.str_weak)
  br label %merge

merge:
  ; continue
```

**cycle (C-style):**

```ofs
cycle (forge i = 0; i < 5; i++) { echo(i) }
```
→ LLVM IR:
```llvm
  %i = alloca i64
  store i64 0, i64* %i
  br label %cycle_cond

cycle_cond:
  %i_val = load i64, i64* %i
  %cond  = icmp slt i64 %i_val, 5
  br i1 %cond, label %cycle_body, label %cycle_exit

cycle_body:
  %i_print = load i64, i64* %i
  call void @echo_stone(i64 %i_print)
  br label %cycle_step

cycle_step:
  %i_cur  = load i64, i64* %i
  %i_next = add i64 %i_cur, 1
  store i64 %i_next, i64* %i
  br label %cycle_cond

cycle_exit:
  ; continue
```

**monolith (struct):**

```ofs
monolith Player { name: obsidian, hp: stone }
```
→ LLVM IR:
```llvm
%Player = type { i8*, i64 }
; field 0: name (i8*)
; field 1: hp   (i64)
```

Access `p.hp`:
```llvm
  %hp_ptr = getelementptr %Player, %Player* %p, i32 0, i32 1
  %hp_val = load i64, i64* %hp_ptr
```

**fracture (safe pointer block):**

```ofs
fracture {
    shard p: *stone = &x
    *p = 50
}
```
→ LLVM IR:
```llvm
  ; fracture block — same as normal code, just allows pointer ops
  %p = alloca i64*
  store i64* %x, i64** %p          ; p = &x
  %p_val = load i64*, i64** %p
  store i64 50, i64* %p_val        ; *p = 50
```

---

## PHASE 6 — RUNTIME LIBRARY

### File: `src/runtime/ofs_runtime.c`

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// ── Echo functions (called by echo() in OFS) ─────────────────────────────

void echo_stone(int64_t v) {
    printf("%lld\n", (long long)v);
}

void echo_crystal(double v) {
    // Print without trailing zeros when possible
    if (v == (int64_t)v) printf("%.1f\n", v);
    else                  printf("%g\n", v);
}

void echo_obsidian(const char* s) {
    puts(s ? s : "(null)");
}

void echo_bool(int v) {
    puts(v ? "true" : "false");
}

void echo_newline(void) {
    putchar('\n');
}

// ── Memory allocation ─────────────────────────────────────────────────────

void* ofs_alloc(int64_t size) {
    void* ptr = malloc((size_t)size);
    if (!ptr) { fputs("OFS: out of memory\n", stderr); exit(1); }
    return ptr;
}

void ofs_free(void* ptr) {
    free(ptr);
}

// ── Dynamic array ─────────────────────────────────────────────────────────
// Layout: { data: void*, len: i64, cap: i64 }

typedef struct {
    void*   data;
    int64_t len;
    int64_t cap;
    int64_t elem_size;
} OfsArray;

OfsArray* ofs_array_new(int64_t initial_cap, int64_t elem_size) {
    OfsArray* arr   = ofs_alloc(sizeof(OfsArray));
    arr->data       = ofs_alloc(initial_cap * elem_size);
    arr->len        = 0;
    arr->cap        = initial_cap;
    arr->elem_size  = elem_size;
    return arr;
}

void ofs_array_push(OfsArray* arr, void* element) {
    if (arr->len >= arr->cap) {
        arr->cap  = arr->cap ? arr->cap * 2 : 4;
        arr->data = realloc(arr->data, arr->cap * arr->elem_size);
        if (!arr->data) { fputs("OFS: array grow failed\n", stderr); exit(1); }
    }
    memcpy((char*)arr->data + arr->len * arr->elem_size, element, arr->elem_size);
    arr->len++;
}

void* ofs_array_get(OfsArray* arr, int64_t idx) {
    if (idx < 0 || idx >= arr->len) {
        fprintf(stderr, "OFS: index %lld out of bounds (len=%lld)\n",
                (long long)idx, (long long)arr->len);
        exit(1);
    }
    return (char*)arr->data + idx * arr->elem_size;
}

int64_t ofs_array_len(OfsArray* arr) {
    return arr->len;
}

// ── String operations ─────────────────────────────────────────────────────

char* ofs_str_concat(const char* a, const char* b) {
    size_t la = strlen(a), lb = strlen(b);
    char* result = ofs_alloc(la + lb + 1);
    memcpy(result, a, la);
    memcpy(result + la, b, lb);
    result[la + lb] = '\0';
    return result;
}

int ofs_str_eq(const char* a, const char* b) {
    return strcmp(a, b) == 0;
}
```

---

## PHASE 7 — CLI ENTRY POINT

### File: `src/main.cpp`

```cpp
#include <iostream>
#include <fstream>
#include <sstream>
#include "lexer/lexer.hpp"
#include "parser/parser.hpp"
#include "semantic/semantic.hpp"
#include "codegen/codegen.hpp"

void print_usage() {
    std::cout << R"(
ofs — Obsidian Fault Script compiler

Usage:
  ofs build  <file.ofs> [-o output]     Compile to native executable
  ofs check  <file.ofs>                 Type-check only (no output)
  ofs tokens <file.ofs>                 Print token stream (debug)
  ofs ast    <file.ofs>                 Print AST (debug)
  ofs ir     <file.ofs>                 Emit LLVM IR (debug)
  ofs run    <file.ofs>                 Compile and run immediately
  ofs version                           Print compiler version

)";
}

int main(int argc, char** argv) {
    if (argc < 2) { print_usage(); return 1; }

    std::string cmd  = argv[1];
    std::string file = argc > 2 ? argv[2] : "";
    std::string out  = "a.out";

    // Parse -o flag
    for (int i = 2; i < argc - 1; i++) {
        if (std::string(argv[i]) == "-o") out = argv[i+1];
    }

    if (cmd == "version") {
        std::cout << "ofs 0.1.0 — Obsidian Fault Script\n";
        return 0;
    }

    if (file.empty()) { print_usage(); return 1; }

    // Read source
    std::ifstream f(file);
    if (!f) { std::cerr << "ofs: cannot open '" << file << "'\n"; return 1; }
    std::string source((std::istreambuf_iterator<char>(f)), {});

    try {
        // 1. Lex
        ofs::Lexer lexer(source, file);
        auto tokens = lexer.tokenize();

        if (cmd == "tokens") {
            for (auto& t : tokens)
                std::cout << t.to_string() << "\n";
            return 0;
        }

        // 2. Parse
        ofs::Parser parser(std::move(tokens));
        auto mod = parser.parse();

        if (cmd == "ast") {
            ofs::print_ast(mod);
            return 0;
        }

        // 3. Semantic
        ofs::SemanticAnalyzer sem;
        sem.analyze(mod);

        if (cmd == "check") {
            std::cout << "OK — no errors\n";
            return 0;
        }

        // 4. Codegen
        ofs::CodeGen codegen;
        codegen.generate(mod);

        if (cmd == "ir") {
            codegen.emit_ir(out + ".ll");
            std::cout << "LLVM IR written to " << out << ".ll\n";
            return 0;
        }

        if (cmd == "build") {
            std::string obj = out + ".o";
            codegen.emit_object(obj);
            codegen.link(obj, out);
            std::cout << "Built: " << out << "\n";
            return 0;
        }

        if (cmd == "run") {
            codegen.emit_object("/tmp/ofs_tmp.o");
            codegen.link("/tmp/ofs_tmp.o", "/tmp/ofs_tmp");
            return std::system("/tmp/ofs_tmp");
        }

    } catch (const ofs::ParseError& e) {
        std::cerr << file << ":" << e.line << ":" << e.col
                  << ": parse error: " << e.what() << "\n";
        return 1;
    } catch (const ofs::SemanticError& e) {
        std::cerr << file << ":" << e.line << ":" << e.col
                  << ": type error: " << e.what() << "\n";
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "ofs: error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
```

---

## PHASE 8 — BUILD SYSTEM

### File: `CMakeLists.txt`

```cmake
cmake_minimum_required(VERSION 3.20)
project(ofs CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Find LLVM ─────────────────────────────────────────────────────────────
# Install LLVM first:
#   Ubuntu:  sudo apt install llvm-17-dev
#   macOS:   brew install llvm@17
#   Windows: choco install llvm  or download from llvm.org

find_package(LLVM REQUIRED CONFIG)
message(STATUS "LLVM version: ${LLVM_PACKAGE_VERSION}")

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

# ── LLVM components needed ─────────────────────────────────────────────────
llvm_map_components_to_libnames(LLVM_LIBS
    Core
    Support
    IRReader
    Target
    MC
    X86CodeGen
    X86AsmParser
    X86Desc
    X86Info
    AArch64CodeGen
    AArch64AsmParser
    AArch64Desc
    AArch64Info
    passes
    Analysis
    TransformUtils
    ScalarOpts
    InstCombine
    native
)

# ── OFS compiler sources ───────────────────────────────────────────────────
add_executable(ofs
    src/main.cpp
    src/lexer/lexer.cpp
    src/parser/parser.cpp
    src/semantic/semantic.cpp
    src/semantic/type_system.cpp
    src/codegen/codegen.cpp
)

target_include_directories(ofs PRIVATE src)
target_link_libraries(ofs PRIVATE ${LLVM_LIBS})

# ── Runtime (compiled as C, linked into every OFS program) ────────────────
add_library(ofs_runtime STATIC src/runtime/ofs_runtime.c)

# ── Tests ─────────────────────────────────────────────────────────────────
enable_testing()

add_executable(test_lexer    tests/test_lexer.cpp    src/lexer/lexer.cpp)
add_executable(test_parser   tests/test_parser.cpp   src/lexer/lexer.cpp src/parser/parser.cpp)
add_executable(test_semantic tests/test_semantic.cpp src/lexer/lexer.cpp src/parser/parser.cpp
                                                      src/semantic/semantic.cpp)

foreach(t test_lexer test_parser test_semantic)
    target_include_directories(${t} PRIVATE src)
    add_test(NAME ${t} COMMAND ${t})
endforeach()
```

---

## PHASE 9 — EXAMPLE PROGRAMS

### `examples/hello.ofs`
```
core main() {
    echo("Hello, World!")
}
```

### `examples/fizzbuzz.ofs`
```
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

### `examples/structs.ofs`
```
monolith Player {
    name: obsidian
    hp:   stone
    speed: crystal
}

vein greet(p: Player) -> void {
    echo(p.name)
    echo(p.hp)
}

core main() {
    forge hero: Player
    hero.name = "Obsidian Knight"
    hero.hp   = 100
    hero.speed = 1.5
    greet(hero)
}
```

### `examples/pointers.ofs`
```
core main() {
    forge x: stone = 10
    echo(x)

    fracture {
        shard p: *stone = &x
        *p = 42
    }

    echo(x)  // prints 42
}
```

### `examples/collections.ofs`
```
core main() {
    forge nums = [10, 20, 30, 40, 50]

    cycle (i in nums) {
        echo(i)
    }

    forge sum: stone = 0
    cycle (forge i = 0; i < 5; i++) {
        sum += nums[i]
    }
    echo(sum)
}
```

---

## PHASE 10 — TESTS

### `tests/test_lexer.cpp`

```cpp
#include "lexer/lexer.hpp"
#include <cassert>
#include <iostream>

using namespace ofs;

void test(const std::string& name, bool cond) {
    if (!cond) { std::cerr << "FAILED: " << name << "\n"; assert(false); }
    std::cout << "OK: " << name << "\n";
}

int main() {
    auto lex = [](const std::string& s) {
        return Lexer(s).tokenize();
    };

    // Keywords
    auto t = lex("forge x core vein monolith");
    test("forge keyword",    t[0].kind == TokenKind::KW_FORGE);
    test("x identifier",     t[1].kind == TokenKind::IDENT && t[1].lexeme == "x");
    test("core keyword",     t[2].kind == TokenKind::KW_CORE);
    test("vein keyword",     t[3].kind == TokenKind::KW_VEIN);
    test("monolith keyword", t[4].kind == TokenKind::KW_MONOLITH);

    // Types
    t = lex("stone crystal obsidian shard");
    test("stone type",    t[0].kind == TokenKind::KW_STONE);
    test("crystal type",  t[1].kind == TokenKind::KW_CRYSTAL);
    test("obsidian type", t[2].kind == TokenKind::KW_OBSIDIAN);
    test("shard type",    t[3].kind == TokenKind::KW_SHARD);

    // Literals
    t = lex("42 3.14 \"hello\" true false");
    test("int literal",    t[0].kind == TokenKind::INT_LIT && t[0].int_val == 42);
    test("float literal",  t[1].kind == TokenKind::FLOAT_LIT);
    test("string literal", t[2].kind == TokenKind::STRING_LIT && t[2].lexeme == "hello");
    test("true literal",   t[3].kind == TokenKind::KW_TRUE);
    test("false literal",  t[4].kind == TokenKind::KW_FALSE);

    // Operators
    t = lex("+ - * / % == != < <= > >= && || -> ++ --");
    test("plus",     t[0].kind == TokenKind::PLUS);
    test("eq_eq",    t[5].kind == TokenKind::EQ_EQ);
    test("arrow",    t[12].kind == TokenKind::ARROW);
    test("plus_plus",t[13].kind == TokenKind::PLUS_PLUS);

    // Comments are skipped
    t = lex("42 // this is a comment\n 99");
    test("comment skip",   t[0].int_val == 42);
    test("after comment",  t[1].int_val == 99);

    std::cout << "\nAll lexer tests passed!\n";
    return 0;
}
```

---

## IMPLEMENTATION ORDER

Implement phases in this exact order — run tests after each phase:

**Phase 1 — Lexer** (2–3 days)
1. `token.hpp` — define all TokenKind values and KEYWORDS map
2. `lexer.cpp` — implement tokenize(), lex_string(), lex_number(), lex_ident()
3. `tests/test_lexer.cpp` — all tokens recognized correctly

**Phase 2 — AST** (1 day)
1. `ast.hpp` — all node types (no logic, just data structures)
2. `ast_printer.hpp` — pretty-print for debugging

**Phase 3 — Parser** (3–4 days)
1. `parser.cpp` — implement Pratt parser for expressions, then statements, then declarations
2. `tests/test_parser.cpp` — parse all example programs and verify AST shape

**Phase 4 — Semantic** (2–3 days)
1. `type_system.hpp` — Scope class with push/pop/define/lookup
2. `semantic.cpp` — walk AST, fill resolved_type on every Expr node
3. `tests/test_semantic.cpp` — type errors caught, valid programs accepted

**Phase 5 — Runtime** (1 day)
1. `ofs_runtime.c` — echo functions, alloc, array, string concat
2. Compile as static library: `gcc -c ofs_runtime.c -o ofs_runtime.o`

**Phase 6 — LLVM Codegen** (4–5 days)
1. Setup: LLVMContext, Module, IRBuilder in `codegen.cpp`
2. Implement `declare_runtime()` — declare all runtime functions
3. Implement literal expressions (int, float, string, bool)
4. Implement binary expressions (add, sub, icmp, fcmp)
5. Implement variables (alloca/load/store)
6. Implement if/else (basic blocks + br)
7. Implement cycle loops (cond/body/step/exit basic blocks)
8. Implement function calls
9. Implement monolith (struct) + field access (GEP)
10. Implement fracture/abyss (pointer ops)
11. Implement echo() dispatch (call correct runtime fn based on type)

**Phase 7 — CLI** (1 day)
1. `main.cpp` — wire all phases together
2. Test: `ofs build examples/hello.ofs && ./a.out`

---

## INSTALL LLVM

```bash
# Ubuntu / Debian
sudo apt install llvm-17-dev clang-17 lld-17

# macOS
brew install llvm@17
echo 'export PATH="/opt/homebrew/opt/llvm@17/bin:$PATH"' >> ~/.zshrc

# Windows (MSVC)
winget install LLVM.LLVM

# Verify
llvm-config --version
```

## BUILD COMMANDS

```bash
# Configure
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build

# Run tests
cd build && ctest --output-on-failure

# Compile a .ofs file
./build/ofs build examples/hello.ofs -o hello
./hello

# Debug: see LLVM IR
./build/ofs ir examples/hello.ofs -o hello
cat hello.ll

# Debug: see tokens
./build/ofs tokens examples/fizzbuzz.ofs

# Debug: see AST
./build/ofs ast examples/fizzbuzz.ofs
```

---

## ROADMAP

| Version | Features                                                          | Status |
|---------|-------------------------------------------------------------------|--------|
| v0.1    | Lexer, Parser, AST                                                | ✅ Done |
| v0.2    | Semantic analysis + type inference + CLI runner (like Python)      | ✅ Done |
| v0.3    | LLVM codegen: literals, arithmetic, variables, if, cycle, echo   | ✅ Done |
| v0.4    | Functions (vein), return, recursion                               | ✅ Done |
| v0.5    | monolith (structs) + field access                                 | ✅ Done |
| v0.6    | fracture (safe pointers) + shard type                             | ✅ Done |
| v0.7    | Dynamic arrays + range-based cycle                                | ✅ Done |
| v0.8    | abyss (unsafe memory), raw mem access                             | ✅ Done |
| v0.9    | String operations (concat, compare)                               | ✅ Done |
| v1.0    | Standard library, import, extern, while, type casts, expanded runtime | ✅ Done |

### What's New in v1.0

- **Import system**: `import "path/to/file.ofs"` to include other OFS modules
- **Extern functions**: `extern vein name(params) -> type` for C FFI declarations
- **While loops**: `while (cond) { }` as syntactic sugar over cycle
- **Type casting**: `expr as type` for explicit type conversions (stone ↔ crystal ↔ bool)
- **Expanded runtime**: 17 new runtime functions (string ops, type conversions, math, I/O)
- **Standard library**: 4 modules — core.ofs, math.ofs, string.ofs, io.ofs
- **New examples**: while_loop, type_cast, extern_func, recursion, string_ops, math_ops, monolith_advanced

### What's New in v0.2

- **Direct runner**: `ofs file.ofs` runs scripts directly (like Python)
- **Shebang support**: `#!/usr/bin/env ofs` at the top of `.ofs` files
- **Improved CLI**: `ofs help`, `ofs --version`, `-h`/`-v` flags
- **Language documentation**: Complete reference and getting started guide
- **More examples**: factorial, calculator, variables
