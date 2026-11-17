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
- **Estrato e permissões** (A5.4 + Ideia 4): selos `stratum ...`, `claim ...` e `caprock ...` no topo da página do módulo, lidos do cabeçalho do arquivo (também na forma `//! claim`); `stratum abyss` ganha o selo "pede tudo". No `--dump`, uma linha `modulo|stratum ...|claim ...` por módulo que declara algo.
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

## `ofs assay` e `ofs build --release`: contratos

```bash
ofs assay arquivo.ofs          # casos de borda das veins com contrato
ofs build --release app.ofs    # binario sem as checagens de contrato em execucao
```

- `ofs assay`: para cada vein com `bearing`/`settle` (ou parâmetro `mineral`) e só parâmetros inteiros, chama a vein nos limites das regras (o valor da fronteira e ±1; parâmetro sem limite usa -1, 0 e 1; até 64 casos por vein), interpretado. Casos fora do `bearing`/`grain` são listados com `--` e não rodam; os outros mostram `ok` depois que o `settle` confere o resultado. A primeira violação para com `ofs: [CONTRATO VIOLADO ...]` no stderr e código 1. Sem nenhuma vein elegível, diz isso e sai com 0. O `core main` do programa não roda.
- Checagens de execução de contrato: ligadas no `ofs run` e no `ofs build` padrão; `ofs build --release` (ou `OFS_CONTRACTS=0` em qualquer modo) tira. A checagem estática (`[ERRO DE CONTRATO]`) vale sempre. Regras em `docs/LANGUAGE_REFERENCE.md`, seção 4.
- `ofs doc` mostra os contratos: a página da vein ganha a tabela **Contratos** (`bearing`/`settle`) e cada `mineral` aparece entre os tipos, com a base na assinatura e as `grain` na mesma tabela.

## `ofs check`: estrato e permissões dos módulos

Depois de checar, o `ofs check` lista cada módulo que declara ou usa baixo nível (o principal e os `attach`), com o estrato e os claims (regra em `docs/LANGUAGE_REFERENCE.md`, seção 12, "Estrato e permissões por módulo"):

```
Estrato e permissoes (stratum / claim) por modulo:
  app.ofs (principal): stratum fault (padrao), claim abyss
  stdlib/fs.ofs: stratum fault (padrao), claim rift
  plugins/cru.ofs: stratum fault (padrao), sem claim  [usa sem claim: rift]
  caprock do principal: os anexados podem pedir rift, abyss
```

`[usa sem claim: ...]` é o que dá `[AVISO DE PERMISSAO]` (erro com `OFS_STRICT_FAULT=1`); `[claim sem uso: ...]` é claim que o módulo não usa. Não aparece com `OFS_PROGRESS=json` (o JSON da extensão não muda).

## `ofs refract`: edição ao vivo (Fault Refract)

`ofs refract` roda o programa interpretado e aplica cada edição salva **sem reiniciar e sem perder o estado**: a vein editada entra no lugar da antiga na próxima chamada, e globais, arrays e objetos continuam com os valores que já tinham.

```bash
ofs refract app.ofs            # roda e observa app.ofs e os attach locais
ofs refract --quiet app.ofs    # sem os avisos de troca (erros e recusas continuam)
ofs refract ofs/examples/refract_demo.ofs   # exemplo: edite desenhar/atualizar e salve
```

- **Observa** o arquivo principal e os `attach` locais (a stdlib instalada e o runtime ficam de fora). A verificação acontece entre chamadas de vein, a cada `OFS_REFRACT_PERIOD` ms (padrão 200), e só recompila depois que o conteúdo fica igual por um período (o editor pode estar no meio da gravação).
- **Edições sem salvar (lava).** No VS Code, a mudança vale **sem salvar**: com a extensão (configuração `ofs.refract.liveEdits`, ligada por padrão), ~250 ms depois da última tecla o conteúdo do editor vai para um rascunho, a *lava*, em `${XDG_RUNTIME_DIR:-/tmp}/ofs-refract-<uid>/lava/<caminho absoluto do arquivo sem a primeira barra>` (no Windows, `%TEMP%\ofs-refract\lava\C\...`). Para cada arquivo observado, a Refract usa quem mudou por último: a lava ou o arquivo do disco. Salvar, fechar o arquivo ou desfazer tudo apaga a lava e a Refract volta ao disco; uma lava que já existia quando a Refract começou só vale se mudar. Não há o que configurar: a Refract cria a pasta (0700, do próprio usuário) e deixa `pulse/<pid>` enquanto roda, e a extensão só grava a lava se houver um `pulse` de processo vivo (sem Refract rodando, nada é gravado). `OFS_REFRACT_DRAFT=0` desliga a lava (na Refract; na extensão, `ofs.refract.liveEdits: false`). Fora do VS Code, é preciso **salvar** (ou ligar o salvamento automático do editor: para a Refract é igual a salvar).
- **Digitação incompleta.** Com a fonte vindo da lava, o código passa por estados inválidos enquanto você digita: a versão em execução continua e o erro **só aparece se o conteúdo ficar parado com erro por ~1 s**, uma vez (o mesmo erro não se repete; `erro de compilacao no rascunho do editor (acima); a versao em execucao continua`). Quando volta a compilar, a troca é normal. A estabilidade de sempre (só recompila com o conteúdo igual por um período) continua valendo.
- **Erro de compilação** (sintaxe, tipo, attach): o diagnóstico vai para o stderr e a versão em execução continua. Corrija e salve de novo.
- **Só o que mudou** é trocado: cada vein é comparada com a versão em execução. Veins novas e globais novos entram (um global novo começa com o valor inicial; os que já existiam mantêm o valor, mesmo que o inicializador mude no texto).
- **Veins nativas** (cristalizadas ou com `quartz`) são recompiladas num `.so` novo em segundo plano, junto com as veins nativas que as chamam; até o `.so` ficar pronto, a versão nova roda interpretada. Veins com `quartz` são compiladas antes da troca (não rodam interpretadas).
- **Assinatura compatível é aceita** mesmo com a vein em uso (onda 4): parâmetros antigos iguais e no começo, parâmetro novo no fim **com valor padrão** (`vein versao(k: stone = 7)`), retorno igual, antes `void`, inteiro estreito onde era `stone` ou `f32` onde era `crystal`. As chamadas antigas recebem o padrão. Mensagem: `a assinatura de 'versao' mudou de ()->stone para (stone)->stone: compativel (1 parametro novo com padrao: k)`.
- **`monolith` com instâncias vivas migra** (onda 4): campo novo ganha o valor padrão do tipo, campo removido some, a ordem pode mudar e campo numérico muda de tipo convertendo o valor (`stone`↔`crystal`↔inteiros estreitos↔`bool`). As instâncias são reescritas no lugar (os ponteiros para elas continuam valendo): na Refract cada instância nasce com 64 bytes de folga. Mensagem: `monolith 'Ponto' migrado ao vivo (2 instancias); campo(s) novo(s) com o valor padrao: z; convertido(s): y stone->crystal`.
- **Recusas** (o programa segue com a versão antiga e a mensagem diz o que precisa de reinício): assinatura incompatível de uma vein que uma vein em execução ainda chama (ou que foi entregue ao C como callback, ou usada como valor), vein removida que alguém em execução ainda chama, campo de `monolith` vivo que muda para um tipo que não converte (ex.: `stone` → `obsidian`) ou que cresce além da folga, global que mudou de tipo, `extern` com assinatura nova, variantes de `strata` removidas ou reordenadas.
- **Laços trocam no meio (OSR, onda 4).** Um laço em execução numa vein editada (inclusive o `while` do `main`, que nunca retorna) passa para a versão nova no próximo cabeçalho, com as variáveis locais como estão; o resto da vein depois do laço também é o novo. Variável nova antes do laço ganha o valor inicial na hora da troca, variável removida some; se uma mudou de tipo, ou o laço está dentro de `cycle`/`match`/`bedrock`, o laço termina na versão antiga (mensagem `o laco de 'main' segue na versao antiga: ...`). Um laço que virou nativo pela OSR volta ao interpretador pelo cabeçalho quando o arquivo muda (o laço nativo pergunta a cada 65536 voltas) e depois vira nativo de novo, já com a versão nova. Fora de laço, a vein em execução termina na versão antiga e a nova vale na próxima chamada.

Mensagens no stderr (curtas, com prefixo `ofs refract:`): `observando app.ofs; salve (ou so edite, no VS Code) para refratar`, `2 veins refratadas (desenhar, atualizar)`, `1 vein nova (pulsar)`, `1 global novo (bonus)`, `recristalizando em segundo plano: desenhar`, `desenhar: cristalizada(s) de novo (.so novo)`, `laco de 'main' continua na versao nova (OSR)`, `variavel(is) nova(s) antes do laco, com o valor inicial: extra`, `erro de compilacao (acima); a versao em execucao continua`, `troca recusada: ...`.

Variáveis: `OFS_REFRACT_PERIOD` (ms, 10 a 60000), `OFS_REFRACT_DRAFT=0` (ignora a lava), `OFS_REFRACT_QUIET=1` (o mesmo que `--quiet`), e as da cristalização (`OFS_CRYSTALLIZE`, `OFS_CRYSTALLIZE_THRESHOLD`, `OFS_OSR=0` para desligar a OSR nativa, `OFS_QUARTZ_CACHE`, `OFS_VERBOSE=1`). Sem o wrapper: `OFS_REFRACT=1 OFSCC_MODE=run OFSCC_INPUT=app.ofs ofscc`. Disponível no Linux e no macOS (precisa do `ofscc` ligado com `-rdynamic` para a parte nativa).

Teste: `ofs/tests/refract/run.sh` (troca interpretada, troca de vein cristalizada, erro de compilação, assinatura recusada, vein e global novos, wrapper `--quiet`, contratos, geode; onda 4: OSR no laço do `main`, OSR de laço nativo, migração de `monolith`, assinatura compatível aceita; lava: rascunho troca a vein com o disco intacto, rascunho com erro não troca nem fala antes de ~1 s, apagar o rascunho volta ao disco, `OFS_REFRACT_DRAFT=0`, e a parte do PC do `--remote` sem aparelho).

### `ofs refract --remote`: no celular (L3)

```bash
ofs refract --remote app.oal               # ou app.ofs com android_event
ofs refract --remote app.oal --serial <id>
ofs build --target android app.oal --refract --install   # só o APK de depuração
```

O app roda **nativo no aparelho** e o PC envia só as veins editadas: o build `--refract` põe uma célula de despacho por vein e o agente `geophone` (socket abstrato `ofs-refract.<pacote>`, alcançado por `adb forward`); a cada gravação o PC compara vein a vein com o manifesto que o app mandou, aplica as regras de segurança, compila o remendo com o clang do NDK (`-shared -fPIC`, ligado contra a lib do app) e envia; o app faz `dlopen`, troca as células entre dois eventos da thread principal e responde. Mesmo processo, globais com o mesmo endereço. Mensagens (prefixo `refract:`): `conectado a <pacote> (pid N, geracao G, ...)`, `alteradas: painel.texto`, `novas: painel.rotulo`, `religadas (chamam uma vein sem celula): ...`, `geracao 2 aplicada no app em 3 ms: ...`, `recusada: a assinatura de 'painel.somar' mudou (()->void -> ()->stone): ...`, `o programa nao compila; o app segue como estava`, `o app recusou o remendo: dlopen: ...`. Detalhes, regras, protocolo e segurança (o agente só existe no build `--refract` e só aceita o uid do `adb`, root ou o próprio app) em [ANDROID.md](ANDROID.md#edição-ao-vivo-no-aparelho-fault-refract-l3). Teste no aparelho: `ofs/tests/refract/android.sh` (pula sem aparelho; mesmo PID antes e depois, contagem preservada, vein nova, recusa, erro de compilação). A lava também vale aqui: o PC lê o rascunho do editor dos arquivos observados (o `.oal` e os `.ofs` locais) com a mesma regra e o mesmo silêncio de ~1 s para erro de digitação.

## `ofs fossil`: gravar uma execução e voltar no tempo

`ofs fossil` roda o programa interpretado e grava num arquivo `.fossil` tudo o que vem de fora e não é determinístico. `ofs fossil replay` roda de novo devolvendo os valores gravados no lugar dessas chamadas, sem tocar o mundo real, e a saída sai idêntica. Com `--to`, `--back` e `--dig` dá para parar em qualquer ponto da execução e voltar no tempo.

```bash
ofs fossil app.ofs                       # roda e grava app.fossil (-o outro.fossil)
ofs fossil replay app.fossil             # reproduz: mesma saida, sem ler entrada, relogio, arquivos...
ofs fossil replay app.fossil --to 120    # para antes do passo 120 e mostra locais e globais
ofs fossil replay app.fossil --to L42    # primeira vez que a linha 42 do principal executa
ofs fossil replay app.fossil --to util.ofs:7   # linha de um attach local
ofs fossil replay app.fossil --to pesar  # entrada na vein pesar
ofs fossil replay app.fossil --to L42 --back 10   # 10 passos antes desse ponto
ofs fossil replay app.fossil --back 5    # 5 passos antes do fim
ofs fossil replay app.fossil --dig       # modo interativo (abaixo)
```

**O que é gravado.** Toda chamada externa passa pelo Rift do interpretador (`extern vein`, `rift vein` e as veins nativas do runtime, como `ofs_read_line`), e cada símbolo tem uma política:

- **mundo** (gravado; no replay não é chamado): entrada (`read`, `fgets`, `fread`, `getchar`, `ofs_read_line`, `recv`, `SSL_read`, `getrandom`), relógio (`time`, `clock_gettime`, `gettimeofday`, `clock`, `localtime(_r)`, `pulse.now_ms`, `sleep`/`usleep`, que no replay não espera), aleatório (`rand`, `random`, `drand48`, `srand`, `arc4random`...), ambiente (`getenv`, `ofs_getenv`), processo (`getpid`), arquivos e rede (`fopen`, `fclose`, `fseek`, `open`, `close`, `stat`, `socket`, `connect`, `accept`, `system`, `remove`...). Grava o retorno e **os bytes que o C escreveu no buffer do programa** (`read` → `r` bytes, `fgets` → o texto, `fread` → `r * tamanho`, `clock_gettime` → 16 bytes...). No replay os bytes vão para o buffer atual (o endereço muda de uma execução para outra; o argumento é o mesmo). Texto devolvido pelo C (`getenv`, `ofs_read_line`) volta como cópia; `FILE*`/`DIR*` voltam como valor opaco que só serve às chamadas gravadas.
- **saída** (`write`, `fwrite`, `fputs`, `fputc`, `fprintf`, `fflush`, `printf`, `puts`, `putchar`, `send`, `SSL_write`): grava o retorno e um hash dos dados. No replay, a saída para o console (fd 1/2, `stdout`/`stderr`) roda de verdade; para arquivo ou rede vira **no-op conferido**: se os dados mudaram, `ofs fossil: aviso: a saida de 'fputs' (evento 13) difere da gravada`.
- **resto** (funções puras do C, `strlen`, `snprintf`, `malloc`, `memcpy`, matemática): roda de verdade nos dois modos. `OFS_FOSSIL_WORLD="minha_fn:outra"` trata funções suas como mundo (só o retorno).
- `echo` é do interpretador e se repete sozinho. Argumentos de linha de comando: a linguagem ainda não lê `argv`, então não há o que gravar.

**Cristalização e OSR ficam desligadas** durante `ofs fossil` e o replay: o código nativo não passa pelo gancho das chamadas externas nem conta passos. Veins com `quartz` continuam nativas (aviso: as chamadas externas de dentro delas não são gravadas).

**Formato** (versão 1, texto com cargas brutas prefixadas pelo tamanho; ver `ofs/ofscc/fossil.ofs`):

```
OFS-FOSSIL 1
S <hash do fonte> <n>\n<caminho do programa>
E <simbolo> <rk> <valor> [<bits float> [<hash da saida>]]   uma chamada
<n>\n<bytes>                     texto devolvido (rk 1) ou bloco novo (rk 3)
M <arg> <n>\n<bytes>             memoria escrita pelo C no argumento
F <passos> <eventos> <codigo>    fim
```

`rk`: 0 = valor bruto, 1 = texto, 2 = o próprio argumento (o `fgets` devolve o buffer), 3 = bloco de memória novo (`localtime`). Zeros do fim da linha `E` são omitidos. Um programa de 200 mil `rand()` grava 3,9 MB.

**Programa alterado.** O cabeçalho guarda o hash do conteúdo do principal e dos `attach` locais. Se mudou: `ofs fossil: aviso: o programa mudou desde a gravacao (o hash do fonte difere); o replay pode divergir`. Se o programa fizer outra chamada externa que a gravada, o replay para com código 3: `o replay divergiu no evento 1 (passo 2): a gravacao tem 'time', o programa chamou 'rand'`. No fim, o número de passos é conferido com o gravado. Sem `OFSCC_INPUT`, o replay usa o caminho do cabeçalho.

**Voltar no tempo.** Um *passo* é um statement executado (os do `main`, das veins chamadas e da inicialização dos globais), contado igual na gravação e no replay. `--to` para **antes** do passo e mostra onde está e as variáveis (locais da vein e globais declarados nos arquivos do programa; arrays e `monolith` por dentro):

```
fossil: passo 28 | mundo.ofs:22 | vein pesar
  22 |     acumulado = acumulado + peso
  locais:
    nome: obsidian = "samuel"
    sorte: stone = 769
    peso: stone = 1369
  globais:
    acumulado: stone = 0
```

A saída do programa não aparece até o ponto. Para voltar, o replay guarda **camadas**: a cada `--layer N` passos (padrão 2000) o processo inteiro é duplicado com `fork` (cópia sob demanda: variáveis, heap do interpretador, memória crua e a do C) e a cópia dorme. Ir a um passo anterior acorda a camada mais próxima antes dele, que reexecuta (sem saída) até o passo; a linha do tempo atual termina. São no máximo 32 camadas: passando disso, fica uma sim, outra não, e o intervalo dobra. O processo que o shell espera vira um supervisor que só recebe o código de saída (no Linux é "subreaper", então percebe se a execução cair: `ofs fossil: o replay caiu (sinal 9)`).

**Modo interativo** (`--dig`, ou `-i`): começa no passo 1 (ou no `--to`) e lê comandos do terminal:

| comando | atalho | o quê |
|---|---|---|
| `segue [n]` | `n` | avança n passos mostrando a saída do programa |
| `cava [n]` | `b` | volta n passos no tempo |
| `ir <alvo>` | `g` | vai a um passo (antes ou depois), `L<linha>`, `arquivo.ofs:<linha>` ou vein (linha e vein procuram para a frente) |
| `ver <nome>` | `p` | mostra uma variável (local e depois global) |
| `estrato` | `e` | locais e globais |
| `onde` | `w` | passo, arquivo:linha, vein e o texto da linha |
| `sai` | `q` | encerra |

No fim do programa o modo continua (`fossil: fim do programa (passo 47, codigo 0)`) e `cava`/`ir <passo>` ainda funcionam; um erro de execução também cai no prompt.

**Custo** (Ryzen 5 5600GT, `bench/ofs/fib.ofs` interpretado, `fib(35)`, 74,6 milhões de passos): puro 6,21 s (antes desta mudança: 6,42 s, ou seja, o interpretador sem gravação não ficou mais lento); gravando 6,40 s (+3%); replay 6,43 s. Com muitas chamadas externas o custo é por chamada: 200 mil `rand()` levam 0,05 s puro, 0,32 s gravando (cada evento vai para o disco na hora, para um programa que cai deixar a gravação até a queda) e 0,14 s com `OFS_FOSSIL_SYNC=0` (grava no fim); o replay leva 0,12 s.

Variáveis (sem o wrapper): `OFS_FOSSIL=record|replay`, `OFS_FOSSIL_FILE=arq.fossil`, `OFS_FOSSIL_TO`, `OFS_FOSSIL_BACK`, `OFS_FOSSIL_DIG=1`, `OFS_FOSSIL_LAYER`, `OFS_FOSSIL_SYNC=0`, `OFS_FOSSIL_WORLD`; ex.: `OFS_FOSSIL=record OFS_FOSSIL_FILE=app.fossil OFSCC_MODE=run OFSCC_INPUT=app.ofs ofscc`. Gravar e reproduzir funcionam onde o `ofs run` funciona; voltar no tempo (`--back`, `--dig`, `cava`) precisa de `fork` (Linux, Android/Termux, macOS; no Windows só `--to` para a frente).

**VS Code:** comandos `OFS: Fossil: Go to a Point of a Recording`, `Fossil: Next Step` e `Fossil: Previous Step` rodam `ofs fossil replay --to`, mostram o estado no canal "OFS Fossil" e marcam a linha do passo no editor. Um adaptador de depuração (DAP) com passo para trás sobre o `--dig` é o próximo passo.

Limites conhecidos: `scanf`/`fscanf` (saídas por ponteiros variádicos) e funções que preenchem estruturas fora da tabela não restauram a memória (use `read`/`fgets` + `sscanf`, que é puro); apps de UI (eventos X11/Android) não são gravados; chamadas por valor de vein vindo do C (`call_native_sig`) rodam de verdade.

Teste: `ofs/tests/fossil/run.sh` (grava um programa com entrada padrão, relógio, aleatório, ambiente, arquivo e `rift vein` com `bind`; reproduz com tudo trocado e confere a saída idêntica e o arquivo não reescrito; `--to` por passo, linha, `arquivo:linha` e vein com o valor das variáveis; `--back` com camadas de 7 e 2000 passos; `--dig` com `ir`/`ver`/`segue`/`cava`/`estrato`; programa alterado avisa e diverge; nenhuma camada sobra).

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
