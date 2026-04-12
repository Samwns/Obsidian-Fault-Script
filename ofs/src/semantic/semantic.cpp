#include "semantic.hpp"
#include <algorithm>
#include "../i18n.hpp"

namespace ofs {

namespace {

bool is_fault_intrinsic_name(const std::string& name) {
    return name == "fault_count" ||
           name == "fault_fence" ||
           name == "fault_prefetch" ||
           name == "fault_trap" ||
           name == "fault_lead" ||
           name == "fault_trail" ||
           name == "fault_swap" ||
           name == "fault_spin_left" ||
           name == "fault_spin_right" ||
           name == "fault_step" ||
           name == "fault_cut" ||
           name == "fault_patch" ||
           name == "fault_weave" ||
           name == "fault_unreachable" ||
           name == "fault_memcpy" ||
           name == "fault_memset";
}

static std::string join_namespace(const std::vector<std::string>& ns_stack) {
    if (ns_stack.empty()) return "";
    std::string out;
    for (size_t i = 0; i < ns_stack.size(); i++) {
        if (i > 0) out += "::";
        out += ns_stack[i];
    }
    return out;
}

static std::string qualify_name(const std::vector<std::string>& ns_stack, const std::string& name) {
    std::string ns = join_namespace(ns_stack);
    if (ns.empty()) return name;
    return ns + "::" + name;
}

bool is_low_level_scope(bool inside_fracture, bool inside_abyss, bool inside_bedrock) {
    return inside_fracture || inside_abyss || inside_bedrock;
}

}

// ── Main entry ────────────────────────────────────────────────────────────

void SemanticAnalyzer::analyze(Module& mod) {
    register_builtin_symbols();

    auto register_decl = [&](auto&& self, Decl& d) -> void {
        if (auto* m = dynamic_cast<MonolithDecl*>(&d)) {
            monoliths_[m->name] = m;
            Symbol sym;
            sym.name = m->name;
            sym.type = OFSType::named(m->name);
            sym.kind = SymbolKind::Monolith;
            scope_.define(m->name, sym);
            return;
        }

        if (auto* fn = dynamic_cast<FuncDecl*>(&d)) {
            Symbol sym;
            sym.name = qualify_name(namespace_stack_, fn->name);
            sym.type = function_signature_type(fn->params, fn->return_type);
            sym.kind = SymbolKind::Function;
            scope_.define(sym.name, sym);
            scope_.define(fn->name, sym);
            function_intents_[sym.name] = fn->intent;
            function_types_[sym.name] = sym.type;
            return;
        }

        if (auto* ext = dynamic_cast<ExternFuncDecl*>(&d)) {
            Symbol sym;
            sym.name = qualify_name(namespace_stack_, ext->name);
            sym.type = function_signature_type(ext->params, ext->return_type);
            sym.kind = SymbolKind::Function;
            scope_.define(sym.name, sym);
            scope_.define(ext->name, sym);
            function_types_[sym.name] = sym.type;
            return;
        }

        if (auto* st = dynamic_cast<StrataDecl*>(&d)) {
            Symbol sym;
            sym.name = st->name;
            sym.type = OFSType::named(st->name);
            sym.kind = SymbolKind::Monolith;
            scope_.define(st->name, sym);
            return;
        }

        if (auto* impl = dynamic_cast<ImplDecl*>(&d)) {
            for (auto& method : impl->methods) {
                std::string mangled = impl->target_name + "_" + method->name;
                OFSType sig = function_signature_type(method->params, method->return_type);
                impl_method_types_[impl->target_name + "::" + method->name] = sig;
                impl_method_owner_[impl->target_name + "::" + method->name] = impl->target_name;

                Symbol sym;
                sym.name = mangled;
                sym.type = sig;
                sym.kind = SymbolKind::Function;
                scope_.define(mangled, sym);
                function_types_[mangled] = sig;
            }
            return;
        }

        if (auto* ns = dynamic_cast<NamespaceDecl*>(&d)) {
            namespace_stack_.push_back(ns->name);
            for (auto& inner : ns->declarations) {
                self(self, *inner);
            }
            namespace_stack_.pop_back();
            return;
        }
    };

    // First pass: register all monoliths and functions
    for (auto& d : mod.decls) {
        register_decl(register_decl, *d);
    }

    namespace_stack_.clear();

    // Second pass: check everything
    for (auto& d : mod.decls) {
        if (auto* fn = dynamic_cast<FuncDecl*>(d.get())) {
            check_func(*fn);
        } else if (auto* m = dynamic_cast<MonolithDecl*>(d.get())) {
            check_monolith(*m);
        } else if (auto* i = dynamic_cast<ImplDecl*>(d.get())) {
            check_impl(*i);
        } else if (auto* n = dynamic_cast<NamespaceDecl*>(d.get())) {
            check_namespace(*n);
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

void SemanticAnalyzer::register_builtin_symbols() {
    struct BuiltinDef {
        const char* name;
        std::vector<OFSType> params;
        OFSType ret;
    };
    BuiltinDef builtins[] = {
        {"fault_count",      {OFSType::stone()}, OFSType::stone()},
        {"fault_lead",       {OFSType::stone()}, OFSType::stone()},
        {"fault_trail",      {OFSType::stone()}, OFSType::stone()},
        {"fault_swap",       {OFSType::stone()}, OFSType::stone()},
        {"fault_spin_left",  {OFSType::stone(), OFSType::stone()}, OFSType::stone()},
        {"fault_spin_right", {OFSType::stone(), OFSType::stone()}, OFSType::stone()},
        {"fault_cut",        {OFSType::stone(), OFSType::stone(), OFSType::stone()}, OFSType::stone()},
        {"fault_patch",      {OFSType::stone(), OFSType::stone(), OFSType::stone(), OFSType::stone()}, OFSType::stone()},
        {"fault_weave",      {OFSType::stone(), OFSType::stone(), OFSType::stone()}, OFSType::stone()},
        {"fault_fence",       {}, OFSType::void_t()},
        {"fault_prefetch",    {OFSType::shard_of(OFSType::stone())}, OFSType::void_t()},
        {"fault_trap",        {}, OFSType::void_t()},
        {"fault_unreachable", {}, OFSType::void_t()},
        {"fault_memcpy",      {OFSType::shard_of(OFSType::stone()), OFSType::shard_of(OFSType::stone()), OFSType::stone()}, OFSType::void_t()},
        {"fault_memset",      {OFSType::shard_of(OFSType::stone()), OFSType::stone(), OFSType::stone()}, OFSType::void_t()},
        {"fault_step",        {OFSType::shard_of(OFSType::stone()), OFSType::stone()}, OFSType::shard_of(OFSType::stone())},
    };

    for (auto& b : builtins) {
        Symbol sym;
        sym.name = b.name;
        sym.type = OFSType::function_of(b.params, b.ret);
        sym.kind = SymbolKind::Function;
        scope_.define(sym.name, sym);
        function_types_[sym.name] = sym.type;
    }
}

OFSType SemanticAnalyzer::function_signature_type(const std::vector<Param>& params, const OFSType& ret) const {
    std::vector<OFSType> param_types;
    for (const auto& p : params) param_types.push_back(p.type);
    return OFSType::function_of(std::move(param_types), ret);
}

// ── Declaration checks ────────────────────────────────────────────────────

void SemanticAnalyzer::check_func(FuncDecl& fn) {
    FuncIntent prev_intent = current_intent_;
    current_return_type_ = fn.return_type;
    current_intent_ = fn.intent;
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
    current_intent_ = prev_intent;
}

void SemanticAnalyzer::check_monolith(MonolithDecl& m) {
    if (m.layout != "native" && m.layout != "packed" && m.layout != "c") {
        throw SemanticError(
            OFS_MSG("unsupported monolith layout '" + m.layout + "' (supported: native, packed, c)",
                    "layout de monolith não suportado '" + m.layout + "' (suportados: native, packed, c)"),
            m.line, m.col);
    }

    // Check methods if any
    for (auto& method : m.methods) {
        check_func(*method);
    }
}

void SemanticAnalyzer::check_impl(ImplDecl& i) {
    auto mono_it = monoliths_.find(i.target_name);
    if (mono_it == monoliths_.end()) {
        throw SemanticError(
            OFS_MSG("impl target '" + i.target_name + "' not found",
                    "alvo de impl '" + i.target_name + "' não encontrado"),
            i.line, i.col);
    }

    for (auto& method : i.methods) {
        if (method->params.empty() || method->params[0].name != "self") {
            Param self;
            self.name = "self";
            self.type = OFSType::named(i.target_name);
            method->params.insert(method->params.begin(), std::move(self));
        }
        if (method->params[0].type.base == BaseType::Infer) {
            method->params[0].type = OFSType::named(i.target_name);
        }
        check_func(*method);
    }
}

void SemanticAnalyzer::check_namespace(NamespaceDecl& n) {
    namespace_stack_.push_back(n.name);
    for (auto& d : n.declarations) {
        if (auto* fn = dynamic_cast<FuncDecl*>(d.get())) {
            check_func(*fn);
        } else if (auto* m = dynamic_cast<MonolithDecl*>(d.get())) {
            check_monolith(*m);
        } else if (auto* i = dynamic_cast<ImplDecl*>(d.get())) {
            check_impl(*i);
        } else if (auto* g = dynamic_cast<GlobalForgeDecl*>(d.get())) {
            check_global_forge(*g);
        } else if (auto* e = dynamic_cast<ExternFuncDecl*>(d.get())) {
            check_extern(*e);
        } else if (auto* s = dynamic_cast<StrataDecl*>(d.get())) {
            check_strata(*s);
        } else if (auto* child = dynamic_cast<NamespaceDecl*>(d.get())) {
            check_namespace(*child);
        }
    }
    namespace_stack_.pop_back();
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
    // Attach resolution is handled at codegen time.
    // Semantic check: just validate the path is non-empty.
    if (i.path.empty()) {
        throw SemanticError(OFS_MSG("attach path cannot be empty", "caminho do attach não pode ser vazio"), i.line, i.col);
    }
}

void SemanticAnalyzer::check_extern(ExternFuncDecl& e) {
    // Extern functions are already registered in the first pass
    // Just validate parameter types are valid
    for (auto& p : e.params) {
        if (p.type.base == BaseType::Infer) {
            throw SemanticError(OFS_MSG(
                "extern function parameter '" + p.name + "' must have explicit type",
                "parâmetro '" + p.name + "' da função extern deve ter tipo explícito"),
                e.line, e.col);
        }
    }

    if (!e.abi.empty() && e.abi != "c" && e.abi != "system") {
        throw SemanticError(
            OFS_MSG("unsupported ABI '" + e.abi + "' (supported: c, system)",
                    "ABI não suportada '" + e.abi + "' (suportadas: c, system)"),
            e.line, e.col);
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
    } else if (auto* fb = dynamic_cast<FractalStmt*>(&s)) {
        check_fractal(*fb);
    } else if (auto* bb = dynamic_cast<BedrockStmt*>(&s)) {
        check_bedrock(*bb);
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
            throw SemanticError(OFS_MSG("'break' outside of cycle", "'break' fora de um cycle"), s.line, s.col);
        }
    } else if (dynamic_cast<ContinueStmt*>(&s)) {
        if (!inside_cycle_) {
            throw SemanticError(OFS_MSG("'continue' outside of cycle", "'continue' fora de um cycle"), s.line, s.col);
        }
    }
}

void SemanticAnalyzer::predeclare_block_symbols(BlockStmt& s) {
    for (auto& st : s.stmts) {
        if (auto* f = dynamic_cast<ForgeStmt*>(st.get())) {
            Symbol sym;
            sym.name = f->name;
            sym.type = f->type_ann.value_or(OFSType::infer());
            sym.kind = SymbolKind::Variable;
            scope_.define(f->name, sym);
        } else if (auto* c = dynamic_cast<ConstStmt*>(st.get())) {
            Symbol sym;
            sym.name = c->name;
            sym.type = c->type_ann.value_or(OFSType::infer());
            sym.kind = SymbolKind::Variable;
            sym.is_const = true;
            sym.is_mut = false;
            scope_.define(c->name, sym);
        }
    }
}

void SemanticAnalyzer::check_block(BlockStmt& s) {
    scope_.push();
    predeclare_block_symbols(s);
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
            if (!(inside_abyss_ || inside_bedrock_) && !is_assignable(*s.type_ann, type)) {
                throw SemanticError(
                    OFS_MSG("type mismatch: cannot assign " + type.to_string() +
                            " to " + s.type_ann->to_string(),
                            "incompatibilidade de tipos: não é possível atribuir " + type.to_string() +
                            " a " + s.type_ann->to_string()),
                    s.line, s.col);
            }
        }
        type = *s.type_ann;
    } else if (type.base == BaseType::Infer) {
        throw SemanticError(OFS_MSG(
            "cannot infer type for '" + s.name + "' without initializer or type annotation",
            "não foi possível inferir o tipo de '" + s.name + "' sem inicializador ou anotação de tipo"),
            s.line, s.col);
    } else {
        s.type_ann = type;
    }

    Symbol sym;
    sym.name = s.name;
    sym.type = type;
    sym.kind = SymbolKind::Variable;
    if (auto* existing = scope_.lookup(s.name)) {
        sym.is_const = existing->is_const;
        sym.is_mut = existing->is_mut;
    }
    scope_.define(s.name, sym);
}

void SemanticAnalyzer::check_const(ConstStmt& s) {
    if (!s.initializer) {
        throw SemanticError(OFS_MSG("const declaration requires initializer", "declaração const requer inicializador"), s.line, s.col);
    }

    OFSType init_type = check_expr(*s.initializer);
    OFSType type = init_type;

    if (s.type_ann) {
        if (!is_assignable(*s.type_ann, init_type)) {
            throw SemanticError(
                OFS_MSG("type mismatch: cannot assign " + init_type.to_string() +
                        " to " + s.type_ann->to_string(),
                        "incompatibilidade de tipos: não é possível atribuir " + init_type.to_string() +
                        " a " + s.type_ann->to_string()),
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
        throw SemanticError(OFS_MSG("if condition must be bool or numeric", "condição do if deve ser bool ou numérico"), s.line, s.col);
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
        // Returning a value from a void function is an error
        if (current_return_type_.base == BaseType::Void) {
            throw SemanticError(OFS_MSG(
                "void function cannot return a value",
                "função void não pode retornar um valor"),
                s.line, s.col);
        }
        OFSType ret_type = check_expr(*s.value);
        if (current_return_type_.base != BaseType::Infer) {
            if (!is_assignable(current_return_type_, ret_type)) {
                throw SemanticError(
                    OFS_MSG("return type mismatch: expected " + current_return_type_.to_string() +
                            " but got " + ret_type.to_string(),
                            "incompatibilidade de tipo de retorno: esperado " + current_return_type_.to_string() +
                            " mas encontrado " + ret_type.to_string()),
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

void SemanticAnalyzer::check_fractal(FractalStmt& s) {
    bool prev_fractal = inside_fractal_;
    bool prev_abyss = inside_abyss_;
    inside_fractal_ = true;
    inside_abyss_ = true; // fractal block lifts unsafe restrictions
    if (s.body) check_stmt(*s.body);
    inside_abyss_ = prev_abyss;
    inside_fractal_ = prev_fractal;
}

void SemanticAnalyzer::check_bedrock(BedrockStmt& s) {
    bool prev_bedrock = inside_bedrock_;
    bool prev_fracture = inside_fracture_;
    bool prev_fractal = inside_fractal_;
    inside_bedrock_ = true;
    inside_fracture_ = true;
    inside_fractal_ = true;
    if (s.body) check_stmt(*s.body);
    inside_fractal_ = prev_fractal;
    inside_fracture_ = prev_fracture;
    inside_bedrock_ = prev_bedrock;
}

void SemanticAnalyzer::check_while(WhileCycleStmt& s) {
    bool prev_cycle = inside_cycle_;
    inside_cycle_ = true;

    OFSType cond_type = check_expr(*s.cond);
    if (!cond_type.is_numeric() && cond_type.base != BaseType::Bool) {
        throw SemanticError(OFS_MSG("while condition must be bool or numeric", "condição do while deve ser bool ou numérico"), s.line, s.col);
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
                throw SemanticError(OFS_MSG("match arm pattern type mismatch", "incompatibilidade de tipo no padrão do match"), s.line, s.col);
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
            std::string qualified = qualify_name(namespace_stack_, id->name);
            sym = scope_.lookup(qualified);
        }
        if (!sym) {
            throw SemanticError(OFS_MSG("undefined variable '" + id->name + "'",
                                        "variável '" + id->name + "' não definida"),
                                e.line, e.col);
        }
        if (sym->kind == SymbolKind::Function && sym->type.base == BaseType::Function && sym->type.fn_return) {
            result = sym->type;
        } else {
            result = sym->type;
        }
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
    } else if (auto* lambda = dynamic_cast<LambdaExpr*>(&e)) {
        result = check_lambda(*lambda);
    } else if (auto* ia = dynamic_cast<InlineAsmExpr*>(&e)) {
        // Inline assembly: check input expressions, returns void
        for (auto& inp : ia->inputs) check_expr(*inp);
        result = OFSType::void_t();
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
            throw SemanticError(OFS_MSG("relational operator requires numeric operands",
                                        "operador relacional requer operandos numéricos"),
                                e.line, e.col);
        }
        return OFSType::boolean();
    }

    // Logical operators
    if (e.op == "&&" || e.op == "||") {
        return OFSType::boolean();
    }

    // Bitwise operators and shifts (integral only)
    if (e.op == "&" || e.op == "|" || e.op == "^") {
        if (!is_integral(left) || !is_integral(right)) {
            throw SemanticError(OFS_MSG("bitwise operators require integral operands",
                                        "operadores bit a bit requerem operandos inteiros"),
                                e.line, e.col);
        }
        return promote(left, right);
    }

    if (e.op == "<<" || e.op == ">>") {
        if (!is_integral(left) || !is_integral(right)) {
            throw SemanticError(OFS_MSG("shift operators require integral operands",
                                        "operadores de deslocamento requerem operandos inteiros"),
                                e.line, e.col);
        }
        return left;
    }

    // Arithmetic operators
    if (e.op == "+" || e.op == "-" || e.op == "*" || e.op == "/" || e.op == "%") {
        if (!is_numeric(left) || !is_numeric(right)) {
            if (inside_abyss_ || inside_bedrock_) {
                return OFSType::stone();
            }
            throw SemanticError(OFS_MSG("arithmetic operator requires numeric operands",
                                        "operador aritmético requer operandos numéricos"),
                                e.line, e.col);
        }
        return promote(left, right);
    }

    return OFSType::void_t();
}

OFSType SemanticAnalyzer::check_unary(UnaryExpr& e) {
    OFSType operand = check_expr(*e.operand);

    if (e.op == "-") {
        if (!is_numeric(operand)) {
            throw SemanticError(OFS_MSG("unary '-' requires numeric operand",
                                        "operador unário '-' requer operando numérico"),
                                e.line, e.col);
        }
        return operand;
    }
    if (e.op == "!") {
        return OFSType::boolean();
    }
    if (e.op == "&") {
        // Address-of: only in fracture/abyss
        if (!inside_fracture_ && !inside_abyss_ && !inside_bedrock_) {
            throw SemanticError(OFS_MSG("address-of (&) operator only allowed in fracture or abyss block",
                                        "operador de endereço (&) só é permitido em blocos fracture ou abyss"),
                                e.line, e.col);
        }
        return OFSType::shard_of(operand);
    }
    if (e.op == "*") {
        // Dereference
        if (!inside_fracture_ && !inside_abyss_ && !inside_bedrock_) {
            throw SemanticError(OFS_MSG("dereference (*) operator only allowed in fracture or abyss block",
                                        "operador de desreferência (*) só é permitido em blocos fracture ou abyss"),
                                e.line, e.col);
        }
        if (operand.base == BaseType::Shard && operand.inner) {
            return *operand.inner;
        }
        throw SemanticError(OFS_MSG("cannot dereference non-pointer type",
                                    "não é possível desreferenciar um tipo não-ponteiro"),
                            e.line, e.col);
    }
    if (e.op == "++" || e.op == "--") {
        if (!is_numeric(operand)) {
            throw SemanticError(OFS_MSG("increment/decrement requires numeric operand",
                                        "incremento/decremento requer operando numérico"),
                                e.line, e.col);
        }
        return operand;
    }
    if (e.op == "~") {
        return operand;
    }

    return operand;
}

OFSType SemanticAnalyzer::check_call(CallExpr& e) {
    OFSType callee_type = OFSType::infer();
    bool callee_is_member = dynamic_cast<MemberExpr*>(e.callee.get()) != nullptr;
    auto* callee_ident = dynamic_cast<IdentExpr*>(e.callee.get());
    if (!callee_is_member) {
        callee_type = check_expr(*e.callee);
    }
    for (auto& arg : e.args) check_expr(*arg);

    // Function variable/pointer call
    bool ident_is_declared_function = false;
    if (callee_ident) {
        if (auto* sym = scope_.lookup(callee_ident->name)) {
            ident_is_declared_function = (sym->kind == SymbolKind::Function);
        }
    }

    if (callee_type.base == BaseType::Function &&
        !(callee_ident && is_fault_intrinsic_name(callee_ident->name)) &&
        (!callee_ident || !ident_is_declared_function)) {
        if (callee_type.fn_params.size() != e.args.size()) {
            throw SemanticError(
                OFS_MSG("function expected " + std::to_string(callee_type.fn_params.size()) +
                        " arguments but got " + std::to_string(e.args.size()),
                        "função esperava " + std::to_string(callee_type.fn_params.size()) +
                        " argumentos mas recebeu " + std::to_string(e.args.size())),
                e.line, e.col);
        }
        for (size_t i = 0; i < e.args.size(); i++) {
            if (!is_assignable(callee_type.fn_params[i], e.args[i]->resolved_type)) {
                throw SemanticError(
                    OFS_MSG("function argument type mismatch at position " + std::to_string(i),
                            "incompatibilidade de tipo de argumento de função na posição " + std::to_string(i)),
                    e.line, e.col);
            }
        }
        return callee_type.fn_return ? *callee_type.fn_return : OFSType::void_t();
    }

    // Look up function
    if (auto* id = dynamic_cast<IdentExpr*>(e.callee.get())) {
        if (is_fault_intrinsic_name(id->name)) {
            if (!is_low_level_scope(inside_fracture_, inside_abyss_, inside_bedrock_)) {
                throw SemanticError(
                    OFS_MSG("fault intrinsics require fracture, abyss, or bedrock block",
                            "intrinsics fault exigem bloco fracture, abyss ou bedrock"),
                    e.line, e.col);
            }

            const size_t arg_count = e.args.size();
            if ((id->name == "fault_fence" || id->name == "fault_trap") && arg_count != 0) {
                throw SemanticError(OFS_MSG("fault intrinsic expects 0 arguments", "intrinsic fault espera 0 argumentos"), e.line, e.col);
            }
            if (id->name == "fault_prefetch" && arg_count != 1) {
                throw SemanticError(OFS_MSG("fault_prefetch expects 1 argument", "fault_prefetch espera 1 argumento"), e.line, e.col);
            }
            if ((id->name == "fault_count" || id->name == "fault_lead" || id->name == "fault_trail" || id->name == "fault_swap") && arg_count != 1) {
                throw SemanticError(OFS_MSG("fault intrinsic expects 1 argument", "intrinsic fault espera 1 argumento"), e.line, e.col);
            }
            if ((id->name == "fault_spin_left" || id->name == "fault_spin_right") && arg_count != 2) {
                throw SemanticError(OFS_MSG("fault spin intrinsic expects 2 arguments", "intrinsic fault spin espera 2 argumentos"), e.line, e.col);
            }
            if (id->name == "fault_step" && arg_count != 2) {
                throw SemanticError(OFS_MSG("fault_step expects 2 arguments", "fault_step espera 2 argumentos"), e.line, e.col);
            }
            if (id->name == "fault_cut" && arg_count != 3) {
                throw SemanticError(OFS_MSG("fault_cut expects 3 arguments", "fault_cut espera 3 argumentos"), e.line, e.col);
            }
            if (id->name == "fault_patch" && arg_count != 4) {
                throw SemanticError(OFS_MSG("fault_patch expects 4 arguments", "fault_patch espera 4 argumentos"), e.line, e.col);
            }
            if (id->name == "fault_weave" && arg_count != 3) {
                throw SemanticError(OFS_MSG("fault_weave expects 3 arguments", "fault_weave espera 3 argumentos"), e.line, e.col);
            }
            if (id->name == "fault_unreachable" && arg_count != 0) {
                throw SemanticError(OFS_MSG("fault_unreachable expects 0 arguments", "fault_unreachable espera 0 argumentos"), e.line, e.col);
            }
            if (id->name == "fault_memcpy" && arg_count != 3) {
                throw SemanticError(OFS_MSG("fault_memcpy expects 3 arguments (dst, src, len)", "fault_memcpy espera 3 argumentos (dst, src, len)"), e.line, e.col);
            }
            if (id->name == "fault_memset" && arg_count != 3) {
                throw SemanticError(OFS_MSG("fault_memset expects 3 arguments (dst, val, len)", "fault_memset espera 3 argumentos (dst, val, len)"), e.line, e.col);
            }

            if (id->name == "fault_fence" || id->name == "fault_trap") {
                return OFSType::void_t();
            }

            if (id->name == "fault_unreachable") {
                return OFSType::void_t();
            }

            if (id->name == "fault_memcpy" || id->name == "fault_memset") {
                if (e.args[0]->resolved_type.base != BaseType::Shard) {
                    throw SemanticError(
                        OFS_MSG(id->name + " requires a pointer as first argument",
                                id->name + " exige ponteiro como primeiro argumento"),
                        e.line, e.col);
                }
                return OFSType::void_t();
            }

            if (id->name == "fault_prefetch") {
                if (e.args[0]->resolved_type.base != BaseType::Shard) {
                    throw SemanticError(
                        OFS_MSG("fault_prefetch requires a pointer argument", "fault_prefetch exige um argumento ponteiro"),
                        e.line, e.col);
                }
                return OFSType::void_t();
            }

            if (id->name == "fault_step") {
                if (e.args[0]->resolved_type.base != BaseType::Shard) {
                    throw SemanticError(
                        OFS_MSG("fault_step requires a pointer as first argument", "fault_step exige ponteiro no primeiro argumento"),
                        e.line, e.col);
                }
                if (e.args[1]->resolved_type.base != BaseType::Stone) {
                    throw SemanticError(
                        OFS_MSG("fault_step requires stone offset", "fault_step exige offset stone"),
                        e.line, e.col);
                }
                return e.args[0]->resolved_type;
            }

            for (auto& arg : e.args) {
                if (arg->resolved_type.base != BaseType::Stone) {
                    throw SemanticError(
                        OFS_MSG("fault intrinsics require stone arguments", "intrinsics fault exigem argumentos stone"),
                        e.line, e.col);
                }
            }
            return OFSType::stone();
        }

        auto* sym = scope_.lookup(id->name);
        if (sym && sym->kind == SymbolKind::Function) {
            auto it = function_intents_.find(sym->name);
            FuncIntent called_intent = (it != function_intents_.end()) ? it->second : FuncIntent::Impure;

            if (current_intent_ == FuncIntent::Pure &&
                (called_intent == FuncIntent::Impure || called_intent == FuncIntent::Fractal)) {
                throw SemanticError(
                    OFS_MSG("pure function cannot call impure/fractal function '" + id->name + "'",
                            "função pure não pode chamar função impure/fractal '" + id->name + "'"),
                    e.line, e.col);
            }

            if (current_intent_ == FuncIntent::Impure &&
                called_intent == FuncIntent::Fractal && !inside_fractal_) {
                throw SemanticError(
                    OFS_MSG("impure function cannot call fractal function '" + id->name + "' outside fractal block",
                            "função impure não pode chamar função fractal '" + id->name + "' fora de bloco fractal"),
                    e.line, e.col);
            }

            if (sym->type.base == BaseType::Function && sym->type.fn_return) {
                return *sym->type.fn_return;
            }
            return OFSType::void_t();
        }
        // Monolith constructor
        if (sym && sym->kind == SymbolKind::Monolith) {
            return OFSType::named(id->name);
        }

        std::string qname = qualify_name(namespace_stack_, id->name);
        auto* qsym = scope_.lookup(qname);
        if (qsym && qsym->kind == SymbolKind::Function && qsym->type.base == BaseType::Function && qsym->type.fn_return) {
            return *qsym->type.fn_return;
        }
    }

    // obj.method(args) or namespace.function(args)
    if (auto* mem = dynamic_cast<MemberExpr*>(e.callee.get())) {
        if (auto* obj_ident = dynamic_cast<IdentExpr*>(mem->object.get())) {
            std::string ns_candidate = obj_ident->name + "::" + mem->field;
            auto* ns_fn = scope_.lookup(ns_candidate);
            if (ns_fn && ns_fn->kind == SymbolKind::Function && ns_fn->type.base == BaseType::Function) {
                if (ns_fn->type.fn_return) return *ns_fn->type.fn_return;
                return OFSType::void_t();
            }
        }

        OFSType recv_type = check_expr(*mem->object);
        if (recv_type.base == BaseType::Named) {
            std::string key = recv_type.name + "::" + mem->field;
            auto it = impl_method_types_.find(key);
            if (it == impl_method_types_.end()) {
                throw SemanticError(
                    OFS_MSG("method '" + mem->field + "' not found for type '" + recv_type.name + "'",
                            "método '" + mem->field + "' não encontrado para o tipo '" + recv_type.name + "'"),
                    e.line, e.col);
            }
            OFSType sig = it->second;
            if (sig.base == BaseType::Function && sig.fn_return) {
                return *sig.fn_return;
            }
            return OFSType::void_t();
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
                throw SemanticError(OFS_MSG("cannot assign to const variable '" + id->name + "'",
                                            "não é possível atribuir à variável const '" + id->name + "'"),
                                    e.line, e.col);
            }
        }
    }

    OFSType target = check_expr(*e.target);
    OFSType value = check_expr(*e.value);

    if (e.op == "=") {
        if (inside_abyss_ || inside_bedrock_) {
            return target;
        }
        if (!is_assignable(target, value)) {
            throw SemanticError(
                OFS_MSG("cannot assign " + value.to_string() + " to " + target.to_string(),
                        "não é possível atribuir " + value.to_string() + " a " + target.to_string()),
                e.line, e.col);
        }
    } else {
        // Compound assignment (+=, -=, *=, /=, %=)
        if (!is_numeric(target) && !(target.base == BaseType::Obsidian && e.op == "+=")) {
            throw SemanticError(OFS_MSG("compound assignment requires numeric target",
                                        "atribuição composta requer destino numérico"),
                                e.line, e.col);
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
            throw SemanticError(OFS_MSG("monolith '" + obj.name + "' has no field '" + e.field + "'",
                                        "monolith '" + obj.name + "' não possui o campo '" + e.field + "'"),
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

    throw SemanticError(OFS_MSG("cannot cast " + from.to_string() + " to " + to.to_string(),
                                "não é possível converter " + from.to_string() + " para " + to.to_string()),
                        e.line, e.col);
}

OFSType SemanticAnalyzer::check_lambda(LambdaExpr& e) {
    scope_.push();
    for (auto& p : e.params) {
        Symbol sym;
        sym.name = p.name;
        sym.type = p.type;
        sym.kind = SymbolKind::Param;
        scope_.define(p.name, sym);
    }

    OFSType prev_ret = current_return_type_;
    current_return_type_ = e.return_type;
    if (e.body) check_stmt(*e.body);
    current_return_type_ = prev_ret;

    scope_.pop();
    return function_signature_type(e.params, e.return_type);
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
    // Numeric assignments are allowed (narrowing/widening handled by codegen cast)
    if (is_numeric(to) && is_numeric(from)) return true;
    // Infer is always compatible
    if (to.base == BaseType::Infer || from.base == BaseType::Infer) return true;
    return false;
}

bool SemanticAnalyzer::is_numeric(const OFSType& t) const {
    return t.is_numeric();
}

bool SemanticAnalyzer::is_integral(const OFSType& t) const {
    return t.is_integral();
}

bool SemanticAnalyzer::is_unsigned_integer(const OFSType& t) const {
    return t.is_unsigned_integer();
}

int SemanticAnalyzer::integer_bits(const OFSType& t) const {
    return t.integer_bits();
}

OFSType SemanticAnalyzer::promote(const OFSType& a, const OFSType& b) const {
    if (a.base == BaseType::Crystal || b.base == BaseType::Crystal) {
        return OFSType::crystal();
    }

    // Any operation with stone promotes to stone.
    if (a.base == BaseType::Stone || b.base == BaseType::Stone) {
        return OFSType::stone();
    }

    // Requested unsigned cascade promotion rules.
    if (a.base == BaseType::U8 && b.base == BaseType::U8)   return OFSType::u16();
    if (a.base == BaseType::U16 && b.base == BaseType::U16) return OFSType::u32();
    if (a.base == BaseType::U32 && b.base == BaseType::U32) return OFSType::u64();

    // Signed small integer promotions.
    if (a.base == BaseType::I8 && b.base == BaseType::I8)   return OFSType::i32();
    if (a.base == BaseType::I32 && b.base == BaseType::I32) return OFSType::i32();

    // Generic integer promotion fallback.
    if (is_integral(a) && is_integral(b)) {
        const int bits = std::max(integer_bits(a), integer_bits(b));
        const bool both_unsigned = is_unsigned_integer(a) && is_unsigned_integer(b);

        if (both_unsigned) {
            if (bits <= 8)  return OFSType::u8();
            if (bits <= 16) return OFSType::u16();
            if (bits <= 32) return OFSType::u32();
            return OFSType::u64();
        }

        if (bits <= 8)  return OFSType::i8();
        if (bits <= 32) return OFSType::i32();
        return OFSType::stone();
    }

    return OFSType::stone();
}

void SemanticAnalyzer::expect_type(const OFSType& got, const OFSType& want, int line, int col) {
    if (!is_assignable(want, got)) {
        throw SemanticError(
            OFS_MSG("expected " + want.to_string() + " but got " + got.to_string(),
                    "esperado " + want.to_string() + " mas encontrado " + got.to_string()),
            line, col);
    }
}

} // namespace ofs
