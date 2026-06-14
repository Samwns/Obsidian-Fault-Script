# OFS Web Stack

The OFS web stack has three explicit source layers.

| Layer | File | Purpose |
|---|---|---|
| ODL | `.odl` | Browser document structure and content |
| OES | `.oes` | Design tokens, layout, responsive rules, and motion |
| OFS | `.ofs` | Native programs, components, generation, and HTTP serving |

ODL and OES are languages compiled by OFS, not aliases for handwritten HTML and CSS. HTML/CSS remain browser deployment formats.

## Static Site

```bash
ofs odl page.odl -o public/index.html
ofs oes theme.oes -o public/theme.css
```

The output works on GitHub Pages and conventional static hosting.

## Native OFS Server

```ofs
attach {webserver}

core main() {
    webserver.serve_html_forever(8080, "<h1>OFS server</h1>")
}
```

ODL can be used for authored pages, while an OFS build program can generate routes or serve the generated artifact.

## Interoperability

ODL includes explicit forms for existing web ecosystems.

- `mark` / `markdown`: native Markdown content
- `raw` / `html`: explicit raw HTML
- `wire` / `script`: browser JavaScript
- `spark` / `module` / `node`: ES modules
- `pulse` / `js`: inline JavaScript
- `server` / `php`: PHP-compatible template blocks
- `raw` / `css` in OES: gradual CSS migration

## VS Code Go Live

Go Live recognizes a file as a site only when it is an `.odl` document or an `.ofs` program using `webui`/`webserver`.

For `.odl`, the extension compiles the document, compiles neighboring `.oes` files, copies browser assets, and starts a local HTTP server.

The live server watches the document, neighboring OES files, JavaScript, CSS, and browser assets. When a source file changes, it rebuilds and reloads the browser automatically.

## Browser Acceptance

Browsers do not execute ODL/OES directly yet. OFS follows the practical path used by TypeScript, JSX, Sass, and similar languages:

- source files stay as `.odl` and `.oes`;
- tools register syntax, MIME, compile commands, and preview behavior;
- browsers receive generated HTML/CSS/JS;
- static hosts and GitHub Pages deploy the generated artifacts.

The OFS native webserver and VS Code live server expose `text/odl` and `text/oes` MIME types for source inspection.

## Current Boundaries

- ODL/OES currently compile indentation-based source to static browser artifacts.
- Reverse HTML/CSS imports preserve complex constructs in raw blocks.
- OFS native routing and request handlers remain separate from the document/effect languages.
- React-style stateful rendering is planned; current components are static/generator-oriented.
