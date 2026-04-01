#include "semantic.hpp"
#include <algorithm>

namespace ofs {

// ── Main entry ────────────────────────────────────────────────────────────

void SemanticAnalyzer::analyze(Module& mod) {
    // First pass: register all monoliths and functions
    for (auto& d : mod.decls) {
        if (auto* m = dynamic_cast<MonolithDecl*>(d.get())) {
            monoliths_[m->name] = m;
            Symbol sym;
            sym.name = m->name;
            sym.type = OFSType::named(m->name);
            sym.kind = SymbolKind::Monolith;
            scope_.define(m->name, sym);
        } else if (auto* fn = dynamic_cast<FuncDecl*>(d.get())) {
            Symbol sym;
            sym.name = fn->name;
            sym.type = fn->return_type;
            sym.kind = SymbolKind::Function;
            scope_.define(fn->name, sym);
        } else if (auto* ext = dynamic_cast<ExternFuncDecl*>(d.get())) {
            Symbol sym;
            sym.name = ext->name;
            sym.type = ext->return_type;
            sym.kind = SymbolKind::Function;
            scope_.define(ext->name, sym);
        } else if (auto* st = dynamic_cast<StrataDecl*>(d.get())) {
            // Register strata type name
            Symbol sym;
            sym.name = st->name;
            sym.type = OFSType::named(st->name);
            sym.kind = SymbolKind::Monolith;
            scope_.define(st->name, sym);
        }
    }

    // Second pass: check everything
    for (auto& d : mod.decls) {
        if (auto* fn = dynamic_cast<FuncDecl*>(d.get())) {
            check_func(*fn);
        } else if (auto* m = dynamic_cast<MonolithDecl*>(d.get())) {
            check_monolith(*m);
        } else if (auto* g = dynamic_cast<GlobalForgeDecl*>(d.get())) {
            check_global_forge(*g);
        } else if (auto* ext = dynamic_cast<ExternFuncDecl*>(d.get())) {
            check_extern(*ext);
        } else if (auto* imp = dynamic_cast<ImportDecl*>(d.get())) {
            check_import(*imp);
        } else if (auto* st = dynamic_cast<StrataDecl*>(d.get())) {
            check_strata(*st);
        }
    }
}

// ── Declaration checks ────────────────────────────────────────────────────

void SemanticAnalyzer::check_func(FuncDecl& fn) {
    current_return_type_ = fn.return_type;
    scope_.push();

    // Define parameters
    for (auto& p : fn.params) {
        Symbol sym;
        sym.name = p.name;
        sym.type = p.type;
        sym.kind = SymbolKind::Param;
        scope_.define(p.name, sym);
    }

    if (fn.body) check_stmt(*fn.body);
    scope_.pop();
}

void SemanticAnalyzer::check_monolith(MonolithDecl& m) {
    // Check methods if any
    for (auto& method : m.methods) {
        check_func(*method);
    }
}

void SemanticAnalyzer::check_global_forge(GlobalForgeDecl& g) {
    OFSType type;
    if (g.initializer) {
        type = check_expr(*g.initializer);
    }
    if (g.type_ann) {
        type = *g.type_ann;
    } else if (g.initializer) {
        g.type_ann = type;
    }
    Symbol sym;
    sym.name = g.name;
    sym.type = type;
    sym.kind = SymbolKind::Variable;
    scope_.define(g.name, sym);
}

void SemanticAnalyzer::check_import(ImportDecl& i) {
    // Import resolution is handled at codegen time
    // Semantic check: just validate the path is non-empty
    if (i.path.empty()) {
        throw SemanticError("import path cannot be empty", i.line, i.col);
    }
}

void SemanticAnalyzer::check_extern(ExternFuncDecl& e) {
    // Extern functions are already registered in the first pass
    // Just validate parameter types are valid
    for (auto& p : e.params) {
        if (p.type.base == BaseType::Infer) {
            throw SemanticError("extern function parameter '" + p.name + "' must have explicit type", e.line, e.col);
        }
    }
}

void SemanticAnalyzer::check_strata(StrataDecl& s) {
    // Register each variant as a constant symbol of the enum (named) type.
    OFSType strata_type = OFSType::named(s.name);
    for (auto& variant : s.variants) {
        Symbol sym;
        sym.name = variant;
        sym.type = strata_type;
        sym.kind = SymbolKind::Variable;
        sym.is_const = true;
        sym.is_mut = false;
        scope_.define(variant, sym);
    }
}

// ── Statement checks ──────────────────────────────────────────────────────

void SemanticAnalyzer::check_stmt(Stmt& s) {
    if (auto* b = dynamic_cast<BlockStmt*>(&s)) {
        check_block(*b);
    } else if (auto* f = dynamic_cast<ForgeStmt*>(&s)) {
        check_forge(*f);
    } else if (auto* i = dynamic_cast<IfStmt*>(&s)) {
        check_if(*i);
    } else if (auto* c = dynamic_cast<CycleStmt*>(&s)) {
        check_cycle(*c);
    } else if (auto* r = dynamic_cast<ReturnStmt*>(&s)) {
        check_return(*r);
    } else if (auto* fr = dynamic_cast<FractureStmt*>(&s)) {
        check_fracture(*fr);
    } else if (auto* ab = dynamic_cast<AbyssStmt*>(&s)) {
        check_abyss(*ab);
    } else if (auto* w = dynamic_cast<WhileCycleStmt*>(&s)) {
        check_while(*w);
    } else if (auto* cs = dynamic_cast<ConstStmt*>(&s)) {
        check_const(*cs);
    } else if (auto* ms = dynamic_cast<MatchStmt*>(&s)) {
        check_match(*ms);
    } else if (auto* ts = dynamic_cast<TremorStmt*>(&s)) {
        check_tremor(*ts);
    } else if (auto* th = dynamic_cast<ThrowStmt*>(&s)) {
        check_throw(*th);
    } else if (auto* es = dynamic_cast<ExprStmt*>(&s)) {
        check_expr(*es->expr);
    } else if (dynamic_cast<BreakStmt*>(&s)) {
        if (!inside_cycle_) {
            throw SemanticError("'break' outside of cycle", s.line, s.col);
        }
    } else if (dynamic_cast<ContinueStmt*>(&s)) {
        if (!inside_cycle_) {
            throw SemanticError("'continue' outside of cycle", s.line, s.col);
        }
    }
}

void SemanticAnalyzer::check_block(BlockStmt& s) {
    scope_.push();
    for (auto& st : s.stmts) check_stmt(*st);
    scope_.pop();
}

void SemanticAnalyzer::check_forge(ForgeStmt& s) {
    OFSType type = OFSType::infer();

    if (s.initializer) {
        type = check_expr(*s.initializer);
    }

    if (s.type_ann) {
        if (s.initializer && type.base != BaseType::Infer) {
            if (!is_assignable(*s.type_ann, type)) {
                throw SemanticError(
                    "type mismatch: cannot assign " + type.to_string() +
                    " to " + s.type_ann->to_string(),
                    s.line, s.col);
            }
        }
        type = *s.type_ann;
    } else if (type.base == BaseType::Infer) {
        throw SemanticError("cannot infer type for '" + s.name + "' without initializer or type annotation",
                           s.line, s.col);
    } else {
        s.type_ann = type;
    }

    Symbol sym;
    sym.name = s.name;
    sym.type = type;
    sym.kind = SymbolKind::Variable;
    scope_.define(s.name, sym);
}

void SemanticAnalyzer::check_const(ConstStmt& s) {
    if (!s.initializer) {
        throw SemanticError("const declaration requires initializer", s.line, s.col);
    }

    OFSType init_type = check_expr(*s.initializer);
    OFSType type = init_type;

    if (s.type_ann) {
        if (!is_assignable(*s.type_ann, init_type)) {
            throw SemanticError(
                "type mismatch: cannot assign " + init_type.to_string() +
                " to " + s.type_ann->to_string(),
                s.line, s.col);
        }
        type = *s.type_ann;
    } else {
        s.type_ann = init_type;
    }

    Symbol sym;
    sym.name = s.name;
    sym.type = type;
    sym.kind = SymbolKind::Variable;
    sym.is_const = true;
    sym.is_mut = false;
    scope_.define(s.name, sym);
}

void SemanticAnalyzer::check_if(IfStmt& s) {
    OFSType cond_type = check_expr(*s.cond);
    // Condition should be bool or numeric (truthy)
    if (!cond_type.is_numeric() && cond_type.base != BaseType::Bool) {
        throw SemanticError("if condition must be bool or numeric", s.line, s.col);
    }
    if (s.then_block) check_stmt(*s.then_block);
    if (s.else_block) check_stmt(*s.else_block);
}

void SemanticAnalyzer::check_cycle(CycleStmt& s) {
    bool prev_cycle = inside_cycle_;
    inside_cycle_ = true;
    scope_.push();

    if (s.is_range) {
        OFSType range_type = check_expr(*s.range_expr);
        // Range expression should be an array
        OFSType elem_type = OFSType::infer();
        if (range_type.base == BaseType::Array && range_type.inner) {
            elem_type = *range_type.inner;
        } else {
            elem_type = OFSType::stone(); // default for unknown ranges
        }
        Symbol sym;
        sym.name = s.range_var;
        sym.type = elem_type;
        sym.kind = SymbolKind::Variable;
        scope_.define(s.range_var, sym);
    } else {
        if (s.init) check_stmt(*s.init);
        if (s.cond) check_expr(*s.cond);
        if (s.step) check_expr(*s.step);
    }

    if (s.body) check_stmt(*s.body);
    scope_.pop();
    inside_cycle_ = prev_cycle;
}

void SemanticAnalyzer::check_return(ReturnStmt& s) {
    if (s.value) {
        OFSType ret_type = check_expr(*s.value);
        if (current_return_type_.base != BaseType::Void &&
            current_return_type_.base != BaseType::Infer) {
            if (!is_assignable(current_return_type_, ret_type)) {
                throw SemanticError(
                    "return type mismatch: expected " + current_return_type_.to_string() +
                    " but got " + ret_type.to_string(),
                    s.line, s.col);
            }
        }
    }
}

void SemanticAnalyzer::check_fracture(FractureStmt& s) {
    bool prev = inside_fracture_;
    inside_fracture_ = true;
    if (s.body) check_stmt(*s.body);
    inside_fracture_ = prev;
}

void SemanticAnalyzer::check_abyss(AbyssStmt& s) {
    bool prev = inside_abyss_;
    inside_abyss_ = true;
    // Inside abyss, skip type checking (unsafe block)
    if (s.body) check_stmt(*s.body);
    inside_abyss_ = prev;
}

void SemanticAnalyzer::check_while(WhileCycleStmt& s) {
    bool prev_cycle = inside_cycle_;
    inside_cycle_ = true;

    OFSType cond_type = check_expr(*s.cond);
    if (!cond_type.is_numeric() && cond_type.base != BaseType::Bool) {
        throw SemanticError("while condition must be bool or numeric", s.line, s.col);
    }

    if (s.body) check_stmt(*s.body);
    inside_cycle_ = prev_cycle;
}

void SemanticAnalyzer::check_match(MatchStmt& s) {
    OFSType subject = check_expr(*s.subject);
    for (auto& arm : s.arms) {
        if (!arm.is_default && arm.pattern) {
            OFSType pattern = check_expr(*arm.pattern);
            if (!is_assignable(subject, pattern) && !is_assignable(pattern, subject)) {
                throw SemanticError("match arm pattern type mismatch", s.line, s.col);
            }
        }
        if (arm.body) check_stmt(*arm.body);
    }
}

void SemanticAnalyzer::check_tremor(TremorStmt& s) {
    if (s.body) check_stmt(*s.body);

    scope_.push();
    Symbol catch_sym;
    catch_sym.name = s.catch_var;
    catch_sym.type = (s.catch_type.base == BaseType::Infer)
        ? OFSType::obsidian()
        : s.catch_type;
    catch_sym.kind = SymbolKind::Variable;
    scope_.define(catch_sym.name, catch_sym);
    if (s.catch_body) check_stmt(*s.catch_body);
    scope_.pop();
}

void SemanticAnalyzer::check_throw(ThrowStmt& s) {
    if (s.value) {
        check_expr(*s.value);
    }
}

// ── Expression checks ─────────────────────────────────────────────────────

OFSType SemanticAnalyzer::check_expr(Expr& e) {
    OFSType result;

    if (auto* lit = dynamic_cast<IntLitExpr*>(&e)) {
        result = OFSType::stone();
    } else if (auto* lit = dynamic_cast<FloatLitExpr*>(&e)) {
        result = OFSType::crystal();
    } else if (auto* lit = dynamic_cast<StringLitExpr*>(&e)) {
        result = OFSType::obsidian();
    } else if (auto* lit = dynamic_cast<BoolLitExpr*>(&e)) {
        result = OFSType::boolean();
    } else if (dynamic_cast<NullLitExpr*>(&e)) {
        result = OFSType::void_t();
    } else if (auto* id = dynamic_cast<IdentExpr*>(&e)) {
        auto* sym = scope_.lookup(id->name);
        if (!sym) {
            throw SemanticError("undefined variable '" + id->name + "'", e.line, e.col);
        }
        result = sym->type;
    } else if (auto* bin = dynamic_cast<BinaryExpr*>(&e)) {
        result = check_binary(*bin);
    } else if (auto* un = dynamic_cast<UnaryExpr*>(&e)) {
        result = check_unary(*un);
    } else if (auto* call = dynamic_cast<CallExpr*>(&e)) {
        result = check_call(*call);
    } else if (auto* echo = dynamic_cast<EchoExpr*>(&e)) {
        result = check_echo(*echo);
    } else if (auto* assign = dynamic_cast<AssignExpr*>(&e)) {
        result = check_assign(*assign);
    } else if (auto* idx = dynamic_cast<IndexExpr*>(&e)) {
        result = check_index(*idx);
    } else if (auto* mem = dynamic_cast<MemberExpr*>(&e)) {
        result = check_member(*mem);
    } else if (auto* arr = dynamic_cast<ArrayLitExpr*>(&e)) {
        result = check_array_lit(*arr);
    } else if (auto* cast = dynamic_cast<CastExpr*>(&e)) {
        result = check_cast(*cast);
    } else {
        result = OFSType::void_t();
    }

    e.resolved_type = result;
    return result;
}

OFSType SemanticAnalyzer::check_binary(BinaryExpr& e) {
    OFSType left = check_expr(*e.left);
    OFSType right = check_expr(*e.right);

    // String concatenation
    if (e.op == "+" && left.base == BaseType::Obsidian && right.base == BaseType::Obsidian) {
        return OFSType::obsidian();
    }

    // Comparison operators
    if (e.op == "==" || e.op == "!=") {
        return OFSType::boolean();
    }

    // Relational operators
    if (e.op == "<" || e.op == "<=" || e.op == ">" || e.op == ">=") {
        if (!is_numeric(left) || !is_numeric(right)) {
            throw SemanticError("relational operator requires numeric operands", e.line, e.col);
        }
        return OFSType::boolean();
    }

    // Logical operators
    if (e.op == "&&" || e.op == "||") {
        return OFSType::boolean();
    }

    // Bitwise operators and shifts (stone only)
    if (e.op == "&" || e.op == "|" || e.op == "^" || e.op == "<<" || e.op == ">>") {
        if (left.base != BaseType::Stone || right.base != BaseType::Stone) {
            throw SemanticError("bitwise operators require stone operands", e.line, e.col);
        }
        return OFSType::stone();
    }

    // Arithmetic operators
    if (e.op == "+" || e.op == "-" || e.op == "*" || e.op == "/" || e.op == "%") {
        if (!is_numeric(left) || !is_numeric(right)) {
            throw SemanticError("arithmetic operator requires numeric operands", e.line, e.col);
        }
        return promote(left, right);
    }

    return OFSType::void_t();
}

OFSType SemanticAnalyzer::check_unary(UnaryExpr& e) {
    OFSType operand = check_expr(*e.operand);

    if (e.op == "-") {
        if (!is_numeric(operand)) {
            throw SemanticError("unary '-' requires numeric operand", e.line, e.col);
        }
        return operand;
    }
    if (e.op == "!") {
        return OFSType::boolean();
    }
    if (e.op == "&") {
        // Address-of: only in fracture/abyss
        if (!inside_fracture_ && !inside_abyss_) {
            throw SemanticError("address-of (&) operator only allowed in fracture or abyss block", e.line, e.col);
        }
        return OFSType::shard_of(operand);
    }
    if (e.op == "*") {
        // Dereference
        if (!inside_fracture_ && !inside_abyss_) {
            throw SemanticError("dereference (*) operator only allowed in fracture or abyss block", e.line, e.col);
        }
        if (operand.base == BaseType::Shard && operand.inner) {
            return *operand.inner;
        }
        throw SemanticError("cannot dereference non-pointer type", e.line, e.col);
    }
    if (e.op == "++" || e.op == "--") {
        if (!is_numeric(operand)) {
            throw SemanticError("increment/decrement requires numeric operand", e.line, e.col);
        }
        return operand;
    }
    if (e.op == "~") {
        return operand;
    }

    return operand;
}

OFSType SemanticAnalyzer::check_call(CallExpr& e) {
    OFSType callee_type = check_expr(*e.callee);
    for (auto& arg : e.args) check_expr(*arg);

    // Look up function
    if (auto* id = dynamic_cast<IdentExpr*>(e.callee.get())) {
        auto* sym = scope_.lookup(id->name);
        if (sym && sym->kind == SymbolKind::Function) {
            return sym->type;
        }
        // Monolith constructor
        if (sym && sym->kind == SymbolKind::Monolith) {
            return OFSType::named(id->name);
        }
    }

    return callee_type;
}

OFSType SemanticAnalyzer::check_echo(EchoExpr& e) {
    for (auto& arg : e.args) check_expr(*arg);
    return OFSType::void_t();
}

OFSType SemanticAnalyzer::check_assign(AssignExpr& e) {
    if (auto* id = dynamic_cast<IdentExpr*>(e.target.get())) {
        if (auto* sym = scope_.lookup(id->name)) {
            if (sym->is_const) {
                throw SemanticError("cannot assign to const variable '" + id->name + "'", e.line, e.col);
            }
        }
    }

    OFSType target = check_expr(*e.target);
    OFSType value = check_expr(*e.value);

    if (e.op == "=") {
        if (!is_assignable(target, value)) {
            throw SemanticError(
                "cannot assign " + value.to_string() + " to " + target.to_string(),
                e.line, e.col);
        }
    } else {
        // Compound assignment (+=, -=, *=, /=, %=)
        if (!is_numeric(target) && !(target.base == BaseType::Obsidian && e.op == "+=")) {
            throw SemanticError("compound assignment requires numeric target", e.line, e.col);
        }
    }

    return target;
}

OFSType SemanticAnalyzer::check_index(IndexExpr& e) {
    OFSType obj = check_expr(*e.object);
    OFSType idx = check_expr(*e.index);

    if (obj.base == BaseType::Array && obj.inner) {
        return *obj.inner;
    }
    // Allow indexing on any type in abyss mode
    if (inside_abyss_) return OFSType::stone();

    // Default for unknown array types
    return OFSType::stone();
}

OFSType SemanticAnalyzer::check_member(MemberExpr& e) {
    OFSType obj = check_expr(*e.object);

    if (obj.base == BaseType::Named) {
        auto it = monoliths_.find(obj.name);
        if (it != monoliths_.end()) {
            for (auto& f : it->second->fields) {
                if (f.name == e.field) return f.type;
            }
            throw SemanticError("monolith '" + obj.name + "' has no field '" + e.field + "'",
                               e.line, e.col);
        }
    }

    // For unknown types, return infer
    return OFSType::infer();
}

OFSType SemanticAnalyzer::check_array_lit(ArrayLitExpr& e) {
    if (e.elements.empty()) return OFSType::array_of(OFSType::stone());

    OFSType elem_type = check_expr(*e.elements[0]);
    for (size_t i = 1; i < e.elements.size(); i++) {
        OFSType t = check_expr(*e.elements[i]);
        if (is_numeric(elem_type) && is_numeric(t)) {
            elem_type = promote(elem_type, t);
        }
    }

    return OFSType::array_of(elem_type);
}

OFSType SemanticAnalyzer::check_cast(CastExpr& e) {
    OFSType from = check_expr(*e.expr);
    OFSType to = e.target_type;

    // Allow same-type cast (no-op)
    if (from == to) return to;

    // Allowed casts:
    // - numeric -> numeric (stone <-> crystal)
    // - bool -> numeric
    // - numeric -> bool
    if ((from.is_numeric() || from.base == BaseType::Bool) &&
        (to.is_numeric() || to.base == BaseType::Bool)) {
        return to;
    }

    // Allow any cast in abyss mode
    if (inside_abyss_) return to;

    throw SemanticError("cannot cast " + from.to_string() + " to " + to.to_string(), e.line, e.col);
}

// ── Type helpers ──────────────────────────────────────────────────────────

OFSType SemanticAnalyzer::infer_type(const Expr& init) {
    // This is handled by check_expr which fills resolved_type
    return init.resolved_type;
}

bool SemanticAnalyzer::is_assignable(const OFSType& to, const OFSType& from) const {
    if (to == from) return true;
    // Stone can be widened to Crystal
    if (to.base == BaseType::Crystal && from.base == BaseType::Stone) return true;
    // Infer is always compatible
    if (to.base == BaseType::Infer || from.base == BaseType::Infer) return true;
    return false;
}

bool SemanticAnalyzer::is_numeric(const OFSType& t) const {
    return t.base == BaseType::Stone || t.base == BaseType::Crystal;
}

OFSType SemanticAnalyzer::promote(const OFSType& a, const OFSType& b) const {
    if (a.base == BaseType::Crystal || b.base == BaseType::Crystal) {
        return OFSType::crystal();
    }
    return OFSType::stone();
}

void SemanticAnalyzer::expect_type(const OFSType& got, const OFSType& want, int line, int col) {
    if (!is_assignable(want, got)) {
        throw SemanticError(
            "expected " + want.to_string() + " but got " + got.to_string(),
            line, col);
    }
}

} // namespace ofs
