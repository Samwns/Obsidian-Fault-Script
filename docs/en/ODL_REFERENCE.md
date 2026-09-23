# ODL Reference

ODL (Obsidian Document Language) is the structural document language of the OFS ecosystem.

You write `.odl` source files, `ofs odl` compiles them into standard HTML5, and the browser consumes the resulting artifact. ODL remains the project's source of truth; HTML is only the web distribution target.

---

## Minimal Example

```odl
page "My Site" "en"
skin "site.css"

flow
  deck.topbar
    title1 "My Site"
    link.button "Documentation" "#docs"
  stage#content
    band.docs
      mark "## Content\nFormatted with **Markdown**."
      raw "<small>Raw HTML when explicit markup is needed.</small>"
  spark "app.js"
```

---

## Typed Syntax and Explicit Scope

The standard indentation-based syntax is fully supported. An explicit block syntax with braces, method calls, and typed signatures is also available for strongly contracts:

```odl
page "OFS Web" "en"
  skin("theme.oes")

  flow {
    deck.topbar {
      title1("OFS Web")
      link.button("Docs", "#docs")
    }

    stage#content {
      band.docs {
        use Notice(title: "HMR Active", status: "success")
      }
    }
  }
end

component Notice(title: String, status: String)
  tile.notice(data-status: status) {
    title2(title)
    text("Injected component with strong typing and closed scope.")
  }
end
```

---

## Syntax Rules

- Indentation opens and closes semantic elements in standard syntax.
- Braces `{}` and `end` blocks can be used for explicit lexical scoping.
- Typed arguments use `name: Type` in component signatures.
- Element identifiers may attach CSS classes using `.classname`.
- Element identifiers may attach an HTML ID using `#id`.
- Quoted string literals are escaped against HTML injection by default.
- The `raw` directive preserves raw, unescaped HTML strings.
- The `mark` directive compiles Markdown text directly to semantic HTML.

---

## Core Directives and Elements

| Directive / Element | Equivalent HTML | Purpose |
|---|---|---|
| `page "title" "lang"` | `<title>` / `<html>` | Document metadata, head title, and document language |
| `skin "file.css"` | `<link rel="stylesheet">` | External stylesheet inclusion |
| `flow` | `<body>` | Root document flow container |
| `deck` | `<nav>` | Semantic navigation bar |
| `stage` | `<main>` | Main page content container |
| `band` | `<section>` | Thematic grouping section |
| `tile` | `<article>` | Independent article or card |
| `title1`, `title2`, `title3` | `<h1>`, `<h2>`, `<h3>` | Structural heading hierarchy |
| `text "..."` | `<p>` | Standard paragraph text |
| `link "label" "url"` | `<a href="url">` | Hyperlinks and navigation anchors |
| `asset "path" "alt"` | `<img src="path">` | Images and graphical assets |
| `mark "markdown"` | `<div>` (rendered) | Markdown parsing and inline formatting |
| `raw "code"` | Unwrapped | Direct raw HTML emission |
| `wire "script.js"` | `<script src="...">` | Standard JavaScript script inclusion |
| `spark "module.js"` | `<script type="module">` | ES module script inclusion |
| `pulse "code"` | `<script>` | Inline JavaScript execution |
| `server "code"` | `<?php ... ?>` | Server-side template directive |

---

## Backward Compatibility

Legacy keywords including `document`, `style`, `body`, `markdown`, `html`, `script`, `module`, `js`, and `php` are accepted by the parser for migration purposes. New projects should use canonical ODL keywords.

---

## Compilation

Compile an ODL document to HTML5:

```bash
ofs odl document.odl -o public/index.html
```
