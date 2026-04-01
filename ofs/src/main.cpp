#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <unistd.h>
#ifdef _WIN32
#include <process.h>
#endif
#include "lexer/lexer.hpp"
#include "parser/parser.hpp"
#include "semantic/semantic.hpp"
#include "codegen/codegen.hpp"
#include "ast/ast_printer.hpp"
#include "i18n.hpp"

static const char* OFS_VERSION = "1.0.0";

void print_usage() {
    if (ofs_locale_is_pt()) {
        std::cout <<
"\nofs — Obsidian Fault Script compilador v" << OFS_VERSION << "\n"
"\nUso:\n"
"  ofs <arquivo.ofs>                     Executa o script diretamente (como python)\n"
"  ofs run    <arquivo.ofs>              Compila e executa imediatamente\n"
"  ofs build  <arquivo.ofs> [-o saida]  Compila para executável nativo\n"
"  ofs check  <arquivo.ofs>             Verifica tipos sem compilar\n"
"  ofs tokens <arquivo.ofs>             Exibe o fluxo de tokens (debug)\n"
"  ofs ast    <arquivo.ofs>             Exibe a AST (debug)\n"
"  ofs ir     <arquivo.ofs>             Emite o LLVM IR (debug)\n"
"  ofs version                          Exibe a versão do compilador\n"
"  ofs help                             Mostra esta mensagem de ajuda\n"
"\nExemplos:\n"
"  ofs hello.ofs                        Executa hello.ofs diretamente\n"
"  ofs build hello.ofs -o hello         Compila para executável\n"
"  ofs check hello.ofs                  Verifica tipos apenas\n\n";
    } else {
        std::cout <<
"\nofs — Obsidian Fault Script compiler v" << OFS_VERSION << "\n"
"\nUsage:\n"
"  ofs <file.ofs>                        Run a script directly (like python)\n"
"  ofs run    <file.ofs>                 Compile and run immediately\n"
"  ofs build  <file.ofs> [-o output]     Compile to native executable\n"
"  ofs check  <file.ofs>                 Type-check only (no output)\n"
"  ofs tokens <file.ofs>                 Print token stream (debug)\n"
"  ofs ast    <file.ofs>                 Print AST (debug)\n"
"  ofs ir     <file.ofs>                 Emit LLVM IR (debug)\n"
"  ofs version                           Print compiler version\n"
"  ofs help                              Show this help message\n"
"\nExamples:\n"
"  ofs hello.ofs                         Run hello.ofs directly\n"
"  ofs build hello.ofs -o hello          Compile to executable\n"
"  ofs check hello.ofs                   Type-check only\n\n";
    }
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
#ifdef _WIN32
    const int pid = _getpid();
#else
    const int pid = getpid();
#endif

    std::error_code ec;
    auto tmp_dir = std::filesystem::temp_directory_path(ec);
    if (ec) {
        tmp_dir = std::filesystem::current_path();
    }

    auto name = std::string("ofs_") + std::to_string(pid) + suffix;
    return (tmp_dir / name).string();
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
    int result = std::system(("\"" + exe + "\"").c_str());
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
        if (!f) { std::cerr << OFS_MSG("ofs: cannot open '", "ofs: não foi possível abrir '") << file << "'\n"; return 1; }
        std::string source((std::istreambuf_iterator<char>(f)), {});
        source = strip_shebang(source);

        try {
            return run_file(file, source);
        } catch (const ofs::ParseError& e) {
            std::cerr << file << ":" << e.line << ":" << e.col
                      << OFS_MSG(": parse error: ", ": erro de sintaxe: ") << e.what() << "\n";
            return 1;
        } catch (const ofs::SemanticError& e) {
            std::cerr << file << ":" << e.line << ":" << e.col
                      << OFS_MSG(": type error: ", ": erro de tipo: ") << e.what() << "\n";
            return 1;
        } catch (const std::exception& e) {
            std::cerr << OFS_MSG("ofs: error: ", "ofs: erro: ") << e.what() << "\n";
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
    if (!f) { std::cerr << OFS_MSG("ofs: cannot open '", "ofs: não foi possível abrir '") << file << "'\n"; return 1; }
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
            std::cout << OFS_MSG("OK — no errors\n", "OK — sem erros\n");
            return 0;
        }

        // 4. Codegen
        ofs::CodeGen codegen;
        codegen.generate(mod);

        if (cmd == "ir") {
            codegen.emit_ir(out + ".ll");
            std::cout << OFS_MSG("LLVM IR written to ", "LLVM IR gravado em ") << out << ".ll\n";
            return 0;
        }

        if (cmd == "build") {
            std::string obj = out + ".o";
            codegen.emit_object(obj);
            codegen.link(obj, out);
            std::cout << OFS_MSG("Built: ", "Compilado: ") << out << "\n";
            return 0;
        }

        if (cmd == "run") {
            std::string obj = temp_path(".o");
            std::string exe = temp_path("");
            codegen.emit_object(obj);
            codegen.link(obj, exe);
            int result = std::system(("\"" + exe + "\"").c_str());
            std::remove(obj.c_str());
            std::remove(exe.c_str());
            return result;
        }

        std::cerr << OFS_MSG("ofs: unknown command '", "ofs: comando desconhecido '") << cmd << "'\n";
        print_usage();
        return 1;

    } catch (const ofs::ParseError& e) {
        std::cerr << file << ":" << e.line << ":" << e.col
                  << OFS_MSG(": parse error: ", ": erro de sintaxe: ") << e.what() << "\n";
        return 1;
    } catch (const ofs::SemanticError& e) {
        std::cerr << file << ":" << e.line << ":" << e.col
                  << OFS_MSG(": type error: ", ": erro de tipo: ") << e.what() << "\n";
        return 1;
    } catch (const std::exception& e) {
        std::cerr << OFS_MSG("ofs: error: ", "ofs: erro: ") << e.what() << "\n";
        return 1;
    }
}
