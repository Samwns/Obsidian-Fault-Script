#pragma once
#include <string>
#include <unordered_map>
#include <cstdint>

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
    KW_IMPL,        // impl     (methods for monolith)
    KW_NAMESPACE,   // namespace (symbol grouping)
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
    KW_IMPORT,      // attach / import
    KW_WHILE,       // while   (condition-only loop)
    KW_EXTERN,      // extern  (foreign function)
    KW_RIFT,        // rift    (native interop declaration)
    KW_AS,          // as      (type cast)
    KW_MATCH,       // match   (pattern matching)
    KW_CASE,        // case
    KW_DEFAULT,     // default
    KW_CONST,       // const   (immutable variable)
    KW_STRATA,      // strata  (enum)
    KW_TREMOR,      // tremor  (try/error handling)
    KW_CATCH,       // catch
    KW_THROW,       // throw
    KW_INTENT,      // intent
    KW_TECTONIC,    // tectonic (mode directive prefix)
    KW_BEDROCK,     // bedrock (native low-level block)
    KW_PURE,        // pure
    KW_IMPURE,      // impure
    KW_FRACTAL,     // fractal
    KW_OBSID,       // obsid (block terminator)
    KW_ASM,          // asm      (inline assembly escape hatch)

    // ── Type Keywords ─────────────────────────────────────────────────────
    KW_STONE,       // stone    -> i64
    KW_U8,          // u8       -> i8 (unsigned)
    KW_U16,         // u16      -> i16 (unsigned)
    KW_U32,         // u32      -> i32 (unsigned)
    KW_U64,         // u64      -> i64 (unsigned)
    KW_I8,          // i8       -> i8 (signed)
    KW_I32,         // i32      -> i32 (signed)
    KW_CRYSTAL,     // crystal  -> f64
    KW_OBSIDIAN,    // obsidian -> i8* (string)
    KW_BOOL,        // bool     -> i1
    KW_VOID,        // void
    KW_FLOW,        // flow     -> crystal (alias)
    KW_TRUTH,       // truth    -> bool (alias)
    KW_GLYPH,       // glyph    -> obsidian (alias)
    KW_PTR,         // ptr      -> shard<stone> (alias)

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
    PERCENT_EQ,     // %=
    PLUS_PLUS,      // ++
    MINUS_MINUS,    // --
    LSHIFT,         // <<
    RSHIFT,         // >>
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
    ELLIPSIS,       // ... (variadic parameter marker in extern)

    // ── Special ───────────────────────────────────────────────────────────
    END_OF_FILE,
    TOK_ERROR
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
    {"impl",     TokenKind::KW_IMPL},
    {"namespace",TokenKind::KW_NAMESPACE},
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
    {"attach",   TokenKind::KW_IMPORT},
    {"import",   TokenKind::KW_IMPORT},
    {"while",    TokenKind::KW_WHILE},
    {"extern",   TokenKind::KW_EXTERN},
    {"rift",     TokenKind::KW_RIFT},
    {"as",       TokenKind::KW_AS},
    {"match",    TokenKind::KW_MATCH},
    {"case",     TokenKind::KW_CASE},
    {"default",  TokenKind::KW_DEFAULT},
    {"const",    TokenKind::KW_CONST},
    {"strata",   TokenKind::KW_STRATA},
    {"tremor",   TokenKind::KW_TREMOR},
    {"catch",    TokenKind::KW_CATCH},
    {"throw",    TokenKind::KW_THROW},
    {"intent",   TokenKind::KW_INTENT},
    {"tectonic", TokenKind::KW_TECTONIC},
    {"bedrock",  TokenKind::KW_BEDROCK},
    {"pure",     TokenKind::KW_PURE},
    {"impure",   TokenKind::KW_IMPURE},
    {"fractal",  TokenKind::KW_FRACTAL},
    {"obsid",    TokenKind::KW_OBSID},
    {"stone",    TokenKind::KW_STONE},
    {"u8",       TokenKind::KW_U8},
    {"u16",      TokenKind::KW_U16},
    {"u32",      TokenKind::KW_U32},
    {"u64",      TokenKind::KW_U64},
    {"i8",       TokenKind::KW_I8},
    {"i32",      TokenKind::KW_I32},
    {"crystal",  TokenKind::KW_CRYSTAL},
    {"obsidian", TokenKind::KW_OBSIDIAN},
    {"bool",     TokenKind::KW_BOOL},
    {"void",     TokenKind::KW_VOID},
    {"flow",     TokenKind::KW_FLOW},
    {"truth",    TokenKind::KW_TRUTH},
    {"glyph",    TokenKind::KW_GLYPH},
    {"ptr",      TokenKind::KW_PTR},
    {"asm",      TokenKind::KW_ASM},
};

} // namespace ofs
