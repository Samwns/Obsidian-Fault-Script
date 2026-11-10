# Obsidian Fault Script — Referência da Linguagem

Referência prática da linguagem suportada hoje pelo compilador OFS.

---

## Índice

1. Estrutura de programa
2. Tipos
3. Variáveis e constantes
4. Funções e intents
5. `monolith`, `impl` e layouts
6. `namespace`
7. Arrays e coleções
8. Controle de fluxo
9. Operadores e casts
10. `attach`
11. `extern vein` e `rift vein`
12. Blocos de baixo nível
13. Biblioteca padrão e pacotes
14. Comandos do compilador

---

## 1. Estrutura de programa

Todo programa executável precisa de um `core main()`:

```ofs
core main() {
    echo("inicio")
}
```

`core` é palavra reservada: só marca o ponto de entrada. Usá-la como nome (`vein core()`, `core(1)`, inclusive dentro de `namespace`) é **erro de compilação** ("`core` e palavra reservada…"); renomeie a função (ex.: `core_fn`). Qualquer erro sintático encerra a compilação com código 1.

Um arquivo `.ofs` pode conter:

- `attach { ... }`
- `extern vein` e `rift vein`
- `vein` e `core`
- `monolith`
- `impl`
- `namespace`
- `strata`
- `forge` global

---

### Comentários

```ofs
// Comentário de uma linha

/* Comentário
   de várias linhas */

forge x = 42  // comentário no fim da linha
```

### Terminação de instruções

Instruções terminam com **quebra de linha** ou **ponto e vírgula**. Quebras de linha dentro de `()`, `[]` e `{}` são ignoradas (continuação implícita).

```ofs
forge x = 10; forge y = 20

forge resultado = soma(
    10,
    20
)
```

### Palavras reservadas

| Palavra | Token | Finalidade |
|---|---|---|
| `core` | `KW_CORE` | Ponto de entrada |
| `vein` | `KW_VEIN` | Definição de função |
| `forge` | `KW_FORGE` | Declaração de variável |
| `monolith` | `KW_MONOLITH` | Definição de estrutura |
| `fracture` | `KW_FRACTURE` | Bloco de ponteiros seguro |
| `abyss` | `KW_ABYSS` | Bloco de memória insegura |
| `cycle` | `KW_CYCLE` | Laço |
| `if` | `KW_IF` | Condicional |
| `else` | `KW_ELSE` | Ramo alternativo |
| `return` | `KW_RETURN` | Retorna de função |
| `echo` | `KW_ECHO` | Impressão (embutida) |
| `in` | `KW_IN` | Laço por intervalo |
| `shard` | `KW_SHARD` | Tipo ponteiro |
| `true` | `KW_TRUE` | Booleano verdadeiro |
| `false` | `KW_FALSE` | Booleano falso |
| `null` | `KW_NULL` | Valor nulo |
| `break` | `KW_BREAK` | Sai do laço |
| `continue` | `KW_CONTINUE` | Pula para a próxima iteração |
| `stone` | `KW_STONE` | Tipo inteiro (i64) |
| `crystal` | `KW_CRYSTAL` | Tipo ponto flutuante (f64) |
| `obsidian` | `KW_OBSIDIAN` | Tipo string (i8*) |
| `bool` | `KW_BOOL` | Tipo booleano (i1) |
| `void` | `KW_VOID` | Tipo vazio |
| `attach` | `KW_IMPORT` | Anexa módulo |
| `while` | `KW_WHILE` | Laço só com condição |
| `extern` | `KW_EXTERN` | Função nativa do Rift (biblioteca do sistema) |
| `rift` | `KW_RIFT` | Declaração de função nativa pelo Rift |
| `as` | `KW_AS` | Conversão de tipo |
| `match` | `KW_MATCH` | Casamento de padrão |
| `case` | `KW_CASE` | Braço do `match` |
| `default` | `KW_DEFAULT` | Braço padrão do `match` |
| `const` | `KW_CONST` | Variável imutável |
| `strata` | `KW_STRATA` | Enumeração |
| `tremor` | `KW_TREMOR` | Bloco de tratamento de erro |
| `catch` | `KW_CATCH` | Tratador de erro |
| `throw` | `KW_THROW` | Lança valor de erro |
| `tectonic` | `KW_TECTONIC` | Prefixo de diretiva de modo |
| `bedrock` | `KW_BEDROCK` | Bloco de baixo nível tipado |
| `anchor` / `seal` | `TOK_ANCHOR` | Ação que roda ao sair do escopo (os dois nomes são o mesmo token) |
| `asm` | `KW_ASM` | Escape de assembly inline (**planejado**; ainda não reconhecido pelo lexer) |

---

## 2. Tipos

### Tipos primitivos

| Tipo | Uso | Representação geral |
|---|---|---|
| `stone` | inteiro principal | 64 bits |
| `crystal` | ponto flutuante | 64 bits |
| `obsidian` | string | ponteiro |
| `bool` | booleano | lógico |
| `void` | sem retorno | retorno de função |

### Tipos inteiros pequenos

| Tipo | Sinal | Tamanho |
|---|---|---|
| `u8` | sem sinal | 8 bits |
| `u16` | sem sinal | 16 bits |
| `u32` | sem sinal | 32 bits |
| `u64` | sem sinal | 64 bits |
| `i8` | com sinal | 8 bits |
| `i32` | com sinal | 32 bits |
| `i16` | com sinal | 16 bits |
| `i64` | com sinal | 64 bits |

Exemplo:

```ofs
forge r: u8 = 255
forge g: u8 = 128
forge pixel: u32 = (r as u32 << 16) | (g as u32 << 8)
```

### Tipos compostos

| Tipo | Exemplo |
|---|---|
| `Array<T>` | `Array<stone>` |
| tipo de função | `vein(stone) -> stone` |
| `monolith` | `monolith Player { ... }` |

### Inferência

```ofs
forge n = 42
forge nome = "Ana"
forge ativo = true
forge nums = [1, 2, 3]
```

### Conversões implícitas e operações (regra única)

O verificador de tipos (`typeck`) calcula o tipo final de cada expressão e escreve as conversões implícitas na própria árvore do programa. O código nativo (`ofs build`) e o interpretador (`ofs run`) só executam essas conversões, então o mesmo programa dá o mesmo resultado nos dois modos.

**Tipo de cada expressão**

| Expressão | Tipo |
|---|---|
| `a[i]` com `a: Array<T>` | `T` (sem `T` declarado: `stone`) |
| `p.campo` | o tipo declarado do campo |
| `f(...)`, `x.metodo(...)` | o retorno declarado da `vein` |
| `ofs_array_get(a, i)` / `ofs_array_pop(a)` | o elemento de `Array<T>` |
| `cycle (x in a)` com `a: Array<T>` escalar | `x: T` |
| `a op b` inteiro com inteiro | `stone` |

**Conversões implícitas (aceitas)**

| Onde | De → para | Como |
|---|---|---|
| aritmética e comparação mistas | inteiro → `crystal` | `c + 1` vale `c + 1.0`; `2 * c` é `crystal` |
| aritmética de inteiros | `u8`…`i64` → `stone` | a conta é feita em 64 bits (`iN` estende com sinal, `uN` com zero) e o resultado é `stone` |
| `forge x: T = v`, `x = v`, argumento, `return`, elemento de `Array<T>` | inteiro → `crystal` | `forge c: crystal = 1` vale `1.0` |
| idem | `crystal` → inteiro | trunca em direção a zero, igual a `as` |
| idem | `bool` → inteiro | `true` → 1, `false` → 0 |
| idem | inteiro → inteiro de outra largura | trunca ou estende ao guardar (`forge u: u8 = 300` vale 44) |
| comparação com número | `bool`/variante → `stone` | `ok == 1`; `Azul == 2` compara a etiqueta |
| `stone == null` | `null` → 0 | endereço guardado em `stone` |
| `"texto" + v` | `stone`, `crystal`, `bool`, variante → `obsidian` | `"c=" + 1.5` dá `c=1.5`; `"ok=" + true` dá `ok=true`; variante dá o nome |
| `x op= v` | igual a `x = x op v` | `c += 1` com `c: crystal` soma `1.0` |

**Combinações sem sentido: erro de compilação**

Saem como `[ERRO DE TIPO arquivo:linha:coluna]` com uma dica, nos dois modos, antes de executar qualquer coisa:

| Exemplo | Por quê |
|---|---|
| `p + 1`, `p * 2` com `p: *u8` | aritmética de ponteiro: use `fault_step(p, n)` ou `(p as stone) + n` |
| `s + b` com `b: bool` | `bool` não é número: `(b as stone)` |
| `nome * c`, `nome - 1` com `nome: obsidian` | `obsidian` só aceita `+` (concatenação) |
| `c % 2` com `c: crystal` | `%` só vale para inteiros |
| `c & 1`, `b \| b` | bits só existem em inteiros (para `bool` use `&&`/`\|\|`) |
| `"a" < "b"`, `p < q` (ponteiros) | `<`, `<=`, `>`, `>=` só valem para números |
| `s == 0` com `s: obsidian` | comparação entre tipos diferentes: converta com `as` |
| `-texto`, `~c` | sinal só em números, `~` só em inteiros |
| `s += 0.5` com `s: stone` | escreva `s = (s + 0.5) as stone` |

**Avisos (não barram o programa)**

`[AVISO DE TIPO]` aparece quando um valor é guardado num tipo sem conversão definida, por exemplo `forge x: stone = "abc"` ou `forge c: crystal = true`. Nesses casos o nativo reinterpreta os bits (ou o endereço) e o interpretador guarda o valor como está. Isso continua aviso porque código de baixo nível às vezes quer exatamente essa reinterpretação: use `as T` para deixar a intenção explícita.

Quando o `typeck` não sabe o tipo de um lado (chamada sem assinatura conhecida, por exemplo), ele não converte nem acusa erro.

**Variantes no `echo`**: `echo(v)` de uma variante (tipo `strata` ou literal `Verde`) imprime o **nome** nos dois modos. A etiqueta numérica sai com `v as stone`.

---

## 3. Variáveis e constantes

### `forge`

```ofs
forge nome = "Rex"
forge idade: stone = 20
forge saldo: crystal = 42.5
forge canal: u8 = 255
```

### `const`

```ofs
const limite: stone = 10
```

### Atribuições comuns

```ofs
x = 10
x += 2
x -= 1
x *= 3
x /= 2
x++
x--
```

---

## 4. Funções e intents

### Funções normais

```ofs
vein soma(a: stone, b: stone) -> stone {
    return a + b
}
```

### Entrada do programa

```ofs
core main() {
    echo("ok")
}
```

### Intent

```ofs
vein square(x: stone) -> stone intent pure {
    return x * x
}
```

Intents aceitos:

- `pure`
- `impure`
- `fractal`

### Composição de funções

Funções podem invocar outras funções e suportam recursão direta:

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

> **Nota**: Expressões anônimas de função (`vein(...) -> ...` inline) estão especificadas na árvore sintática (`NK_LAMBDA`) e planejadas para o roadmap de evolução da linguagem. Atualmente, todas as funções devem ser declaradas com nome via `vein`.

---

## 5. `monolith`, `impl` e layouts

### Estrutura

```ofs
monolith Rect {
    w: stone
    h: stone
}
```

Um campo pode ser outro `monolith`. O campo guarda uma **referência** ao sub-objeto (8 bytes no layout do sistema); `forge c: Corpo` já cria o sub-objeto zerado, então `c.pos.x = 1` funciona direto. `c.pos = v` faz o campo apontar para `v`; `forge w: Vec3 = c.pos` copia. Igual no nativo e no `ofs run` (teste: `ofs/tests/regress/monolith_nested.ofs`).

```ofs
monolith Vec3 {
    x: stone
    y: stone
    z: stone
}

monolith Corpo {
    id: stone
    pos: Vec3
    massa: stone
}

core main() {
    forge c: Corpo
    c.pos.z = 3
    echo(c.pos.z)   // 3
}
```

### Métodos com `impl`

```ofs
impl Rect {
    vein area(self: Rect) -> stone {
        return self.w * self.h
    }
}
```

Uso:

```ofs
forge r: Rect
r.w = 10
r.h = 5
echo(r.area())
```

### Layout explícito

> **Planejado:** `layout` é palavra reservada, mas o parser ainda não aceita `layout` em `monolith` (dá `expected '{' after monolith name`). Hoje todo `monolith` usa o layout nativo. A sintaxe abaixo é a prevista.

```ofs
monolith Header layout packed {
    tag: stone
    flags: stone
}
```

Layouts suportados:

- `native`
- `packed`
- `c`

---

## 6. `namespace`

Use namespace para agrupar funções e evitar colisão de nomes.

```ofs
namespace mathx {
    vein square(x: stone) -> stone {
        return x * x
    }
}

core main() {
    echo(mathx.square(4))
}
```

---

## 7. Arrays e coleções

```ofs
forge nums = [1, 2, 3]
echo(nums[0])
```

Exemplo com passagem de função:

```ofs
vein map_array(arr: Array<stone>, fn: vein(stone) -> stone) -> Array<stone> {
    forge result: Array<stone> = []
    cycle (n in arr) {
        ofs_array_push(result, fn(n))
    }
    return result
}
```

---

Indexação é **base zero** e tem **verificação de limites** em tempo de execução:

```ofs
forge nums = [10, 20, 30, 40, 50]
forge primeiro = nums[0]   // 10
nums[2] = 99               // altera elemento
```

Fora do limite (`nums[7]`, `ofs_array_get`/`ofs_array_set`), o programa para com código 1 e a mensagem no stderr, igual nos dois modos:

```
ofs: indice 7 fora do limite do array (tamanho 5)
```

---

## 8. Controle de fluxo

### `if / else`

```ofs
if (x > 10) {
    echo("alto")
} else {
    echo("baixo")
}
```

### `while`

```ofs
while (energia > 0) {
    energia -= 1
}
```

### `cycle`

Forma C-like:

```ofs
cycle (forge i = 0; i < 10; i++) {
    echo(i)
}
```

Forma iterativa:

```ofs
cycle (n in nums) {
    echo(n)
}
```

### Seleção de múltiplos caminhos

A seleção de múltiplos ramos é realizada através de cadeias de `if / else if`:

```ofs
if (code == 200) {
    echo("ok")
} else if (code == 404) {
    echo("nao encontrado")
} else {
    echo("erro")
}
```

Ou com `match` (sinônimo: `assay`), que compara o valor com cada caso usando `==`; `_` é o caso padrão:

```ofs
match (code) {
    200 => { echo("ok") }
    404 => { echo("nao encontrado") }
    _ => { echo("erro") }
}
```

### Tratamento de erro e interrupção (`throw`)

A instrução `throw` exibe uma mensagem de diagnóstico e encerra a execução com código de saída 1:

```ofs
vein dividir(dividendo: stone, divisor: stone) -> stone {
    if (divisor == 0) {
        throw "divisao por zero indefinida"
    }
    return dividendo / divisor
}

core main() {
    echo(dividir(10, 2))
}
```

> **Nota**: A sintaxe `tremor { ... } catch (e: obsidian) { ... }` é reconhecida pela gramática (`NK_TREMOR`), com desbobinamento estruturado de pilha e captura em execução planejados para as camadas de interpretador e evolução do runtime.

### `strata` (enumerações de variantes)

```ofs
strata Status { Idle, Running, Failed }
```

> **Nota**: uma variante vale uma etiqueta numérica (`v as stone`) e pode ser guardada num tipo `strata` (`forge c: Status = Idle`, parâmetros, retorno, `Array<Status>`). `echo(c)` imprime o nome (`Idle`) nos dois modos. O payload de `Ok(7)` ainda não é guardado no nativo (ver Propagação `?`).

### Propagação `?`

`expr?` desembrulha uma variante de resultado:

- `Solid(v)?` e `Ore(v)?` valem `v`.
- `Crack(e)?` e `Barren?` saem da `vein` atual na hora (os `anchor` pendentes rodam), propagando a falha. As variantes de falha são as de nome `Crack` e `Barren`; qualquer outra conta como sucesso.

O valor devolvido pela propagação é igual nos dois modos:

- No build nativo variantes são etiquetas `i64` sem payload: a etiqueta é a posição da variante na ordem de declaração de todas as `strata` do programa (com `strata Outcome { Solid, Crack }` declarada primeiro, `Crack` = 1). Uma `vein ... -> stone` que propaga `Crack` devolve essa etiqueta, e `echo` imprime `1` nos dois modos. Com retorno `obsidian` vem o texto da etiqueta; com `crystal`, o número; em `core main` vira o código de saída.
- No interpretador, se o retorno declarado for o próprio tipo `strata` (`-> Outcome`), a variante é devolvida inteira (com payload).
- Como o nativo não guarda payload, `x?` sobre uma expressão que não é um construtor direto (ex.: `f()?`) vale a etiqueta da variante de sucesso, não o payload.

```ofs
strata Outcome { Solid, Crack }

vein ok() -> stone {
    forge v = Solid(7)?      // 7
    return v
}

vein bad() -> stone {
    forge v = Crack("boom")? // sai aqui devolvendo 1
    return v
}

core main() {
    echo(ok())   // 7
    echo(bad())  // 1
}
```

Teste: `ofs/tests/regress/question_propagation.ofs`.


### `break` e `continue`

```ofs
cycle (forge i = 0; i < 100; i++) {
    if (i == 50) {
        break       // sai do laço
    }
    if (i % 2 == 0) {
        continue    // pula números pares
    }
    echo(i)
}
```

`break` e `continue` funcionam em `cycle` e em `while`. `while` é açúcar sintático para um `cycle` só com condição.

### `seal` e `anchor` (rodar ao sair do escopo)

`seal { … }` e `anchor { … }` são **a mesma coisa** (o mesmo token): registram uma ação que roda quando o escopo atual termina, em qualquer saída (fim do bloco, `return`, `break`/`shatter`, `continue`/`erode`, propagação `?`). Várias ações rodam em ordem inversa (a última registrada roda primeiro). `seal` é o nome temático ("sela o recurso ao sair"); `anchor` continua valendo. Aceitam um bloco ou um comando só.

```ofs
vein copiar(origem: obsidian) -> stone {
    forge f = fopen(origem, "r")
    seal { fclose(f) }          // roda em qualquer return abaixo
    anchor echo("fim de copiar") // igual a seal; roda antes do fclose (LIFO)
    if (f == null) { return 1 }
    // ...
    return 0
}
```

- A ação é avaliada **na saída**, com os valores daquele momento.
- `return`, `break`, `continue` e `throw` **dentro** de `seal`/`anchor` são proibidos (erro `E0042`).
- Funciona igual no nativo e no `ofs run`. Teste: `ofs/tests/regress/seal_defer.ofs`.

---

## 9. Operadores e casts

### Operadores principais

- aritméticos: `+`, `-`, `*`, `/`, `%`
- comparação: `==`, `!=`, `<`, `<=`, `>`, `>=`
- lógicos: `&&`, `||`, `!`
- bitwise: `&`, `|`, `^`, `<<`, `>>`

### Cast com `as`

```ofs
forge r: u8 = 255
forge x: u32 = r as u32
forge y: stone = x as stone
forge z: u8 = y as u8
```

Também funciona em conversões entre inteiro e float:

```ofs
forge c: crystal = 10.5
forge n: u8 = c as u8
```


### Precedência de operadores (do menor para o maior)

| Nível | Operadores | Associatividade |
|---|---|---|
| 1 | `\|\|` | Esquerda |
| 2 | `&&` | Esquerda |
| 3 | `==` `!=` | Esquerda |
| 4 | `<` `<=` `>` `>=` | Esquerda |
| 5 | `\|` | Esquerda |
| 6 | `^` | Esquerda |
| 7 | `&` | Esquerda |
| 8 | `<<` `>>` | Esquerda |
| 9 | `+` `-` | Esquerda |
| 10 | `*` `/` `%` | Esquerda |
| 11 | Unários: `-` `!` `&` `*` `++` `--` `~` | Direita |

### Casts permitidos

| De | Para | Resultado |
|---|---|---|
| `stone` | `crystal` | Inteiro para float |
| `crystal` | `stone` | Float para inteiro (trunca) |
| `bool` | `stone` | `true` → 1, `false` → 0 |
| `stone` | `bool` | 0 → `false`, diferente de zero → `true` |
| `bool` | `crystal` | `true` → 1.0, `false` → 0.0 |
| `crystal` | `bool` | 0.0 → `false`, diferente de zero → `true` |

Conversões de string (`obsidian`) exigem funções de runtime (`ofs_stone_to_obsidian` etc.).

---

## 10. `attach`

### Biblioteca padrão ou pacote instalado

```ofs
attach {core}
attach {window}
attach {terminal-colors}
```

### Arquivo local

```ofs
attach {F:modulos/minha_lib.ofs}
```

No Windows também funciona com caminho absoluto:

```ofs
attach {F:G:\projeto\minha_lib.ofs}
```

O `attach` embute o código OFS do outro arquivo no processo de compilação. Ele não carrega DLL nativa.

### Onde o `attach` procura

A primeira opção que existir vence:

| Forma | Ordem de busca |
|---|---|
| `attach {modulo}` | 1. `$OFS_STDLIB_PATH/modulo.ofs` · 2. pasta do arquivo que faz o `attach` · 3. cada pasta de `$OFS_LIB_PATH` (separadas por `:`) · 4. `$OFS_LIB_HOME`, ou `~/.ofs/libs` (onde `src/packages/install_library.sh` instala) |
| `attach {F:caminho}` | 1. caminho absoluto, ou relativo à pasta do arquivo · 2. `$OFS_STDLIB_PATH/caminho` · 3. e 4. como acima |

No `attach {modulo}`, hífen vira sublinhado (`terminal-colors` → `terminal_colors.ofs`). Um módulo que não existe em nenhum desses lugares é **erro de compilação**, com a lista dos caminhos tentados:

```
[ERRO DE ATTACH app.ofs:2]: modulo 'graficos' nao encontrado
  procurado em (nesta ordem):
    /usr/lib/ofs/stdlib/graficos.ofs
    ./graficos.ofs
    /home/voce/.ofs/libs/graficos.ofs
```

(Antes, `attach {x}` de um módulo inexistente era ignorado em silêncio.)

### Nomes indefinidos

Usar um nome que não foi declarado (variável, global, constante) é **erro de compilação**, nos dois modos, com o arquivo e a linha de verdade e uma sugestão quando há um nome parecido:

```
[ERRO SEMANTICO calc.ofs:5:12]: identificador indefinido 'contadr'
  voce quis dizer 'contador'?
```

Globais de topo podem ser usados numa `vein` antes da linha que os declara. Um módulo verificado sozinho (`ofs check modulo.ofs`) não enxerga os nomes de quem o anexa: verifique o arquivo principal. Tipos incompatíveis numa operação (`[AVISO DE TIPO]`) ainda são só aviso.

---

## 11. `extern vein` e `rift vein`

O **Rift** (a fenda) é a fronteira por onde o OFS alcança as bibliotecas nativas do sistema (libc, libm, X11, JNI do Android, kernel32). `extern vein`/`rift vein` declaram uma função que mora numa biblioteca nativa. Não há código C no OFS. (A convenção de chamada do sistema é conhecida tecnicamente como ABI C; não envolve código C.)

### `extern vein`

```ofs
extern vein puts(s: obsidian) -> stone
```

### `rift vein`

```ofs
rift vein ofs_window_create(title: obsidian, w: stone, h: stone) -> void abi c
```

### Metadados de link

```ofs
rift vein text_size(s: obsidian) -> stone bind "strlen" abi c
```

### Endereço de função e chamada indireta (`fault_addr`, `fault_call`)

O OFS não tem tipo "ponteiro de função", mas pode obter o endereço de uma `vein` como `stone` e chamar um endereço pela convenção de chamada do sistema. Serve para callbacks de bibliotecas nativas (`qsort`, `ANativeActivity`, `ALooper`, `AChoreographer`) e para tabelas de funções nativas (JNI).

```ofs
rift vein c_qsort(base: *stone, n: stone, size: stone, cmp: stone) -> void bind "qsort" abi c

vein crescente(a: *stone, b: *stone) -> stone {
    forge x = 0
    forge y = 0
    fracture { x = *a; y = *b; }
    if (x < y) { return -1 }
    if (x > y) { return 1 }
    return 0
}

core main() {
    forge buf = ofs_alloc(64)
    // ...preenche 8 valores...
    abyss {
        c_qsort(buf, 8, 8, fault_addr(crescente))
        forge r = fault_call(fault_addr(crescente), buf, fault_step(buf, 1))
    }
}
```

- `fault_addr(nome)` → `stone`: endereço da vein. Aceita `vein` do módulo, `ns.vein` e `extern`/`rift vein` (com `bind`, devolve o símbolo nativo). Em IR: `ptrtoint ptr @simbolo to i64`. Fora de uma vein, é erro semântico.
- `fault_call(endereco, args...)` → `stone`: chamada indireta pela convenção de chamada do sistema (`inttoptr` + `call i64`). Argumentos inteiros, ponteiros e `obsidian`; o retorno é lido como `i64` (funções `void` devolvem lixo, ignore). Sem `float`/`double`.
- A vein usada como callback deve ter parâmetros compatíveis com a convenção de chamada do sistema: `stone` para inteiros (inteiros de 32 bits chegam com o topo indefinido; mascare com `& 4294967295`) e `*stone` para ponteiros.
- No interpretador (`ofs run`), `fault_addr` de `extern`/`rift vein` devolve o endereço nativo e `fault_addr` de uma vein interpretada devolve um trampolim nativo que reentra no interpretador (o `qsort` acima funciona igual). Ver "No interpretador (`ofs run`): o Rift".
- Teste: `ofs/tests/runtime/test_fault_addr_qsort.ofs`.


Campos importantes:

- `bind "..."` define o símbolo nativo real
- `abi c` seleciona a convenção de chamada do sistema
- `...` é aceito em função variádica nativa

### No interpretador (`ofs run`): o Rift

O `ofs run` chama as mesmas funções nativas das bibliotecas do sistema que o build nativo ligaria, sem código C e sem libffi:

- **Resolução no carregamento.** Cada `extern vein`/`rift vein` chamada no programa (ou passada a `fault_addr`) vira um endereço com `dlsym(RTLD_DEFAULT, "símbolo")`, usando o `bind` quando houver. Antes, o interpretador abre a `libm` (o build liga `-lm`) e as bibliotecas de `OFS_FFI_LIBS` (`libfoo.so:/caminho/libbar.so`); se algum símbolo faltar, tenta também `libX11`/`libXft` (o `ofs build` liga as duas quando existem). Símbolo ausente dá o mesmo erro do linker (`undefined reference to 'x'`) antes de rodar, com código de saída 1. Uma `extern` declarada e nunca chamada não precisa existir (como no link).
- **Runtime OFS.** O `ofscc` é linkado com `-rdynamic`, então `ofs_alloc`, `ofs_echo_color` etc. também são achados. As veins definidas em `stdlib/runtime/*.ofs` (anexadas com `attach`) rodam na versão nativa linkada no `ofscc`, como os builtins, quando a assinatura cabe no Rift; `OFS_RUN_RUNTIME=interp` desliga isso.
- **Conversão.** `stone`, `bool`, inteiros (`i8`..`u64`, ajustados à largura e ao sinal), ponteiros e `obsidian` (o `char*` do próprio valor) vão como `i64`; `crystal` vai em registrador FP. O retorno segue o tipo declarado: `char*` vira `obsidian` sem cópia (como no nativo, onde `obsidian` é o ponteiro; `NULL` vira `null`), `*T` vira o endereço (pode voltar à função nativa, ser comparado com `null`, convertido com `as` e desreferenciado), `monolith` o bloco e `Array` o próprio `ofs_array`.
- **Trampolins.** A chamada passa por `__ofs_ffi_dispatch`, gerada pelo `llvmgen` no próprio `ofscc`: um caso de tipo LLVM exato por classe de assinatura (até 4 argumentos com qualquer mistura `stone`/`crystal` e retorno inteiro ou `crystal`; até 12 só inteiros; variádicas com 1 a 3 fixos e até 4 extras). Classe fora disso é erro claro.
- **Variádicas** (`printf`, `snprintf`, `open`...): as mesmas que o `llvmgen` conhece por nome; os extras vão no tipo declarado (ou no tipo do valor), com a convenção correta (`%al` no x86-64).
- `fault_addr(extern)` devolve o endereço nativo e `fault_call(endereco, ...)` chama com os argumentos no tipo do valor (como no nativo). `fault_step(p, i)` soma `i * sizeof(*p)` com o tipo estático de `p`, como o `getelementptr` do nativo.
- **Memória crua (F2).** O interpretador roda nativo, então ponteiro é endereço real. `*p` e `*p = v` leem e escrevem pela largura do tipo apontado (`u8`/`i8`/`u16`/`i16`/`u32`/`i32`/`u64`/`i64`/`stone`/`f32`/`f64`/`crystal`/`bool` (4 bytes)/`obsidian`/`*T`), com o tipo estático do `typeck` (o mesmo que o `llvmgen` usa); inteiros estreitos ficam com sinal estendido (`iN`) ou zeros (`uN`), como no nativo. `p` nulo falha como no nativo (endereço 0). `fault_memcpy`/`fault_memset` usam `memcpy`/`memset` de verdade, de qualquer tamanho.
- **`as`** segue a regra do `llvmgen`: mesmo tipo LLVM só troca o tipo; `ptrtoint`/`inttoptr` (incluindo `obsidian`, que é o próprio `char*`); `fptosi`; `sitofp` de `iN`/`stone` e `uitofp` de `uN` (`(255 as u8) as crystal` dá 255.0); `fpext`/`fptrunc` (`f32` arredonda); `trunc` se a origem é mais larga, senão `sext` de `iN` e `zext` de `uN`/`bool` (`(0 - 1) as i8 as stone` dá -1). `forge x: u8 = 300` e `x = v` num `u8`/`i32`/`f32` truncam como o `store` do nativo.
- **`&x`.** O endereço de uma variável do interpretador é real: no primeiro `&x` a variável passa a morar num slot de memória crua de 8 bytes (a função nativa pode ler e escrever nele: `sscanf(s, "%d", &n)`, `strtol(s, &fim, 10)`, `modf(x, &inteira)`), liberado quando a variável sai de escopo, como a `alloca` do nativo. `&x` de um `monolith` é o próprio bloco. Só existe endereço de variável (o nativo também não tem `&s.campo`/`&a[i]`: use `(&s) as stone + offset`); `&x` de uma variável `Array` é erro claro.
- **`monolith` com layout do sistema.** As instâncias vivem em memória crua com o layout de `llvmgen.struct_layout` (o mesmo do código nativo: campos na ordem, cada um alinhado ao próprio tamanho, até 8; `bool` ocupa 4 bytes; `Array` e `monolith` aninhado ocupam 8). `s` ou `&s` passado à função nativa (`memcpy`, `qsort` de structs...) vê os mesmos bytes do build nativo. Como no nativo, `monolith` passado a uma vein é por referência e `forge y: T = x`/`y = x` copiam. Um campo `Array` guarda o array do interpretador (a função nativa não deve lê-lo).
- **`Array` e strings.** Um `Array` passado à função nativa é o próprio `ofs_array` nativo (cabeçalho `[data, len, cap, elem_size]` + dados) da coluna de valores do interpretador; a função nativa escreve nos elementos no lugar e o comprimento é sincronizado na volta. Elementos de um só tipo (inteiros/ponteiros, `crystal`, `bool`, `obsidian` ou `monolith`); tipos misturados são erro claro. Um `Array` devolvido pela função nativa usa o mesmo bloco, sem cópia. `obsidian` já é o `char*`. Use `ofs_array_data(a)` para passar só os dados (ex.: `qsort`).
- **Callbacks nativos → OFS.** `fault_addr(vein)` de uma vein interpretada liga a vein a um de 16 trampolins nativos (`__ofs_cb_0`..`__ofs_cb_15`, gerados pelo `llvmgen` no `ofscc`); a biblioteca chama o trampolim, que reentra no interpretador, inclusive dentro de uma chamada nativa feita pelo próprio programa (`qsort` dentro de `qsort`, `fault_call` de outro callback). A mesma vein reusa o slot. Até 6 parâmetros inteiros, ponteiros, `obsidian`, `bool` ou `monolith` (por ponteiro) e retorno inteiro/ponteiro. Mais de 16 veins diferentes, mais de 6 parâmetros ou `crystal`/`f32` no callback dão erro claro com código 1 (use `ofs run --native`). `f32` como argumento do Rift também ainda não é suportado.
- `bedrock`, `fracture` e `abyss` não mudam a execução nos dois modos (o nativo gera o mesmo código dentro deles); o `typeck` os exige para as operações de memória crua (ver seção 12, "Regras"). As células e regiões do `bedrock` (stdlib) usam a memória crua acima.
- Testes: `ofs/tests/regress/ffi_*.ofs`, `ptr_widths.ofs`, `tests/parity/cases/` (`ptr_*`, `libc_*`, `monolith_memcpy`, `qsort_fault_addr`, `fault_call_basic`, `bedrock_block`).


### Direção da interoperabilidade

`extern vein` é a superfície atual do Rift, mais adequada a símbolos de runtime e bibliotecas nativas do sistema. O roadmap amplia isso para: fronteiras com objetos e bibliotecas nativas, pontes para outros ecossistemas, embedding de Python e escapes de assembly específicos do destino. Esses itens são roadmap, não recursos implementados hoje.

### Padrão híbrido

É válido e recomendado misturar alto e baixo nível no mesmo arquivo:

- lógica de negócio em `vein` comuns;
- chamada de bibliotecas nativas pelo Rift (`rift vein`);
- trabalho próximo à máquina em `bedrock`;
- armazenamento explícito com `bedrock.ofs`.

---

## 12. Blocos de baixo nível

### `fracture`

Camada de baixo nível mais tipada.

### `abyss`

Camada crua e irrestrita.

### `fractal`

Camada especial ligada a intent/efeitos.

### `bedrock`

Vocabulário OFS para memória, regiões, lanes, packets e intrinsics `fault_*`.

Exemplo:

```ofs
attach {bedrock}

core main() {
    forge region = bedrock_region_new(4)
    bedrock_region_write(region, 0, 10)
    bedrock_region_write(region, 1, 20)
    echo(bedrock_region_read(region, 1))
    bedrock_region_drop(region)
}
```

### Alias `tectonic`

O parser aceita aliases textuais como:

- `tectonic fracture`
- `tectonic abyss`
- `tectonic fractal`
- `tectonic safe`
- `tectonic unsafe`
- `tectonic bedrock`


### Regras de `fracture`, `bedrock`, `tectonic` e `abyss` (F4)

O `typeck` exige um bloco de baixo nível para as operações que mexem em memória crua. A regra vale igual no `ofs build` e no `ofs run` (vem do `typeck`):

| Operação | `fracture` / `bedrock` / `tectonic` | `abyss` |
|---|---|---|
| `*p` (ler) e `*p = v` (escrever) | libera | libera |
| `&x` | libera | libera |
| `as` com ponteiro (`n as *u8`, `p as stone`, `s as *u8`, `p as *T`) | libera | libera |
| `fault_step`, `fault_memcpy`, `fault_memset` | libera | libera |
| `fault_call`, `fault_addr` | — | libera |

- Aritmética de endereço passa por essas operações: `p + 1` com ponteiro já é `[ERRO DE TIPO]`; use `fault_step(p, n)` ou `((p as stone) + n) as *T`, ambos dentro do bloco.
- Continuam livres fora de bloco: chamar `extern`/`rift vein` (mesmo com ponteiro como argumento ou retorno, ex. `forge buf = ofs_alloc(64)`), comparar ponteiro com `null`, guardar e passar ponteiro adiante, e os intrínsecos de bits (`fault_count`, `fault_cut`, `fault_weave`...).
- `tectonic X` vale como `X` (`tectonic abyss { }` = `abyss`); `tectonic { }` sozinho vale como `fracture`. Blocos aninhados somam: dentro de `abyss`, tudo é liberado.
- A vein inteira pode ser marcada: `fracture vein ler(p: *u8) -> stone { return *p }`, `abyss vein chamar(f: stone) -> stone { return fault_call(f) }` (também em `namespace` e `impl`). O corpo vale como o bloco. Código que precisa compilar com um `ofscc` anterior a esta regra (o próprio compilador, por causa da semente, e o stdlib) usa o bloco `fracture { }` dentro da vein, que todo compilador entende.
- Na árvore, o bloco é um `NK_BLOCK` com o nome no `val` (`"fracture"`, `"abyss"`...); o `llvmgen` e o interpretador o executam como um bloco comum (mesmo IR).

**Migração.** Até **2027-01-09**, operação fora de bloco é só aviso, no stderr (no `ofs run` o stdout é do programa), e o programa compila igual:

```
[AVISO DE FALHA app.ofs:12:15]: acesso por ponteiro '*p' (leitura ou escrita) fora de bloco de baixo nivel (libera: fracture, bedrock ou abyss)
  dica: fracture { forge v = *p; *p = v + 1 }  ou marque a vein: 'fracture vein f(...)'
[AVISO DE FALHA] 1 operacao(oes) de baixo nivel fora de bloco: viram erro em 2027-01-09; OFS_STRICT_FAULT=1 ja trata como erro
```

Com `OFS_STRICT_FAULT=1` (o modo dos testes do repositório) a mesma linha vira `[ERRO DE FALHA arquivo:linha:col]` e a compilação para com código 1. A partir de 2027-01-09 o modo estrito passa a ser o padrão. O stdlib, o runtime, o compilador, as ferramentas, os exemplos e os testes já compilam sem nenhum aviso.

```ofs
core main() {
    forge x: stone = 10

    fracture {
        shard p: *stone = &x   // p aponta para x
        *p = 42
    }

    echo(x)  // imprime 42
}
```

Tipos ponteiro: `shard p: *stone` (ponteiro para inteiro, i64*), `shard q: *crystal` (f64*), `shard s: *obsidian` (i8**). Testes: `ofs/tests/regress/fault_blocks.ofs`, `fault_err_*.ofs`, `fault_warn_default.ofs`.

### Intrínsecos `fault_*`

Os de bits valem em qualquer lugar; `fault_step`, `fault_memcpy` e `fault_memset` pedem `fracture`/`bedrock`/`abyss`, e `fault_addr`/`fault_call` pedem `abyss` (tabela acima):

- `fault_count(stone)` → contagem de bits 1 (popcount)
- `fault_fence()` → barreira de memória
- `fault_prefetch(*stone)` → pré-busca de memória para o alvo do ponteiro
- `fault_trap()` → armadilha (trap) da máquina
- `fault_lead(stone)` → zeros à esquerda
- `fault_trail(stone)` → zeros à direita
- `fault_swap(stone)` → troca de bytes
- `fault_spin_left(stone, stone)` / `fault_spin_right(stone, stone)` → rotação de bits
- `fault_step(*stone, stone)` → avanço de ponteiro tipado, em elementos
- `fault_addr(vein)` → endereço da `vein` como `stone` (ver seção 11)
- `fault_call(endereco, args...)` → chamada indireta pela convenção de chamada do sistema (ver seção 11)
- `fault_cut(value, shift, width)` → extrai um campo de bits
- `fault_patch(base, shift, width, insert)` → substitui um campo de bits
- `fault_weave(mask, left, right)` → mistura bits de `left` e `right` conforme `mask`
- `fault_unreachable()` → marca o caminho como inalcançável (gera `llvm.trap` + `unreachable`)
- `fault_memcpy(dst, src, len)` → cópia de memória em bloco (`llvm.memcpy`); `dst` e `src` são ponteiros
- `fault_memset(dst, val, len)` → preenchimento de memória com o byte `val` (`llvm.memset`); `dst` é ponteiro

Semântica exata (igual no build nativo e no interpretador `ofs run`):

- Tudo em 64 bits, complemento de 2: `fault_count(-1)` = 64; `fault_lead(0)` = `fault_trail(0)` = 64; `fault_lead(-1)` = 0.
- `fault_spin_left`/`fault_spin_right`: rotação de 64 bits (`llvm.fshl`/`llvm.fshr`); a distância é tomada módulo 64 (`fault_spin_right(6, 65)` = 3).
- `fault_weave(mask, left, right)` = `(left & mask) | (right & ~mask)`: o bit vem de `left` onde `mask` tem 1 e de `right` onde tem 0.
- `fault_trap()` e `fault_unreachable()` encerram o processo com SIGILL (o interpretador faz o mesmo com `raise(SIGILL)`).
- `fault_fence()`/`fault_prefetch()` não fazem nada no interpretador.
- No interpretador ponteiros são endereços reais: `fault_memcpy`/`fault_memset` copiam/preenchem qualquer tamanho, e `fault_step`, `fault_addr` e `fault_call` funcionam como no nativo (ver "No interpretador (`ofs run`): o Rift").
- Teste: `ofs/tests/regress/fault_intrinsics.ofs`.

Esses intrínsecos cobrem operações que o assembly já tem e dão ao OFS vocabulário próprio, orientado à máquina.

### `asm` inline (planejado)

> **Estado: planejado.** O `asm` inline ainda **não está disponível** no compilador: o lexer não reconhece a palavra `asm` e o backend LLVM não gera `InlineAsm`. Os exemplos abaixo mostram o desenho previsto e **não compilam hoje**. Enquanto isso, use os intrínsecos `fault_*`.

```ofs
bedrock {
    asm "nop"     // planejado
    asm "int3"    // planejado
}
```

Restrições previstas:
- O argumento deve ser um literal de string.
- Operandos de entrada/saída (restrições estilo AT&T) ainda não serão analisáveis na primeira versão; use `fault_*` quando precisar vincular operandos.
- Deve aparecer apenas dentro de `abyss` (F4/F5).

---

## 13. Biblioteca padrão e pacotes

Módulos e pacotes relevantes já presentes no projeto:

- `fs`
- `net`
- `http`
- `odl`
- `oes`
- `oll`
- `canvas`
- `core`
- `fmt`
- `io`
- `math`
- `string`
- `rift`
- `bedrock`
- `bedrock-packet`
- `memory-modes`
- `terminal-colors`
- `test-lib`
- `webserver`
- `window`

Módulo `fs` expõe:

- `fs.exists(path: obsidian) -> bool`
- `fs.size(path: obsidian) -> stone`
- `fs.read(path: obsidian) -> obsidian`
- `fs.write(path: obsidian, content: obsidian) -> bool`
- `fs.append(path: obsidian, content: obsidian) -> bool`

Módulo `net` expõe:

- `net.tcp_connect(host: obsidian, port: stone) -> stone`
- `net.tcp_send(fd: stone, data: obsidian) -> stone`
- `net.tcp_recv(fd: stone, max_len: stone) -> obsidian`
- `net.tcp_close(fd: stone) -> void`
- `net.tcp_listen(port: stone, backlog: stone) -> stone`
- `net.tcp_accept(server_fd: stone) -> stone`

Módulo `http` expõe:

- `http.get(ip: obsidian, port: stone, host: obsidian, path: obsidian) -> HttpResponse`
- `http.parse_status_code(response_text: obsidian) -> stone`
- `http.extract_body(response_text: obsidian) -> obsidian`

Módulo `odl` expõe construtores de HTML: cada função devolve o trecho como `obsidian`, e trechos se aninham passando um como `children` do outro. O HTML é só o artefato gerado; o fonte continua em OFS.

- `odl.attr(name: obsidian, value: obsidian) -> obsidian`
- `odl.empty_attrs() -> obsidian`
- `odl.tag(name: obsidian, attrs: obsidian, children: obsidian) -> obsidian`
- `odl.void_tag(name: obsidian, attrs: obsidian) -> obsidian`
- `odl.text(value: obsidian) -> obsidian`
- `odl.document(lang: obsidian, title: obsidian, description: obsidian, head_extra: obsidian, body: obsidian) -> obsidian`
- `odl.css(href: obsidian) -> obsidian`
- `odl.icon(href: obsidian) -> obsidian`
- `odl.script(src: obsidian) -> obsidian`
- `odl.script_module(src: obsidian) -> obsidian`
- `odl.meta(name: obsidian, content: obsidian) -> obsidian`
- `odl.component(name: obsidian, class_name: obsidian, children: obsidian) -> obsidian`
- `odl.div(class_name: obsidian, children: obsidian) -> obsidian`
- `odl.section(id: obsidian, class_name: obsidian, children: obsidian) -> obsidian`
- `odl.article(class_name: obsidian, children: obsidian) -> obsidian`
- `odl.nav(class_name: obsidian, children: obsidian) -> obsidian`
- `odl.header(class_name: obsidian, children: obsidian) -> obsidian`
- `odl.main(id: obsidian, children: obsidian) -> obsidian`
- `odl.footer(class_name: obsidian, children: obsidian) -> obsidian`
- `odl.a(class_name: obsidian, href: obsidian, children: obsidian) -> obsidian`
- `odl.img(src: obsidian, alt: obsidian) -> obsidian`
- `odl.p(class_name: obsidian, children: obsidian) -> obsidian`
- `odl.h1(children: obsidian) -> obsidian`
- `odl.h2(children: obsidian) -> obsidian`
- `odl.h3(children: obsidian) -> obsidian`
- `odl.strong(children: obsidian) -> obsidian`
- `odl.span(class_name: obsidian, children: obsidian) -> obsidian`
- `odl.ul(class_name: obsidian, children: obsidian) -> obsidian`
- `odl.ol(class_name: obsidian, children: obsidian) -> obsidian`
- `odl.li(children: obsidian) -> obsidian`
- `odl.form(action: obsidian, method: obsidian, children: obsidian) -> obsidian`
- `odl.label(for_id: obsidian, children: obsidian) -> obsidian`
- `odl.input(input_type: obsidian, name: obsidian, value: obsidian) -> obsidian`
- `odl.button(button_type: obsidian, class_name: obsidian, children: obsidian) -> obsidian`
- `odl.code_block(children: obsidian) -> obsidian`
- `odl.table(head: obsidian, body: obsidian) -> obsidian`
- `odl.table_id(id: obsidian, head: obsidian, body: obsidian) -> obsidian`

Módulo `oes` expõe construtores de CSS, no mesmo estilo: cada função devolve o texto da regra ou declaração.

- `oes.prop(name: obsidian, value: obsidian) -> obsidian`
- `oes.rule(selector: obsidian, body: obsidian) -> obsidian`
- `oes.media(query: obsidian, body: obsidian) -> obsidian`
- `oes.supports(query: obsidian, body: obsidian) -> obsidian`
- `oes.layer(name: obsidian, body: obsidian) -> obsidian`
- `oes.font_face(body: obsidian) -> obsidian`
- `oes.keyframes(name: obsidian, frames: obsidian) -> obsidian`
- `oes.frame(step: obsidian, body: obsidian) -> obsidian`
- `oes.var(name: obsidian, value: obsidian) -> obsidian`
- `oes.use(name: obsidian) -> obsidian`
- `oes.transition(property: obsidian, duration: obsidian, easing: obsidian) -> obsidian`
- `oes.animation(name: obsidian, duration: obsidian, easing: obsidian) -> obsidian`

```ofs
attach {odl}
attach {oes}

forge corpo = odl.section("inicio", "hero", odl.h1("OFS") + odl.p("lead", "Uma linguagem só."))
forge estilo = oes.rule(".hero", oes.prop("padding", "2rem") + oes.prop("color", oes.use("texto")))
```

Módulo `oll` expõe:

- `oll.parse(src: obsidian) -> stone`
- `oll.load(path: obsidian) -> stone`
- `oll.node_count() -> stone`
- `oll.get_node(id: stone) -> OllNode`
- `oll.set_node(id: stone, n: OllNode) -> void`
- `oll.find_by_name(name: obsidian) -> stone`
- `oll.compute_layout(root_id: stone) -> void`
- `oll.render(cv: Canvas) -> void`
- `oll.hit_test(x: stone, y: stone) -> stone`
- `oll.click(id: stone) -> obsidian`
- `oll.reset() -> void`

Módulo `canvas` expõe:

- `canvas.create(w: stone, h: stone) -> Canvas`
- `canvas.destroy(cv: Canvas) -> void`
- `canvas.clear(cv: Canvas, color: stone) -> void`
- `canvas.set_pixel(cv: Canvas, x: stone, y: stone, color: stone) -> void`
- `canvas.get_pixel(cv: Canvas, x: stone, y: stone) -> stone`
- `canvas.fill_rect(cv: Canvas, x: stone, y: stone, w: stone, h: stone, color: stone) -> void`
- `canvas.draw_rect(cv: Canvas, x: stone, y: stone, w: stone, h: stone, color: stone) -> void`
- `canvas.draw_char(cv: Canvas, x: stone, y: stone, ch: stone, color: stone) -> void`
- `canvas.draw_text(cv: Canvas, x: stone, y: stone, text: obsidian, color: stone) -> void`
- `canvas.save_ppm(cv: Canvas, path: obsidian) -> bool`
- `canvas.present(cv: Canvas) -> void`

Módulo `window` expõe:

- `window.create`
- `window.destroy`
- `window.present`
- `window.poll`
- `window.is_open`
- `window.width`
- `window.height`
- `window.set_title`
- `input.mouse_x`
- `input.mouse_y`
- `input.is_down`
- `input.key`


### Módulos da biblioteca padrão

| Nome | Descrição |
|---|---|
| `core` | Auxiliares de matemática, predicados, fatorial, Fibonacci |
| `math` | Primos, utilitários de dígitos, inversão de números |
| `string` | Utilitários de string: repeat, starts_with, is_empty |
| `io` | prompt, print_separator, print_header |
| `webserver` | Construtor de resposta HTTP, tipos MIME, auxiliares de log |
| `bedrock` | Células, regiões e auxiliares de campos de bits em heap |
| `bedrock-packet` | Auxiliares de layout de pacote sobre regiões `bedrock` |
| `rift` | Wrappers do Rift (auxiliares de biblioteca nativa) |
| `terminal-colors` | Saída de cores ANSI no terminal |
| `memory-modes` | Documentação e auxiliares dos modos fracture/abyss/fractal |
| `test-lib` | Utilitários de teste unitário |

### Funções matemáticas (`stdlib/core.ofs`)

```ofs
vein abs(x: stone) -> stone
vein max(a: stone, b: stone) -> stone
vein min(a: stone, b: stone) -> stone
vein clamp(val: stone, lo: stone, hi: stone) -> stone
```

### Auxiliares de `bedrock.ofs`

- Células: `bedrock_cell_new`, `bedrock_cell_read`, `bedrock_cell_write`, `bedrock_cell_add`, `bedrock_cell_sub`, `bedrock_cell_drop`.
- Regiões: `bedrock_region_new`, `bedrock_region_read`, `bedrock_region_write`, `bedrock_region_add`, `bedrock_region_drop`, `bedrock_prefetch`.
- Visões: `bedrock_view_at`, `bedrock_view_read`, `bedrock_view_write`.
- Campos e lanes: `bedrock_field_cut`, `bedrock_field_patch`, `bedrock_lane8_get/set`, `bedrock_lane16_le/be_get/set`, `bedrock_lane32_le/be_get/set`.
- Barreira: `bedrock_barrier`.

A finalidade desse módulo não é imitar APIs de outra linguagem, e sim fazer a programação de baixo nível crescer no vocabulário do OFS. Os auxiliares de lanes dão uma superfície com endianness para janelas de bits voltadas a ABI, sem exigir deslocamentos manuais.

### Auxiliares de `rift.ofs` e `bedrock_packet.ofs`

- `rift.ofs`: `rift_text_size`, `rift_line`, `rift_banner`, `rift_report_line`. Permite consumir bibliotecas nativas do sistema por APIs em OFS.
- `bedrock_packet.ofs`: `bedrock_packet_store`, `bedrock_packet_header`, `bedrock_packet_payload`, `bedrock_packet_opcode`, `bedrock_packet_lane`, `bedrock_packet_opcode_be16`, `bedrock_packet_tail_le16`, `bedrock_packet_set_opcode`, `bedrock_packet_set_lane`. Visões de cabeçalho e payload sobre regiões `bedrock`.

### Runtime nativo OFS (Stack Magma)

A implementação principal do runtime e da biblioteca padrão é escrita em OFS (`stdlib/runtime/`), sem runtime em outra linguagem:

| Subsistema | Módulo | Responsabilidade |
|---|---|---|
| **Magma** | `magma.ofs` | Bootstrap e orquestração da execução do runtime nativo |
| **Reservoir** | `reservoir.ofs` | Alocação e gerenciamento de memória (`ofs_alloc`, `ofs_free`) |
| **Facet** | `facet.ofs` | Texto e conversão numérica (`ofs_str_*`, `ofs_*_to_*`) |
| **Matrix** | `matrix.ofs` | Coleções dinâmicas e vetores (`ofs_array_*`) |
| **Outlet** | `outlet.ofs` | Entrada e saída do terminal (`echo_*`, cores ANSI, formatação) |
| **Foundation** | `foundation.ofs` | Sistema operacional e ambiente (`ofs_getenv`, `ofs_mod`, `ofs_exit`) |
| **Pulse** | `pulse.ofs` | Relógio monotônico de alta precisão e timers (`now_ms`, `sleep_ms`) |
| **Frame** | `frame.ofs` | Gerenciamento de janelas e superfícies nativas |
| **Impulse** | `impulse.ofs` | Eventos de entrada: mouse e teclado |
| **Prism** | `prism.ofs` | Rasterização 2D de pixels, geometria e superfícies |

Todas as primitivas de runtime são compiladas pelo `ofscc` diretamente para LLVM IR nativo.

---

## 14. Comandos do compilador

### Compilador principal (`ofs`)

```bash
ofs run arquivo.ofs
ofs build arquivo.ofs -o app
ofs check arquivo.ofs
ofs clean
ofs tokens arquivo.ofs
ofs ast arquivo.ofs
ofs ir arquivo.ofs
ofs asm arquivo.ofs
```

### Compilador self-hosted (`ofscc`) no estado atual

`ofscc` já suporta otimização e modos de inspeção via bridge de ambiente:

Variáveis:

- `OFSCC_INPUT`
- `OFSCC_OUTPUT`
- `OFSCC_C_OUT`
- `OFSCC_OPT` (`-O0`, `-O2`, `-O3`)
- `OFSCC_MODE` (`check`, `tokens`, `ast`)

Exemplo:

```bash
export OFSCC_INPUT=programa.ofs
export OFSCC_OUTPUT=programa
export OFSCC_OPT=-O3
./ofscc_v1
```

```bash
ofs arquivo.ofs
ofs run arquivo.ofs
ofs build arquivo.ofs -o app
ofs check arquivo.ofs
ofs tokens arquivo.ofs
ofs ast arquivo.ofs
ofs ir arquivo.ofs
ofs asm arquivo.ofs
ofs version
ofs update
```

Pacotes:

```bash
uncover fmt
infuse fmt
infuse {fmt:stable}
reinfuse fmt
```
