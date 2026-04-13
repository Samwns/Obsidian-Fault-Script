#!/bin/bash
# OFS Scripts Permissions Fixer
# Makes all scripts executable on Unix-like systems

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

echo "🔧 Configurando permissões dos scripts..."
echo ""

# Fazer todos os .sh executáveis
for script in "$SCRIPT_DIR"/*.sh; do
    if [ -f "$script" ]; then
        chmod +x "$script"
        echo "✅ $(basename "$script")"
    fi
done

# Fazer Makefile (geralmente não precisa, mas não custa)
if [ -f "$SCRIPT_DIR/Makefile" ]; then
    chmod +x "$SCRIPT_DIR/Makefile" 2>/dev/null || true
fi

echo ""
echo "✅ Todos os scripts estão executáveis!"
echo ""
echo "Próximos passos:"
echo "  1. cd ofscc/scripts"
echo "  2. bash bootstrap-linux.sh    # Linux/macOS"
echo "     ou"
echo "  3. .\bootstrap-windows.ps1    # Windows PowerShell"
echo ""
