# OFS Packages

Pacotes OFS são módulos `.ofs` organizados para instalação local ou publicação no catálogo do projeto.

[English](#english) | [Português (Brasil)](#português-brasil)

---

## English

### Package flow

1. Search with `uncover`
2. Install with `infuse`
3. Use in code with `attach {name}`
4. Update with `reinfuse`

### CLI usage

```bash
uncover fmt
infuse fmt
infuse {fmt:stable}
reinfuse fmt
```

### Install local package archive

Linux/macOS:

```bash
./packages/install_library.sh packages/dist/window.tar.gz
```

Windows PowerShell:

```powershell
./packages/install_library.ps1 -Package packages/dist/window.tar.gz
```

### Uninstall

Linux/macOS:

```bash
./packages/uninstall_library.sh window
```

Windows PowerShell:

```powershell
./packages/uninstall_library.ps1 -LibraryName window
```

### Use in code

```ofs
attach {window}
attach {fmt}
```

### Notes

- The default user packages directory is searched automatically.
- `OFS_LIB_PATH` is only needed when you want extra custom search paths.

### Current packages in the repository

- `canvas` (native, unstable)
- `bedrock`
- `bedrock-packet`
- `core`
- `fmt`
- `io`
- `math`
- `memory-modes`
- `rift`
- `string`
- `terminal-colors`
- `test-lib`
- `webserver`
- `window`

---

## Português (Brasil)

### Fluxo de pacote

1. Busque com `uncover`
2. Instale com `infuse`
3. Use no código com `attach {nome}`
4. Atualize com `reinfuse`

### Uso pela CLI

```bash
uncover fmt
infuse fmt
infuse {fmt:stable}
reinfuse fmt
```

### Instalar pacote local

Linux/macOS:

```bash
./packages/install_library.sh packages/dist/window.tar.gz
```

Windows PowerShell:

```powershell
./packages/install_library.ps1 -Package packages/dist/window.tar.gz
```

### Desinstalar

Linux/macOS:

```bash
./packages/uninstall_library.sh window
```

Windows PowerShell:

```powershell
./packages/uninstall_library.ps1 -LibraryName window
```

### Usar no código

```ofs
attach {window}
attach {fmt}
```

### Observações

- O diretório padrão de pacotes do usuário já entra na busca automaticamente.
- `OFS_LIB_PATH` só é necessário se você quiser adicionar diretórios extras.

### Pacotes atuais no repositório

- `canvas` (nativo, instável)
- `bedrock`
- `bedrock-packet`
- `core`
- `fmt`
- `io`
- `math`
- `memory-modes`
- `rift`
- `string`
- `terminal-colors`
- `test-lib`
- `webserver`
- `window`
