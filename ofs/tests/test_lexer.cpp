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
    t = lex("+ - * / % %= == != < <= > >= << >> & | ^ && || -> ++ --");
    test("plus",      t[0].kind == TokenKind::PLUS);
    test("minus",     t[1].kind == TokenKind::MINUS);
    test("star",      t[2].kind == TokenKind::STAR);
    test("slash",     t[3].kind == TokenKind::SLASH);
    test("percent",   t[4].kind == TokenKind::PERCENT);
    test("percent_eq", t[5].kind == TokenKind::PERCENT_EQ);
    test("eq_eq",     t[6].kind == TokenKind::EQ_EQ);
    test("bang_eq",   t[7].kind == TokenKind::BANG_EQ);
    test("lt",        t[8].kind == TokenKind::LT);
    test("lt_eq",     t[9].kind == TokenKind::LT_EQ);
    test("gt",        t[10].kind == TokenKind::GT);
    test("gt_eq",     t[11].kind == TokenKind::GT_EQ);
    test("lshift",    t[12].kind == TokenKind::LSHIFT);
    test("rshift",    t[13].kind == TokenKind::RSHIFT);
    test("amp",       t[14].kind == TokenKind::AMP);
    test("pipe",      t[15].kind == TokenKind::PIPE);
    test("caret",     t[16].kind == TokenKind::CARET);
    test("amp_amp",   t[17].kind == TokenKind::AMP_AMP);
    test("pipe_pipe", t[18].kind == TokenKind::PIPE_PIPE);
    test("arrow",     t[19].kind == TokenKind::ARROW);
    test("plus_plus", t[20].kind == TokenKind::PLUS_PLUS);
    test("minus_minus", t[21].kind == TokenKind::MINUS_MINUS);

    // Delimiters
    t = lex("( ) { } [ ] , ; :");
    test("lparen",    t[0].kind == TokenKind::LPAREN);
    test("rparen",    t[1].kind == TokenKind::RPAREN);
    test("lbrace",    t[2].kind == TokenKind::LBRACE);
    test("rbrace",    t[3].kind == TokenKind::RBRACE);
    test("lbracket",  t[4].kind == TokenKind::LBRACKET);
    test("rbracket",  t[5].kind == TokenKind::RBRACKET);
    test("comma",     t[6].kind == TokenKind::COMMA);
    test("semicolon", t[7].kind == TokenKind::SEMICOLON);
    test("colon",     t[8].kind == TokenKind::COLON);

    // Comments are skipped
    t = lex("42 // this is a comment\n 99");
    test("comment skip",   t[0].int_val == 42);
    test("after comment",  t[2].int_val == 99);

    // Block comments
    t = lex("1 /* block comment */ 2");
    test("block comment skip", t[0].int_val == 1);
    test("after block comment", t[1].int_val == 2);

    // String escape sequences
    t = lex("\"hello\\nworld\"");
    test("string escape", t[0].kind == TokenKind::STRING_LIT && t[0].lexeme == "hello\nworld");

    // Hex literal
    t = lex("0xFF");
    test("hex literal", t[0].kind == TokenKind::INT_LIT && t[0].int_val == 255);

    // EOF
    t = lex("");
    test("empty source EOF", t[0].kind == TokenKind::END_OF_FILE);

    // Newlines
    t = lex("a\nb");
    test("newline token", t[1].kind == TokenKind::NEWLINE);

    // Full program
    t = lex("core main() {\n    echo(\"Hello\")\n}");
    test("program: core",   t[0].kind == TokenKind::KW_CORE);
    test("program: main",   t[1].kind == TokenKind::IDENT && t[1].lexeme == "main");
    test("program: lparen", t[2].kind == TokenKind::LPAREN);
    test("program: rparen", t[3].kind == TokenKind::RPAREN);
    test("program: lbrace", t[4].kind == TokenKind::LBRACE);
    test("program: echo",   t[5].kind == TokenKind::KW_ECHO);

    // New keywords: import, while, extern, as, match/case/default/const/strata/tremor/catch/throw
    t = lex("import while extern as match case default const strata tremor catch throw");
    test("import keyword",  t[0].kind == TokenKind::KW_IMPORT);
    test("while keyword",   t[1].kind == TokenKind::KW_WHILE);
    test("extern keyword",  t[2].kind == TokenKind::KW_EXTERN);
    test("as keyword",      t[3].kind == TokenKind::KW_AS);
    test("match keyword",   t[4].kind == TokenKind::KW_MATCH);
    test("case keyword",    t[5].kind == TokenKind::KW_CASE);
    test("default keyword", t[6].kind == TokenKind::KW_DEFAULT);
    test("const keyword",   t[7].kind == TokenKind::KW_CONST);
    test("strata keyword",  t[8].kind == TokenKind::KW_STRATA);
    test("tremor keyword",  t[9].kind == TokenKind::KW_TREMOR);
    test("catch keyword",   t[10].kind == TokenKind::KW_CATCH);
    test("throw keyword",   t[11].kind == TokenKind::KW_THROW);

    // Import statement
    t = lex("import \"stdlib/core.ofs\"");
    test("import token",  t[0].kind == TokenKind::KW_IMPORT);
    test("import path",   t[1].kind == TokenKind::STRING_LIT && t[1].lexeme == "stdlib/core.ofs");

    // While loop tokens
    t = lex("while (x > 0) { }");
    test("while token",   t[0].kind == TokenKind::KW_WHILE);
    test("while lparen",  t[1].kind == TokenKind::LPAREN);

    // Type cast tokens
    t = lex("x as stone");
    test("cast ident",    t[0].kind == TokenKind::IDENT);
    test("cast as",       t[1].kind == TokenKind::KW_AS);
    test("cast type",     t[2].kind == TokenKind::KW_STONE);

    // Extern function tokens
    t = lex("extern vein printf");
    test("extern token",  t[0].kind == TokenKind::KW_EXTERN);
    test("extern vein",   t[1].kind == TokenKind::KW_VEIN);

    // OSF keywords
    t = lex("intent pure impure fractal obsid flow truth glyph ptr");
    test("intent keyword",  t[0].kind == TokenKind::KW_INTENT);
    test("pure keyword",    t[1].kind == TokenKind::KW_PURE);
    test("impure keyword",  t[2].kind == TokenKind::KW_IMPURE);
    test("fractal keyword", t[3].kind == TokenKind::KW_FRACTAL);
    test("obsid keyword",   t[4].kind == TokenKind::KW_OBSID);
    test("flow keyword",    t[5].kind == TokenKind::KW_FLOW);
    test("truth keyword",   t[6].kind == TokenKind::KW_TRUTH);
    test("glyph keyword",   t[7].kind == TokenKind::KW_GLYPH);
    test("ptr keyword",     t[8].kind == TokenKind::KW_PTR);

    std::cout << "\nAll lexer tests passed!\n";
    return 0;
}
