#pragma once
#include "../ast/ast.hpp"
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Type.h>
#include <memory>
#include <unordered_map>

namespace ofs {

class CodeGen {
public:
    CodeGen();

    // Generate LLVM IR from AST module and keep it in internal state.
    void generate(const Module& mod);

    // Emit LLVM IR as text to a file (for debugging: ofs build --emit-llvm)
    void emit_ir(const std::string& filepath);

    // Compile to native object file
    void emit_object(const std::string& filepath);

    // Link and produce final executable
    void link(const std::string& obj_file, const std::string& out_file);

private:
    // ── LLVM IR Generation ────────────────────────────────────────────────
    void       gen_func(const FuncDecl& fn);
    void       gen_monolith(const MonolithDecl& m);
    void       gen_global_forge(const GlobalForgeDecl& g);
    void       gen_extern(const ExternFuncDecl& e);
    void       gen_strata(const StrataDecl& s);

    void       gen_stmt(const Stmt& s);
    void       gen_block(const BlockStmt& s);
    void       gen_forge(const ForgeStmt& s);
    void       gen_const(const ConstStmt& s);
    void       gen_if(const IfStmt& s);
    void       gen_cycle(const CycleStmt& s);
    void       gen_return(const ReturnStmt& s);
    void       gen_fracture(const FractureStmt& s);
    void       gen_abyss(const AbyssStmt& s);
    void       gen_fractal(const FractalStmt& s);
    void       gen_while(const WhileCycleStmt& s);
    void       gen_match(const MatchStmt& s);
    void       gen_tremor(const TremorStmt& s);
    void       gen_throw(const ThrowStmt& s);
    void       gen_echo(const EchoExpr& e);

    llvm::Value* gen_expr(const Expr& e);
    llvm::Value* gen_binary(const BinaryExpr& e);
    llvm::Value* gen_unary(const UnaryExpr& e);
    llvm::Value* gen_call(const CallExpr& e);
    llvm::Value* gen_assign(const AssignExpr& e);
    llvm::Value* gen_index(const IndexExpr& e);
    llvm::Value* gen_member(const MemberExpr& e);
    llvm::Value* gen_array_lit(const ArrayLitExpr& e);
    llvm::Value* gen_cast(const CastExpr& e);

    // Get address of an lvalue expression (for assignment, address-of)
    llvm::Value* gen_lvalue(const Expr& e);

    // ── LLVM Type Mapping ─────────────────────────────────────────────────
    llvm::Type*     llvm_type(const OFSType& t);
    llvm::Type*     llvm_array_struct(const OFSType& element_type);

    // ── Runtime function declarations ─────────────────────────────────────
    void declare_runtime();
    llvm::Function* get_runtime_fn(const std::string& name);

    // ── Helpers ───────────────────────────────────────────────────────────
    llvm::Value*    get_var(const std::string& name);
    void            set_var(const std::string& name, llvm::Value* alloca_val);
    llvm::AllocaInst* create_alloca(llvm::Function* fn,
                                     const std::string& name,
                                     llvm::Type* ty);

    // ── State ─────────────────────────────────────────────────────────────
    llvm::LLVMContext                 ctx_;
    llvm::IRBuilder<>                 builder_;
    std::unique_ptr<llvm::Module>     mod_;

    // Variable name -> alloca (stack slot)
    // Stack of scopes: each scope is a map
    std::vector<std::unordered_map<std::string, llvm::Value*>> var_stack_;

    // Monolith (struct) type cache
    std::unordered_map<std::string, llvm::StructType*> struct_types_;

    // Monolith field names (for GEP)
    std::unordered_map<std::string, std::vector<std::string>> struct_fields_;

    // Function declarations (for calls)
    std::unordered_map<std::string, llvm::Function*> functions_;

    // Current function being generated
    llvm::Function*  cur_fn_ = nullptr;

    // Labels for break/continue in cycle
    llvm::BasicBlock* cycle_exit_bb_    = nullptr;
    llvm::BasicBlock* cycle_header_bb_  = nullptr;

    // Runtime function cache
    std::unordered_map<std::string, llvm::Function*> runtime_fns_;
};

} // namespace ofs
