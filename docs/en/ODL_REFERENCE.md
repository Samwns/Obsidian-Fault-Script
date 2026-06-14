# ODL Reference

ODL, Obsidian Document Language, is the structural document language of OFS.

You write `.odl`, `ofs odl` compiles it to HTML, and the browser receives a standard artifact. ODL remains the source of truth; HTML is the hosting output.

## Minimal Example

```odl
page "My site" "en"
skin "site.css"
flow
  deck.topbar
    title1 "My site"
    link.button "Docs" "#docs"
  stage#content
    band.docs
      mark "## Content\nText in **Markdown**."
      raw "<small>Raw HTML when needed.</small>"
  spark "app.js"
```

## Main Forms

| Form | Purpose |
|---|---|
| `page "title" "lang"` | Document metadata |
| `skin "file.css"` | Stylesheet link |
| `flow` | Document body |
| `deck`, `stage`, `band`, `tile` | Semantic layout |
| `title1`, `title2`, `title3`, `text` | Content nodes |
| `link`, `asset` | Links and images |
| `mark` | Markdown |
| `raw` | Raw HTML |
| `wire`, `spark`, `pulse` | JavaScript integration |
| `server` | PHP-compatible template block |

Compile with:

```bash
ofs odl page.odl -o index.html
```
