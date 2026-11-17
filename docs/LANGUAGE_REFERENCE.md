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
| `bedrock` | `KW_BEDROCK` | Bloco de baixo nível tipado; quando aloca, é uma arena (seção 12) |
| `anchor` / `seal` | `TOK_ANCHOR` | Ação que roda ao sair do escopo (os dois nomes são o mesmo token) |
| `quartz` | `TOK_QUARTZ` | Assembly inline, somente em `abyss` |

Palavras de contexto (não reservadas; valem como nome fora daí): `mineral`, `grain`, `bearing`, `settle`, `value` e `result` (contratos, seção 4) e `geode` (função que captura, seção 4: só quando vem `geode (...)` seguido de `->` ou `{`).

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
| nome de uma `vein`/`extern` usado como valor (`forge f = dobro`) | `vein(T1, T2) -> R`, a assinatura dela (seção 4) |
| `f(...)` com `f: vein(...) -> R` | `R` |

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
| `forge f: vein(stone) -> stone = nome` com `nome(x: stone) -> obsidian` | a assinatura precisa ser igual (parâmetros e retorno) |
| `f(1, 2)` ou `f("dez")` com `f: vein(stone) -> stone` | aridade e tipo de argumento conferidos na chamada de valor de vein |
| `f + 1`, `f < g` com `f: vein(...)` | valor de vein só aceita `==`/`!=` (mesma assinatura ou `null`) e a chamada |

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

**Valor padrão de parâmetro:** `vein escala(x: stone, k: stone = 2) -> stone`. O padrão é um literal (número, texto, `bool`, `null` ou `-número`) e os parâmetros com padrão ficam no fim (`[ERRO SEMANTICO]` se não). A chamada que omite os últimos argumentos recebe o padrão (`escala(5)` = `escala(5, 2)`), igual no nativo e no interpretador; vale também para métodos de `impl`. Um tipo de função `vein(...)` não guarda padrões: a chamada por valor de vein passa todos os argumentos. Na Fault Refract, acrescentar um parâmetro com padrão a uma vein em uso é uma troca aceita (as chamadas antigas recebem o padrão).

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

### Tipos de função e valores de vein

Uma `vein` declarada é também um **valor**: o nome dela (sem chamar) tem o tipo de função `vein(T1, T2) -> R`, escrito com a mesma palavra que declara funções. Sem `-> R`, o retorno é `void`. O tipo vale em variável, parâmetro, retorno, campo de `monolith` e `Array<...>`, e não precisa de bloco de baixo nível: é seguro e tipado.

```ofs
monolith Operacao {
    nome: obsidian
    calc: vein(stone, stone) -> stone
}

vein dobro(x: stone) -> stone { return x * 2 }
vein triplo(x: stone) -> stone { return x * 3 }
vein soma(a: stone, b: stone) -> stone { return a + b }

// vein como argumento e como retorno
vein aplicar(f: vein(stone) -> stone, x: stone) -> stone { return f(x) }
vein escolher(grande: bool) -> vein(stone) -> stone {
    if (grande) { return triplo }
    return dobro
}

core main() {
    forge f: vein(stone) -> stone = dobro   // ou: forge f = dobro
    echo(f(21))                  // 42
    echo(aplicar(triplo, 4))     // 12
    echo(escolher(true)(10))     // 30

    forge op: Operacao
    op.calc = soma
    echo(op.calc(3, 4))          // 7

    forge ops: Array<vein(stone, stone) -> stone> = [soma]
    echo(ops[0](6, 7))           // 13

    echo(f == dobro)             // true
    echo(f)                      // dobro
}
```

- **Valor:** o nome de uma `vein` do módulo, de `ns.vein`, de um método (`Rect__area`) ou de uma `extern`/`rift vein` (com `bind`, o símbolo nativo) é um valor do tipo da assinatura dela. Uma `extern` variádica (`printf`) não vira valor (erro claro).
- **Chamada:** `f(x)` com `f` variável, parâmetro, global, campo (`s.cb(x)`), elemento (`a[i](x)`) ou resultado (`escolher(true)(10)`) é uma chamada indireta tipada. O `typeck` confere a aridade e os argumentos com as conversões da seção 2 (`f(5)` com parâmetro `crystal` vale `f(5.0)`); um argumento sem conversão definida (`obsidian` onde o tipo pede `stone`) é `[ERRO DE TIPO]`, não aviso. O resultado tem o tipo `R`.
- **Assinatura exata:** guardar uma vein num lugar de tipo de função (`forge`, `=`, argumento, `return`, campo, `push`) exige a mesma assinatura, parâmetro a parâmetro e no retorno; o erro diz o que difere (`retorno: obsidian onde o tipo pede stone`). `null` cabe em qualquer tipo de função e é o valor inicial de variável e campo sem inicializador.
- **Igualdade:** `==` e `!=` entre valores da mesma assinatura ou com `null`. Dois valores da mesma vein são iguais nos dois modos.
- **`echo` e texto:** `echo(f)` e `"x" + f` dão o nome da vein (`dobro`; `ns.f` para `ns__f`), `null` se nulo e `<vein>` para um endereço que veio do C e não tem nome no programa. Igual nos dois modos.
- **Chamar `null`:** erro de execução `ofs: chamada de vein nula (o valor de vein e null)` no stderr e código de saída 1, nos dois modos.
- **Nativo:** o valor é o ponteiro da função (`ptr @dobro`) e a chamada é `call R (T1, T2) %f(...)` com a assinatura LLVM exata do tipo, inclusive `crystal`.
- **Interpretador (`ofs run`):** o valor de uma vein interpretada é a própria vein (chamada direta, sem passar pelo C). Quando o valor vai para o C (argumento de `extern`/`rift vein`), ele vira o trampolim de callback da seção 11; quando vem do C (retorno de `extern`, parâmetro de callback, memória crua), a assinatura do tipo escolhe o trampolim do Rift. Um trampolim que volta do C é reconhecido como a vein de origem.
- **Endereço cru:** `f as stone`, `f as *u8`, `endereco as vein(...)` e `fault_addr(f)` convertem entre valor de vein e endereço; só dentro de `abyss` (seção 12). `fault_call(f, args...)` com `f` de tipo de função confere a assinatura e vale `f(args...)`.
- Na árvore, o tipo é um `NK_TYPE_FN` cujo `val` é o texto canônico sem espaços (`vein(stone,crystal)->stone`), o mesmo `type_str` de toda expressão desse tipo; a chamada indireta é um `NK_CALL` `__vein_call` com a vein em `c0`.
- Testes: `ofs/tests/regress/vein_type_*.ofs`.

### `geode`: vein que captura (closure)

`geode (parâmetros) -> R { corpo }` é uma **expressão**: uma vein sem nome que lê as variáveis do escopo de fora. O valor tem o tipo de função `vein(T...) -> R` acima e se usa igual a qualquer valor de vein: guardar em variável, campo ou `Array`, passar, devolver e chamar. Sem `-> R`, o retorno é `void`. O nome vem do geodo, a pedra oca que guarda o que capturou.

```ofs
vein somador(base: stone) -> vein(stone) -> stone {
    return geode (x: stone) -> stone { return x + base }
}

core main() {
    forge k = 10
    forge mais_k = geode (x: stone) -> stone { return x + k }
    k = 100
    echo(mais_k(5))            // 15: k foi copiada quando o geode nasceu
    forge s7 = somador(7)
    echo(s7(1))                // 8
    echo(mais_k)               // <geode>
}
```

- **Captura por valor.** Toda variável local (ou parâmetro) de fora que o corpo lê vira uma captura, copiada no momento em que o geode é criado; mudar a variável depois não muda o geode. Globais não são capturados (o geode lê o global atual, como uma vein). Geode dentro de geode captura em cadeia.
- **O que a cópia significa:** a mesma coisa que passar a variável como argumento de uma vein. Números, `bool` e etiquetas de `strata` são copiados; `obsidian` é um valor imutável (a cópia é o texto); `Array` e `monolith` são o mesmo objeto (o geode vê os `push` e campos alterados depois, e o que ele muda nos elementos/campos aparece fora). Para um `monolith` independente, copie antes (`forge c: P = m`) e capture `c`.
- **Atribuir a uma captura** dentro do geode (`n = n + 1`, `n++`, `&n`) é `[ERRO DE GEODE]`: a captura é uma cópia, e a atribuição não mudaria a variável de fora. Use uma local (`forge v = n`) ou devolva o valor. `shatter`/`erode` no corpo sem laço do próprio geode também é erro (o corpo é outra vein).
- **Igualdade e texto:** cada criação de um geode com captura é um valor distinto (`==` só é `true` com ele mesmo); um geode sem captura é sempre o mesmo valor. `echo(g)` e `"x" + g` dão `<geode>` nos dois modos.
- **Rift:** um geode **sem captura** é uma vein comum e vai para o sistema (Rift) como callback (`qsort(v, n, 8, geode (a: *stone, b: *stone) -> i32 { ... })`). Um geode **com captura** não tem como ir para o C (o C chama só o endereço de uma função): passar um geode literal com captura a uma `extern` é `[ERRO DE GEODE]` em compilação; se ele chegar ao C por uma variável, ou virar endereço (`as stone`, `fault_addr`), o programa para com `ofs: geode com captura nao pode ir para o C (Rift) nem virar endereco; ...` no stderr e código 1, nos dois modos.
- **Como funciona:** o `typeck` acha as capturas e tira o corpo para uma vein `__geode_<vein>_<k>(capturas..., parâmetros...)` no fim do programa. No nativo, sem captura o valor é `ptr @__geode_...`; com captura é um ambiente no heap `[adaptador, marca, captura 0, ...]` (8 bytes cada) e o valor é o endereço com o bit 0 ligado; a chamada indireta testa o bit (só nos módulos que têm geode com captura, então o IR dos outros programas não muda) e chama `adaptador(ambiente, args)`. No interpretador o valor é a vein do corpo com um ambiente (array interpretado com as capturas). O ambiente vive até o fim do programa.
- **Cristalização e Fault Refract:** num programa com geode, a vein que mexe com valores de vein (ou chama quem mexe) fica interpretada; o corpo de um geode é uma vein comum e pode cristalizar. No `ofs refract`, editar o corpo de um geode (mesmas capturas e parâmetros) troca ao vivo, inclusive nos geodes já criados; se as capturas mudam, os geodes já criados seguem com o corpo antigo e os criados depois usam o novo.
- Testes: `ofs/tests/regress/geode_*.ofs`, caso `geode` em `ofs/tests/refract/run.sh`.


### Contratos e tipos com regra (`mineral`, `grain`, `bearing`, `settle`)

Um **`mineral`** é um tipo com regra: um tipo base mais uma ou mais **`grain`** (na regra, `value` é o valor conferido). Uma vein pode ter **`bearing`** (pré-condição: quem chama garante) e **`settle`** (pós-condição: vale para o retorno; `result` é o valor devolvido), quantos quiser, entre a assinatura e o corpo.

```ofs
mineral UserId = stone grain value > 0
mineral Email = obsidian grain contains("@")
mineral Nota = stone grain value >= 0 grain value <= 10

vein dividir(a: stone, b: stone) -> stone
    bearing b != 0
    settle result * b + a % b == a
{
    return a / b
}
```

- **Compatibilidade:** o mineral é o tipo base para o typeck, o llvmgen e o interpretador. Base → mineral confere a regra (`forge x: UserId = e`, `x = e`, `x += e`, `x++`, campo `obj.f = e` de monolith com campo mineral, argumento de parâmetro mineral, `return` de vein `-> UserId`); mineral → base é livre. `Array<UserId>`, `*UserId`, `as UserId` e `vein(UserId)->T` valem como o base, sem checagem.
- **Mineral de mineral:** `mineral Pequeno = UserId grain value < 100` soma as regras (as do base primeiro).
- **Forma curta da `grain`:** uma chamada sem `value` nos argumentos recebe `value` como 1º argumento; `contains`/`len` sem vein com esse nome são as de texto: `contains("@")` = `ofs_str_contains(value, "@")`, `len() >= 2` = `ofs_str_len(value) >= 2`.
- **`result`** só vale no `settle` (no `bearing` é erro). No `settle`, os parâmetros valem como na entrada, mesmo que o corpo os altere. Numa vein sem retorno, o `settle` é conferido em cada `return` e ao cair no fim.
- **`mineral`, `grain`, `bearing`, `settle`, `value` e `result` são palavras de contexto**, não reservadas: continuam valendo como nomes fora dessas posições.

**Quando a regra é conferida**

| Situação | Checagem |
|---|---|
| Violação comprovada em compilação: literal, constante (`const`) ou conta com eles (`forge e: Email = "x"`, `dividir(1, 0)`, `return -1` em `-> UserId`, `settle` que só depende de `result`) | `[ERRO DE CONTRATO arquivo:linha:col]` com a regra e os valores, nos dois modos; vale também no release |
| Em execução (`ofs run` e `ofs build` padrão) | `ofs: [CONTRATO VIOLADO arquivo:linha:col]: bearing de 'dividir' falhou: b != 0 (b = 0)` no stderr e **código de saída 1**; o mesmo texto nos dois modos |
| Release: `ofs build --release` ou `OFS_CONTRACTS=0` | sem checagem em execução (a estática fica); o IR não tem as checagens |

Valor provado correto em compilação não ganha checagem de execução. A falha de contrato não é um `throw`: `tremor`/`catch` não a pega. Uma saída por `?` (propagação) não confere o `settle`.

**Casos de borda: `ofs assay arquivo.ofs`.** Para cada vein com contrato e só parâmetros inteiros (`stone`, `iN`, `uN` ou mineral de base inteira), gera casos nos limites das regras: cada comparação de um parâmetro com uma constante (`b != 0`, `n <= 10`, grain `value > 0`) dá o valor da fronteira e ±1; parâmetro sem limite usa -1, 0 e 1 (até 64 casos por vein). Os casos fora do `bearing`/da `grain` só são listados (`--`); os outros são chamados (`ok`) e o `settle` confere o resultado. A primeira violação para com a mensagem do contrato e código 1. Regra que depende de algo além de constantes e parâmetros deixa a vein de fora (`pulada`). Veins de `impl`, parâmetros não inteiros e intervalos de laço ficam para depois.

```
$ ofs assay metade.ofs
assay metade(n: stone)
  --   metade(-1): fora do bearing n >= 0
  ok   metade(0)
ofs: [CONTRATO VIOLADO metade.ofs:3:5]: settle de 'metade' falhou: result * 2 == n (result = 0, n = 1)
```

**Fault Refract:** alterar um `bearing`/`settle` é editar a vein (ela é trocada ao vivo); alterar a `grain` de um mineral usado numa assinatura é mudança de tipo (troca recusada se a vein estiver em uso).

Testes: `ofs/tests/regress/contract_*.ofs`. Pendente: checagem de mineral em `Array<Mineral>`, em `as` e em globais com valor não literal; intervalos de laço na checagem estática; `ofs assay` com parâmetros não inteiros e sem parar na primeira violação.

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

A checagem vale no `ofs run`, no `ofs build` padrão e no `ofs build --release`: o caminho rápido inline dos arrays (A6) já compara o índice com o tamanho antes do acesso direto e cai no runtime fora do limite. Custo medido (A5.4, `bench/`): nenhuma diferença perceptível nos 6 programas do benchmark (sort 14 × 13 ms); num laço que só lê e grava `a[k]` (100 milhões de acessos) 137 × 126 ms (~9%). Por isso o release mantém a checagem (o IR do release não muda).

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
| `attach {modulo}` | 1. `$OFS_STDLIB_PATH/modulo.ofs` · 2. pasta do arquivo que faz o `attach` · 3. cada pasta de `$OFS_LIB_PATH` (separadas por `:`; no Windows por `;`, como o `PATH`, porque `:` aparece na letra da unidade `C:\libs`) · 4. `$OFS_LIB_HOME`, ou `~/.ofs/libs` (onde `src/packages/install_library.sh` instala) |
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

Um erro de sintaxe dentro de um arquivo anexado diz o arquivo e a linha dele: `[ERRO SINTATICO lib/util.ofs:4:11]: expected ')'` (antes saía só `4:11`, a linha do fonte já expandido).

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

O jeito tipado de passar uma vein ao C é o tipo de função (seção 4): declare o parâmetro da `rift vein` com a assinatura e passe o nome da vein, sem `abyss`:

```ofs
rift vein c_qsort(base: *stone, n: stone, size: stone, cmp: vein(*stone, *stone) -> stone) -> void bind "qsort" abi c

core main() {
    forge v: Array<stone> = [42, 7, 19]
    c_qsort(ofs_array_data(v), ofs_array_len(v), 8, crescente)   // crescente: abaixo
}
```

O caso cru continua existindo: obter o endereço de uma `vein` como `stone` e chamar um endereço pela convenção de chamada do sistema. Serve para tabelas de funções nativas (JNI) e para código antigo com o parâmetro declarado como `stone` (`ANativeActivity`, `ALooper`, `AChoreographer`).

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

- `fault_addr(nome)` → `stone`: endereço da vein. Aceita `vein` do módulo, `ns.vein` e `extern`/`rift vein` (com `bind`, devolve o símbolo nativo). Em IR: `ptrtoint ptr @simbolo to i64`. `fault_addr(f)` com `f` valor de vein vale `f as stone`. Outra coisa é erro semântico.
- `fault_call(endereco, args...)` → `stone`: chamada indireta pela convenção de chamada do sistema (`inttoptr` + `call i64`). Argumentos inteiros, ponteiros e `obsidian`; o retorno é lido como `i64` (funções `void` devolvem lixo, ignore). Sem `float`/`double`. Quando o primeiro argumento tem tipo de função (`fault_call(f, a, b)` com `f: vein(...)`), a assinatura é conferida (aridade e argumentos, como em `f(a, b)`) e a chamada é a tipada, com o retorno `R`.
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
- **Callbacks nativos → OFS.** `fault_addr(vein)` de uma vein interpretada liga a vein a um de 16 trampolins nativos (`__ofs_cb_0`..`__ofs_cb_15`, gerados pelo `llvmgen` no `ofscc`); a biblioteca chama o trampolim, que reentra no interpretador, inclusive dentro de uma chamada nativa feita pelo próprio programa (`qsort` dentro de `qsort`, `fault_call` de outro callback). A mesma vein reusa o slot. Até 6 parâmetros inteiros, ponteiros, `obsidian`, `bool` ou `monolith` (por ponteiro) e retorno inteiro/ponteiro. Callback com `crystal`/`f32` (parâmetro ou retorno) usa outra tabela, `__ofs_cbx_0`..`__ofs_cbx_15`, que recebe também os registradores de ponto flutuante (até 6 inteiros e 8 `crystal`/`f32`; onda 4). Mais de 16 veins diferentes ou parâmetros além desses limites dão erro claro com código 1 (use `ofs run --native`); no Windows x64 (registradores por posição) a callback com `crystal`/`f32` continua com erro claro. O mesmo trampolim serve para um valor de vein passado num parâmetro `vein(...)` (seção 4), e a cristalização usa os dois para chamar de volta veins interpretadas. `f32` no Rift (parâmetro e retorno, como `sqrtf`/`powf`) funciona nos dois modos.
- `fracture` e `abyss` não mudam a execução nos dois modos (o nativo gera o mesmo código dentro deles); o `typeck` os exige para as operações de memória crua (ver seção 12, "Regras"). Um `bedrock` que aloca é também uma arena (seção 12); no interpretador, o `ofs_alloc` escrito no bloco vem da arena do runtime e o que pode sair dela é decidido em compilação, igual ao nativo. As células e regiões do `bedrock` (stdlib) usam a memória crua acima.
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

Vocabulário OFS para memória, regiões, lanes, packets e intrinsics `fault_*`. Como bloco, libera as operações de baixo nível (F4) e, quando o próprio bloco aloca, é uma **arena** (abaixo).

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

### Arena `bedrock { }` (A5.3)

Um bloco `bedrock { ... }` (também `tectonic bedrock { }` e `bedrock vein f() { }`, que torna o corpo inteiro um bloco) em que o **próprio código do bloco aloca** é uma arena: tudo o que ele aloca vem de uma região e sai de uma vez na saída do bloco, em **qualquer** saída (fim, `return`, `shatter`/`erode`, `?`), depois dos `seal`/`anchor` do bloco. Continua liberando as operações de baixo nível da F4. Um `bedrock` que não aloca nada é só o bloco de baixo nível (mesmo código de antes).

```ofs
core main() {
    forge total = 0
    forge ultimo = ""
    forge i = 0
    while (i < 100000) {
        bedrock {
            forge partes: Array<obsidian> = []
            ofs_array_push(partes, "k" + ofs_stone_to_obsidian(i))
            forge buf: *stone = ofs_alloc(1024)
            fracture { *buf = ofs_str_len(partes[0]) }
            fracture { total = total + *buf }
            ultimo = partes[0]          // texto sai copiado
        }                               // a arena inteira sai aqui
        i++
    }
    echo(total)
    echo(ultimo)
}
```

- **O que vem da arena:** o que o código escrito no bloco aloca: concatenação e conversões para texto, `[...]` e o crescimento de um `Array` criado no bloco, `forge m: Monolith`, `ofs_alloc`/`ofs_realloc` e os builtins de texto e de `Array` do runtime. `ofs_free` de um bloco da arena não faz nada (ele sai com a região). A arena é **léxica**: uma vein chamada de dentro do bloco aloca no heap normal (o compilador desliga a arena durante a chamada), e um `Array` de fora que cresce dentro do bloco continua no heap. Um `bedrock` dentro de outro abre a própria arena, que fecha antes da de fora.
- **Escape (regra):** um valor que pode estar na arena e vai para fora dela é tratado assim: **texto `obsidian` é copiado para o heap** automaticamente (só se estiver mesmo numa arena viva); **`Array`, `monolith`, ponteiro e geode** são `[ERRO DE ARENA arquivo:linha:col]` em compilação. "Para fora" é: `return` de dentro do bloco, atribuição a variável declarada fora do bloco (local, parâmetro ou global), campo ou elemento de um objeto de fora, `push`/`set` num `Array` de fora, argumento de uma vein que guarda o parâmetro (análise por vein, a mesma ideia da A6), de um valor de vein ou de uma função desconhecida, e o runtime que guarda o ponteiro (título da janela). Copiar um `monolith` só com campos numéricos para uma variável de fora vale (a cópia é por valor). Funções C (`extern`/`rift`) não guardam os argumentos (convenção do C); se uma guardar, aloque fora do bedrock. Dentro de `abyss { }` o erro não é dado: o programador assume o risco.

```
[ERRO DE ARENA app.ofs:5:17]: valor alocado na arena do bedrock e guardado fora dele (a arena e liberada na saida do bloco)
  dica: aloque fora do bedrock o que precisa sair dele (so texto obsidian e copiado para fora automaticamente), ou use abyss { } para assumir o risco
```

- **Nos dois modos:** a regra é decidida em compilação (`ofscc/sediment.ofs`, depois do `typeck`), igual para o nativo, o interpretador, a cristalização e a Fault Refract. No nativo, `ofs_arena_open()` na entrada e `ofs_arena_close()` como anchor sintético; o runtime (`stdlib/runtime/reservoir.ofs`) tira os blocos de pedaços de 64 KB que dobram (zerados como o `calloc`) e guarda o maior para a próxima arena. No interpretador, o `ofs_alloc` escrito no bloco vem da mesma arena do runtime e o texto copiado para fora passa pela mesma função (`ofs_arena_lift`).
- **Medida** (laço de 200 mil voltas com 16 textos num `Array`, um `monolith` e 1 KB de `ofs_alloc` por volta, nativo `-O2`): com `bedrock` 1,9 MB e 0,38 s; o mesmo laço num bloco comum (com `ofs_free` do buffer) 258 MB e 0,37 s.

#### Aviso de vazamento

Um `forge p = ofs_alloc(n)` (ou `malloc`, `calloc`, `realloc`, `ofs_realloc`) fora de uma arena cujo ponteiro nunca chega a um `seal`/`anchor`, `ofs_free`, chamada (dono), `return`, atribuição, campo, `Array` ou geode gera um aviso (não erro) no stderr, nos dois modos; `ofs_alloc(n)` solto, com o resultado descartado, também. `OFS_LEAK_WARN=0` silencia.

```
[AVISO DE VAZAMENTO app.ofs:3:24]: 'p' recebe ofs_alloc e nunca chega a um seal/ofs_free, return ou dono (o bloco vaza)
  dica: libere com seal { ofs_free(p) } logo depois, devolva ou guarde o ponteiro, ou aloque dentro de bedrock { } (a arena libera ao sair); OFS_LEAK_WARN=0 silencia
```

Testes: `ofs/tests/regress/bedrock_arena.ofs`, `arena_memory.ofs` (20 mil voltas de 64 KB com `ulimit -v` de 256 MB nos dois modos), `arena_err_*.ofs`, `leak_warn.ofs`.

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
| `as` entre valor de vein e endereço (`f as stone`, `n as vein(...)`) | — | libera |

- Aritmética de endereço passa por essas operações: `p + 1` com ponteiro já é `[ERRO DE TIPO]`; use `fault_step(p, n)` ou `((p as stone) + n) as *T`, ambos dentro do bloco.
- Continuam livres fora de bloco: chamar `extern`/`rift vein` (mesmo com ponteiro como argumento ou retorno, ex. `forge buf = ofs_alloc(64)`), comparar ponteiro com `null`, guardar e passar ponteiro adiante, os intrínsecos de bits (`fault_count`, `fault_cut`, `fault_weave`...) e tudo de valores de vein tipados (usar o nome da vein como valor, guardar, passar ao C num parâmetro `vein(...)`, chamar `f(x)`).
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

### `quartz`: assembly inline

`quartz` emite assembly diretamente no LLVM IR, com efeito colateral (`sideeffect`) por padrão. Só é permitido em `abyss { ... }` ou `abyss vein`; `fracture` e `bedrock` sozinhos não autorizam assembly. `asm` continua sendo o comando CLI `ofs asm`, que gera um arquivo `.s`.

```ofs
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
    echo(somar(20, 22)) // 42 (x86-64)
}
```

- Sem saída: `quartz { "nop" }`. Com saída: `quartz { ... } -> variavel "=r"`; declare a variável escalar antes. A restrição de saída é opcional (`=r` por padrão) e fica na mesma linha do destino. `+r` lê e escreve o destino; `=&r` marca saída antecipada, que não pode compartilhar o registrador de uma entrada.
- Entradas: `in "r" expressao`, `in "{rdi}" expressao` ou `in "m" variavel` (memória de uma variável escalar). Uma única saída escalar pode ser vinculada a um registrador. `$0` é a saída, quando houver; as entradas seguintes são `$1`, `$2`, etc. Sem saída, a primeira entrada é `$0`.
- Clobbers: coloque `melt "rdx" "memory"` depois das instruções e entradas, para declarar registradores e memória alterados. O LLVM não descobre esses efeitos pelo texto; declare todos os efeitos adicionais.
- O dialeto padrão é AT&T; `quartz att { ... }` é explícito e `quartz intel { ... }` seleciona Intel. Um `$` literal se escreve `$$`: `"shlq $$32, %rdx"`.
- O texto é específico da arquitetura. `OFSCC_TARGET` seleciona o alvo, mas não traduz instruções x86-64 para AArch64/Android. Use assembly apropriado ao alvo.
- No `ofs build`, o LLVM compila o trecho. No `ofs run`, a vein que contém `quartz` é carregada como código nativo no mesmo processo; veja [Modelo de execução](COMPILED_LANGUAGE.md).

### Edição ao vivo: `ofs refract` (Fault Refract)

`ofs refract app.ofs` roda o programa interpretado e troca, com ele aberto, as veins que você edita e salva. A unidade de troca é a **vein**:

- A troca acontece **entre chamadas**: a próxima chamada da vein usa a versão nova. Um **laço** em execução numa vein editada (inclusive o `while` do `main`) troca no próximo cabeçalho (**OSR**, onda 4): o laço novo continua com as variáveis locais como estão, e o resto da vein depois dele também é o novo; variável nova antes do laço ganha o valor inicial na hora da troca. Se uma variável antes do laço mudou de tipo (ou o laço está dentro de `cycle`/`match`/`bedrock`), o laço termina na versão antiga e a nova vale na próxima chamada. Fora de laço, a chamada em execução termina na versão antiga.
- O **estado** continua: globais existentes mantêm o valor (o inicializador não roda de novo), e arrays e objetos vivos não mudam. Um global novo é criado com o valor inicial.
- Uma vein **cristalizada** (nativa) ou com `quartz` é recompilada para um `.so` novo, junto com as veins nativas que a chamam; a versão nova roda interpretada até o `.so` ficar pronto (as com `quartz`, que não rodam interpretadas, são compiladas antes da troca).
- O programa novo inteiro é checado antes (sintaxe e tipos): com erro, nada muda.
- **Assinatura compatível** troca mesmo com a vein em uso: parâmetro novo no fim com valor padrão; retorno igual, que era `void`, inteiro estreito onde era `stone`, `f32` onde era `crystal`.
- **`monolith` com instâncias vivas migra**: campo novo ganha o valor padrão do tipo, campo removido some, a ordem pode mudar e um campo numérico pode mudar de tipo (o valor é convertido). As instâncias são reescritas no lugar.
- **Precisam de reinício** (a troca é recusada e o programa segue com a versão antiga): mudar a assinatura de forma incompatível numa vein que uma vein em execução ainda chama, ou de uma vein entregue ao C como callback ou usada como valor; remover uma vein que alguém em execução ainda chama; mudar um campo de `monolith` vivo para um tipo que não converte (ex.: `stone` → `obsidian`) ou crescer o `monolith` mais que 64 bytes; mudar o tipo de um global; mudar uma `extern`; remover ou reordenar variantes de `strata` (acrescentar no fim pode).

Exemplo: `ofs/examples/refract_demo.ofs`. Comando, mensagens e variáveis: [TOOLS.md](TOOLS.md). Mecanismo: [COMPILER_ARCHITECTURE.md](COMPILER_ARCHITECTURE.md).

### Estrato e permissões por módulo (`stratum`, `claim`, `caprock`)

Segurança gradual (Ideia 4) e permissões por módulo (A5.4), uma regra só. Cada arquivo (o principal e cada `attach`) é um **módulo** e declara no topo, antes do código (comentários, linhas em branco e `attach` podem vir antes ou entre as linhas), **quanto** baixo nível usa e **qual**:

```ofs
// io_cru.ofs
stratum fault          // opcional: fault é o padrão
claim rift, abyss      // o que o módulo usa

extern vein getenv(name: obsidian) -> obsidian
```

| Estrato | Baixo nível | Blocos | `claim` |
|---|---|---|---|
| `stratum crust` | nenhum: `*p`, `&x`, `as` com ponteiro, `fault_*` de memória, `extern`/`rift vein`, `quartz` são `[ERRO DE ESTRATO]` | `fracture`/`tectonic`/`abyss` são erro; `bedrock` vale só como arena (não libera ponteiro) | nenhum (`claim` num arquivo `crust` é erro) |
| `stratum fault` (padrão) | só dentro do bloco que o libera (regra da F4, tabela acima) | como na F4 | pede o que usar |
| `stratum abyss` | em qualquer lugar do arquivo, sem bloco | opcionais | vale como `claim rift, abyss, quartz` |

| Permissão | Libera | Exemplo |
|---|---|---|
| `claim rift` | declarar `extern vein` e `rift vein` (convenção do sistema, `abi c`) | `extern vein fopen(...)` |
| `claim abyss` | bloco `abyss { }` e `abyss vein` | `abyss { forge r = fault_call(f) }` |
| `claim quartz` | `quartz { }` (que ainda exige `abyss`, então junto vem `claim abyss`) | `claim abyss, quartz` |
| `claim magma` | chamar `magma.cast` (compilar e carregar código em execução; Ideia 5). Sem ele é sempre erro | `claim magma` |

- `fracture`/`bedrock`/`tectonic` não pedem claim: o estrato `fault` já os aceita; o estrato `crust` os recusa.
- `extern`/`rift vein` de símbolo do runtime do OFS (nome começando com `ofs_`, ex. `ofs_str_len`) não pede `claim rift`: é a própria linguagem, não código C de fora.
- **Quem anexa vê o que o módulo pede:** o `ofs check` lista, por módulo, o estrato e os claims (`[usa sem claim: ...]` e `[claim sem uso: ...]` quando não batem), e o `ofs doc` mostra selos `stratum`/`claim`/`caprock` na página do módulo.
- **O principal restringe os anexados** com `caprock` (só no programa principal; em módulo é erro): `caprock rift, abyss` deixa os anexados pedirem no máximo isso; `caprock crust` não deixa pedir nada. Um anexado que pede, usa ou é `stratum abyss` além do caprock é `[ERRO DE PERMISSAO principal:linha]` (sempre erro: o caprock é opcional). O runtime (`stdlib/runtime/`) fica fora do caprock; o próprio principal também (os claims dele valem para ele).
- Relação com blocos: o estrato diz **onde** o baixo nível pode aparecer (nenhum lugar / em blocos / em qualquer lugar), os blocos marcam **o trecho**, e o claim diz **qual** poder o módulo usa e é o que aparece para quem anexa. `abyss` é o nome do bloco, da permissão e do estrato mais fundo: o bloco libera o trecho, `claim abyss` autoriza o módulo a ter esse bloco, `stratum abyss` é o arquivo inteiro como bloco.
- Forma de comentário: `//! claim rift` (e `//! stratum ...`) vale igual. Serve ao código que também precisa compilar com um `ofscc` anterior à onda 5: o próprio compilador (`ofs/ofscc/`), por causa da semente `ofs/dist/ofscc`.
- O driver lê essas linhas por texto (como o `attach`) e não as passa ao lexer; diretiva depois do código, palavra desconhecida (`claim lava`) ou `stratum` repetido são `[ERRO DE ESTRATO arquivo:linha]`.

**Migração (igual à da F4).** Até **2027-03-16**, uso sem o claim é só aviso no stderr, uma mensagem por módulo e permissão, e o programa compila igual:

```
[AVISO DE PERMISSAO app.ofs:3:17]: 'extern vein labs' sem 'claim rift' no topo do arquivo
  dica: acrescente a linha 'claim rift' no topo de app.ofs (quem anexa o modulo ve o que ele pede no ofs check)
[AVISO DE PERMISSAO] 1 uso(s) de baixo nivel sem 'claim' no topo do modulo: viram erro em 2027-03-16; OFS_STRICT_FAULT=1 ja trata como erro
```

Com `OFS_STRICT_FAULT=1` (a mesma chave da F4, modo dos testes do repositório) vira `[ERRO DE PERMISSAO arquivo:linha:col]`. Os erros de estrato (`crust`) e de `caprock` são sempre erro, porque só valem para quem os declara. O stdlib, o runtime, o compilador, as ferramentas, os exemplos e os testes do repositório já declaram o que usam e passam no modo estrito. Testes: `ofs/tests/regress/stratum_*.ofs`, `claim_*.ofs` e `stratum_mods/`.

### Compilar e carregar OFS em execução: `magma.cast` (Ideia 5)

Um programa (no `ofs run` **e** no binário do `ofs build`) compila um **mod** escrito em OFS enquanto roda e chama as veins dele: plugins, scripts de jogo, mods. O mod é um arquivo OFS só com veins (sem `core main`; `attach` do stdlib vale). Módulo do stdlib: `attach {magma}`.

```ofs
attach {magma}
claim magma                       // carregar código em execução é uma permissão

core main() {
    forge m = magma.cast("mods/forja.ofs")      // caminho, ou o próprio fonte (texto com \n)
    if (!magma.cooled(m)) {
        echo(magma.crack(m))                    // a mensagem do compilador; o programa segue
        return
    }
    forge forjar: vein(stone) -> stone = magma.vent(m, "forjar")
    echo(forjar(10))
    forge erro: vein(crystal) -> stone = magma.vent(m, "forjar")
    if (erro == null) { echo(magma.crack(m)) }  // assinatura do mod diferente da pedida
    magma.release(m)
}
```

- `magma.cast(fonte)` → handle `stone` (sempre ≥ 1). `fonte` com quebra de linha é o próprio código; sem, é o caminho do arquivo. Erro de sintaxe, de tipo, de permissão ou de carga não derruba o programa: `magma.cooled(m)` é `false` e `magma.crack(m)` traz a mensagem do compilador (o fonte em texto aparece como `<fonte>`).
- `magma.vent(m, "nome")` → valor de vein **tipado** (F6). O tipo é o do lugar onde o valor é guardado (`forge f: vein(T) -> R = ...`, `=`, argumento, `return`): o compilador o passa como a assinatura pedida, e em execução ela é conferida contra a declarada no mod (manifesto do `.so`). Não bate, ou a vein não existe: o valor é `null` e `magma.crack(m)` diz o que difere (`parametro 1: stone no mod, o tipo pede crystal`). Sem tipo de destino (`forge f = magma.vent(...)`) é `[ERRO DE TIPO]`. Veins de namespace com o nome do programa (`"lava.grau"`).
- `magma.veins(m)` lista `nome: assinatura`; `magma.release(m)` faz o mod parar de entregar veins (o código **fica carregado**: valores de vein, textos e endereços já entregues podem estar guardados, a mesma cautela da onda 4 da Refract); `magma.caprock("crust")` é o teto dos próximos mods (`"rift"`, `"rift, abyss"`…; `""` tira o teto).
- **Globais:** o mod não vê os globais do programa nem o contrário; a conversa é por argumentos, retornos e valores de vein (o mod pode receber uma vein do programa como callback). Os globais do mod são dele e começam no carregamento. O mesmo fonte (mesmo código gerado) dá o mesmo `.so` e o **mesmo módulo** (estado compartilhado); o mod editado é outro `.so`, e a versão antiga continua valendo para quem guardou as veins dela.
- **Fronteira:** os tipos da assinatura são os da F6 (escalares, `obsidian`, `Array<T>`, valores de vein, `monolith` pelo nome: os dois lados precisam declarar igual). Limites do Rift no `ofs run`: até 12 parâmetros inteiros, ou até 4 com `crystal`.
- **Segurança:** chamar `magma.cast` pede `claim magma` no topo do arquivo; é **sempre erro** (`[ERRO DE PERMISSAO]`, sem migração). Num `stratum crust` é `[ERRO DE ESTRATO]`; `stratum abyss` vale como `claim magma`. O módulo `magma` declara `claim rift, abyss, magma`, então um `caprock` sem `magma` impede anexá-lo. O mod é compilado no modo estrito (`OFS_STRICT_FAULT=1`) e com o teto do `magma.caprock` valendo para ele inteiro (inclusive o arquivo do mod e os `attach` dele).
- **Como funciona:** o mod é compilado pelo `ofscc` num processo à parte (`OFSCC_MODE=cast`: o IR do `ir` mais o manifesto `@__magma_manifest`), vira `.so` com o clang (`-shared -fPIC -O2 -Wl,-Bsymbolic`) num **cache por hash** do IR gerado (o fonte expandido com os `attach`, o clang e o sistema; `OFS_MAGMA_CACHE`, `$XDG_CACHE_HOME/ofs/magma` ou `~/.cache/ofs/magma`) e é aberto com `dlopen` (`RTLD_LOCAL`); os globais do mod são iniciados uma vez (`__ofs_init_globals`). O mod usa o runtime do programa (mesmo heap, stdout e strings): no `ofs run` o `ofscc` já é ligado com `-rdynamic`; o `ofs build` liga com `--export-dynamic` o programa que usa `magma.cast` (ligando à mão: `-rdynamic`; sem isso, `magma.crack` diz como).
- **Compilador e clang:** `OFS_MAGMA_OFSCC`; no `ofs run`, o próprio `ofscc` em execução; senão `$OFSCC`, o `ofscc` ao lado do `ofs` do `PATH`, `~/.local/lib/ofs/ofscc`, `ofscc` do `PATH`, `/usr/local/bin`, `/usr/lib/ofs`. O stdlib do mod: `OFS_STDLIB_PATH` ou a pasta ao lado do `ofscc`. clang: `OFS_MAGMA_CC` (ou `OFS_QUARTZ_CC`), senão `clang`. Faltando um deles, o erro volta em `magma.crack`. `OFS_VERBOSE=1` mostra o `.so` e se veio do cache. Linux e macOS (no Windows, `magma.crack` avisa que ainda não existe).
- Exemplo: `ofs/examples/magma_cast_demo.ofs` (mod em `ofs/examples/magma_mods/forja.ofs`). Testes: `ofs/tests/regress/magma/` e `magma_err_*.ofs`.

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
