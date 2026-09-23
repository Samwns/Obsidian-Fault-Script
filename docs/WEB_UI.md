# Arquitetura da Stack Web OFS

A stack web do OFS é composta por três camadas bem delimitadas:

| Camada | Extensão | Finalidade |
|---|---|---|
| ODL | `.odl` | Estrutura semântica e conteúdo textual do documento |
| OES | `.oes` | Tokens visuais, layout responsivo e animações |
| OFS | `.ofs` | Lógica nativa de processamento, geração estática e servidor HTTP |

ODL e OES são linguagens compiladas pelo próprio toolchain do OFS, e não meros atalhos sintáticos para HTML e CSS manuais. No entanto, HTML5 e CSS padrão continuam sendo os formatos de saída distribuídos para navegadores.

---

## 1. Geração de Sites Estáticos

O toolchain compila os arquivos de código-fonte ODL e OES diretamente para artefatos consumíveis por qualquer servidor ou CDN:

```bash
ofs odl page.odl -o public/index.html
ofs oes theme.oes -o public/theme.css
```

A saída gerada pode ser hospedada em plataformas estáticas convencionais, como GitHub Pages, Cloudflare Pages ou servidores Nginx/Apache.

---

## 2. Servidor Nativo Integrado (`webserver`)

A biblioteca padrão fornece um servidor HTTP de alta performance escrito e executado nativamente em OFS:

```ofs
attach {webserver}

core main() {
    forge corpo = "<h1>Servidor Nativo OFS</h1><p>Execução sem dependências externas de runtime.</p>"
    webserver.serve_html_forever(8080, corpo)
}
```

O programa compilado em OFS gerencia os sockets de rede e entrega os artefatos compilados sob demanda.

---

## 3. Interoperabilidade com o Ecossistema Web

O compilador ODL possui diretivas específicas para integração direta com código web existente:

- `mark` / `markdown`: Processamento de conteúdo formatado em Markdown nativo.
- `raw` / `html`: Inclusão direta de fragmentos de HTML bruto sem filtros de escape.
- `wire` / `script`: Inclusão de scripts JavaScript convencionais (`<script src="...">`).
- `spark` / `module` / `node`: Inclusão de módulos JavaScript ES (`<script type="module" src="...">`).
- `pulse` / `js`: Execução de blocos inline de JavaScript (`<script>...</script>`).
- `server` / `php`: Emissão de blocos de processamento server-side (`<?php ... ?>`).
- `raw` / `css` no OES: Injeção direta de blocos CSS para migração gradual.

---

## 4. Extensão do VS Code e Recarregamento em Tempo Real (*Go Live*)

A extensão oficial do Visual Studio Code ativa o modo *Go Live* ao detectar arquivos `.odl` ou códigos `.ofs` que importam `webui` ou `webserver`.

Ao iniciar o *Go Live*:
1. O documento `.odl` é compilado para HTML.
2. Arquivos de estilo `.oes` no mesmo diretório são compilados para CSS.
3. Um servidor HTTP de desenvolvimento local é inicializado.
4. O servidor monitora alterações em arquivos `.odl`, `.oes`, scripts e ativos gráficos, disparando a recompilação e o recarregamento automático do navegador via SSE (*Server-Sent Events*).

---

## 5. Compatibilidade com Navegadores

Navegadores não interpretam arquivos `.odl` e `.oes` diretamente em tempo de execução. O ecossistema OFS adota o modelo consolidado na indústria (semelhante ao TypeScript, JSX e Sass):

1. O código-fonte do projeto é mantido exclusivamente em `.odl` e `.oes`.
2. As ferramentas de build compilam os arquivos em HTML5, CSS e JS compatíveis.
3. Servidores e navegadores consomem os artefatos finais gerados.
4. Para fins de depuração e inspeção, os servidores nativos expõem os tipos MIME `text/odl` e `text/oes`.

---

## 6. Escopo Técnico Atual

- As linguagens ODL e OES compilam código-fonte estruturado para páginas estáticas e folhas de estilo.
- A importação reversa de HTML/CSS armazena trechos complexos em blocos de texto bruto (`raw`).
- A lógica de roteamento HTTP em programas OFS permanece desacoplada da declaração estrutural dos documentos.
- Os componentes atuais são orientados à geração e emissão estática em tempo de compilação ou inicialização.
