#!/bin/bash
# OFS Release Builder - Create multi-platform releases
# Generates installers for Windows, Linux, and macOS

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$(dirname "$SCRIPT_DIR")")"
OFS_REPO="$PROJECT_ROOT/ofs"
OFSCC_REPO="$PROJECT_ROOT/ofscc"
RELEASE_DATE=$(date -u +'%Y-%m-%d')
VERSION="${1:-1.0.0-native}"
DIST_DIR="${DIST_DIR:-dist}"

# ──────────────────────────────────────────────────────────────────────────
# Utilities
# ──────────────────────────────────────────────────────────────────────────

print_step() { echo -e "\033[1;36m→ $1\033[0m"; }
print_success() { echo -e "\033[1;32m✅ $1\033[0m"; }
print_error() { echo -e "\033[1;31m❌ $1\033[0m"; exit 1; }
print_info() { echo -e "\033[0;37mℹ️  $1\033[0m"; }

check_file() {
    if [ ! -f "$1" ]; then
        print_error "Required file not found: $1"
    fi
}

check_dir() {
    if [ ! -d "$1" ]; then
        print_error "Required directory not found: $1"
    fi
}

# ──────────────────────────────────────────────────────────────────────────
# Prepare release structure
# ──────────────────────────────────────────────────────────────────────────

print_step "Preparing release v$VERSION ($RELEASE_DATE)..."

RELEASE_STAGING="release-staging"
mkdir -p "$RELEASE_STAGING"

# Copy core files
mkdir -p "$RELEASE_STAGING/bin"
mkdir -p "$RELEASE_STAGING/lib"
mkdir -p "$RELEASE_STAGING/stdlib"
mkdir -p "$RELEASE_STAGING/doc"

check_file "$DIST_DIR/ofscc"
cp "$DIST_DIR/ofscc" "$RELEASE_STAGING/bin/"
chmod +x "$RELEASE_STAGING/bin/ofscc"
print_success "Compiler binary copied"

# Copy runtime libs
if [ -f "$DIST_DIR/libofs_runtime.a" ]; then
    cp "$DIST_DIR/libofs_runtime.a" "$RELEASE_STAGING/lib/"
    print_success "Runtime library copied"
fi

# Copy stdlib
check_dir "$DIST_DIR/stdlib"
cp -r "$DIST_DIR/stdlib"/* "$RELEASE_STAGING/stdlib/"
print_success "Standard library copied ($(ls $RELEASE_STAGING/stdlib | wc -l) modules)"

# Copy documentation
cp "$PROJECT_ROOT/docs/GETTING_STARTED.md" "$RELEASE_STAGING/doc/GETTING_STARTED.md"
cp "$PROJECT_ROOT/docs/LANGUAGE_REFERENCE.md" "$RELEASE_STAGING/doc/LANGUAGE_REFERENCE.md"
cp "$PROJECT_ROOT/docs/COMPILER_ARCHITECTURE.md" "$RELEASE_STAGING/doc/COMPILER_ARCHITECTURE.md"
cp "$PROJECT_ROOT/docs/BOOTSTRAP.md" "$RELEASE_STAGING/doc/BOOTSTRAP.md"
cp "$PROJECT_ROOT/README.md" "$RELEASE_STAGING/doc/README.md"
print_success "Documentation copied"

# Create LICENSE
cp "$PROJECT_ROOT/LICENSE" "$RELEASE_STAGING/LICENSE"

# Create version metadata
cat > "$RELEASE_STAGING/VERSION.txt" << EOF
OFS Compiler
Version: $VERSION
Released: $RELEASE_DATE
Type: Native (Self-Hosted)
Build: Bootstrap from OFS self-hosting compiler

Supported Features:
- Full OFS language syntax
- Type checking and inference
- Pattern matching (match/throw/catch)
- Module system (attach/namespace)
- Function values and lambdas
- Monolith with impl blocks
- External function binding (extern vein, rift vein)
- Low-level blocks (fracture, abyss, bedrock, fractal)
- Small integer types (u8, u16, u32, i8, i16, i32)
- Package system with registry
- Standard library (core, io, math, canvas, bedrock)

Compiler Commands:
  ofscc build <file.ofs> -o <output> [-O0|-O2|-O3]
  ofscc check <file.ofs>
  ofscc tokens <file.ofs>
  ofscc ast <file.ofs>
  ofscc ir <file.ofs>
  ofscc asm <file.ofs>

Installation:
- Linux: tar xzf ofs-linux-x64-installer.tar.gz && ./install.sh
- macOS: sudo installer -pkg ofs-macos-arm64-installer.pkg -target /
- Windows: ofs-windows-x64-installer.exe (run installer)

Homepage: https://github.com/Samwns/Obsidian-Fault-Script
EOF
print_success "Version metadata created"

# ──────────────────────────────────────────────────────────────────────────
# Create installable packages
# ──────────────────────────────────────────────────────────────────────────

print_step "Creating release packages..."

mkdir -p "$RELEASE_STAGING/installers"

# Linux package
if command -v tar &> /dev/null; then
    LINUX_PKG="$RELEASE_STAGING/installers/ofs-linux-x64-${VERSION}.tar.gz"
    tar czf "$LINUX_PKG" -C "$RELEASE_STAGING" --exclude=installers .
    if [ -f "$LINUX_PKG" ]; then
        SIZE_KB=$(du -k "$LINUX_PKG" | cut -f1)
        print_success "Linux package created: $(basename $LINUX_PKG) ($SIZE_KB KB)"
    fi
fi

# macOS package
if command -v hdiutil &> /dev/null; then
    MACOS_PKG="$RELEASE_STAGING/installers/ofs-macos-arm64-${VERSION}.dmg"
    hdiutil create -volname "OFS Compiler" -srcfolder "$RELEASE_STAGING" "$MACOS_PKG" 2>/dev/null || true
    if [ -f "$MACOS_PKG" ]; then
        SIZE_KB=$(du -k "$MACOS_PKG" | cut -f1)
        print_success "macOS package created: $(basename $MACOS_PKG) ($SIZE_KB KB)"
    else
        print_info "Skipping macOS DMG (hdiutil not available or not on macOS)"
    fi
fi

# Windows portable ZIP (can be created on any platform)
if command -v zip &> /dev/null; then
    WIN_PKG="$RELEASE_STAGING/installers/ofs-windows-x64-portable-${VERSION}.zip"
    cd "$RELEASE_STAGING"
    zip -r -q "$WIN_PKG" --exclude=installers/* .
    cd - > /dev/null
    if [ -f "$WIN_PKG" ]; then
        SIZE_KB=$(du -k "$WIN_PKG" | cut -f1)
        print_success "Windows portable package created: $(basename $WIN_PKG) ($SIZE_KB KB)"
    fi
fi

# ──────────────────────────────────────────────────────────────────────────
# Move packages to dist
# ──────────────────────────────────────────────────────────────────────────

print_step "Finalizing release artifacts..."

mkdir -p "releases"
if [ -d "$RELEASE_STAGING/installers" ]; then
    cp -r "$RELEASE_STAGING/installers"/* "releases/" 2>/dev/null || true
    print_success "Release packages in: releases/"
    ls -lh releases/ | grep -E ".tar.gz|.zip|.dmg|.exe" | awk '{print "  - " $9 " (" $5 ")"}'
fi

# Create checksums
if command -v sha256sum &> /dev/null; then
    cd releases 2>/dev/null || true
    sha256sum * > CHECKSUMS.sha256 || true
    print_success "Checksums generated: CHECKSUMS.sha256"
    cd - > /dev/null
fi

# Clean staging
rm -rf "$RELEASE_STAGING"

# ──────────────────────────────────────────────────────────────────────────
# Summary
# ──────────────────────────────────────────────────────────────────────────

echo ""
echo "╔════════════════════════════════════════════════════════════╗"
echo "║            RELEASE PACKAGE CREATION COMPLETE               ║"
echo "╚════════════════════════════════════════════════════════════╝"
echo ""
echo "📦 Release: OFS Compiler $VERSION"
echo "📅 Date: $RELEASE_DATE"
echo "📍 Location: $(pwd)/releases/"
echo ""
echo "Generated packages:"
ls -1 releases/ | sed 's/^/   ✓ /'
echo ""
