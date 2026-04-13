#!/bin/bash
# OFS Bootstrap Test Script
# Executa as 3 passagens do bootstrap e verifica determinismo

set -e

echo "=== OFS Bootstrap Test ==="
echo ""

COMPILER_CPP="${1:-ofs}"
OFSCC_SRC="ofs/ofscc/ofscc.ofs"
TEST_PROG="ofs/examples/hello.ofs"

if [ ! -f "$OFSCC_SRC" ]; then
    echo "ERROR: $OFSCC_SRC not found"
    exit 1
fi

if [ ! -f "$TEST_PROG" ]; then
    echo "ERROR: $TEST_PROG not found"
    exit 1
fi

# Passo 1: Compilar ofscc com o compilador C++
echo "[1/4] Building ofscc_v1 with C++ compiler..."
$COMPILER_CPP build $OFSCC_SRC -o ofscc_v1 2>/dev/null || {
    echo "ERROR: Failed to compile with C++ compiler"
    exit 1
}
echo "  ✓ ofscc_v1 created"
echo ""

# Passo 2: OFS compila OFS (primeira vez)
echo "[2/4] Building ofscc_v2 with ofscc_v1 (OFS → OFS)..."
./ofscc_v1 $OFSCC_SRC -o ofscc_v2 2>/dev/null || {
    echo "ERROR: ofscc_v1 failed to compile ofscc.ofs"
    exit 1
}
echo "  ✓ ofscc_v2 created"
echo ""

# Passo 3: OFS compila OFS (segunda vez)
echo "[3/4] Building ofscc_v3 with ofscc_v2 (OFS → OFS)..."
./ofscc_v2 $OFSCC_SRC -o ofscc_v3 2>/dev/null || {
    echo "ERROR: ofscc_v2 failed to compile ofscc.ofs"
    exit 1
}
echo "  ✓ ofscc_v3 created"
echo ""

# Passo 4: Verificar determinismo
echo "[4/4] Verifying determinism (v2 === v3)..."
if cmp -s ofscc_v2 ofscc_v3; then
    echo "  ✓ BINARIES IDENTICAL"
    echo ""
    echo "╔════════════════════════════════════════╗"
    echo "║     BOOTSTRAP COMPLETE SUCCESS ✓       ║"
    echo "║  OFS is now self-hosting and ready!   ║"
    echo "╚════════════════════════════════════════╝"
    echo ""
    
    # Cleanup
    rm -f ofscc_v1 ofscc_v2 ofscc_v3 output.c
    exit 0
else
    echo "  ✗ BINARIES DIFFER"
    echo ""
    echo "Differences found in bootstrap:"
    diff <(xxd ofscc_v2) <(xxd ofscc_v3) | head -20
    echo ""
    echo "Investigating..."
    ls -lh ofscc_v*
    exit 1
fi
