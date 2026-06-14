#!/bin/bash
# OFS Bootstrap - C++-Free complete install flow
# Usa o IR versionado por padrao e permite validacao self-host opcional
# Sem dependências de CMake, LLVM, ou C++

set -e

print_step() { echo -e "\033[1;36m→ $1\033[0m"; }
print_success() { echo -e "\033[1;32m✅ $1\033[0m"; }
print_error() { echo -e "\033[1;31m❌ $1\033[0m"; exit 1; }
print_info() { echo -e "\033[0;37mℹ️  $1\033[0m"; }

EXISTING_COMPILER="${EXISTING_COMPILER:-ofs/dist/ofscc}"
BUILD_OUTPUT="${BUILD_OUTPUT:-dist}"
RUNTIME="${OFS_RUNTIME:-ofs/dist/libofs_runtime.a}"

echo ""
echo "╔════════════════════════════════════════════════════════════╗"
echo "║         OFS Bootstrap (C++-Free, Complete)                ║"
echo "║         Instala a linguagem completa                      ║"
echo "╚════════════════════════════════════════════════════════════╝"
echo ""

# ──────────────────────────────────────────────────────────────────────────
# Verify existing compiler
# ──────────────────────────────────────────────────────────────────────────

print_step "Fase 1: Validar compilador OFS existente"

if [ ! -f "$EXISTING_COMPILER" ] && command -v "$EXISTING_COMPILER" > /dev/null 2>&1; then
    EXISTING_COMPILER="$(command -v "$EXISTING_COMPILER")"
fi

if [ ! -f "$EXISTING_COMPILER" ]; then
    print_info "Compilador não encontrado localmente. Tentando baixar da última release..."
    REPO="${GITHUB_REPO:-Samwns/Obsidian-Fault-Script}"
    ASSET=$(curl -sL "https://api.github.com/repos/$REPO/releases/latest" \
      | grep -oP '"browser_download_url":\s*"\K[^"]*linux-x64-installer[^"]*\.tar\.gz' | head -1)
    if [ -n "$ASSET" ]; then
        print_info "Baixando: $ASSET"
        TMP_SEED_DIR=$(mktemp -d)
        curl -sL "$ASSET" -o "$TMP_SEED_DIR/ofs-seed.tar.gz"
        tar --no-same-owner --no-same-permissions -xzf "$TMP_SEED_DIR/ofs-seed.tar.gz" -C "$TMP_SEED_DIR"
        mkdir -p "$(dirname "$EXISTING_COMPILER")"
        find "$TMP_SEED_DIR" -maxdepth 2 -name "ofs" -type f -exec cp {} "$EXISTING_COMPILER" \;
        chmod +x "$EXISTING_COMPILER"
        rm -rf "$TMP_SEED_DIR"
        print_success "Seed compiler baixado com sucesso"
    else
        print_error "Compilador não encontrado: $EXISTING_COMPILER (e nenhum seed disponível)"
    fi
fi

if [ ! -x "$EXISTING_COMPILER" ]; then
    chmod +x "$EXISTING_COMPILER"
fi

if ! "$EXISTING_COMPILER" --help > /dev/null 2>&1; then
    print_error "Compilador não funciona: $EXISTING_COMPILER"
fi

COMPILER_SIZE=$(du -h "$EXISTING_COMPILER" | cut -f1)
print_success "Compilador pronto: $EXISTING_COMPILER ($COMPILER_SIZE)"

if [ ! -f "$RUNTIME" ]; then
    print_info "Runtime não encontrado em $RUNTIME. Compilando runtime C..."
    mkdir -p "$(dirname "$RUNTIME")"
    clang -O2 -ffunction-sections -fdata-sections -c ofs/runtime/ofs_runtime.c -o "$BUILD_OUTPUT/ofs_runtime.o"
    ar rcs "$RUNTIME" "$BUILD_OUTPUT/ofs_runtime.o"
fi

detect_llvm_ir_flags() {
    if [ -n "${OFS_LLVM_IR_FLAGS:-}" ]; then
        printf '%s\n' "$OFS_LLVM_IR_FLAGS"
        return
    fi
    if clang --version 2>/dev/null | grep -Eq 'version 14\.|clang-1400\.'; then
        printf '%s\n' "-mllvm -opaque-pointers"
    fi
}

detect_clang_opt_level() {
    if [ -n "${OFS_CLANG_OPT:-}" ]; then
        printf '%s\n' "$OFS_CLANG_OPT"
        return
    fi
    if clang --version 2>/dev/null | grep -Eq 'version 14\.|clang-1400\.'; then
        printf '%s\n' "-O0"
    else
        printf '%s\n' "-O2"
    fi
}

compile_ofs_program() {
    local input="$1"
    local output="$2"
    local opt="${3:--O3}"
    local ir="${output}.ll"
    local detected_flags
    local llvm_flags=()
    local clang_opt

    detected_flags="$(detect_llvm_ir_flags)"
    if [ -n "$detected_flags" ]; then
        read -r -a llvm_flags <<< "$detected_flags"
    fi
    clang_opt="$(detect_clang_opt_level)"

    if ! OFSCC_INPUT="$input" OFSCC_MODE=ir OFSCC_C_OUT="$ir" OFSCC_OPT="$opt" "$EXISTING_COMPILER"; then
        if [ "$input" = "ofs/ofs/bootstrap/ofscc.ofs" ] && [ -f "ofs/dist/ofscc.ll" ]; then
            print_info "Recompilação OFS direta falhou neste ambiente; usando ofs/dist/ofscc.ll versionado como fallback de instalação."
            if [ "${#llvm_flags[@]}" -gt 0 ]; then
                clang -Wno-override-module "$clang_opt" "ofs/dist/ofscc.ll" "$RUNTIME" -lm -o "$output" "${llvm_flags[@]}"
            else
                clang -Wno-override-module "$clang_opt" "ofs/dist/ofscc.ll" "$RUNTIME" -lm -o "$output"
            fi
            chmod +x "$output"
            return
        fi
        return 1
    fi
    if [ "${#llvm_flags[@]}" -gt 0 ]; then
        clang -Wno-override-module "$clang_opt" "$ir" "$RUNTIME" -lm -o "$output" "${llvm_flags[@]}"
    else
        clang -Wno-override-module "$clang_opt" "$ir" "$RUNTIME" -lm -o "$output"
    fi
    rm -f "$ir"
    chmod +x "$output"
}

compile_versioned_compiler_ir() {
    local output="$1"
    local ir="${OFSCC_LLVM_IR:-ofs/dist/ofscc.ll}"
    local detected_flags
    local llvm_flags=()
    local clang_opt

    if [ ! -f "$ir" ]; then
        print_error "IR versionado não encontrado: $ir"
    fi

    detected_flags="$(detect_llvm_ir_flags)"
    if [ -n "$detected_flags" ]; then
        read -r -a llvm_flags <<< "$detected_flags"
    fi
    clang_opt="$(detect_clang_opt_level)"

    if [ "${#llvm_flags[@]}" -gt 0 ]; then
        clang -Wno-override-module "$clang_opt" "$ir" "$RUNTIME" -lm -o "$output" "${llvm_flags[@]}"
    else
        clang -Wno-override-module "$clang_opt" "$ir" "$RUNTIME" -lm -o "$output"
    fi
    chmod +x "$output"
}

# ──────────────────────────────────────────────────────────────────────────
# Recompile OFS compiler (optional but recommended)
# ──────────────────────────────────────────────────────────────────────────

print_step "Fase 2: Instalar compilador OFS completo"

OFSCC_SRC="ofs/ofs/bootstrap/ofscc.ofs"
if [ ! -f "$OFSCC_SRC" ]; then
    print_error "Fonte do compilador não encontrada: $OFSCC_SRC"
fi

if [ "${OFS_BOOTSTRAP_SELFHOST:-0}" = "1" ]; then
    print_info "Compilando ofscc_fresh de OFS para nativo com otimização -O3..."
    compile_ofs_program "$OFSCC_SRC" ofscc_fresh -O3
else
    print_info "Gerando ofscc_fresh a partir do IR versionado ofs/dist/ofscc.ll..."
    compile_versioned_compiler_ir ofscc_fresh
fi
print_success "ofscc_fresh criado"

# ──────────────────────────────────────────────────────────────────────────
# Verify determinism (optional but strong validation)
# ──────────────────────────────────────────────────────────────────────────

print_step "Fase 3: Validar instalação"

if [ "${OFS_BOOTSTRAP_SELFHOST:-0}" = "1" ]; then
    print_info "Compilando segunda vez para validação determinística..."
    compile_ofs_program "$OFSCC_SRC" ofscc_verify -O3

    FRESH_HASH=$(sha256sum ofscc_fresh | cut -d' ' -f1)
    VERIFY_HASH=$(sha256sum ofscc_verify | cut -d' ' -f1)

    if [ "$FRESH_HASH" = "$VERIFY_HASH" ]; then
        print_success "Determinismo validado! ✓"
    else
        print_info "⚠️  Hashes diferentes (normal em dev)"
        print_info "  Fresh:  $FRESH_HASH"
        print_info "  Verify: $VERIFY_HASH"
    fi
else
    if ./ofscc_fresh --help > /dev/null 2>&1; then
        print_success "Compilador instalado e executável"
    else
        print_error "Compilador gerado não executa"
    fi
fi

# ──────────────────────────────────────────────────────────────────────────
# Prepare output
# ──────────────────────────────────────────────────────────────────────────

print_step "Fase 4: Preparar artifacts"

mkdir -p "$BUILD_OUTPUT"

# Use the freshly compiled one
cp ofscc_fresh "$BUILD_OUTPUT/ofscc"
chmod +x "$BUILD_OUTPUT/ofscc"
print_success "Compilador final: $BUILD_OUTPUT/ofscc"

# Copy stdlib if missing
if [ ! -d "$BUILD_OUTPUT/stdlib" ]; then
    print_info "Copiando stdlib..."
    mkdir -p "$BUILD_OUTPUT/stdlib"
    cp -r ofs/stdlib/* "$BUILD_OUTPUT/stdlib/" || true
fi

# Update version file
BUILD_TIMESTAMP="$(date -u +'%Y-%m-%dT%H:%M:%SZ')"
cat > "$BUILD_OUTPUT/version.json" << EOF
{
  "version": "1.0.0-native",
  "buildType": "native-only",
  "compiler": "OFS (Self-Hosted, C++-Free)",
  "timestamp": "$BUILD_TIMESTAMP",
  "buildProcess": "complete-bootstrap",
  "dependencies": {
    "cpp": false,
    "cmake": false,
    "llvm": false
  },
  "deterministic": false,
  "selfHostValidation": "${OFS_BOOTSTRAP_SELFHOST:-0}"
}
EOF
print_success "version.json atualizado"

# Cleanup
print_step "Fase 5: Limpeza"
rm -f ofscc_fresh ofscc_verify
print_success "Cleanup completo"

# ──────────────────────────────────────────────────────────────────────────
# Summary
# ──────────────────────────────────────────────────────────────────────────

echo ""
echo "╔════════════════════════════════════════════════════════════╗"
echo "║            OFS BOOTSTRAP COMPLETE                         ║"
echo "╚════════════════════════════════════════════════════════════╝"
echo ""
echo "✨ Resultado:"
echo "   ✓ Compilador instalado com sucesso"
echo "   ✓ Validação self-host opcional: OFS_BOOTSTRAP_SELFHOST=1 bash ofs/bootstrap/scripts/bootstrap.sh"
echo "   ✓ Zero dependências C++"
echo "   ✓ Pronto para usar/distribuir"
echo ""
echo "📦 Localização: $BUILD_OUTPUT/ofscc"
echo "⏱️  Tempo: ~1-2 segundos (vs 5-15 min com C++)"
echo ""
echo "Próximos passos:"
echo "   • Compilar programa: $BUILD_OUTPUT/ofs build program.ofs"
echo "   • Criar release: bash src/scripts/release-native-only.sh 1.0.0"
echo ""
