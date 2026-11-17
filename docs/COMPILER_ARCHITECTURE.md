# Arquitetura do Compilador OFS

## Visão geral

```
source.ofs
    │
    ├─────────────────────────────────────────┐
    │                                         │
    ▼                                         ▼
┌──────────────┐                    ┌────────────────────┐
│   Lexer      │ → Array<Token>     │  Symbol Table      │
│  (chars,     │   (87 kinds)       │  (scope stack)     │
│   tokens)    │                    └────────────────────┘
└──────┬───────┘
       │
       ▼
┌──────────────┐
│   Parser     │ → AST Root ID (stone)
│  (recursive  │   (48 node kinds)
│   descent +  │   (pool-based allocation)
│   Pratt)     │
└──────┬───────┘
       │
       ▼
┌──────────────┐
│  Contracts   │ → mineral/bearing/settle viram checagens
│ (contracts   │   e os minerais viram o tipo base (A5.1)
│    .ofs)     │
└──────┬───────┘
       │
       ▼
┌──────────────┐
│  TypeChecker │ → Annotated AST
│  (with scope │   (type_str on each node)
│   stack)     │   geode: corpo vira vein __geode_*
└──────┬───────┘
       │
       ▼
┌──────────────┐
│  Sediment    │ → arena do bedrock (escape: copia
│ (sediment    │   o texto ou [ERRO DE ARENA]) e
│    .ofs)     │   [AVISO DE VAZAMENTO] (A5.3)
└──────┬───────┘
       │
       ├─────────────────────────────────┐
       │ (ofs run / interpreted)         │ (ofs build / native)
       ▼                                 ▼
┌──────────────┐                  ┌──────────────┐
│ Interpreter  │                  │  LLVM Gen    │ → output.ll (LLVM IR nativo)
│(interpreter  │                  │ (llvmgen.ofs)│   (mapeamento OFS → LLVM IR SSA)
│    .ofs)     │                  └──────┬───────┘
└──────┬───────┘                         │
       │                                 ▼
       ▼                             llc -filetype=obj output.ll -o output.o
 Direct Execution                        │
(instant evaluation)                     ▼
                             ld / ld.lld output.o magma.o → executável nativo
```

---

## Fase 1: Lexer (tokenização)

**Arquivo**: `ofs/ofscc/lexer.ofs` (433 linhas)

### Entrada
```ofs
forge x: stone = 42
```

### Processamento

1. **Classificação de caracteres** (`chars.ofs`):
   ```ofs
   is_digit(c), is_alpha(c), is_alnum(c), is_space(c)
   ```

2. **Reconhecimento de tokens**:
   - Palavras-chave (38): forge, vein, monolith, if, cycle, match/assay, strata, attach, etc.
   - Operadores (25+): +, -, *, /, ==, !=, &&, ||, &, |, ^, ~, %, ?, etc.
   - Literais: 42, 3.14, "hello\n", true, false, null
   - Identificadores: x, _var, CamelCase
   - Comentários: // and /* */

3. **Operadores de vários caracteres**:
   - ++ / -- / += / -= / *= /= / == / != / <= / >= / && / || / << / >> / -> / =>

### Saída
```
Token { kind: TOK_FORGE, val: "forge", line: 1, col: 1 }
Token { kind: TOK_IDENT, val: "x", line: 1, col: 7 }
Token { kind: TOK_COLON, val: ":", line: 1, col: 8 }
Token { kind: TOK_IDENT, val: "stone", line: 1, col: 10 }
Token { kind: TOK_EQ, val: "=", line: 1, col: 16 }
Token { kind: TOK_INT, val: "42", line: 1, col: 18 }
Token { kind: TOK_EOF, val: "", line: 1, col: 21 }
```

### Decisões de projeto

- **Sem ações no lexer** — apenas tokeniza, sem analisar ainda
- **Rastreio de linha/coluna** — para mensagens de erro
- **Sequências de escape** — \n, \t, \\, \", \r, \0 tratadas
- **Literais hex/float** — 0xFF, 1.0e-5 suportados

---

## Fase 2: Parser (construção da AST)

**Arquivo**: `ofs/ofscc/parser.ofs` (1.246 linhas)

### Estratégia

1. **Descida recursiva** para comandos e declarações
2. **Parser de Pratt** para expressões (precedência de operadores)
3. **AST em pool** — nós guardados em um `Array<Node>` global

### Gramática (simplificada)

```
program     ::= (fn_decl | monolith_decl | impl_decl | namespace_decl
                | strata_decl | extern_decl | attach_decl | var_decl)*

fn_decl     ::= ("vein" | "core") IDENT "(" params? ")" ("->" type)? block
params      ::= IDENT ":" type ("," IDENT ":" type)*
extern_decl ::= ("extern" | "rift") "vein" IDENT "(" params? ")" ("->" type)?
                ("bind" STRING)? ("abi" IDENT)?
monolith_decl ::= "monolith" IDENT "{" (IDENT ":" type (","|";")?)* "}"
impl_decl   ::= "impl" IDENT "{" fn_decl* "}"
namespace_decl ::= "namespace" IDENT "{" (fn_decl | var_decl | monolith_decl
                | extern_decl | impl_decl | strata_decl)* "}"
strata_decl ::= "strata" IDENT "{" (IDENT ("(" type ")")? (","|";")?)* "}"
attach_decl ::= "attach" "{" caminho "}"          // ex.: attach {core}, attach {F:lib.ofs}
var_decl    ::= ("forge" | "const" | "shard") IDENT (":" type)? ("=" expr)? ";"?

type        ::= IDENT | "Array" "<" type ">" | "*" type
              | "vein" "(" (type ("," type)*)? ")" ("->" type)?   // tipo de funcao (F6)

stmt        ::= var_decl | if_stmt | while_stmt | cycle_stmt | mine_stmt
              | match_stmt | return_stmt | throw_stmt | tremor_stmt
              | anchor_stmt | quartz_stmt | unsafe_block | block | "break" | "continue"
              | "shatter" | "erode" | expr ";"?
if_stmt     ::= "if" "(" expr ")" block ("else" (if_stmt | block))?
while_stmt  ::= "while" "(" expr ")" block
cycle_stmt  ::= "cycle" "(" IDENT "in" expr ")" block          // sobre uma coleção
              | "cycle" "(" "forge" IDENT "in" expr ")" block
              | "cycle" "(" init? ";" expr? ";" expr? ")" block // estilo C
mine_stmt   ::= "mine" "(" IDENT "in" expr ")" block
match_stmt  ::= ("match" | "assay") "(" expr ")" "{" (pattern "=>" (block | stmt))* "}"
pattern     ::= expr | "_"                                      // "_" é o caso padrão
return_stmt ::= "return" expr? ";"?
throw_stmt  ::= "throw" expr ";"?
tremor_stmt ::= "tremor" block ("catch" "(" IDENT (":" type)? ")" block)?
anchor_stmt ::= "anchor" stmt
unsafe_block::= ("fracture" | "bedrock" | "abyss" | "tectonic") ("fracture"|"bedrock"|"abyss")? block
quartz_stmt ::= "quartz" ("att" | "intel")? "{" quartz_item+ "}"
                ("->" IDENT STRING?)? ";"?
quartz_item ::= STRING | "in" STRING expr | "melt" STRING+ | "," | ";"
block       ::= "{" stmt* "}"

expr        ::= unary (binop expr)*         // precedência na tabela abaixo
unary       ::= ("-" | "!" | "~" | "&" | "*") postfix(primary)
postfix     ::= primary ( "(" args? ")" | "[" expr "]" | "." IDENT
                        | "++" | "--" | "?" | "as" type )*
primary     ::= IDENT | INT | FLOAT | STRING | "true" | "false" | "null"
              | "(" expr ")" | "[" (expr ("," expr)*)? "]" | "echo" "(" args? ")"
```

Pontos que a gramática não mostra:

- Os parênteses de `if`, `while`, `match`/`assay` e `cycle`/`mine` são obrigatórios. `if x == 1 { }` dá o erro `expected '('`. O corpo precisa ser um bloco `{ }`.
- O ponto e vírgula é opcional no fim de comandos e declarações. Só é exigido dentro do `cycle` estilo C.
- `assay` é só outro nome de `match` (mesmo token). O `match` vira uma cadeia de `if` comparando com `==`; `_` é o caso padrão e `=>` separa padrão e corpo. Não há padrões de desestruturação.
- `shatter` e `erode` são sinônimos de `break` e `continue`. `mine` é como o `cycle ... in`, mas só aceita o formato `item in coleção`.
- Chamadas com namespace (`ns.fn(x)`) viram o símbolo `ns__fn`. Métodos de `impl` viram `Tipo__metodo`.
- `attach {caminho}` só vale no nível do arquivo. O texto entre as chaves é colado token a token (`F:` indica caminho de arquivo).
- Não existe literal de struct (`P { x: 1 }`) nem `monolith ... layout ...`; o parser espera `{` logo depois do nome do `monolith`. Declare a variável (`forge p: P`) e atribua os campos.
- `core` é palavra reservada: marca o ponto de entrada (`core main()`) e não serve como nome.
- Tipo de função (F6): `vein(T1, T2) -> R` vira um `NK_TYPE_FN` com os tipos dos parâmetros em `extra`, o retorno em `c1` e, no `val`, o texto canônico sem espaços (`vein(T1,T2)->R`; sem `->`, `void`), que é o `type_str` de toda expressão desse tipo. As funções de texto do tipo (`vein_type_params`, `vein_type_ret`, `type_list_at`, que separa por vírgula fora de parênteses) ficam em `ast.ofs`, usadas pelos três estágios.
- Chamada de algo que não é nome (`a[i](x)`, `f(1)(2)`, `s.a.b(x)`): o parser guarda a expressão chamada em `c0` do `NK_CALL`; o typeck decide se é valor de vein ou a chamada pelo nome de antes.

### Pool de nós da AST

Todos os nós ficam num array global, com IDs numéricos:

```ofs
monolith Node {
    kind: stone        // constante NK_*
    val: obsidian      // nome ou valor literal
    type_str: obsidian // vindo do verificador de tipos
    line, col: stone   // posição no fonte
    c0, c1, c2, c3: stone  // índices dos filhos (fixos)
    extra: Array<stone>    // filhos adicionais
}

forge _node_pool: Array<Node> = []
```

### Exemplo: análise de `vein add(a: stone) -> stone { return a }`

```
NK_FN_DECL
├─ val: "add"
├─ c0: 2 (bloco de parâmetros)
│  └─ extra: [3] (nó de parâmetro)
│     ├─ kind: NK_PARAM
│     ├─ val: "a"
│     └─ c0: 4 (nó de tipo)
│        └─ kind: NK_TYPE, val: "stone"
├─ c1: 5 (tipo de retorno)
│  └─ kind: NK_TYPE, val: "stone"
└─ c2: 6 (bloco do corpo)
   └─ kind: NK_BLOCK
      └─ extra: [7] (comando return)
         ├─ kind: NK_RETURN
         └─ c0: 8 (nó de identificador)
            └─ kind: NK_IDENT, val: "a"
```

### Tabela de precedência (parser de Pratt)

```
Ordem de precedência (da menor para a maior):
1.  = += -= *= /=   (atribuição)
2.  ||              (OU lógico)
3.  &&              (E lógico)
4.  == !=           (igualdade)
5.  < <= > >=       (comparação)
6.  |               (OU bit a bit)
7.  ^               (XOR bit a bit)
8.  &               (E bit a bit)
9.  << >>           (deslocamento de bits)
10. + -             (adição)
11. * / %           (multiplicação)
12. Unário - ! ~ & *  (prefixo)
13. ++ -- ? as    (pós-fixo: incremento, propagação, conversão de tipo)
14. [] . ()         (pós-fixo: índice, campo, chamada)
```

---

## Fase 2b: Contratos (`ofs/ofscc/contracts.ofs`, A5.1)

Roda entre o parser e o typeck, nos dois modos (o llvmgen e o interpretador recebem a mesma árvore), e em cada recarga da Fault Refract (`refract_reload`):

1. **Registro:** os `NK_MINERAL` (`mineral Nome = T grain ...`: `c0` = tipo base, `extra` = `NK_CONTRACT` "grain") saem da árvore; os `bearing`/`settle` de cada vein estão num `NK_BLOCK` no `c3` do `NK_FN_DECL` (`NK_CONTRACT`, `val` = a palavra, `c0` = a regra). Mineral de mineral soma as regras; nome repetido, ciclo ou nome de tipo existente é `[ERRO DE CONTRATO]`. A forma curta da `grain` (`contains("@")` → `ofs_str_contains(value, "@")`) é reescrita aqui.
2. **Checagem estática:** um avaliador de constantes (inteiros, booleanos e textos; `+ - * / % & | ^`, comparações, `&& || !`, `ofs_str_contains`/`ofs_str_eq`/`ofs_str_len`, `const` globais com literal; nome declarado no escopo local é desconhecido) confere chamadas a veins com `bearing`/parâmetro mineral, `forge`/atribuição de mineral, `return` de vein `-> Mineral` e `settle` que só depende de `result`. Falso com certeza = `[ERRO DE CONTRATO arquivo:linha:col]` + `regra em ...`; o driver para (como no erro de sintaxe). Verdadeiro com certeza dispensa a checagem de execução.
3. **Checagem de execução** (desligada com `OFS_CONTRACTS=0`, que `ofs build --release` define): código OFS comum inserido na árvore. Na entrada da vein, `if (!(grain))` + chamada do builtin `__ofs_contract_fail` com a mensagem por parâmetro mineral e por `bearing`; depois de `forge`/`=`/`+=`/`++` num mineral (local, parâmetro ou `obj.campo` com objeto de tipo declarado); cada `return e` vira `{ forge __settle_result: R = e; checagens; return __settle_result }` (`result` → `__settle_result`; parâmetro alterado no corpo e citado num `settle` ganha uma cópia `__settle_in_p` na entrada). A mensagem é montada em execução com o texto da regra (`expr_text`), o lugar (`ast_src_loc`) e os valores (`ofs_stone_to_obsidian`, `ofs_crystal_to_obsidian`, `__ofs_bool_text`, texto entre aspas).
4. **Apagar minerais:** todo `NK_TYPE`/`NK_TYPE_FN`/`NK_CAST` do programa troca o nome do mineral pelo base (`Array<UserId>` → `Array<stone>`); o typeck, o llvmgen e o interpretador não conhecem minerais. Cada nó apagado guarda uma nota (`contracts.type_note`) com o mineral e as regras, que a Fault Refract soma à assinatura (`rf_sig`/`rf_mshape`/tipo de global): mudar a `grain` de um tipo usado é mudança de tipo.

O builtin `__ofs_contract_fail` (um argumento: a mensagem) é o único novo: no typeck é `void`; no llvmgen vira `ofs_array_fail(msg, ofs_str_len(msg))` do runtime (já existente: `"ofs: " + msg` no stderr e `exit(1)`; a declaração só é emitida quando o módulo usa); no interpretador, `BI_CONTRACT_FAIL` escreve o mesmo texto e sai com 1. Programa sem `mineral`/contrato não passa por nada disso (IR idêntico).

`OFSCC_MODE=assay` (`ofs assay`): antes da fase 2b, `contracts.assay_source` monta o texto de um `core main` com os casos de borda (limites das comparações com constante nas regras, ±1; casos fora do `bearing`/`grain` só listados), o driver o analisa e `assay_splice` troca o `core main` do programa por ele; depois roda interpretado com as checagens ligadas.

---

## Fase 3b: Sedimento (`ofs/ofscc/sediment.ofs`, A5.3): arena do `bedrock` e vazamento

Roda depois do typeck (usa os tipos), nos dois modos, no `ofs check` e em cada recarga da Fault Refract. Programa sem `bedrock` que aloca e sem `ofs_alloc` solto não muda (IR idêntico).

1. **Níveis.** Cada expressão tem um nível: 0 = heap (ou fora de qualquer arena), d = arena do `bedrock` de profundidade d. Alocação no próprio bloco (`+` de texto, `[...]`, `forge m: Monolith`, builtins de texto/`Array`/`ofs_alloc` do runtime) vale d; leitura de variável vale o conteúdo dela; campo, elemento, `*p` e `?` valem o da base; chamada de vein vale o maior nível dos argumentos (ela pode devolver um deles). Variável: nível "de casa" (profundidade da declaração) e conteúdo (maior nível atribuído, limitado à casa), em passadas até o ponto fixo, guardados por nó de declaração.
2. **Parâmetros guardados.** Por vein, a máscara dos parâmetros que podem ser guardados (atribuídos a global, campo, elemento, `*p`, `push`/`set`, capturados por geode, passados a um parâmetro guardado de outra vein ou a vein/C desconhecido); `return` não guarda. Ponto fixo entre todas as veins, começando pessimista (recursão fica guardada). `extern`/`rift` (C) não guardam; o runtime que guarda (título da janela) guarda.
3. **Escape.** Valor de nível maior que o do lugar para onde vai (`return` dentro do bedrock, variável/global/parâmetro de fora, campo/elemento de objeto de fora, `push`/`set`, argumento guardado): `obsidian` é embrulhado pelo embrulho __bedrock_lift; `s += x` de texto para fora vira `s = lift(s + x)`; o resto é `[ERRO DE ARENA]` (fora de `abyss`). Cópia de `monolith` sem ponteiros para variável de fora vale.
4. **Marca.** O `NK_BLOCK` `bedrock` com alguma alocação no próprio código recebe `type_str = "arena"`. O llvmgen abre `ofs_arena_open()` (slot na pilha) e registra o fechamento como anchor sintético (`register_scope_free`, tipo 3: `ofs_arena_close`), que roda em toda saída (`emit_anchors_from_depth`); dentro dele, toda chamada que não é builtin de texto/`Array` do runtime (`sediment.arena_builtin`) é feita com `__ofs_arena_cur = null` (a arena é léxica). __bedrock_lift vira `ofs_arena_lift`. No interpretador, `exec_arena_block` abre a arena do runtime por `dlsym`, deixa a arena inativa para as alocações do próprio interpretador e só o `ofs_alloc` escrito no bloco (mesma vein) usa `__ofs_arena_alloc`.
5. **Runtime** (`stdlib/runtime/reservoir.ofs`): `ofs_alloc` tira da arena ativa (bump em pedaços de 64 KB que dobram, zerado); `ofs_free` de bloco de arena viva não faz nada; `ofs_realloc` cresce no lugar (último bloco) ou copia dentro da mesma arena; o crescimento de um `Array` usa a região do próprio `Array` (`__ofs_alloc_like`), então um `Array` de fora continua no heap; `ofs_arena_close` fecha também as arenas abertas por cima, guarda o maior pedaço e zera o cache de `ofs_str_char_at`.
6. **Vazamento.** Por vein, fora de bloco "arena": variável local que recebe `ofs_alloc`/`malloc`/`calloc`/`realloc`/`ofs_realloc` e nunca aparece num uso de dono (argumento de chamada, `return`, lado direito de atribuição, `Array`, `seal`/`anchor`, geode, `&p`; `*p`, campo, índice e comparação não contam), ou alocação solta: `[AVISO DE VAZAMENTO]` no stderr. `OFS_LEAK_WARN=0` desliga.

---

## Fase 3: Verificador de tipos

**Arquivo**: `ofs/ofscc/typeck.ofs` (825 linhas)

### Tabela de símbolos

```ofs
monolith Symbol {
    name: obsidian       // nome da variável/função
    type_str: obsidian   // "stone", "obsidian", "Array<stone>", etc.
    kind: stone          // SYM_VAR, SYM_CONST, SYM_FN, SYM_PARAM
    fn_arity: stone      // número de parâmetros
    is_const: bool       // imutável?
}
```

### Pilha de escopos

```ofs
forge _scopes: Array<Array<Symbol>> = []

// Empilha escopo ao entrar em bloco/função
push_scope()

// Desempilha ao sair
pop_scope()

// Declara símbolo no escopo atual
declare(sym)

// Resolve símbolo (busca nos escopos externos)
resolve(name)
```

### Regras de verificação de tipos

1. **Literais**:
   - `42` → stone
   - `3.14` → crystal
   - `"hello"` → obsidian
   - `true`/`false` → bool

2. **Operações binárias**:
   - Aritmética (+ - * /): os dois operandos devem ser numéricos
   - Comparação (== != < <= > >=): retorna bool
   - Lógicos (&& ||): operandos e resultado são bool

3. **Declarações de variáveis**:
   - `forge x: stone = 42` → tipo explícito
   - `forge y = 42` → inferido como stone
   - `const z = 100` → inferido, imutável

4. **Chamadas de função**:
   - O número de argumentos deve bater com os parâmetros
   - Os tipos dos argumentos devem ser compatíveis com os parâmetros

5. **Tipos de função (F6)**: o nome de uma `vein`/`extern` (`SYM_FN`) usado como valor recebe o tipo `vein(parametros)->retorno` da declaração (`fn_value_ident`; `extern` de símbolo C variádico é erro). `f(x)` com `f` variável/parâmetro de tipo vein, `s.campo(x)` com campo vein (`vein_field_callee`), `c0` vindo do parser e `fault_call(f, ...)` com `f` tipado viram `NK_CALL` `__vein_call` com a vein em `c0` (`check_vein_call`): aridade conferida e cada argumento convertido para o tipo do parâmetro com as regras da F3, mas sem conversão definida é `[ERRO DE TIPO]` (não aviso). `coerce_value` exige a mesma assinatura ao guardar um valor de vein (`fn_store_check`, com a dica do que difere); `==`/`!=` só entre a mesma assinatura ou com `null`; `as` entre vein e endereço exige `abyss`. `echo(f)` e `"x" + f` ganham a conversão `__ofs_vein_name` (o nome da vein nos dois modos).

6. **Baixo nível exige bloco (F4)**: o parser guarda `fracture`/`bedrock`/`abyss`/`tectonic [X]` como `NK_BLOCK` com o nome no `val` (o corpo de `fracture vein`/`abyss vein` também); o `llvmgen` e o interpretador executam como bloco comum (o `bedrock` que aloca é também arena: Fase 3b). O typeck conta a profundidade (`_fracture_depth`, `_abyss_depth`, no molde do `_anchor_depth`) e, em `require_fault`, cobra o bloco para `*p`, `&x`, `as` com ponteiro, `fault_step`/`fault_memcpy`/`fault_memset` (qualquer bloco) e `fault_call`/`fault_addr` (só `abyss`). Fora do bloco: `[AVISO DE FALHA arquivo:linha:col]` no stderr até 2027-01-09; com `OFS_STRICT_FAULT=1`, `[ERRO DE FALHA ...]` e `_semantic_errors++`. Regra completa: [LANGUAGE_REFERENCE.md](LANGUAGE_REFERENCE.md), seção 12.
7. **Estrato e permissões por módulo (A5.4 + Ideia 4)**: o driver (`ofscc.ofs`, `expand_attaches_body`) lê o cabeçalho de cada arquivo por texto, como o `attach` (`stratum`, `claim`, `caprock` e a forma `//! claim`), e não passa essas linhas ao lexer; cada arquivo vira um módulo na tabela do `ast.ofs` (`ast_mod_*`: estrato, claims declarados, claims usados, se é runtime, o `caprock` do principal), e o mapa de linhas guarda o módulo de cada faixa (`ast_mod_of_line`). O typeck acha o módulo de cada nó pela linha: `require_fault` libera tudo no `stratum abyss` e é `[ERRO DE ESTRATO]` no `crust`; a entrada de bloco (`fault_block_open`) recusa `fracture`/`tectonic`/`abyss` no `crust` (o `bedrock` vira só arena, sem contar na profundidade) e cobra `claim abyss`; `extern`/`rift vein` (fora de `ofs_*`) cobram `claim rift` e `quartz` cobra `claim quartz` (`require_claim`, uma mensagem por módulo e permissão; `[AVISO DE PERMISSAO]` até 2027-03-16, erro com `OFS_STRICT_FAULT=1`). No fim (`check_modules`) vêm o `claim` em arquivo `crust` e o `caprock` (claims declarados ou usados de cada anexado, fora o runtime). O `ofs check` imprime a tabela de módulos (`print_module_claims`). Nada disso muda o IR.

### Tratamento de erros (atual)

- Apenas avisos (ainda sem `tremor`/`catch`)
- Linha/coluna reportadas
- Mensagem: `[typeck warn line:col] description`
- Erro de sintaxe: `[ERRO SINTATICO arquivo:linha:col]` com o arquivo e a linha reais (também dentro de um `attach`), pelo mapa de linhas do fonte expandido (`ast_src_loc`)

---

## Fase 4A: Backend interpretador de AST (interpretação direta)

**Arquivo**: `ofs/ofscc/interpreter.ofs` (70 KB, 2.086 linhas)

O backend interpretador da OFS executa a árvore sintática (AST) tipada diretamente em memória. Veins com `quartz` usam a execução híbrida descrita abaixo, com LLVM IR e biblioteca nativa em cache. É o backend padrão utilizado pelo comando `ofs <arquivo.ofs>` e `ofs run <arquivo.ofs>`.

### Arquitetura do Interpretador

1. **Sistema `Value`**: Monolith universal que unifica valores escalares (`stone`, `crystal`, `obsidian`, `bool`, `null`) e handles virtuais na heap para `Array<T>`, objetos `monolith` e ponteiros.
2. **Ambiente e Frames de Chamada**: Escopo léxico estrito baseado em marcadores de frame (`_current_frame_base` e `_globals_count`), impedindo vazamento de escopo entre funções chamadoras e variáveis globais.
3. **Semântica por Valor**: `forge y: T = x` e `y = x` copiam o bloco do `monolith` (`obj_clone()`/`memcpy` no bloco que `y` já tem, como o `store %T` do nativo); parâmetros de vein recebem o `monolith` por referência, como o ponteiro do nativo.
4. **Despacho Dinâmico de Métodos**: Resolução automática de métodos definidos em blocos `impl Type` com injeção automática de `self`.
5. **Ponteiros reais (F2)**: ponteiro é endereço (`VAL_INT` com tipo `*T`). `*p`/`*p = v` usam `mem_load`/`mem_store` pela classe de memória do tipo apontado (`MC_*`: largura, sinal, `f32`, `obsidian`, `*T`, `monolith`, variante), com o tipo estático do `typeck` pré-calculado por nó em `annotate_nodes` (o mesmo tipo que o `llvmgen` usa); a leitura/escrita é o próprio código nativo do interpretador (`fracture { *p8 = v as u8 }`). `&x` de variável troca a entrada por `VAL_BOXED`: o valor passa a viver num slot de 8 bytes (`box_var`), lido/escrito por `env_load`/`env_store` e liberado em `env_pop_one` (`_boxed_live` mantém o caminho rápido sem custo quando não há nenhum). `as` replica o `NK_CAST` do `llvmgen` (`eval_cast`). `monolith` vive em memória crua com o layout de `llvmgen.struct_layout` (a mesma função calcula o tamanho do `ofs_alloc` no nativo e os offsets no interpretador; `compute_layouts`), e o `VAL_OBJECT` guarda o endereço do bloco; campo `Array` guarda o handle interpretado como `-(h+1)`.
6. **Builtins Integrados**: Resolução imediata de funções de entrada/saída (`echo`), strings (`ofs_str_*`), vetores (`ofs_array_*`) e matemáticas (`ofs_pow`, `ofs_sqrt`). A lista (`is_builtin`) é só um caminho rápido, não o limite do que pode ser chamado.
7. **Rift (`extern vein`/`rift vein`)**: no carregamento (`collect_declarations` → `resolve_calls` → `ffi_check_symbols`), cada extern chamada vira um endereço via `dlsym(RTLD_DEFAULT, sym)` (com `bind`), depois de abrir `libm` e `OFS_FFI_LIBS`; símbolo ausente é o erro do linker (`undefined reference`), código 1. Veins de `stdlib/runtime/*.ofs` (faixas de linha marcadas por `expand_attaches` com `ast_mark_runtime_lines`) usam a versão nativa linkada no `ofscc`. A chamada (`call_extern`) converte os argumentos pelo tipo declarado (inteiros/ponteiros/`obsidian` como `i64`, `crystal` como `double`), grava num buffer e chama `__ofs_ffi_dispatch`, que o `llvmgen` define no próprio `ofscc` (`emit_ffi_dispatch`, quando o módulo declara essa extern): um `switch` com uma chamada de tipo LLVM exato por classe de assinatura (`ffi_sig`/`ffi_sig_supported`: até 4 args `i64|double` com retorno `i64|double`, até 12 só inteiros, variádicas de 1 a 3 fixos + até 4 extras; 155 casos). Por isso o `ofscc` precisa ser linkado com `-rdynamic` (exporta o runtime e o dispatcher para o `dlsym`). Na F2, `&x`, `monolith` (o bloco) e `Array` vão à função nativa como endereço: o `Array` é a coluna de valores do interpretador, que já é um `ofs_array` nativo (`arr_native_ptr`), com o comprimento sincronizado na volta (`ffi_sync_arrays`); um `Array` devolvido pela função nativa vira um array interpretado que usa o mesmo bloco (`arr_adopt`). Callbacks nativos → OFS: o módulo que declara `extern vein __ofs_cb_table` recebe do `llvmgen` (`emit_cb_trampolines`) 16 funções `__ofs_cb_K(i64 x 6)` que chamam `__ofs_cb_enter(K, ...)`, definida no fim do `interpreter.ofs`; `fault_addr` de uma vein interpretada liga a vein a um slot (`cb_slot_for`) e devolve o endereço de `__ofs_cb_K` (achado por `dlsym`, para um `ofscc` gerado por compilador antigo continuar ligando); `cb_enter` empilha os argumentos pelo tipo de cada parâmetro, roda a vein e devolve o resultado como `i64`, preservando os registradores do interpretador (reentrante: funciona dentro de uma chamada nativa feita pelo interpretador). Slots esgotados, mais de 6 parâmetros ou `crystal`/`f32` no callback são erro claro (código 1).
8. **Valores de vein (F6)**: `VAL_FN` guarda o tipo (`_rt`), a vein interpretada (`_rr` = id do nó da declaração, ou -1), o endereço nativo (`_ri`, de `extern` ou vindo do C) e o nome (`_rs`). `resolve_calls` marca cada `NK_IDENT` de vein com `_naux` (vein `id+1`, extern `-(ei+1)`); `eval_ident` só usa isso se não houver variável com o nome. `__vein_call` (`CALL_VEIN`) chama a vein interpretada direto (`call_function_by_id`) ou o endereço nativo com a assinatura do tipo (`call_native_sig` → `ffi_call_addr`, o mesmo caminho de `call_extern`). Indo para o C (`ffi_arg_int`, `reg_as_int`, `as stone`), uma vein interpretada vira o trampolim `cb_slot_for`; voltando do C (`fn_from_raw`: retorno de extern, parâmetro de callback), um endereço de trampolim volta a ser a vein de origem e um de extern ganha o nome dela. Em memória crua (campo de `monolith`, `*p`) a classe `MC_FN` guarda o endereço nativo ou `-(id+1)` para uma vein interpretada, sem gastar trampolim.

---

## Fase 4B: Gerador de LLVM IR (geração nativa)

**Arquivo**: `ofs/ofscc/llvmgen.ofs` (74 KB, 2.509 linhas)

O backend de compilação da OFS gera diretamente LLVM Intermediate Representation (IR), dispensando transpiladores para C ou dependências do Clang. É utilizado pelo comando `ofs build`.

### Mapeamento de tipos (OFS → LLVM IR)

```ofs
stone      → i64
crystal    → double
obsidian   → i8* (ponteiro para sequência UTF-8)
bool       → i1
void       → void
u8, u16, u32, u64 → i8, i16, i32, i64
i8, i32    → i8, i32
Array<T>   → %struct.Array* (vetor dinâmico do Stack Magma)
vein(T) -> R → ptr (ponteiro de função; a chamada usa o tipo R (T))
```

### Estratégia de emissão de LLVM IR

1. **Target Datalayout & Triple** — Configuração da arquitetura de destino (x86_64 ELF ou Windows PE32+)
2. **Declarações Externas** — Protótipos das rotinas do Stack Magma (`ofs_alloc`, `ofs_str_concat`, `ofs_array_push`, etc.)
3. **Definições de Funções** — Blocos básicos rotulados (`entry:`, etc.), alocações de variáveis (`alloca`), instruções SSA (`load`, `store`, `add`, `icmp`, `br`, `call`)
4. **Constantes de String** — Globais imutáveis `@.str.N = private unnamed_addr constant [...]`

### Exemplo: `forge x = 42`

**LLVM IR Gerado**:
```llvm
%x = alloca i64, align 8
store i64 42, i64* %x, align 8
```

### Exemplo: `vein add(a: stone, b: stone) -> stone { return a + b }`

**LLVM IR Gerado**:
```llvm
define i64 @add(i64 %a, i64 %b) {
entry:
  %0 = add i64 %a, %b
  ret i64 %0
}
```

---

### Alvos (`OFSCC_TARGET`)

Sem a variável, o IR sai com o triple do host (x86-64 Linux/Windows/macOS), idêntico ao de antes. `OFSCC_TARGET=android` emite `aarch64-unknown-linux-android26` (ou passe um triple completo). O runtime `magma` usa só a libc, então para o Android basta gerar o IR do `magma.ofs` com o mesmo triple e compilar com o clang do NDK; a `.so` é linkada com `-Wl,-init,__ofs_init_globals` porque não há `main()`. Ver [ANDROID.md](ANDROID.md).

Um `extern vein` que o próprio módulo define não é mais declarado de novo (antes: `invalid redefinition`).

### Endereço de função (`fault_addr`, `fault_call`)

`fault_addr(nome)`: o typeck troca o argumento pelo símbolo mangled (`ns.fn` → `ns__fn`) e exige que seja uma vein (`SYM_FN`); o llvmgen resolve `bind` de `extern`/`rift` e emite `ptrtoint ptr @simbolo to i64`. `fault_call(endereco, args...)` emite `inttoptr` + `call i64 %p(...)` com os tipos LLVM dos argumentos. Ver [LANGUAGE_REFERENCE.md](LANGUAGE_REFERENCE.md).

### Valores de vein (F6)

Um tipo `vein(...) -> R` é `ptr` no LLVM (8 bytes num `monolith`, como qualquer ponteiro). O nome de uma vein usado como valor (`NK_IDENT` com tipo vein que não é variável, `is_vein_ref`) vira `@simbolo` (com o `bind` da extern); um global iniciado com o nome de uma vein recebe a constante `@simbolo`. `__vein_call` (`emit_vein_call`) avalia a vein, testa `null` (desvia para `__ofs_vein_null`, que chama `ofs_array_fail` do runtime: mensagem e código 1) e emite `call R %f(T1 a, T2 b)` com a assinatura LLVM exata do tipo; cada argumento passa por `coerce_for_store` para o tipo do parâmetro. `==` entre valores de vein é `icmp ptr`. `__ofs_vein_name` é uma cadeia de `icmp eq ptr %p, @simbolo` sobre as veins que o módulo usa como valor. As duas funções auxiliares só são emitidas quando o módulo as usa: o IR de um programa sem valores de vein não muda.

### geode (closures)

O parser lê `geode (params) -> R { corpo }` (palavra de contexto: `geode` seguido de `(` cuja lista fecha antes de `->` ou `{`) como `NK_LAMBDA` (`c0` params, `c1` retorno, `c2` corpo). O typeck confere o corpo no escopo de quem o cria, com uma pilha de geodes abertos (`_gd_base`: profundidade dos parâmetros de cada um); um identificador local da vein (profundidade ≥ `_fn_base`) abaixo da base de um geode aberto vira captura desse geode (e dos de fora, em cadeia). Atribuir a uma captura, `&captura` e `shatter`/`erode` sem laço no corpo são `[ERRO DE GEODE]`. Depois o corpo sai para um `NK_FN_DECL` `__geode_<vein>_<k>` acrescentado ao fim do programa, com as capturas como primeiros parâmetros (tipos reconstruídos do texto canônico, `make_type_node`); o `NK_LAMBDA` fica com `val` = esse nome, `extra` = as capturas (`NK_IDENT`) e `c0..c3 = -1`, tipo `vein(params)->R`. Assim o resto do compilador (A6, contratos, cristalização, Refract) vê uma vein comum.

**Nativo:** sem captura, `@__geode_...` (vai ao C como qualquer vein). Com captura, `emit_geode` aloca no heap (arena desligada) `[@nome.env, MAGIC, captura 0..n]` (8 bytes por captura, `store` no tipo LLVM dela) e o valor é o endereço `| 1`. O adaptador `@nome.env(ptr %env, params)` (fim do módulo, `emit_sediment_helpers`) lê as capturas e chama a vein do corpo. Se o módulo tem geode com captura (`scan_geodes`), `emit_vein_call` testa o bit 0 e a marca e chama `adaptador(env, args)` (ou a vein direta, com `phi` no retorno), `__ofs_vein_name` devolve `<geode>` para ponteiro marcado, e valores de vein passados a funções fora do módulo ou convertidos em endereço passam por `__ofs_geode_c_check` (marcado = `ofs_array_fail`, código 1). Sem geode com captura, nada disso é emitido. Na A6, cada captura escapa (`esc_walk(captura, 1)`); a análise não se desliga mais.

**Interpretador:** `eval_geode` faz `VAL_FN` com `_rr` = id da vein do corpo (`_naux` resolvido em `resolve_calls_from`) e, com captura, `_ri` = handle + 1 de um array interpretado com as capturas copiadas. `call_vein_value` chama `call_geode` (capturas e depois os argumentos na pilha). Em memória crua o valor é `-(id + 1 + amb·2^24)`. `fn_native_addr` de geode com captura é o erro de execução. A cristalização (`cr_supported`) deixa interpretada, quando o programa tem geode, a vein que mexe com valores de vein ou alcança (`rf_walk_calls`) quem mexe; e a vein com arena só cristaliza se o runtime de arena estiver no processo. Na Refract, a vein `__geode_*` com assinatura nova (capturas mudaram) não é recusada nem redirecionada: os geodes já criados seguem com o corpo antigo.

## Fase 4C: assembly inline e execução híbrida

`TOK_QUARTZ` → `NK_QUARTZ` representa um comando com textos de instrução, entradas (`NK_QUARTZ_IN`), destino opcional e opções (`NK_QUARTZ_OPT`: restrições, clobbers e dialeto). `parser.parse_quartz` lê a sintaxe; `typeck.check_quartz` valida destino escalar, operandos e restrições, e `require_fault` exige `abyss`. `llvmgen.emit_quartz` gera `call ... asm sideeffect`, `inteldialect` quando solicitado e o armazenamento da saída. Entradas de memória usam `ptr elementtype(T)` com `*m`; saídas `+r` recebem uma entrada vinculada ao mesmo registrador.

No interpretador, `hy_prepare` encontra as veins que contêm quartz e solicita o fechamento a `llvmgen.hy_generate`. O módulo inclui funções e globais usados e declara o runtime exportado pelo próprio `ofscc` (`-rdynamic`). O IR e o comando de compilação determinam a chave do cache. `clang -shared -fPIC` cria a biblioteca, e `dlopen`/`dlsym` resolvem as raízes e os globais. A chamada tipada passa por `ffi_call_addr`/`__ofs_ffi_dispatch`; `hy_sync_in`/`hy_sync_out` sincronizam globais também em torno dos callbacks reentrantes. Heap e runtime pertencem ao mesmo processo. O backend do assembly e o fechamento nativo são reutilizáveis pela cristalização; a edição ao vivo (Fault Refract, Fase 4D) usa os dois.

`cr_tick` conta chamadas e, ao atingir `OFS_CRYSTALLIZE_THRESHOLD` (1000 por padrão), solicita o mesmo módulo para todas as raízes elegíveis. A geração de IR é síncrona; a compilação `clang -O2` roda em segundo plano e as chamadas seguintes verificam um marcador de conclusão antes de instalar o despacho. `OFS_CRYSTALLIZE=0` desativa a promoção opcional; compilador ausente ou falha opcional mantém o modo interpretado. Laços longos dentro de uma chamada usam a OSR (abaixo); a troca por edição de arquivo é a Fault Refract (Fase 4D).

**Globais compartilhados (onda 4).** Os módulos híbridos não definem os globais: cada global do fechamento vira `@__ofsg.<nome> = global ptr null` e todo acesso passa por ele (`var_ptr` emite `load ptr, ptr @__ofsg.nome`). Depois do `dlopen`, `hy_load` grava nesse ponteiro o endereço da **célula única** do global (`hy_global_cell`, 8 bytes): o interpretador guarda o próprio global nessa célula (`VAL_BOXED`, o mesmo mecanismo do `&x`), então o interpretador e todos os `.so` (cristalização, OSR, recristalizações da Refract) leem e escrevem a mesma memória, sem cópia na fronteira. Só ficam com cópia (`_hy_gmode` = 1, sincronizados por `hy_sync_in/out`) os globais `Array` (o interpretador tem handle próprio), os de tipo `vein(...)` e os ainda não iniciados. Medida (`gsync.ofs`, 24 globais, 400 mil chamadas interpretado→nativo, `OFS_OSR=0`): 0,43 s → 0,18 s (o interpretado puro leva 0,29 s: antes a cristalização perdia para ele nesse caso).

**OSR (onda 4).** `exec_while` conta as voltas de cada execução do laço; a cada 1024, `osr_tick` (com `OFS_OSR` diferente de `0`): um laço elegível (`osr_prepare`: só `while`; o corpo sem `return`/`throw`/`anchor`/`?`/`&`/geode/valor de vein/`bedrock`; as veins alcançadas sem geode e com o runtime de arena quando usam `bedrock`; o caminho até ele por bloco/`while`/`if`/`cycle`) que soma `OFS_CRYSTALLIZE_THRESHOLD` voltas é pedido ao `llvmgen` (`hy_osr_add`). As **variáveis vivas** são as declarações visíveis no cabeçalho (parâmetros e `forge` dos blocos do caminho, `osr_scope_decls`) cujo nome aparece no laço (`osr_live`). `hy_emit_osr` gera `__ofs_osr_<nó>(ptr %st) -> i64` num módulo próprio (o fechamento do corpo; os globais são as células compartilhadas): carrega cada variável de `st[1..n]` (8 bytes por variável; `monolith` é o ponteiro do bloco do interpretador, `Array` o `ofs_array` nativo, texto com dono 0), entra pelo cabeçalho (testa a condição), roda o laço como o `while` normal e devolve as variáveis em `st` com retorno 0. `osr_enter` faz a transferência (`mem_store`/`mem_load` por classe; `Array` com `arr_native_ptr`/`arr_sync`, como os globais) e o interpretador segue depois do laço. A compilação é em segundo plano, um trabalho por vez (`_cr_josr`, `_hy_load_loops`); cache por hash do IR. Variável com `&x` tomado ou fora do quadro: o laço fica interpretado. `bench/ofs/loop.ofs`: 18,6 s → 0,34 s (nativo 0,41 s).

**Callbacks com ponto flutuante e `f32` no FFI (onda 4).** Além de `__ofs_cb_K(i64 x6)`, o módulo que declara `__ofs_cb_table` (o `ofscc`) recebe `__ofs_cbx_K(i64 x6, double x8) -> { i64, double }` e `__ofs_cbx_table`: SysV x86-64 e AArch64 numeram inteiros e floats em registradores separados, então o trampolim recebe todos; o retorno `{ i64, double }` sai em rax/x0 **e** xmm0/d0 e serve a callback inteira, `crystal` ou `f32` (bits na metade baixa). `__ofs_cbx_enter` (interpretador) recebe os doubles como bits e distribui pelos parâmetros na ordem de cada classe. No Windows x64 (registradores por posição) os `cbx` não são gerados e a callback com float segue com erro claro. `f32` no Rift/FFI e nas raízes híbridas vai como `double` cujos 32 bits de baixo são o `f32` (o C lê só eles); o retorno `f32` sai dos 32 bits de baixo de xmm0.

O texto de assembly permanece específico do alvo. O LLVM não traduz instruções entre x86-64 e AArch64. A interface híbrida conserva os limites de aridade do FFI (até 12 inteiros, até 4 com `crystal`/`f32`; callback até 6 inteiros e 8 floats). Referência e regra de paridade: [LANGUAGE_REFERENCE.md](LANGUAGE_REFERENCE.md) e [COMPILED_LANGUAGE.md](COMPILED_LANGUAGE.md).

## Fase 4D: Fault Refract (edição ao vivo, L0/L1/L2/L3)

**Arquivos**: `ofs/ofscc/interpreter.ofs` (seção Fault Refract), `ofs/ofscc/ofscc.ofs` (`refract_reload`, `refract_paths`), `typeck.reset`, `ast_srcmap_reset`. Comando: `ofs refract` (wrapper) = `OFS_REFRACT=1` no modo `run`.

**Célula de despacho.** Toda chamada de vein passa por `call_function_by_id(fid)`. Com a refract ligada, `_rf_cell[fid]` (uma entrada por nó da AST) leva o id para a versão viva: a versão antiga de uma vein trocada aponta para a nova, e as cópias idênticas da versão nova apontam para a viva (assim o estado nativo e a contagem da cristalização ficam na versão que já roda). `rf_redirect` mantém as cadeias curtas (A→B e depois B→C vira A→C). Um contador por vein (`_rf_active`) diz quem está na pilha; quem está executando termina na versão antiga, porque o corpo em execução é o do nó antigo (sem OSR).

**Observação.** `rf_tick` roda no começo de cada chamada; a cada `OFS_REFRACT_PERIOD` ms (relógio por `dlsym("clock_gettime")`, `time` como reserva) faz `fflush` do stdout (saída ao vivo em pipe/arquivo), confere o trabalho nativo pendente e calcula um hash FNV do conteúdo dos arquivos observados (`refract_paths`: o principal e os attach fora da stdlib e do runtime). Conteúdo diferente e estável por um período dispara `rf_refract`. Arquivo ausente (gravação atômica em curso) conta como sem mudança.

**Recompilação no processo.** `refract_reload` refaz o caminho do início (expansão dos attach, lexer, parser, `typeck.reset` + `typeck.check`) com o stdout desviado para o stderr (`dup`/`dup2`), e os nós novos entram no mesmo pool. `build_node_tables_from`, `annotate_nodes_from` e `resolve_calls_from` processam só os nós novos; as tabelas por nó da cristalização (`_cr_heat`, `_cr_eligible`, `_cr_quartz`) crescem junto. Com erro, as tabelas crescem (os ids são globais), mas nenhuma célula muda.

**Diff.** `rf_scan` lista veins (chave = nome com namespace/monolith, como em `collect_declarations`), globais, monoliths e externs. Cada vein tem um hash estrutural da árvore tipada (`rf_hash`: tipo, texto e `type_str` de cada nó, sem linhas); hash igual = vein igual. Assinatura (`rf_sig`), forma do monolith (campos e tipos), tipo do global e forma da extern entram nas regras.

**Segurança** (qualquer regra recusa a troca inteira e nada muda): assinatura nova de uma vein chamada por uma vein ativa (`rf_active_user`, pelas chamadas já resolvidas: `CALL_FN`, valores de vein e métodos `CALL_DYN` pelo sufixo), entregue ao C (`_cb_fn`) ou usada como valor; vein removida chamada por uma ativa; layout de monolith com instâncias (`obj_new` conta por monolith); global com tipo novo; extern nova sem símbolo ou extern alterada; lista de variantes que não começa pela antiga (as etiquetas numéricas precisam continuar as mesmas). As chamadas de veins não alteradas foram checadas pelo typeck contra a assinatura nova, porque o programa inteiro é checado.

**Troca (L0).** Veins novas são registradas pelo nome (como no início); as alteradas não: o nome continua no id antigo e a célula leva à versão nova. Globais novos ocupam um dos `RF_GLOBAL_SLOTS` (64) slots reservados no fim da área de globais (`[0, _globals_count)`, onde `find_var` procura), executando o inicializador sem enxergar os locais do chamador; os existentes não são reexecutados. Monolith novo (ou com layout novo e sem instâncias) recebe uma faixa nova de campos e `compute_layouts`. Registradores e `_cur_call_id` são preservados em volta da troca.

**L1 (nativo).** Os `.so` carregados compartilham as células dos globais (Fase 4C, onda 4). Na troca, as raízes nativas afetadas são as que mudaram e as que alcançam (`rf_reaches`, transitivo) uma vein alterada, porque dentro do `.so` as chamadas são diretas. Elas ficam com endereço 0 (rodam interpretadas, na versão nova) e vão para `_rf_dirty`; `rf_launch` gera o fechamento a partir da raiz nova (`llvmgen.hy_generate`) e compila em segundo plano. Quando o `.so` fica pronto, `hy_set_root` substitui a entrada da raiz (`rf_live_fid`). Um trabalho só por vez; o que terminar depois de outra troca (geração `_cr_jgen` diferente de `_rf_gen`) é descartado e refeito com o programa atual. Veins com `quartz` são compiladas antes da troca em modo brando (`_hy_soft`: falha do clang/dlopen recusa a troca em vez de encerrar o programa). O `.so` antigo continua carregado. Descarregá-lo (`dlclose`) exigiria provar que nada mais aponta para ele: não basta não haver quadro dentro dele, porque literais de texto e endereços de veins do `.so` podem ter ido para globais, arrays e objetos; fica como pendência (copiar literais para o heap ou contar referências).

**OSR na Refract (onda 4).** Em cada cabeçalho de `while`, se `_rf_gen` mudou desde o começo do laço, `rf_osr_target` procura na versão viva da vein em execução (`_cur_fn_id` → `rf_live_fid`) o laço da mesma posição entre os `while` (pré-ordem) e compara as variáveis antes dele (`osr_scope_decls`, só bloco/`while`/`if`): mesmo nome e tipo segue com o valor vivo, nova ganha o valor inicial (`execute_stmt` da declaração, na hora da troca), removida sai de vista (`_env_sym` = -2), tipo diferente ou parâmetro novo recusa. `rf_osr_switch` roda o laço novo e depois o resto da versão nova pelo caminho (`rf_exec_after` nos blocos, reentrada nos `while` acima, nada nos `if`) e devolve `EXEC_RETURN` para o quadro antigo só desempilhar; a contagem de quadros ativos (`rf_frame`) passa para a versão nova. Um laço sem chamadas também vê a edição (`rf_tick` a cada 1024 voltas). Um laço **nativo** (OSR) da Refract recebe em `st[0]` o gancho `__ofs_osr_poll` e pergunta a cada 65536 voltas; com o arquivo mudado (`rf_osr_poll`, no ritmo de `OFS_REFRACT_PERIOD`) devolve 1 pelo cabeçalho, o interpretador segue interpretado (`_osr_hold` até a edição ser tratada), troca de versão e pede a OSR de novo com o programa novo (`rf_apply` zera `_osr_addr`: os laços nativos chamam direto as veins de antes).

**Assinatura compatível (onda 4).** `rf_sig_compat`: parâmetros antigos iguais (`rf_tstr`, com a grain) e no começo, novos no fim com valor padrão; retorno igual, antes `void`, inteiro estreito onde era `stone`/`i64`, `f32` onde era `crystal`. Compatível e com usuário ativo = aceita (callback ou valor de vein continuam recusando). O valor padrão de parâmetro (`b: stone = 3`, literal) é do typeck: `note_defaults` valida (literal, só no fim) e `fill_defaults` completa as chamadas com cópias do literal; a chamada antiga de uma vein que ganhou parâmetro recebe o padrão em `bind_params` (interpretada) ou em `rf_pad_defaults` (empilhado antes da raiz nativa).

**Migração de monolith (onda 4).** Na Refract cada bloco de monolith é registrado (`rf_obj_alloc`: `obj_new`, `obj_clone` e, no código nativo dos módulos híbridos, `__ofs_obj_alloc` no lugar do `ofs_alloc`, exceto dentro de um `bedrock` nativo, cujo bloco morre no fim dele) e nasce com `RF_OBJ_SLACK` (64) bytes de folga. Layout novo com instâncias vivas: `rf_migrate_check` recusa campo que muda para um tipo que não converte (numérico↔numérico converte) ou tamanho além da folga; senão `rf_migrate` reescreve cada instância **no lugar** (cópia do bloco antigo, zera, campo a campo pelo nome: mesmo tipo copia os bytes, numérico converte por `mem_load`/`mem_store`, novo ganha o padrão do tipo como em `obj_new`), então os ponteiros para ela continuam valendo. Os campos antigos saem da tabela (dono vazio), o que invalida o cache de campo (`_naux`) dos nós que já rodaram. Instâncias criadas pelo C ou com `ofs_alloc` cru não são conhecidas.

**Limites:** uma vein interpretada que vira quente depois de editada só cristaliza se já era elegível no início; até 64 globais novos por execução; OSR só de `while` (não de `cycle`) e sem laço com `return`/`throw`/`?` no corpo; a OSR da Refract não acompanha laço dentro de `cycle`/`match`/`bedrock`; o `.so` antigo não é descarregado.

**L3 (remoto/Android), feita.** **Arquivos:** `ofs/ofscc/ofscc.ofs` (seção Fault Refract L3: `r3_scan`, `r3_hash`, `r3_build_manifest`, `r3_patch_plan`), `ofs/ofscc/llvmgen.ofs` (`rf3_set`, `rf3_emit_cells`, `rf3_patch_generate`, a chamada pela célula em `emit_expr` e os globais `external` em `emit_header`), `ofs/stdlib/geophone.ofs` (agente), wrapper (`android_build` com `--refract`, `refract_remote_run`). O app roda **nativo AOT**, não interpretado. **Build com células** (`OFSCC_REFRACT=cells`): o driver acrescenta `attach {geophone}` ao fonte, lista as veins do usuário (as dos arquivos de `refract_paths`; `main` e `__*`, como os geodes, ficam sem célula) e o `llvmgen` emite `@__ofs_cell.<vein> = global ptr @<vein>` e, em toda `NK_CALL` cujo alvo tem célula, `load ptr` + chamada indireta (variádicas e valores de vein continuam diretos). O manifesto (`@__ofs_refract_manifest`, texto com um registro por linha: `V` símbolo/hash/assinatura/célula, `G` global/tipo, `M` monolith/campos, `X` extern/forma, `S` variantes) usa um hash estrutural da árvore tipada igual ao `rf_hash` (tipo, texto e `type_str`, sem linhas), calculado depois do typeck e do `sediment`. O agente sobe por `@llvm.global_ctors` (depois do `__ofs_init_globals`, que o build liga com `-Wl,-init`), na thread que carrega a lib (a principal), e põe o socket abstrato `ofs-refract.<pacote>` no looper dela (`ALooper_addFd`): sem thread nova, porque o runtime não é thread-safe, e o remendo é aplicado entre dois eventos. `SO_PEERCRED` aceita só os uid 2000 (adb/shell), 0 e o do app. **Remendo** (`OFSCC_REFRACT=patch`, `OFSCC_REFRACT_BASE` = manifesto que o app mandou ao conectar, com os registros dos remendos anteriores já juntados): `r3_patch_plan` compara vein a vein e aplica as regras (assinatura nova de vein com célula, global novo ou de tipo novo, layout de monolith existente, extern mudada, variantes fora de ordem: rc 4 com a mensagem; nada mudou: rc 3). Entram as alteradas, as sem célula no app (veins novas de remendos anteriores, geodes: redefinidas em todo remendo, porque as chamadas a elas são diretas) e as com célula que usam uma sem célula. `rf3_patch_generate` gera só essas veins; os globais viram `external` (o endereço único do app: o estado continua, como as células de globais da onda 4), as variantes ficam `internal` no remendo (acrescentar no fim funciona), as outras veins que elas usam viram `declare` (o fechamento do `hy_walk`, sem transitividade) e `@__ofs_refract_install` grava o endereço novo de cada vein com célula. No PC o `clang` do NDK liga o remendo com `-shared -fPIC -Wl,-Bsymbolic -Wl,--no-undefined` contra a `lib<nome>.so` do build (que ganhou `-soname`): o `dlopen` no app resolve os `declare` e os globais nela pelo `DT_NEEDED`, e um símbolo que o app não tem vira erro de ligação no PC. O app grava o `.so` em `code_cache/ofs-refract/`, faz `dlopen`, chama o instalador, junta o manifesto do remendo e responde `OK`/`ERR`; depois chama `oal__refresh` (se existir) ou pede um quadro. Medido no moto g(8) power (Android 11): remendo de 6 KB aplicado em 1 a 3 ms, mesmo PID, contagem preservada (`ofs/tests/refract/android.sh`). **Limites:** sem OSR no nativo (a vein em execução termina na versão antiga); valor de vein guardado aponta para a versão do build (o PC avisa); assinatura nova, global novo e layout novo exigem reinstalar; o `.so` de cada remendo fica carregado; só aarch64.

## Fase 4E: `magma.cast` (Ideia 5): compilar e carregar OFS em execução

**Arquivos**: `ofs/stdlib/magma.ofs` (a API, 100% OFS, igual nos dois modos), `ofs/ofscc/ofscc.ofs` (`OFSCC_MODE=cast`, `magma_manifest`, `OFS_MAGMA_CAPROCK`), `llvmgen.set_magma_manifest`, `typeck` (`claim magma`, `magma_vent_seat`), `ast.ofs` (`CLAIM_MAGMA`, teto do mod), wrapper (`--export-dynamic`).

**Decisão: o compilador num processo à parte, nos dois modos.** No `ofs run` o compilador está no processo, mas recompilar outro programa no mesmo pool de nós e no mesmo `typeck` (o caminho da `refract_reload`) trocaria o estado que o interpretador e a cristalização do programa hospedeiro ainda usam, e um erro do compilador chama `exit` em vários pontos: o mod quebrado derrubaria o programa. Um processo à parte isola as duas coisas e é o mesmo caminho no nativo, onde o binário não tem o compilador dentro (a alternativa, ligar o `ofscc` como biblioteca, deixaria todo binário com o compilador). No `ofs run` o processo é o próprio `ofscc` em execução (`/proc/<pid>/exe`, achado pelo símbolo `interpreter__run` exportado com `-rdynamic`); no nativo, `OFS_MAGMA_OFSCC` ou o `ofscc` instalado, procurado como o wrapper procura.

**Pipeline.** `magma.cast` escreve um script `sh` num `mkdtemp` (limite `ulimit -v 3000000`, `timeout 120`): `ofscc` com `OFSCC_MODE=cast`, `OFS_STRICT_FAULT=1` e o teto (`OFS_MAGMA_CAPROCK`) → IR; chave = sha256 do IR + clang + flags + `uname`; sem `.so` no cache, `clang -shared -fPIC -O2 -Wno-override-module -Wl,-Bsymbolic -lm` (com `-mllvm -opaque-pointers` no clang 14) e publicação atômica (`cp` + `mv`). O `.so` é aberto com `dlopen(RTLD_NOW)` (local); `__magma_manifest` (texto `nome\tsimbolo\tassinatura` por vein do arquivo do mod) é lido por `dlsym`, e `__ofs_init_globals` roda uma vez por `.so` (o mesmo fonte reaproveita o módulo). Erro em qualquer passo vira o `magma.crack(m)` (log do compilador sem as linhas de progresso).

**Modo `cast` no driver.** É o `ir` com: o teto `OFS_MAGMA_CAPROCK` aplicado como `caprock` (interseção com o do mod) e valendo também para o módulo 0 (`ast_mod_set_cast_ceiling`, `check_modules` começa do 0); `[ERRO DE MAGMA]` se o mod tem `core main`; o manifesto (`magma_manifest`: veins de primeiro nível e de namespace cuja linha é do módulo 0, sem `__*`, assinatura por `typeck.param_types_of` + `vein_type_make`, o texto canônico da F6) emitido pelo `llvmgen` no fim do `generate` como `@__magma_manifest`.

**`magma.vent` tipado.** `coerce_value` (forge com tipo, `=`, argumento, `return`) reconhece `magma__vent` (com `m`, o nome e `""`; o 3º parâmetro tem padrão `""`, preenchido por `fill_defaults`) num lugar de tipo `vein(...)`: troca o literal pelo texto canônico do tipo e embrulha a chamada num `NK_CAST` para esse tipo, criado pelo próprio typeck (sem passar pela regra do `abyss`, como as conversões implícitas). Em execução, o stdlib só devolve o endereço se a assinatura do manifesto é igual; senão 0, que vira `null`. Nativo: `inttoptr` e `call` com a assinatura exata; interpretador: `fn_from_raw` → chamada pelo Rift (`call_native_sig`), e uma vein interpretada passada ao mod vira trampolim `__ofs_cb_K`.

**Runtime compartilhado.** O `.so` não leva runtime: os `ofs_*`/`echo_*` vêm do processo (mesmo heap, stdout, strings, arena). O `ofscc` já é `-rdynamic`; o wrapper liga com `--export-dynamic`/`-rdynamic` quando o IR define `@magma__cast`; sem os símbolos (`dlsym(RTLD_DEFAULT, "ofs_str_concat") == 0`), `magma.cast` devolve o erro com a dica. `-Wl,-Bsymbolic` faz as veins e os globais do mod resolverem dentro dele, mesmo com o programa exportando símbolos de mesmo nome.

**Segurança.** `CLAIM_MAGMA` (bit 8; `CLAIM_ALL` = 15): `require_claim` na chamada de `magma__cast` e na definição dela (o módulo `magma` declara `claim rift, abyss, magma`), sempre erro (sem a migração até 2027-03-16); `crust` = `[ERRO DE ESTRATO]`; `caprock` sem `magma` impede anexar o módulo.

**Limites:** sem `dlclose` (o `.so` fica carregado depois de `magma.release`: literais e endereços podem estar guardados, como na onda 4); só Linux/macOS; `monolith` na fronteira pelo nome (sem conferir o layout); o mod não vê os globais do programa.

---

## Fase 4F: `ofs fossil` (Ideia 2): gravar, reproduzir e voltar no tempo

**Arquivos**: `ofs/ofscc/fossil.ofs` (formato `.fossil`, leitura/escrita, tabela de política por símbolo `fz_spec`, hash do fonte), `interpreter.ofs` (seção "ofs fossil": `fs_*`; ganchos em `execute_stmt`, `call_extern`, `ffi_call_addr`, `call_builtin` (`echo`/`exit`), `fault_runtime_error`, `hy_prepare`, `run`), `ast.ofs` (`ast_src_file`, `ast_src_line`, `ast_exp_line`), `ofscc.ofs` (`OFS_FOSSIL`, fonte do cabeçalho, `fs_enable`/`fs_finish`), wrapper (`ofs fossil`, `--to`, `--back`, `--dig`, `--layer`), extensão do VS Code (`ofs.fossilReplay/Next/Back`).

**Onde gravar.** O que já existia decidiu o desenho: toda chamada nativa do interpretador passa por `call_extern` → `ffi_call_addr` → `ffi_invoke` (F1), inclusive as veins nativas do runtime (`ofs_read_line`, `pulse__now_ms`, registradas como extern por `ffi_runtime_native`). `call_extern` consulta a política do símbolo (`fs_spec_of`, cache por extern; 0 = chamada normal, sem custo além de um teste de `_fs_mode`) e deixa em `_fs_pol`; `ffi_call_addr` consome e, com os argumentos já nos slots do FFI, chama `fs_invoke` no lugar de `ffi_invoke`. Ali os argumentos inteiros/ponteiros são copiados antes da chamada (um callback pode reusar o buffer). Gravando: chama o C e escreve o evento (retorno bruto, bits do float pelo slot de retorno, texto devolvido, bloco de `localtime`, bytes que o C escreveu no argumento do efeito e o hash FNV-1a dos dados de saída). Reproduzindo: lê o próximo evento, confere o símbolo (senão diverge com rc 3), monta o retorno (texto vira cópia nova, "o próprio argumento" vira o argumento atual), copia os bytes para o ponteiro atual do argumento, confere o hash da saída e só chama o C de verdade se for saída para o console e não estiver silenciado. O resto (`ffi_set_result`, `ffi_sync_arrays`) é o mesmo caminho de antes, então os registradores ficam iguais aos da gravação.

**Política** (`fz_spec`, um inteiro por símbolo: política, tipo do retorno, argumento e tamanho do efeito, regra do console, argumento e dados do hash): mundo (entrada, relógio, aleatório, ambiente, arquivos, rede), saída (console de verdade, arquivo/rede no-op conferido), saída do processo (`exit`: fecha o fossil antes) e vivo (o resto, determinístico). `OFS_FOSSIL_WORLD` acrescenta símbolos.

**Determinismo do resto.** O interpretador é determinístico dado o que vem de fora: endereços mudam (ponteiros do C, buffers), mas o programa só os passa de volta ao C. Por isso a cristalização e a OSR ficam desligadas (`hy_prepare`: `_cr_enabled = false` com `_fs_mode != 0`): o código nativo não passaria pelo gancho nem contaria passos. `quartz` continua nativo, com aviso.

**Passos.** `execute_stmt` chama `fs_step(id)` quando `_fs_on` (gravando e reproduzindo): conta o statement e, no replay, compara com o alvo (passo, linha expandida via `_fs_nline` pré-calculado só quando há alvo de linha, ou o primeiro statement do corpo da vein). Sem fossil o custo é um teste de global por statement (medido: `fib(35)` interpretado 6,21 s depois × 6,42 s antes, dentro do ruído).

**Voltar no tempo: camadas por `fork`.** Restaurar o estado do interpretador por cópia (tabelas `_env_*`, `_arr_*`, memória crua dos `monolith`, `ofs_alloc`, memória do C) seria grande e frágil; `fork` copia tudo sob demanda e é a mesma ideia dos *checkpoints* do `rr`. A cada `_fs_every` passos (`--layer`, padrão 2000; no máximo 32 camadas, depois uma sim outra não e o intervalo dobra), `fs_layer_make` faz `fflush` e `fork`: o filho vira camada e dorme lendo um pipe próprio; o pai segue. Voltar ao passo `t`: mata as camadas depois da escolhida, manda `go t` para a camada mais próxima ≤ `t` e sai; a camada faz `fork` de novo (continua guardando o ponto) e o filho segue silenciado até `t`. Como o replay é determinístico, todas as camadas estão na mesma linha do tempo e continuam válidas. A leitura de comandos é byte a byte por `read` (o buffer do stdio seria duplicado no `fork`).

**Supervisor.** Com navegação, o processo que o shell espera faz `fork` logo no início e fica fora da execução: recebe por um pipe `A <pid>` (nova linha do tempo ativa), `H <pid>` (a ativa passou a vez), `L/K <pid>` (camada criada/encerrada) e `X <código>` (fim). No Linux ele é *subreaper* (`prctl(PR_SET_CHILD_SUBREAPER)`): recolhe os órfãos e, se a ativa morrer sem `X`/`H`, mata as camadas e sai com `128 + sinal`. Funções POSIX por `dlsym` + `fault_call` (o `ofscc` do Windows liga sem elas; lá só `--to` para a frente).

**Estado mostrado.** `fs_where` (passo, `arquivo:linha` pelo mapa de linhas, vein, texto da linha) e `fs_show_vars` (locais do quadro atual `_current_frame_base..`, globais cujo `NK_VAR_DECL` está num arquivo do programa, coletados em `fs_collect_user_globals`; arrays e `monolith` por `interp_array_get`/`obj_load_field`; `FILE*` opaco como `<ponteiro do C>`).

**Limites:** `scanf` e estruturas fora da tabela; UI (eventos X11/Android); chamadas por valor de vein vindo do C (`call_native_sig`) rodam de verdade; `argv` ainda não existe na linguagem. Próximo passo: adaptador DAP com *step back* sobre o `--dig`.

---

## Fase 5: Driver do compilador

**Arquivo**: `ofs/ofscc/ofscc.ofs`

### Pipeline de Execução Self-Hosted

```ofs
core main() {
    forge src = read_file(input_file)
    src = expand_attaches(input_file, src)
    
    forge tokens = lexer.lex(src)
    forge root_id = parser.parse(tokens)
    typeck.check(root_id)
    
    llvmgen.generate(root_id, ll_output)
    
    system("llc -filetype=obj " + ll_output + " -o " + obj_output)
    system("ld " + obj_output + " ofs/dist/magma.o -lc -lm -o " + output_file)
}
```

---

## Modelo de memória

### Alocação estática

- Array de tokens: ~10 KB (típico)
- Pool de nós: ~1 MB (programas grandes)
- Tabela de símbolos: ~100 KB

### Alocação dinâmica (via Rift)

```ofs
extern vein ofs_alloc(size: stone) -> stone
extern vein ofs_free(ptr: stone) -> void
```

Usada para:
- Concatenação de strings (mínima)
- Crescimento de arrays (`Array<stone>` etc.)

---

## Garantias de determinismo

Para o bootstrap funcionar (`ofscc_v2 === ofscc_v3`):

**Determinístico**:
- A ordem dos tokens segue a ordem do fonte
- A ordem da AST segue a ordem do fonte  
- Tabela de símbolos ordenada por inserção
- Sem timestamps nem dados aleatórios

**Não determinístico** (evitar):
- Tabelas hash (iteração imprevisível)
- Ponto flutuante sem arredondamento
- Timestamps embutidos
- Saída dependente de endereços

---

## Limitações conhecidas

### Atuais

1. **Sem análise de argv** — argumentos de linha de comando fixos
2. **Verificação de tipos em uma passada** — sem declarações adiantadas
3. **Recuperação de erros limitada** — erros interrompem a compilação
4. **Diagnósticos em texto** — erros e avisos saem como `[ERRO ... arquivo:linha:col]` no stderr (sem formato estruturado, exceto com `OFS_PROGRESS=json`)

### Planejadas

1. Implementar a camada de rebaixamento OIR (Obsidian Intermediate Representation)
2. Tratamento de erros `tremor`/`catch` com desenrolamento de pilha (hoje só `throw`, que encerra o programa)
3. Argumentos de linha de comando (`argv`) na linguagem
4. `layout` em `monolith` (palavra reservada; o parser ainda não a aceita)

O interpretador (`interpreter.ofs`), a cristalização, o OSR, a Fault Refract (L0 a L3), o `quartz` e as mensagens com `arquivo:linha:col` já existem.

---

## Estratégia de testes

| Nível | Arquivos | Objetivo |
|---|---|---|
| **Unitário** | `test_lexer_units.ofs` | Classificação de caracteres, palavras-chave |
| **Integração** | `test_lexer_parser.ofs` | Lexer + Parser juntos |
| **E2E** | `test_bootstrap.sh` | Bootstrap completo em 3 passos |
| **Regressão** | `examples/` | Programas reais |

---

## Desempenho

O tempo depende do tamanho do programa, do nível de otimização e da ferramenta nativa. Use `bash bench/run.sh` para medir programas e registre a configuração e os resultados. Para a cristalização, meça separadamente geração de IR, compilação a frio, carregamento com cache e execução após a promoção; os tempos do build AOT não incluem a execução do programa.

---

## Arquitetura ativa: backend LLVM direto

O compilador OFS emite LLVM IR diretamente, sem código C intermediário:

```
llvmgen.ofs → output.ll (LLVM IR) → llc -filetype=obj → output.o → ld (with magma.o) → executable
```

Benefícios:
- Emissão direta de LLVM IR, sem código C intermediário; o driver pode usar `llc` e o linker nativo. O caminho híbrido compila a biblioteca com `clang`.
- Reprodutibilidade byte a byte e emissão de IR determinística
- Tempo de compilação dependente do programa, das otimizações e da ferramenta de geração nativa escolhida
- Runtime 100% OFS puro via Stack Magma (`magma.o`)
- Compilação cruzada de primeira classe (Linux ELF e Windows PE32+)

---

**Estado da arquitetura**: produção e self-hosted
**Verificação**: via `cmp -s v3.ll v4.ll` (diff zero)
