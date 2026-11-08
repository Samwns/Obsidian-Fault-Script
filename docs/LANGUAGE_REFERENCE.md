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
| `extern` | `KW_EXTERN` | Função estrangeira (FFI) |
| `rift` | `KW_RIFT` | Declaração de interop nativa |
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

> **Nota**: `match` e `case` estão reservados na gramática e no AST (`NK_MATCH`), com suporte completo no parser previsto no roadmap de evolução do compilador.

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

> **Nota**: `strata` é analisada sintaticamente e semanticamente, com geração direta para o backend LLVM prevista no roadmap de tipos algébricos.


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

---

## 11. `extern vein` e `rift vein`

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

O OFS não tem tipo "ponteiro de função", mas pode obter o endereço de uma `vein` como `stone` e chamar um endereço com a ABI C. Serve para callbacks de bibliotecas C (`qsort`, `ANativeActivity`, `ALooper`, `AChoreographer`) e para tabelas de funções C (JNI).

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
    c_qsort(buf, 8, 8, fault_addr(crescente))
    forge r = fault_call(fault_addr(crescente), buf, fault_step(buf, 1))
}
```

- `fault_addr(nome)` → `stone`: endereço da vein. Aceita `vein` do módulo, `ns.vein` e `extern`/`rift vein` (com `bind`, devolve o símbolo C). Em IR: `ptrtoint ptr @simbolo to i64`. Fora de uma vein, é erro semântico.
- `fault_call(endereco, args...)` → `stone`: chamada indireta com a ABI C (`inttoptr` + `call i64`). Argumentos inteiros, ponteiros e `obsidian`; o retorno é lido como `i64` (funções `void` devolvem lixo, ignore). Sem `float`/`double`.
- A vein usada como callback deve ter parâmetros compatíveis com a ABI C: `stone` para inteiros (inteiros de 32 bits chegam com o topo indefinido; mascare com `& 4294967295`) e `*stone` para ponteiros.
- Só no build nativo (`ofs build`); o interpretador (`ofs run`) não implementa `fault_*`.
- Teste: `ofs/tests/runtime/test_fault_addr_qsort.ofs`.


Campos importantes:

- `bind "..."` define o símbolo nativo real
- `abi c` define ABI C
- `...` é aceito em função variádica nativa


### Direção da interoperabilidade

`extern vein` é a superfície FFI atual, mais adequada a símbolos de runtime e compatíveis com C. O roadmap amplia isso para: fronteiras com objetos e bibliotecas C, pontes C++, pontes C#, embedding de Python e escapes de assembly específicos do destino. Esses itens são roadmap, não recursos implementados hoje.

### Padrão híbrido

É válido e recomendado misturar alto e baixo nível no mesmo arquivo:

- lógica de negócio em `vein` comuns;
- interop estrangeira na fronteira, por `rift vein`;
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


### Regras de `fracture`, `bedrock` e `abyss`

- Ponteiros só podem ser criados e usados dentro de `fracture`, `bedrock` ou `abyss`.
- `fracture`: o compilador checa os tipos de todas as operações com ponteiros; `&x` obtém o endereço de `x` e `*p` desreferencia `p`.
- `abyss`: checagem de tipos relaxada e endereçamento direto permitido. Use só quando for indispensável; prefira `fracture`.

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

Tipos ponteiro: `shard p: *stone` (ponteiro para inteiro, i64*), `shard q: *crystal` (f64*), `shard s: *obsidian` (i8**).

### Intrínsecos `fault_*`

Disponíveis dentro de `bedrock`, `fracture` ou `abyss`:

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
- `fault_call(endereco, args...)` → chamada indireta com ABI C (ver seção 11)
- `fault_cut(value, shift, width)` → extrai um campo de bits
- `fault_patch(base, shift, width, insert)` → substitui um campo de bits
- `fault_weave(mask, left, right)` → mistura bits de `left` e `right` conforme `mask`
- `fault_unreachable()` → marca o caminho como inalcançável (gera `llvm.trap` + `unreachable`)
- `fault_memcpy(dst, src, len)` → cópia de memória em bloco (`llvm.memcpy`); `dst` e `src` são ponteiros
- `fault_memset(dst, val, len)` → preenchimento de memória com o byte `val` (`llvm.memset`); `dst` é ponteiro

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
- Deve aparecer apenas dentro de `bedrock`, `fracture` ou `abyss`.

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

Módulo `odl` expõe:

- `odl.parse(src: obsidian) -> stone`
- `odl.element_count() -> stone`
- `odl.new_element(tag: obsidian) -> stone`
- `odl.get_element(id: stone) -> OdlElement`
- `odl.set_element(id: stone, elem: OdlElement) -> void`
- `odl.add_child(parent_id: stone, child_id: stone) -> void`
- `odl.child_count(parent_id: stone) -> stone`
- `odl.get_child_at(parent_id: stone, index: stone) -> stone`
- `odl.reset() -> void`

Módulo `oes` expõe:

- `oes.parse_sheet(css_src: obsidian) -> Array<OesRule>`
- `oes.apply_rules(node_id: stone, rules: Array<OesRule>) -> void`
- `oes.matches_selector(elem: OdlElement, sel: obsidian) -> bool`
- `oes.parse_color(cname: obsidian) -> stone`

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
| `rift` | Wrappers de runtime estrangeiro (auxiliares de ABI C) |
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

A finalidade desse módulo não é imitar APIs de C, e sim fazer a programação de baixo nível crescer no vocabulário do OFS. Os auxiliares de lanes dão uma superfície com endianness para janelas de bits voltadas a ABI, sem exigir deslocamentos manuais.

### Auxiliares de `rift.ofs` e `bedrock_packet.ofs`

- `rift.ofs`: `rift_text_size`, `rift_line`, `rift_banner`, `rift_report_line`. Permite consumir interfaces estrangeiras por APIs em OFS.
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
