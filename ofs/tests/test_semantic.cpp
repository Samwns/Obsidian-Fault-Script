#include "lexer/lexer.hpp"
#include "parser/parser.hpp"
#include "semantic/semantic.hpp"
#include <cassert>
#include <iostream>

using namespace ofs;

void test(const std::string& name, bool cond) {
    if (!cond) { std::cerr << "FAILED: " << name << "\n"; assert(false); }
    std::cout << "OK: " << name << "\n";
}

bool analyze_ok(const std::string& src) {
    try {
        Lexer lexer(src);
        auto tokens = lexer.tokenize();
        Parser parser(std::move(tokens));
        auto mod = parser.parse();
        SemanticAnalyzer sem;
        sem.analyze(mod);
        return true;
    } catch (const SemanticError& e) {
        std::cerr << "  SemanticError: " << e.what() << "\n";
        return false;
    } catch (const ParseError& e) {
        std::cerr << "  ParseError: " << e.what() << "\n";
        return false;
    }
}

bool analyze_fails(const std::string& src) {
    return !analyze_ok(src);
}

int main() {
    // Valid programs
    test("hello world",
        analyze_ok("core main() {\n    echo(\"Hello\")\n}"));

    test("forge with type",
        analyze_ok("core main() {\n    forge x: stone = 42\n    echo(x)\n}"));

    test("forge infer int",
        analyze_ok("core main() {\n    forge x = 10\n    echo(x)\n}"));

    test("forge infer float",
        analyze_ok("core main() {\n    forge x = 3.14\n    echo(x)\n}"));

    test("forge infer string",
        analyze_ok("core main() {\n    forge x = \"hello\"\n    echo(x)\n}"));

    test("forge infer bool",
        analyze_ok("core main() {\n    forge x = true\n    echo(x)\n}"));

    test("arithmetic",
        analyze_ok("core main() {\n    forge x = 1 + 2\n    forge y = x * 3\n}"));

    test("if statement",
        analyze_ok("core main() {\n    forge x = 10\n    if (x > 5) {\n        echo(x)\n    }\n}"));

    test("cycle c-style",
        analyze_ok("core main() {\n    cycle (forge i = 0; i < 10; i++) {\n        echo(i)\n    }\n}"));

    test("function call",
        analyze_ok("vein add(a: stone, b: stone) -> stone {\n    return a + b\n}\ncore main() {\n    forge r = add(1, 2)\n    echo(r)\n}"));

    test("monolith",
        analyze_ok("monolith Point {\n    x: stone\n    y: stone\n}\ncore main() {\n    forge p: Point\n}"));

    test("stone + crystal -> crystal (widening)",
        analyze_ok("core main() {\n    forge a: stone = 1\n    forge b: crystal = 2.0\n    forge c = a + b\n}"));

    test("string concat",
        analyze_ok("core main() {\n    forge s = \"hello\" + \" world\"\n    echo(s)\n}"));

    // Invalid programs
    test("undefined var",
        analyze_fails("core main() {\n    echo(z)\n}"));

    test("break outside cycle",
        analyze_fails("core main() {\n    break\n}"));

    test("continue outside cycle",
        analyze_fails("core main() {\n    continue\n}"));

    // While loop
    test("while loop valid",
        analyze_ok("core main() {\n    forge x: stone = 10\n    while (x > 0) {\n        x -= 1\n    }\n}"));

    // Type cast
    test("cast stone to crystal",
        analyze_ok("core main() {\n    forge x: stone = 42\n    forge y: crystal = x as crystal\n}"));

    test("cast crystal to stone",
        analyze_ok("core main() {\n    forge x: crystal = 3.14\n    forge y: stone = x as stone\n}"));

    test("cast bool to stone",
        analyze_ok("core main() {\n    forge b: bool = true\n    forge n: stone = b as stone\n}"));

    // Extern function
    test("extern function",
        analyze_ok("extern vein ofs_pow(base: crystal, exp: crystal) -> crystal\ncore main() {\n    forge r: crystal = ofs_pow(2.0, 10.0)\n}"));

    // Import
    test("import declaration",
        analyze_ok("import \"stdlib/core.ofs\"\ncore main() {\n    echo(\"hello\")\n}"));

    // Invalid: break in while should work
    test("break in while",
        analyze_ok("core main() {\n    forge x: stone = 10\n    while (x > 0) {\n        break\n    }\n}"));

    // Invalid: invalid cast
    test("invalid cast string to stone",
        analyze_fails("core main() {\n    forge s: obsidian = \"hello\"\n    forge n: stone = s as stone\n}"));

    test("pure cannot call impure",
        analyze_fails(
            "vein b() intent impure { return }\n"
            "vein a() intent pure { b() }\n"
            "core main() { a() }"));

    test("impure cannot call fractal outside fractal block",
        analyze_fails(
            "vein f() intent fractal { return }\n"
            "vein a() intent impure { f() }\n"
            "core main() { a() }"));

    test("impure can call fractal inside fractal block",
        analyze_ok(
            "vein f() intent fractal { return }\n"
            "vein a() intent impure { fractal { f() } }\n"
            "core main() { a() }"));

    std::cout << "\nAll semantic tests passed!\n";
    return 0;
}
