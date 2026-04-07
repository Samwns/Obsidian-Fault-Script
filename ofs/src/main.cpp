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

static const char* OFS_VERSION = "1.0.38";
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
    auto exe = std::filesystem::read_symlink("/proc/self/exe", ec);
    if (!ec) {
        return exe.parent_path().string();
    }
    auto cwd = std::filesystem::current_path(ec);
    if (ec) return "";
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

static std::string to_lower_copy(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}

static void safe_remove_all(const std::filesystem::path& target) {
    std::error_code ec;
    std::filesystem::remove_all(target, ec);
}

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
    // Use the HTML redirect endpoint instead of the rate-limited JSON API.
    // https://github.com/OWNER/REPO/releases/latest  redirects to  .../tag/<tag>
    const std::string release_url = "https://github.com/" + std::string(OFS_REPO_OWNER) + "/" + OFS_REPO_NAME + "/releases/latest";

#ifdef _WIN32
    std::string raw = run_capture(
        "powershell -NoProfile -ExecutionPolicy Bypass -Command "
        "\"$ProgressPreference='SilentlyContinue'; "
        "try { "
        "  $r = Invoke-WebRequest -Uri " + quote_ps_single(release_url) + " -MaximumRedirection 0 -ErrorAction Stop; "
        "  '' "
        "} catch { "
        "  $loc = $_.Exception.Response.Headers['Location']; "
        "  if ($loc) { $loc.Split('/')[-1] } else { '' } "
        "}\"");
#else
    // -I = HEAD only, -s = silent, follow up to 0 redirects but grab Location
    std::string raw = run_capture(
        "curl -sI -o /dev/null -w '%{redirect_url}' " + quote_arg(release_url));
    // redirect_url gives the full target URL; extract last segment (the tag)
    if (raw.empty()) {
        raw = run_capture("curl -sIL " + quote_arg(release_url) +
                          " | grep -i '^location:' | tail -1 | sed 's|.*/tag/||' | tr -d '\\r\\n'");
    }
#endif

    raw = trim_copy(raw);
    if (raw.empty()) return "";

    // Extract just the tag name (last path segment) from a full URL like
    // https://github.com/.../releases/tag/v1.2.3
    auto slash = raw.rfind('/');
    if (slash != std::string::npos && slash + 1 < raw.size()) {
        raw = raw.substr(slash + 1);
    }
    raw = trim_copy(raw);
    if (!raw.empty() && raw[0] == 'v') return raw;
    return "";
}

static int run_self_update(bool force_repair = false) {
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
    #ifdef _WIN32
            if (!force_repair) {
            std::cout << cli_style::paint(OFS_MSG("OFS is already up to date (", "OFS ja esta atualizado ("), cli_style::GREEN, true)
                  << current << ")\n";
            std::cout << cli_style::paint(
                OFS_MSG("Running Windows repair update to refresh files in place...\n",
                    "Executando atualizacao de reparo no Windows para atualizar arquivos no local...\n"),
                cli_style::YELLOW, true);
            }
    #else
            std::cout << cli_style::paint(OFS_MSG("OFS is already up to date (", "OFS ja esta atualizado ("), cli_style::GREEN, true)
                  << current << ")\n";
            return 0;
    #endif
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
                << "Add-Type -Namespace OfsUpdate -Name NativeMethods -MemberDefinition @'\n"
                << "[System.Runtime.InteropServices.DllImport(\"kernel32.dll\", SetLastError=true, CharSet=System.Runtime.InteropServices.CharSet.Unicode)]\n"
                << "public static extern bool MoveFileEx(string lpExistingFileName, string lpNewFileName, int dwFlags);\n"
                << "'@\n"
                << "$src = " << quote_ps_single(unpack.string()) << "\n"
                << "$dst = " << quote_ps_single(install_dir) << "\n"
                << "$targetPid = " << pid << "\n"
                << "$logFile = " << quote_ps_single((workdir / "update.log").string()) << "\n"
                << "$delayedFiles = [System.Collections.Generic.List[string]]::new()\n"
                << "function Write-UpdateLog([string]$message) {\n"
                << "  \"$(Get-Date -Format 'yyyy-MM-dd HH:mm:ss') $message\" | Out-File -FilePath $logFile -Append\n"
                << "}\n"
                << "function Schedule-Replacement([string]$sourceFile, [string]$destFile) {\n"
                << "  $stagedFile = Join-Path (Split-Path -Parent $destFile) (([System.Guid]::NewGuid().ToString()) + '.' + (Split-Path -Leaf $destFile) + '.ofs-pending')\n"
                << "  Copy-Item $sourceFile -Destination $stagedFile -Force -ErrorAction Stop\n"
                << "  $moveFlags = 0x1 -bor 0x4\n"
                << "  if (-not [OfsUpdate.NativeMethods]::MoveFileEx($stagedFile, $destFile, $moveFlags)) {\n"
                << "    $win32Error = [Runtime.InteropServices.Marshal]::GetLastWin32Error()\n"
                << "    throw \"Failed to schedule replacement for $destFile (Win32=$win32Error)\"\n"
                << "  }\n"
                << "}\n"
                << "Write-UpdateLog 'Update script started'\n"
                << "try {\n"
                << "  Write-UpdateLog \"Waiting for process $targetPid to exit...\"\n"
                << "  for ($i = 0; $i -lt 240; $i++) { if (-not (Get-Process -Id $targetPid -ErrorAction SilentlyContinue)) { break }; Start-Sleep -Milliseconds 250 }\n"
                << "  Write-UpdateLog 'Process wait complete'\n"
                << "  if (-not (Test-Path $dst)) { New-Item -ItemType Directory -Path $dst -Force | Out-Null }\n"
                << "  $files = Get-ChildItem -Path $src -File\n"
                << "  Write-UpdateLog \"Found $($files.Count) files to copy from $src\"\n"
                << "  foreach ($item in $files) {\n"
                << "    $destFile = Join-Path $dst $item.Name\n"
                << "    $copied = $false\n"
                << "    for ($r = 0; $r -lt 40; $r++) {\n"
                << "      try {\n"
                << "        Copy-Item $item.FullName -Destination $destFile -Force -ErrorAction Stop\n"
                << "        $copied = $true\n"
                << "        Write-UpdateLog \"Copied $($item.Name)\"\n"
                << "        break\n"
                << "      }\n"
                << "      catch {\n"
                << "        if ($r -eq 39) {\n"
                << "          Write-UpdateLog \"Direct copy still locked for $($item.Name); scheduling replacement after reboot\"\n"
                << "        }\n"
                << "        Start-Sleep -Milliseconds 250\n"
                << "      }\n"
                << "    }\n"
                << "    if (-not $copied) {\n"
                << "      Schedule-Replacement -sourceFile $item.FullName -destFile $destFile\n"
                << "      $delayedFiles.Add($item.Name) | Out-Null\n"
                << "      Write-UpdateLog \"Scheduled delayed replacement for $($item.Name)\"\n"
                << "    }\n"
                << "  }\n"
                << "  if ($delayedFiles.Count -gt 0) {\n"
                << "    Write-UpdateLog \"Delayed replacements pending until next Windows restart: $($delayedFiles -join ', ')\"\n"
                << "  }\n"
                << "  Write-UpdateLog 'Update completed successfully'\n"
                << "  exit 0\n"
                << "}\n"
                << "catch {\n"
                << "  Write-UpdateLog \"ERROR: $($_.Exception.Message)\"\n"
                << "  exit 1\n"
                << "}\n";
            script.close();

            std::string apply_cmd = "powershell -NoProfile -ExecutionPolicy Bypass -Command "
                                    "\"Start-Process -FilePath 'powershell' -Verb RunAs -WindowStyle Hidden "
                                    "-ArgumentList '-NoProfile -ExecutionPolicy Bypass -File ''"
                                    + script_path.string() + "'''\"";
            rc = std::system(apply_cmd.c_str());

            if (rc != 0) {
                // Fallback: no elevation (for user-local installs where admin is unnecessary).
                std::string fallback_apply = "powershell -NoProfile -ExecutionPolicy Bypass -Command "
                                             "\"Start-Process -FilePath 'powershell' -WindowStyle Hidden "
                                             "-ArgumentList '-NoProfile -ExecutionPolicy Bypass -File ''"
                                             + script_path.string() + "'''\"";
                rc = std::system(fallback_apply.c_str());
            }

            if (rc != 0) {
                std::cerr << cli_style::paint(
                    OFS_MSG("ofs update: failed to apply portable update\n",
                            "ofs update: falha ao aplicar atualizacao portatil\n"),
                    cli_style::RED, true);
                    std::string log_path = (workdir / "update.log").string();
                    std::cout << cli_style::paint(
                        OFS_MSG("See detailed log at: ",
                                "Veja log detalhado em: "),
                        cli_style::YELLOW, true) << log_path << "\n";
                return rc;
            }

            std::cout << cli_style::paint(
                OFS_MSG("Portable update scheduled. Close this OFS process and wait a few seconds for file replacement. Locked files will be finalized after the next Windows restart.\n",
                    "Atualizacao portatil agendada. Feche este processo OFS e aguarde alguns segundos para a substituicao dos arquivos. Arquivos bloqueados serao finalizados apos o proximo reinicio do Windows.\n"),
                cli_style::GREEN, true);

            // Intentionally keep workdir in case script logging is needed by users/admins.
            std::cout << cli_style::paint(OFS_MSG("Update process started for ", "Processo de atualizacao iniciado para "), cli_style::GREEN, true)
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

    // Always include the default OFS packages directory so installed packages
    // work with attach {} without requiring OFS_LIB_PATH to be set.
#ifdef _WIN32
    if (const char* appdata = std::getenv("APPDATA")) {
        if (*appdata) paths.emplace_back(std::string(appdata) + "\\ofs\\packages");
    }
#else
    if (const char* home = std::getenv("HOME")) {
        if (*home) paths.emplace_back(std::string(home) + "/.ofs/packages");
    }
#endif

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

// Maps clean library names to their stdlib filenames.
static const std::unordered_map<std::string, std::string>& stdlib_names() {
    static const std::unordered_map<std::string, std::string> NAMES = {
        {"core",            "core.ofs"},
        {"math",            "math.ofs"},
        {"string",          "string.ofs"},
        {"io",              "io.ofs"},
        {"webserver",       "webserver.ofs"},
        {"serve",           "webserver.ofs"},
        {"bedrock",         "bedrock.ofs"},
        {"bedrock-packet",  "bedrock_packet.ofs"},
        {"terminal-colors", "terminal_colors.ofs"},
        {"memory-modes",    "memory_modes.ofs"},
        {"test-lib",        "test_lib.ofs"},
    };
    return NAMES;
}

// Returns candidate stdlib directories in priority order.
static std::vector<std::filesystem::path> get_stdlib_search_dirs() {
    std::vector<std::filesystem::path> dirs;

    if (const char* env = std::getenv("OFS_STDLIB_PATH")) {
        dirs.emplace_back(env);
    }

    const std::string exe_dir = get_executable_dir();
    if (!exe_dir.empty()) {
        auto d = std::filesystem::path(exe_dir);
        dirs.push_back(d / "stdlib");                                    // ofs binary next to stdlib/
        dirs.push_back(d.parent_path() / "stdlib");                      // build/ -> src root stdlib/
        dirs.push_back(d.parent_path().parent_path() / "stdlib");        // bin/arch/ -> root stdlib/
        dirs.push_back(d.parent_path() / "share" / "ofs" / "stdlib");    // /usr/local layout
    }

    dirs.emplace_back("/usr/local/share/ofs/stdlib");
    dirs.emplace_back("/usr/share/ofs/stdlib");
    return dirs;
}

// Resolves a library name to an actual .ofs file path, or returns empty if not found.
static std::filesystem::path resolve_stdlib_path(const std::string& name) {
    const auto& names = stdlib_names();
    auto it = names.find(name);
    const std::string filename = (it != names.end()) ? it->second : (name + ".ofs");

    for (const auto& dir : get_stdlib_search_dirs()) {
        auto candidate = (dir / filename).lexically_normal();
        if (std::filesystem::exists(candidate)) return candidate;
    }

    // Also search OFS_LIB_PATH package directories.
    for (const auto& lib_root : parse_lib_paths()) {
        auto pkg = (lib_root / name / "libs" / filename).lexically_normal();
        if (std::filesystem::exists(pkg)) return pkg;
        auto direct = (lib_root / filename).lexically_normal();
        if (std::filesystem::exists(direct)) return direct;
    }

    return {};
}

static std::string builtin_module_source(const std::string& module_name) {
    if (module_name != "serve" && module_name != "webserver") {
        return "";
    }

    return R"OFS(
extern vein ofs_str_len(s: obsidian) -> stone
extern vein ofs_str_contains(haystack: obsidian, needle: obsidian) -> bool
extern vein ofs_stone_to_obsidian(v: stone) -> obsidian

forge MIME_JSON = "application/json"
forge MIME_HTML = "text/html; charset=utf-8"
forge MIME_TEXT = "text/plain; charset=utf-8"
forge MIME_CSS  = "text/css"
forge MIME_JS   = "application/javascript"

vein status_text(code: stone) -> obsidian {
    if (code == 200) { return "OK" }
    else if (code == 201) { return "Created" }
    else if (code == 400) { return "Bad Request" }
    else if (code == 404) { return "Not Found" }
    else if (code == 500) { return "Internal Server Error" }
    return "Unknown"
}

vein http_response(status: stone, content_type: obsidian, body: obsidian) -> obsidian {
    forge response = "HTTP/1.1 " + ofs_stone_to_obsidian(status) + " " + status_text(status)
    response = response + "\r\nContent-Type: " + content_type
    response = response + "\r\nContent-Length: " + ofs_stone_to_obsidian(ofs_str_len(body))
    response = response + "\r\nConnection: close\r\n\r\n" + body
    return response
}

vein json_string(key: obsidian, value: obsidian) -> obsidian {
    return "\"" + key + "\":\"" + value + "\""
}

vein json_number(key: obsidian, value: stone) -> obsidian {
    return "\"" + key + "\":" + ofs_stone_to_obsidian(value)
}

vein get_mime_type(filename: obsidian) -> obsidian {
    if (ofs_str_contains(filename, ".json")) { return MIME_JSON }
    else if (ofs_str_contains(filename, ".html")) { return MIME_HTML }
    else if (ofs_str_contains(filename, ".css")) { return MIME_CSS }
    else if (ofs_str_contains(filename, ".js")) { return MIME_JS }
    return MIME_TEXT
}

vein log_request(method: obsidian, path: obsidian, status: stone) {
    echo("[" + method + "] " + path + " -> " + ofs_stone_to_obsidian(status))
}

vein html_page(title: obsidian, body: obsidian) -> obsidian {
    forge html = "<html><head><title>" + title + "</title></head>"
    html = html + "<body>" + body + "</body></html>"
    return html
}
)OFS";
}

static std::string resolve_attaches_recursive(const std::string& source,
                                              const std::filesystem::path& base_dir,
                                              std::unordered_set<std::string>& visited) {
    // attach {name}       — library or stdlib module
    static const std::regex kAttachLib(
        "^\\s*(?:attach|import)\\s*\\{\\s*([A-Za-z_][A-Za-z0-9_.\\-]*)\\s*\\}\\s*$");
    // attach {F:path}  or  attach {File:path}  — explicit file reference
    static const std::regex kAttachFile(
        "^\\s*attach\\s*\\{\\s*[Ff](?:ile)?:\\s*([^\\}]+?)\\s*\\}\\s*$");

    std::stringstream in(source);
    std::string line;
    std::string imports_merged;
    std::string body;

    while (std::getline(in, line)) {
        std::smatch m;

        if (std::regex_match(line, m, kAttachLib)) {
            const std::string name = m[1].str();

            // 1. Try to locate the module file on disk.
            auto stdlib_path = resolve_stdlib_path(name);
            if (!stdlib_path.empty()) {
                std::error_code ec;
                auto key = std::filesystem::weakly_canonical(stdlib_path, ec).string();
                if (ec) key = stdlib_path.lexically_normal().string();
                if (visited.find(key) == visited.end()) {
                    visited.insert(key);
                    std::ifstream f(stdlib_path);
                    std::string mod_src((std::istreambuf_iterator<char>(f)), {});
                    imports_merged += resolve_attaches_recursive(mod_src, stdlib_path.parent_path(), visited);
                    imports_merged += "\n";
                }
                continue;
            }

            // 2. Fallback: embedded source (webserver only, for offline/portable use).
            std::string builtin = builtin_module_source(name);
            if (!builtin.empty()) {
                std::string builtin_key = "builtin:" + name;
                if (visited.find(builtin_key) == visited.end()) {
                    visited.insert(builtin_key);
                    imports_merged += resolve_attaches_recursive(builtin, base_dir, visited);
                    imports_merged += "\n";
                }
                continue;
            }

            throw std::runtime_error(
                OFS_MSG("unknown library in attach {}: ", "biblioteca desconhecida em attach {}: ") + name);
        }

        if (std::regex_match(line, m, kAttachFile)) {
            const std::string file_ref = m[1].str();
            auto target = resolve_attach_path(file_ref, base_dir);
            std::error_code ec;
            auto key = std::filesystem::weakly_canonical(target, ec).string();
            if (ec) key = target.lexically_normal().string();

            if (visited.find(key) != visited.end()) continue;
            visited.insert(key);

            std::ifstream f(target);
            if (!f) {
                throw std::runtime_error(
                    OFS_MSG("cannot open attached file: ", "não foi possível abrir arquivo attach: ") + target.string());
            }

            std::string mod_src((std::istreambuf_iterator<char>(f)), {});
            imports_merged += resolve_attaches_recursive(mod_src, target.parent_path(), visited);
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

// ── Package manager ───────────────────────────────────────────────────────

static const char* OFS_REGISTRY_URL =
    "https://raw.githubusercontent.com/Samwns/Obsidian-Fault-Script/main/packages/registry.json";
static const char* OFS_PACKAGES_RAW =
    "https://raw.githubusercontent.com/Samwns/Obsidian-Fault-Script/main/packages/src/";

static std::string get_packages_dir() {
#ifdef _WIN32
    const char* appdata = std::getenv("APPDATA");
    if (appdata && *appdata) return std::string(appdata) + "\\ofs\\packages";
    return "";
#else
    const char* home = std::getenv("HOME");
    if (home && *home) return std::string(home) + "/.ofs/packages";
    return "";
#endif
}

static bool looks_like_package_spec(const std::string& raw) {
    std::string s = trim_copy(raw);
    if (s.empty()) return false;
    if (s.size() >= 2 && s.front() == '{' && s.back() == '}') {
        s = trim_copy(s.substr(1, s.size() - 2));
    }

    static const std::regex spec_re(
        R"(^[A-Za-z_][A-Za-z0-9_.\-]*(?::[A-Za-z0-9_.\-]+)?$)");
    return std::regex_match(s, spec_re);
}

static bool is_powershell_meta_arg(const std::string& raw) {
    const std::string s = to_lower_copy(trim_copy(raw));
    return s == "-encodedcommand" || s == "-command" || s == "-file" ||
           s == "-executionpolicy" || s == "bypass" || s == "-noprofile" ||
           s == "-noninteractive" || s == "-nop" || s == "-ep";
}

static std::string extract_package_cli_arg(int argc, char** argv, int start_index) {
    std::string fallback = (argc > start_index) ? trim_copy(argv[start_index]) : "";
    if (looks_like_package_spec(fallback)) return fallback;

    bool skip_next = false;
    for (int i = start_index; i < argc; ++i) {
        std::string arg = trim_copy(argv[i]);
        if (arg.empty()) continue;
        if (skip_next) {
            skip_next = false;
            continue;
        }
        if (is_powershell_meta_arg(arg)) {
            skip_next = true;
            continue;
        }
        if (arg[0] == '-' && !looks_like_package_spec(arg)) continue;
        if (looks_like_package_spec(arg)) return arg;
    }
    return fallback;
}

// Parse "fmt", "{fmt}", "{fmt:1.2}", "{fmt:stable}"
struct PackageSpec {
    std::string name;
    std::string version; // empty = latest
    std::string channel; // "stable", "beta", "nightly", or empty
};

static PackageSpec parse_package_spec(const std::string& raw) {
    PackageSpec spec;
    std::string s = trim_copy(raw);
    if (!s.empty() && s.front() == '{' && s.back() == '}')
        s = trim_copy(s.substr(1, s.size() - 2));
    auto colon = s.find(':');
    if (colon != std::string::npos) {
        spec.name = trim_copy(s.substr(0, colon));
        std::string ver = trim_copy(s.substr(colon + 1));
        if (ver == "stable" || ver == "beta" || ver == "nightly")
            spec.channel = ver;
        else
            spec.version = ver;
    } else {
        spec.name = s;
    }
    return spec;
}

// Minimal JSON string extraction for a key in the nearest surrounding context
static std::string json_get_str(const std::string& json, const std::string& key) {
    std::smatch m;
    std::regex re("\"" + key + "\"\\s*:\\s*\"([^\"]+)\"");
    if (std::regex_search(json, m, re)) return m[1].str();
    return "";
}

// HTTP GET via curl/wget, returns body string
static std::string http_get(const std::string& url) {
    std::string r = run_capture("curl -fsSL --max-time 20 " + quote_arg(url));
    if (r.empty())
        r = run_capture("wget -q -O- --timeout=20 " + quote_arg(url));
    return trim_copy(r);
}

// Download URL to file path
static bool download_raw(const std::string& url, const std::filesystem::path& dest) {
    std::error_code ec;
    std::filesystem::create_directories(dest.parent_path(), ec);
    std::string d = quote_arg(dest.string());
    std::string cmd = "curl -fsSL --max-time 30 -o " + d + " " + quote_arg(url)
                    + " || wget -q --timeout=30 -O " + d + " " + quote_arg(url);
    return std::system(cmd.c_str()) == 0;
}

static int run_uncover(const std::string& query) {
    std::cout << cli_style::paint(
        OFS_MSG("Fetching package registry...\n", "Buscando registro de pacotes...\n"),
        cli_style::CYAN, true);

    std::string body = http_get(OFS_REGISTRY_URL);
    if (body.empty()) {
        std::cerr << cli_style::paint(
            OFS_MSG("uncover: failed to fetch registry — check your internet connection\n",
                    "uncover: falha ao buscar registro — verifique sua conexao\n"),
            cli_style::RED, true);
        return 1;
    }

    // Scan all name/description/version pairs in the registry
    std::regex block_re(
        "\"name\"\\s*:\\s*\"([^\"]+)\"[^}]*?"
        "(?:\"description\"\\s*:\\s*\"([^\"]+)\")?[^}]*?"
        "(?:\"stable\"\\s*:\\s*\"([^\"]+)\")?",
        std::regex::multiline);

    std::string qlower = query;
    std::transform(qlower.begin(), qlower.end(), qlower.begin(), ::tolower);

    bool found = false;
    auto begin = std::sregex_iterator(body.begin(), body.end(), block_re);
    auto end   = std::sregex_iterator();

    std::cout << "\n";
    for (auto it = begin; it != end; ++it) {
        std::string pkg_name = (*it)[1].str();
        std::string desc     = (*it)[2].str();
        std::string ver      = (*it)[3].str();

        std::string nl = pkg_name;
        std::transform(nl.begin(), nl.end(), nl.begin(), ::tolower);

        if (!query.empty() && nl.find(qlower) == std::string::npos &&
            desc.find(query) == std::string::npos) continue;

        std::cout << "  " << cli_style::paint(pkg_name, cli_style::CYAN, true);
        if (!ver.empty())
            std::cout << "  " << cli_style::paint("v" + ver, cli_style::GREEN, false);
        std::cout << "\n";
        if (!desc.empty())
            std::cout << "    " << desc << "\n";
        std::cout << "\n";
        found = true;
    }

    if (!found) {
        std::cout << OFS_MSG("  No packages found matching '", "  Nenhum pacote encontrado para '")
                  << query << "'\n";
    }
    return 0;
}

static int run_infuse(const std::string& raw_spec, bool update_mode = false) {
    auto spec = parse_package_spec(raw_spec);
    if (spec.name.empty()) {
        std::cerr << cli_style::paint(
            OFS_MSG("infuse: package name required. Example: infuse fmt\n",
                    "infuse: nome do pacote obrigatorio. Exemplo: infuse fmt\n"),
            cli_style::RED, true);
        return 1;
    }

    const std::string pkg_dir_str = get_packages_dir();
    if (pkg_dir_str.empty()) {
        std::cerr << cli_style::paint(
            OFS_MSG("infuse: cannot determine packages directory (HOME/APPDATA not set)\n",
                    "infuse: nao foi possivel determinar o diretorio de pacotes\n"),
            cli_style::RED, true);
        return 1;
    }

    std::cout << cli_style::paint(
        OFS_MSG("Fetching registry...\n", "Buscando registro...\n"),
        cli_style::CYAN, true);

    std::string registry = http_get(OFS_REGISTRY_URL);
    if (registry.empty()) {
        std::cerr << cli_style::paint(
            OFS_MSG("infuse: failed to reach package registry — check your internet connection\n",
                    "infuse: falha ao acessar registro — verifique sua conexao\n"),
            cli_style::RED, true);
        return 1;
    }

    // Verify package exists in registry
    std::regex exists_re("\"name\"\\s*:\\s*\"" + spec.name + "\"");
    if (!std::regex_search(registry, exists_re)) {
        std::cerr << cli_style::paint(
            OFS_MSG("infuse: package not found: ", "infuse: pacote nao encontrado: "),
            cli_style::RED, true) << spec.name << "\n";
        std::cout << OFS_MSG("  Tip: uncover ", "  Dica: uncover ")
                  << spec.name << OFS_MSG(" to search\n", " para buscar\n");
        return 1;
    }

    // Build download base URL
    std::string base_url = std::string(OFS_PACKAGES_RAW) + spec.name + "/";

    // Download ofspkg.json
    auto tmp_dir = std::filesystem::temp_directory_path() /
                   ("ofs_infuse_" + spec.name + "_" + std::to_string(
#ifdef _WIN32
                        _getpid()
#else
                        getpid()
#endif
                    ) + "_" + std::to_string(
                        static_cast<long long>(std::chrono::system_clock::now().time_since_epoch().count())));
    std::filesystem::create_directories(tmp_dir);
    auto pkg_json_tmp = tmp_dir / "ofspkg.json";

    if (!download_raw(base_url + "ofspkg.json", pkg_json_tmp)) {
        std::cerr << cli_style::paint(
            OFS_MSG("infuse: failed to download package metadata\n",
                    "infuse: falha ao baixar metadados do pacote\n"),
            cli_style::RED, true);
        safe_remove_all(tmp_dir);
        return 1;
    }

    std::string pkg_json;
    {
        std::ifstream pf(pkg_json_tmp);
        pkg_json.assign((std::istreambuf_iterator<char>(pf)), {});
    }
    std::string pkg_name    = json_get_str(pkg_json, "name");
    std::string pkg_version = json_get_str(pkg_json, "version");
    std::string pkg_main    = json_get_str(pkg_json, "main");

    if (pkg_name.empty() || pkg_main.empty()) {
        std::cerr << cli_style::paint(
            OFS_MSG("infuse: invalid package metadata (missing name/main)\n",
                    "infuse: metadados invalidos (name/main ausentes)\n"),
            cli_style::RED, true);
        safe_remove_all(tmp_dir);
        return 1;
    }

    // Check already installed
    auto install_dir = std::filesystem::path(pkg_dir_str) / pkg_name;
    if (!update_mode && std::filesystem::exists(install_dir / "ofspkg.json")) {
        std::ifstream ef(install_dir / "ofspkg.json");
        std::string ej((std::istreambuf_iterator<char>(ef)), {});
        std::string cur_ver = json_get_str(ej, "version");
        if (cur_ver == pkg_version) {
            std::cout << cli_style::paint(
                OFS_MSG("Package '", "Pacote '"), cli_style::GREEN, true)
                << pkg_name << OFS_MSG("' already at v", "' ja esta na v") << cur_ver
                << OFS_MSG(". Use reinfuse to force update.\n",
                           ". Use reinfuse para forcar atualizacao.\n");
            safe_remove_all(tmp_dir);
            return 0;
        }
    }

    // Download main library file
    auto main_filename = std::filesystem::path(pkg_main).filename();
    auto main_tmp = tmp_dir / main_filename;

    std::cout << cli_style::paint(
        OFS_MSG("Downloading ", "Baixando "), cli_style::BLUE, true)
        << pkg_name << " v" << pkg_version << "...\n";

    if (!download_raw(base_url + pkg_main, main_tmp)) {
        std::cerr << cli_style::paint(
            OFS_MSG("infuse: failed to download package library\n",
                    "infuse: falha ao baixar biblioteca do pacote\n"),
            cli_style::RED, true);
        safe_remove_all(tmp_dir);
        return 1;
    }

    // Install: copy to packages dir
    auto libs_dest = install_dir / "libs";
    std::error_code ec;
    std::filesystem::create_directories(libs_dest, ec);
    if (ec) {
        std::cerr << cli_style::paint(
            OFS_MSG("infuse: cannot create install directory: ", "infuse: nao foi possivel criar o diretorio: "),
            cli_style::RED, true) << libs_dest.string() << "\n";
        safe_remove_all(tmp_dir);
        return 1;
    }

    std::filesystem::copy_file(main_tmp, libs_dest / main_filename,
                               std::filesystem::copy_options::overwrite_existing, ec);
    std::filesystem::copy_file(pkg_json_tmp, install_dir / "ofspkg.json",
                               std::filesystem::copy_options::overwrite_existing, ec);
    safe_remove_all(tmp_dir);

    if (ec) {
        std::cerr << cli_style::paint(
            OFS_MSG("infuse: install failed during file copy\n",
                    "infuse: falha ao copiar arquivos durante instalacao\n"),
            cli_style::RED, true);
        return 1;
    }

    std::cout << cli_style::paint(
        OFS_MSG("Installed: ", "Instalado: "), cli_style::GREEN, true)
        << pkg_name << " v" << pkg_version << "\n"
        << OFS_MSG("  Path: ", "  Local: ") << install_dir.string() << "\n\n";

    // Usage hint
    std::cout << OFS_MSG("Usage in code:\n", "Uso no codigo:\n")
              << "  attach {" << pkg_name << "}\n\n";

    // Only remind about OFS_LIB_PATH if the user has a custom packages dir set
    // (the default packages dir is now always searched automatically).
    const char* lib_path_env = std::getenv("OFS_LIB_PATH");
    if (lib_path_env && *lib_path_env) {
        bool in_path = std::string(lib_path_env).find(pkg_dir_str) != std::string::npos;
        if (!in_path) {
            std::cout << cli_style::paint(
                OFS_MSG("Tip: add your packages dir to OFS_LIB_PATH to search it too:\n",
                        "Dica: adicione seu diretorio ao OFS_LIB_PATH para busca personalizada:\n"),
                cli_style::YELLOW, true);
#ifdef _WIN32
            std::cout << "  [Environment]::SetEnvironmentVariable('OFS_LIB_PATH', '"
                      << pkg_dir_str << "', 'User')\n\n";
#else
            std::cout << "  export OFS_LIB_PATH=" << quote_arg(pkg_dir_str) << "\n\n";
#endif
        }
    }
    return 0;
}

static int run_reinfuse(const std::string& raw_spec) {
    auto spec = parse_package_spec(raw_spec);
    if (spec.name.empty()) {
        std::cerr << cli_style::paint(
            OFS_MSG("reinfuse: package name required. Example: reinfuse fmt\n",
                    "reinfuse: nome do pacote obrigatorio. Exemplo: reinfuse fmt\n"),
            cli_style::RED, true);
        return 1;
    }
    std::cout << cli_style::paint(
        OFS_MSG("Updating ", "Atualizando "), cli_style::CYAN, true)
        << spec.name << "...\n";
    return run_infuse(spec.name, /*update_mode=*/true);
}

static void print_ascii_banner() {
    std::cout
        << "\n"
    << cli_style::paint(" ______        ______      ______    ", cli_style::BLUE, true) << "\n"
    << cli_style::paint("/\\  __ \\      /\\  ___\\    /\\  ___\\   ", cli_style::BLUE, true) << "\n"
    << cli_style::paint("\\ \\ \\/\\ \\     \\ \\  __\\    \\ \\___  \\  ", cli_style::BLUE, true) << "\n"
    << cli_style::paint(" \\ \\_____\\     \\ \\_\\       \\/\\_____\\ ", cli_style::BLUE, true) << "\n"
    << cli_style::paint("  \\/_____/      \\/_/        \\/_____/ ", cli_style::BLUE, true) << "\n\n";
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
"  ofs asm    <arquivo.ofs>             Emite assembly nativo do alvo\n"
"  ofs version                          Exibe a versão do compilador\n"
"  ofs update                           Atualiza para a release mais recente\n"
"  ofs help                             Mostra esta mensagem de ajuda\n"
"\nPacotes:\n"
"  uncover <nome>                       Busca pacotes no repositorio\n"
"  infuse  <nome>                       Instala um pacote\n"
"  infuse  <nome>:<versao>              Instala versao especifica\n"
"  infuse  <nome>:stable|beta|nightly   Instala por canal\n"
"  reinfuse <nome>                      Atualiza um pacote instalado\n"
"\nExemplos:\n"
"  uncover fmt                          Busca pacotes de formatacao\n"
"  infuse fmt                           Instala o pacote fmt\n"
"  infuse {fmt:stable}                  Instala canal stable de fmt\n"
"  reinfuse fmt                         Atualiza fmt para a versao mais recente\n"
"\nContexto dos comandos:\n"
"  run    -> fluxo rápido (desenvolvimento diário)\n"
"  build  -> gera binário final para distribuir/publicar\n"
"  check  -> valida tipos e erros sem gerar artefatos\n"
"  tokens -> debug léxico (inspecionar tokenização)\n"
"  ast    -> debug sintático (inspecionar árvore)\n"
"  ir     -> debug/otimização LLVM (inspecionar IR)\n"
"  asm    -> inspeção do código de máquina textual do alvo\n"
"  update -> autoatualização via releases do GitHub\n"
"\nExemplos:\n"
"  ofs hello.ofs                        Executa hello.ofs diretamente\n"
"  ofs build hello.ofs -o hello         Compila para executável\n"
"  ofs asm hello.ofs -o hello           Gera hello.s\n"
"  ofs check hello.ofs                  Verifica tipos apenas\n"
"  ofs update                           Atualiza OFS via GitHub Releases\n"
"  uncover fmt                          Busca pacotes de formatacao\n"
"  infuse fmt                           Instala o pacote fmt\n"
"  reinfuse fmt                         Atualiza fmt\n\n";
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
"  ofs asm    <file.ofs>                 Emit target-native assembly\n"
"  ofs version                           Print compiler version\n"
"  ofs update                            Update to the latest release\n"
"  ofs help                              Show this help message\n"
"\nPackages:\n"
"  uncover <name>                        Search packages in the repository\n"
"  infuse  <name>                        Install a package\n"
"  infuse  <name>:<version>              Install specific version\n"
"  infuse  <name>:stable|beta|nightly    Install by channel\n"
"  reinfuse <name>                       Update an installed package\n"
"\nExamples:\n"
"  ofs hello.ofs                         Run hello.ofs directly\n"
"  ofs build hello.ofs -o hello          Compile to executable\n"
"  ofs asm hello.ofs -o hello            Generate hello.s\n"
"  ofs check hello.ofs                   Type-check only\n"
"  ofs update                            Update OFS from GitHub Releases\n"
"  uncover fmt                           Search formatting packages\n"
"  infuse fmt                            Install the fmt package\n"
"  infuse {fmt:stable}                   Install stable channel of fmt\n"
"  reinfuse fmt                          Update fmt to latest version\n\n";
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
    if (!std::filesystem::exists(exe)) {
        std::remove(obj.c_str());
        return 1;
    }
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
        bool force_repair = false;
        if (argc > 2) {
            std::string flag = argv[2];
            if (flag == "--force" || flag == "-f") {
                force_repair = true;
            }
        }
        return run_self_update(force_repair);
    }

    // ── Package commands (no .ofs file needed) ────────────────────────────
    if (cmd == "uncover") {
        std::string query = extract_package_cli_arg(argc, argv, 2);
        // Strip braces if user typed e.g. uncover {fmt}
        if (query.size() >= 2 && query.front() == '{' && query.back() == '}')
            query = trim_copy(query.substr(1, query.size() - 2));
        return run_uncover(query);
    }

    if (cmd == "infuse") {
        if (argc < 3) {
            std::cerr << cli_style::paint(
                OFS_MSG("infuse: package spec required. Example: infuse fmt\n",
                        "infuse: especificacao do pacote obrigatoria. Exemplo: infuse fmt\n"),
                cli_style::RED, true);
            return 1;
        }
        // Allow "infuse fmt 1.2" as alternative to "infuse {fmt:1.2}"
        std::string spec_str = extract_package_cli_arg(argc, argv, 2);
        if (argc > 3) spec_str = spec_str + ":" + argv[3];
        return run_infuse(spec_str);
    }

    if (cmd == "reinfuse") {
        if (argc < 3) {
            std::cerr << cli_style::paint(
                OFS_MSG("reinfuse: package spec required. Example: reinfuse fmt\n",
                        "reinfuse: especificacao do pacote obrigatoria. Exemplo: reinfuse fmt\n"),
                cli_style::RED, true);
            return 1;
        }
        return run_reinfuse(extract_package_cli_arg(argc, argv, 2));
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

        if (cmd == "asm") {
            codegen.emit_asm(out + ".s");
            std::cout << OFS_MSG("Assembly written to ", "Assembly gravado em ") << out << ".s\n";
            return 0;
        }

        if (cmd == "build") {
            std::string obj = out + ".o";
            codegen.emit_object(obj);
            codegen.link(obj, out);
            if (!std::filesystem::exists(out)) {
                return 1;
            }
            std::cout << OFS_MSG("Built: ", "Compilado: ") << out << "\n";
            return 0;
        }

        if (cmd == "run") {
            std::string obj = temp_path(".o");
            std::string exe = temp_executable_path();
            codegen.emit_object(obj);
            codegen.link(obj, exe);
            if (!std::filesystem::exists(exe)) {
                std::remove(obj.c_str());
                return 1;
            }
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
