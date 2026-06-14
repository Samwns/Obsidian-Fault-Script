# ODL and OES

ODL (**Obsidian Document Language**) and OES (**Obsidian Effect Scripts**) are standalone OFS web languages. They keep the browser output compatible with HTML and CSS, but the authored source uses OFS vocabulary: `page`, `skin`, `flow`, `deck`, `stage`, `band`, `gem`, `paint`, `veil`, and `pulse`.

The OFS toolchain compiles them natively. Browsers receive generated HTML, CSS, and JavaScript artifacts.

| Source | Responsibility | Generated artifact |
|---|---|---|
| `.odl` | document structure, content, components, interop | HTML |
| `.oes` | tokens, layout, responsive rules, motion | CSS |
| `.ofs` | native logic, generators, servers, build tools | executable |

## ODL

```odl
page "OFS Web" "pt"
skin "theme.css"
flow
  deck.topbar
    title1 "OFS Web"
    link.button "Documentacao" "#docs"
  stage#content
    mark "## Conteudo nativo\nODL suporta **Markdown**."
    raw "<custom-element></custom-element>"
  spark "app.js"
```

ODL supports `tag.class#id`, native Markdown, raw HTML, browser scripts, ES/Node-style modules, inline JavaScript, and explicit PHP-compatible blocks. Interoperability is opt-in, so generated output stays understandable.

### Safe Visual Positioning

ODL includes a local-only studio hook:

```odl
move.card.feature
  h2 "Drag me locally"
  p "Open the page with ?ofs-studio=1."
studio "safe-localhost"
```

`studio` only activates on `localhost` or `127.0.0.1`. It never runs on a public host by default. Marked `move.*` blocks can be dragged visually, and the studio exports OES transform rules that can be pasted back into `.oes` source.

## OES

```oes
gem accent "#29e0bd"
gem surface "#101722"

paint .button
  fill "$accent"
  curve "8px"
  transition "transform .2s ease"

veil "(max-width: 700px)"
  paint .grid
    grid-cols "1fr"

pulse "enter"
  rise
    opacity "0"
  rest
    opacity "1"
```

OES provides tokens, selector blocks, property aliases, responsive groups, feature queries, layers, and animation timelines.

## CLI

```bash
ofs odl page.odl -o index.html
ofs oes theme.oes -o theme.css
ofs translate legacy.html --to odl
ofs translate legacy.css --to oes
```

Direct invocation also works:

```bash
ofs page.odl
ofs theme.oes
```

HTML-to-ODL and CSS-to-OES import preserves unsupported syntax in explicit raw blocks. This makes migration lossless while the structured reverse translator evolves.

## Hosting

Generated artifacts can be hosted by GitHub Pages, any static host, Node, PHP, or the OFS native web server. The VS Code extension recognizes `.odl` as a site document and its **Go Live** command compiles neighboring `.oes` files before starting a local preview with automatic reload.

When publishing to GitHub Pages, keep the generated `index.html` and `site.css` at the public root. If you want the authored language available too, copy the sources into a non-executed folder such as `source/page.odl` and `source/site.oes`. They are served as source files and do not interfere with the browser artifact.

Example site:

```bash
ofs/dist/ofs build ofs/examples/native_site/build.ofs -o /tmp/ofs-native-site-build
/tmp/ofs-native-site-build
python3 -m http.server 4180 --directory ofs/examples/native_site/dist
```
