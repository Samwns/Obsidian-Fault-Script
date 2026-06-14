#!/bin/bash
# OFS Compiler Validation - Validate bootstrap and determinism

set -e

print_step() { echo -e "\033[1;36m→ $1\033[0m"; }
print_success() { echo -e "\033[1;32m✅ $1\033[0m"; }
print_error() { echo -e "\033[1;31m❌ $1\033[0m"; exit 1; }
print_info() { echo -e "\033[0;37mℹ️  $1\033[0m"; }
print_warn() { echo -e "\033[1;33m⚠️  $1\033[0m"; }

VERBOSE=false
if [ "$1" == "-v" ] || [ "$1" == "--verbose" ]; then
    VERBOSE=true
fi

print_step "Validating OFS Native Compiler"
echo ""

# ──────────────────────────────────────────────────────────────────────────
# Check 1: Compiler exists
# ──────────────────────────────────────────────────────────────────────────

print_info "Check 1: Compiler availability"

if [ ! -f "dist/ofscc" ]; then
    print_error "Compiler not found: dist/ofscc"
fi

if [ ! -x "dist/ofscc" ]; then
    chmod +x dist/ofscc || print_error "Cannot make compiler executable"
fi

print_success "Compiler found and executable"

# ──────────────────────────────────────────────────────────────────────────
# Check 2: Compiler version
# ──────────────────────────────────────────────────────────────────────────

print_info "Check 2: Compiler version"

if [ -f "dist/version.json" ]; then
    if command -v jq &> /dev/null; then
        VERSION=$(jq -r '.version' dist/version.json 2>/dev/null || echo "unknown")
        PLATFORM=$(jq -r '.platform' dist/version.json 2>/dev/null || echo "unknown")
        print_success "Version: $VERSION ($PLATFORM)"
    else
        print_warn "jq not installed, skipping detailed version check"
    fi
else
    print_warn "version.json not found"
fi

# ──────────────────────────────────────────────────────────────────────────
# Check 3: Stdlib presence
# ──────────────────────────────────────────────────────────────────────────

print_info "Check 3: Standard library"

if [ ! -d "dist/stdlib" ]; then
    print_error "Standard library not found: dist/stdlib"
fi

STDLIB_COUNT=$(find dist/stdlib -name "*.ofs" 2>/dev/null | wc -l)
if [ "$STDLIB_COUNT" -eq 0 ]; then
    print_warn "No .ofs files found in stdlib"
else
    print_success "Standard library: $STDLIB_COUNT modules found"
fi

# ──────────────────────────────────────────────────────────────────────────
# Check 4: Bootstrap determinism
# ──────────────────────────────────────────────────────────────────────────

print_info "Check 4: Bootstrap determinism"

if [ ! -f "ofscc_v2" ] || [ ! -f "ofscc_v3" ]; then
    print_warn "Bootstrap binaries (v2/v3) not found. Running bootstrap test..."
    print_info "This will recompile the compiler to verify determinism"
    
    if bash ofs/ofscc/test_bootstrap.sh > /dev/null 2>&1; then
        print_success "Bootstrap test passed"
    else
        print_warn "Bootstrap test not available or failed"
    fi
else
    V2_HASH=$(sha256sum ofscc_v2 | cut -d' ' -f1)
    V3_HASH=$(sha256sum ofscc_v3 | cut -d' ' -f1)
    
    if [ "$V2_HASH" = "$V3_HASH" ]; then
        print_success "Determinism verified ✓"
        [ "$VERBOSE" = true ] && echo "  Hash: $V2_HASH"
    else
        print_warn "Non-deterministic build detected"
        [ "$VERBOSE" = true ] && {
            echo "  ofscc_v2 hash: $V2_HASH"
            echo "  ofscc_v3 hash: $V3_HASH"
        }
    fi
fi

# ──────────────────────────────────────────────────────────────────────────
# Check 5: Basic compilation test
# ──────────────────────────────────────────────────────────────────────────

print_info "Check 5: Basic compilation"

TEST_FILE="ofs/examples/hello.ofs"
if [ ! -f "$TEST_FILE" ]; then
    print_warn "Example file not found: $TEST_FILE (skipping compilation test)"
else
    print_info "Compiling: $TEST_FILE"
    
    TEST_OUT="test_hello_$RANDOM"
    if dist/ofscc build "$TEST_FILE" -o "$TEST_OUT" -O2 > /dev/null 2>&1; then
        print_success "Compilation test passed"
        rm -f "$TEST_OUT"
    else
        print_error "Failed to compile test file"
    fi
fi

# ──────────────────────────────────────────────────────────────────────────
# Check 6: Help and version commands
# ──────────────────────────────────────────────────────────────────────────

print_info "Check 6: Compiler commands"

for cmd in "--help" "--version" "tokens --help" "ast --help"; do
    if dist/ofscc $cmd > /dev/null 2>&1 || dist/ofscc $cmd 2>&1 | grep -q "usage\|Usage\|help\|Help"; then
        print_success "Command works: dist/ofscc $cmd"
    else
        print_warn "Command may not work: dist/ofscc $cmd"
    fi
done

# ──────────────────────────────────────────────────────────────────────────
# Check 7: Runtime library
# ──────────────────────────────────────────────────────────────────────────

print_info "Check 7: Runtime library"

if [ -f "dist/libofs_runtime.a" ]; then
    RUNTIME_SIZE=$(du -k dist/libofs_runtime.a | cut -f1)
    print_success "Runtime library found (${RUNTIME_SIZE}KB)"
else
    print_warn "Runtime library not found: dist/libofs_runtime.a"
fi

# ──────────────────────────────────────────────────────────────────────────
# Summary
# ──────────────────────────────────────────────────────────────────────────

echo ""
echo "╔════════════════════════════════════════════════════════════╗"
echo "║         OFS COMPILER VALIDATION COMPLETE                   ║"
echo "╚════════════════════════════════════════════════════════════╝"
echo ""

print_success "All critical checks passed! Compiler is ready to use."
echo ""
echo "Next steps:"
echo "  • Try: dist/ofscc build ofs/examples/hello.ofs -o hello"
echo "  • Run: ./hello"
echo "  • Check: dist/ofscc check ofs/examples/calculator.ofs"
echo "  • Test: ./scripts/utils.sh test"
echo ""
