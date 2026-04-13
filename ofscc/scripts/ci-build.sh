#!/bin/bash
# OFS CI/CD Release Builder - Automate release creation
# This script is designed for CI/CD pipelines (GitHub Actions, GitLab CI, etc)

set -e

print_step() { echo -e "\033[1;36m→ $1\033[0m"; }
print_success() { echo -e "\033[1;32m✅ $1\033[0m"; }
print_error() { echo -e "\033[1;31m❌ $1\033[0m"; exit 1; }
print_info() { echo -e "\033[0;37mℹ️  $1\033[0m"; }

# Configuration
BRANCH="${CI_COMMIT_REF_NAME:-${GITHUB_REF#refs/heads/}}"
COMMIT="${CI_COMMIT_SHA:-${GITHUB_SHA}}"
VERSION="${VERSION:-1.0.0}"
BUILD_TYPE="${BUILD_TYPE:-Release}"
PLATFORMS="${PLATFORMS:-linux-x64,windows-x64,macos-arm64}"
ENABLE_LTO="${ENABLE_LTO:-false}"

print_step "OFS CI/CD Release Builder"
print_info "Branch: $BRANCH"
print_info "Commit: ${COMMIT:0:8}"
print_info "Version: $VERSION"
print_info "Platforms: $PLATFORMS"

# ──────────────────────────────────────────────────────────────────────────
# Functions
# ──────────────────────────────────────────────────────────────────────────

build_for_platform() {
    local platform=$1
    local build_type="${2:-Release}"
    
    print_step "Building for: $platform"
    
    # Clean previous build
    rm -rf build-$platform-* 2>/dev/null || true
    
    case "$platform" in
        linux-x64)
            build_linux_x64 "$build_type"
            ;;
        windows-x64)
            build_windows_x64 "$build_type"
            ;;
        macos-arm64)
            build_macos_arm64 "$build_type"
            ;;
        *)
            print_error "Unknown platform: $platform"
            ;;
    esac
}

build_linux_x64() {
    local build_type=$1
    
    # Verify LLVM is available
    if ! command -v llvm-config &> /dev/null; then
        print_error "LLVM not found. Install LLVM development files."
    fi
    
    mkdir -p build-linux-x64
    cd build-linux-x64
    
    cmake .. \
        -DCMAKE_BUILD_TYPE="$build_type" \
        -DCMAKE_CXX_FLAGS_RELEASE="${CMAKE_CXX_FLAGS_RELEASE} $([ "$ENABLE_LTO" = "true" ] && echo -n '-flto')" \
        -DCMAKE_EXE_LINKER_FLAGS="${CMAKE_EXE_LINKER_FLAGS} $([ "$ENABLE_LTO" = "true" ] && echo -n '-flto')"
    
    cmake --build . -j $(nproc)
    cmake --install . --prefix ../dist-linux-x64
    
    cd ..
    
    print_success "Linux build complete"
}

build_windows_x64() {
    local build_type=$1
    
    # This would be Windows-specific build
    # For CI, we'd typically cross-compile or use GitHub Actions Windows runner
    
    print_info "Windows build requires Windows runner. Use GitHub Actions or AppVeyor for cross-platform CI."
    print_info "Skipping Windows build on non-Windows platform."
}

build_macos_arm64() {
    local build_type=$1
    
    # This would be macOS-specific build
    # For CI, we'd typically use GitHub Actions macOS runner
    
    print_info "macOS build requires macOS runner. Use GitHub Actions for ARM64 support."
    print_info "Skipping macOS build on non-macOS platform."
}

# ──────────────────────────────────────────────────────────────────────────
# Main build routine
# ──────────────────────────────────────────────────────────────────────────

print_step "Phase 1: Building C++ compiler"
mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
cmake --build . -j $(nproc)
cd ..
print_success "C++ compiler ready"

print_step "Phase 2: Bootstrap OFS compiler"
bash ofscc/scripts/bootstrap-linux.sh --jobs $(nproc)
print_success "OFS compiler bootstrapped"

print_step "Phase 3: Validate build"
bash ofscc/scripts/validate.sh
print_success "Validation complete"

print_step "Phase 4: Create release packages"
bash ofscc/scripts/create-release.sh "$VERSION-$COMMIT"
print_success "Release packages created"

# ──────────────────────────────────────────────────────────────────────────
# Upload artifacts (CI-platform specific)
# ──────────────────────────────────────────────────────────────────────────

print_step "Phase 5: Prepare artifacts"

ARTIFACTS_DIR="release-artifacts"
mkdir -p "$ARTIFACTS_DIR"

# Copy release packages
cp releases/* "$ARTIFACTS_DIR/" 2>/dev/null || true

# Create build metadata
cat > "$ARTIFACTS_DIR/BUILD_INFO.json" << EOF
{
  "version": "$VERSION",
  "commit": "$COMMIT",
  "branch": "$BRANCH",
  "timestamp": "$(date -u +'%Y-%m-%dT%H:%M:%SZ')",
  "compiler": "OFS Self-Hosted",
  "build_type": "$BUILD_TYPE",
  "lto_enabled": $([[ "$ENABLE_LTO" = "true" ]] && echo "true" || echo "false"),
  "platforms": ["$(echo $PLATFORMS | tr ',' '","')"]
}
EOF

print_success "Artifacts prepared in: $ARTIFACTS_DIR"

# ──────────────────────────────────────────────────────────────────────────
# Summary
# ──────────────────────────────────────────────────────────────────────────

echo ""
echo "╔════════════════════════════════════════════════════════════╗"
echo "║              CI/CD BUILD COMPLETE                          ║"
echo "╚════════════════════════════════════════════════════════════╝"
echo ""
echo "Build Information:"
echo "  Version: $VERSION"
echo "  Commit: ${COMMIT:0:8}"
echo "  Branch: $BRANCH"
echo "  Build Type: $BUILD_TYPE"
echo ""
echo "Artifacts available in: $ARTIFACTS_DIR"
ls -lh "$ARTIFACTS_DIR" | tail -n +2 | awk '{print "  - " $9 " (" $5 ")"}'
echo ""
