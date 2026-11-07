# ODL e OES

ODL (**Obsidian Document Language**) e OES (**Obsidian Effect Scripts**) são linguagens web independentes do OFS. A saída para o navegador continua compatível com HTML e CSS, mas o código-fonte usa o vocabulário do OFS: `page`, `skin`, `flow`, `deck`, `stage`, `band`, `gem`, `paint`, `veil`, and `pulse`.

A toolchain do OFS as compila nativamente. Os navegadores recebem artefatos HTML, CSS e JavaScript gerados.

| Fonte | Responsabilidade | Artefato gerado |
|---|---|---|
| `.odl` | estrutura do documento, conteúdo, componentes, interop | HTML |
| `.oes` | tokens, layout, regras responsivas, movimento | CSS |
| `.oll` | componentes de UI desktop nativa, layouts, janelas | UI nativa (60 FPS) / prévia PPM |
| `.oal` | app Android: manifesto, telas de toque, estilos, ações, ciclo de vida ([OAL_REFERENCE.md](OAL_REFERENCE.md)) | `AndroidManifest.xml` + OFS → APK |
| `.ofs` | lógica nativa, geradores, servidores, ferramentas de build | executável |

## ODL

```odl
page "OFS Web" "pt"
skin "theme.css"
flow
  deck.topbar
    title1 "OFS Web"
    link.button "Documentação" "#docs"
  stage#content
    mark "## Conteúdo nativo\nODL suporta **Markdown**."
    raw "<custom-element></custom-element>"
  spark "app.js"
```

O ODL aceita `tag.class#id`, Markdown nativo, HTML bruto, scripts de navegador, módulos no estilo ES/Node, JavaScript inline e blocos explícitos compatíveis com PHP. A interoperabilidade é opcional, então a saída gerada continua fácil de entender.

### Posicionamento visual seguro

O ODL inclui um gancho de estúdio apenas local:

```odl
move.card.feature
  h2 "Arraste-me localmente"
  p "Abra a página com ?ofs-studio=1."
studio "safe-localhost"
```

`studio` só é ativado em `localhost` ou `127.0.0.1` e nunca roda em host público por padrão. Os blocos `move.*` marcados podem ser arrastados visualmente, e o estúdio exporta regras de transformação OES que podem ser coladas de volta no fonte `.oes`.

## OES

```oes
gem accent "#a978ff"
gem surface "#120d1f"

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

O OES oferece tokens, blocos de seletor, apelidos de propriedades, grupos responsivos, consultas de recurso, camadas e linhas do tempo de animação.

## CLI

```bash
ODLC_INPUT=page.odl ODLC_OUTPUT=index.html ofs run ofs/tools/odlc.ofs
OESC_INPUT=theme.oes OESC_OUTPUT=theme.css ofs run ofs/tools/oesc.ofs
```

A invocação direta também funciona:

```bash
ofs page.odl
ofs theme.oes
```

A importação de HTML para ODL e de CSS para OES preserva a sintaxe sem suporte em blocos brutos explícitos. Isso torna a migração sem perdas enquanto o tradutor reverso estruturado evolui.

## Hospedagem

Os artefatos gerados podem ser hospedados no GitHub Pages, em qualquer host estático, no Node, no PHP ou no servidor web nativo do OFS. A extensão do VS Code reconhece `.odl` como documento de site e o comando **Go Live** compila os `.oes` vizinhos antes de iniciar uma prévia local com recarga automática.

Ao publicar no GitHub Pages, mantenha o `index.html` e o `site.css` gerados na raiz pública. Se quiser disponibilizar também o código original, copie os fontes para uma pasta não executada, como `source/page.odl` e `source/site.oes`. Eles são servidos como arquivos-fonte e não interferem no artefato do navegador.

Site de exemplo:

```bash
ofs/dist/ofs build ofs/examples/native_site/build.ofs -o /tmp/ofs-native-site-build
/tmp/ofs-native-site-build
python3 -m http.server 4180 --directory ofs/examples/native_site/dist
```
