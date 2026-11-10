# Ferramentas do OFS

## `ofs doc`: documentação gerada do código

`ofs doc` lê arquivos `.ofs` e gera uma referência HTML estática (como `rustdoc` ou `godoc`): uma página por módulo, com cartões de função, busca e um relatório de cobertura. A ferramenta é escrita em OFS (`ofs/tools/ofsdoc.ofs`) e funciona em qualquer projeto OFS.

```bash
ofs doc                          # ofs/stdlib, ofs/tools e ofs/ofscc do repositório -> docs_api/
ofs doc minha_pasta -o docs_api/ # qualquer pasta (recursivo) ou um único .ofs
ofs doc --check                  # só verifica: imprime a cobertura, não gera HTML
```

Opções: `-o <pasta>` (saída, padrão `docs_api`), `--check`, `--docs <pasta>` (os `.md` do projeto; padrão `docs/` se existir), `--title <nome>`, `--repo <url do GitHub>` (liga "Fonte" à linha no GitHub), `--lang pt|en|es` (idioma da interface; padrão `OFS_LANG`). Para ver o resultado: `cd docs_api && python3 -m http.server 8765`.

No site, o `build_site.ofs` roda o mesmo gerador a cada build e junta a API ao Guia numa só navegação em `docs/ref/` (Referência).

### O que aparece em cada módulo

- `namespace`, `monolith` (campos e tipos), `impl` (métodos), `vein`, `rift vein` (selo **exportada · abi c**), `extern vein` (selo **externa: libc/NDK**), `core` e constantes/globais.
- Assinatura completa: parâmetros, tipos e retorno. Nomes de `monolith` nos tipos viram links.
- **Chama** (funções conhecidas usadas no corpo) e **Usado por** (o inverso, em todo o código analisado).
- **Veja também**: seções dos `.md` de `docs/` que citam a função ou o módulo.
- **Fonte**: link para a linha no GitHub.
- Funções que começam com `_` são internas: ficam numa seção recolhida e não entram na cobertura.

### Como comentar o código

O comentário de uma declaração são as linhas `//` contíguas imediatamente acima dela (sem linha em branco no meio), ou um bloco `/** ... */`. O primeiro bloco do arquivo, separado por uma linha em branco, descreve o módulo.

```ofs
// hashmap: tabela hash inteiro -> inteiro (comentário do módulo).
//
//   forge m = hashmap.new(64)
//   hashmap.set(m, 42, 7)

namespace hashmap {
    // Cria uma tabela com capacidade inicial `cap`.
    //
    // O primeiro parágrafo é o resumo (aparece na busca e nos índices).
    // Linhas indentadas com 2+ espaços viram bloco de código;
    // `nome` vira link quando existe no código.
    // @param cap capacidade inicial (potência de 2 é melhor)
    // @returns o identificador da tabela
    // @example
    //   forge m = hashmap.new(64)
    vein new(cap: stone) -> stone {
        return _alloc(cap)
    }
}
```

| Marca | Efeito |
|---|---|
| `@param nome texto` | descrição na tabela de parâmetros |
| `@returns texto` | descrição do retorno |
| `@example` | o resto do comentário vira um bloco de código |
| `` `nome` `` | link para a função, tipo ou módulo, se existir |
| `// ──── título ────` / `// ----` | linha decorativa, ignorada |

Limitações: as chaves `{` de `namespace`, `impl`, `monolith` e do corpo de `vein` devem estar na mesma linha da declaração (estilo do stdlib).

### Cobertura ("o que falta documentar")

A página **Cobertura** (e a saída de `ofs doc --check`) mostra:

1. quantas funções públicas (`vein`/`rift vein` sem `_`) têm comentário, por módulo e no total, com o percentual e a lista do que falta;
2. as funções **citadas em `docs/*.md` que não existem no código**: `modulo.funcao(` quando `modulo` é um módulo ou namespace conhecido, e um nome com `_` citado entre crases com parêntese de abertura que não existe em lugar nenhum. Nomes `fault_*`, funções definidas como exemplo em blocos de código dos próprios `.md` e assinaturas de exemplo (`` `f(x) -> T` ``) não contam.

`ofs doc --check` sai com código **1** se houver função citada inexistente, para usar no CI:

```bash
ofs doc --check || exit 1
```

### Teste

`ofs/tests/ofsdoc/run.sh` compara o extrator com `ofs/tests/ofsdoc/expected.txt` (nomes, assinaturas, comentários, chama/usado por) e confere o código de saída do `--check`.

## `ofs fmt`: formatador

`ofs fmt` formata código `.ofs` (ferramenta escrita em OFS, `ofs/tools/ofsfmt.ofs`).

```bash
ofs fmt arquivo.ofs     # reescreve no lugar
ofs fmt src/            # pasta, recursivo (só .ofs); sem alvo = pasta atual
ofs fmt --check [alvos] # CI: lista o que mudaria e sai com código 1
ofs fmt --diff [alvos]  # mostra o diff, não escreve
ofs fmt --stdin         # lê da entrada padrão, escreve na saída (editores)
```

Regras: recuo de 4 espaços (linhas de continuação dentro de `(`/`[` ou após operador ganham +4), `{` na mesma linha, espaço em volta de operadores binários e depois de `,` e `:`, no máximo uma linha em branco seguida, sem espaço no fim da linha, um `\n` no fim do arquivo. As quebras de linha do autor são mantidas, e comentários, strings e linhas `attach` nunca são alteradas.

Garantias (testadas em `ofs/tests/fmt/run.sh`): formatar duas vezes dá o mesmo arquivo, e o IR do `ofscc` do original e do formatado é idêntico (o programa não muda). Pendente: quebra automática de linhas acima de 100 colunas e ordenação dos `attach`.

No VS Code, a extensão registra o `ofs fmt --stdin` como formatador; para formatar ao salvar: `"[ofs]": { "editor.formatOnSave": true }`.

## `ofs test`, `ofs twin`, `ofs polish`

```bash
ofs test [--run] [--twin] [pasta]   # padrão: ./tests
ofs twin arquivo.ofs
ofs polish [--check] [--diff] [alvos]
```

- `ofs test`: acha os `*.ofs` que têm um `.expected` ao lado, compila e roda cada um (nativo; `--run` usa o interpretado; `--twin` também compara os dois modos) e compara a saída com o `.expected`. Imprime `ok`/`FAIL` por arquivo e um resumo; sai com 1 se algum falhar. `.ofs` sem `.expected` são ignorados.
- `ofs twin`: roda o programa interpretado e nativo em diretórios temporários e compara stdout, código de saída e stderr normalizado. Imprime `iguais` (código 0) ou a primeira diferença (código 1). Variável `OFS_TEST_TIMEOUT` (padrão 30 s por execução).
- `ofs polish`: sinônimo de `ofs fmt`, com as mesmas opções.

## `ofs quarry`: pacotes

`ofs quarry` é o gerenciador de pacotes (a pedreira de onde se extraem as bibliotecas). Ferramenta escrita em OFS (`ofs/tools/quarry.ofs` + `quarry_json.ofs`), compilada e guardada em cache pelo wrapper como `ofs fmt`/`ofs doc`.

```bash
ofs quarry add granito            # faixa ^<última>; também granito@1.2.3, granito@^1.2, granito@beta (canal)
ofs quarry remove granito         # tira do projeto e apaga as dependências que ficaram órfãs
ofs quarry list                   # o que está instalado, com versão
ofs quarry search texto           # procura em nome, descrição e tags do registro
ofs quarry update [granito]       # sobe versões dentro das faixas do ofs.claim
ofs quarry install                # reproduz o projeto: instala o que ofs.lode diz, conferindo o sha256
ofs quarry publish [--out dist] [--origin <url>] [--deposit registry.json]
```

**Onde instala:** em `$OFS_LIB_HOME`, ou `~/.ofs/libs` — pasta onde o `attach` já procura. Cada pacote fica em `~/.ofs/libs/<pacote>/` (conteúdo do `.tar.gz` + `.assay`, com a versão e o hash conferidos) e ganha um "veio" `~/.ofs/libs/<modulo>.ofs` de uma linha (`attach {F:<pacote>/<main>}`). Assim, `attach {granito}` acha o pacote sem configuração, em `ofs build` e em `ofs run` (hífen vira sublinhado: `terminal-colors` → `attach {terminal_colors}`). A instalação é por usuário, não por projeto: dois projetos que precisem de versões diferentes do mesmo pacote alternam com `ofs quarry install`. Se o stdlib tiver um módulo com o mesmo nome (`core`, `fmt`, `io`...), ele vem primeiro na busca; o `quarry` avisa e sugere `attach {F:<pacote>/<main>}`.

**De onde baixa:** do registro `OFS_QUARRY_REGISTRY` (ou `--registry`): URL `http(s)://`, `file://` ou caminho (arquivo `.json` ou pasta com `registry.json`). Padrão: `src/packages/registry.json` do repositório no GitHub (raw). Cada versão aponta para uma release do GitHub `quarry-<pacote>-<versão>` (publicada pelo workflow `publish-libraries.yml`, sem npm nem `NPM_TOKEN`); se ela falhar, o `quarry` tenta `<pasta do registro>/dist/<arquivo>`. Com registro local, o arquivo ao lado dele vem primeiro (sem rede). O download usa `curl` do sistema: o `tls.ofs` do stdlib não segue redirecionamentos (as releases do GitHub redirecionam) e exige linkar `-lssl`; extrair usa `tar`. O sha256 é calculado em OFS (`sha256.ofs`).

### Manifesto `ofs.claim` e lock `ofs.lode`

O manifesto é um arquivo chave = valor (o ODL descreve documentos HTML, não serve para isto). Comentários com `#`:

```ini
name = meu-app
version = 0.1.0
description = Exemplo
main = libs/meu_app.ofs          # para publish; padrão libs/<nome>.ofs
# opcionais para publish: author, license, tags = a, b, channel = beta, include = libs, assets

[deps]
granito = ^1.2      # >=1.2.0 <2.0.0  (^0.3 = >=0.3.0 <0.4.0)
basalto = ~1.0.3    # >=1.0.3 <1.1.0
xisto = 1.x         # qualquer 1.*.*
ardosia = 2.0.1     # exata (também >=1.0 <2.0, *)
```

`ofs.lode` é gerado: uma seção por pacote (diretos e transitivos) com `version` exata, `sha256` do `.tar.gz`, `url` de onde veio, `main` e `deps`. `install` usa só o lock quando ele cobre o manifesto (não precisa do registro) e **recusa** o pacote se o sha256 não bater; o mesmo vale se o registro trouxer, para a versão travada, um hash diferente do lock. Versão do registro sem `sha256` não é instalável.

**Dependências entre pacotes:** um pacote publicado com `[deps]` no seu `ofs.claim` leva essas faixas para a entrada do registro. A resolução escolhe, para cada pacote, a maior versão que satisfaz todas as faixas pedidas (a do lock, se ainda servir) e segue as dependências dela. Se nenhuma versão satisfaz todas, é conflito: nada é gravado e a mensagem lista quem pediu o quê (`conflito: nenhuma versao de basalto satisfaz ^1.0 (pedido por granito@1.0.0); ^2.0 (pedido por xisto@1.0.0)`).

**Publicar:** `ofs quarry publish` (na pasta do pacote) gera `dist/<nome>-<versão>.tar.gz` reprodutível (ordem por nome, datas e donos zerados, `gzip -n`: o mesmo fonte dá o mesmo sha256) com `include` (padrão: a pasta do `main`), o manifesto, `README.md` e `LICENSE`, e imprime a entrada de registro com o hash. Não envia nada. `--origin` define a URL (`{name}`, `{version}`, `{file}` são trocados; sem ela a URL é o nome do arquivo, relativo ao registro) e `--deposit registry.json` grava a entrada no registro. `src/packages/build_packages.sh` faz isso para as 19 bibliotecas de `src/packages/src/` (que ainda usam o `ofspkg.json` antigo, também aceito).

Teste: `ofs/tests/quarry/run.sh` (registro local temporário; `HOME`/`OFS_LIB_HOME` temporários). Pendente: assinatura dos pacotes (hoje só sha256 do registro/lock), instalação por projeto, e o envio da release a partir do `publish`.
