#include "lexer.hpp"
#include <sstream>
#include <cstdlib>
#include "../i18n.hpp"

namespace ofs {

// ── Token::to_string ──────────────────────────────────────────────────────

std::string Token::to_string() const {
    std::ostringstream ss;
    ss << line << ":" << col << " ";
    switch (kind) {
        case TokenKind::INT_LIT:    ss << "INT(" << int_val << ")"; break;
        case TokenKind::FLOAT_LIT:  ss << "FLOAT(" << float_val << ")"; break;
        case TokenKind::STRING_LIT: ss << "STRING(\"" << lexeme << "\")"; break;
        case TokenKind::IDENT:      ss << "IDENT(" << lexeme << ")"; break;
        case TokenKind::NEWLINE:    ss << "NEWLINE"; break;
        case TokenKind::END_OF_FILE:ss << "EOF"; break;
        case TokenKind::TOK_ERROR:  ss << "ERROR(" << lexeme << ")"; break;
        default:                    ss << "TOKEN(" << lexeme << ")"; break;
    }
    return ss.str();
}

// ── Lexer ─────────────────────────────────────────────────────────────────

Lexer::Lexer(const std::string& source, const std::string& filename)
    : source_(source), filename_(filename) {}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    while (!is_at_end()) {
        Token t = next_token();
        if (t.kind != TokenKind::TOK_ERROR || !t.lexeme.empty()) {
            tokens.push_back(std::move(t));
        }
        if (tokens.back().kind == TokenKind::END_OF_FILE) break;
    }
    if (tokens.empty() || tokens.back().kind != TokenKind::END_OF_FILE) {
        Token eof;
        eof.kind = TokenKind::END_OF_FILE;
        eof.lexeme = "";
        eof.line = line_;
        eof.col = col_;
        tokens.push_back(eof);
    }
    return tokens;
}

Token Lexer::next_token() {
    skip_whitespace();

    if (is_at_end()) {
        tok_start_ = pos_;
        tok_line_ = line_;
        tok_col_ = col_;
        return make_token(TokenKind::END_OF_FILE);
    }

    tok_start_ = pos_;
    tok_line_ = line_;
    tok_col_ = col_;

    char c = advance();

    // Newline
    if (c == '\n') {
        // If inside parens/brackets/braces, skip newlines (implicit continuation)
        if (paren_depth_ > 0 || bracket_depth_ > 0 || brace_depth_ > 0) {
            return next_token();
        }
        return make_token(TokenKind::NEWLINE);
    }

    // String literal
    if (c == '"') return lex_string();

    // Number literal
    if (is_digit(c)) return lex_number();

    // Hex literal: 0x...
    if (c == '0' && peek() == 'x') {
        return lex_number();
    }

    // Identifier or keyword
    if (is_alpha(c)) return lex_ident();

    // Two-character operators
    switch (c) {
        case '(': paren_depth_++; return make_token(TokenKind::LPAREN);
        case ')': paren_depth_--; return make_token(TokenKind::RPAREN);
        case '{': brace_depth_++; return make_token(TokenKind::LBRACE);
        case '}': brace_depth_--; return make_token(TokenKind::RBRACE);
        case '[': bracket_depth_++; return make_token(TokenKind::LBRACKET);
        case ']': bracket_depth_--; return make_token(TokenKind::RBRACKET);
        case ',': return make_token(TokenKind::COMMA);
        case ';': return make_token(TokenKind::SEMICOLON);
        case ':': return make_token(TokenKind::COLON);
        case '.':
            if (match('.') && match('.')) return make_token(TokenKind::ELLIPSIS);
            return make_token(TokenKind::DOT);
        case '~': return make_token(TokenKind::TILDE);
        case '^': return make_token(TokenKind::CARET);

        case '+':
            if (match('+')) return make_token(TokenKind::PLUS_PLUS);
            if (match('=')) return make_token(TokenKind::PLUS_EQ);
            return make_token(TokenKind::PLUS);

        case '-':
            if (match('-')) return make_token(TokenKind::MINUS_MINUS);
            if (match('>')) return make_token(TokenKind::ARROW);
            if (match('=')) return make_token(TokenKind::MINUS_EQ);
            return make_token(TokenKind::MINUS);

        case '*':
            if (match('=')) return make_token(TokenKind::STAR_EQ);
            return make_token(TokenKind::STAR);

        case '/':
            if (match('=')) return make_token(TokenKind::SLASH_EQ);
            return make_token(TokenKind::SLASH);

        case '%':
            if (match('=')) return make_token(TokenKind::PERCENT_EQ);
            return make_token(TokenKind::PERCENT);

        case '&':
            if (match('&')) return make_token(TokenKind::AMP_AMP);
            return make_token(TokenKind::AMP);

        case '|':
            if (match('|')) return make_token(TokenKind::PIPE_PIPE);
            return make_token(TokenKind::PIPE);

        case '!':
            if (match('=')) return make_token(TokenKind::BANG_EQ);
            return make_token(TokenKind::BANG);

        case '=':
            if (match('=')) return make_token(TokenKind::EQ_EQ);
            return make_token(TokenKind::EQ);

        case '<':
            if (match('<')) return make_token(TokenKind::LSHIFT);
            if (match('=')) return make_token(TokenKind::LT_EQ);
            return make_token(TokenKind::LT);

        case '>':
            if (match('>')) return make_token(TokenKind::RSHIFT);
            if (match('=')) return make_token(TokenKind::GT_EQ);
            return make_token(TokenKind::GT);
    }

    return make_error(OFS_MSG(std::string("unexpected character: ") + c,
                              std::string("caractere inesperado: ") + c));
}

// ── Character helpers ─────────────────────────────────────────────────────

char Lexer::peek(int offset) const {
    size_t idx = pos_ + offset;
    if (idx >= source_.size()) return '\0';
    return source_[idx];
}

char Lexer::advance() {
    char c = source_[pos_++];
    if (c == '\n') { line_++; col_ = 1; }
    else           { col_++; }
    return c;
}

bool Lexer::match(char expected) {
    if (is_at_end() || source_[pos_] != expected) return false;
    advance();
    return true;
}

bool Lexer::is_at_end() const { return pos_ >= source_.size(); }
bool Lexer::is_digit(char c) const { return c >= '0' && c <= '9'; }
bool Lexer::is_alpha(char c) const { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_'; }
bool Lexer::is_alnum(char c) const { return is_alpha(c) || is_digit(c); }

// ── Token builders ────────────────────────────────────────────────────────

Token Lexer::make_token(TokenKind kind) {
    Token t;
    t.kind = kind;
    t.lexeme = source_.substr(tok_start_, pos_ - tok_start_);
    t.line = tok_line_;
    t.col = tok_col_;
    return t;
}

Token Lexer::make_error(const std::string& msg) {
    Token t;
    t.kind = TokenKind::TOK_ERROR;
    t.lexeme = msg;
    t.line = tok_line_;
    t.col = tok_col_;
    return t;
}

Token Lexer::lex_string() {
    // Opening " already consumed
    std::string value;
    while (!is_at_end() && peek() != '"') {
        char c = peek();
        if (c == '\n') {
            return make_error(OFS_MSG("unterminated string literal", "literal de string não terminada"));
        }
        if (c == '\\') {
            advance(); // skip backslash
            if (is_at_end()) return make_error(OFS_MSG("unterminated escape in string", "sequência de escape não terminada na string"));
            char esc = advance();
            switch (esc) {
                case 'n':  value += '\n'; break;
                case 't':  value += '\t'; break;
                case '\\': value += '\\'; break;
                case '"':  value += '"'; break;
                case '0':  value += '\0'; break;
                default:
                    value += '\\';
                    value += esc;
                    break;
            }
        } else {
            value += advance();
        }
    }
    if (is_at_end()) return make_error(OFS_MSG("unterminated string literal", "literal de string não terminada"));
    advance(); // consume closing "

    Token t;
    t.kind = TokenKind::STRING_LIT;
    t.lexeme = value;
    t.line = tok_line_;
    t.col = tok_col_;
    return t;
}

Token Lexer::lex_number() {
    // First digit already consumed, step back
    pos_ = tok_start_;
    col_ = tok_col_;
    line_ = tok_line_;

    bool is_hex = false;
    bool is_float = false;

    // Check for hex
    if (peek() == '0' && peek(1) == 'x') {
        is_hex = true;
        advance(); // 0
        advance(); // x
        while (!is_at_end() && (is_digit(peek()) ||
               (peek() >= 'a' && peek() <= 'f') ||
               (peek() >= 'A' && peek() <= 'F'))) {
            advance();
        }
    } else {
        // Decimal integer or float
        while (!is_at_end() && is_digit(peek())) advance();
        if (!is_at_end() && peek() == '.' && is_digit(peek(1))) {
            is_float = true;
            advance(); // .
            while (!is_at_end() && is_digit(peek())) advance();
        }
    }

    std::string text = source_.substr(tok_start_, pos_ - tok_start_);
    Token t;
    t.line = tok_line_;
    t.col = tok_col_;
    t.lexeme = text;

    if (is_float) {
        t.kind = TokenKind::FLOAT_LIT;
        t.float_val = std::stod(text);
    } else {
        t.kind = TokenKind::INT_LIT;
        if (is_hex) {
            t.int_val = std::stoll(text, nullptr, 16);
        } else {
            t.int_val = std::stoll(text);
        }
    }

    return t;
}

Token Lexer::lex_ident() {
    // First alpha char already consumed, step back
    pos_ = tok_start_;
    col_ = tok_col_;
    line_ = tok_line_;

    while (!is_at_end() && is_alnum(peek())) advance();

    std::string text = source_.substr(tok_start_, pos_ - tok_start_);
    Token t;
    t.line = tok_line_;
    t.col = tok_col_;
    t.lexeme = text;

    auto it = KEYWORDS.find(text);
    if (it != KEYWORDS.end()) {
        t.kind = it->second;
        // Handle boolean literals
        if (t.kind == TokenKind::KW_TRUE) {
            t.kind = TokenKind::KW_TRUE;
        } else if (t.kind == TokenKind::KW_FALSE) {
            t.kind = TokenKind::KW_FALSE;
        }
    } else {
        t.kind = TokenKind::IDENT;
    }

    return t;
}

void Lexer::skip_whitespace() {
    while (!is_at_end()) {
        char c = peek();
        if (c == ' ' || c == '\t' || c == '\r') {
            advance();
        } else if (c == '/' && peek(1) == '/') {
            // Single-line comment: skip until end of line
            while (!is_at_end() && peek() != '\n') advance();
        } else if (c == '/' && peek(1) == '*') {
            // Block comment
            skip_block_comment();
        } else {
            break;
        }
    }
}

void Lexer::skip_block_comment() {
    advance(); // /
    advance(); // *
    int depth = 1;
    while (!is_at_end() && depth > 0) {
        if (peek() == '/' && peek(1) == '*') {
            advance(); advance();
            depth++;
        } else if (peek() == '*' && peek(1) == '/') {
            advance(); advance();
            depth--;
        } else {
            advance();
        }
    }
}

} // namespace ofs
