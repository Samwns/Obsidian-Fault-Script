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
│  TypeChecker │ → Annotated AST
│  (with scope │   (type_str on each node)
│   stack)     │
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

stmt        ::= var_decl | if_stmt | while_stmt | cycle_stmt | mine_stmt
              | match_stmt | return_stmt | throw_stmt | tremor_stmt
              | anchor_stmt | unsafe_block | block | "break" | "continue"
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

### Tratamento de erros (atual)

- Apenas avisos (ainda sem `tremor`/`catch`)
- Linha/coluna reportadas
- Mensagem: `[typeck warn line:col] description`

---

## Fase 4A: Backend interpretador de AST (interpretação direta)

**Arquivo**: `ofs/ofscc/interpreter.ofs` (70 KB, 2.086 linhas)

O backend interpretador da OFS executa a árvore sintática (AST) tipada diretamente em memória, sem gerar arquivos intermediários, código C ou LLVM IR. É o backend padrão utilizado pelo comando `ofs <arquivo.ofs>` e `ofs run <arquivo.ofs>`.

### Arquitetura do Interpretador

1. **Sistema `Value`**: Monolith universal que unifica valores escalares (`stone`, `crystal`, `obsidian`, `bool`, `null`) e handles virtuais na heap para `Array<T>`, objetos `monolith` e ponteiros.
2. **Ambiente e Frames de Chamada**: Escopo léxico estrito baseado em marcadores de frame (`_current_frame_base` e `_globals_count`), impedindo vazamento de escopo entre funções chamadoras e variáveis globais.
3. **Semântica por Valor**: `forge y: T = x` e `y = x` copiam o bloco do `monolith` (`obj_clone()`/`memcpy` no bloco que `y` já tem, como o `store %T` do nativo); parâmetros de vein recebem o `monolith` por referência, como o ponteiro do nativo.
4. **Despacho Dinâmico de Métodos**: Resolução automática de métodos definidos em blocos `impl Type` com injeção automática de `self`.
5. **Ponteiros reais (F2)**: ponteiro é endereço (`VAL_INT` com tipo `*T`). `*p`/`*p = v` usam `mem_load`/`mem_store` pela classe de memória do tipo apontado (`MC_*`: largura, sinal, `f32`, `obsidian`, `*T`, `monolith`, variante), com o tipo estático do `typeck` pré-calculado por nó em `annotate_nodes` (o mesmo tipo que o `llvmgen` usa); a leitura/escrita é o próprio código nativo do interpretador (`fracture { *p8 = v as u8 }`). `&x` de variável troca a entrada por `VAL_BOXED`: o valor passa a viver num slot de 8 bytes (`box_var`), lido/escrito por `env_load`/`env_store` e liberado em `env_pop_one` (`_boxed_live` mantém o caminho rápido sem custo quando não há nenhum). `as` replica o `NK_CAST` do `llvmgen` (`eval_cast`). `monolith` vive em memória crua com o layout de `llvmgen.struct_layout` (a mesma função calcula o tamanho do `ofs_alloc` no nativo e os offsets no interpretador; `compute_layouts`), e o `VAL_OBJECT` guarda o endereço do bloco; campo `Array` guarda o handle interpretado como `-(h+1)`.
6. **Builtins Integrados**: Resolução imediata de funções de entrada/saída (`echo`), strings (`ofs_str_*`), vetores (`ofs_array_*`) e matemáticas (`ofs_pow`, `ofs_sqrt`). A lista (`is_builtin`) é só um caminho rápido, não o limite do que pode ser chamado.
7. **Rift (`extern vein`/`rift vein`)**: no carregamento (`collect_declarations` → `resolve_calls` → `ffi_check_symbols`), cada extern chamada vira um endereço via `dlsym(RTLD_DEFAULT, sym)` (com `bind`), depois de abrir `libm` e `OFS_FFI_LIBS`; símbolo ausente é o erro do linker (`undefined reference`), código 1. Veins de `stdlib/runtime/*.ofs` (faixas de linha marcadas por `expand_attaches` com `ast_mark_runtime_lines`) usam a versão nativa linkada no `ofscc`. A chamada (`call_extern`) converte os argumentos pelo tipo declarado (inteiros/ponteiros/`obsidian` como `i64`, `crystal` como `double`), grava num buffer e chama `__ofs_ffi_dispatch`, que o `llvmgen` define no próprio `ofscc` (`emit_ffi_dispatch`, quando o módulo declara essa extern): um `switch` com uma chamada de tipo LLVM exato por classe de assinatura (`ffi_sig`/`ffi_sig_supported`: até 4 args `i64|double` com retorno `i64|double`, até 12 só inteiros, variádicas de 1 a 3 fixos + até 4 extras; 155 casos). Por isso o `ofscc` precisa ser linkado com `-rdynamic` (exporta o runtime e o dispatcher para o `dlsym`). Na F2, `&x`, `monolith` (o bloco) e `Array` vão à função nativa como endereço: o `Array` é a coluna de valores do interpretador, que já é um `ofs_array` nativo (`arr_native_ptr`), com o comprimento sincronizado na volta (`ffi_sync_arrays`); um `Array` devolvido pela função nativa vira um array interpretado que usa o mesmo bloco (`arr_adopt`). Callbacks nativos → OFS: o módulo que declara `extern vein __ofs_cb_table` recebe do `llvmgen` (`emit_cb_trampolines`) 16 funções `__ofs_cb_K(i64 x 6)` que chamam `__ofs_cb_enter(K, ...)`, definida no fim do `interpreter.ofs`; `fault_addr` de uma vein interpretada liga a vein a um slot (`cb_slot_for`) e devolve o endereço de `__ofs_cb_K` (achado por `dlsym`, para um `ofscc` gerado por compilador antigo continuar ligando); `cb_enter` empilha os argumentos pelo tipo de cada parâmetro, roda a vein e devolve o resultado como `i64`, preservando os registradores do interpretador (reentrante: funciona dentro de uma chamada nativa feita pelo interpretador). Slots esgotados, mais de 6 parâmetros ou `crystal`/`f32` no callback são erro claro (código 1).

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
4. **Sem diagnósticos** — apenas mensagens com echo

### Planejadas

1. Backend interpretador (`interpreter.ofs`) para avaliação instantânea via `ofs run` (já existe; evoluir)
2. Implementar a camada de rebaixamento OIR (Obsidian Intermediate Representation)
3. Adicionar tratamento de erros tremor/catch e desenrolamento de pilha
4. Mensagens de erro e diagnósticos com caminho do arquivo
5. Módulos padrão nativos de gráficos, janelas e áudio

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

| Fase | Tempo (típico) | Fator limitante |
|---|---|---|
| Lexer | <1ms | E/S de arquivo |
| Parser | ~10ms | Alocação de nós |
| TypeCheck | ~5ms | Busca de símbolos |
| LLVMGen | ~10ms | Geração de IR SSA |
| LLC (Assembler) | ~35ms | Emissão de código de máquina |
| Linker (ld / ld.lld) | ~15ms | Ligação de objetos com `magma.o` |
| **Total** | ~70ms | Geração de código de máquina nativo |

---

## Arquitetura ativa: backend LLVM direto (zero Clang)

O compilador OFS emite LLVM IR diretamente, sem código C intermediário:

```
llvmgen.ofs → output.ll (LLVM IR) → llc -filetype=obj → output.o → ld (with magma.o) → executable
```

Benefícios:
- Geração direta de código de máquina (sem depender de GCC ou Clang)
- Reprodutibilidade byte a byte e emissão de IR determinística
- Compilação rápida (~50-100 ms de ponta a ponta)
- Runtime 100% OFS puro via Stack Magma (`magma.o`)
- Compilação cruzada de primeira classe (Linux ELF e Windows PE32+)

---

**Estado da arquitetura**: produção e self-hosted
**Verificação**: via `cmp -s v3.ll v4.ll` (diff zero)
