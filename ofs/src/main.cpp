#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstdlib>
#include <cstdio>
#include <filesystem>
#include <regex>
#include <unordered_set>
#include <vector>
#include <chrono>
#include <thread>
#include <unistd.h>
#ifdef _WIN32
#include <process.h>
#include <windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#endif
#include "lexer/lexer.hpp"
#include "parser/parser.hpp"
#include "semantic/semantic.hpp"
#include "codegen/codegen.hpp"
#include "ast/ast_printer.hpp"
#include "i18n.hpp"

static const char* OFS_VERSION = "1.0.4";
static const char* OFS_REPO_OWNER = "Samwns";
static const char* OFS_REPO_NAME  = "Obsidian-Fault-Script";

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
    std::error_code ec;
    auto cwd = std::filesystem::current_path(ec);
    if (ec) {
        return "";
    }
    return cwd.string();
#endif
}

namespace cli_style {
static const char* RESET  = "\x1b[0m";
static const char* BOLD   = "\x1b[1m";
static const char* CYAN   = "\x1b[36m";
static const char* BLUE   = "\x1b[34m";
static const char* GREEN  = "\x1b[32m";
static const char* YELLOW = "\x1b[33m";
static const char* RED    = "\x1b[31m";

static std::string paint(const std::string& s, const char* color, bool bold = false) {
    std::string out;
    if (bold) out += BOLD;
    out += color;
    out += s;
    out += RESET;
    return out;
}
}

static std::string trim_copy(const std::string& s);

#ifdef _WIN32
#define OFS_POPEN _popen
#define OFS_PCLOSE _pclose
#else
#define OFS_POPEN popen
#define OFS_PCLOSE pclose
#endif

static std::string quote_arg(const std::string& s) {
    std::string out = "\"";
    for (char c : s) {
        if (c == '"') out += "\\\"";
        else out += c;
    }
    out += '"';
    return out;
}

static std::string quote_ps_single(const std::string& s) {
    std::string out = "'";
    for (char c : s) {
        if (c == '\'') out += "''";
        else out += c;
    }
    out += "'";
    return out;
}

static std::string run_capture(const std::string& cmd) {
    std::string out;
    FILE* pipe = OFS_POPEN(cmd.c_str(), "r");
    if (!pipe) return out;
    char buffer[512];
    while (fgets(buffer, sizeof(buffer), pipe)) {
        out += buffer;
    }
    OFS_PCLOSE(pipe);
    return out;
}

static std::vector<int> parse_version_nums(const std::string& version_like) {
    std::vector<int> nums;
    std::regex re("(\\d+)");
    auto begin = std::sregex_iterator(version_like.begin(), version_like.end(), re);
    auto end = std::sregex_iterator();
    for (auto it = begin; it != end; ++it) {
        nums.push_back(std::stoi((*it)[1].str()));
        if (nums.size() == 3) break;
    }
    while (nums.size() < 3) nums.push_back(0);
    return nums;
}

static int compare_semver(const std::string& a, const std::string& b) {
    auto av = parse_version_nums(a);
    auto bv = parse_version_nums(b);
    for (int i = 0; i < 3; ++i) {
        if (av[i] < bv[i]) return -1;
        if (av[i] > bv[i]) return 1;
    }
    return 0;
}

static bool download_file(const std::string& url, const std::filesystem::path& out_path) {
#ifdef _WIN32
    // Use PowerShell because it's available on all supported Windows hosts.
    std::string cmd = "powershell -NoProfile -ExecutionPolicy Bypass -Command "
                      "\"$ProgressPreference='SilentlyContinue'; "
                      "Invoke-WebRequest -Uri " + quote_ps_single(url) + " -OutFile " + quote_ps_single(out_path.string()) + "\"";
    return std::system(cmd.c_str()) == 0;
#else
    std::string out = quote_arg(out_path.string());
    std::string cmd = "curl -fsSL --retry 3 -o " + out + " " + quote_arg(url)
                    + " || wget -q -O " + out + " " + quote_arg(url);
    return std::system(cmd.c_str()) == 0;
#endif
}

static std::string fetch_latest_tag() {
    const std::string api = "https://api.github.com/repos/" + std::string(OFS_REPO_OWNER) + "/" + OFS_REPO_NAME + "/releases/latest";

#ifdef _WIN32
    std::string raw = run_capture(
        "powershell -NoProfile -ExecutionPolicy Bypass -Command "
        "\"$ProgressPreference='SilentlyContinue'; "
    "$r = Invoke-RestMethod -Uri " + quote_ps_single(api) + "; "
        "if ($r.tag_name) { $r.tag_name }\"");
#else
    std::string raw = run_capture("curl -fsSL " + quote_arg(api));
#endif

    raw = trim_copy(raw);
    if (raw.empty()) return "";

    // Windows path already returns only tag_name; Unix path returns full JSON.
    if (!raw.empty() && raw[0] == 'v') {
        return trim_copy(raw);
    }

    std::smatch m;
    if (std::regex_search(raw, m, std::regex("\"tag_name\"\\s*:\\s*\"([^\"]+)\""))) {
        return m[1].str();
    }
    return "";
}

static int run_self_update() {
    try {
        const char* frames[] = {"[=     ]", "[==    ]", "[===   ]", "[ ==== ]", "[  === ]", "[   == ]", "[    = ]"};
        auto animate = [&](const std::string& label, int loops = 7) {
            for (int i = 0; i < loops; ++i) {
                std::cout << "\r" << label << " " << frames[i % 7] << std::flush;
                std::this_thread::sleep_for(std::chrono::milliseconds(45));
            }
            std::cout << "\r" << label << " "
                      << cli_style::paint("[done ]", cli_style::GREEN, true) << "\n";
        };

        std::string current = std::string("v") + OFS_VERSION;
        animate(cli_style::paint(OFS_MSG("Checking releases", "Verificando releases"), cli_style::CYAN, true));
        std::string latest = fetch_latest_tag();

        if (latest.empty()) {
            std::cerr << cli_style::paint(
                OFS_MSG("ofs update: failed to fetch latest release tag\n",
                        "ofs update: falha ao buscar tag da release mais recente\n"),
                cli_style::RED, true);
            return 1;
        }

        if (compare_semver(current, latest) >= 0) {
            std::cout << cli_style::paint(OFS_MSG("OFS is already up to date (", "OFS ja esta atualizado ("), cli_style::GREEN, true)
                      << current << ")\n";
            return 0;
        }

        std::vector<std::string> candidates;
#ifdef _WIN32
    candidates.push_back("ofs-windows-x64-portable-" + latest + ".zip");
    candidates.push_back("ofs-windows-x64-portable.zip");
        candidates.push_back("ofs-windows-x64-installer-" + latest + ".exe");
        candidates.push_back("ofs-windows-x64-installer.exe");
#elif __APPLE__
        candidates.push_back("ofs-macos-arm64-installer-" + latest + ".pkg");
        candidates.push_back("ofs-macos-arm64-installer.pkg");
#else
        candidates.push_back("ofs-linux-x64-installer-" + latest + ".tar.gz");
        candidates.push_back("ofs-linux-x64-installer.tar.gz");
#endif

        std::error_code ec;
        auto tmp_root = std::filesystem::temp_directory_path(ec);
        if (ec) tmp_root = std::filesystem::current_path();
    #ifdef _WIN32
        const int pid = _getpid();
    #else
        const int pid = getpid();
    #endif
        auto workdir = tmp_root / ("ofs_update_" + std::to_string(pid));
        std::filesystem::create_directories(workdir);

        std::string downloaded_asset;
        std::filesystem::path downloaded_path;
        for (const auto& asset : candidates) {
            std::string url = "https://github.com/" + std::string(OFS_REPO_OWNER) + "/" + OFS_REPO_NAME + "/releases/download/" + latest + "/" + asset;
            auto out = workdir / asset;
            std::cout << cli_style::paint(OFS_MSG("Trying: ", "Tentando: "), cli_style::YELLOW, true)
                      << asset << "\n";
            animate(cli_style::paint(OFS_MSG("Downloading", "Baixando"), cli_style::BLUE, true));
            if (download_file(url, out)) {
                downloaded_asset = asset;
                downloaded_path = out;
                break;
            }
        }

        if (downloaded_asset.empty()) {
            std::filesystem::remove_all(workdir);
                std::cerr << cli_style::paint(
                OFS_MSG("ofs update: failed to download installer asset\n",
                    "ofs update: falha ao baixar artefato do instalador\n"),
                cli_style::RED, true);
            return 1;
        }

            std::cout << cli_style::paint(OFS_MSG("Downloaded: ", "Baixado: "), cli_style::GREEN, true)
                  << downloaded_asset << "\n";

#ifdef _WIN32
        auto has_suffix = [](const std::string& s, const std::string& suffix) {
            return s.size() >= suffix.size() &&
                   s.compare(s.size() - suffix.size(), suffix.size(), suffix) == 0;
        };

        int rc = 0;
        if (has_suffix(downloaded_asset, ".zip")) {
            animate(cli_style::paint(OFS_MSG("Applying portable update", "Aplicando atualizacao portatil"), cli_style::CYAN, true));

            auto unpack = workdir / "unpack";
            std::filesystem::create_directories(unpack);

            std::string expand_cmd = "powershell -NoProfile -ExecutionPolicy Bypass -Command "
                                     "\"Expand-Archive -LiteralPath " + quote_ps_single(downloaded_path.string())
                                     + " -DestinationPath " + quote_ps_single(unpack.string()) + " -Force\"";
            rc = std::system(expand_cmd.c_str());
            if (rc != 0) {
                std::filesystem::remove_all(workdir);
                std::cerr << cli_style::paint(
                    OFS_MSG("ofs update: failed to extract portable update package\n",
                            "ofs update: falha ao extrair pacote portatil de atualizacao\n"),
                    cli_style::RED, true);
                return rc;
            }

            const std::string install_dir = get_executable_dir();
            if (install_dir.empty()) {
                std::filesystem::remove_all(workdir);
                std::cerr << cli_style::paint(
                    OFS_MSG("ofs update: could not determine installation directory\n",
                            "ofs update: nao foi possivel determinar o diretorio de instalacao\n"),
                    cli_style::RED, true);
                return 1;
            }

            auto script_path = workdir / "apply_update.ps1";
            std::ofstream script(script_path);
            if (!script) {
                std::filesystem::remove_all(workdir);
                std::cerr << cli_style::paint(
                    OFS_MSG("ofs update: failed to prepare update script\n",
                            "ofs update: falha ao preparar script de atualizacao\n"),
                    cli_style::RED, true);
                return 1;
            }

            const int pid = _getpid();
            script
                << "$ErrorActionPreference = 'Stop'\n"
                << "$src = " << quote_ps_single(unpack.string()) << "\n"
                << "$dst = " << quote_ps_single(install_dir) << "\n"
                << "$pid = " << pid << "\n"
                << "for ($i = 0; $i -lt 120; $i++) { if (-not (Get-Process -Id $pid -ErrorAction SilentlyContinue)) { break }; Start-Sleep -Milliseconds 250 }\n"
                << "$filesToReplace = @('ofs.exe','libstdc++-6.dll','libgcc_s_seh-1.dll','libwinpthread-1.dll','libofs_runtime.a')\n"
                << "foreach ($f in $filesToReplace) { $p = Join-Path $dst $f; if (Test-Path $p) { Remove-Item $p -Force -ErrorAction SilentlyContinue } }\n"
                << "Get-ChildItem -Path $src -File | ForEach-Object { Copy-Item $_.FullName -Destination (Join-Path $dst $_.Name) -Force }\n"
                << "exit 0\n";
            script.close();

            std::string apply_cmd = "powershell -NoProfile -ExecutionPolicy Bypass -File " + quote_arg(script_path.string());
            rc = std::system(apply_cmd.c_str());

            if (rc != 0) {
                std::cerr << cli_style::paint(
                    OFS_MSG("ofs update: failed to apply portable update\n",
                            "ofs update: falha ao aplicar atualizacao portatil\n"),
                    cli_style::RED, true);
                return rc;
            }

            std::cout << cli_style::paint(
                OFS_MSG("Portable update applied. Restart OFS terminal/session to use the new version.\n",
                        "Atualizacao portatil aplicada. Reinicie o terminal/sessao do OFS para usar a nova versao.\n"),
                cli_style::GREEN, true);

            // Intentionally keep workdir in case script logging is needed by users/admins.
            std::cout << cli_style::paint(OFS_MSG("Update completed to ", "Atualizacao concluida para "), cli_style::GREEN, true)
                      << latest << "\n";
            return 0;
        }

        animate(cli_style::paint(OFS_MSG("Installing", "Instalando"), cli_style::CYAN, true));
        std::string install_cmd = "powershell -NoProfile -ExecutionPolicy Bypass -Command "
                                  "\"Start-Process -FilePath " + quote_ps_single(downloaded_path.string()) + " -Verb RunAs -Wait\"";
        rc = std::system(install_cmd.c_str());
        if (rc != 0) {
            // Fallback: try non-elevated install for environments where UAC prompt is blocked.
            std::string fallback_cmd = "powershell -NoProfile -ExecutionPolicy Bypass -Command "
                                       "\"Start-Process -FilePath " + quote_ps_single(downloaded_path.string()) + " -Wait\"";
            rc = std::system(fallback_cmd.c_str());
        }
#elif __APPLE__
    animate(cli_style::paint(OFS_MSG("Installing", "Instalando"), cli_style::CYAN, true));
        std::string install_cmd = "sudo installer -pkg " + quote_arg(downloaded_path.string()) + " -target /";
        int rc = std::system(install_cmd.c_str());
#else
        auto unpack = workdir / "unpack";
        std::filesystem::create_directories(unpack);
        auto install_script = unpack / "install.sh";
    animate(cli_style::paint(OFS_MSG("Installing", "Instalando"), cli_style::CYAN, true));
        std::string install_cmd = "tar -xzf " + quote_arg(downloaded_path.string()) + " -C " + quote_arg(unpack.string())
                                + " && chmod +x " + quote_arg(install_script.string())
                                + " && " + quote_arg(install_script.string());
        int rc = std::system(install_cmd.c_str());
#endif

        std::filesystem::remove_all(workdir);

        if (rc != 0) {
            std::cerr << cli_style::paint(
                OFS_MSG("ofs update: installer execution failed\n",
                        "ofs update: falha ao executar instalador\n"),
                cli_style::RED, true);
            return rc;
        }

        std::cout << cli_style::paint(OFS_MSG("Update completed to ", "Atualizacao concluida para "), cli_style::GREEN, true)
                  << latest << "\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << cli_style::paint(OFS_MSG("ofs update: error: ", "ofs update: erro: "), cli_style::RED, true)
                  << e.what() << "\n";
        return 1;
    }
}

static std::string trim_copy(const std::string& s) {
    const auto begin = s.find_first_not_of(" \t\r\n");
    if (begin == std::string::npos) return "";
    const auto end = s.find_last_not_of(" \t\r\n");
    return s.substr(begin, end - begin + 1);
}

static std::vector<std::filesystem::path> parse_lib_paths() {
    std::vector<std::filesystem::path> paths;
    const char* env = std::getenv("OFS_LIB_PATH");
    if (!env || !*env) return paths;

    std::string raw(env);
#ifdef _WIN32
    const char sep = ';';
#else
    const char sep = ':';
#endif

    std::stringstream ss(raw);
    std::string item;
    while (std::getline(ss, item, sep)) {
        item = trim_copy(item);
        if (!item.empty()) paths.emplace_back(item);
    }
    return paths;
}

static std::filesystem::path resolve_attach_path(const std::string& attach_path,
                                                 const std::filesystem::path& base_dir) {
    std::filesystem::path candidate(attach_path);
    if (candidate.is_absolute()) return candidate;

    auto local = (base_dir / candidate).lexically_normal();
    if (std::filesystem::exists(local)) return local;

    for (const auto& lib_root : parse_lib_paths()) {
        auto from_lib = (lib_root / candidate).lexically_normal();
        if (std::filesystem::exists(from_lib)) return from_lib;
    }

    return local;
}

static std::string resolve_attaches_recursive(const std::string& source,
                                              const std::filesystem::path& base_dir,
                                              std::unordered_set<std::string>& visited) {
    static const std::regex kAttachRegex("^\\s*(attach|import)\\s+\"([^\"]+)\"\\s*$");

    std::stringstream in(source);
    std::string line;
    std::string imports_merged;
    std::string body;

    while (std::getline(in, line)) {
        std::smatch m;
        if (std::regex_match(line, m, kAttachRegex)) {
            auto target = resolve_attach_path(m[2].str(), base_dir);
            std::error_code ec;
            auto key = std::filesystem::weakly_canonical(target, ec).string();
            if (ec) key = target.lexically_normal().string();

            if (visited.find(key) != visited.end()) {
                continue;
            }
            visited.insert(key);

            std::ifstream f(target);
            if (!f) {
                throw std::runtime_error(OFS_MSG("cannot open attached module: ", "não foi possível abrir módulo attach: ") + target.string());
            }

            std::string mod_source((std::istreambuf_iterator<char>(f)), {});
            imports_merged += resolve_attaches_recursive(mod_source, target.parent_path(), visited);
            imports_merged += "\n";
            continue;
        }

        body += line;
        body += "\n";
    }

    return imports_merged + body;
}

static std::string preprocess_source(const std::string& file, const std::string& source) {
    std::filesystem::path p(file);
    std::unordered_set<std::string> visited;
    return resolve_attaches_recursive(source, p.parent_path().empty() ? std::filesystem::current_path() : p.parent_path(), visited);
}

static void print_ascii_banner() {
    std::cout
        << "\n"
        << cli_style::paint("   ____  _____ ____  ", cli_style::BLUE, true) << "\n"
        << cli_style::paint("  / __ \\/ ___// __/  ", cli_style::BLUE, true) << "  "
        << cli_style::paint("OFS", cli_style::CYAN, true) << "  Obsidian Fault Script\n"
        << cli_style::paint(" / /_/ / /__ /\\ \\    ", cli_style::BLUE, true) << "  language + compiler\n"
        << cli_style::paint(" \\____/\\___//___/    ", cli_style::BLUE, true) << "\n\n";
}

void print_usage() {
    print_ascii_banner();
    if (ofs_locale_is_pt()) {
        std::cout <<
"\n" << cli_style::paint("ofs", cli_style::CYAN, true)
<< " - Obsidian Fault Script compilador v" << OFS_VERSION << "\n"
"\nUso:\n"
"  ofs <arquivo.ofs>                     Executa o script diretamente\n"
"  ofs run    <arquivo.ofs>              Compila e executa imediatamente\n"
"  ofs build  <arquivo.ofs> [-o saida]  Compila para executável nativo\n"
"  ofs check  <arquivo.ofs>             Verifica tipos sem compilar\n"
"  ofs tokens <arquivo.ofs>             Exibe o fluxo de tokens (debug)\n"
"  ofs ast    <arquivo.ofs>             Exibe a AST (debug)\n"
"  ofs ir     <arquivo.ofs>             Emite o LLVM IR (debug)\n"
"  ofs version                          Exibe a versão do compilador\n"
"  ofs update                           Atualiza para a release mais recente\n"
"  ofs help                             Mostra esta mensagem de ajuda\n"
"\nContexto dos comandos:\n"
"  run    -> fluxo rápido (desenvolvimento diário)\n"
"  build  -> gera binário final para distribuir/publicar\n"
"  check  -> valida tipos e erros sem gerar artefatos\n"
"  tokens -> debug léxico (inspecionar tokenização)\n"
"  ast    -> debug sintático (inspecionar árvore)\n"
"  ir     -> debug/otimização LLVM (inspecionar IR)\n"
"  update -> autoatualização via releases do GitHub\n"
"\nExemplos:\n"
"  ofs hello.ofs                        Executa hello.ofs diretamente\n"
"  ofs build hello.ofs -o hello         Compila para executável\n"
"  ofs check hello.ofs                  Verifica tipos apenas\n"
"  ofs update                           Atualiza OFS via GitHub Releases\n\n";
    } else {
        std::cout <<
"\n" << cli_style::paint("ofs", cli_style::CYAN, true)
<< " - Obsidian Fault Script compiler v" << OFS_VERSION << "\n"
"\nUsage:\n"
"  ofs <file.ofs>                        Run a script directly\n"
"  ofs run    <file.ofs>                 Compile and run immediately\n"
"  ofs build  <file.ofs> [-o output]     Compile to native executable\n"
"  ofs check  <file.ofs>                 Type-check only (no output)\n"
"  ofs tokens <file.ofs>                 Print token stream (debug)\n"
"  ofs ast    <file.ofs>                 Print AST (debug)\n"
"  ofs ir     <file.ofs>                 Emit LLVM IR (debug)\n"
"  ofs version                           Print compiler version\n"
"  ofs update                            Update to the latest release\n"
"  ofs help                              Show this help message\n"
"\nCommand context:\n"
"  run    -> fastest dev loop (compile + execute now)\n"
"  build  -> generate final native binary for shipping\n"
"  check  -> type/safety validation without output artifacts\n"
"  tokens -> lexical debug (inspect token stream)\n"
"  ast    -> syntax debug (inspect parsed tree)\n"
"  ir     -> LLVM debug/optimization inspection\n"
"  update -> self-update from GitHub releases\n"
"\nExamples:\n"
"  ofs hello.ofs                         Run hello.ofs directly\n"
"  ofs build hello.ofs -o hello          Compile to executable\n"
"  ofs check hello.ofs                   Type-check only\n"
"  ofs update                            Update OFS from GitHub Releases\n\n";
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

static std::string temp_executable_path() {
#ifdef _WIN32
    return temp_path(".exe");
#else
    return temp_path("");
#endif
}

// Execute the full compile-and-run pipeline for a source file
static int run_file(const std::string& file, const std::string& source) {
    std::string merged_source = preprocess_source(file, source);
    ofs::Lexer lexer(merged_source, file);
    auto tokens = lexer.tokenize();

    ofs::Parser parser(std::move(tokens));
    auto mod = parser.parse();

    ofs::SemanticAnalyzer sem;
    sem.analyze(mod);

    ofs::CodeGen codegen;
    codegen.generate(mod);

    std::string obj = temp_path(".o");
    std::string exe = temp_executable_path();
    codegen.emit_object(obj);
    codegen.link(obj, exe);
    int result = std::system(("\"" + exe + "\"").c_str());
    std::remove(obj.c_str());
    std::remove(exe.c_str());
    return result;
}

int main(int argc, char** argv) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    if (argc < 2) { print_usage(); return 1; }

    std::string cmd  = argv[1];
    std::string out  = "a.out";

    // ── Quick commands (no file needed) ──────────────────────────────────
    if (cmd == "version" || cmd == "--version" || cmd == "-v") {
        print_ascii_banner();
        std::cout << cli_style::paint("ofs ", cli_style::CYAN, true)
                  << OFS_VERSION << " - Obsidian Fault Script\n";
        std::cout << cli_style::paint(OFS_MSG("status: stable\n", "status: estavel\n"), cli_style::GREEN, true);
        return 0;
    }

    if (cmd == "help" || cmd == "--help" || cmd == "-h") {
        print_usage();
        return 0;
    }

    if (cmd == "update") {
        return run_self_update();
    }

    // ── Auto-detect: ofs file.ofs → run directly ─────────────────────────
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
        source = preprocess_source(file, source);

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
            std::string exe = temp_executable_path();
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
