#pragma once
#include <string>
#include <vector>
#include <memory>
#include <optional>
#include <cstdint>

namespace ofs {

// Forward declarations
struct Expr;
struct Stmt;
struct Decl;
struct Param;
using ExprPtr = std::unique_ptr<Expr>;
using StmtPtr = std::unique_ptr<Stmt>;
using DeclPtr = std::unique_ptr<Decl>;

// ── Type representation ───────────────────────────────────────────────────

enum class BaseType {
    Stone,      // i64
    U8,         // i8  (unsigned)
    U16,        // i16 (unsigned)
    U32,        // i32 (unsigned)
    U64,        // i64 (unsigned)
    I8,         // i8  (signed)
    I32,        // i32 (signed)
    Crystal,    // f64
    Obsidian,   // i8* (string)
    Bool,       // i1
    Void,       // void
    Shard,      // pointer (wraps another type)
    Array,      // dynamic array
    Function,   // function pointer/type
    Named,      // user-defined struct name
    Infer,      // type to be inferred
};

struct OFSType {
    BaseType              base = BaseType::Infer;
    std::string           name;           // for Named types
    std::shared_ptr<OFSType> inner;       // for Shard<T> and Array<T>
    std::vector<OFSType>  fn_params;      // for Function
    std::shared_ptr<OFSType> fn_return;   // for Function
    bool                  is_mut = true;   // mutability

    static OFSType stone()    { return {BaseType::Stone}; }
    static OFSType u8()       { return {BaseType::U8}; }
    static OFSType u16()      { return {BaseType::U16}; }
    static OFSType u32()      { return {BaseType::U32}; }
    static OFSType u64()      { return {BaseType::U64}; }
    static OFSType i8()       { return {BaseType::I8}; }
    static OFSType i32()      { return {BaseType::I32}; }
    static OFSType crystal()  { return {BaseType::Crystal}; }
    static OFSType obsidian() { return {BaseType::Obsidian}; }
    static OFSType boolean()  { return {BaseType::Bool}; }
    static OFSType void_t()   { return {BaseType::Void}; }
    static OFSType infer()    { return {BaseType::Infer}; }
    static OFSType shard_of(OFSType inner_type) {
        OFSType t;
        t.base = BaseType::Shard;
        t.inner = std::make_shared<OFSType>(std::move(inner_type));
        return t;
    }
    static OFSType array_of(OFSType inner_type) {
        OFSType t;
        t.base = BaseType::Array;
        t.inner = std::make_shared<OFSType>(std::move(inner_type));
        return t;
    }
    static OFSType function_of(std::vector<OFSType> params, OFSType ret) {
        OFSType t;
        t.base = BaseType::Function;
        t.fn_params = std::move(params);
        t.fn_return = std::make_shared<OFSType>(std::move(ret));
        return t;
    }
    static OFSType named(const std::string& n) {
        OFSType t;
        t.base = BaseType::Named;
        t.name = n;
        return t;
    }

    std::string to_string() const;
    bool operator==(const OFSType& o) const;
    bool operator!=(const OFSType& o) const { return !(*this == o); }
    bool is_numeric() const;
    bool is_integral() const;
    bool is_unsigned_integer() const;
    int  integer_bits() const;
    bool is_pointer() const;
};

enum class BlockStyle {
    Brace,
    Obsid,
};

enum class FuncIntent {
    Pure,
    Impure,
    Fractal,
};

struct Param {
    std::string name;
    OFSType     type;
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
    std::string op;     // "=", "+=", "-=", "*=", "/=", "%="
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

struct LambdaExpr : Expr {
    std::vector<Param> params;
    OFSType            return_type;
    StmtPtr            body;
};

// ── Inline Assembly Escape Hatch ─────────────────────────────
struct InlineAsmExpr : Expr {
    std::string asm_code;
    std::vector<std::string> outputs; // nomes das variáveis de saída
    std::vector<ExprPtr> inputs;      // expressões de entrada
    std::string clobbers;             // registradores/clobbers
    bool volatile_ = false;
};

// ── Statements ────────────────────────────────────────────────────────────

struct Stmt {
    int line = 0, col = 0;
    virtual ~Stmt() = default;
};

struct ExprStmt     : Stmt { ExprPtr expr; };
struct BlockStmt    : Stmt {
    BlockStyle style = BlockStyle::Brace;
    std::vector<StmtPtr> stmts;
};
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

// while (cond) { body } — condition-only loop (syntactic sugar over cycle)
struct WhileCycleStmt : Stmt {
    ExprPtr cond;
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

struct FractalStmt : Stmt {
    // fractal { ... } — unrestricted effect block
    StmtPtr body;
};

struct BedrockStmt : Stmt {
    // bedrock { ... } — OFS-native typed low-level block
    StmtPtr body;
};

// const x: stone = 42  — immutable variable
struct ConstStmt : Stmt {
    std::string            name;
    std::optional<OFSType> type_ann;
    ExprPtr                initializer;
};

// throw expr
struct ThrowStmt : Stmt {
    ExprPtr value;
};

// match expr { case val: body ... default: body }
struct MatchArm {
    ExprPtr  pattern;   // nullable for default arm
    bool     is_default = false;
    StmtPtr  body;
};

struct MatchStmt : Stmt {
    ExprPtr               subject;
    std::vector<MatchArm> arms;
};

// tremor { ... } catch (e: obsidian) { ... }
struct TremorStmt : Stmt {
    StmtPtr     body;
    std::string catch_var;
    OFSType     catch_type;
    StmtPtr     catch_body;
};

// ── Declarations ──────────────────────────────────────────────────────────

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
    FuncIntent         intent = FuncIntent::Impure;
    bool               is_core = false; // core main()
};

// monolith Player { name: obsidian, hp: stone }
struct MonolithDecl : Decl {
    std::string              name;
    std::vector<Param>       fields;  // Param reused: {name, type}
    std::vector<std::unique_ptr<FuncDecl>> methods; // optional methods
    std::string              layout = "native";
};

// Top-level forge (global variable)
struct GlobalForgeDecl : Decl {
    std::string         name;
    std::optional<OFSType> type_ann;
    ExprPtr             initializer;
};

// attach "path/to/file.ofs"
struct ImportDecl : Decl {
    std::string path;  // the import path string
};

// extern vein printf(fmt: obsidian) -> stone
struct ExternFuncDecl : Decl {
    std::string        name;
    std::vector<Param> params;
    OFSType            return_type;
    bool               is_variadic = false;
    std::string        abi = "c";
    std::string        link_name;
    bool               is_rift = false;
};

struct ImplDecl : Decl {
    std::string target_name;
    std::vector<std::unique_ptr<FuncDecl>> methods;
};

struct NamespaceDecl : Decl {
    std::string name;
    std::vector<DeclPtr> declarations;
};

// strata Color { Red, Green, Blue }
struct StrataDecl : Decl {
    std::string              name;
    std::vector<std::string> variants;
};

// ── Translation unit ──────────────────────────────────────────────────────

struct Module {
    std::string         filename;
    std::vector<DeclPtr> decls;
};

// ── OFSType implementations ──────────────────────────────────────────────

inline std::string OFSType::to_string() const {
    switch (base) {
        case BaseType::Stone:    return "stone";
        case BaseType::U8:       return "u8";
        case BaseType::U16:      return "u16";
        case BaseType::U32:      return "u32";
        case BaseType::U64:      return "u64";
        case BaseType::I8:       return "i8";
        case BaseType::I32:      return "i32";
        case BaseType::Crystal:  return "crystal";
        case BaseType::Obsidian: return "obsidian";
        case BaseType::Bool:     return "bool";
        case BaseType::Void:     return "void";
        case BaseType::Shard:    return "*" + (inner ? inner->to_string() : "?");
        case BaseType::Array:    return "[" + (inner ? inner->to_string() : "?") + "]";
        case BaseType::Function: {
            std::string out = "vein(";
            for (size_t i = 0; i < fn_params.size(); i++) {
                if (i > 0) out += ", ";
                out += fn_params[i].to_string();
            }
            out += ") -> ";
            out += fn_return ? fn_return->to_string() : "void";
            return out;
        }
        case BaseType::Named:    return name;
        case BaseType::Infer:    return "infer";
    }
    return "unknown";
}

inline bool OFSType::operator==(const OFSType& o) const {
    if (base != o.base) return false;
    if (base == BaseType::Named) return name == o.name;
    if (base == BaseType::Function) {
        if (fn_params.size() != o.fn_params.size()) return false;
        for (size_t i = 0; i < fn_params.size(); i++) {
            if (fn_params[i] != o.fn_params[i]) return false;
        }
        if (!fn_return && !o.fn_return) return true;
        if (!fn_return || !o.fn_return) return false;
        return *fn_return == *o.fn_return;
    }
    if (base == BaseType::Shard || base == BaseType::Array) {
        if (!inner && !o.inner) return true;
        if (!inner || !o.inner) return false;
        return *inner == *o.inner;
    }
    return true;
}

inline bool OFSType::is_numeric() const {
    return base == BaseType::Stone ||
           base == BaseType::U8 || base == BaseType::U16 ||
           base == BaseType::U32 || base == BaseType::U64 ||
           base == BaseType::I8 || base == BaseType::I32 ||
           base == BaseType::Crystal;
}

inline bool OFSType::is_integral() const {
    return base == BaseType::Stone ||
           base == BaseType::U8 || base == BaseType::U16 ||
           base == BaseType::U32 || base == BaseType::U64 ||
           base == BaseType::I8 || base == BaseType::I32;
}

inline bool OFSType::is_unsigned_integer() const {
    return base == BaseType::U8 || base == BaseType::U16 ||
           base == BaseType::U32 || base == BaseType::U64;
}

inline int OFSType::integer_bits() const {
    switch (base) {
        case BaseType::U8:
        case BaseType::I8:
            return 8;
        case BaseType::U16:
            return 16;
        case BaseType::U32:
        case BaseType::I32:
            return 32;
        case BaseType::Stone:
        case BaseType::U64:
            return 64;
        default:
            return 0;
    }
}

inline bool OFSType::is_pointer() const {
    return base == BaseType::Shard;
}

} // namespace ofs
