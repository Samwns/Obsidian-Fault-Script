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
