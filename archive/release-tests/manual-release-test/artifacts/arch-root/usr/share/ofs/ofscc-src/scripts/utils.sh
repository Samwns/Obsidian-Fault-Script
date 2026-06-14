#!/bin/bash
# OFS Development Utilities - Quick build and test helpers

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$(dirname "$SCRIPT_DIR")")"

# ──────────────────────────────────────────────────────────────────────────
# Colors
# ──────────────────────────────────────────────────────────────────────────

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m'

# ──────────────────────────────────────────────────────────────────────────
# usage
# ──────────────────────────────────────────────────────────────────────────

usage() {
    cat << EOF
${CYAN}OFS Development Utilities${NC}

Usage: ./utils.sh <command> [options]

Commands:
  compile <file.ofs>        Compile OFS file to executable
  check <file.ofs>          Type-check OFS file without codegen
  tokens <file.ofs>         Show lexer tokens
  ast <file.ofs>            Show AST
  ir <file.ofs>             Show LLVM IR output
  asm <file.ofs>            Show native assembly

  test <pattern>            Run tests matching pattern
  test-lexer                Run lexer tests
  test-parser               Run parser tests
  test-semantic             Run semantic tests

  benchmark                 Run performance benchmarks
  clean                     Clean build artifacts
  rebuild                   Clean and rebuild everything
  version                   Show compiler version

Examples:
  ./utils.sh compile hello.ofs -o hello
  ./utils.sh check example.ofs
  ./utils.sh tokens lexer_test.ofs
  ./utils.sh ast my_program.ofs
  ./utils.sh test-lexer
  ./utils.sh benchmark

EOF
    exit 1
}

# ──────────────────────────────────────────────────────────────────────────
# Helpers
# ──────────────────────────────────────────────────────────────────────────

print_header() {
    echo -e "${CYAN}→ $1${NC}"
}

print_success() {
    echo -e "${GREEN}✅ $1${NC}"
}

print_error() {
    echo -e "${RED}❌ $1${NC}"
    exit 1
}

print_info() {
    echo -e "${BLUE}ℹ️  $1${NC}"
}

# Find compiler
find_compiler() {
    if [ -f "dist/ofscc" ]; then
        echo "dist/ofscc"
    elif [ -f "ofscc_v3" ]; then
        echo "./ofscc_v3"
    elif [ -f "ofscc_v2" ]; then
        echo "./ofscc_v2"
    elif command -v ofscc &> /dev/null; then
        echo "ofscc"
    else
        return 1
    fi
}

# ──────────────────────────────────────────────────────────────────────────
# Commands
# ──────────────────────────────────────────────────────────────────────────

cmd_compile() {
    [ $# -lt 1 ] && { print_error "Usage: compile <file.ofs> [-o <output>] [-O0/-O2/-O3]"; }
    
    COMPILER=$(find_compiler) || print_error "No OFS compiler found. Run bootstrap first."
    
    print_header "Compiling: $1"
    "$COMPILER" build "$1" "${@:2}"
    print_success "Compilation complete"
}

cmd_check() {
    [ $# -lt 1 ] && { print_error "Usage: check <file.ofs>"; }
    
    COMPILER=$(find_compiler) || print_error "No OFS compiler found"
    
    print_header "Type-checking: $1"
    "$COMPILER" check "$1"
    print_success "Type check passed"
}

cmd_tokens() {
    [ $# -lt 1 ] && { print_error "Usage: tokens <file.ofs>"; }
    
    COMPILER=$(find_compiler) || print_error "No OFS compiler found"
    
    print_header "Lexer tokens for: $1"
    "$COMPILER" tokens "$1"
}

cmd_ast() {
    [ $# -lt 1 ] && { print_error "Usage: ast <file.ofs>"; }
    
    COMPILER=$(find_compiler) || print_error "No OFS compiler found"
    
    print_header "AST for: $1"
    "$COMPILER" ast "$1"
}

cmd_ir() {
    [ $# -lt 1 ] && { print_error "Usage: ir <file.ofs>"; }
    
    COMPILER=$(find_compiler) || print_error "No OFS compiler found"
    
    print_header "LLVM IR for: $1"
    "$COMPILER" ir "$1"
}

cmd_asm() {
    [ $# -lt 1 ] && { print_error "Usage: asm <file.ofs>"; }
    
    COMPILER=$(find_compiler) || print_error "No OFS compiler found"
    
    print_header "Assembly for: $1"
    "$COMPILER" asm "$1"
}

cmd_test() {
    PATTERN="${1:-.}"
    
    print_header "Running tests matching: $PATTERN"
    
    if [ -d "test-build" ]; then
        cd test-build
        ctest -R "$PATTERN" --output-on-failure
        local result=$?
        cd ..
        exit $result
    else
        print_error "test-build directory not found. Run 'utils.sh rebuild' first."
    fi
}

cmd_test_lexer() {
    cmd_test "lexer"
}

cmd_test_parser() {
    cmd_test "parser"
}

cmd_test_semantic() {
    cmd_test "semantic"
}

cmd_benchmark() {
    print_header "Running performance benchmarks"
    
    COMPILER=$(find_compiler) || print_error "No OFS compiler found"
    
    print_info "Benchmarking compiler on various inputs..."
    
    for test_file in ofs/examples/*.ofs; do
        if [ -f "$test_file" ]; then
            filename=$(basename "$test_file")
            start=$(date +%s%N)
            "$COMPILER" check "$test_file" > /dev/null 2>&1 || true
            end=$(date +%s%N)
            elapsed=$((($end - $start) / 1000000))
            echo "  $filename: ${elapsed}ms"
        fi
    done
    
    print_success "Benchmarks complete"
}

cmd_clean() {
    print_header "Cleaning build artifacts"
    
    rm -rf build-ofscc build-mingw test-build dist
    rm -f ofscc_v1 ofscc_v2 ofscc_v3 *.exe
    rm -f *.c *.o *.a output.c
    
    print_success "Clean complete"
}

cmd_rebuild() {
    cmd_clean
    print_header "Running full rebuild..."
    
    if command -v pwsh &> /dev/null && [ "$(uname -s)" = "MINGW64_NT"* ]; then
        pwsh "$SCRIPT_DIR/bootstrap-windows.ps1"
    else
        bash "$SCRIPT_DIR/bootstrap-linux.sh"
    fi
}

cmd_version() {
    COMPILER=$(find_compiler) || { echo "No compiler found"; exit 1; }
    
    print_header "Compiler version"
    echo "Compiler: $COMPILER"
    if [ -f "dist/version.json" ]; then
        echo ""
        cat dist/version.json | grep -E '"version"|"platform"|"buildDate"'
    fi
}

# ──────────────────────────────────────────────────────────────────────────
# Main
# ──────────────────────────────────────────────────────────────────────────

[ $# -eq 0 ] && usage

case "$1" in
    compile)       shift; cmd_compile "$@" ;;
    check)         shift; cmd_check "$@" ;;
    tokens)        shift; cmd_tokens "$@" ;;
    ast)           shift; cmd_ast "$@" ;;
    ir)            shift; cmd_ir "$@" ;;
    asm)           shift; cmd_asm "$@" ;;
    test)          shift; cmd_test "$@" ;;
    test-lexer)    cmd_test_lexer ;;
    test-parser)   cmd_test_parser ;;
    test-semantic) cmd_test_semantic ;;
    benchmark)     cmd_benchmark ;;
    clean)         cmd_clean ;;
    rebuild)       cmd_rebuild ;;
    version)       cmd_version ;;
    *)             echo "Unknown command: $1"; usage ;;
esac
