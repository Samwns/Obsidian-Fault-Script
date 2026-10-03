#!/usr/bin/env bash
# OFS Differential Test Suite
# Validates that 'ofs run' (interpreted) and 'ofs run --native' (compiled LLVM) produce identical output.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../../.." && pwd)"
OFS_BIN="${OFS_BIN:-$REPO_ROOT/ofs/dist/ofs}"

if [ ! -x "$OFS_BIN" ]; then
    echo "ERROR: ofs executable not found at $OFS_BIN"
    exit 1
fi

echo "============================================================"
echo "          OFS Differential Test Suite (Interpreter vs Native)"
echo "============================================================"
echo ""

PASSED=0
FAILED=0
TOTAL=0

# Tests that have full native + interpreter feature support
DIFF_TESTS=(
    "$SCRIPT_DIR/01_arithmetic.ofs"
    "$SCRIPT_DIR/02_control_flow.ofs"
    "$SCRIPT_DIR/03_variables_scoping.ofs"
    "$SCRIPT_DIR/04_functions_recursion.ofs"
    "$SCRIPT_DIR/05_arrays.ofs"
    "$SCRIPT_DIR/06_structs_monolith.ofs"
    "$SCRIPT_DIR/07_pointers.ofs"
    "$SCRIPT_DIR/08_namespaces.ofs"
)

for test_file in "${DIFF_TESTS[@]}"; do
    test_name="$(basename "$test_file")"
    TOTAL=$((TOTAL + 1))
    printf "[%d/%d] Testing %s... " "$TOTAL" "${#DIFF_TESTS[@]}" "$test_name"

    # 1. Interpreter execution (stdout only; reporter progress/diagnostics live on stderr)
    interp_err="$(mktemp)"
    interp_out="$("$OFS_BIN" "$test_file" 2>"$interp_err")" || {
        echo "FAIL (interpreter crashed)"
        echo "$interp_out"
        cat "$interp_err"
        rm -f "$interp_err"
        FAILED=$((FAILED + 1))
        continue
    }

    # 2. Native execution (stdout only; the compiler progress reporter writes to stderr)
    native_err="$(mktemp)"
    native_raw="$("$OFS_BIN" run --native "$test_file" 2>"$native_err")" || {
        echo "FAIL (native build/run failed)"
        echo "$native_raw"
        cat "$native_err"
        rm -f "$native_err"
        FAILED=$((FAILED + 1))
        continue
    }
    rm -f "$interp_err" "$native_err"

    # Strip compile progress lines and leading blank lines from native run output
    native_out="$(echo "$native_raw" | grep -v '^ofscc —' | grep -v '^\[' | grep -v '^  OK' | sed -e '/./,$!d')"

    # 3. Compare outputs
    if [ "$interp_out" = "$native_out" ]; then
        echo "PASS (interpreter === native)"
        PASSED=$((PASSED + 1))
    else
        echo "FAIL (output mismatch)"
        echo "--- Interpreter Output ---"
        echo "$interp_out"
        echo "--- Native Output ---"
        echo "$native_out"
        echo "--- Diff ---"
        diff -u <(echo "$interp_out") <(echo "$native_out") || true
        FAILED=$((FAILED + 1))
    fi
done

# Test interpreter-only advanced features (e.g. dynamic impl dispatch)
echo ""
echo "--- Interpreter Advanced Features ---"
if [ -f "$SCRIPT_DIR/09_impl_methods.ofs" ]; then
    printf "Testing 09_impl_methods.ofs (interpreter)... "
    impl_out="$("$OFS_BIN" "$SCRIPT_DIR/09_impl_methods.ofs" 2>&1)"
    expected_impl=$'50\n30'
    if [ "$impl_out" = "$expected_impl" ]; then
        echo "PASS"
        PASSED=$((PASSED + 1))
    else
        echo "FAIL"
        echo "Expected: $expected_impl"
        echo "Got: $impl_out"
        FAILED=$((FAILED + 1))
    fi
    TOTAL=$((TOTAL + 1))
fi

echo ""
echo "============================================================"
echo "Summary: $PASSED / $TOTAL passed ($FAILED failed)"
echo "============================================================"

if [ "$FAILED" -eq 0 ]; then
    echo "✓ ALL DIFFERENTIAL TESTS PASSED!"
    exit 0
else
    echo "✗ SOME DIFFERENTIAL TESTS FAILED"
    exit 1
fi
