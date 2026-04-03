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
    std::unique_ptr<ImportDecl>     parse_import();
    std::unique_ptr<ExternFuncDecl> parse_extern();
    std::unique_ptr<StrataDecl>     parse_strata();

    // ── Statements ────────────────────────────────────────────────────────
    StmtPtr parse_stmt();
    StmtPtr parse_block();           // { stmts... }
    StmtPtr parse_forge_stmt();      // forge x = ...
    StmtPtr parse_shard_stmt();      // shard p: *stone = &x
    StmtPtr parse_const_stmt();      // const x = ...
    StmtPtr parse_if_stmt();         // if (cond) { } else { }
    StmtPtr parse_cycle_stmt();      // cycle (...) { }
    StmtPtr parse_while_stmt();      // while (cond) { }
    StmtPtr parse_return_stmt();     // return expr
    StmtPtr parse_tectonic_stmt();   // tectonic fracture|abyss|fractal { }
    StmtPtr parse_fracture_stmt();   // fracture { }
    StmtPtr parse_abyss_stmt();      // abyss { }
    StmtPtr parse_fractal_stmt();    // fractal { }
    StmtPtr parse_match_stmt();      // match expr { case val: ... default: ... }
    StmtPtr parse_tremor_stmt();     // tremor { } catch (e: type) { }
    StmtPtr parse_throw_stmt();      // throw expr
    StmtPtr parse_obsid_block();     // : stmts... obsid
    StmtPtr parse_stmt_block();      // { ... } or : ... obsid

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
