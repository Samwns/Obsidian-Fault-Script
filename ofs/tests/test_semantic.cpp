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

    test("forward local forge reference in block",
        analyze_ok("core main() {\n    echo(x)\n    forge x = 1 + 2\n}"));

    test("if statement",
        analyze_ok("core main() {\n    forge x = 10\n    if (x > 5) {\n        echo(x)\n    }\n}"));

    test("cycle c-style",
        analyze_ok("core main() {\n    cycle (forge i = 0; i < 10; i++) {\n        echo(i)\n    }\n}"));

    test("function call",
        analyze_ok("vein add(a: stone, b: stone) -> stone {\n    return a + b\n}\ncore main() {\n    forge r = add(1, 2)\n    echo(r)\n}"));

    test("monolith",
        analyze_ok("monolith Point {\n    x: stone\n    y: stone\n}\ncore main() {\n    forge p: Point\n}"));

    test("monolith packed layout",
        analyze_ok("monolith Header layout packed {\n    tag: stone\n    flags: stone\n}\ncore main() {\n    forge h: Header\n}"));

    test("monolith invalid layout",
        analyze_fails("monolith Header layout weird {\n    tag: stone\n}\ncore main() {\n    forge h: Header\n}"));

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

    test("u8 assignment from literal",
        analyze_ok("core main() {\n    forge r: u8 = 255\n}"));

    test("u8 + u8 promotion to u16",
        analyze_ok("core main() {\n    forge a: u8 = 10\n    forge b: u8 = 20\n    forge c: u16 = a + b\n}"));

    test("u8 as u32 cast",
        analyze_ok("core main() {\n    forge a: u8 = 255\n    forge b: u32 = a as u32\n}"));

    test("stone as u8 cast",
        analyze_ok("core main() {\n    forge a: stone = 1024\n    forge b: u8 = a as u8\n}"));

    test("bitwise with u32",
        analyze_ok("core main() {\n    forge r: u8 = 255\n    forge g: u8 = 128\n    forge pixel: u32 = (r as u32 << 16) | (g as u32 << 8)\n}"));

    test("impl method call",
        analyze_ok(
            "monolith Rect {\n"
            "    w: stone\n"
            "    h: stone\n"
            "}\n"
            "impl Rect {\n"
            "    vein area(self) -> stone {\n"
            "        return self.w * self.h\n"
            "    }\n"
            "}\n"
            "core main() {\n"
            "    forge r: Rect\n"
            "    echo(r.area())\n"
            "}"));

    test("namespace call",
        analyze_ok(
            "namespace mymath {\n"
            "    vein square(x: stone) -> stone { return x * x }\n"
            "}\n"
            "core main() {\n"
            "    echo(mymath.square(4))\n"
            "}"));

    test("function value type and lambda",
        analyze_ok(
            "vein aplicar(v: stone, fn: vein(stone) -> stone) -> stone {\n"
            "    return fn(v)\n"
            "}\n"
            "core main() {\n"
            "    forge dobrar: vein(stone) -> stone = vein(x: stone) -> stone { return x * 2 }\n"
            "    echo(aplicar(10, dobrar))\n"
            "}"));

    // Extern function
    test("extern function",
        analyze_ok("extern vein ofs_pow(base: crystal, exp: crystal) -> crystal\ncore main() {\n    forge r: crystal = ofs_pow(2.0, 10.0)\n}"));

    test("rift function",
        analyze_ok("rift vein strlen(text: obsidian) -> stone\ncore main() {\n    forge r: stone = strlen(\"fault\")\n}"));

    test("rift abi metadata",
        analyze_ok("rift vein text_size(text: obsidian) -> stone bind \"strlen\" abi c\ncore main() {\n    forge r: stone = text_size(\"fault\")\n}"));

    test("invalid abi metadata",
        analyze_fails("rift vein text_size(text: obsidian) -> stone abi weird\ncore main() {\n    forge r: stone = text_size(\"fault\")\n}"));

    // Attach
    test("attach declaration",
        analyze_ok("attach {F:stdlib/core.ofs}\ncore main() {\n    echo(\"hello\")\n}"));

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

    test("bedrock allows fault intrinsic",
        analyze_ok(
            "core main() {\n"
            "    bedrock {\n"
            "        forge x: stone = fault_count(15)\n"
            "        echo(x)\n"
            "    }\n"
            "}"));

    test("fault intrinsic fails outside low-level block",
        analyze_fails(
            "core main() {\n"
            "    forge x: stone = fault_count(15)\n"
            "}"));

    test("fault intrinsic requires stone args",
        analyze_fails(
            "core main() {\n"
            "    bedrock {\n"
            "        forge x: stone = fault_count(3.14)\n"
            "    }\n"
            "}"));

    test("bedrock allows pointer work",
        analyze_ok(
            "core main() {\n"
            "    forge x: stone = 10\n"
            "    bedrock {\n"
            "        shard p: *stone = &x\n"
            "        *p = *p + 1\n"
            "    }\n"
            "}"));

    test("fault_step advances stone pointer",
        analyze_ok(
            "extern vein ofs_alloc(size: stone) -> *stone\n"
            "core main() {\n"
            "    forge base: *stone = ofs_alloc(16)\n"
            "    bedrock {\n"
            "        shard slot: *stone = fault_step(base, 1)\n"
            "        *slot = 42\n"
            "    }\n"
            "}"));

    test("fault_step requires pointer first arg",
        analyze_fails(
            "core main() {\n"
            "    bedrock {\n"
            "        forge x: stone = fault_step(10, 1)\n"
            "    }\n"
            "}"));

    test("fault_cut extracts field",
        analyze_ok(
            "core main() {\n"
            "    bedrock {\n"
            "        forge x: stone = fault_cut(255, 4, 4)\n"
            "        echo(x)\n"
            "    }\n"
            "}"));

    test("fault_patch writes field",
        analyze_ok(
            "core main() {\n"
            "    bedrock {\n"
            "        forge x: stone = fault_patch(0, 8, 8, 42)\n"
            "        echo(x)\n"
            "    }\n"
            "}"));

    test("fault_fence allowed in bedrock",
        analyze_ok(
            "core main() {\n"
            "    bedrock {\n"
            "        fault_fence()\n"
            "    }\n"
            "}"));

    test("fault_prefetch allowed in bedrock",
        analyze_ok(
            "extern vein ofs_alloc(size: stone) -> *stone\n"
            "core main() {\n"
            "    forge base: *stone = ofs_alloc(16)\n"
            "    bedrock {\n"
            "        fault_prefetch(base)\n"
            "    }\n"
            "}"));

    test("fault_prefetch requires pointer",
        analyze_fails(
            "core main() {\n"
            "    bedrock {\n"
            "        fault_prefetch(10)\n"
            "    }\n"
            "}"));

    test("fault_trap allowed in bedrock",
        analyze_ok(
            "core main() {\n"
            "    bedrock {\n"
            "        fault_trap()\n"
            "    }\n"
            "}"));

    std::cout << "\nAll semantic tests passed!\n";
    return 0;
}
