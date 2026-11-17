# Guia de Introdução ao OFS

O OFS (Obsidian Fault Script) é uma linguagem de programação híbrida, self-hosted, que conta com um interpretador AST integrado para execução instantânea (`ofs run`) e um backend LLVM para geração de executáveis binários nativos de alta performance (`ofs build`).

O ecossistema inclui compilador, biblioteca padrão, ferramentas de linha de comando e as linguagens de domínio web ODL e OES.

---

## 1. Instalação

### Pacotes pré-compilados

Instaladores oficiais estão disponíveis para diversas distribuições e sistemas operacionais:

- **Debian / Ubuntu**:
  ```bash
  sudo dpkg -i ofs-debian-ubuntu-x64-installer-v*.deb
  ```
- **Fedora / RHEL / openSUSE**:
  ```bash
  sudo rpm -Uvh ofs-fedora-x64-installer-v*.rpm
  ```
- **Arch / Manjaro**:
  ```bash
  sudo pacman -U ofs-arch-x64-installer-v*.pkg.tar.zst
  ```
- **macOS (Apple Silicon / ARM64)**:
  Execute o script instalador fornecido no release do GitHub.
- **Windows (x64)**:
  Execute o instalador `ofs-windows-x64-installer-v*.exe`.
- **VS Code**:
  Instale a extensão oficial `ofs-vscode-extension.vsix` para suporte a sintaxe, realce e recarregamento dinâmico (*Go Live*).

### Compilação a partir do código-fonte (Bootstrap)

Para compilar o compilador diretamente a partir do repositório:

```bash
git clone https://github.com/Samwns/Obsidian-Fault-Script.git
cd Obsidian-Fault-Script
bash ofs/bootstrap/scripts/bootstrap-minimal.sh
```

O binário do compilador será gerado em `ofs/dist/ofs` (ou `ofs/dist/ofscc`).

---

### Termux (Android)

O OFS roda no próprio celular, dentro do Termux (aarch64).

**Instalar** (no Termux):

```bash
curl -fsSL https://raw.githubusercontent.com/Samwns/Obsidian-Fault-Script/main/src/packaging/termux/install_termux.sh | bash
# ou, com o pacote local:  bash install_termux.sh ofs-termux-aarch64.tar.gz
```

O instalador roda `pkg install -y clang`, baixa a última release `ofs-termux-aarch64.tar.gz`, extrai em `$PREFIX` (`bin/ofs` e `lib/ofs/`) e testa `ofs --version`.

**Usar:** `ofs run hello.ofs` (interpretado), `ofs build hello.ofs -o hello && ./hello` (nativo aarch64, liga com o `clang` do Termux). As ferramentas ficam em `$PREFIX/lib/ofs/tools/` (`ofs run $PREFIX/lib/ofs/tools/odlc.ofs`).

**Limitações:** sem janelas X11 por enquanto (`.oll`/`.oal` em janela); gerar APK no celular (`ofs build --target android`) fica para depois.

## 2. Comandos da Linha de Comando (CLI)

O executável `ofs` expõe os seguintes subcomandos:

| Comando | Descrição |
|---|---|
| `ofs <arquivo.ofs>` | Executa o programa diretamente (equivale a `ofs run`). |
| `ofs run <arquivo.ofs> [--native]` | Executa no interpretador, que cristaliza as veins quentes em nativo sozinho (ou com `--native`, compila o programa inteiro num temporário). Capítulo 12. |
| `ofs build <arquivo.ofs> -o <binário>` | Compila o arquivo para um executável binário nativo via LLVM. |
| `ofs check <arquivo.ofs>` | Valida a sintaxe e a tipagem sem emitir código. |
| `ofs refract <arquivo.ofs>` | Roda e troca ao vivo as veins que você editar e salvar; `--remote` faz isso num app Android. Capítulo 13. |
| `ofs assay <arquivo.ofs>` | Casos de borda das veins com contrato (`bearing`/`settle`). Capítulo 14. |
| `ofs fossil <arquivo.ofs>` | Grava uma execução e permite reproduzi-la e voltar no tempo (`ofs fossil replay`). Capítulo 19. |
| `ofs quarry <add\|list\|install\|...>` | Gerenciador de pacotes. Capítulo 19. |
| `ofs fmt` / `ofs polish` | Formata o código (`--check` e `--diff` para CI). |
| `ofs test [pasta]` | Roda os `.ofs` que têm um `.expected` ao lado e compara a saída. |
| `ofs twin <arquivo.ofs>` | Roda interpretado e nativo, comparando as saídas. |
| `ofs doc [alvo]` | Gera a documentação HTML a partir dos comentários do código. |
| `ofs language [en\|pt\|es]` | Mostra ou altera o idioma da CLI. |
| `ofs ir <arquivo.ofs>` / `ofs asm` / `ofs tokens` / `ofs ast` | Artefatos de depuração: LLVM IR, assembly, tokens e AST. |
| `ofs ui <arquivo.oll> [-o saída.ppm]` | Renderiza um layout OLL nativo para imagem. |
| `ofs clean` | Limpa artefatos temporários de build. |
| `ofs version` / `ofs update` / `ofs help` | Versão, atualização para a última release e ajuda. |

Variáveis de ambiente reconhecidas pelo launcher: `OFS_LANG` (idioma en/pt/es), `OFS_PROGRESS` (reporter fancy/plain/json), `OFS_VERBOSE` (logs de link/build), `OFS_RUNTIME` e `OFS_STDLIB_PATH` (caminhos da runtime e da biblioteca padrão), `OFS_CRYSTALLIZE=0` (desliga a promoção automática a nativo no `ofs run`) e `OFS_CONTRACTS=0` (tira as checagens de contrato em execução). Referência completa das ferramentas: [TOOLS.md](TOOLS.md).

---

## 3. Primeiro Programa

Crie um arquivo chamado `hello.ofs`:

```ofs
core main() {
    echo("Olá, Mundo via OFS!")
}
```

Execução direta no interpretador AST:

```bash
ofs hello.ofs
# ou: ofs run hello.ofs
```

Ou compilação para executável binário nativo:

```bash
ofs build hello.ofs -o hello
./hello
```

Saída:
```text
Olá, Mundo via OFS!
```

---

## 4. Tipos de Dados

OFS é estaticamente tipado. O compilador oferece inferência de tipos em declarações de variáveis locais (`forge`).

### Tipos fundamentais

- `stone`: Inteiro de 64 bits com sinal.
- `crystal`: Ponto flutuante de 64 bits (IEEE 754).
- `obsidian`: Sequência de caracteres UTF-8 (*string*).
- `bool`: Booleano (`true` ou `false`).
- `void`: Ausência de valor de retorno.

### Inteiros de largura fixa

- Sem sinal: `u8`, `u16`, `u32`, `u64`
- Com sinal: `i8`, `i32`

Exemplo de declaração e conversão explícita:

```ofs
core main() {
    forge contador: stone = 10
    forge taxa: crystal = 3.14159
    forge ativo: bool = true
    forge nome: obsidian = "Obsidian"

    forge canal_r: u8 = 255
    forge canal_g: u8 = 128
    forge pixel: u32 = (canal_r as u32 << 16) | (canal_g as u32 << 8)

    echo(nome)
    echo(pixel)
}
```

---

## 5. Funções

Funções são definidas com a palavra-chave `vein`. O ponto de entrada nativo do programa é a função `core main()`.

```ofs
vein somar(a: stone, b: stone) -> stone {
    return a + b
}

vein fatorial(n: stone) -> stone {
    if (n <= 1) {
        return 1
    }
    return n * fatorial(n - 1)
}

core main() {
    forge s = somar(15, 27)
    forge f = fatorial(5)
    echo(s)
    echo(f)
}
```

Funções podem compor chamadas entre si e invocar outras rotinas:

```ofs
vein quadrado(n: stone) -> stone {
    return n * n
}

vein soma_quadrados(a: stone, b: stone) -> stone {
    return quadrado(a) + quadrado(b)
}

core main() {
    echo(soma_quadrados(3, 4))
}
```

> **Nota**: Funções anônimas que capturam variáveis são `geode (x: stone) -> stone { return x + base }` (closures; ver a seção "`geode`" da [Referência](LANGUAGE_REFERENCE.md)). O valor tem o tipo `vein(stone) -> stone` e se passa, guarda e chama como qualquer vein.

---

## 6. Estruturas de Dados (`monolith`)

OFS utiliza a palavra-chave `monolith` para definir tipos compostos e blocos `impl` para associar métodos:

```ofs
monolith Retangulo {
    largura: stone
    altura: stone
}

impl Retangulo {
    vein area(self: Retangulo) -> stone {
        return self.largura * self.altura
    }

    vein perimetro(self: Retangulo) -> stone {
        return (self.largura + self.altura) * 2
    }
}

core main() {
    forge r: Retangulo
    r.largura = 10
    r.altura = 5

    echo(r.area())
    echo(r.perimetro())
}
```

---

## 7. Módulos e Importações (`attach`)

A declaração `attach` importa módulos da biblioteca padrão instalada ou arquivos locais:

```ofs
attach {math}
attach {string}
attach {io}
```

Para importar arquivos locais relativos ou absolutos, utilize o prefixo `F:`:

```ofs
attach {F:./modulos/auxiliar.ofs}
```

---

## 8. Controle de Fluxo

### Condicionais (`if` / `else`)

```ofs
core main() {
    forge x: stone = 10
    if (x > 0) {
        echo("positivo")
    } else if (x < 0) {
        echo("negativo")
    } else {
        echo("zero")
    }
}
```

### Laços de repetição (`while`)

```ofs
core main() {
    forge i: stone = 0
    while (i < 5) {
        echo(i)
        i = i + 1
    }
}
```

### Seleção de múltiplos caminhos

Ramificações com múltiplas condições utilizam `if / else if / else`:

```ofs
core main() {
    forge status_code = 200
    if (status_code == 200) {
        echo("Sucesso")
    } else if (status_code == 404) {
        echo("Não encontrado")
    } else {
        echo("Outro código")
    }
}
```

Quando todas as condições comparam o mesmo valor, `match` (sinônimo: `assay`) é mais curto; `_` é o caso padrão:

```ofs
core main() {
    forge status_code = 404
    match (status_code) {
        200 => { echo("Sucesso") }
        404 => { echo("Não encontrado") }
        _ => { echo("Outro código") }
    }
}
```

### Tratamento e validação de erros (`throw`)

A instrução `throw` exibe uma mensagem de falha e encerra o processo com código de erro 1:

```ofs
vein dividir(dividendo: stone, divisor: stone) -> stone {
    if (divisor == 0) {
        throw "Divisão por zero inválida"
    }
    return dividendo / divisor
}

core main() {
    echo(dividir(10, 2))
}
```

> **Nota**: A estrutura `tremor { ... } catch (...) { ... }` está modelada na gramática (`NK_TREMOR`), com desbobinamento estruturado previsto para as próximas etapas de runtime e interpretador.

---

## 9. O Rift: bibliotecas nativas do sistema (`rift` / `extern vein`)

O **Rift** (a fenda) é a fronteira por onde o OFS alcança as bibliotecas nativas do sistema (libc, libm, X11, JNI do Android, kernel32). `rift vein` e `extern vein` declaram uma função que mora numa biblioteca nativa, e ela é chamada pela convenção de chamada do sistema. Não há código C no OFS: o compilador e o runtime são escritos em OFS.

> A convenção de chamada do sistema é conhecida tecnicamente como ABI C (`abi c`); não envolve código C.

```ofs
extern vein puts(s: obsidian) -> stone
rift vein strlen(s: obsidian) -> stone bind "strlen" abi c

core main() {
    puts("Mensagem enviada via puts da libc")
}
```

Funciona igual no `ofs run` e no `ofs build`. Um módulo que declara `extern`/`rift vein` pede `claim rift` no topo (veja o capítulo 17, que também mostra callbacks e `f32`/`crystal` pelo Rift).

---

## 10. Stack Web: ODL e OES

O ecossistema OFS possui duas linguagens de domínio específico (DSLs) dedicadas à construção de interfaces web estáticas e dinâmicas:

### ODL (Obsidian Document Language)

Estrutura semântica para documentos web compilada para HTML5:

```odl
page "Aplicação OFS" "pt"
skin "estilo.css"

flow
  deck.navegacao
    title1 "Título Principal"
    link.botao "Documentação" "#docs"
  stage#conteudo
    band.artigo
      mark "## Conteúdo nativo\nTexto formatado em **Markdown**."
      raw "<div class=\"custom\">HTML bruto quando necessário</div>"
  spark "app.js"
```

Compilação do ODL (a ferramenta `odlc` faz parte da biblioteca padrão de ferramentas e roda com o próprio `ofs`):

```bash
# no repositório OFS (ou com as tools instaladas em /usr/share/ofs/tools)
ODLC_INPUT=documento.odl ODLC_OUTPUT=public/index.html ofs run ofs/tools/odlc.ofs
```

### OES (Obsidian Effect Scripts)

Definição de estilos, variáveis, regras responsivas e animações compilada para CSS:

```oes
gem primario "#5b34ea"
gem fundo "#120e24"

paint .botao
  fill "$primario"
  curve "6px"
  space "8px 16px"
  edge "none"

veil "(max-width: 768px)"
  paint .grade
    grid-cols "1fr"

pulse "surgir"
  rise
    opacity "0"
  rest
    opacity "1"
```

Compilação do OES (mesmo padrão, usando a ferramenta `oesc`):

```bash
OESC_INPUT=estilo.oes OESC_OUTPUT=public/estilo.css ofs run ofs/tools/oesc.ofs
```

### Servidor HTTP nativo (`webserver`)

A biblioteca padrão fornece um servidor HTTP nativo integrado ao runtime:

```ofs
attach {webserver}

core main() {
    forge html = "<h1>Servidor Nativo OFS</h1><p>Executando sem dependências externas.</p>"
    webserver.serve_html_forever(8080, html)
}
```

---

## 11. Recursos de Baixo Nível

OFS suporta manipulação direta de memória e contextos específicos quando necessário:

- `fracture { ... }`: Bloco com contexto de ponteiros tipados.
- `abyss { ... }`: Fronteira explícita para operações de baixo nível de máquina.
- `bedrock`: Biblioteca padrão para alocações explícitas de células e regiões de memória (e, como bloco, a arena do capítulo 15).

```ofs
attach {bedrock}

core main() {
    forge celula = bedrock_cell_new(42)
    echo(bedrock_cell_read(celula))
    bedrock_cell_drop(celula)
}
```

Os capítulos 15 e 17 mostram como fazer isso com segurança (`seal`, arena `bedrock { }`, `fracture`, `abyss`, `stratum`/`claim`/`caprock`).

---

## 12. `ofs run` × `ofs build`: rápido de escrever, rápido de rodar

Os dois comandos aceitam o mesmo programa e dão a mesma saída. A diferença é quando o trabalho acontece:

- **`ofs run app.ofs`** interpreta o programa na hora: não gera binário, ótimo para escrever e testar.
- **`ofs build app.ofs -o app`** gera um executável nativo (LLVM) que você distribui e roda sem o `ofs`.

Você não precisa escolher o `ofs build` só por velocidade: o `ofs run` **se acelera sozinho**. Ele conta as chamadas de cada `vein`; quando uma fica "quente" (1000 chamadas, por padrão), é compilada para código nativo em segundo plano (**cristalização**) e as chamadas seguintes passam a usá-la. Um laço longo dentro do `main` também vira nativo no meio da execução. Se não houver `clang`, o programa continua interpretado, só que mais devagar.

```ofs
vein fib(n: stone) -> stone {
    if (n < 2) { return n }
    return fib(n - 1) + fib(n - 2)
}

core main() {
    echo(fib(32))
}
```

```bash
ofs run fib.ofs                       # 2178309, cristaliza sozinho (cerca de 0,06 s aqui)
OFS_CRYSTALLIZE=0 ofs run fib.ofs     # 2178309, só interpretado (cerca de 1,5 s aqui)
ofs build fib.ofs -o fib && ./fib     # 2178309, nativo
```

Use `OFS_CRYSTALLIZE=0` para desligar a promoção (útil para depurar) e `OFS_VERBOSE=1` para ver quando ela acontece. Os tempos variam com a máquina; as medidas comparáveis ficam em [PERFORMANCE.md](PERFORMANCE.md). O modelo completo está em [COMPILED_LANGUAGE.md](COMPILED_LANGUAGE.md#cristalização-veins-quentes-viram-nativas).

---

## 13. Edição ao vivo: `ofs refract`

`ofs refract app.ofs` roda o programa e observa o arquivo. Cada vez que você edita uma `vein` e salva, ela é trocada **com o programa aberto**, sem reiniciar e sem perder as variáveis globais.

```ofs
claim rift
extern vein usleep(us: stone) -> stone

vein msg() -> obsidian { return "ola" }

core main() {
    forge i = 0
    while (i < 20) {
        echo(msg() + " " + ofs_stone_to_obsidian(i))
        usleep(250000)
        i++
    }
}
```

```bash
ofs refract vivo.ofs
# ofs refract: observando vivo.ofs; salve para refratar
# ola 0
# ola 1
# (troque "ola" por "alo" no editor e salve)
# ofs refract: 1 vein refratada (msg)
# alo 2
```

O laço do `main` continua de onde estava: só a `vein` editada muda. Se o arquivo tem erro, o diagnóstico aparece e a versão em execução segue. Mudanças que não dá para fazer ao vivo (por exemplo, trocar o tipo de um global) são recusadas com uma mensagem que diz o que precisa de reinício.

**No celular (Android):** `ofs refract --remote app.oal` mantém o app **nativo rodando no aparelho** e envia só as veins que você editou (por `adb`). Veja [ANDROID.md](ANDROID.md#edição-ao-vivo-no-aparelho-fault-refract-l3) para preparar o app e [TOOLS.md](TOOLS.md#ofs-refract-edição-ao-vivo-fault-refract) para as regras e variáveis. Referência da linguagem: [Edição ao vivo](LANGUAGE_REFERENCE.md#edição-ao-vivo-ofs-refract-fault-refract).

---

## 14. Contratos: `mineral`, `grain`, `bearing`, `settle`

Contratos tiram do código o `if` de validação repetido. Um **`mineral`** é um tipo com regra (`grain`); uma `vein` pode exigir algo de quem chama (`bearing`) e prometer algo sobre o retorno (`settle`, onde `result` é o valor devolvido).

```ofs
mineral Idade = stone grain value >= 0 grain value <= 150

vein dividir(a: stone, b: stone) -> stone
    bearing b != 0
    settle result * b + a % b == a
{
    return a / b
}

core main() {
    forge i: Idade = 30        // confere a regra
    echo(dividir(17, 5))       // 3
    forge zero = 0
    echo(dividir(1, zero))     // viola o bearing
}
```

Saída: `3` e depois, no stderr e com código de saída 1:

```text
ofs: [CONTRATO VIOLADO contrato.ofs:4:12]: bearing de 'dividir' falhou: b != 0 (b = 0)
```

Quando o erro é provável em compilação (`dividir(1, 0)`, `forge i: Idade = 200`), o compilador avisa com `[ERRO DE CONTRATO]` antes de rodar.

- **`ofs assay arquivo.ofs`** chama as veins com contrato nos valores de fronteira das regras e mostra o que passa:

```text
$ ofs assay metade.ofs
assay metade(n: stone)
  --   metade(-1): fora do bearing n >= 0
  ok   metade(0)
  ok   metade(1)
assay: 2 caso(s) ok em 1 vein(s); 1 fora das regras de entrada
```

- **`ofs build --release`** (ou `OFS_CONTRACTS=0`) tira as checagens de **execução** do binário; as checagens de compilação ficam.

Referência: [Contratos e tipos com regra](LANGUAGE_REFERENCE.md#contratos-e-tipos-com-regra-mineral-grain-bearing-settle) e [`ofs assay`](TOOLS.md#ofs-assay-e-ofs-build---release-contratos).

---

## 15. Limpeza segura: `seal` e a arena `bedrock { }`

### `seal`: roda ao sair do escopo

`seal { ... }` (ou `anchor`, é a mesma coisa) registra uma ação que roda **quando o escopo termina, por qualquer caminho**: fim do bloco, `return`, `shatter`/`erode`. Serve para fechar arquivos, liberar memória e desfazer estado, sem repetir a limpeza em cada `return`.

```ofs
vein processar(nome: obsidian) -> stone {
    echo("abre " + nome)
    seal { echo("fecha " + nome) }
    if (nome == "x") { return 1 }
    echo("trabalhando em " + nome)
    return 0
}

core main() {
    echo(processar("a"))
    echo(processar("x"))
}
```

```text
abre a
trabalhando em a
fecha a
0
abre x
fecha x
1
```

Vários `seal` rodam em ordem inversa (o último registrado roda primeiro). Referência: [`seal` e `anchor`](LANGUAGE_REFERENCE.md#seal-e-anchor-rodar-ao-sair-do-escopo).

### `bedrock { }`: uma arena de memória

Um bloco `bedrock { ... }` que **aloca** (texto, `Array`, `ofs_alloc`) usa uma **arena**: tudo o que ele aloca sai de uma vez quando o bloco termina. É a forma mais simples de processar muita coisa temporária sem vazar memória.

```ofs
core main() {
    forge total = 0
    forge ultimo = ""
    forge i = 0
    while (i < 1000) {
        bedrock {
            forge partes: Array<obsidian> = []
            ofs_array_push(partes, "item" + ofs_stone_to_obsidian(i))
            total = total + ofs_str_len(partes[0])
            ultimo = partes[0]        // o texto sai copiado para fora
        }                             // a arena inteira é liberada aqui
        i++
    }
    echo(total)    // 6890
    echo(ultimo)   // item999
}
```

Regra de ouro: **texto `obsidian` é copiado** para fora da arena automaticamente; `Array`, `monolith` e ponteiros que "fogem" do bloco são `[ERRO DE ARENA]` em compilação, com a dica de alocar fora do bloco. Também existe o aviso `[AVISO DE VAZAMENTO]` para um `ofs_alloc` que nunca é liberado. Referência: [Arena `bedrock { }`](LANGUAGE_REFERENCE.md#arena-bedrock---a53).

---

## 16. Veins como valores: `vein(T) -> R` e `geode`

O nome de uma `vein` é um **valor** com tipo de função, escrito com a mesma palavra: `vein(stone) -> stone`. Dá para guardá-lo em variável, `Array` ou campo, passar como argumento e devolver.

```ofs
vein dobro(x: stone) -> stone { return x * 2 }
vein triplo(x: stone) -> stone { return x * 3 }

vein aplicar(f: vein(stone) -> stone, x: stone) -> stone { return f(x) }

core main() {
    forge f: vein(stone) -> stone = dobro
    echo(f(21))                 // 42
    echo(aplicar(triplo, 4))    // 12
    forge ops: Array<vein(stone) -> stone> = [dobro, triplo]
    echo(ops[1](10))            // 30
}
```

Um **`geode`** é uma vein sem nome que **captura** variáveis do escopo de fora (o que outras linguagens chamam de closure ou lambda). A captura é uma cópia do valor no momento em que o geode nasce.

```ofs
vein somador(base: stone) -> vein(stone) -> stone {
    return geode (x: stone) -> stone { return x + base }
}

core main() {
    forge mais10 = somador(10)
    forge mais1 = somador(1)
    echo(mais10(5))    // 15
    echo(mais1(5))     // 6
}
```

Referência: [Tipos de função e valores de vein](LANGUAGE_REFERENCE.md#tipos-de-função-e-valores-de-vein) e [`geode`](LANGUAGE_REFERENCE.md#geode-vein-que-captura-closure).

---

## 17. Baixo nível com grades

O OFS deixa mexer em memória crua e em assembly, mas separa **onde** e **o quê**, e cada módulo diz o que usa.

### Blocos `fracture` e `abyss`

`*p` (ler/escrever por ponteiro) e `&x` (endereço) só valem dentro de um bloco de baixo nível. `fracture { }` é o bloco tipado para ponteiros; `abyss { }` é o cru e irrestrito (endereços de função, `fault_call`, `quartz`).

```ofs
core main() {
    forge x = 5
    fracture {
        forge p: *stone = &x
        *p = *p + 1
    }
    echo(x)    // 6
}
```

Fora do bloco, a mesma conta é erro de compilação. Regras completas: [Regras de `fracture`, `bedrock`, `tectonic` e `abyss`](LANGUAGE_REFERENCE.md#regras-de-fracture-bedrock-tectonic-e-abyss-f4).

### `stratum`, `claim` e `caprock`: permissões por módulo

No topo do arquivo, o módulo declara o **estrato** (quanto baixo nível usa) e os **claims** (qual). Quem anexa o módulo vê o que ele pede, e o programa principal pode impor um teto com `caprock`.

```ofs
// io_cru.ofs
claim rift                 // este módulo declara extern vein

extern vein getenv(name: obsidian) -> obsidian
vein casa() -> obsidian { return getenv("HOME") }
```

```ofs
// principal.ofs
caprock crust              // nenhum módulo anexado pode pedir baixo nível
attach {F:io_cru.ofs}

core main() { echo(1) }
```

Rodando o principal: `[ERRO DE PERMISSAO ...]: o modulo io_cru.ofs pede 'rift', mas o caprock do programa principal libera so: nada (caprock crust)`. E um arquivo com `stratum crust` não aceita `&x` nem `extern`:

```text
[ERRO DE ESTRATO stratum.ofs:5:24]: endereco '&x' num arquivo 'stratum crust' (sem baixo nivel)
```

Referência: [Estrato e permissões por módulo](LANGUAGE_REFERENCE.md#estrato-e-permissões-por-módulo-stratum-claim-caprock).

### O Rift: funções nativas do sistema

O **Rift** é a fenda por onde o OFS chama as bibliotecas nativas do sistema (libc, libm, X11, JNI do Android). Não há código C no OFS: o compilador e o runtime são escritos em OFS. Declare a função com `rift vein` (ou `extern vein`) e peça `claim rift`. Funciona igual no `ofs run` e no `ofs build`.

```ofs
claim rift

rift vein strlen(s: obsidian) -> stone bind "strlen" abi c
extern vein puts(s: obsidian) -> stone

core main() {
    echo(strlen("obsidiana"))    // 9
    puts("escrito pela libc")
}
```

Uma `vein` ou um `geode` **sem captura** pode ser entregue ao sistema como callback (por exemplo, o comparador do `qsort`); `f32` e `crystal` também atravessam o Rift. Referência: [`extern vein` e `rift vein`](LANGUAGE_REFERENCE.md#11-extern-vein-e-rift-vein).

### `quartz`: assembly inline

`quartz { ... }` escreve instruções de máquina dentro da `vein`, em AT&T por padrão. Só vale em `abyss` e é específico da arquitetura (o exemplo é x86-64).

```ofs
claim abyss, quartz

abyss vein somar(a: stone, b: stone) -> stone {
    forge total: stone = 0
    quartz {
        "movq $1, $0"
        "addq $2, $0"
        in "r" a
        in "r" b
    } -> total "=&r"
    return total
}

core main() {
    echo(somar(20, 22))    // 42
}
```

No `ofs run`, a vein com `quartz` vira nativa automaticamente (precisa do `clang` na primeira execução). Referência: [`quartz`](LANGUAGE_REFERENCE.md#quartz-assembly-inline).

---

## 18. Carregar OFS em execução: `magma.cast`

Um programa pode compilar um **mod** escrito em OFS enquanto roda e chamar as veins dele: plugins, scripts de jogo. Como é código novo entrando no processo, o arquivo precisa de `claim magma`.

```ofs
// mods/forja.ofs
vein forjar(n: stone) -> stone { return n * n + 1 }
```

```ofs
attach {magma}
claim magma

core main() {
    forge m = magma.cast("mods/forja.ofs")
    if (!magma.cooled(m)) {
        echo(magma.crack(m))       // mensagem do compilador
        return
    }
    forge forjar: vein(stone) -> stone = magma.vent(m, "forjar")
    echo(forjar(10))               // 101
    magma.release(m)
}
```

Um erro no mod não derruba o programa: `magma.cooled` dá `false` e `magma.crack` traz a mensagem. Referência: [`magma.cast`](LANGUAGE_REFERENCE.md#compilar-e-carregar-ofs-em-execução-magmacast-ideia-5). Exemplo do repositório: `ofs/examples/magma_cast_demo.ofs`.

---

## 19. Ferramentas do dia a dia

Todas as ferramentas abaixo são programas escritos em OFS que o `ofs` compila na primeira vez e guarda em cache. A lista completa está em [TOOLS.md](TOOLS.md).

### `ofs fmt` e `ofs polish`: formatar

Arruma recuo e espaços sem mudar o programa. `ofs polish` é o mesmo comando com outro nome.

```bash
ofs fmt feio.ofs          # reescreve no lugar
ofs fmt --diff feio.ofs   # só mostra a diferença
ofs fmt --check src/      # no CI: lista o que mudaria e sai com 1
```

```ofs
// antes                          // depois
vein soma(n:stone)->stone {       vein soma(n: stone) -> stone {
  forge t=0                           forge t = 0
  ...                                 ...
```

### `ofs test` e `ofs twin`: conferir

`ofs test [pasta]` roda cada `x.ofs` que tem um `x.expected` ao lado e compara a saída. `ofs twin x.ofs` roda interpretado **e** nativo e confirma que dão o mesmo resultado.

```bash
printf '55\n' > conta.expected     # a saída esperada de conta.ofs
ofs test .        # ok    conta.ofs  /  resumo: 1 ok, 0 falha(s), 1 teste(s)
ofs twin conta.ofs                 # iguais: conta.ofs
```

### `ofs doc`: documentação do código

Comente cada declaração com `//` logo acima e o `ofs doc` gera um site de referência (uma página por módulo, busca e cobertura).

```ofs
// Soma os n primeiros números.
// @param n quantos números somar
// @returns a soma de 1 até n
vein soma(n: stone) -> stone { ... }
```

```bash
ofs doc conta.ofs -o docs_api   # gera docs_api/index.html
ofs doc --check                 # só a cobertura e as funções citadas nos .md que não existem
```

### `ofs fossil`: gravar uma execução e voltar no tempo

`ofs fossil` roda o programa e grava, num `.fossil`, tudo o que vem de fora e muda a cada vez (relógio, aleatório, entrada, arquivos, rede). O `replay` repete a execução com os valores gravados, sem tocar o mundo, e deixa **parar em qualquer ponto**.

```ofs
claim rift
extern vein rand() -> stone
extern vein time(t: stone) -> stone

core main() {
    echo(rand() % 1000)
    echo(time(0) > 0)
}
```

```bash
ofs fossil rel.ofs                      # 383, true  (grava rel.fossil)
ofs fossil replay rel.fossil            # 383, true  (o mesmo, sempre)
ofs fossil replay rel.fossil --to L7    # para na linha 7 e mostra locais e globais
ofs fossil replay rel.fossil --dig      # modo interativo, com passo para trás
```

(O número aleatório muda a cada gravação; o replay devolve sempre o gravado.)

### `ofs quarry`: pacotes

`ofs quarry` instala bibliotecas da comunidade para o `attach` achar sem configuração.

```bash
ofs quarry search cores     # procura no registro
ofs quarry add terminal-colors   # instala e anota em ofs.claim / ofs.lode
ofs quarry list             # o que está instalado
ofs quarry install          # reproduz o projeto a partir do ofs.lode (confere o sha256)
```

```ofs
attach {terminal_colors}    // usa o pacote instalado (hífen vira sublinhado)
```

O manifesto `ofs.claim` e o publicar (`ofs quarry publish`) estão em [TOOLS.md](TOOLS.md#ofs-quarry-pacotes).

---

## 20. Próximos Passos

- Consulte a [Referência da Linguagem](LANGUAGE_REFERENCE.md) para detalhes exaustivos da gramática e dos tipos.
- Consulte a [Referência ODL](ODL_REFERENCE.md) e a [Referência OES](OES_REFERENCE.md) para desenvolvimento web.
- Consulte a documentação da [Biblioteca Padrão](STANDARD_LIBRARY.md).
- Veja todas as ferramentas (`fmt`, `doc`, `refract`, `fossil`, `quarry`, ...) em [TOOLS.md](TOOLS.md), o modelo `ofs run`/`ofs build` em [COMPILED_LANGUAGE.md](COMPILED_LANGUAGE.md) e o app Android em [ANDROID.md](ANDROID.md).
- Os exemplos testados de cada recurso estão em `ofs/tests/regress/` e `ofs/examples/`.
