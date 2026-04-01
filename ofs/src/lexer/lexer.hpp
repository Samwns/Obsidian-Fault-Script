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

    // Track nesting depth for implicit line continuation
    int paren_depth_   = 0;
    int bracket_depth_ = 0;
    int brace_depth_   = 0;
};

} // namespace ofs
