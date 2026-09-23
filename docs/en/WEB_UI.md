# OFS Web Stack Architecture

The OFS web stack is structured across three distinct layers:

| Layer | Extension | Purpose |
|---|---|---|
| ODL | `.odl` | Semantic document structure and textual content |
| OES | `.oes` | Design tokens, responsive layout rules, and keyframe animations |
| OFS | `.ofs` | Native processing logic, static page generation, and HTTP serving |

ODL and OES are standalone languages compiled by the OFS toolchain, rather than mere aliases for handwritten HTML and CSS. However, standard HTML5 and CSS remain the browser distribution targets.

---

## 1. Static Site Generation

The toolchain compiles ODL and OES source files directly into production-ready browser assets:

```bash
ofs odl page.odl -o public/index.html
ofs oes theme.oes -o public/theme.css
```

The output can be deployed to standard static hosting providers, such as GitHub Pages, Cloudflare Pages, or traditional web servers (Nginx/Apache).

---

## 2. Integrated Native Web Server (`webserver`)

The standard library provides an HTTP server written in OFS and executed as a standalone binary:

```ofs
attach {webserver}

core main() {
    forge body = "<h1>OFS Native Server</h1><p>Running without external runtime dependencies.</p>"
    webserver.serve_html_forever(8080, body)
}
```

The compiled binary manages network sockets and serves the pre-rendered artifacts on demand.

---

## 3. Web Ecosystem Interoperability

The ODL compiler provides explicit directives to interoperate with existing web technologies:

- `mark` / `markdown`: Compiles embedded Markdown into semantic HTML.
- `raw` / `html`: Injects raw HTML fragments without automatic escaping.
- `wire` / `script`: Injects classic JavaScript scripts (`<script src="...">`).
- `spark` / `module` / `node`: Injects modern ES modules (`<script type="module" src="...">`).
- `pulse` / `js`: Embeds inline JavaScript code (`<script>...</script>`).
- `server` / `php`: Emits server-side processing blocks (`<?php ... ?>`).
- `raw` / `css` in OES: Injects direct CSS rules for gradual migrations.

---

## 4. VS Code Extension and Live Reload (*Go Live*)

The official VS Code extension enables *Go Live* development mode when opening `.odl` documents or `.ofs` programs that import `webui` or `webserver`.

When *Go Live* is started:
1. The `.odl` document is compiled to HTML.
2. Sibling `.oes` style files in the directory are compiled to CSS.
3. A local development HTTP server is initiated.
4. The server watches `.odl`, `.oes`, scripts, and graphical assets, automatically triggering compilation and browser reload via Server-Sent Events (SSE).

---

## 5. Browser Compatibility

Web browsers do not execute `.odl` and `.oes` files natively. OFS adopts the industry-standard workflow established by TypeScript, JSX, and Sass:

1. Project source code is authored in `.odl` and `.oes`.
2. Build tools compile the source files into standard HTML5, CSS, and JS.
3. Hosting providers and browsers receive the compiled artifacts.
4. For debugging and inspection, the native server exposes `text/odl` and `text/oes` MIME types.

---

## 6. Current Technical Scope

- ODL and OES compile whitespace-delimited source code into static markup and stylesheets.
- Reverse HTML/CSS import tools encapsulate complex legacy blocks into raw directives (`raw`).
- Native HTTP routing logic remains separate from document layout definitions.
- Components are designed for compile-time or bootstrap-time static evaluation.
