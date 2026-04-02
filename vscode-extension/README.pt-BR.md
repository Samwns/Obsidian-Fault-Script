# Extensao VS Code do Obsidian Fault Script

Suporte de linguagem para OFS com:

- Highlight de sintaxe
- Autocomplete de palavras-chave principais
- Autocomplete para funcoes/monoliths de bibliotecas anexadas (`attach`) resolvidas via `OFS_LIB_PATH`
- Hover docs para comandos/palavras-chave OFS com exemplos inline
- Snippets
- Diagnosticos com `ofs check`
- Comandos para executar e validar o arquivo atual
- Tema roxo: Obsidian Fault Purple

## Configuracoes

- `ofs.path`: caminho para o executavel OFS (padrao: `ofs`)
- `ofs.diagnosticsOnType`: executar diagnosticos enquanto digita (padrao: true)

## Comandos

- `OFS: Run Current File`
- `OFS: Check Current File`

Quando um arquivo `.ofs` esta ativo, a barra de titulo do editor mostra os botoes rapidos de Run e Check.

## Instalacao (.vsix)

Use um dos metodos:

- `code --install-extension ofs-vscode-extension.vsix`
- VS Code -> Extensions -> `...` -> `Install from VSIX...`

Nao use o `VSIXInstaller.exe` do Visual Studio para este pacote. Ele so suporta extensoes do Visual Studio IDE, enquanto este pacote e para o Microsoft Visual Studio Code.

## Build no GitHub Actions

O workflow de release empacota a extensao com:

- `npm ci`
- `node ./node_modules/@vscode/vsce/vsce package --out ../ofs-vscode-extension.vsix`

## Notas

A extensao chama a CLI do OFS para diagnosticos e execucao. Entao, garanta que o OFS esta instalado e disponivel no PATH.
