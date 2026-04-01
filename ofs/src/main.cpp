#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstdlib>
#include <unistd.h>
#include "lexer/lexer.hpp"
#include "parser/parser.hpp"
#include "semantic/semantic.hpp"
#include "codegen/codegen.hpp"
#include "ast/ast_printer.hpp"

static const char* OFS_VERSION = "1.0.0";

void print_usage() {
    std::cout << R"(
ofs — Obsidian Fault Script compiler v)" << OFS_VERSION << R"(

Usage:
  ofs <file.ofs>                        Run a script directly (like python)
  ofs run    <file.ofs>                 Compile and run immediately
  ofs build  <file.ofs> [-o output]     Compile to native executable
  ofs check  <file.ofs>                 Type-check only (no output)
  ofs tokens <file.ofs>                 Print token stream (debug)
  ofs ast    <file.ofs>                 Print AST (debug)
  ofs ir     <file.ofs>                 Emit LLVM IR (debug)
  ofs version                           Print compiler version
  ofs help                              Show this help message

Examples:
  ofs hello.ofs                         Run hello.ofs directly
  ofs build hello.ofs -o hello          Compile to executable
  ofs check hello.ofs                   Type-check only

)";
}

// Check if a string ends with a given suffix
static bool ends_with(const std::string& str, const std::string& suffix) {
    if (suffix.size() > str.size()) return false;
    return str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

// Strip shebang line (#!/...) from source if present
static std::string strip_shebang(const std::string& source) {
    if (source.size() >= 2 && source[0] == '#' && source[1] == '!') {
        auto nl = source.find('\n');
        if (nl != std::string::npos) return source.substr(nl + 1);
        return "";
    }
    return source;
}

// Generate a unique temporary file path using the process ID
static std::string temp_path(const std::string& suffix) {
    return "/tmp/ofs_" + std::to_string(getpid()) + suffix;
}

// Execute the full compile-and-run pipeline for a source file
static int run_file(const std::string& file, const std::string& source) {
    ofs::Lexer lexer(source, file);
    auto tokens = lexer.tokenize();

    ofs::Parser parser(std::move(tokens));
    auto mod = parser.parse();

    ofs::SemanticAnalyzer sem;
    sem.analyze(mod);

    ofs::CodeGen codegen;
    codegen.generate(mod);

    std::string obj = temp_path(".o");
    std::string exe = temp_path("");
    codegen.emit_object(obj);
    codegen.link(obj, exe);
    int result = std::system(exe.c_str());
    std::remove(obj.c_str());
    std::remove(exe.c_str());
    return result;
}

int main(int argc, char** argv) {
    if (argc < 2) { print_usage(); return 1; }

    std::string cmd  = argv[1];
    std::string out  = "a.out";

    // ── Quick commands (no file needed) ──────────────────────────────────
    if (cmd == "version" || cmd == "--version" || cmd == "-v") {
        std::cout << "ofs " << OFS_VERSION << " — Obsidian Fault Script\n";
        return 0;
    }

    if (cmd == "help" || cmd == "--help" || cmd == "-h") {
        print_usage();
        return 0;
    }

    // ── Auto-detect: ofs file.ofs → run directly (like python) ───────────
    if (ends_with(cmd, ".ofs")) {
        std::string file = cmd;
        std::ifstream f(file);
        if (!f) { std::cerr << "ofs: cannot open '" << file << "'\n"; return 1; }
        std::string source((std::istreambuf_iterator<char>(f)), {});
        source = strip_shebang(source);

        try {
            return run_file(file, source);
        } catch (const ofs::ParseError& e) {
            std::cerr << file << ":" << e.line << ":" << e.col
                      << ": parse error: " << e.what() << "\n";
            return 1;
        } catch (const ofs::SemanticError& e) {
            std::cerr << file << ":" << e.line << ":" << e.col
                      << ": type error: " << e.what() << "\n";
            return 1;
        } catch (const std::exception& e) {
            std::cerr << "ofs: error: " << e.what() << "\n";
            return 1;
        }
    }

    // ── Subcommand mode: ofs <cmd> <file.ofs> [flags] ────────────────────
    std::string file = argc > 2 ? argv[2] : "";

    // Parse -o flag
    for (int i = 2; i < argc - 1; i++) {
        if (std::string(argv[i]) == "-o") out = argv[i+1];
    }

    if (file.empty()) { print_usage(); return 1; }

    // Read source
    std::ifstream f(file);
    if (!f) { std::cerr << "ofs: cannot open '" << file << "'\n"; return 1; }
    std::string source((std::istreambuf_iterator<char>(f)), {});
    source = strip_shebang(source);

    try {
        // 1. Lex
        ofs::Lexer lexer(source, file);
        auto tokens = lexer.tokenize();

        if (cmd == "tokens") {
            for (auto& t : tokens)
                std::cout << t.to_string() << "\n";
            return 0;
        }

        // 2. Parse
        ofs::Parser parser(std::move(tokens));
        auto mod = parser.parse();

        if (cmd == "ast") {
            ofs::print_ast(mod);
            return 0;
        }

        // 3. Semantic
        ofs::SemanticAnalyzer sem;
        sem.analyze(mod);

        if (cmd == "check") {
            std::cout << "OK — no errors\n";
            return 0;
        }

        // 4. Codegen
        ofs::CodeGen codegen;
        codegen.generate(mod);

        if (cmd == "ir") {
            codegen.emit_ir(out + ".ll");
            std::cout << "LLVM IR written to " << out << ".ll\n";
            return 0;
        }

        if (cmd == "build") {
            std::string obj = out + ".o";
            codegen.emit_object(obj);
            codegen.link(obj, out);
            std::cout << "Built: " << out << "\n";
            return 0;
        }

        if (cmd == "run") {
            std::string obj = temp_path(".o");
            std::string exe = temp_path("");
            codegen.emit_object(obj);
            codegen.link(obj, exe);
            int result = std::system(exe.c_str());
            std::remove(obj.c_str());
            std::remove(exe.c_str());
            return result;
        }

        std::cerr << "ofs: unknown command '" << cmd << "'\n";
        print_usage();
        return 1;

    } catch (const ofs::ParseError& e) {
        std::cerr << file << ":" << e.line << ":" << e.col
                  << ": parse error: " << e.what() << "\n";
        return 1;
    } catch (const ofs::SemanticError& e) {
        std::cerr << file << ":" << e.line << ":" << e.col
                  << ": type error: " << e.what() << "\n";
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "ofs: error: " << e.what() << "\n";
        return 1;
    }
}
