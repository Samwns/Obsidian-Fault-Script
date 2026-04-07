#pragma once
#include "type_system.hpp"

namespace ofs {

class SemanticAnalyzer {
public:
    void analyze(Module& mod); // mutates AST: fills resolved_type fields

private:
    // ── Declarations ─────────────────────────────────────────────────────
    void check_func(FuncDecl& fn);
    void check_monolith(MonolithDecl& m);
    void check_global_forge(GlobalForgeDecl& g);
    void check_import(ImportDecl& i);
    void check_extern(ExternFuncDecl& e);
    void check_strata(StrataDecl& s);

    // ── Statements ────────────────────────────────────────────────────────
    void check_stmt(Stmt& s);
    void predeclare_block_symbols(BlockStmt& s);
    void check_block(BlockStmt& s);
    void check_forge(ForgeStmt& s);
    void check_const(ConstStmt& s);
    void check_if(IfStmt& s);
    void check_cycle(CycleStmt& s);
    void check_return(ReturnStmt& s);
    void check_fracture(FractureStmt& s);
    void check_abyss(AbyssStmt& s);
    void check_fractal(FractalStmt& s);
    void check_bedrock(BedrockStmt& s);
    void check_while(WhileCycleStmt& s);
    void check_match(MatchStmt& s);
    void check_tremor(TremorStmt& s);
    void check_throw(ThrowStmt& s);

    // ── Expressions ───────────────────────────────────────────────────────
    OFSType check_expr(Expr& e);
    OFSType check_binary(BinaryExpr& e);
    OFSType check_unary(UnaryExpr& e);
    OFSType check_call(CallExpr& e);
    OFSType check_echo(EchoExpr& e);
    OFSType check_assign(AssignExpr& e);
    OFSType check_index(IndexExpr& e);
    OFSType check_member(MemberExpr& e);
    OFSType check_array_lit(ArrayLitExpr& e);
    OFSType check_cast(CastExpr& e);

    // ── Type helpers ──────────────────────────────────────────────────────
    OFSType infer_type(const Expr& init); // infer type from initializer
    bool    is_assignable(const OFSType& to, const OFSType& from) const;
    bool    is_numeric(const OFSType& t)  const;
    OFSType promote(const OFSType& a, const OFSType& b) const; // int+float -> float
    void    expect_type(const OFSType& got, const OFSType& want, int line, int col);
    void    register_builtin_symbols();

    // ── State ─────────────────────────────────────────────────────────────
    Scope       scope_;
    OFSType     current_return_type_;    // for return checking
    FuncIntent  current_intent_ = FuncIntent::Impure;
    bool        inside_fracture_ = false;
    bool        inside_abyss_    = false;
    bool        inside_fractal_  = false;
    bool        inside_bedrock_  = false;
    bool        inside_cycle_    = false;

    std::unordered_map<std::string, FuncIntent> function_intents_;

    // Monolith registry (for field access checks)
    std::unordered_map<std::string, MonolithDecl*> monoliths_;
};

} // namespace ofs
