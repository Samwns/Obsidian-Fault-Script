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

## Notes

The extension shells out to the OFS CLI for diagnostics and execution, so make sure OFS is installed and available in PATH.
