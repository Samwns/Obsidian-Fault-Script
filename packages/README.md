# OFS Packages (Bibliotecas)

[English](#english) | [Português (Brasil)](#português-brasil)

---

## English

### Install local package (Linux/macOS)

```bash
./packages/install_library.sh packages/dist/terminal-colors.tar.gz
export OFS_LIB_PATH="$HOME/.ofs/libs"
```

### Install local package (Windows PowerShell)

```powershell
./packages/install_library.ps1 -Package packages/dist/terminal-colors.tar.gz
$env:OFS_LIB_PATH = "$HOME/.ofs/libs"
```

### Uninstall library (Linux/macOS)

```bash
./packages/uninstall_library.sh terminal-colors
```

### Uninstall library (Windows PowerShell)

```powershell
./packages/uninstall_library.ps1 -LibraryName terminal-colors
```

### Install from GitHub Packages (requires NPM_TOKEN)

Linux/macOS:
```bash
NPM_TOKEN=your_token ./packages/install_from_github_packages.sh terminal-colors 1.0.0
export OFS_LIB_PATH="$HOME/.ofs/libs"
```

Windows PowerShell:
```powershell
$env:NPM_TOKEN = "your_token"
./packages/install_from_github_packages.ps1 -LibraryName terminal-colors -Version 1.0.0
$env:OFS_LIB_PATH = "$HOME/.ofs/libs"
```

### Test

```bash
OFS_LIB_PATH="$HOME/.ofs/libs" ./ofs/build/ofs ofs/examples/packages_demo.ofs
```

---

## Português (Brasil)

### Instalar pacote local (Linux/macOS)

```bash
./packages/install_library.sh packages/dist/terminal-colors.tar.gz
export OFS_LIB_PATH="$HOME/.ofs/libs"
```

### Instalar pacote local (Windows PowerShell)

```powershell
./packages/install_library.ps1 -Package packages/dist/terminal-colors.tar.gz
$env:OFS_LIB_PATH = "$HOME/.ofs/libs"
```

### Desinstalar biblioteca (Linux/macOS)

```bash
./packages/uninstall_library.sh terminal-colors
```

### Desinstalar biblioteca (Windows PowerShell)

```powershell
./packages/uninstall_library.ps1 -LibraryName terminal-colors
```

### Instalar do GitHub Packages (requer NPM_TOKEN)

Linux/macOS:
```bash
NPM_TOKEN=seu_token ./packages/install_from_github_packages.sh terminal-colors 1.0.0
export OFS_LIB_PATH="$HOME/.ofs/libs"
```

Windows PowerShell:
```powershell
$env:NPM_TOKEN = "seu_token"
./packages/install_from_github_packages.ps1 -LibraryName terminal-colors -Version 1.0.0
$env:OFS_LIB_PATH = "$HOME/.ofs/libs"
```

### Testar

```bash
OFS_LIB_PATH="$HOME/.ofs/libs" ./ofs/build/ofs ofs/examples/packages_demo.ofs
```
