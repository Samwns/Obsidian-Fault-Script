#include "codegen.hpp"
#include <llvm/IR/Verifier.h>
#include <llvm/IR/InlineAsm.h>
#include <llvm/IR/Intrinsics.h>
#include <llvm/Support/AtomicOrdering.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#if LLVM_VERSION_MAJOR >= 17
#include <llvm/TargetParser/Host.h>
#else
#include <llvm/Support/Host.h>
#endif
#include <llvm/Config/llvm-config.h>
#include <llvm/Support/CodeGen.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <cstdlib>
#include <iostream>
#include <filesystem>
#include <vector>
#ifdef _WIN32
#include <windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#include <limits.h>
#else
#include <unistd.h>
#include <limits.h>
#endif

namespace ofs {

namespace {
bool runtime_file_exists(const std::filesystem::path& path) {
    std::error_code ec;
    return !path.empty() && std::filesystem::is_regular_file(path, ec);
}

void add_runtime_candidates(std::vector<std::string>& paths, const std::filesystem::path& dir) {
    if (dir.empty()) {
        return;
    }

    paths.push_back((dir / "libofs_runtime.a").string());
    paths.push_back((dir / "ofs_runtime.lib").string());
    paths.push_back((dir / "ofs_runtime.o").string());
}

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

}

// ── Constructor ───────────────────────────────────────────────────────────

CodeGen::CodeGen() : builder_(ctx_) {
    // Initialize only native target (X86 on this platform)
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();
}

// ── Generate ──────────────────────────────────────────────────────────────

void CodeGen::generate(const Module& mod) {
    mod_ = std::make_unique<llvm::Module>("ofs_module", ctx_);

    // Set target triple
    std::string triple = llvm::sys::getDefaultTargetTriple();
#if LLVM_VERSION_MAJOR >= 20
    mod_->setTargetTriple(llvm::Triple(triple));
#else
    mod_->setTargetTriple(triple);
#endif

    // Declare runtime functions
    declare_runtime();

    // Push global scope
    var_stack_.push_back({});

    auto declare_decl = [&](auto&& self, const Decl& d) -> void {
        if (auto* fn = dynamic_cast<const FuncDecl*>(&d)) {
            std::vector<llvm::Type*> param_types;
            for (auto& p : fn->params) {
                param_types.push_back(llvm_type(p.type));
            }
            llvm::Type* ret_type = llvm_type(fn->return_type);

            std::string symbol_name = qualify_symbol(fn->name);
            if (fn->is_core && fn->name == "main") {
                symbol_name = "main";
                ret_type = llvm::Type::getInt32Ty(ctx_);
            }

            auto* ft = llvm::FunctionType::get(ret_type, param_types, false);
            auto* f = llvm::Function::Create(ft, llvm::Function::ExternalLinkage,
                                             symbol_name, mod_.get());
            functions_[fn->name] = f;
            functions_[symbol_name] = f;

            size_t i = 0;
            for (auto& arg : f->args()) {
                arg.setName(fn->params[i].name);
                i++;
            }
            return;
        }

        if (auto* m = dynamic_cast<const MonolithDecl*>(&d)) {
            gen_monolith(*m);
            return;
        }

        if (auto* st = dynamic_cast<const StrataDecl*>(&d)) {
            gen_strata(*st);
            return;
        }

        if (auto* ext = dynamic_cast<const ExternFuncDecl*>(&d)) {
            gen_extern(*ext);
            return;
        }

        if (auto* impl = dynamic_cast<const ImplDecl*>(&d)) {
            for (const auto& method : impl->methods) {
                std::vector<llvm::Type*> param_types;
                for (auto& p : method->params) {
                    param_types.push_back(llvm_type(p.type));
                }
                llvm::Type* ret_type = llvm_type(method->return_type);
                std::string mangled = mangle_method_name(impl->target_name, method->name);
                auto* ft = llvm::FunctionType::get(ret_type, param_types, false);
                auto* f = llvm::Function::Create(ft, llvm::Function::ExternalLinkage,
                                                 mangled, mod_.get());
                method_functions_[impl->target_name + "::" + method->name] = f;
                functions_[mangled] = f;
                size_t i = 0;
                for (auto& arg : f->args()) {
                    arg.setName(method->params[i].name);
                    i++;
                }
            }
            return;
        }

        if (auto* ns = dynamic_cast<const NamespaceDecl*>(&d)) {
            namespace_stack_.push_back(ns->name);
            for (const auto& inner : ns->declarations) self(self, *inner);
            namespace_stack_.pop_back();
            return;
        }
    };

    auto emit_decl = [&](auto&& self, const Decl& d) -> void {
        if (auto* fn = dynamic_cast<const FuncDecl*>(&d)) {
            gen_func(*fn);
            return;
        }
        if (auto* g = dynamic_cast<const GlobalForgeDecl*>(&d)) {
            gen_global_forge(*g);
            return;
        }
        if (auto* impl = dynamic_cast<const ImplDecl*>(&d)) {
            for (const auto& method : impl->methods) gen_func(*method);
            return;
        }
        if (auto* ns = dynamic_cast<const NamespaceDecl*>(&d)) {
            namespace_stack_.push_back(ns->name);
            for (const auto& inner : ns->declarations) self(self, *inner);
            namespace_stack_.pop_back();
            return;
        }
    };

    for (auto& d : mod.decls) declare_decl(declare_decl, *d);
    namespace_stack_.clear();
    for (auto& d : mod.decls) emit_decl(emit_decl, *d);

    // Pop global scope
    var_stack_.pop_back();

    // Verify module
    std::string err_str;
    llvm::raw_string_ostream err_stream(err_str);
    if (llvm::verifyModule(*mod_, &err_stream)) {
        std::cerr << "LLVM verification error:\n" << err_str << "\n";
    }
}

// ── Emit IR ───────────────────────────────────────────────────────────────

void CodeGen::emit_ir(const std::string& filepath) {
    std::error_code ec;
    llvm::raw_fd_ostream os(filepath, ec, llvm::sys::fs::OF_None);
    if (ec) {
        std::cerr << "could not open file: " << ec.message() << "\n";
        return;
    }
    mod_->print(os, nullptr);
}

void CodeGen::emit_asm(const std::string& filepath) {
    std::string triple = llvm::sys::getDefaultTargetTriple();
    std::string error;
    auto target = llvm::TargetRegistry::lookupTarget(triple, error);
    if (!target) {
        std::cerr << "target error: " << error << "\n";
        return;
    }

    auto cpu = "generic";
    auto features = "";
    llvm::TargetOptions opt;
#if LLVM_VERSION_MAJOR >= 16
    auto rm = std::optional<llvm::Reloc::Model>(llvm::Reloc::PIC_);
#else
    auto rm = llvm::Optional<llvm::Reloc::Model>(llvm::Reloc::PIC_);
#endif
#if LLVM_VERSION_MAJOR >= 20
    auto tm = target->createTargetMachine(llvm::Triple(triple), cpu, features, opt, rm);
#else
    auto tm = target->createTargetMachine(triple, cpu, features, opt, rm);
#endif
    mod_->setDataLayout(tm->createDataLayout());

    std::error_code ec;
    llvm::raw_fd_ostream dest(filepath, ec, llvm::sys::fs::OF_None);
    if (ec) {
        std::cerr << "could not open file: " << ec.message() << "\n";
        return;
    }

    llvm::legacy::PassManager pass;
#if LLVM_VERSION_MAJOR >= 18
    if (tm->addPassesToEmitFile(pass, dest, nullptr, llvm::CodeGenFileType::AssemblyFile)) {
#else
    if (tm->addPassesToEmitFile(pass, dest, nullptr, llvm::CGFT_AssemblyFile)) {
#endif
        std::cerr << "target machine can't emit a file of this type\n";
        return;
    }

    pass.run(*mod_);
    dest.flush();
}

// ── Emit Object ───────────────────────────────────────────────────────────

void CodeGen::emit_object(const std::string& filepath) {
    std::string triple = llvm::sys::getDefaultTargetTriple();
    std::string error;
    auto target = llvm::TargetRegistry::lookupTarget(triple, error);
    if (!target) {
        std::cerr << "target error: " << error << "\n";
        return;
    }

    auto cpu = "generic";
    auto features = "";
    llvm::TargetOptions opt;
#if LLVM_VERSION_MAJOR >= 16
    auto rm = std::optional<llvm::Reloc::Model>(llvm::Reloc::PIC_);
#else
    auto rm = llvm::Optional<llvm::Reloc::Model>(llvm::Reloc::PIC_);
#endif
#if LLVM_VERSION_MAJOR >= 20
    auto tm = target->createTargetMachine(llvm::Triple(triple), cpu, features, opt, rm);
#else
    auto tm = target->createTargetMachine(triple, cpu, features, opt, rm);
#endif
    mod_->setDataLayout(tm->createDataLayout());

    std::error_code ec;
    llvm::raw_fd_ostream dest(filepath, ec, llvm::sys::fs::OF_None);
    if (ec) {
        std::cerr << "could not open file: " << ec.message() << "\n";
        return;
    }

    llvm::legacy::PassManager pass;
#if LLVM_VERSION_MAJOR >= 18
    if (tm->addPassesToEmitFile(pass, dest, nullptr, llvm::CodeGenFileType::ObjectFile)) {
#else
    if (tm->addPassesToEmitFile(pass, dest, nullptr, llvm::CGFT_ObjectFile)) {
#endif
        std::cerr << "target machine can't emit a file of this type\n";
        return;
    }

    pass.run(*mod_);
    dest.flush();
}

// ── Link ──────────────────────────────────────────────────────────────────

static std::string get_executable_dir() {
#ifdef _WIN32
    char buffer[MAX_PATH];
    DWORD len = GetModuleFileNameA(nullptr, buffer, MAX_PATH);
    if (len == 0 || len == MAX_PATH) {
        return "";
    }
    return std::filesystem::path(buffer).parent_path().string();
#elif defined(__APPLE__)
    uint32_t size = 0;
    _NSGetExecutablePath(nullptr, &size);
    std::string exe_path(size, '\0');
    if (_NSGetExecutablePath(exe_path.data(), &size) != 0) {
        return "";
    }
    return std::filesystem::path(exe_path.c_str()).parent_path().string();
#else
    char buffer[PATH_MAX] = {0};
    ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
    if (len <= 0) {
        return "";
    }
    buffer[len] = '\0';
    return std::filesystem::path(buffer).parent_path().string();
#endif
}

void CodeGen::link(const std::string& obj_file, const std::string& out_file) {
    std::string runtime_obj;
    const std::string exe_dir = get_executable_dir();

    std::vector<std::string> paths;

    if (const char* runtime_env = std::getenv("OFS_RUNTIME_PATH")) {
        std::filesystem::path env_path(runtime_env);
        if (runtime_file_exists(env_path)) {
            paths.push_back(env_path.string());
        } else {
            add_runtime_candidates(paths, env_path);
        }
    }

    if (!exe_dir.empty()) {
        auto exe_path = std::filesystem::path(exe_dir);
        // Prefer runtime shipped next to the active compiler binary
        // (important for VS Code embedded compiler and portable installs).
        add_runtime_candidates(paths, exe_path);
        add_runtime_candidates(paths, exe_path / "lib");
        add_runtime_candidates(paths, exe_path.parent_path());
        add_runtime_candidates(paths, exe_path.parent_path() / "lib");
        add_runtime_candidates(paths, exe_path.parent_path() / "lib" / "ofs");
    }

    add_runtime_candidates(paths, std::filesystem::current_path());
    add_runtime_candidates(paths, std::filesystem::current_path().parent_path());

    // Do not search workspace-local build outputs here; they may belong to
    // another OFS build and cause ABI/runtime symbol mismatches.
    paths.push_back("libofs_runtime.a");
    paths.push_back("ofs_runtime.lib");
    paths.push_back("ofs_runtime.o");

#ifndef _WIN32
    add_runtime_candidates(paths, "/usr/local/lib");
    add_runtime_candidates(paths, "/usr/local/lib/ofs");
    add_runtime_candidates(paths, "/usr/lib");
    add_runtime_candidates(paths, "/usr/lib/ofs");
#endif

    for (const auto& p : paths) {
        if (FILE* f = fopen(p.c_str(), "r")) {
            fclose(f);
            runtime_obj = p;
            break;
        }
    }

    std::string cmd = "cc \"" + obj_file + "\"";
    if (!runtime_obj.empty()) {
        cmd += " \"" + runtime_obj + "\"";
    } else {
        std::cerr << "runtime library not found (expected libofs_runtime.a/ofs_runtime.o near compiler or in standard library paths)\n";
    }
    cmd += " -o \"" + out_file + "\" -lm";
#ifdef _WIN32
    cmd += " -lws2_32";
#endif

    int ret = std::system(cmd.c_str());
    if (ret != 0) {
        std::cerr << "link failed (exit " << ret << ")\n";
    }
}

// ── Declare Runtime ───────────────────────────────────────────────────────

void CodeGen::declare_runtime() {
    auto* void_ty  = llvm::Type::getVoidTy(ctx_);
    auto* i64_ty   = llvm::Type::getInt64Ty(ctx_);
    auto* i32_ty   = llvm::Type::getInt32Ty(ctx_);
    auto* i8ptr_ty = llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(ctx_));
    auto* f64_ty   = llvm::Type::getDoubleTy(ctx_);
    auto* i1_ty    = llvm::Type::getInt1Ty(ctx_);
    auto* ptr_ty   = llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(ctx_));

    auto declare = [&](const std::string& name, llvm::Type* ret,
                       std::vector<llvm::Type*> args) {
        auto* ft = llvm::FunctionType::get(ret, args, false);
        auto* fn = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, name, mod_.get());
        runtime_fns_[name] = fn;
    };

    declare("echo_stone",    void_ty, {i64_ty});
    declare("echo_crystal",  void_ty, {f64_ty});
    declare("echo_obsidian", void_ty, {i8ptr_ty});
    declare("echo_bool",     void_ty, {i32_ty});
    declare("ofs_echo_color",void_ty, {i8ptr_ty, i8ptr_ty});
    declare("echo_newline",  void_ty, {});
    declare("ofs_alloc",     i8ptr_ty, {i64_ty});
    declare("ofs_free",      void_ty, {i8ptr_ty});
    declare("ofs_array_new", ptr_ty,  {i64_ty, i64_ty});
    declare("ofs_array_push",void_ty, {ptr_ty, ptr_ty});
    declare("ofs_array_get", ptr_ty,  {ptr_ty, i64_ty});
    declare("ofs_array_len", i64_ty,  {ptr_ty});
    declare("ofs_str_concat",i8ptr_ty,{i8ptr_ty, i8ptr_ty});
    declare("ofs_str_eq",    i32_ty,  {i8ptr_ty, i8ptr_ty});

    // String operations
    declare("ofs_str_len",      i64_ty,   {i8ptr_ty});
    declare("ofs_str_char_at",  i64_ty,   {i8ptr_ty, i64_ty});
    declare("ofs_str_substr",   i8ptr_ty, {i8ptr_ty, i64_ty, i64_ty});
    declare("ofs_str_contains", i32_ty,   {i8ptr_ty, i8ptr_ty});
    declare("ofs_str_upper",    i8ptr_ty, {i8ptr_ty});
    declare("ofs_str_lower",    i8ptr_ty, {i8ptr_ty});

    // Type conversions
    declare("ofs_stone_to_obsidian",   i8ptr_ty, {i64_ty});
    declare("ofs_crystal_to_obsidian", i8ptr_ty, {f64_ty});
    declare("ofs_obsidian_to_stone",   i64_ty,   {i8ptr_ty});
    declare("ofs_obsidian_to_crystal", f64_ty,   {i8ptr_ty});

    // Array operations
    declare("ofs_array_set",  void_ty, {ptr_ty, i64_ty, ptr_ty});
    declare("ofs_array_pop",  ptr_ty,  {ptr_ty});

    // Math operations
    declare("ofs_pow",  f64_ty, {f64_ty, f64_ty});
    declare("ofs_sqrt", f64_ty, {f64_ty});
    declare("ofs_mod",  i64_ty, {i64_ty, i64_ty});

    // I/O operations
    declare("ofs_read_line",     i8ptr_ty, {});
    declare("echo_stone_nn",     void_ty,  {i64_ty});
    declare("echo_crystal_nn",   void_ty,  {f64_ty});
    declare("echo_obsidian_nn",  void_ty,  {i8ptr_ty});

    // Also declare printf for flexibility
    auto* printf_ty = llvm::FunctionType::get(i32_ty, {i8ptr_ty}, true);
    auto* printf_fn = llvm::Function::Create(printf_ty, llvm::Function::ExternalLinkage,
                                              "printf", mod_.get());
    runtime_fns_["printf"] = printf_fn;
}

llvm::Function* CodeGen::get_runtime_fn(const std::string& name) {
    auto it = runtime_fns_.find(name);
    if (it != runtime_fns_.end()) return it->second;
    return nullptr;
}

// ── Variable scope helpers ────────────────────────────────────────────────

llvm::Value* CodeGen::get_var(const std::string& name) {
    for (auto it = var_stack_.rbegin(); it != var_stack_.rend(); ++it) {
        auto found = it->find(name);
        if (found != it->end()) return found->second;
    }
    return nullptr;
}

void CodeGen::set_var(const std::string& name, llvm::Value* alloca_val) {
    var_stack_.back()[name] = alloca_val;
}

llvm::AllocaInst* CodeGen::create_alloca(llvm::Function* fn,
                                          const std::string& name,
                                          llvm::Type* ty) {
    llvm::IRBuilder<> tmp_builder(&fn->getEntryBlock(), fn->getEntryBlock().begin());
    return tmp_builder.CreateAlloca(ty, nullptr, name);
}

std::string CodeGen::current_namespace() const {
    if (namespace_stack_.empty()) return "";
    std::string out;
    for (size_t i = 0; i < namespace_stack_.size(); i++) {
        if (i > 0) out += "::";
        out += namespace_stack_[i];
    }
    return out;
}

std::string CodeGen::qualify_symbol(const std::string& name) const {
    std::string ns = current_namespace();
    if (ns.empty()) return name;
    return ns + "::" + name;
}

std::string CodeGen::mangle_method_name(const std::string& type_name, const std::string& method) const {
    return type_name + "_" + method;
}

bool CodeGen::is_integral_type(const OFSType& t) const {
    return t.base == BaseType::Stone ||
           t.base == BaseType::U8 || t.base == BaseType::U16 ||
           t.base == BaseType::U32 || t.base == BaseType::U64 ||
           t.base == BaseType::I8 || t.base == BaseType::I32;
}

bool CodeGen::is_unsigned_type(const OFSType& t) const {
    return t.base == BaseType::U8 || t.base == BaseType::U16 ||
           t.base == BaseType::U32 || t.base == BaseType::U64;
}

llvm::FunctionType* CodeGen::llvm_function_type(const OFSType& t) {
    if (t.base != BaseType::Function) return nullptr;

    std::vector<llvm::Type*> params;
    for (const auto& p : t.fn_params) {
        params.push_back(llvm_type(p));
    }
    llvm::Type* ret = t.fn_return ? llvm_type(*t.fn_return) : llvm::Type::getVoidTy(ctx_);
    return llvm::FunctionType::get(ret, params, false);
}

llvm::Value* CodeGen::cast_to_type(llvm::Value* val, const OFSType& from, const OFSType& to) {
    if (!val) return nullptr;

    auto* dst_ty = llvm_type(to);
    auto* src_ty = val->getType();
    if (src_ty == dst_ty) return val;

    // To bool
    if (to.base == BaseType::Bool) {
        if (src_ty->isIntegerTy()) {
            return builder_.CreateICmpNE(val, llvm::ConstantInt::get(src_ty, 0), "to_bool");
        }
        if (src_ty->isDoubleTy()) {
            return builder_.CreateFCmpONE(val, llvm::ConstantFP::get(ctx_, llvm::APFloat(0.0)), "to_bool");
        }
    }

    // From bool
    if (from.base == BaseType::Bool) {
        if (dst_ty->isIntegerTy()) {
            return builder_.CreateZExt(val, dst_ty, "bool_to_int");
        }
        if (dst_ty->isDoubleTy()) {
            return builder_.CreateUIToFP(val, dst_ty, "bool_to_float");
        }
    }

    // Integer <-> Integer
    if (src_ty->isIntegerTy() && dst_ty->isIntegerTy()) {
        unsigned src_bits = src_ty->getIntegerBitWidth();
        unsigned dst_bits = dst_ty->getIntegerBitWidth();
        if (src_bits == dst_bits) return val;
        if (src_bits < dst_bits) {
            if (is_unsigned_type(from) || from.base == BaseType::Bool) {
                return builder_.CreateZExt(val, dst_ty, "zext");
            }
            return builder_.CreateSExt(val, dst_ty, "sext");
        }
        return builder_.CreateTrunc(val, dst_ty, "trunc");
    }

    // Integer -> Float
    if (src_ty->isIntegerTy() && dst_ty->isDoubleTy()) {
        if (is_unsigned_type(from) || from.base == BaseType::Bool) {
            return builder_.CreateUIToFP(val, dst_ty, "uitofp");
        }
        return builder_.CreateSIToFP(val, dst_ty, "sitofp");
    }

    // Float -> Integer
    if (src_ty->isDoubleTy() && dst_ty->isIntegerTy()) {
        if (is_unsigned_type(to)) {
            return builder_.CreateFPToUI(val, dst_ty, "fptoui");
        }
        return builder_.CreateFPToSI(val, dst_ty, "fptosi");
    }

    // Pointer casts fallback
    if (src_ty->isPointerTy() && dst_ty->isPointerTy()) {
        return builder_.CreateBitCast(val, dst_ty, "ptr_cast");
    }

    return builder_.CreateBitCast(val, dst_ty, "cast");
}

// ── LLVM Type Mapping ─────────────────────────────────────────────────────

llvm::Type* CodeGen::llvm_type(const OFSType& t) {
    switch (t.base) {
        case BaseType::Stone:    return llvm::Type::getInt64Ty(ctx_);
        case BaseType::U8:
        case BaseType::I8:       return llvm::Type::getInt8Ty(ctx_);
        case BaseType::U16:      return llvm::Type::getInt16Ty(ctx_);
        case BaseType::U32:
        case BaseType::I32:      return llvm::Type::getInt32Ty(ctx_);
        case BaseType::U64:      return llvm::Type::getInt64Ty(ctx_);
        case BaseType::Crystal:  return llvm::Type::getDoubleTy(ctx_);
        case BaseType::Obsidian: return llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(ctx_));
        case BaseType::Bool:     return llvm::Type::getInt1Ty(ctx_);
        case BaseType::Void:     return llvm::Type::getVoidTy(ctx_);
        case BaseType::Shard:
            if (t.inner) return llvm::PointerType::getUnqual(llvm_type(*t.inner));
            return llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(ctx_));
        case BaseType::Array:
            return llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(ctx_));
        case BaseType::Function: {
            auto* ft = llvm_function_type(t);
            if (!ft) return llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(ctx_));
            return llvm::PointerType::getUnqual(ft);
        }
        case BaseType::Named: {
            auto it = struct_types_.find(t.name);
            if (it != struct_types_.end()) {
                return llvm::PointerType::getUnqual(it->second);
            }
            return llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(ctx_));
        }
        case BaseType::Infer:
            return llvm::Type::getInt64Ty(ctx_); // default
    }
    return llvm::Type::getInt64Ty(ctx_);
}

llvm::Type* CodeGen::llvm_array_struct(const OFSType& element_type) {
    // OfsArray: { data: ptr, len: i64, cap: i64, elem_size: i64 }
    return llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(ctx_));
}

// ── Generate Functions ────────────────────────────────────────────────────

void CodeGen::gen_func(const FuncDecl& fn) {
    llvm::Function* llvm_fn = nullptr;

    auto it_direct = functions_.find(fn.name);
    if (it_direct != functions_.end()) llvm_fn = it_direct->second;

    if (!llvm_fn) {
        auto it_q = functions_.find(qualify_symbol(fn.name));
        if (it_q != functions_.end()) llvm_fn = it_q->second;
    }

    if (!llvm_fn && !fn.params.empty() && fn.params[0].name == "self" && fn.params[0].type.base == BaseType::Named) {
        auto mangled = mangle_method_name(fn.params[0].type.name, fn.name);
        auto it_m = functions_.find(mangled);
        if (it_m != functions_.end()) llvm_fn = it_m->second;
    }

    if (!llvm_fn) return;

    auto* entry = llvm::BasicBlock::Create(ctx_, "entry", llvm_fn);
    builder_.SetInsertPoint(entry);

    cur_fn_ = llvm_fn;
    var_stack_.push_back({});

    // Create alloca for each parameter
    size_t i = 0;
    for (auto& arg : llvm_fn->args()) {
        auto* alloca = create_alloca(llvm_fn, fn.params[i].name, arg.getType());
        builder_.CreateStore(&arg, alloca);
        set_var(fn.params[i].name, alloca);
        i++;
    }

    // Generate body
    if (fn.body) gen_stmt(*fn.body);

    // If the function doesn't end with a terminator, add one
    auto* bb = builder_.GetInsertBlock();
    if (!bb->getTerminator()) {
        if (fn.is_core && fn.name == "main") {
            builder_.CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx_), 0));
        } else if (fn.return_type.base == BaseType::Void) {
            builder_.CreateRetVoid();
        } else {
            // Default return for non-void
            builder_.CreateRet(llvm::Constant::getNullValue(llvm_type(fn.return_type)));
        }
    }

    var_stack_.pop_back();
}

void CodeGen::gen_monolith(const MonolithDecl& m) {
    std::vector<llvm::Type*> field_types;
    std::vector<std::string> field_names;

    for (auto& f : m.fields) {
        field_types.push_back(llvm_type(f.type));
        field_names.push_back(f.name);
    }

    bool packed = m.layout == "packed";
    auto* st = llvm::StructType::create(ctx_, field_types, m.name, packed);
    struct_types_[m.name] = st;
    struct_fields_[m.name] = field_names;
}

void CodeGen::gen_global_forge(const GlobalForgeDecl& g) {
    OFSType type = g.type_ann.value_or(OFSType::stone());
    auto* llvm_ty = llvm_type(type);

    auto* gv = new llvm::GlobalVariable(
        *mod_, llvm_ty, false,
        llvm::GlobalValue::InternalLinkage,
        llvm::Constant::getNullValue(llvm_ty),
        g.name
    );

    var_stack_.front()[g.name] = gv;
}

void CodeGen::gen_extern(const ExternFuncDecl& e) {
    const std::string symbol_name = e.link_name.empty() ? e.name : e.link_name;
    const std::string exposed_name = qualify_symbol(e.name);

    if (auto* existing = mod_->getFunction(symbol_name)) {
        functions_[e.name] = existing;
        functions_[exposed_name] = existing;
        return;
    }

    std::vector<llvm::Type*> param_types;
    for (auto& p : e.params) {
        param_types.push_back(llvm_type(p.type));
    }
    llvm::Type* ret_type = llvm_type(e.return_type);

    auto* ft = llvm::FunctionType::get(ret_type, param_types, e.is_variadic);
    auto* fn = llvm::Function::Create(ft, llvm::Function::ExternalLinkage,
                                       symbol_name, mod_.get());
    functions_[e.name] = fn;
    functions_[exposed_name] = fn;

    // Name parameters
    size_t i = 0;
    for (auto& arg : fn->args()) {
        if (i < e.params.size()) {
            arg.setName(e.params[i].name);
        }
        i++;
    }
}

// ── Generate Statements ───────────────────────────────────────────────────

void CodeGen::gen_stmt(const Stmt& s) {
    if (auto* b = dynamic_cast<const BlockStmt*>(&s)) {
        gen_block(*b);
    } else if (auto* f = dynamic_cast<const ForgeStmt*>(&s)) {
        gen_forge(*f);
    } else if (auto* i = dynamic_cast<const IfStmt*>(&s)) {
        gen_if(*i);
    } else if (auto* c = dynamic_cast<const CycleStmt*>(&s)) {
        gen_cycle(*c);
    } else if (auto* r = dynamic_cast<const ReturnStmt*>(&s)) {
        gen_return(*r);
    } else if (auto* fr = dynamic_cast<const FractureStmt*>(&s)) {
        gen_fracture(*fr);
    } else if (auto* ab = dynamic_cast<const AbyssStmt*>(&s)) {
        gen_abyss(*ab);
    } else if (auto* fb = dynamic_cast<const FractalStmt*>(&s)) {
        gen_fractal(*fb);
    } else if (auto* bb = dynamic_cast<const BedrockStmt*>(&s)) {
        gen_bedrock(*bb);
    } else if (auto* w = dynamic_cast<const WhileCycleStmt*>(&s)) {
        gen_while(*w);
    } else if (auto* cs = dynamic_cast<const ConstStmt*>(&s)) {
        gen_const(*cs);
    } else if (auto* ms = dynamic_cast<const MatchStmt*>(&s)) {
        gen_match(*ms);
    } else if (auto* ts = dynamic_cast<const TremorStmt*>(&s)) {
        gen_tremor(*ts);
    } else if (auto* th = dynamic_cast<const ThrowStmt*>(&s)) {
        gen_throw(*th);
    } else if (auto* es = dynamic_cast<const ExprStmt*>(&s)) {
        gen_expr(*es->expr);
    } else if (dynamic_cast<const BreakStmt*>(&s)) {
        if (cycle_exit_bb_) builder_.CreateBr(cycle_exit_bb_);
    } else if (dynamic_cast<const ContinueStmt*>(&s)) {
        if (cycle_header_bb_) builder_.CreateBr(cycle_header_bb_);
    }
}

void CodeGen::predeclare_block_locals(const BlockStmt& s) {
    for (auto& st : s.stmts) {
        if (auto* f = dynamic_cast<const ForgeStmt*>(st.get())) {
            OFSType type = f->type_ann.value_or(OFSType::stone());
            auto* llvm_ty = llvm_type(type);
            if (type.base == BaseType::Named) {
                auto it = struct_types_.find(type.name);
                if (it != struct_types_.end()) {
                    auto* alloca = create_alloca(cur_fn_, f->name, it->second);
                    set_var(f->name, alloca);
                    continue;
                }
            }
            auto* alloca = create_alloca(cur_fn_, f->name, llvm_ty);
            builder_.CreateStore(llvm::Constant::getNullValue(llvm_ty), alloca);
            set_var(f->name, alloca);
        } else if (auto* c = dynamic_cast<const ConstStmt*>(st.get())) {
            OFSType type = c->type_ann.value_or(OFSType::stone());
            auto* llvm_ty = llvm_type(type);
            if (type.base == BaseType::Named) {
                auto it = struct_types_.find(type.name);
                if (it != struct_types_.end()) {
                    auto* alloca = create_alloca(cur_fn_, c->name, it->second);
                    set_var(c->name, alloca);
                    continue;
                }
            }
            auto* alloca = create_alloca(cur_fn_, c->name, llvm_ty);
            builder_.CreateStore(llvm::Constant::getNullValue(llvm_ty), alloca);
            set_var(c->name, alloca);
        }
    }
}

void CodeGen::gen_block(const BlockStmt& s) {
    var_stack_.push_back({});
    predeclare_block_locals(s);
    for (auto& st : s.stmts) {
        gen_stmt(*st);
        // Stop generating after a terminator
        if (builder_.GetInsertBlock()->getTerminator()) break;
    }
    var_stack_.pop_back();
}

void CodeGen::gen_forge(const ForgeStmt& s) {
    OFSType type = s.type_ann.value_or(OFSType::stone());
    auto* llvm_ty = llvm_type(type);
    auto* alloca = get_var(s.name);

    // For named types (monolith), allocate the struct directly
    if (!alloca && type.base == BaseType::Named) {
        auto it = struct_types_.find(type.name);
        if (it != struct_types_.end()) {
            alloca = create_alloca(cur_fn_, s.name, it->second);
            set_var(s.name, alloca);
            return;
        }
    }

    if (!alloca) {
        alloca = create_alloca(cur_fn_, s.name, llvm_ty);
        builder_.CreateStore(llvm::Constant::getNullValue(llvm_ty), alloca);
        set_var(s.name, alloca);
    }

    if (s.initializer) {
        auto* val = gen_expr(*s.initializer);
        if (val) {
            // Handle type coercion
            if (val->getType() != llvm_ty) {
                val = cast_to_type(val, s.initializer->resolved_type, type);
            }
            builder_.CreateStore(val, alloca);
        }
    }
}

void CodeGen::gen_if(const IfStmt& s) {
    auto* cond = gen_expr(*s.cond);
    if (!cond) return;

    // Convert to i1 if needed
    if (!cond->getType()->isIntegerTy(1)) {
        if (cond->getType()->isIntegerTy()) {
            cond = builder_.CreateICmpNE(cond,
                llvm::Constant::getNullValue(cond->getType()), "ifcond");
        } else if (cond->getType()->isDoubleTy()) {
            cond = builder_.CreateFCmpONE(cond,
                llvm::ConstantFP::get(ctx_, llvm::APFloat(0.0)), "ifcond");
        }
    }

    auto* then_bb  = llvm::BasicBlock::Create(ctx_, "then", cur_fn_);
    auto* else_bb  = llvm::BasicBlock::Create(ctx_, "else", cur_fn_);
    auto* merge_bb = llvm::BasicBlock::Create(ctx_, "merge", cur_fn_);

    builder_.CreateCondBr(cond, then_bb, s.else_block ? else_bb : merge_bb);

    // Then block
    builder_.SetInsertPoint(then_bb);
    if (s.then_block) gen_stmt(*s.then_block);
    if (!builder_.GetInsertBlock()->getTerminator())
        builder_.CreateBr(merge_bb);

    // Else block
    builder_.SetInsertPoint(else_bb);
    if (s.else_block) gen_stmt(*s.else_block);
    if (!builder_.GetInsertBlock()->getTerminator())
        builder_.CreateBr(merge_bb);

    builder_.SetInsertPoint(merge_bb);
}

void CodeGen::gen_cycle(const CycleStmt& s) {
    auto* saved_exit   = cycle_exit_bb_;
    auto* saved_header = cycle_header_bb_;

    if (s.is_range) {
        // Range-based cycle: cycle (i in collection)
        auto* arr_val = gen_expr(*s.range_expr);
        auto* len = builder_.CreateCall(get_runtime_fn("ofs_array_len"), {arr_val}, "arr_len");

        // Create loop variable (index)
        auto* idx_alloca = create_alloca(cur_fn_, "__idx", llvm::Type::getInt64Ty(ctx_));
        builder_.CreateStore(llvm::ConstantInt::get(llvm::Type::getInt64Ty(ctx_), 0), idx_alloca);

        // Create variable for the element
        auto* elem_alloca = create_alloca(cur_fn_, s.range_var, llvm::Type::getInt64Ty(ctx_));

        var_stack_.push_back({});
        set_var(s.range_var, elem_alloca);

        auto* cond_bb = llvm::BasicBlock::Create(ctx_, "cycle_cond", cur_fn_);
        auto* body_bb = llvm::BasicBlock::Create(ctx_, "cycle_body", cur_fn_);
        auto* step_bb = llvm::BasicBlock::Create(ctx_, "cycle_step", cur_fn_);
        auto* exit_bb = llvm::BasicBlock::Create(ctx_, "cycle_exit", cur_fn_);

        cycle_exit_bb_ = exit_bb;
        cycle_header_bb_ = step_bb;

        builder_.CreateBr(cond_bb);

        // Condition: idx < len
        builder_.SetInsertPoint(cond_bb);
        auto* idx_val = builder_.CreateLoad(llvm::Type::getInt64Ty(ctx_), idx_alloca, "idx");
        auto* cmp = builder_.CreateICmpSLT(idx_val, len, "range_cmp");
        builder_.CreateCondBr(cmp, body_bb, exit_bb);

        // Body: get element, store to variable
        builder_.SetInsertPoint(body_bb);
        auto* elem_ptr = builder_.CreateCall(get_runtime_fn("ofs_array_get"), {arr_val, idx_val}, "elem_ptr");
        auto* elem_val = builder_.CreateLoad(llvm::Type::getInt64Ty(ctx_),
            builder_.CreateBitCast(elem_ptr, llvm::PointerType::getUnqual(llvm::Type::getInt64Ty(ctx_))),
            "elem");
        builder_.CreateStore(elem_val, elem_alloca);

        if (s.body) gen_stmt(*s.body);
        if (!builder_.GetInsertBlock()->getTerminator())
            builder_.CreateBr(step_bb);

        // Step: idx++
        builder_.SetInsertPoint(step_bb);
        auto* idx_cur = builder_.CreateLoad(llvm::Type::getInt64Ty(ctx_), idx_alloca, "idx_cur");
        auto* idx_next = builder_.CreateAdd(idx_cur, llvm::ConstantInt::get(llvm::Type::getInt64Ty(ctx_), 1), "idx_next");
        builder_.CreateStore(idx_next, idx_alloca);
        builder_.CreateBr(cond_bb);

        builder_.SetInsertPoint(exit_bb);
        var_stack_.pop_back();
    } else {
        // C-style cycle
        var_stack_.push_back({});

        // Init
        if (s.init) gen_stmt(*s.init);

        auto* cond_bb = llvm::BasicBlock::Create(ctx_, "cycle_cond", cur_fn_);
        auto* body_bb = llvm::BasicBlock::Create(ctx_, "cycle_body", cur_fn_);
        auto* step_bb = llvm::BasicBlock::Create(ctx_, "cycle_step", cur_fn_);
        auto* exit_bb = llvm::BasicBlock::Create(ctx_, "cycle_exit", cur_fn_);

        cycle_exit_bb_ = exit_bb;
        cycle_header_bb_ = step_bb;

        builder_.CreateBr(cond_bb);

        // Condition
        builder_.SetInsertPoint(cond_bb);
        if (s.cond) {
            auto* cond = gen_expr(*s.cond);
            if (cond && !cond->getType()->isIntegerTy(1)) {
                if (cond->getType()->isIntegerTy()) {
                    cond = builder_.CreateICmpNE(cond,
                        llvm::Constant::getNullValue(cond->getType()), "cyclecond");
                }
            }
            builder_.CreateCondBr(cond, body_bb, exit_bb);
        } else {
            builder_.CreateBr(body_bb);
        }

        // Body
        builder_.SetInsertPoint(body_bb);
        if (s.body) gen_stmt(*s.body);
        if (!builder_.GetInsertBlock()->getTerminator())
            builder_.CreateBr(step_bb);

        // Step
        builder_.SetInsertPoint(step_bb);
        if (s.step) gen_expr(*s.step);
        builder_.CreateBr(cond_bb);

        builder_.SetInsertPoint(exit_bb);
        var_stack_.pop_back();
    }

    cycle_exit_bb_ = saved_exit;
    cycle_header_bb_ = saved_header;
}

void CodeGen::gen_return(const ReturnStmt& s) {
    if (s.value) {
        // Safety: if function return type is void, discard the value
        if (cur_fn_->getReturnType()->isVoidTy()) {
            gen_expr(*s.value); // evaluate for side effects
            builder_.CreateRetVoid();
            return;
        }
        auto* val = gen_expr(*s.value);
        if (val) {
            auto* ret_ty = cur_fn_->getReturnType();
            if (val->getType() != ret_ty) {
                if (ret_ty->isIntegerTy() && val->getType()->isIntegerTy()) {
                    unsigned src_bits = val->getType()->getIntegerBitWidth();
                    unsigned dst_bits = ret_ty->getIntegerBitWidth();
                    if (src_bits < dst_bits) {
                        if (is_unsigned_type(s.value->resolved_type)) {
                            val = builder_.CreateZExt(val, ret_ty, "ret_zext");
                        } else {
                            val = builder_.CreateSExt(val, ret_ty, "ret_sext");
                        }
                    } else if (src_bits > dst_bits) {
                        val = builder_.CreateTrunc(val, ret_ty, "ret_trunc");
                    }
                } else if (ret_ty->isDoubleTy() && val->getType()->isIntegerTy()) {
                    if (is_unsigned_type(s.value->resolved_type)) {
                        val = builder_.CreateUIToFP(val, ret_ty, "ret_uitofp");
                    } else {
                        val = builder_.CreateSIToFP(val, ret_ty, "ret_sitofp");
                    }
                } else if (ret_ty->isIntegerTy(1) && val->getType()->isIntegerTy()) {
                    // C runtime bool functions return i32; OFS bool = i1. Convert.
                    val = builder_.CreateICmpNE(val,
                        llvm::ConstantInt::get(val->getType(), 0), "to_bool");
                } else if (ret_ty->isIntegerTy() && val->getType()->isDoubleTy()) {
                    val = builder_.CreateFPToSI(val, ret_ty, "ret_fptosi");
                }
            }
            builder_.CreateRet(val);
        } else {
            builder_.CreateRetVoid();
        }
    } else {
        if (cur_fn_->getReturnType()->isVoidTy()) {
            builder_.CreateRetVoid();
        } else {
            builder_.CreateRet(llvm::Constant::getNullValue(cur_fn_->getReturnType()));
        }
    }
}

void CodeGen::gen_fracture(const FractureStmt& s) {
    if (s.body) gen_stmt(*s.body);
}

void CodeGen::gen_abyss(const AbyssStmt& s) {
    if (s.body) gen_stmt(*s.body);
}

void CodeGen::gen_fractal(const FractalStmt& s) {
    if (s.body) gen_stmt(*s.body);
}

void CodeGen::gen_bedrock(const BedrockStmt& s) {
    if (s.body) gen_stmt(*s.body);
}

void CodeGen::gen_while(const WhileCycleStmt& s) {
    auto* saved_exit   = cycle_exit_bb_;
    auto* saved_header = cycle_header_bb_;

    auto* cond_bb = llvm::BasicBlock::Create(ctx_, "while_cond", cur_fn_);
    auto* body_bb = llvm::BasicBlock::Create(ctx_, "while_body", cur_fn_);
    auto* exit_bb = llvm::BasicBlock::Create(ctx_, "while_exit", cur_fn_);

    cycle_exit_bb_ = exit_bb;
    cycle_header_bb_ = cond_bb;

    builder_.CreateBr(cond_bb);

    // Condition
    builder_.SetInsertPoint(cond_bb);
    auto* cond = gen_expr(*s.cond);
    if (!cond) {
        // Fallback: always exit the loop
        builder_.CreateBr(exit_bb);
    } else {
        if (!cond->getType()->isIntegerTy(1)) {
            if (cond->getType()->isIntegerTy()) {
                cond = builder_.CreateICmpNE(cond,
                    llvm::Constant::getNullValue(cond->getType()), "whilecond");
            } else if (cond->getType()->isDoubleTy()) {
                cond = builder_.CreateFCmpONE(cond,
                    llvm::ConstantFP::get(ctx_, llvm::APFloat(0.0)), "whilecond");
            }
        }
        builder_.CreateCondBr(cond, body_bb, exit_bb);
    }

    // Body
    builder_.SetInsertPoint(body_bb);
    if (s.body) gen_stmt(*s.body);
    if (!builder_.GetInsertBlock()->getTerminator())
        builder_.CreateBr(cond_bb);

    builder_.SetInsertPoint(exit_bb);

    cycle_exit_bb_ = saved_exit;
    cycle_header_bb_ = saved_header;
}

void CodeGen::gen_const(const ConstStmt& s) {
    // Backend lowering is equivalent to forge for now; semantic layer tracks const intent.
    OFSType type = s.type_ann.value_or(OFSType::stone());
    auto* llvm_ty = llvm_type(type);
    auto* alloca = get_var(s.name);

    if (!alloca && type.base == BaseType::Named) {
        auto it = struct_types_.find(type.name);
        if (it != struct_types_.end()) {
            alloca = create_alloca(cur_fn_, s.name, it->second);
            set_var(s.name, alloca);
            return;
        }
    }

    if (!alloca) {
        alloca = create_alloca(cur_fn_, s.name, llvm_ty);
        builder_.CreateStore(llvm::Constant::getNullValue(llvm_ty), alloca);
        set_var(s.name, alloca);
    }

    if (s.initializer) {
        auto* val = gen_expr(*s.initializer);
        if (val) {
            if (val->getType() != llvm_ty) {
                val = cast_to_type(val, s.initializer->resolved_type, type);
            }
            builder_.CreateStore(val, alloca);
        }
    }
}

void CodeGen::gen_match(const MatchStmt& s) {
    if (!s.subject) return;

    auto* subject_val = gen_expr(*s.subject);
    if (!subject_val) return;

    auto* merge_bb = llvm::BasicBlock::Create(ctx_, "match_merge", cur_fn_);

    for (size_t i = 0; i < s.arms.size(); i++) {
        const auto& arm = s.arms[i];

        if (arm.is_default) {
            // Default arm — unconditional
            if (arm.body) gen_stmt(*arm.body);
            if (!builder_.GetInsertBlock()->getTerminator())
                builder_.CreateBr(merge_bb);
            // No further arms after default
            break;
        }

        // Generate pattern value
        auto* pattern_val = gen_expr(*arm.pattern);
        if (!pattern_val) continue;

        // Choose comparison strategy based on subject type
        llvm::Value* cmp;
        auto& subj_type = s.subject->resolved_type;
        if (subj_type.base == BaseType::Obsidian) {
            // String comparison via runtime
            auto* eq_i32 = builder_.CreateCall(
                get_runtime_fn("ofs_str_eq"), {subject_val, pattern_val}, "match_str_eq");
            cmp = builder_.CreateICmpNE(eq_i32,
                llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx_), 0), "match_cmp");
        } else if (subject_val->getType()->isDoubleTy()) {
            cmp = builder_.CreateFCmpOEQ(subject_val, pattern_val, "match_cmp");
        } else {
            cmp = builder_.CreateICmpEQ(subject_val, pattern_val, "match_cmp");
        }

        auto* arm_bb  = llvm::BasicBlock::Create(ctx_, "match_arm",  cur_fn_);
        auto* next_bb = llvm::BasicBlock::Create(ctx_, "match_next", cur_fn_);
        builder_.CreateCondBr(cmp, arm_bb, next_bb);

        // Arm body
        builder_.SetInsertPoint(arm_bb);
        if (arm.body) gen_stmt(*arm.body);
        if (!builder_.GetInsertBlock()->getTerminator())
            builder_.CreateBr(merge_bb);

        builder_.SetInsertPoint(next_bb);
    }

    // If nothing terminated the current block, jump to merge
    if (!builder_.GetInsertBlock()->getTerminator())
        builder_.CreateBr(merge_bb);

    builder_.SetInsertPoint(merge_bb);
}

void CodeGen::gen_tremor(const TremorStmt& s) {
    // Stub: OFS exceptions are not lowered yet; run body then catch as no-op fallback.
    if (s.body) gen_stmt(*s.body);
    if (s.catch_body) gen_stmt(*s.catch_body);
}

void CodeGen::gen_throw(const ThrowStmt& s) {
    // Stub: evaluate thrown value for side effects only.
    if (s.value) {
        gen_expr(*s.value);
    }
}

void CodeGen::gen_strata(const StrataDecl& s) {
    (void)s;
    // Stub: enum type metadata is semantic-only for now.
}

void CodeGen::gen_echo(const EchoExpr& e) {
    for (auto& arg : e.args) {
        auto* val = gen_expr(*arg);
        if (!val) continue;

        auto& type = arg->resolved_type;
        switch (type.base) {
            case BaseType::Stone:
                builder_.CreateCall(get_runtime_fn("echo_stone"), {val});
                break;
            case BaseType::U8:
            case BaseType::U16:
            case BaseType::U32:
            case BaseType::U64: {
                auto* ext = builder_.CreateZExtOrTrunc(val, llvm::Type::getInt64Ty(ctx_), "echo_uint_to_i64");
                builder_.CreateCall(get_runtime_fn("echo_stone"), {ext});
                break;
            }
            case BaseType::I8:
            case BaseType::I32: {
                auto* ext = builder_.CreateSExtOrTrunc(val, llvm::Type::getInt64Ty(ctx_), "echo_int_to_i64");
                builder_.CreateCall(get_runtime_fn("echo_stone"), {ext});
                break;
            }
            case BaseType::Crystal:
                builder_.CreateCall(get_runtime_fn("echo_crystal"), {val});
                break;
            case BaseType::Obsidian:
                builder_.CreateCall(get_runtime_fn("echo_obsidian"), {val});
                break;
            case BaseType::Bool: {
                auto* ext = builder_.CreateZExt(val, llvm::Type::getInt32Ty(ctx_), "bool_to_i32");
                builder_.CreateCall(get_runtime_fn("echo_bool"), {ext});
                break;
            }
            default:
                // Default: try to print as stone
                if (val->getType()->isIntegerTy(64)) {
                    builder_.CreateCall(get_runtime_fn("echo_stone"), {val});
                } else if (val->getType()->isDoubleTy()) {
                    builder_.CreateCall(get_runtime_fn("echo_crystal"), {val});
                } else if (val->getType()->isPointerTy()) {
                    builder_.CreateCall(get_runtime_fn("echo_obsidian"), {val});
                }
                break;
        }
    }
}

// ── Generate Expressions ──────────────────────────────────────────────────

llvm::Value* CodeGen::gen_expr(const Expr& e) {
    if (auto* lit = dynamic_cast<const IntLitExpr*>(&e)) {
        return llvm::ConstantInt::get(llvm::Type::getInt64Ty(ctx_), lit->value);
    }
    if (auto* lit = dynamic_cast<const FloatLitExpr*>(&e)) {
        return llvm::ConstantFP::get(llvm::Type::getDoubleTy(ctx_), lit->value);
    }
    if (auto* lit = dynamic_cast<const StringLitExpr*>(&e)) {
        return builder_.CreateGlobalStringPtr(lit->value, "str");
    }
    if (auto* lit = dynamic_cast<const BoolLitExpr*>(&e)) {
        return llvm::ConstantInt::get(llvm::Type::getInt1Ty(ctx_), lit->value ? 1 : 0);
    }
    if (dynamic_cast<const NullLitExpr*>(&e)) {
        return llvm::ConstantPointerNull::get(
            llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(ctx_)));
    }
    if (auto* id = dynamic_cast<const IdentExpr*>(&e)) {
        auto* alloca = get_var(id->name);
        if (!alloca) {
            auto it_fn = functions_.find(id->name);
            if (it_fn == functions_.end()) {
                it_fn = functions_.find(qualify_symbol(id->name));
            }
            if (it_fn != functions_.end()) {
                return it_fn->second;
            }
            std::cerr << "codegen: undefined variable '" << id->name << "'\n";
            return llvm::ConstantInt::get(llvm::Type::getInt64Ty(ctx_), 0);
        }
        // Determine load type
        auto& type = e.resolved_type;
        if (type.base == BaseType::Named) {
            auto* named_ptr_ty = llvm_type(type);

            if (auto* ai = llvm::dyn_cast<llvm::AllocaInst>(alloca)) {
                auto* alloc_ty = ai->getAllocatedType();
                if (alloc_ty == named_ptr_ty) {
                    return builder_.CreateLoad(named_ptr_ty, alloca, id->name + "_val");
                }
                // Local named value is stored as struct on stack; expression value is its address.
                return alloca;
            }

            if (auto* gv = llvm::dyn_cast<llvm::GlobalVariable>(alloca)) {
                if (gv->getValueType() == named_ptr_ty) {
                    return builder_.CreateLoad(named_ptr_ty, alloca, id->name + "_val");
                }
                return alloca;
            }
        }
        auto* load_ty = llvm_type(type);
        return builder_.CreateLoad(load_ty, alloca, id->name + "_val");
    }
    if (auto* bin = dynamic_cast<const BinaryExpr*>(&e)) {
        return gen_binary(*bin);
    }
    if (auto* un = dynamic_cast<const UnaryExpr*>(&e)) {
        return gen_unary(*un);
    }
    if (auto* call = dynamic_cast<const CallExpr*>(&e)) {
        return gen_call(*call);
    }
    if (auto* echo = dynamic_cast<const EchoExpr*>(&e)) {
        gen_echo(*echo);
        return nullptr;
    }
    if (auto* assign = dynamic_cast<const AssignExpr*>(&e)) {
        return gen_assign(*assign);
    }
    if (auto* idx = dynamic_cast<const IndexExpr*>(&e)) {
        return gen_index(*idx);
    }
    if (auto* mem = dynamic_cast<const MemberExpr*>(&e)) {
        return gen_member(*mem);
    }
    if (auto* arr = dynamic_cast<const ArrayLitExpr*>(&e)) {
        return gen_array_lit(*arr);
    }
    if (auto* cast = dynamic_cast<const CastExpr*>(&e)) {
        return gen_cast(*cast);
    }
    if (auto* lambda = dynamic_cast<const LambdaExpr*>(&e)) {
        return gen_lambda(*lambda);
    }
    if (auto* ia = dynamic_cast<const InlineAsmExpr*>(&e)) {
        return gen_inline_asm(*ia);
    }

    return llvm::ConstantInt::get(llvm::Type::getInt64Ty(ctx_), 0);
}

llvm::Value* CodeGen::gen_binary(const BinaryExpr& e) {
    auto* left = gen_expr(*e.left);
    auto* right = gen_expr(*e.right);
    if (!left || !right) return nullptr;

    // String concatenation
    if (e.op == "+" &&
        e.left->resolved_type.base == BaseType::Obsidian &&
        e.right->resolved_type.base == BaseType::Obsidian) {
        return builder_.CreateCall(get_runtime_fn("ofs_str_concat"), {left, right}, "concat");
    }

    // String equality/inequality — must use content comparison, not pointer comparison
    if ((e.op == "==" || e.op == "!=") &&
        e.left->resolved_type.base == BaseType::Obsidian &&
        e.right->resolved_type.base == BaseType::Obsidian) {
        auto* cmp = builder_.CreateCall(get_runtime_fn("ofs_str_eq"), {left, right}, "str_eq");
        if (e.op == "!=") {
            return builder_.CreateICmpEQ(cmp,
                llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx_), 0), "str_ne");
        }
        return builder_.CreateICmpNE(cmp,
            llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx_), 0), "str_eq_bool");
    }

    // Type promotion: if one is float and one is int, promote int to float
    bool is_float = left->getType()->isDoubleTy() || right->getType()->isDoubleTy();
    if (is_float) {
        OFSType f64 = OFSType::crystal();
        if (left->getType()->isIntegerTy()) {
            left = cast_to_type(left, e.left->resolved_type, f64);
        }
        if (right->getType()->isIntegerTy()) {
            right = cast_to_type(right, e.right->resolved_type, f64);
        }
    } else if (left->getType()->isIntegerTy() && right->getType()->isIntegerTy()) {
        unsigned lbits = left->getType()->getIntegerBitWidth();
        unsigned rbits = right->getType()->getIntegerBitWidth();
        if (lbits < rbits) {
            if (is_unsigned_type(e.left->resolved_type)) left = builder_.CreateZExt(left, right->getType(), "zext_l");
            else left = builder_.CreateSExt(left, right->getType(), "sext_l");
        } else if (rbits < lbits) {
            if (is_unsigned_type(e.right->resolved_type)) right = builder_.CreateZExt(right, left->getType(), "zext_r");
            else right = builder_.CreateSExt(right, left->getType(), "sext_r");
        }
    }

    // Arithmetic
    if (is_float) {
        if (e.op == "+") return builder_.CreateFAdd(left, right, "fadd");
        if (e.op == "-") return builder_.CreateFSub(left, right, "fsub");
        if (e.op == "*") return builder_.CreateFMul(left, right, "fmul");
        if (e.op == "/") return builder_.CreateFDiv(left, right, "fdiv");
        if (e.op == "%") return builder_.CreateFRem(left, right, "fmod");
        if (e.op == "==") return builder_.CreateFCmpOEQ(left, right, "feq");
        if (e.op == "!=") return builder_.CreateFCmpONE(left, right, "fne");
        if (e.op == "<")  return builder_.CreateFCmpOLT(left, right, "flt");
        if (e.op == "<=") return builder_.CreateFCmpOLE(left, right, "fle");
        if (e.op == ">")  return builder_.CreateFCmpOGT(left, right, "fgt");
        if (e.op == ">=") return builder_.CreateFCmpOGE(left, right, "fge");
    } else {
        bool unsigned_int_op = is_unsigned_type(e.left->resolved_type) &&
                               is_unsigned_type(e.right->resolved_type);

        if (e.op == "+") return builder_.CreateAdd(left, right, "add");
        if (e.op == "-") return builder_.CreateSub(left, right, "sub");
        if (e.op == "*") return builder_.CreateMul(left, right, "mul");
        if (e.op == "/") return unsigned_int_op ? builder_.CreateUDiv(left, right, "udiv")
                                                 : builder_.CreateSDiv(left, right, "sdiv");
        if (e.op == "%") return unsigned_int_op ? builder_.CreateURem(left, right, "umod")
                                                 : builder_.CreateSRem(left, right, "smod");
        if (e.op == "&") return builder_.CreateAnd(left, right, "band");
        if (e.op == "|") return builder_.CreateOr(left, right, "bor");
        if (e.op == "^") return builder_.CreateXor(left, right, "bxor");
        if (e.op == "<<") return builder_.CreateShl(left, right, "shl");
        if (e.op == ">>") return is_unsigned_type(e.left->resolved_type)
                                 ? builder_.CreateLShr(left, right, "lshr")
                                 : builder_.CreateAShr(left, right, "ashr");
        if (e.op == "==") return builder_.CreateICmpEQ(left, right, "eq");
        if (e.op == "!=") return builder_.CreateICmpNE(left, right, "ne");
        if (e.op == "<")  return unsigned_int_op ? builder_.CreateICmpULT(left, right, "ult")
                                                   : builder_.CreateICmpSLT(left, right, "slt");
        if (e.op == "<=") return unsigned_int_op ? builder_.CreateICmpULE(left, right, "ule")
                                                   : builder_.CreateICmpSLE(left, right, "sle");
        if (e.op == ">")  return unsigned_int_op ? builder_.CreateICmpUGT(left, right, "ugt")
                                                   : builder_.CreateICmpSGT(left, right, "sgt");
        if (e.op == ">=") return unsigned_int_op ? builder_.CreateICmpUGE(left, right, "uge")
                                                   : builder_.CreateICmpSGE(left, right, "sge");
    }

    // Logical
    if (e.op == "&&") return builder_.CreateAnd(left, right, "and");
    if (e.op == "||") return builder_.CreateOr(left, right, "or");

    return nullptr;
}

llvm::Value* CodeGen::gen_unary(const UnaryExpr& e) {
    if (e.op == "&") {
        // Address-of
        return gen_lvalue(*e.operand);
    }

    auto* val = gen_expr(*e.operand);
    if (!val) return nullptr;

    if (e.op == "-") {
        if (val->getType()->isDoubleTy())
            return builder_.CreateFNeg(val, "fneg");
        return builder_.CreateNeg(val, "neg");
    }
    if (e.op == "!") {
        if (val->getType()->isIntegerTy(1))
            return builder_.CreateNot(val, "not");
        return builder_.CreateICmpEQ(val, llvm::Constant::getNullValue(val->getType()), "not");
    }
    if (e.op == "~") {
        return builder_.CreateNot(val, "bitnot");
    }
    if (e.op == "*") {
        // Dereference
        auto& type = e.resolved_type;
        return builder_.CreateLoad(llvm_type(type), val, "deref");
    }
    if (e.op == "++" || e.op == "--") {
        auto* lval = gen_lvalue(*e.operand);
        if (!lval) return val;
        auto* loaded = builder_.CreateLoad(val->getType(), lval, "inc_val");
        llvm::Value* result;
        if (loaded->getType()->isDoubleTy()) {
            auto* one = llvm::ConstantFP::get(ctx_, llvm::APFloat(1.0));
            result = (e.op == "++") ? builder_.CreateFAdd(loaded, one, "inc")
                                    : builder_.CreateFSub(loaded, one, "dec");
        } else {
            auto* one = llvm::ConstantInt::get(loaded->getType(), 1);
            result = (e.op == "++") ? builder_.CreateAdd(loaded, one, "inc")
                                    : builder_.CreateSub(loaded, one, "dec");
        }
        builder_.CreateStore(result, lval);
        return e.prefix ? result : loaded;
    }

    return val;
}

llvm::Value* CodeGen::gen_call(const CallExpr& e) {
    auto* id = dynamic_cast<const IdentExpr*>(e.callee.get());
    if (id && is_fault_intrinsic_name(id->name)) {
        if (id->name == "fault_fence") {
            builder_.CreateFence(llvm::AtomicOrdering::SequentiallyConsistent);
            return nullptr;
        }
        if (id->name == "fault_prefetch") {
            if (e.args.size() != 1) return nullptr;
            auto* addr = gen_expr(*e.args[0]);
            if (!addr || !addr->getType()->isPointerTy()) return nullptr;
            auto* i8_ptr = builder_.CreateBitCast(addr, llvm::PointerType::getUnqual(builder_.getInt8Ty()), "fault_prefetch_ptr");
            std::vector<llvm::Type*> prefetch_types = {i8_ptr->getType()};
#if LLVM_VERSION_MAJOR >= 20
            auto* fn = llvm::Intrinsic::getOrInsertDeclaration(mod_.get(), llvm::Intrinsic::prefetch, prefetch_types);
#else
            auto* fn = llvm::Intrinsic::getDeclaration(mod_.get(), llvm::Intrinsic::prefetch, prefetch_types);
#endif
            std::vector<llvm::Value*> prefetch_args = {
                i8_ptr,
                llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx_), 0),
                llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx_), 3),
                llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx_), 1)
            };
            builder_.CreateCall(fn, prefetch_args);
            return nullptr;
        }
        if (id->name == "fault_trap") {
#if LLVM_VERSION_MAJOR >= 20
            auto* fn = llvm::Intrinsic::getOrInsertDeclaration(mod_.get(), llvm::Intrinsic::trap);
#else
            auto* fn = llvm::Intrinsic::getDeclaration(mod_.get(), llvm::Intrinsic::trap);
#endif
            builder_.CreateCall(fn, {});
            return nullptr;
        }
        if (id->name == "fault_unreachable") {
#if LLVM_VERSION_MAJOR >= 20
            auto* fn = llvm::Intrinsic::getOrInsertDeclaration(mod_.get(), llvm::Intrinsic::trap);
#else
            auto* fn = llvm::Intrinsic::getDeclaration(mod_.get(), llvm::Intrinsic::trap);
#endif
            builder_.CreateCall(fn, {});
            builder_.CreateUnreachable();
            return nullptr;
        }
        if (id->name == "fault_memcpy") {
            if (e.args.size() != 3) return nullptr;
            auto* dst = gen_expr(*e.args[0]);
            auto* src = gen_expr(*e.args[1]);
            auto* len = gen_expr(*e.args[2]);
            if (!dst || !src || !len) return nullptr;
            // Cast pointers to i8* for memcpy
            auto* i8ptr = llvm::PointerType::getUnqual(builder_.getInt8Ty());
            auto* dst_i8 = builder_.CreateBitCast(dst, i8ptr, "memcpy_dst");
            auto* src_i8 = builder_.CreateBitCast(src, i8ptr, "memcpy_src");
            if (!len->getType()->isIntegerTy(64)) return nullptr;
            std::vector<llvm::Type*> memcpy_types = {i8ptr, i8ptr, llvm::Type::getInt64Ty(ctx_)};
#if LLVM_VERSION_MAJOR >= 20
            auto* fn = llvm::Intrinsic::getOrInsertDeclaration(mod_.get(), llvm::Intrinsic::memcpy, memcpy_types);
#else
            auto* fn = llvm::Intrinsic::getDeclaration(mod_.get(), llvm::Intrinsic::memcpy, memcpy_types);
#endif
            builder_.CreateCall(fn, {dst_i8, src_i8, len,
                llvm::ConstantInt::get(llvm::Type::getInt1Ty(ctx_), 0)});
            return nullptr;
        }
        if (id->name == "fault_memset") {
            if (e.args.size() != 3) return nullptr;
            auto* dst = gen_expr(*e.args[0]);
            auto* val = gen_expr(*e.args[1]);
            auto* len = gen_expr(*e.args[2]);
            if (!dst || !val || !len) return nullptr;
            auto* i8ptr = llvm::PointerType::getUnqual(builder_.getInt8Ty());
            auto* dst_i8 = builder_.CreateBitCast(dst, i8ptr, "memset_dst");
            // val must fit in i8
            auto* val_i8 = builder_.CreateTrunc(val, llvm::Type::getInt8Ty(ctx_), "memset_val");
            std::vector<llvm::Type*> memset_types = {i8ptr, llvm::Type::getInt64Ty(ctx_)};
#if LLVM_VERSION_MAJOR >= 20
            auto* fn = llvm::Intrinsic::getOrInsertDeclaration(mod_.get(), llvm::Intrinsic::memset, memset_types);
#else
            auto* fn = llvm::Intrinsic::getDeclaration(mod_.get(), llvm::Intrinsic::memset, memset_types);
#endif
            builder_.CreateCall(fn, {dst_i8, val_i8, len,
                llvm::ConstantInt::get(llvm::Type::getInt1Ty(ctx_), 0)});
            return nullptr;
        }
        if (id->name == "fault_step") {
            if (e.args.size() != 2) return nullptr;
            auto* base_ptr = gen_expr(*e.args[0]);
            auto* offset = gen_expr(*e.args[1]);
            if (!base_ptr || !offset || !base_ptr->getType()->isPointerTy()) return nullptr;
            if (!offset->getType()->isIntegerTy(64)) return nullptr;
            if (e.args[0]->resolved_type.base != BaseType::Shard || !e.args[0]->resolved_type.inner) {
                return nullptr;
            }
            auto* elem_ty = llvm_type(*e.args[0]->resolved_type.inner);
            return builder_.CreateGEP(elem_ty, base_ptr, {offset}, "fault_step");
        }

        std::vector<llvm::Value*> args;
        for (auto& arg : e.args) {
            auto* arg_val = gen_expr(*arg);
            if (!arg_val) return nullptr;
            if (!arg_val->getType()->isIntegerTy(64)) {
                if (arg_val->getType()->isIntegerTy(1)) {
                    arg_val = builder_.CreateZExt(arg_val, llvm::Type::getInt64Ty(ctx_), "fault_zext");
                } else {
                    return nullptr;
                }
            }
            args.push_back(arg_val);
        }

        if (id->name == "fault_count") {
#if LLVM_VERSION_MAJOR >= 20
            auto* fn = llvm::Intrinsic::getOrInsertDeclaration(mod_.get(), llvm::Intrinsic::ctpop, {llvm::Type::getInt64Ty(ctx_)});
#else
            auto* fn = llvm::Intrinsic::getDeclaration(mod_.get(), llvm::Intrinsic::ctpop, {llvm::Type::getInt64Ty(ctx_)});
#endif
            return builder_.CreateCall(fn, {args[0]}, "fault_count");
        }
        if (id->name == "fault_lead") {
#if LLVM_VERSION_MAJOR >= 20
            auto* fn = llvm::Intrinsic::getOrInsertDeclaration(mod_.get(), llvm::Intrinsic::ctlz, {llvm::Type::getInt64Ty(ctx_)});
#else
            auto* fn = llvm::Intrinsic::getDeclaration(mod_.get(), llvm::Intrinsic::ctlz, {llvm::Type::getInt64Ty(ctx_)});
#endif
            return builder_.CreateCall(fn, {args[0], llvm::ConstantInt::get(llvm::Type::getInt1Ty(ctx_), 0)}, "fault_lead");
        }
        if (id->name == "fault_trail") {
#if LLVM_VERSION_MAJOR >= 20
            auto* fn = llvm::Intrinsic::getOrInsertDeclaration(mod_.get(), llvm::Intrinsic::cttz, {llvm::Type::getInt64Ty(ctx_)});
#else
            auto* fn = llvm::Intrinsic::getDeclaration(mod_.get(), llvm::Intrinsic::cttz, {llvm::Type::getInt64Ty(ctx_)});
#endif
            return builder_.CreateCall(fn, {args[0], llvm::ConstantInt::get(llvm::Type::getInt1Ty(ctx_), 0)}, "fault_trail");
        }
        if (id->name == "fault_swap") {
#if LLVM_VERSION_MAJOR >= 20
            auto* fn = llvm::Intrinsic::getOrInsertDeclaration(mod_.get(), llvm::Intrinsic::bswap, {llvm::Type::getInt64Ty(ctx_)});
#else
            auto* fn = llvm::Intrinsic::getDeclaration(mod_.get(), llvm::Intrinsic::bswap, {llvm::Type::getInt64Ty(ctx_)});
#endif
            return builder_.CreateCall(fn, {args[0]}, "fault_swap");
        }
        if (id->name == "fault_spin_left") {
#if LLVM_VERSION_MAJOR >= 20
            auto* fn = llvm::Intrinsic::getOrInsertDeclaration(mod_.get(), llvm::Intrinsic::fshl, {llvm::Type::getInt64Ty(ctx_)});
#else
            auto* fn = llvm::Intrinsic::getDeclaration(mod_.get(), llvm::Intrinsic::fshl, {llvm::Type::getInt64Ty(ctx_)});
#endif
            return builder_.CreateCall(fn, {args[0], args[0], args[1]}, "fault_spin_left");
        }
        if (id->name == "fault_spin_right") {
#if LLVM_VERSION_MAJOR >= 20
            auto* fn = llvm::Intrinsic::getOrInsertDeclaration(mod_.get(), llvm::Intrinsic::fshr, {llvm::Type::getInt64Ty(ctx_)});
#else
            auto* fn = llvm::Intrinsic::getDeclaration(mod_.get(), llvm::Intrinsic::fshr, {llvm::Type::getInt64Ty(ctx_)});
#endif
            return builder_.CreateCall(fn, {args[0], args[0], args[1]}, "fault_spin_right");
        }
        if (id->name == "fault_cut") {
            auto* shifted = builder_.CreateLShr(args[0], args[1], "fault_cut_shift");
            auto* one = llvm::ConstantInt::get(llvm::Type::getInt64Ty(ctx_), 1);
            auto* width_mask = builder_.CreateSub(builder_.CreateShl(one, args[2], "fault_cut_span"), one, "fault_cut_mask");
            return builder_.CreateAnd(shifted, width_mask, "fault_cut");
        }
        if (id->name == "fault_patch") {
            auto* one = llvm::ConstantInt::get(llvm::Type::getInt64Ty(ctx_), 1);
            auto* width_mask = builder_.CreateSub(builder_.CreateShl(one, args[2], "fault_patch_span"), one, "fault_patch_mask");
            auto* field_mask = builder_.CreateShl(width_mask, args[1], "fault_patch_field_mask");
            auto* cleared = builder_.CreateAnd(args[0], builder_.CreateNot(field_mask, "fault_patch_not_mask"), "fault_patch_cleared");
            auto* inserted = builder_.CreateShl(builder_.CreateAnd(args[3], width_mask, "fault_patch_trim"), args[1], "fault_patch_inserted");
            return builder_.CreateOr(cleared, inserted, "fault_patch");
        }
        if (id->name == "fault_weave") {
            auto* left = builder_.CreateAnd(args[1], args[0], "fault_weave_left");
            auto* mask_not = builder_.CreateNot(args[0], "fault_weave_not");
            auto* right = builder_.CreateAnd(args[2], mask_not, "fault_weave_right");
            return builder_.CreateOr(left, right, "fault_weave");
        }
    }

    llvm::Function* fn = nullptr;
    std::string call_name;

    if (id) {
        call_name = id->name;
        auto it = functions_.find(id->name);
        if (it != functions_.end()) fn = it->second;

        if (!fn) {
            auto q = functions_.find(qualify_symbol(id->name));
            if (q != functions_.end()) fn = q->second;
        }

        if (!fn) fn = get_runtime_fn(id->name);
    }

    // Method call: obj.method(...)
    if (!fn) {
        if (auto* mem = dynamic_cast<const MemberExpr*>(e.callee.get())) {
            if (auto* recv_id = dynamic_cast<const IdentExpr*>(mem->object.get())) {
                // namespace function call: ns.func(...)
                std::string ns_name = recv_id->name + "::" + mem->field;
                auto it_ns = functions_.find(ns_name);
                if (it_ns != functions_.end()) {
                    fn = it_ns->second;
                    call_name = ns_name;
                }
            }

            if (!fn && mem->object->resolved_type.base == BaseType::Named) {
                std::string key = mem->object->resolved_type.name + "::" + mem->field;
                auto it_m = method_functions_.find(key);
                if (it_m != method_functions_.end()) {
                    fn = it_m->second;
                    call_name = mangle_method_name(mem->object->resolved_type.name, mem->field);
                }
            }
        }
    }

    // Function pointer call
    if (!fn) {
        auto* callee_val = gen_expr(*e.callee);
        if (callee_val && e.callee->resolved_type.base == BaseType::Function) {
            auto* callee_ft = llvm_function_type(e.callee->resolved_type);
            if (!callee_ft) return nullptr;
            std::vector<llvm::Value*> args;
            for (size_t i = 0; i < e.args.size(); i++) {
                auto* arg_val = gen_expr(*e.args[i]);
                if (!arg_val) return nullptr;
                auto* param_ty = callee_ft->getParamType(static_cast<unsigned>(i));
                if (arg_val->getType() != param_ty) {
                    if (arg_val->getType()->isIntegerTy() && param_ty->isIntegerTy()) {
                        unsigned src_bits = arg_val->getType()->getIntegerBitWidth();
                        unsigned dst_bits = param_ty->getIntegerBitWidth();
                        if (src_bits < dst_bits) arg_val = builder_.CreateSExt(arg_val, param_ty, "fp_arg_sext");
                        else if (src_bits > dst_bits) arg_val = builder_.CreateTrunc(arg_val, param_ty, "fp_arg_trunc");
                    } else if (param_ty->isDoubleTy() && arg_val->getType()->isIntegerTy()) {
                        arg_val = builder_.CreateSIToFP(arg_val, param_ty, "fp_arg_to_f");
                    } else if (param_ty->isIntegerTy() && arg_val->getType()->isDoubleTy()) {
                        arg_val = builder_.CreateFPToSI(arg_val, param_ty, "fp_arg_to_i");
                    }
                }
                args.push_back(arg_val);
            }

            if (callee_val->getType()->isPointerTy() && callee_val->getType() != llvm::PointerType::getUnqual(callee_ft)) {
                callee_val = builder_.CreateBitCast(callee_val, llvm::PointerType::getUnqual(callee_ft), "fnptr_cast");
            }

            if (callee_ft->getReturnType()->isVoidTy()) {
                builder_.CreateCall(callee_ft, callee_val, args);
                return nullptr;
            }
            return builder_.CreateCall(callee_ft, callee_val, args, "call_fnptr");
        }
    }

    if (!fn) {
        std::cerr << "codegen: undefined function call\n";
        return nullptr;
    }

    std::vector<llvm::Value*> args;
    size_t arg_offset = 0;

    if (auto* mem = dynamic_cast<const MemberExpr*>(e.callee.get())) {
        std::string key = mem->object->resolved_type.name + "::" + mem->field;
        if (method_functions_.find(key) != method_functions_.end()) {
            auto* self_ptr = gen_lvalue(*mem->object);
            if (self_ptr) {
                args.push_back(self_ptr);
                arg_offset = 1;
            }
        }
    }

    for (size_t i = 0; i < e.args.size(); i++) {
        auto* arg_val = gen_expr(*e.args[i]);
        if (arg_val) {
            // Type coercion for parameters
            size_t param_index = i + arg_offset;
            if (param_index < fn->arg_size()) {
                auto* param_ty = fn->getFunctionType()->getParamType(static_cast<unsigned>(param_index));
                if (arg_val->getType() != param_ty) {
                    if (arg_val->getType()->isIntegerTy() && param_ty->isIntegerTy()) {
                        unsigned src_bits = arg_val->getType()->getIntegerBitWidth();
                        unsigned dst_bits = param_ty->getIntegerBitWidth();
                        if (src_bits < dst_bits) {
                            if (is_unsigned_type(e.args[i]->resolved_type)) {
                                arg_val = builder_.CreateZExt(arg_val, param_ty, "arg_zext");
                            } else {
                                arg_val = builder_.CreateSExt(arg_val, param_ty, "arg_sext");
                            }
                        } else if (src_bits > dst_bits) {
                            arg_val = builder_.CreateTrunc(arg_val, param_ty, "arg_trunc");
                        }
                    } else if (param_ty->isDoubleTy() && arg_val->getType()->isIntegerTy()) {
                        if (is_unsigned_type(e.args[i]->resolved_type)) {
                            arg_val = builder_.CreateUIToFP(arg_val, param_ty, "arg_uitofp");
                        } else {
                            arg_val = builder_.CreateSIToFP(arg_val, param_ty, "arg_sitofp");
                        }
                    } else if (param_ty->isIntegerTy() && arg_val->getType()->isDoubleTy()) {
                        arg_val = builder_.CreateFPToSI(arg_val, param_ty, "arg_fptosi");
                    }
                }
            }
            args.push_back(arg_val);
        }
    }

    if (fn->getReturnType()->isVoidTy()) {
        builder_.CreateCall(fn, args);
        return nullptr;
    }
    if (call_name.empty() && id) call_name = id->name;
    if (call_name.empty()) call_name = "fn";
    return builder_.CreateCall(fn, args, "call_" + call_name);
}

llvm::Value* CodeGen::gen_assign(const AssignExpr& e) {
    auto* lval = gen_lvalue(*e.target);
    if (!lval) return nullptr;

    auto* rval = gen_expr(*e.value);
    if (!rval) return nullptr;

    if (e.op == "=") {
        // Type coercion
        auto& target_type = e.target->resolved_type;
        auto* store_ty = llvm_type(target_type);
        if (rval->getType() != store_ty) {
            rval = cast_to_type(rval, e.value->resolved_type, target_type);
        }
        builder_.CreateStore(rval, lval);
    } else {
        // Compound assignment
        auto& target_type = e.target->resolved_type;
        auto* store_ty = llvm_type(target_type);
        auto* cur = builder_.CreateLoad(store_ty, lval, "cur");

        llvm::Value* result = nullptr;
        bool is_float = cur->getType()->isDoubleTy();

        // Promote rval if needed
        if (rval->getType() != cur->getType()) {
            rval = cast_to_type(rval, e.value->resolved_type, target_type);
        }

        if (e.op == "+=") {
            if (target_type.base == BaseType::Obsidian) {
                result = builder_.CreateCall(get_runtime_fn("ofs_str_concat"), {cur, rval}, "concat");
            } else if (is_float) {
                result = builder_.CreateFAdd(cur, rval, "fadd");
            } else {
                result = builder_.CreateAdd(cur, rval, "add");
            }
        } else if (e.op == "-=") {
            result = is_float ? builder_.CreateFSub(cur, rval, "fsub")
                              : builder_.CreateSub(cur, rval, "sub");
        } else if (e.op == "*=") {
            result = is_float ? builder_.CreateFMul(cur, rval, "fmul")
                              : builder_.CreateMul(cur, rval, "mul");
        } else if (e.op == "/=") {
            result = is_float ? builder_.CreateFDiv(cur, rval, "fdiv")
                              : builder_.CreateSDiv(cur, rval, "div");
        } else if (e.op == "%=") {
            result = is_float ? builder_.CreateFRem(cur, rval, "frem")
                              : builder_.CreateSRem(cur, rval, "mod");
        }

        if (result) builder_.CreateStore(result, lval);
    }

    return rval;
}

llvm::Value* CodeGen::gen_index(const IndexExpr& e) {
    auto* obj = gen_expr(*e.object);
    auto* idx = gen_expr(*e.index);
    if (!obj || !idx) return nullptr;

    // Array index via runtime
    auto* elem_ptr = builder_.CreateCall(get_runtime_fn("ofs_array_get"), {obj, idx}, "elem_ptr");

    // Load the value based on resolved type
    auto& type = e.resolved_type;
    auto* load_ty = llvm_type(type);

    // Cast the pointer and load
    auto* typed_ptr = builder_.CreateBitCast(elem_ptr,
        llvm::PointerType::getUnqual(load_ty), "typed_ptr");
    return builder_.CreateLoad(load_ty, typed_ptr, "elem");
}

llvm::Value* CodeGen::gen_member(const MemberExpr& e) {
    auto& obj_type = e.object->resolved_type;

    if (obj_type.base == BaseType::Named) {
        auto it = struct_types_.find(obj_type.name);
        auto fit = struct_fields_.find(obj_type.name);
        if (it != struct_types_.end() && fit != struct_fields_.end()) {
            auto* struct_ty = it->second;
            auto& fields = fit->second;

            // Find field index
            int idx = -1;
            for (size_t i = 0; i < fields.size(); i++) {
                if (fields[i] == e.field) { idx = (int)i; break; }
            }
            if (idx < 0) return nullptr;

            auto* obj_ptr = gen_lvalue(*e.object);
            if (!obj_ptr) return nullptr;

            auto* gep = builder_.CreateStructGEP(struct_ty, obj_ptr, idx, e.field + "_ptr");
            return builder_.CreateLoad(struct_ty->getElementType(idx), gep, e.field);
        }
    }

    return nullptr;
}

llvm::Value* CodeGen::gen_array_lit(const ArrayLitExpr& e) {
    int64_t count = (int64_t)e.elements.size();
    int64_t elem_size = 8; // default i64 size

    auto* arr = builder_.CreateCall(get_runtime_fn("ofs_array_new"),
        {llvm::ConstantInt::get(llvm::Type::getInt64Ty(ctx_), count),
         llvm::ConstantInt::get(llvm::Type::getInt64Ty(ctx_), elem_size)},
        "arr");

    // Push each element
    for (auto& elem : e.elements) {
        auto* val = gen_expr(*elem);
        if (!val) continue;

        // Store value to temp alloca, then push pointer
        auto* tmp = create_alloca(cur_fn_, "tmp_elem", val->getType());
        builder_.CreateStore(val, tmp);
        auto* ptr = builder_.CreateBitCast(tmp,
            llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(ctx_)), "elem_ptr");
        builder_.CreateCall(get_runtime_fn("ofs_array_push"), {arr, ptr});
    }

    return arr;
}

llvm::Value* CodeGen::gen_cast(const CastExpr& e) {
    auto* val = gen_expr(*e.expr);
    if (!val) return nullptr;
    return cast_to_type(val, e.expr->resolved_type, e.target_type);
}

llvm::Value* CodeGen::gen_lambda(const LambdaExpr& e) {
    std::string lambda_name = "__lambda_" + std::to_string(lambda_counter_++);

    std::vector<llvm::Type*> param_types;
    for (const auto& p : e.params) {
        param_types.push_back(llvm_type(p.type));
    }
    llvm::Type* ret_ty = llvm_type(e.return_type);
    auto* ft = llvm::FunctionType::get(ret_ty, param_types, false);
    auto* fn = llvm::Function::Create(ft, llvm::Function::InternalLinkage, lambda_name, mod_.get());

    size_t i = 0;
    for (auto& arg : fn->args()) {
        arg.setName(e.params[i].name);
        i++;
    }

    auto* prev_fn = cur_fn_;
    auto prev_ip = builder_.GetInsertPoint();
    auto* prev_bb = builder_.GetInsertBlock();

    auto* entry = llvm::BasicBlock::Create(ctx_, "entry", fn);
    builder_.SetInsertPoint(entry);
    cur_fn_ = fn;
    var_stack_.push_back({});

    i = 0;
    for (auto& arg : fn->args()) {
        auto* alloca = create_alloca(fn, e.params[i].name, arg.getType());
        builder_.CreateStore(&arg, alloca);
        set_var(e.params[i].name, alloca);
        i++;
    }

    if (e.body) {
        gen_stmt(*e.body);
    }

    if (!builder_.GetInsertBlock()->getTerminator()) {
        if (e.return_type.base == BaseType::Void) {
            builder_.CreateRetVoid();
        } else {
            builder_.CreateRet(llvm::Constant::getNullValue(ret_ty));
        }
    }

    var_stack_.pop_back();
    cur_fn_ = prev_fn;
    if (prev_bb) {
        builder_.SetInsertPoint(prev_bb, prev_ip);
    }

    return fn;
}

// ── LValue generation ─────────────────────────────────────────────────────

llvm::Value* CodeGen::gen_lvalue(const Expr& e) {
    if (auto* id = dynamic_cast<const IdentExpr*>(&e)) {
        auto* slot = get_var(id->name);
        if (!slot) return nullptr;
        if (e.resolved_type.base == BaseType::Named) {
            auto* ptr_ty = llvm_type(e.resolved_type);

            if (auto* ai = llvm::dyn_cast<llvm::AllocaInst>(slot)) {
                if (ai->getAllocatedType() == ptr_ty) {
                    return builder_.CreateLoad(ptr_ty, slot, id->name + "_addr");
                }
                return slot;
            }

            if (auto* gv = llvm::dyn_cast<llvm::GlobalVariable>(slot)) {
                if (gv->getValueType() == ptr_ty) {
                    return builder_.CreateLoad(ptr_ty, slot, id->name + "_addr");
                }
                return slot;
            }
        }
        return slot;
    }
    if (auto* mem = dynamic_cast<const MemberExpr*>(&e)) {
        auto& obj_type = mem->object->resolved_type;
        if (obj_type.base == BaseType::Named) {
            auto it = struct_types_.find(obj_type.name);
            auto fit = struct_fields_.find(obj_type.name);
            if (it != struct_types_.end() && fit != struct_fields_.end()) {
                auto* struct_ty = it->second;
                auto& fields = fit->second;
                int idx = -1;
                for (size_t i = 0; i < fields.size(); i++) {
                    if (fields[i] == mem->field) { idx = (int)i; break; }
                }
                if (idx < 0) return nullptr;
                auto* obj_ptr = gen_lvalue(*mem->object);
                if (!obj_ptr) return nullptr;
                return builder_.CreateStructGEP(struct_ty, obj_ptr, idx, mem->field + "_ptr");
            }
        }
    }
    if (auto* un = dynamic_cast<const UnaryExpr*>(&e)) {
        if (un->op == "*") {
            // Dereference: the address is the value of the pointer
            return gen_expr(*un->operand);
        }
    }
    if (auto* idx = dynamic_cast<const IndexExpr*>(&e)) {
        auto* obj = gen_expr(*idx->object);
        auto* index = gen_expr(*idx->index);
        if (!obj || !index) return nullptr;
        auto* elem_ptr = builder_.CreateCall(get_runtime_fn("ofs_array_get"), {obj, index}, "elem_ptr");
        auto& type = e.resolved_type;
        auto* load_ty = llvm_type(type);
        return builder_.CreateBitCast(elem_ptr, llvm::PointerType::getUnqual(load_ty), "typed_ptr");
    }
    return nullptr;
}

llvm::Value* CodeGen::gen_inline_asm(const InlineAsmExpr& e) {
    // Collect input values
    std::vector<llvm::Type*> input_types;
    std::vector<llvm::Value*> input_vals;
    for (auto& inp : e.inputs) {
        auto* val = gen_expr(*inp);
        if (val) {
            input_types.push_back(val->getType());
            input_vals.push_back(val);
        }
    }

    // Build constraint string: outputs, then inputs (register), then clobbers
    std::string constraints;
    for (auto& out : e.outputs) {
        if (!constraints.empty()) constraints += ",";
        constraints += out;
    }
    for (size_t i = 0; i < input_vals.size(); i++) {
        if (!constraints.empty()) constraints += ",";
        constraints += "r";
    }
    if (!e.clobbers.empty()) {
        if (!constraints.empty()) constraints += ",";
        constraints += "~{" + e.clobbers + "}";
    }

    auto* fn_ty = llvm::FunctionType::get(llvm::Type::getVoidTy(ctx_), input_types, false);
    auto* ia = llvm::InlineAsm::get(fn_ty, e.asm_code, constraints, e.volatile_);
    builder_.CreateCall(ia, input_vals);
    return nullptr;
}

} // namespace ofs
