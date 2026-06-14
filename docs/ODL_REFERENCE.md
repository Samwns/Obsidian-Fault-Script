# ODL Reference

ODL is the structural web language of OFS.

```odl
page "Title" "pt"
skin "theme.css"
flow
  stage#app.shell
    title1 "Hello"
    mark "Markdown **nativo**."
```

Indentation opens and closes elements. A tag may include `.class` and `#id` shorthand.

| Form | Result |
|---|---|
| `page "title" "lang"` | Complete browser document metadata |
| `skin "href"` | Stylesheet link |
| `flow` | Starts the document body |
| `deck`, `stage`, `band`, `tile` | Semantic structure generated as nav, main, section, article |
| `title1`, `title2`, `text`, `link`, `asset` | Common content nodes |
| `tag.class#id "text"` | Element with escaped text |
| `mark "source"` | Native Markdown fragment |
| `raw "source"` | Explicit raw HTML fragment |
| `wire "src"` | Classic browser script |
| `spark "src"` | ES/Node-style module script |
| `pulse "source"` | Inline JavaScript |
| `server "source"` | PHP-compatible template block |
| `move.class` | Locally draggable studio block |
| `studio "safe-localhost"` | Local-only visual positioning overlay |
| `vein name` | Reusable component template |
| `mount name` | Component use marker |

The older compatibility forms `document`, `style`, `body`, `markdown`, `html`, `script`, `module`, `js`, and `php` remain accepted so imported HTML-era code can migrate gradually. New OFS web sources should prefer the ODL vocabulary above.

Compile with `ofs odl page.odl -o index.html`.
