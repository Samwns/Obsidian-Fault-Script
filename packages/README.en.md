# OFS Packages (Libraries)

This directory contains OFS library packages for testing packaging,
unpacking, and installation.

## Included packages

- `terminal-colors`: helpers for colored terminal output.
- `memory-modes`: helpers/documentation for `fracture`, `abyss`, and `fractal`.

## Build packages

```bash
./packages/build_packages.sh
```

Output files are generated in `packages/dist/*.tar.gz`.

## GitHub Packages management

Libraries can be published and managed in GitHub Packages via workflow:

- `.github/workflows/publish-libraries.yml`

Each package is published in the owner namespace:

- `@<owner>/ofs-lib-terminal-colors`
- `@<owner>/ofs-lib-memory-modes`

Manual publish example:

1. Open `Actions` tab on GitHub
2. Select `Publish OFS Libraries` workflow
3. Click `Run workflow`

## Installation (Linux/macOS)

```bash
./packages/install_library.sh packages/dist/terminal-colors.tar.gz
./packages/install_library.sh packages/dist/memory-modes.tar.gz
export OFS_LIB_PATH="$HOME/.ofs/libs"
```

## Installation (Windows PowerShell)

```powershell
./packages/install_library.ps1 -Package packages/dist/terminal-colors.tar.gz
./packages/install_library.ps1 -Package packages/dist/memory-modes.tar.gz
$env:OFS_LIB_PATH = "$HOME/.ofs/libs"
```

## Installation via GitHub Packages

Requires a token with `read:packages` in `NPM_TOKEN`.

Linux/macOS:

```bash
NPM_TOKEN=your_token ./packages/install_from_github_packages.sh terminal-colors 1.0.0
NPM_TOKEN=your_token ./packages/install_from_github_packages.sh memory-modes 1.0.0
export OFS_LIB_PATH="$HOME/.ofs/libs"
```

Windows PowerShell:

```powershell
$env:NPM_TOKEN = "your_token"
./packages/install_from_github_packages.ps1 -LibraryName terminal-colors -Version 1.0.0
./packages/install_from_github_packages.ps1 -LibraryName memory-modes -Version 1.0.0
$env:OFS_LIB_PATH = "$HOME/.ofs/libs"
```

## Quick test

```bash
OFS_LIB_PATH="$HOME/.ofs/libs" ./ofs/build/ofs ofs/examples/packages_demo.ofs
```
