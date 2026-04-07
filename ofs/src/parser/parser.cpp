#include "parser.hpp"
#include <algorithm>
#include <cassert>
#include "../i18n.hpp"

namespace ofs {

Parser::Parser(std::vector<Token> tokens)
    : tokens_(std::move(tokens)) {}

// ── Helpers ───────────────────────────────────────────────────────────────

Token& Parser::peek(int offset) {
    size_t idx = pos_ + offset;
    if (idx >= tokens_.size()) return tokens_.back(); // EOF
    return tokens_[idx];
}

Token& Parser::advance() {
    Token& t = tokens_[pos_];
    if (!is_at_end()) pos_++;
    return t;
}

bool Parser::check(TokenKind kind) const {
    if (pos_ >= tokens_.size()) return false;
    return tokens_[pos_].kind == kind;
}

bool Parser::match(TokenKind kind) {
    if (check(kind)) { advance(); return true; }
    return false;
}

bool Parser::match_any(std::initializer_list<TokenKind> kinds) {
    for (auto k : kinds) {
        if (check(k)) { advance(); return true; }
    }
    return false;
}

Token Parser::expect(TokenKind kind, const std::string& msg) {
    if (check(kind)) return advance();
    throw error(msg + OFS_MSG(" (got '", " (encontrado '") + peek().lexeme + "')");
}

void Parser::skip_newlines() {
    while (check(TokenKind::NEWLINE) || check(TokenKind::SEMICOLON)) advance();
}

bool Parser::is_at_end() const {
    return pos_ >= tokens_.size() || tokens_[pos_].kind == TokenKind::END_OF_FILE;
}

ParseError Parser::error(const std::string& msg) {
    auto& t = peek();
    return ParseError(msg, t.line, t.col);
}

// ── Operator helpers ──────────────────────────────────────────────────────

int Parser::precedence(TokenKind op) const {
    switch (op) {
        case TokenKind::PIPE_PIPE:                                  return 1;
        case TokenKind::AMP_AMP:                                    return 2;
        case TokenKind::EQ_EQ: case TokenKind::BANG_EQ:             return 3;
        case TokenKind::LT: case TokenKind::LT_EQ:
        case TokenKind::GT: case TokenKind::GT_EQ:                  return 4;
        case TokenKind::PIPE:                                       return 5;
        case TokenKind::CARET:                                      return 6;
        case TokenKind::AMP:                                        return 7;
        case TokenKind::LSHIFT: case TokenKind::RSHIFT:             return 8;
        case TokenKind::PLUS: case TokenKind::MINUS:                return 9;
        case TokenKind::STAR: case TokenKind::SLASH:
        case TokenKind::PERCENT:                                    return 10;
        default:                                                    return 0;
    }
}

bool Parser::is_binary_op(TokenKind kind) const {
    return precedence(kind) > 0;
}

bool Parser::is_right_assoc(TokenKind op) const {
    return false; // all binary ops are left-assoc
}

std::string Parser::token_to_op(TokenKind kind) const {
    switch (kind) {
        case TokenKind::PLUS:      return "+";
        case TokenKind::MINUS:     return "-";
        case TokenKind::STAR:      return "*";
        case TokenKind::SLASH:     return "/";
        case TokenKind::PERCENT:   return "%";
        case TokenKind::EQ_EQ:     return "==";
        case TokenKind::BANG_EQ:   return "!=";
        case TokenKind::LT:        return "<";
        case TokenKind::LT_EQ:     return "<=";
        case TokenKind::GT:        return ">";
        case TokenKind::GT_EQ:     return ">=";
        case TokenKind::AMP_AMP:   return "&&";
        case TokenKind::PIPE_PIPE: return "||";
        case TokenKind::AMP:       return "&";
        case TokenKind::PIPE:      return "|";
        case TokenKind::CARET:     return "^";
        case TokenKind::LSHIFT:    return "<<";
        case TokenKind::RSHIFT:    return ">>";
        default:                   return "?";
    }
}

// ── Module ────────────────────────────────────────────────────────────────

Module Parser::parse() {
    Module mod;
    skip_newlines();
    while (!is_at_end()) {
        mod.decls.push_back(parse_decl());
        skip_newlines();
    }
    return mod;
}

// ── Declarations ──────────────────────────────────────────────────────────

DeclPtr Parser::parse_decl() {
    skip_newlines();
    if (check(TokenKind::KW_IMPORT))   return parse_import();
    if (check(TokenKind::KW_EXTERN) || check(TokenKind::KW_RIFT)) return parse_extern();
    if (check(TokenKind::KW_CORE))     return parse_func(true);
    if (check(TokenKind::KW_VEIN))     return parse_func(false);
    if (check(TokenKind::KW_MONOLITH)) return parse_monolith();
    if (check(TokenKind::KW_STRATA))   return parse_strata();
    if (check(TokenKind::KW_FORGE))    return parse_global_forge();
    throw error(OFS_MSG("expected declaration (core, vein, monolith, strata, forge, or attach)",
                         "esperada declaração (core, vein, monolith, strata, forge ou attach)"));
}

std::unique_ptr<FuncDecl> Parser::parse_func(bool is_core) {
    auto fn = std::make_unique<FuncDecl>();
    fn->line = peek().line;
    fn->col = peek().col;
    fn->is_core = is_core;

    advance(); // consume 'core' or 'vein'
    fn->name = expect(TokenKind::IDENT, "expected function name").lexeme;

    expect(TokenKind::LPAREN, "expected '(' after function name");

    // Parse params
    if (!check(TokenKind::RPAREN)) {
        do {
            skip_newlines();
            Param p;
            p.name = expect(TokenKind::IDENT, "expected parameter name").lexeme;
            expect(TokenKind::COLON, "expected ':' after parameter name");
            p.type = parse_type();
            fn->params.push_back(std::move(p));
        } while (match(TokenKind::COMMA));
    }
    skip_newlines();
    expect(TokenKind::RPAREN, "expected ')' after parameters");

    // Return type
    if (match(TokenKind::ARROW)) {
        fn->return_type = parse_type();
    } else {
        fn->return_type = OFSType::void_t();
    }

    // Optional effect intent. Default: impure.
    if (match(TokenKind::KW_INTENT)) {
        if (match(TokenKind::KW_PURE)) {
            fn->intent = FuncIntent::Pure;
        } else if (match(TokenKind::KW_IMPURE)) {
            fn->intent = FuncIntent::Impure;
        } else if (match(TokenKind::KW_FRACTAL)) {
            fn->intent = FuncIntent::Fractal;
        } else {
            throw error("expected intent type: pure, impure, or fractal");
        }
    } else {
        fn->intent = FuncIntent::Impure;
    }

    skip_newlines();
    fn->body = parse_block();

    return fn;
}

std::unique_ptr<ImportDecl> Parser::parse_import() {
    auto imp = std::make_unique<ImportDecl>();
    imp->line = peek().line;
    imp->col = peek().col;
    advance(); // consume 'attach' or legacy 'import'
    if (check(TokenKind::STRING_LIT)) {
        imp->path = advance().lexeme;
    } else if (check(TokenKind::IDENT)) {
        // Built-in module shorthand: attach serve
        imp->path = advance().lexeme;
    } else {
        throw error("expected string path or module name after 'attach'");
    }
    return imp;
}

std::unique_ptr<ExternFuncDecl> Parser::parse_extern() {
    auto ext = std::make_unique<ExternFuncDecl>();
    ext->line = peek().line;
    ext->col = peek().col;
    bool is_rift = check(TokenKind::KW_RIFT);
    ext->is_rift = is_rift;
    advance(); // consume 'extern' or 'rift'
    expect(TokenKind::KW_VEIN, is_rift ? "expected 'vein' after 'rift'" : "expected 'vein' after 'extern'");
    ext->name = expect(TokenKind::IDENT, "expected function name").lexeme;
    expect(TokenKind::LPAREN, "expected '(' after extern function name");

    if (!check(TokenKind::RPAREN)) {
        do {
            skip_newlines();
            Param p;
            p.name = expect(TokenKind::IDENT, "expected parameter name").lexeme;
            expect(TokenKind::COLON, "expected ':' after parameter name");
            p.type = parse_type();
            ext->params.push_back(std::move(p));
        } while (match(TokenKind::COMMA));
    }
    skip_newlines();
    expect(TokenKind::RPAREN, "expected ')' after extern parameters");

    // Return type
    if (match(TokenKind::ARROW)) {
        ext->return_type = parse_type();
    } else {
        ext->return_type = OFSType::void_t();
    }

    while (check(TokenKind::IDENT)) {
        std::string kw = peek().lexeme;
        if (kw == "bind") {
            advance();
            if (check(TokenKind::STRING_LIT)) {
                ext->link_name = advance().lexeme;
            } else {
                throw error("expected string literal after 'bind'");
            }
            continue;
        }
        if (kw == "abi") {
            advance();
            if (check(TokenKind::IDENT)) {
                ext->abi = advance().lexeme;
            } else {
                throw error("expected ABI name after 'abi'");
            }
            continue;
        }
        break;
    }

    return ext;
}

std::unique_ptr<MonolithDecl> Parser::parse_monolith() {
    auto m = std::make_unique<MonolithDecl>();
    m->line = peek().line;
    m->col = peek().col;

    advance(); // consume 'monolith'
    m->name = expect(TokenKind::IDENT, "expected monolith name").lexeme;

    while (check(TokenKind::IDENT)) {
        std::string kw = peek().lexeme;
        if (kw == "layout") {
            advance();
            if (check(TokenKind::IDENT)) {
                m->layout = advance().lexeme;
            } else {
                throw error("expected layout mode after 'layout'");
            }
            continue;
        }
        break;
    }

    skip_newlines();
    expect(TokenKind::LBRACE, "expected '{' after monolith name");
    skip_newlines();

    while (!check(TokenKind::RBRACE) && !is_at_end()) {
        // Check for method (vein keyword)
        if (check(TokenKind::KW_VEIN)) {
            m->methods.push_back(parse_func(false));
        } else {
            Param field;
            field.name = expect(TokenKind::IDENT, "expected field name").lexeme;
            expect(TokenKind::COLON, "expected ':' after field name");
            field.type = parse_type();
            m->fields.push_back(std::move(field));
        }
        skip_newlines();
        // Optional comma separator
        match(TokenKind::COMMA);
        skip_newlines();
    }

    expect(TokenKind::RBRACE, "expected '}' after monolith body");
    return m;
}

std::unique_ptr<GlobalForgeDecl> Parser::parse_global_forge() {
    auto g = std::make_unique<GlobalForgeDecl>();
    g->line = peek().line;
    g->col = peek().col;

    advance(); // consume 'forge'
    g->name = expect(TokenKind::IDENT, "expected variable name").lexeme;

    if (match(TokenKind::COLON)) {
        g->type_ann = parse_type();
    }

    if (match(TokenKind::EQ)) {
        g->initializer = parse_expr();
    }

    return g;
}

// ── Statements ────────────────────────────────────────────────────────────

StmtPtr Parser::parse_stmt() {
    skip_newlines();

    if (check(TokenKind::LBRACE))       return parse_block();
    if (check(TokenKind::KW_FORGE))     return parse_forge_stmt();
    if (check(TokenKind::KW_SHARD))     return parse_shard_stmt();
    if (check(TokenKind::KW_CONST))     return parse_const_stmt();
    if (check(TokenKind::KW_IF))        return parse_if_stmt();
    if (check(TokenKind::KW_CYCLE))     return parse_cycle_stmt();
    if (check(TokenKind::KW_WHILE))     return parse_while_stmt();
    if (check(TokenKind::KW_RETURN))    return parse_return_stmt();
    if (check(TokenKind::KW_TECTONIC))  return parse_tectonic_stmt();
    if (check(TokenKind::KW_FRACTURE))  return parse_fracture_stmt();
    if (check(TokenKind::KW_ABYSS))     return parse_abyss_stmt();
    if (check(TokenKind::KW_FRACTAL))   return parse_fractal_stmt();
    if (check(TokenKind::KW_BEDROCK))   return parse_bedrock_stmt();
    if (check(TokenKind::KW_MATCH))     return parse_match_stmt();
    if (check(TokenKind::KW_TREMOR))    return parse_tremor_stmt();
    if (check(TokenKind::KW_THROW))     return parse_throw_stmt();

    if (check(TokenKind::KW_OBSID)) {
        throw error("unexpected 'obsid' without matching ':' block start");
    }

    if (check(TokenKind::KW_BREAK)) {
        auto s = std::make_unique<BreakStmt>();
        s->line = peek().line;
        s->col = peek().col;
        advance();
        return s;
    }
    if (check(TokenKind::KW_CONTINUE)) {
        auto s = std::make_unique<ContinueStmt>();
        s->line = peek().line;
        s->col = peek().col;
        advance();
        return s;
    }

    // Expression statement
    auto expr = parse_expr();
    auto s = std::make_unique<ExprStmt>();
    s->line = expr->line;
    s->col = expr->col;
    s->expr = std::move(expr);
    return s;
}

StmtPtr Parser::parse_block() {
    auto block = std::make_unique<BlockStmt>();
    block->line = peek().line;
    block->col = peek().col;
    block->style = BlockStyle::Brace;

    expect(TokenKind::LBRACE, "expected '{'");
    skip_newlines();

    while (!check(TokenKind::RBRACE) && !is_at_end()) {
        block->stmts.push_back(parse_stmt());
        skip_newlines();
        // Also skip semicolons between statements
        while (match(TokenKind::SEMICOLON)) skip_newlines();
    }

    expect(TokenKind::RBRACE, "expected '}'");
    return block;
}

StmtPtr Parser::parse_obsid_block() {
    auto block = std::make_unique<BlockStmt>();
    block->line = peek().line;
    block->col = peek().col;
    block->style = BlockStyle::Obsid;

    expect(TokenKind::COLON, "expected ':' to start obsid block");
    skip_newlines();

    while (!check(TokenKind::KW_OBSID) && !is_at_end()) {
        if (check(TokenKind::RBRACE)) {
            throw error("unexpected '}' inside obsid block; close with 'obsid'");
        }
        block->stmts.push_back(parse_stmt());
        skip_newlines();
        while (match(TokenKind::SEMICOLON)) skip_newlines();
    }

    expect(TokenKind::KW_OBSID, "expected 'obsid' to close block started by ':'");
    return block;
}

StmtPtr Parser::parse_stmt_block() {
    if (check(TokenKind::LBRACE)) return parse_block();
    if (check(TokenKind::COLON)) return parse_obsid_block();
    throw error("expected block body: '{...}' or ': ... obsid'");
}

StmtPtr Parser::parse_forge_stmt() {
    auto s = std::make_unique<ForgeStmt>();
    s->line = peek().line;
    s->col = peek().col;

    advance(); // consume 'forge'
    s->name = expect(TokenKind::IDENT, "expected variable name after 'forge'").lexeme;

    if (match(TokenKind::COLON)) {
        s->type_ann = parse_type();
    }

    if (match(TokenKind::EQ)) {
        s->initializer = parse_expr();
    }

    return s;
}

StmtPtr Parser::parse_shard_stmt() {
    auto s = std::make_unique<ForgeStmt>();
    s->line = peek().line;
    s->col = peek().col;

    advance(); // consume 'shard'
    s->name = expect(TokenKind::IDENT, "expected variable name after 'shard'").lexeme;

    expect(TokenKind::COLON, "expected ':' after shard variable name");
    s->type_ann = parse_type();

    if (match(TokenKind::EQ)) {
        s->initializer = parse_expr();
    }

    return s;
}

StmtPtr Parser::parse_if_stmt() {
    auto s = std::make_unique<IfStmt>();
    s->line = peek().line;
    s->col = peek().col;

    advance(); // consume 'if'
    if (match(TokenKind::LPAREN)) {
        s->cond = parse_expr();
        expect(TokenKind::RPAREN, "expected ')' after if condition");
    } else {
        s->cond = parse_expr();
    }
    skip_newlines();
    s->then_block = parse_stmt_block();
    skip_newlines();

    if (match(TokenKind::KW_ELSE)) {
        skip_newlines();
        if (check(TokenKind::KW_IF)) {
            // else if: wrap in a block for consistency
            s->else_block = parse_if_stmt();
        } else {
            s->else_block = parse_stmt_block();
        }
    }

    return s;
}

StmtPtr Parser::parse_cycle_stmt() {
    auto s = std::make_unique<CycleStmt>();
    s->line = peek().line;
    s->col = peek().col;

    advance(); // consume 'cycle'
    expect(TokenKind::LPAREN, "expected '(' after 'cycle'");

    // Determine if range-based: cycle (ident in expr)
    // Or C-style: cycle (forge i = 0; i < n; i++)
    // Peek ahead to check
    if (check(TokenKind::IDENT) && peek(1).kind == TokenKind::KW_IN) {
        // Range-based
        s->is_range = true;
        s->range_var = advance().lexeme;
        advance(); // consume 'in'
        s->range_expr = parse_expr();
        expect(TokenKind::RPAREN, "expected ')' after cycle range");
        skip_newlines();
        s->body = parse_stmt_block();
    } else {
        // C-style
        s->is_range = false;
        s->init = parse_forge_stmt();
        expect(TokenKind::SEMICOLON, "expected ';' after cycle init");
        s->cond = parse_expr();
        expect(TokenKind::SEMICOLON, "expected ';' after cycle condition");
        s->step = parse_expr();
        expect(TokenKind::RPAREN, "expected ')' after cycle step");
        skip_newlines();
        s->body = parse_stmt_block();
    }

    return s;
}

StmtPtr Parser::parse_while_stmt() {
    auto s = std::make_unique<WhileCycleStmt>();
    s->line = peek().line;
    s->col = peek().col;
    advance(); // consume 'while'
    if (match(TokenKind::LPAREN)) {
        s->cond = parse_expr();
        expect(TokenKind::RPAREN, "expected ')' after while condition");
    } else {
        s->cond = parse_expr();
    }
    skip_newlines();
    s->body = parse_stmt_block();
    return s;
}

StmtPtr Parser::parse_return_stmt() {
    auto s = std::make_unique<ReturnStmt>();
    s->line = peek().line;
    s->col = peek().col;

    advance(); // consume 'return'

    // Return value is optional (void return)
    if (!check(TokenKind::NEWLINE) && !check(TokenKind::RBRACE) &&
        !check(TokenKind::SEMICOLON) && !is_at_end()) {
        s->value = parse_expr();
    }

    return s;
}

StmtPtr Parser::parse_tectonic_stmt() {
    Token tk = advance(); // consume 'tectonic'
    skip_newlines();

    auto make_fracture = [&]() -> StmtPtr {
        auto s = std::make_unique<FractureStmt>();
        s->line = tk.line;
        s->col = tk.col;
        skip_newlines();
        s->body = parse_stmt_block();
        return s;
    };

    auto make_abyss = [&]() -> StmtPtr {
        auto s = std::make_unique<AbyssStmt>();
        s->line = tk.line;
        s->col = tk.col;
        skip_newlines();
        s->body = parse_stmt_block();
        return s;
    };

    auto make_fractal = [&]() -> StmtPtr {
        auto s = std::make_unique<FractalStmt>();
        s->line = tk.line;
        s->col = tk.col;
        skip_newlines();
        s->body = parse_stmt_block();
        return s;
    };

    if (match(TokenKind::KW_FRACTURE)) {
        return make_fracture();
    }

    if (match(TokenKind::KW_ABYSS)) {
        return make_abyss();
    }

    if (match(TokenKind::KW_FRACTAL)) {
        return make_fractal();
    }

    if (match(TokenKind::KW_BEDROCK)) {
        auto s = std::make_unique<BedrockStmt>();
        s->line = tk.line;
        s->col = tk.col;
        skip_newlines();
        s->body = parse_stmt_block();
        return s;
    }

    if (check(TokenKind::IDENT)) {
        std::string mode = peek().lexeme;
        if (mode == "safe") {
            advance();
            return make_fracture();
        }
        if (mode == "unsafe") {
            advance();
            return make_abyss();
        }
        if (mode == "bedrock") {
            advance();
            return make_fractal();
        }
    }

    throw error("expected tectonic mode: fracture|safe, abyss|unsafe, or fractal|bedrock");
}

StmtPtr Parser::parse_fracture_stmt() {
    auto s = std::make_unique<FractureStmt>();
    s->line = peek().line;
    s->col = peek().col;
    advance(); // consume 'fracture'
    skip_newlines();
    s->body = parse_block();
    return s;
}

StmtPtr Parser::parse_abyss_stmt() {
    auto s = std::make_unique<AbyssStmt>();
    s->line = peek().line;
    s->col = peek().col;
    advance(); // consume 'abyss'
    skip_newlines();
    s->body = parse_block();
    return s;
}

StmtPtr Parser::parse_fractal_stmt() {
    auto s = std::make_unique<FractalStmt>();
    s->line = peek().line;
    s->col = peek().col;
    advance(); // consume 'fractal'
    skip_newlines();
    s->body = parse_block();
    return s;
}

StmtPtr Parser::parse_bedrock_stmt() {
    auto s = std::make_unique<BedrockStmt>();
    s->line = peek().line;
    s->col = peek().col;
    advance(); // consume 'bedrock'
    skip_newlines();
    s->body = parse_block();
    return s;
}

StmtPtr Parser::parse_const_stmt() {
    auto s = std::make_unique<ConstStmt>();
    s->line = peek().line;
    s->col = peek().col;

    advance(); // consume 'const'
    s->name = expect(TokenKind::IDENT, "expected variable name after 'const'").lexeme;

    if (match(TokenKind::COLON)) {
        s->type_ann = parse_type();
    }

    expect(TokenKind::EQ, "expected '=' after const name — const must be initialized");
    s->initializer = parse_expr();

    return s;
}

StmtPtr Parser::parse_match_stmt() {
    auto s = std::make_unique<MatchStmt>();
    s->line = peek().line;
    s->col = peek().col;

    advance(); // consume 'match'
    s->subject = parse_expr();

    skip_newlines();
    expect(TokenKind::LBRACE, "expected '{' after match subject");
    skip_newlines();

    while (!check(TokenKind::RBRACE) && !is_at_end()) {
        MatchArm arm;
        arm.body = nullptr;

        if (check(TokenKind::KW_DEFAULT)) {
            advance(); // consume 'default'
            arm.is_default = true;
        } else {
            expect(TokenKind::KW_CASE, "expected 'case' or 'default' in match");
            arm.pattern = parse_expr();
        }

        expect(TokenKind::COLON, "expected ':' after case pattern");
        skip_newlines();
        arm.body = parse_block();
        s->arms.push_back(std::move(arm));
        skip_newlines();
    }

    expect(TokenKind::RBRACE, "expected '}' after match body");
    return s;
}

StmtPtr Parser::parse_tremor_stmt() {
    auto s = std::make_unique<TremorStmt>();
    s->line = peek().line;
    s->col = peek().col;

    advance(); // consume 'tremor'
    skip_newlines();
    s->body = parse_block();

    skip_newlines();
    expect(TokenKind::KW_CATCH, "expected 'catch' after tremor block");
    expect(TokenKind::LPAREN, "expected '(' after 'catch'");
    s->catch_var = expect(TokenKind::IDENT, "expected error variable name").lexeme;
    expect(TokenKind::COLON, "expected ':' after catch variable name");
    s->catch_type = parse_type();
    expect(TokenKind::RPAREN, "expected ')' after catch parameter");
    skip_newlines();
    s->catch_body = parse_block();

    return s;
}

StmtPtr Parser::parse_throw_stmt() {
    auto s = std::make_unique<ThrowStmt>();
    s->line = peek().line;
    s->col = peek().col;
    advance(); // consume 'throw'
    s->value = parse_expr();
    return s;
}

std::unique_ptr<StrataDecl> Parser::parse_strata() {
    auto s = std::make_unique<StrataDecl>();
    s->line = peek().line;
    s->col = peek().col;

    advance(); // consume 'strata'
    s->name = expect(TokenKind::IDENT, "expected enum name after 'strata'").lexeme;

    skip_newlines();
    expect(TokenKind::LBRACE, "expected '{' after strata name");
    skip_newlines();

    while (!check(TokenKind::RBRACE) && !is_at_end()) {
        s->variants.push_back(
            expect(TokenKind::IDENT, "expected variant name").lexeme);
        skip_newlines();
        match(TokenKind::COMMA);
        skip_newlines();
    }

    expect(TokenKind::RBRACE, "expected '}' after strata variants");
    return s;
}

// ── Types ─────────────────────────────────────────────────────────────────

OFSType Parser::parse_type() {
    if (match(TokenKind::KW_STONE))    return OFSType::stone();
    if (match(TokenKind::KW_CRYSTAL))  return OFSType::crystal();
    if (match(TokenKind::KW_OBSIDIAN)) return OFSType::obsidian();
    if (match(TokenKind::KW_BOOL))     return OFSType::boolean();
    if (match(TokenKind::KW_VOID))     return OFSType::void_t();
    if (match(TokenKind::KW_FLOW))     return OFSType::crystal();
    if (match(TokenKind::KW_TRUTH))    return OFSType::boolean();
    if (match(TokenKind::KW_GLYPH))    return OFSType::obsidian();
    if (match(TokenKind::KW_PTR))      return OFSType::shard_of(OFSType::stone());
    if (check(TokenKind::STAR))        return parse_shard_type();

    // Named type (monolith)
    if (check(TokenKind::IDENT)) {
        return OFSType::named(advance().lexeme);
    }

    throw error(OFS_MSG("expected type", "tipo esperado"));
}

OFSType Parser::parse_shard_type() {
    advance(); // consume '*'
    OFSType inner = parse_type();
    return OFSType::shard_of(inner);
}

// ── Expressions (Pratt parser) ────────────────────────────────────────────

ExprPtr Parser::parse_expr(int min_prec) {
    auto left = parse_unary();

    while (true) {
        // Check for assignment operators
        if (check(TokenKind::EQ) || check(TokenKind::PLUS_EQ) ||
            check(TokenKind::MINUS_EQ) || check(TokenKind::STAR_EQ) ||
            check(TokenKind::SLASH_EQ) || check(TokenKind::PERCENT_EQ)) {
            std::string op;
            switch (peek().kind) {
                case TokenKind::EQ:         op = "="; break;
                case TokenKind::PLUS_EQ:    op = "+="; break;
                case TokenKind::MINUS_EQ:   op = "-="; break;
                case TokenKind::STAR_EQ:    op = "*="; break;
                case TokenKind::SLASH_EQ:   op = "/="; break;
                case TokenKind::PERCENT_EQ: op = "%="; break;
                default: break;
            }
            advance();
            auto assign = std::make_unique<AssignExpr>();
            assign->line = left->line;
            assign->col = left->col;
            assign->op = op;
            assign->target = std::move(left);
            assign->value = parse_expr(0);
            left = std::move(assign);
            continue;
        }

        // Binary operators
        if (!is_binary_op(peek().kind)) break;
        int prec = precedence(peek().kind);
        if (prec < min_prec) break;

        std::string op = token_to_op(peek().kind);
        advance();

        int next_min = is_right_assoc(peek().kind) ? prec : prec + 1;
        auto right = parse_expr(next_min);

        auto bin = std::make_unique<BinaryExpr>();
        bin->line = left->line;
        bin->col = left->col;
        bin->op = op;
        bin->left = std::move(left);
        bin->right = std::move(right);
        left = std::move(bin);
    }

    // Type cast: expr as type
    if (check(TokenKind::KW_AS)) {
        advance(); // consume 'as'
        auto cast = std::make_unique<CastExpr>();
        cast->line = left->line;
        cast->col = left->col;
        cast->target_type = parse_type();
        cast->expr = std::move(left);
        left = std::move(cast);
    }

    return left;
}

ExprPtr Parser::parse_unary() {
    // Prefix unary: -, !, ~, *, &, ++, --
    if (check(TokenKind::MINUS) || check(TokenKind::BANG) || check(TokenKind::TILDE) ||
        check(TokenKind::STAR) || check(TokenKind::AMP) ||
        check(TokenKind::PLUS_PLUS) || check(TokenKind::MINUS_MINUS)) {
        auto un = std::make_unique<UnaryExpr>();
        un->line = peek().line;
        un->col = peek().col;
        un->prefix = true;
        switch (peek().kind) {
            case TokenKind::MINUS:       un->op = "-"; break;
            case TokenKind::BANG:        un->op = "!"; break;
            case TokenKind::TILDE:       un->op = "~"; break;
            case TokenKind::STAR:        un->op = "*"; break;
            case TokenKind::AMP:         un->op = "&"; break;
            case TokenKind::PLUS_PLUS:   un->op = "++"; break;
            case TokenKind::MINUS_MINUS: un->op = "--"; break;
            default: break;
        }
        advance();
        un->operand = parse_unary();
        return un;
    }

    auto expr = parse_primary();

    // Postfix: ++, --, (...), [...], .field
    while (true) {
        if (check(TokenKind::PLUS_PLUS)) {
            advance();
            auto un = std::make_unique<UnaryExpr>();
            un->line = expr->line;
            un->col = expr->col;
            un->prefix = false;
            un->op = "++";
            un->operand = std::move(expr);
            expr = std::move(un);
        } else if (check(TokenKind::MINUS_MINUS)) {
            advance();
            auto un = std::make_unique<UnaryExpr>();
            un->line = expr->line;
            un->col = expr->col;
            un->prefix = false;
            un->op = "--";
            un->operand = std::move(expr);
            expr = std::move(un);
        } else if (check(TokenKind::LPAREN)) {
            expr = parse_call(std::move(expr));
        } else if (check(TokenKind::LBRACKET)) {
            expr = parse_index(std::move(expr));
        } else if (check(TokenKind::DOT)) {
            expr = parse_member(std::move(expr));
        } else {
            break;
        }
    }

    return expr;
}

ExprPtr Parser::parse_primary() {
    // Integer literal
    if (check(TokenKind::INT_LIT)) {
        auto e = std::make_unique<IntLitExpr>();
        e->line = peek().line;
        e->col = peek().col;
        e->value = peek().int_val;
        advance();
        return e;
    }

    // Float literal
    if (check(TokenKind::FLOAT_LIT)) {
        auto e = std::make_unique<FloatLitExpr>();
        e->line = peek().line;
        e->col = peek().col;
        e->value = peek().float_val;
        advance();
        return e;
    }

    // String literal
    if (check(TokenKind::STRING_LIT)) {
        auto e = std::make_unique<StringLitExpr>();
        e->line = peek().line;
        e->col = peek().col;
        e->value = peek().lexeme;
        advance();
        return e;
    }

    // Boolean literals
    if (check(TokenKind::KW_TRUE)) {
        auto e = std::make_unique<BoolLitExpr>();
        e->line = peek().line;
        e->col = peek().col;
        e->value = true;
        advance();
        return e;
    }
    if (check(TokenKind::KW_FALSE)) {
        auto e = std::make_unique<BoolLitExpr>();
        e->line = peek().line;
        e->col = peek().col;
        e->value = false;
        advance();
        return e;
    }

    // Null literal
    if (check(TokenKind::KW_NULL)) {
        auto e = std::make_unique<NullLitExpr>();
        e->line = peek().line;
        e->col = peek().col;
        advance();
        return e;
    }

    // echo(...) — built-in print
    if (check(TokenKind::KW_ECHO)) {
        auto e = std::make_unique<EchoExpr>();
        e->line = peek().line;
        e->col = peek().col;
        advance(); // consume 'echo'
        expect(TokenKind::LPAREN, "expected '(' after 'echo'");
        if (!check(TokenKind::RPAREN)) {
            do {
                skip_newlines();
                e->args.push_back(parse_expr());
            } while (match(TokenKind::COMMA));
        }
        skip_newlines();
        expect(TokenKind::RPAREN, "expected ')' after echo arguments");
        return e;
    }

    // shard variable declaration in fracture (shard p: *stone = &x)
    if (check(TokenKind::KW_SHARD)) {
        // Treat as identifier for now - the semantic analyzer handles context
        auto e = std::make_unique<IdentExpr>();
        e->line = peek().line;
        e->col = peek().col;
        e->name = "shard";
        advance();
        return e;
    }

    // Identifier
    if (check(TokenKind::IDENT)) {
        auto e = std::make_unique<IdentExpr>();
        e->line = peek().line;
        e->col = peek().col;
        e->name = peek().lexeme;
        advance();
        return e;
    }

    // Array literal: [expr, expr, ...]
    if (check(TokenKind::LBRACKET)) {
        return parse_array_literal();
    }

    // Parenthesized expression
    if (match(TokenKind::LPAREN)) {
        auto expr = parse_expr();
        expect(TokenKind::RPAREN, "expected ')' after expression");
        return expr;
    }

    throw error(OFS_MSG("expected expression", "expressão esperada"));
}

ExprPtr Parser::parse_call(ExprPtr callee) {
    auto call = std::make_unique<CallExpr>();
    call->line = callee->line;
    call->col = callee->col;
    call->callee = std::move(callee);

    advance(); // consume '('
    if (!check(TokenKind::RPAREN)) {
        do {
            skip_newlines();
            call->args.push_back(parse_expr());
        } while (match(TokenKind::COMMA));
    }
    skip_newlines();
    expect(TokenKind::RPAREN, "expected ')' after arguments");

    return call;
}

ExprPtr Parser::parse_index(ExprPtr object) {
    auto idx = std::make_unique<IndexExpr>();
    idx->line = object->line;
    idx->col = object->col;
    idx->object = std::move(object);

    advance(); // consume '['
    idx->index = parse_expr();
    expect(TokenKind::RBRACKET, "expected ']' after index");

    return idx;
}

ExprPtr Parser::parse_member(ExprPtr object) {
    auto mem = std::make_unique<MemberExpr>();
    mem->line = object->line;
    mem->col = object->col;
    mem->object = std::move(object);

    advance(); // consume '.'
    mem->field = expect(TokenKind::IDENT, "expected field name after '.'").lexeme;

    return mem;
}

ExprPtr Parser::parse_array_literal() {
    auto arr = std::make_unique<ArrayLitExpr>();
    arr->line = peek().line;
    arr->col = peek().col;

    advance(); // consume '['
    if (!check(TokenKind::RBRACKET)) {
        do {
            skip_newlines();
            arr->elements.push_back(parse_expr());
        } while (match(TokenKind::COMMA));
    }
    skip_newlines();
    expect(TokenKind::RBRACKET, "expected ']' after array elements");

    return arr;
}

} // namespace ofs
