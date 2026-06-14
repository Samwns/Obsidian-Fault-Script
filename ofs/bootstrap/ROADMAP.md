# OFS Roadmap

Este arquivo acompanha o que ainda falta para a OFS ficar consistente como linguagem, ferramenta, stack web e distribuicao publica.

## Estado atual

- [x] Compilador self-hosted em OFS com geracao LLVM IR.
- [x] Runtime nativo distribuido com `libofs_runtime.a`.
- [x] Launcher `ofs` com `check`, `run`, `build`, ODL, OES e tradutor web.
- [x] Biblioteca padrao em `ofs/stdlib`.
- [x] ODL: linguagem de documento compilada para HTML.
- [x] OES: linguagem de efeitos compilada para CSS.
- [x] Exemplo de site nativo em `ofs/examples/native_site`.
- [x] Extensao VS Code com suporte a `.ofs`, `.odl`, `.oes` e Go Live.
- [x] Instaladores Linux `.deb`, `.rpm`, Arch/pacman, macOS `.pkg`, Windows `.exe` e VSIX.
- [x] Auto Release com checksums.
- [x] Site/documentacao gerados pela propria OFS em `docs/`.
- [x] Benchmark local com CPU, memoria, startup, build e tamanho de artefato.

## Pendente curto prazo

- [ ] Separar completamente as paginas do site por idioma.
- [ ] Completar todas as paginas em portugues.
- [ ] Completar todas as paginas em ingles.
- [ ] Criar exemplos pequenos por pagina: linguagem, stdlib, ODL, OES, webserver, webui, pacotes e VS Code.
- [ ] Atualizar os Markdown tecnicos para baterem com o site gerado.
- [ ] Revisar os links do site para nao dependerem de conteudo gerado fora de `docs/`.
- [ ] Corrigir configuracao publica do GitHub Pages para usar GitHub Actions ou `main /docs`.
- [ ] Atualizar workflows para actions compatíveis com Node 24.

## Pendente da linguagem

- [ ] Mensagens de erro com linha/coluna mais precisas.
- [ ] Recuperacao de erro no parser.
- [ ] Sugestoes de typo em simbolos e tipos.
- [ ] Cobrir `impl`, `namespace` e `strata` com testes end-to-end.
- [ ] Finalizar semantica de `tremor/catch`.
- [ ] Definir escopo oficial de `fracture`, `abyss`, `fractal` e `bedrock`.
- [ ] Documentar FFI e `extern vein` com exemplos reais.
- [ ] Rodar suite completa de stdlib em Linux, macOS e Windows.

## Pendente de performance

- [ ] Reparar o segmentation fault do teste self-hosted `test_lexer_basic.ofs`.
- [ ] Adicionar cronometro por fase: attach, lexer, parser, type checker, LLVM e linker.
- [ ] Medir fontes sinteticas de 10 KB, 100 KB e 1 MB.
- [ ] Criar acesso interno de caractere com tamanho conhecido sem enfraquecer `ofs_str_char_at`.
- [ ] Avaliar tokens como slices para evitar substring por token.
- [ ] Criar string builder nativo para ODL, OES e geradores.
- [ ] Separar benchmark de linguagem de benchmark de runtime.
- [ ] Adicionar workloads de string, array, alocacao, IO e chamadas de funcao.
- [ ] Medir tempo de compilacao por fase do compilador.
- [ ] Avaliar inlining, constant folding e dead code elimination.
- [ ] Avaliar LTO no binario final.
- [ ] Publicar metodologia de benchmark com hardware, flags e versoes.

## Pendente web

- [ ] Definir especificacao ODL completa.
- [ ] Definir especificacao OES completa.
- [ ] Melhorar tradutor HTML -> ODL e CSS -> OES para casos reais.
- [ ] Criar modo studio visual com persistencia segura em OES.
- [ ] Documentar reload ao vivo da extensao.
- [ ] Criar template oficial de site OFS para download.
- [ ] Adicionar exemplos de Markdown, HTML raw, JS module e PHP template.
- [ ] Definir modelo de componentes reativos sem copiar React.

## Pendente GitHub

- [ ] Preparar pacote para GitHub Linguist com extensoes `.ofs`, `.odl`, `.oes`.
- [ ] Publicar exemplos suficientes em repositorios publicos.
- [ ] Preparar gramatica TextMate revisada para Linguist.
- [ ] Abrir PR no Linguist quando o projeto tiver volume publico suficiente.

## Bloqueios conhecidos

- A configuracao do GitHub Pages ainda pode estar em `main /`. Sem permissao de admin, a automacao nao consegue trocar para Actions ou `/docs`.
- Navegadores nao executam ODL/OES nativamente. A estrategia atual e igual a TypeScript/Sass: fonte propria, artefato HTML/CSS/JS para o browser.
- Resultados de benchmark local nao sao ranking universal de linguagens.
