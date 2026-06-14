# ODL Reference

ODL is the structural web language of OFS.

```odl
document "Title" "en"
style "theme.css"
body
  main#app.shell
    h1 "Hello"
```

Indentation opens and closes elements. A tag may include `.class` and `#id` shorthand.

| Form | Result |
|---|---|
| `document "title" "lang"` | Complete browser document metadata |
| `tag.class#id "text"` | Element with escaped text |
| `a "label" "href"` | Link |
| `image "src" "alt"` | Image |
| `markdown "source"` | Native Markdown fragment |
| `html "source"` | Raw HTML fragment |
| `style "href"` | Stylesheet link |
| `script "src"` | Classic browser script |
| `module "src"` / `node "src"` | ES module script |
| `js "source"` | Inline JavaScript |
| `php "source"` | PHP template block |
| `move.class` | Locally draggable studio block |
| `studio "safe-localhost"` | Local-only visual positioning overlay |
| `component name` | Reusable component template |
| `use name` | Component use marker |

Compile with `ofs odl page.odl -o index.html`.
