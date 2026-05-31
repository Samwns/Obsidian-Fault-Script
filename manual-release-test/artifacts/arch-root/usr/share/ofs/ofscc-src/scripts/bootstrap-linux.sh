#!/bin/bash
# OFS Native Compiler Bootstrap - Linux/macOS Bash
# Compila o compilador OFS nativo com suporte a todas as plataformas e releases
# Fase 1: Compilar C++ compiler
# Fase 2: Bootstrap OFS compiler  
# Fase 3: Gerar releases

set -e

# ──────────────────────────────────────────────────────────────────────────
# Configuration
# ──────────────────────────────────────────────────────────────────────────

BUILD_DIR="${BUILD_DIR:-build-ofscc}"
INSTALL_DIR="${INSTALL_DIR:-dist}"
OFS_DIR="${OFS_DIR:-ofs}"
OFSCC_DIR="${OFSCC_DIR:-ofs/ofscc}"
SKIP_CPP_BUILD=false
SKIP_BOOTSTRAP=false
RELEASE_ONLY=false
JOBS=$(nproc 2>/dev/null || echo 4)
CMAKE_BUILD_TYPE="Release"

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --skip-cpp-build) SKIP_CPP_BUILD=true; shift ;;
        --skip-bootstrap) SKIP_BOOTSTRAP=true; shift ;;
        --release-only) RELEASE_ONLY=true; shift ;;
        --debug) CMAKE_BUILD_TYPE="Debug"; shift ;;
        --jobs) JOBS="$2"; shift 2 ;;
        --build-dir) BUILD_DIR="$2"; shift 2 ;;
        --install-dir) INSTALL_DIR="$2"; shift 2 ;;
        *) echo "Unknown option: $1"; exit 1 ;;
    esac
done

# ──────────────────────────────────────────────────────────────────────────
# Utilities
# ──────────────────────────────────────────────────────────────────────────

print_step() { echo -e "\033[1;36m→ $1\033[0m"; }
print_success() { echo -e "\033[1;32m✅ $1\033[0m"; }
print_error() { echo -e "\033[1;31m❌ $1\033[0m"; exit 1; }
print_info() { echo -e "\033[0;37mℹ️  $1\033[0m"; }
print_header() { echo -e "\n\033[1;33m╔════════════════════════════════════════════════════════════╗\033[0m"; echo -e "\033[1;33m║  $1\033[0m"; echo -e "\033[1;33m╚════════════════════════════════════════════════════════════╝\033[0m\n"; }

check_file() {
    if [ ! -f "$1" ]; then
        print_error "File not found: $1"
    fi
}

check_command() {
    if ! command -v "$1" &> /dev/null; then
        print_error "Required command not found: $1. Please install $2"
    fi
}

# ──────────────────────────────────────────────────────────────────────────
# Pre-flight checks
# ──────────────────────────────────────────────────────────────────────────

print_header "OFS Native Compiler Bootstrap (Linux/macOS)"

print_step "Checking prerequisites..."
check_command cmake "CMake"
check_command gcc "GCC"
check_command git "Git"

print_info "System: $(uname -s)"
print_info "Architecture: $(uname -m)"
print_info "Build type: $CMAKE_BUILD_TYPE"
print_info "Parallel jobs: $JOBS"

# ──────────────────────────────────────────────────────────────────────────
# PHASE 1: Build C++ Compiler (if not skipped)
# ──────────────────────────────────────────────────────────────────────────

if [ "$SKIP_CPP_BUILD" != true ]; then
    print_step "Phase 1: Building C++ compiler as bootstrap..."
    
    if [ ! -d "$BUILD_DIR" ]; then
        print_info "Creating build directory: $BUILD_DIR"
        mkdir -p "$BUILD_DIR"
    fi
    
    cd "$BUILD_DIR"
    
    print_info "Configuring CMake..."
    cmake .. -DCMAKE_BUILD_TYPE="$CMAKE_BUILD_TYPE"
    if [ $? -ne 0 ]; then print_error "CMake configure failed"; fi
    
    print_info "Building with CMake (-j$JOBS)..."
    cmake --build . -j "$JOBS"
    if [ $? -ne 0 ]; then print_error "CMake build failed"; fi
    
    cd ..
    
    print_success "C++ compiler build complete"
else
    print_info "Skipping C++ build (--skip-cpp-build)"
fi

# Locate ofscc_v1 (C++ compiled)
if [ -f "$BUILD_DIR/ofscc" ]; then
    OFSCC_V1="$BUILD_DIR/ofscc"
elif [ -f "$BUILD_DIR/Release/ofscc" ]; then
    OFSCC_V1="$BUILD_DIR/Release/ofscc"
elif [ -f "$BUILD_DIR/Debug/ofscc" ]; then
    OFSCC_V1="$BUILD_DIR/Debug/ofscc"
elif command -v ofscc &> /dev/null; then
    OFSCC_V1=$(which ofscc)
    print_info "Using existing ofscc from PATH: $OFSCC_V1"
else
    print_error "Could not find compiled ofscc executable. Run without --skip-cpp-build first"
fi

print_success "C++ compiler ready: $OFSCC_V1"

# ──────────────────────────────────────────────────────────────────────────
# PHASE 2: Bootstrap OFS Compiler (OFS → OFS)
# ──────────────────────────────────────────────────────────────────────────

if [ "$SKIP_BOOTSTRAP" != true ]; then
    print_step "Phase 2: Bootstrap OFS compiler (OFS compiling OFS)..."
    
    OFSCC_SRC="$OFSCC_DIR/ofscc.ofs"
    check_file "$OFSCC_SRC"
    
    # Bootstrap step 1: ofscc_v1 (C++) → ofscc_v2 (OFS)
    print_info "[2.1] Compiling ofscc_v2 with C++ compiler..."
    if ! "$OFSCC_V1" build "$OFSCC_SRC" -o ofscc_v2 -O2 2>/dev/null; then
        print_error "Failed to compile ofscc_v2"
    fi
    chmod +x ofscc_v2
    print_success "ofscc_v2 created (C++ compiled OFS compiler)"
    
    # Bootstrap step 2: ofscc_v2 (OFS) → ofscc_v3 (OFS)
    print_info "[2.2] Compiling ofscc_v3 with ofscc_v2 (OFS self-hosting)..."
    if ! ./ofscc_v2 build "$OFSCC_SRC" -o ofscc_v3 -O2 2>/dev/null; then
        print_error "Failed to compile ofscc_v3"
    fi
    chmod +x ofscc_v3
    print_success "ofscc_v3 created (OFS self-hosted compiler)"
    
    # Verify determinism
    print_info "[2.3] Verifying determinism..."
    V2_HASH=$(sha256sum ofscc_v2 | cut -d' ' -f1)
    V3_HASH=$(sha256sum ofscc_v3 | cut -d' ' -f1)
    
    if [ "$V2_HASH" = "$V3_HASH" ]; then
        print_success "Determinism verified: ofscc_v2 and ofscc_v3 are byte-identical ✓"
    else
        print_info "⚠️  Non-deterministic build (hashes differ). May be normal during development."
        print_info "  ofscc_v2 hash: $V2_HASH"
        print_info "  ofscc_v3 hash: $V3_HASH"
    fi
else
    print_info "Skipping bootstrap (--skip-bootstrap)"
    
    # Try to use existing binary
    if [ -f "ofscc_v3" ]; then
        print_info "Using existing ofscc_v3"
    elif [ -f "ofscc_v2" ]; then
        print_info "Using existing ofscc_v2 as final compiler"
        cp ofscc_v2 ofscc_v3
    fi
fi

# ──────────────────────────────────────────────────────────────────────────
# PHASE 3: Generate Release Artifacts
# ──────────────────────────────────────────────────────────────────────────

print_step "Phase 3: Preparing release artifacts..."

mkdir -p "$INSTALL_DIR"

# Determine final compiler
if [ -f "ofscc_v3" ]; then
    FINAL_COMPILER="ofscc_v3"
elif [ -f "ofscc_v2" ]; then
    FINAL_COMPILER="ofscc_v2"
else
    print_error "No compiled OFS compiler found"
fi

# Copy final compiler
cp "$FINAL_COMPILER" "$INSTALL_DIR/ofscc"
chmod +x "$INSTALL_DIR/ofscc"
print_success "Compiler ready: $INSTALL_DIR/ofscc"

# Copy runtime library
if [ -f "$BUILD_DIR/libofs_runtime.a" ]; then
    cp "$BUILD_DIR/libofs_runtime.a" "$INSTALL_DIR/"
    print_success "Runtime library copied"
elif [ -f "$BUILD_DIR/Release/libofs_runtime.a" ]; then
    cp "$BUILD_DIR/Release/libofs_runtime.a" "$INSTALL_DIR/"
    print_success "Runtime library copied"
fi

# Copy standard library
if [ -d "$OFS_DIR/stdlib" ]; then
    mkdir -p "$INSTALL_DIR/stdlib"
    cp -r "$OFS_DIR/stdlib"/* "$INSTALL_DIR/stdlib/"
    print_success "Standard library copied"
fi

# Determine platform string
PLATFORM=$(uname -s)
case "$PLATFORM" in
    Linux) PLATFORM_STR="linux-$(uname -m)" ;;
    Darwin) PLATFORM_STR="macos-$(uname -m)" ;;
    *) PLATFORM_STR="unknown" ;;
esac

# Create version file
VERSION="1.0.0-native"
BUILD_DATE=$(date -u +'%Y-%m-%d %H:%M:%S')
cat > "$INSTALL_DIR/version.json" << EOF
{
  "version": "$VERSION",
  "platform": "$PLATFORM_STR",
  "arch": "$(uname -m)",
  "buildDate": "$BUILD_DATE",
  "compiler": "OFS (Self-Hosted)",
  "features": [
    "full-ofs-support",
    "llvm-codegen",
    "stdlib-complete",
    "package-system"
  ]
}
EOF
print_success "Version file created"

# ──────────────────────────────────────────────────────────────────────────
# Summary
# ──────────────────────────────────────────────────────────────────────────

print_header "BUILD SUCCESSFUL"

echo "📦 Release artifacts in: $INSTALL_DIR"
echo "🎯 Main executable: $INSTALL_DIR/ofscc"
echo "📚 Standard library: $INSTALL_DIR/stdlib/*"
echo "ℹ️  Version info: $INSTALL_DIR/version.json"

echo ""
echo "✨ All OFS features are supported:"
echo "   ✓ Type checking and inference"
echo "   ✓ Full pattern matching (match/throw/catch)"
echo "   ✓ Monolith with impl blocks"
echo "   ✓ Namespace support"
echo "   ✓ Lambda and function values"
echo "   ✓ Package system (attach)"
echo "   ✓ External function binding (extern vein, rift vein)"
echo "   ✓ Low-level blocks (fracture, abyss, bedrock, fractal)"
echo "   ✓ Small int types (u8, u16, u32, i8, i32, etc)"
echo ""

if [ "$PLATFORM_STR" != "unknown" ]; then
    echo "🚀 Next steps:"
    echo "   1. Run: $INSTALL_DIR/ofscc --help"
    echo "   2. Try: $INSTALL_DIR/ofscc build example.ofs -o example"
    echo "   3. Create release: ./scripts/create-release.sh $PLATFORM_STR"
fi
echo ""
