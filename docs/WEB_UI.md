# OFS Web Stack / Stack Web OFS

The OFS web stack has three explicit source layers.

A stack web da OFS tem tres camadas de fonte explicitas.

| Layer / Camada | File / Arquivo | Purpose / Finalidade |
|---|---|---|
| ODL | `.odl` | Browser document structure and content / estrutura e conteudo do documento |
| OES | `.oes` | Design tokens, layout, responsive rules, and motion / tokens, layout, responsividade e movimento |
| OFS | `.ofs` | Native programs, components, generation, and HTTP serving / programas nativos, componentes, geracao e servidor HTTP |

ODL and OES are languages compiled by OFS, not aliases for handwritten HTML and CSS. HTML/CSS remain browser deployment formats.

ODL e OES sao linguagens compiladas pela OFS, nao apelidos para HTML e CSS escritos a mao. HTML/CSS continuam sendo formatos de publicacao para navegadores.

## Static Site / Site estatico

```bash
ofs odl page.odl -o public/index.html
ofs oes theme.oes -o public/theme.css
```

The output works on GitHub Pages and conventional static hosting.

A saida funciona em GitHub Pages e hospedagem estatica comum.

## Native OFS Server / Servidor OFS nativo

```ofs
attach {webserver}

core main() {
    webserver.serve_html_forever(8080, "<h1>OFS server</h1>")
}
```

ODL can be used for authored pages, while an OFS build program can generate routes or serve the generated artifact.

ODL pode ser usado para autorar paginas, enquanto um programa OFS pode gerar rotas ou servir o artefato gerado.

## Interoperability / Interoperabilidade

ODL includes explicit forms for existing web ecosystems.

ODL inclui formas explicitas para interoperar com ecossistemas web existentes.

- `mark` / `markdown`: native Markdown content / Markdown nativo
- `raw` / `html`: raw HTML / HTML bruto explicito
- `wire` / `script`: browser JavaScript / JavaScript de navegador
- `spark` / `module` / `node`: ES module entry points / modulos ES
- `pulse` / `js`: inline JavaScript / JavaScript inline
- `server` / `php`: PHP-compatible template blocks / blocos compativeis com templates PHP
- `raw` / `css` in OES: gradual CSS migration / migracao gradual de CSS

## VS Code Go Live

Go Live recognizes a file as a site only when it is an `.odl` document or an `.ofs` program using `webui`/`webserver`.

O Go Live reconhece um arquivo como site somente quando ele e um documento `.odl` ou um programa `.ofs` que usa `webui`/`webserver`.

For `.odl`, the extension compiles the document, compiles neighboring `.oes` files, copies browser assets, and starts a local HTTP server.

Para `.odl`, a extensao compila o documento, compila arquivos `.oes` vizinhos, copia assets de navegador e inicia um servidor HTTP local.

The live server watches the document, neighboring OES files, JavaScript, CSS, and browser assets. When a source file changes, it rebuilds and reloads the browser automatically.

O servidor local observa o documento, arquivos OES vizinhos, JavaScript, CSS e assets. Quando uma fonte muda, ele recompila e recarrega o navegador automaticamente.

## Browser Acceptance / Aceitacao pelo navegador

Browsers do not execute ODL/OES directly yet. OFS follows the practical path used by TypeScript, JSX, Sass, and similar languages:

Navegadores ainda nao executam ODL/OES diretamente. A OFS segue o caminho pratico usado por TypeScript, JSX, Sass e linguagens parecidas:

- source files stay as `.odl` and `.oes`;
- os fontes ficam como `.odl` e `.oes`;
- tools register syntax, MIME, compile commands, and preview behavior;
- ferramentas registram sintaxe, MIME, comandos de compilacao e preview;
- browsers receive generated HTML/CSS/JS;
- navegadores recebem HTML/CSS/JS gerado;
- static hosts and GitHub Pages deploy the generated artifacts.
- hospedagens estaticas e GitHub Pages publicam os artefatos gerados.

The OFS native webserver and VS Code live server expose `text/odl` and `text/oes` MIME types for source inspection.

O webserver nativo OFS e o servidor Go Live da extensao expoem `text/odl` e `text/oes` para inspecao de fonte.

## Current Boundaries / Limites atuais

- ODL/OES currently compile indentation-based source to static browser artifacts.
- ODL/OES hoje compilam fonte baseada em indentacao para artefatos estaticos.
- Reverse HTML/CSS imports preserve complex constructs in raw blocks.
- Importacao reversa de HTML/CSS preserva estruturas complexas em blocos raw.
- OFS native routing and request handlers remain separate from the document/effect languages.
- Roteamento nativo e handlers HTTP continuam separados das linguagens de documento/efeito.
- React-style stateful rendering is planned; current components are static/generator-oriented.
- Renderizacao stateful estilo React ainda e planejada; os componentes atuais sao estaticos ou orientados a geracao.
