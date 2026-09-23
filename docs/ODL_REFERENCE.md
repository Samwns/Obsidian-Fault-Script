# Referência ODL

ODL (Obsidian Document Language) é a linguagem estrutural de documentos do ecossistema OFS.

O desenvolvedor escreve arquivos `.odl`, o compilador `ofs odl` traduz o documento para HTML5 padrão, e o navegador consome o artefato resultante. O código-fonte permanece sendo ODL; o HTML é o formato de distribuição web.

---

## Exemplo Mínimo

```odl
page "Meu Site" "pt"
skin "site.css"

flow
  deck.topbar
    title1 "Meu Site"
    link.button "Documentação" "#docs"
  stage#conteudo
    band.docs
      mark "## Conteúdo\nTexto formatado em **Markdown**."
      raw "<small>HTML bruto para interoperabilidade quando necessário.</small>"
  spark "app.js"
```

---

## Forma Tipada e Escopo Explícito

A sintaxe clássica baseada em indentação é suportada nativamente. A forma com blocos explícitos, chamadas de função e assinaturas tipadas permite estruturar componentes com contratos estritos:

```odl
page "OFS Web" "pt"
  skin("theme.oes")

  flow {
    deck.topbar {
      title1("OFS Web")
      link.button("Docs", "#docs")
    }

    stage#conteudo {
      band.docs {
        use Aviso(titulo: "HMR Ativo", status: "success")
      }
    }
  }
end

component Aviso(titulo: String, status: String)
  tile.aviso(data-status: status) {
    title2(titulo)
    text("Componente injetado com tipagem forte e escopo fechado.")
  }
end
```

---

## Regras Sintáticas

- A indentação delimita abertura e fechamento de elementos na sintaxe padrão.
- Blocos delimitados por chaves `{}` e fechamento `end` podem ser utilizados para escopo explícito.
- Argumentos tipados utilizam o padrão `nome: Tipo` nas assinaturas de componentes.
- O nome de um elemento pode conter classes com a notação `.classe`.
- O nome de um elemento pode conter identificador com `#id`.
- O texto literal entre aspas é escapado contra injeções por padrão.
- A diretiva `raw` permite emissão direta de HTML bruto.
- A diretiva `mark` aceita conteúdo nativo em sintaxe Markdown.

---

## Elementos Principais

| Diretiva / Elemento | Tag HTML Equivalente | Finalidade |
|---|---|---|
| `page "título" "idioma"` | `<title>` / `<html>` | Metadados do cabeçalho e idioma do documento |
| `skin "arquivo.css"` | `<link rel="stylesheet">` | Vínculo de folha de estilos CSS |
| `flow` | `<body>` | Ponto de partida do corpo do documento |
| `deck` | `<nav>` | Barra de navegação semântica |
| `stage` | `<main>` | Região de conteúdo principal da página |
| `band` | `<section>` | Seção estruturada de conteúdo |
| `tile` | `<article>` | Artigo, cartão ou bloco independente |
| `title1`, `title2`, `title3` | `<h1>`, `<h2>`, `<h3>` | Títulos hierárquicos |
| `text` | `<p>` | Parágrafos de texto |
| `link "rótulo" "url"` | `<a href="url">` | Links e âncoras |
| `asset "caminho" "alt"` | `<img src="caminho">` | Imagens e recursos gráficos |
| `mark "texto"` | `<div>` (renderizado) | Processamento de sintaxe Markdown |
| `raw "código"` | Nenhum invólucro | Emissão de HTML bruto sem escape |
| `wire "script.js"` | `<script src="...">` | Inclusão de script clássico |
| `spark "modulo.js"` | `<script type="module">` | Inclusão de módulo JavaScript ES |
| `pulse "código"` | `<script>` | Código JavaScript inline |
| `server "código"` | `<?php ... ?>` | Blocos de modelo de servidor (PHP/template) |

---

## Compatibilidade

Para projetos legados, as palavras-chave `document`, `style`, `body`, `markdown`, `html`, `script`, `module`, `js` e `php` continuam sendo aceitas pelo analisador. Projetos novos devem utilizar as diretivas ODL padronizadas.

---

## Compilação

Para compilar um arquivo ODL para HTML5:

```bash
ofs odl documento.odl -o public/index.html
```
