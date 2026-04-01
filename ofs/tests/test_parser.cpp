#include "lexer/lexer.hpp"
#include "parser/parser.hpp"
#include <cassert>
#include <iostream>

using namespace ofs;

void test(const std::string& name, bool cond) {
    if (!cond) { std::cerr << "FAILED: " << name << "\n"; assert(false); }
    std::cout << "OK: " << name << "\n";
}

Module parse_source(const std::string& src) {
    Lexer lexer(src);
    auto tokens = lexer.tokenize();
    Parser parser(std::move(tokens));
    return parser.parse();
}

int main() {
    // Parse hello world
    {
        auto mod = parse_source("core main() {\n    echo(\"Hello\")\n}");
        test("hello: 1 decl", mod.decls.size() == 1);
        auto* fn = dynamic_cast<FuncDecl*>(mod.decls[0].get());
        test("hello: is FuncDecl", fn != nullptr);
        test("hello: name=main", fn->name == "main");
        test("hello: is_core", fn->is_core);
        test("hello: no params", fn->params.empty());
        auto* body = dynamic_cast<BlockStmt*>(fn->body.get());
        test("hello: body is block", body != nullptr);
        test("hello: 1 stmt", body->stmts.size() == 1);
        auto* expr_stmt = dynamic_cast<ExprStmt*>(body->stmts[0].get());
        test("hello: expr stmt", expr_stmt != nullptr);
        auto* echo = dynamic_cast<EchoExpr*>(expr_stmt->expr.get());
        test("hello: echo expr", echo != nullptr);
        test("hello: echo 1 arg", echo->args.size() == 1);
    }

    // Parse variable declaration
    {
        auto mod = parse_source("core main() {\n    forge x: stone = 42\n}");
        auto* fn = dynamic_cast<FuncDecl*>(mod.decls[0].get());
        auto* body = dynamic_cast<BlockStmt*>(fn->body.get());
        auto* forge = dynamic_cast<ForgeStmt*>(body->stmts[0].get());
        test("forge: exists", forge != nullptr);
        test("forge: name=x", forge->name == "x");
        test("forge: type=stone", forge->type_ann && forge->type_ann->base == BaseType::Stone);
        auto* init = dynamic_cast<IntLitExpr*>(forge->initializer.get());
        test("forge: init=42", init && init->value == 42);
    }

    // Parse if/else
    {
        auto mod = parse_source("core main() {\n    if (x > 10) {\n        echo(\"big\")\n    } else {\n        echo(\"small\")\n    }\n}");
        auto* fn = dynamic_cast<FuncDecl*>(mod.decls[0].get());
        auto* body = dynamic_cast<BlockStmt*>(fn->body.get());
        auto* if_stmt = dynamic_cast<IfStmt*>(body->stmts[0].get());
        test("if: exists", if_stmt != nullptr);
        test("if: has then", if_stmt->then_block != nullptr);
        test("if: has else", if_stmt->else_block != nullptr);
    }

    // Parse C-style cycle
    {
        auto mod = parse_source("core main() {\n    cycle (forge i = 0; i < 10; i++) {\n        echo(i)\n    }\n}");
        auto* fn = dynamic_cast<FuncDecl*>(mod.decls[0].get());
        auto* body = dynamic_cast<BlockStmt*>(fn->body.get());
        auto* cycle = dynamic_cast<CycleStmt*>(body->stmts[0].get());
        test("cycle: exists", cycle != nullptr);
        test("cycle: not range", !cycle->is_range);
        test("cycle: has init", cycle->init != nullptr);
        test("cycle: has cond", cycle->cond != nullptr);
        test("cycle: has step", cycle->step != nullptr);
        test("cycle: has body", cycle->body != nullptr);
    }

    // Parse function with params and return type
    {
        auto mod = parse_source("vein add(a: stone, b: stone) -> stone {\n    return a + b\n}");
        auto* fn = dynamic_cast<FuncDecl*>(mod.decls[0].get());
        test("func: exists", fn != nullptr);
        test("func: name=add", fn->name == "add");
        test("func: 2 params", fn->params.size() == 2);
        test("func: ret=stone", fn->return_type.base == BaseType::Stone);
    }

    // Parse monolith
    {
        auto mod = parse_source("monolith Point {\n    x: stone\n    y: stone\n}");
        auto* m = dynamic_cast<MonolithDecl*>(mod.decls[0].get());
        test("monolith: exists", m != nullptr);
        test("monolith: name=Point", m->name == "Point");
        test("monolith: 2 fields", m->fields.size() == 2);
        test("monolith: field0=x", m->fields[0].name == "x");
        test("monolith: field1=y", m->fields[1].name == "y");
    }

    // Parse array literal
    {
        auto mod = parse_source("core main() {\n    forge arr = [1, 2, 3]\n}");
        auto* fn = dynamic_cast<FuncDecl*>(mod.decls[0].get());
        auto* body = dynamic_cast<BlockStmt*>(fn->body.get());
        auto* forge = dynamic_cast<ForgeStmt*>(body->stmts[0].get());
        test("array: forge exists", forge != nullptr);
        auto* arr = dynamic_cast<ArrayLitExpr*>(forge->initializer.get());
        test("array: is array", arr != nullptr);
        test("array: 3 elements", arr->elements.size() == 3);
    }

    // Parse binary expression
    {
        auto mod = parse_source("core main() {\n    forge z = 1 + 2 * 3\n}");
        auto* fn = dynamic_cast<FuncDecl*>(mod.decls[0].get());
        auto* body = dynamic_cast<BlockStmt*>(fn->body.get());
        auto* forge = dynamic_cast<ForgeStmt*>(body->stmts[0].get());
        auto* bin = dynamic_cast<BinaryExpr*>(forge->initializer.get());
        test("binop: is binary", bin != nullptr);
        test("binop: op=+", bin->op == "+");
        // Right side should be 2*3 (higher precedence)
        auto* right = dynamic_cast<BinaryExpr*>(bin->right.get());
        test("binop: right is binary", right != nullptr);
        test("binop: right op=*", right->op == "*");
    }

    // Parse fracture block
    {
        auto mod = parse_source("core main() {\n    fracture {\n        forge x = 1\n    }\n}");
        auto* fn = dynamic_cast<FuncDecl*>(mod.decls[0].get());
        auto* body = dynamic_cast<BlockStmt*>(fn->body.get());
        auto* frac = dynamic_cast<FractureStmt*>(body->stmts[0].get());
        test("fracture: exists", frac != nullptr);
        test("fracture: has body", frac->body != nullptr);
    }

    // Parse return statement
    {
        auto mod = parse_source("vein foo() -> stone {\n    return 42\n}");
        auto* fn = dynamic_cast<FuncDecl*>(mod.decls[0].get());
        auto* body = dynamic_cast<BlockStmt*>(fn->body.get());
        auto* ret = dynamic_cast<ReturnStmt*>(body->stmts[0].get());
        test("return: exists", ret != nullptr);
        test("return: has value", ret->value != nullptr);
        auto* val = dynamic_cast<IntLitExpr*>(ret->value.get());
        test("return: value=42", val && val->value == 42);
    }

    // Parse import declaration
    {
        auto mod = parse_source("import \"stdlib/core.ofs\"\ncore main() {\n    echo(\"hi\")\n}");
        test("import: 2 decls", mod.decls.size() == 2);
        auto* imp = dynamic_cast<ImportDecl*>(mod.decls[0].get());
        test("import: is ImportDecl", imp != nullptr);
        test("import: path", imp->path == "stdlib/core.ofs");
    }

    // Parse extern function
    {
        auto mod = parse_source("extern vein puts(s: obsidian) -> stone\ncore main() {\n    echo(\"hi\")\n}");
        test("extern: 2 decls", mod.decls.size() == 2);
        auto* ext = dynamic_cast<ExternFuncDecl*>(mod.decls[0].get());
        test("extern: is ExternFuncDecl", ext != nullptr);
        test("extern: name=puts", ext->name == "puts");
        test("extern: 1 param", ext->params.size() == 1);
        test("extern: ret=stone", ext->return_type.base == BaseType::Stone);
    }

    // Parse while loop
    {
        auto mod = parse_source("core main() {\n    while (x > 0) {\n        echo(x)\n    }\n}");
        auto* fn = dynamic_cast<FuncDecl*>(mod.decls[0].get());
        auto* body = dynamic_cast<BlockStmt*>(fn->body.get());
        auto* w = dynamic_cast<WhileCycleStmt*>(body->stmts[0].get());
        test("while: exists", w != nullptr);
        test("while: has cond", w->cond != nullptr);
        test("while: has body", w->body != nullptr);
    }

    // Parse type cast
    {
        auto mod = parse_source("core main() {\n    forge y = 42 as crystal\n}");
        auto* fn = dynamic_cast<FuncDecl*>(mod.decls[0].get());
        auto* body = dynamic_cast<BlockStmt*>(fn->body.get());
        auto* forge = dynamic_cast<ForgeStmt*>(body->stmts[0].get());
        test("cast: forge exists", forge != nullptr);
        auto* cast = dynamic_cast<CastExpr*>(forge->initializer.get());
        test("cast: is CastExpr", cast != nullptr);
        test("cast: target=crystal", cast->target_type.base == BaseType::Crystal);
    }

    // Parse const declaration
    {
        auto mod = parse_source("core main() {\n    const max: stone = 42\n}");
        auto* fn = dynamic_cast<FuncDecl*>(mod.decls[0].get());
        auto* body = dynamic_cast<BlockStmt*>(fn->body.get());
        auto* cst = dynamic_cast<ConstStmt*>(body->stmts[0].get());
        test("const: exists", cst != nullptr);
        test("const: name=max", cst->name == "max");
        test("const: typed", cst->type_ann && cst->type_ann->base == BaseType::Stone);
    }

    // Parse strata declaration
    {
        auto mod = parse_source("strata Color { Red, Green, Blue }\ncore main() {\n    echo(1)\n}");
        test("strata: 2 decls", mod.decls.size() == 2);
        auto* st = dynamic_cast<StrataDecl*>(mod.decls[0].get());
        test("strata: exists", st != nullptr);
        test("strata: name=Color", st->name == "Color");
        test("strata: variants=3", st->variants.size() == 3);
    }

    // Parse match statement
    {
        auto mod = parse_source(
            "core main() {\n"
            "    match x {\n"
            "        case 1: { echo(\"one\") }\n"
            "        default: { echo(\"other\") }\n"
            "    }\n"
            "}");
        auto* fn = dynamic_cast<FuncDecl*>(mod.decls[0].get());
        auto* body = dynamic_cast<BlockStmt*>(fn->body.get());
        auto* m = dynamic_cast<MatchStmt*>(body->stmts[0].get());
        test("match: exists", m != nullptr);
        test("match: arms=2", m->arms.size() == 2);
        test("match: has default", m->arms[1].is_default);
    }

    // Parse tremor/catch + throw
    {
        auto mod = parse_source(
            "core main() {\n"
            "    tremor {\n"
            "        throw \"boom\"\n"
            "    } catch (e: obsidian) {\n"
            "        echo(e)\n"
            "    }\n"
            "}");
        auto* fn = dynamic_cast<FuncDecl*>(mod.decls[0].get());
        auto* body = dynamic_cast<BlockStmt*>(fn->body.get());
        auto* tr = dynamic_cast<TremorStmt*>(body->stmts[0].get());
        test("tremor: exists", tr != nullptr);
        test("tremor: catch var", tr->catch_var == "e");
        auto* tblock = dynamic_cast<BlockStmt*>(tr->body.get());
        auto* th = dynamic_cast<ThrowStmt*>(tblock->stmts[0].get());
        test("throw: exists", th != nullptr);
    }

    // Parse bitwise and shifts precedence
    {
        auto mod = parse_source("core main() {\n    forge x = 1 << 2 | 3\n}");
        auto* fn = dynamic_cast<FuncDecl*>(mod.decls[0].get());
        auto* body = dynamic_cast<BlockStmt*>(fn->body.get());
        auto* forge = dynamic_cast<ForgeStmt*>(body->stmts[0].get());
        auto* top = dynamic_cast<BinaryExpr*>(forge->initializer.get());
        test("bitwise: top is binary", top != nullptr);
        test("bitwise: top op=|", top->op == "|");
    }

    std::cout << "\nAll parser tests passed!\n";
    return 0;
}
