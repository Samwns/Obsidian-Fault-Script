# Obsidian Fault Script VS Code Extension

Language support for OFS with:
- Syntax highlighting
- Autocomplete for core keywords
- Snippets
- Diagnostics powered by `ofs check`
- Commands to run and check current file
- Purple theme: Obsidian Fault Purple

## Settings

- `ofs.path`: path to the OFS executable (default: `ofs`)
- `ofs.diagnosticsOnType`: run diagnostics while typing (default: true)

## Commands

- `OFS: Run Current File`
- `OFS: Check Current File`

## Install (.vsix)

Use one of these methods:
- `code --install-extension ofs-vscode-extension.vsix`
- VS Code -> Extensions -> `...` -> `Install from VSIX...`

Do not use Visual Studio `VSIXInstaller.exe` for this package. It only supports Visual Studio IDE extensions, while this package targets Microsoft Visual Studio Code.

## Build in GitHub Actions

The release workflow packages the extension with:
- `npm ci`
- `node ./node_modules/@vscode/vsce/vsce package --out ../ofs-vscode-extension.vsix`

## Notes

The extension shells out to the OFS CLI for diagnostics and execution, so make sure OFS is installed and available in PATH.
