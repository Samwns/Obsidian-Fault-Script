# Stack Web OFS

A stack web da OFS tem tres camadas de fonte explicitas.

| Camada | Arquivo | Finalidade |
|---|---|---|
| ODL | `.odl` | Estrutura e conteudo do documento |
| OES | `.oes` | Tokens, layout, responsividade e movimento |
| OFS | `.ofs` | Programas nativos, componentes, geracao e servidor HTTP |

ODL e OES sao linguagens compiladas pela OFS, nao apelidos para HTML e CSS escritos a mao. HTML/CSS continuam sendo formatos de publicacao para navegadores.

## Site estatico

```bash
ofs odl page.odl -o public/index.html
ofs oes theme.oes -o public/theme.css
```

A saida funciona em GitHub Pages e hospedagem estatica comum.

## Servidor OFS nativo

```ofs
attach {webserver}

core main() {
    webserver.serve_html_forever(8080, "<h1>OFS server</h1>")
}
```

ODL pode ser usado para autorar paginas, enquanto um programa OFS pode gerar rotas ou servir o artefato gerado.

## Interoperabilidade

ODL inclui formas explicitas para interoperar com ecossistemas web existentes.

- `mark` / `markdown`: Markdown nativo
- `raw` / `html`: HTML bruto explicito
- `wire` / `script`: JavaScript de navegador
- `spark` / `module` / `node`: modulos ES
- `pulse` / `js`: JavaScript inline
- `server` / `php`: blocos compativeis com templates PHP
- `raw` / `css` em OES: migracao gradual de CSS

## VS Code Go Live

O Go Live reconhece um arquivo como site somente quando ele e um documento `.odl` ou um programa `.ofs` que usa `webui`/`webserver`.

Para `.odl`, a extensao compila o documento, compila arquivos `.oes` vizinhos, copia assets de navegador e inicia um servidor HTTP local.

O servidor local observa o documento, arquivos OES vizinhos, JavaScript, CSS e assets. Quando uma fonte muda, ele recompila e recarrega o navegador automaticamente.

## Aceitacao pelo navegador

Navegadores ainda nao executam ODL/OES diretamente. A OFS segue o caminho pratico usado por TypeScript, JSX, Sass e linguagens parecidas:

- os fontes ficam como `.odl` e `.oes`;
- ferramentas registram sintaxe, MIME, comandos de compilacao e preview;
- navegadores recebem HTML/CSS/JS gerado;
- hospedagens estaticas e GitHub Pages publicam os artefatos gerados.

O webserver nativo OFS e o servidor Go Live da extensao expoem `text/odl` e `text/oes` para inspecao de fonte.

## Limites atuais

- ODL/OES hoje compilam fonte baseada em indentacao para artefatos estaticos.
- Importacao reversa de HTML/CSS preserva estruturas complexas em blocos raw.
- Roteamento nativo e handlers HTTP continuam separados das linguagens de documento/efeito.
- Renderizacao stateful estilo React ainda e planejada; os componentes atuais sao estaticos ou orientados a geracao.
