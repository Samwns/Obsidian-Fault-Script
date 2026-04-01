#pragma once
#include "ast.hpp"
#include <iostream>
#include <string>

namespace ofs {

class ASTPrinter {
public:
    void print(const Module& mod) {
        std::cout << "Module: " << mod.filename << "\n";
        for (auto& d : mod.decls) {
            print_decl(*d, 0);
        }
    }

private:
    void indent(int depth) {
        for (int i = 0; i < depth; i++) std::cout << "  ";
    }

    void print_decl(const Decl& d, int depth) {
        if (auto* fn = dynamic_cast<const FuncDecl*>(&d)) {
            indent(depth);
            std::cout << (fn->is_core ? "core " : "vein ") << fn->name << "(";
            for (size_t i = 0; i < fn->params.size(); i++) {
                if (i > 0) std::cout << ", ";
                std::cout << fn->params[i].name << ": " << fn->params[i].type.to_string();
            }
            std::cout << ") -> " << fn->return_type.to_string() << "\n";
            if (fn->body) print_stmt(*fn->body, depth + 1);
        } else if (auto* m = dynamic_cast<const MonolithDecl*>(&d)) {
            indent(depth);
            std::cout << "monolith " << m->name << " {\n";
            for (auto& f : m->fields) {
                indent(depth + 1);
                std::cout << f.name << ": " << f.type.to_string() << "\n";
            }
            indent(depth);
            std::cout << "}\n";
        } else if (auto* g = dynamic_cast<const GlobalForgeDecl*>(&d)) {
            indent(depth);
            std::cout << "forge " << g->name;
            if (g->type_ann) std::cout << ": " << g->type_ann->to_string();
            if (g->initializer) {
                std::cout << " = ";
                print_expr(*g->initializer, 0);
            }
            std::cout << "\n";
        } else if (auto* imp = dynamic_cast<const ImportDecl*>(&d)) {
            indent(depth);
            std::cout << "import \"" << imp->path << "\"\n";
        } else if (auto* ext = dynamic_cast<const ExternFuncDecl*>(&d)) {
            indent(depth);
            std::cout << "extern vein " << ext->name << "(";
            for (size_t i = 0; i < ext->params.size(); i++) {
                if (i > 0) std::cout << ", ";
                std::cout << ext->params[i].name << ": " << ext->params[i].type.to_string();
            }
            if (ext->is_variadic) {
                if (!ext->params.empty()) std::cout << ", ";
                std::cout << "...";
            }
            std::cout << ") -> " << ext->return_type.to_string() << "\n";
        }
    }

    void print_stmt(const Stmt& s, int depth) {
        if (auto* b = dynamic_cast<const BlockStmt*>(&s)) {
            indent(depth);
            std::cout << "{\n";
            for (auto& st : b->stmts) print_stmt(*st, depth + 1);
            indent(depth);
            std::cout << "}\n";
        } else if (auto* f = dynamic_cast<const ForgeStmt*>(&s)) {
            indent(depth);
            std::cout << "forge " << f->name;
            if (f->type_ann) std::cout << ": " << f->type_ann->to_string();
            if (f->initializer) {
                std::cout << " = ";
                print_expr(*f->initializer, 0);
            }
            std::cout << "\n";
        } else if (auto* i = dynamic_cast<const IfStmt*>(&s)) {
            indent(depth);
            std::cout << "if (";
            print_expr(*i->cond, 0);
            std::cout << ")\n";
            if (i->then_block) print_stmt(*i->then_block, depth + 1);
            if (i->else_block) {
                indent(depth);
                std::cout << "else\n";
                print_stmt(*i->else_block, depth + 1);
            }
        } else if (auto* c = dynamic_cast<const CycleStmt*>(&s)) {
            indent(depth);
            if (c->is_range) {
                std::cout << "cycle (" << c->range_var << " in ";
                print_expr(*c->range_expr, 0);
                std::cout << ")\n";
            } else {
                std::cout << "cycle (C-style)\n";
            }
            if (c->body) print_stmt(*c->body, depth + 1);
        } else if (auto* w = dynamic_cast<const WhileCycleStmt*>(&s)) {
            indent(depth);
            std::cout << "while (";
            print_expr(*w->cond, 0);
            std::cout << ")\n";
            if (w->body) print_stmt(*w->body, depth + 1);
        } else if (auto* r = dynamic_cast<const ReturnStmt*>(&s)) {
            indent(depth);
            std::cout << "return";
            if (r->value) {
                std::cout << " ";
                print_expr(*r->value, 0);
            }
            std::cout << "\n";
        } else if (dynamic_cast<const BreakStmt*>(&s)) {
            indent(depth);
            std::cout << "break\n";
        } else if (dynamic_cast<const ContinueStmt*>(&s)) {
            indent(depth);
            std::cout << "continue\n";
        } else if (auto* e = dynamic_cast<const ExprStmt*>(&s)) {
            indent(depth);
            print_expr(*e->expr, 0);
            std::cout << "\n";
        } else if (auto* fr = dynamic_cast<const FractureStmt*>(&s)) {
            indent(depth);
            std::cout << "fracture\n";
            if (fr->body) print_stmt(*fr->body, depth + 1);
        } else if (auto* ab = dynamic_cast<const AbyssStmt*>(&s)) {
            indent(depth);
            std::cout << "abyss\n";
            if (ab->body) print_stmt(*ab->body, depth + 1);
        }
    }

    void print_expr(const Expr& e, int depth) {
        if (auto* i = dynamic_cast<const IntLitExpr*>(&e)) {
            std::cout << i->value;
        } else if (auto* f = dynamic_cast<const FloatLitExpr*>(&e)) {
            std::cout << f->value;
        } else if (auto* s = dynamic_cast<const StringLitExpr*>(&e)) {
            std::cout << "\"" << s->value << "\"";
        } else if (auto* b = dynamic_cast<const BoolLitExpr*>(&e)) {
            std::cout << (b->value ? "true" : "false");
        } else if (dynamic_cast<const NullLitExpr*>(&e)) {
            std::cout << "null";
        } else if (auto* id = dynamic_cast<const IdentExpr*>(&e)) {
            std::cout << id->name;
        } else if (auto* bin = dynamic_cast<const BinaryExpr*>(&e)) {
            std::cout << "(";
            print_expr(*bin->left, 0);
            std::cout << " " << bin->op << " ";
            print_expr(*bin->right, 0);
            std::cout << ")";
        } else if (auto* un = dynamic_cast<const UnaryExpr*>(&e)) {
            if (un->prefix) {
                std::cout << un->op;
                print_expr(*un->operand, 0);
            } else {
                print_expr(*un->operand, 0);
                std::cout << un->op;
            }
        } else if (auto* a = dynamic_cast<const AssignExpr*>(&e)) {
            print_expr(*a->target, 0);
            std::cout << " " << a->op << " ";
            print_expr(*a->value, 0);
        } else if (auto* c = dynamic_cast<const CallExpr*>(&e)) {
            print_expr(*c->callee, 0);
            std::cout << "(";
            for (size_t i = 0; i < c->args.size(); i++) {
                if (i > 0) std::cout << ", ";
                print_expr(*c->args[i], 0);
            }
            std::cout << ")";
        } else if (auto* ec = dynamic_cast<const EchoExpr*>(&e)) {
            std::cout << "echo(";
            for (size_t i = 0; i < ec->args.size(); i++) {
                if (i > 0) std::cout << ", ";
                print_expr(*ec->args[i], 0);
            }
            std::cout << ")";
        } else if (auto* idx = dynamic_cast<const IndexExpr*>(&e)) {
            print_expr(*idx->object, 0);
            std::cout << "[";
            print_expr(*idx->index, 0);
            std::cout << "]";
        } else if (auto* mem = dynamic_cast<const MemberExpr*>(&e)) {
            print_expr(*mem->object, 0);
            std::cout << "." << mem->field;
        } else if (auto* arr = dynamic_cast<const ArrayLitExpr*>(&e)) {
            std::cout << "[";
            for (size_t i = 0; i < arr->elements.size(); i++) {
                if (i > 0) std::cout << ", ";
                print_expr(*arr->elements[i], 0);
            }
            std::cout << "]";
        } else if (auto* cast = dynamic_cast<const CastExpr*>(&e)) {
            std::cout << "(";
            print_expr(*cast->expr, 0);
            std::cout << " as " << cast->target_type.to_string() << ")";
        }
    }
};

inline void print_ast(const Module& mod) {
    ASTPrinter printer;
    printer.print(mod);
}

} // namespace ofs
