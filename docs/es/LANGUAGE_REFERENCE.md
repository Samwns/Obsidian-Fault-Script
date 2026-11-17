# Obsidian Fault Script — Referencia del lenguaje

Referencia práctica del lenguaje soportado actualmente por el compilador de OFS.

---

## Índice

1. Estructura de programa
2. Tipos
3. Variables y constantes
4. Funciones e intents
5. `monolith`, `impl` y layouts
6. `namespace`
7. Arrays y colecciones
8. Control de flujo
9. Operadores y casts
10. `attach`
11. `extern vein` y `rift vein`
12. Bloques de bajo nivel
13. Biblioteca estándar y paquetes
14. Comandos del compilador

---

## 1. Estructura de programa

Todo programa ejecutable necesita un `core main()`:

```ofs
core main() {
    echo("inicio")
}
```

`core` es palabra reservada: solo marca el punto de entrada. Usarla como nombre (`vein core()`, `core(1)`, también dentro de un `namespace`) es **error de compilación** ("`core` e palavra reservada…"); renombre la función (p. ej. `core_fn`). Cualquier error sintáctico detiene la compilación con código 1.

Un archivo `.ofs` puede contener:

- `attach { ... }`
- `extern vein` y `rift vein`
- `vein` y `core`
- `monolith`
- `impl`
- `namespace`
- `strata`
- `forge` global

---

### Comentarios

```ofs
// Comentario de una línea

/* Comentario
   de varias líneas */

forge x = 42  // comentario al final de la línea
```

### Terminación de instrucciones

Las instrucciones terminan con un **salto de línea** o un **punto y coma**. Los saltos de línea dentro de `()`, `[]` y `{}` se ignoran (continuación implícita).

```ofs
forge x = 10; forge y = 20

forge resultado = soma(
    10,
    20
)
```

### Palabras reservadas

| Palabra | Token | Finalidad |
|---|---|---|
| `core` | `KW_CORE` | Función de entrada |
| `vein` | `KW_VEIN` | Definición de función |
| `forge` | `KW_FORGE` | Declaración de variable |
| `monolith` | `KW_MONOLITH` | Definición de struct |
| `fracture` | `KW_FRACTURE` | Bloque de punteros seguro |
| `abyss` | `KW_ABYSS` | Bloque de memoria insegura |
| `cycle` | `KW_CYCLE` | Bucle |
| `if` | `KW_IF` | Condicional |
| `else` | `KW_ELSE` | Rama else |
| `return` | `KW_RETURN` | Retorno desde función |
| `echo` | `KW_ECHO` | Impresión (integrada) |
| `in` | `KW_IN` | Bucle por rango |
| `shard` | `KW_SHARD` | Tipo puntero |
| `true` | `KW_TRUE` | Booleano verdadero |
| `false` | `KW_FALSE` | Booleano falso |
| `null` | `KW_NULL` | Valor nulo |
| `break` | `KW_BREAK` | Salir del bucle |
| `continue` | `KW_CONTINUE` | Omitir iteración |
| `stone` | `KW_STONE` | Tipo entero (i64) |
| `crystal` | `KW_CRYSTAL` | Tipo flotante (f64) |
| `obsidian` | `KW_OBSIDIAN` | Tipo cadena (i8*) |
| `bool` | `KW_BOOL` | Tipo booleano (i1) |
| `void` | `KW_VOID` | Tipo void |
| `attach` | `KW_IMPORT` | Adjuntar módulo |
| `while` | `KW_WHILE` | Bucle solo con condición |
| `extern` | `KW_EXTERN` | Función nativa del Rift (biblioteca del sistema) |
| `rift` | `KW_RIFT` | Declaración de función nativa mediante el Rift |
| `as` | `KW_AS` | Cast de tipo |
| `match` | `KW_MATCH` | Coincidencia de patrones |
| `case` | `KW_CASE` | Rama de match |
| `default` | `KW_DEFAULT` | Rama de respaldo de match |
| `const` | `KW_CONST` | Variable inmutable |
| `strata` | `KW_STRATA` | Declaración de enum |
| `tremor` | `KW_TREMOR` | Bloque de manejo de errores |
| `catch` | `KW_CATCH` | Manejador de errores |
| `throw` | `KW_THROW` | Lanzar valor de error |
| `tectonic` | `KW_TECTONIC` | Prefijo de directiva de modo |
| `bedrock` | `KW_BEDROCK` | Bloque de bajo nivel tipado; arena cuando asigna memoria (sección 12) |
| `anchor` / `seal` | `TOK_ANCHOR` | Acción que se ejecuta al salir del ámbito (los dos nombres son el mismo token) |
| `quartz` | `TOK_QUARTZ` | Ensamblador en línea, requiere `abyss` |

Palabras de contexto (no reservadas; valen como nombre fuera de ahí): `mineral`, `grain`, `bearing`, `settle`, `value` y `result` (contratos, sección 4) y `geode` (función que captura, sección 4: solo cuando viene `geode (...)` seguido de `->` o `{`).

---

## 2. Tipos

### Tipos primitivos

| Tipo | Uso | Representación general |
|---|---|---|
| `stone` | entero principal | 64 bits |
| `crystal` | punto flotante | 64 bits |
| `obsidian` | cadena de texto | puntero |
| `bool` | booleano | lógico |
| `void` | sin retorno | retorno de función |

### Tipos enteros pequeños

| Tipo | Signo | Tamaño |
|---|---|---|
| `u8` | sin signo | 8 bits |
| `u16` | sin signo | 16 bits |
| `u32` | sin signo | 32 bits |
| `u64` | sin signo | 64 bits |
| `i8` | con signo | 8 bits |
| `i32` | con signo | 32 bits |
| `i16` | con signo | 16 bits |
| `i64` | con signo | 64 bits |

Ejemplo:

```ofs
forge r: u8 = 255
forge g: u8 = 128
forge pixel: u32 = (r as u32 << 16) | (g as u32 << 8)
```

### Tipos compuestos

| Tipo | Ejemplo |
|---|---|
| `Array<T>` | `Array<stone>` |
| tipo de función | `vein(stone) -> stone` |
| `monolith` | `monolith Player { ... }` |

### Inferencia

```ofs
forge n = 42
forge nome = "Ana"
forge ativo = true
forge nums = [1, 2, 3]
```

### Conversiones implícitas y operaciones (regla única)

El verificador de tipos (`typeck`) calcula el tipo final de cada expresión y escribe las conversiones implícitas en el propio árbol del programa. El código nativo (`ofs build`) y el intérprete (`ofs run`) solo ejecutan esas conversiones, así que el mismo programa da el mismo resultado en los dos modos.

**Tipo de cada expresión**

| Expresión | Tipo |
|---|---|
| `a[i]` con `a: Array<T>` | `T` (sin `T` declarado: `stone`) |
| `p.campo` | el tipo declarado del campo |
| `f(...)`, `x.metodo(...)` | el retorno declarado de la `vein` |
| `ofs_array_get(a, i)` / `ofs_array_pop(a)` | el elemento de `Array<T>` |
| `cycle (x in a)` con `a: Array<T>` escalar | `x: T` |
| `a op b` entero con entero | `stone` |
| nombre de una `vein`/`extern` usado como valor (`forge f = dobro`) | `vein(T1, T2) -> R`, su firma (sección 4) |
| `f(...)` con `f: vein(...) -> R` | `R` |

**Conversiones implícitas (aceptadas)**

| Dónde | De → a | Cómo |
|---|---|---|
| aritmética y comparación mixtas | entero → `crystal` | `c + 1` vale `c + 1.0`; `2 * c` es `crystal` |
| aritmética de enteros | `u8`…`i64` → `stone` | la cuenta se hace en 64 bits (`iN` extiende con signo, `uN` con cero) y el resultado es `stone` |
| `forge x: T = v`, `x = v`, argumento, `return`, elemento de `Array<T>` | entero → `crystal` | `forge c: crystal = 1` vale `1.0` |
| ídem | `crystal` → entero | trunca hacia cero, igual que `as` |
| ídem | `bool` → entero | `true` → 1, `false` → 0 |
| ídem | entero → entero de otro ancho | trunca o extiende al guardar (`forge u: u8 = 300` vale 44) |
| comparación con número | `bool`/variante → `stone` | `ok == 1`; `Azul == 2` compara la etiqueta |
| `stone == null` | `null` → 0 | dirección guardada en `stone` |
| `"texto" + v` | `stone`, `crystal`, `bool`, variante → `obsidian` | `"c=" + 1.5` da `c=1.5`; `"ok=" + true` da `ok=true`; una variante da su nombre |
| `x op= v` | igual a `x = x op v` | `c += 1` con `c: crystal` suma `1.0` |

**Combinaciones sin sentido: error de compilación**

Salen como `[ERRO DE TIPO archivo:línea:columna]` con una pista, en los dos modos, antes de ejecutar nada:

| Ejemplo | Por qué |
|---|---|
| `p + 1`, `p * 2` con `p: *u8` | aritmética de punteros: use `fault_step(p, n)` o `(p as stone) + n` |
| `s + b` con `b: bool` | `bool` no es número: `(b as stone)` |
| `nombre * c`, `nombre - 1` con `nombre: obsidian` | `obsidian` solo acepta `+` (concatenación) |
| `c % 2` con `c: crystal` | `%` solo vale para enteros |
| `c & 1`, `b \| b` | los bits solo existen en enteros (para `bool` use `&&`/`\|\|`) |
| `"a" < "b"`, `p < q` (punteros) | `<`, `<=`, `>`, `>=` solo valen para números |
| `s == 0` con `s: obsidian` | comparación entre tipos distintos: convierta con `as` |
| `-texto`, `~c` | signo solo en números, `~` solo en enteros |
| `s += 0.5` con `s: stone` | escriba `s = (s + 0.5) as stone` |
| `forge f: vein(stone) -> stone = nome` con `nome(x: stone) -> obsidian` | la firma tiene que ser igual (parámetros y retorno) |
| `f(1, 2)` o `f("dez")` con `f: vein(stone) -> stone` | aridad y tipo de argumento se verifican en la llamada de valor de vein |
| `f + 1`, `f < g` con `f: vein(...)` | un valor de vein solo acepta `==`/`!=` (misma firma o `null`) y la llamada |

**Avisos (no bloquean el programa)**

`[AVISO DE TIPO]` aparece cuando un valor se guarda en un tipo sin conversión definida, por ejemplo `forge x: stone = "abc"` o `forge c: crystal = true`. En esos casos el nativo reinterpreta los bits (o la dirección) y el intérprete guarda el valor tal cual. Sigue siendo aviso porque el código de bajo nivel a veces quiere justamente esa reinterpretación: use `as T` para dejar clara la intención.

Cuando el `typeck` no conoce el tipo de un lado (una llamada sin firma conocida, por ejemplo), no convierte ni marca error.

**Variantes en `echo`**: `echo(v)` de una variante (tipo `strata` o literal `Verde`) imprime el **nombre** en los dos modos. La etiqueta numérica sale con `v as stone`.

---

## 3. Variables y constantes

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

### Asignaciones comunes

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

## 4. Funciones e intents

### Funciones normales

```ofs
vein soma(a: stone, b: stone) -> stone {
    return a + b
}
```

**Valor por defecto de parámetro:** `vein escala(x: stone, k: stone = 2) -> stone`. El valor por defecto es un literal (número, texto, `bool`, `null` o `-número`) y los parámetros con valor por defecto van al final (`[ERRO SEMANTICO]` si no). La llamada que omite los últimos argumentos recibe el valor por defecto (`escala(5)` = `escala(5, 2)`), igual en nativo y en el intérprete; vale también para métodos de `impl`. Un tipo de función `vein(...)` no guarda valores por defecto: la llamada por valor de vein pasa todos los argumentos. En la Fault Refract, añadir un parámetro con valor por defecto a una vein en uso es un cambio aceptado (las llamadas antiguas reciben el valor por defecto).

### Entrada del programa

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

Intents aceptados:

- `pure`
- `impure`
- `fractal`

### Composición de funciones

Las funciones pueden invocar a otras funciones y admiten recursión directa:

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

### Tipos de función y valores de vein

Una `vein` declarada también es un **valor**: su nombre (sin llamarla) tiene el tipo de función `vein(T1, T2) -> R`, escrito con la misma palabra que declara funciones. Sin `-> R`, el retorno es `void`. El tipo vale en variable, parámetro, retorno, campo de `monolith` y `Array<...>`, y no necesita bloque de bajo nivel: es seguro y tipado.

```ofs
monolith Operacao {
    nome: obsidian
    calc: vein(stone, stone) -> stone
}

vein dobro(x: stone) -> stone { return x * 2 }
vein triplo(x: stone) -> stone { return x * 3 }
vein soma(a: stone, b: stone) -> stone { return a + b }

// vein como argumento y como retorno
vein aplicar(f: vein(stone) -> stone, x: stone) -> stone { return f(x) }
vein escolher(grande: bool) -> vein(stone) -> stone {
    if (grande) { return triplo }
    return dobro
}

core main() {
    forge f: vein(stone) -> stone = dobro   // o: forge f = dobro
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

- **Valor:** el nombre de una `vein` del módulo, de `ns.vein`, de un método (`Rect__area`) o de una `extern`/`rift vein` (con `bind`, el símbolo nativo) es un valor del tipo de su firma. Una `extern` variádica (`printf`) no se vuelve valor (error claro).
- **Llamada:** `f(x)` con `f` variable, parámetro, global, campo (`s.cb(x)`), elemento (`a[i](x)`) o resultado (`escolher(true)(10)`) es una llamada indirecta tipada. El `typeck` verifica la aridad y los argumentos con las conversiones de la sección 2 (`f(5)` con parámetro `crystal` vale `f(5.0)`); un argumento sin conversión definida (`obsidian` donde el tipo pide `stone`) es `[ERRO DE TIPO]`, no aviso. El resultado tiene el tipo `R`.
- **Firma exacta:** guardar una vein en un lugar de tipo de función (`forge`, `=`, argumento, `return`, campo, `push`) exige la misma firma, parámetro por parámetro y en el retorno; el error dice qué difiere (`retorno: obsidian onde o tipo pede stone`). `null` cabe en cualquier tipo de función y es el valor inicial de variable y campo sin inicializador.
- **Igualdad:** `==` y `!=` entre valores de la misma firma o con `null`. Dos valores de la misma vein son iguales en los dos modos.
- **`echo` y texto:** `echo(f)` y `"x" + f` dan el nombre de la vein (`dobro`; `ns.f` para `ns__f`), `null` si es nulo y `<vein>` para una dirección que vino de C y no tiene nombre en el programa. Igual en los dos modos.
- **Llamar `null`:** error de ejecución `ofs: chamada de vein nula (o valor de vein e null)` en stderr y código de salida 1, en los dos modos.
- **Nativo:** el valor es el puntero de la función (`ptr @dobro`) y la llamada es `call R (T1, T2) %f(...)` con la firma LLVM exacta del tipo, incluido `crystal`.
- **Intérprete (`ofs run`):** el valor de una vein interpretada es la propia vein (llamada directa, sin pasar por C). Cuando el valor va a C (argumento de `extern`/`rift vein`) se convierte en el trampolín de callback de la sección 11; cuando viene de C (retorno de `extern`, parámetro de callback, memoria cruda) la firma del tipo elige el trampolín del Rift. Un trampolín que vuelve de C se reconoce como la vein de origen.
- **Dirección cruda:** `f as stone`, `f as *u8`, `direccion as vein(...)` y `fault_addr(f)` convierten entre valor de vein y dirección; solo dentro de `abyss` (sección 12). `fault_call(f, args...)` con `f` de tipo de función verifica la firma y vale `f(args...)`.
- En el árbol el tipo es un `NK_TYPE_FN` cuyo `val` es el texto canónico sin espacios (`vein(stone,crystal)->stone`), el mismo `type_str` de toda expresión de ese tipo; la llamada indirecta es un `NK_CALL` `__vein_call` con la vein en `c0`.
- Pruebas: `ofs/tests/regress/vein_type_*.ofs`.

### `geode`: vein que captura (closure)

`geode (parámetros) -> R { cuerpo }` es una **expresión**: una vein sin nombre que lee las variables del ámbito de fuera. El valor tiene el tipo de función `vein(T...) -> R` de arriba y se usa como cualquier valor de vein: guardarlo en variable, campo o `Array`, pasarlo, devolverlo y llamarlo. Sin `-> R`, el retorno es `void`. El nombre viene de la geoda, la piedra hueca que guarda lo que capturó.

```ofs
vein somador(base: stone) -> vein(stone) -> stone {
    return geode (x: stone) -> stone { return x + base }
}

core main() {
    forge k = 10
    forge mais_k = geode (x: stone) -> stone { return x + k }
    k = 100
    echo(mais_k(5))             // 15: k se copió cuando nació el geode
    forge s7 = somador(7)
    echo(s7(1))                // 8
    echo(mais_k)                // <geode>
}
```

- **Captura por valor.** Toda variable local (o parámetro) de fuera que el cuerpo lee se vuelve una captura, copiada en el momento en que se crea el geode; cambiar la variable después no cambia el geode. Los globales no se capturan (el geode lee el global actual, como una vein). Un geode dentro de otro captura en cadena.
- **Qué significa la copia:** lo mismo que pasar la variable como argumento de una vein. Números, `bool` y etiquetas de `strata` se copian; `obsidian` es un valor inmutable (la copia es el texto); `Array` y `monolith` son el mismo objeto (el geode ve los `push` y campos cambiados después, y lo que él cambia en elementos/campos se ve fuera). Para un `monolith` independiente, cópielo antes (`forge c: P = m`) y capture `c`.
- **Asignar a una captura** dentro del geode (`n = n + 1`, `n++`, `&n`) es `[ERRO DE GEODE]`: la captura es una copia y la asignación no cambiaría la variable de fuera. Use una local (`forge v = n`) o devuelva el valor. `shatter`/`erode` en el cuerpo sin un bucle del propio geode también es error (el cuerpo es otra vein).
- **Igualdad y texto:** cada creación de un geode con captura es un valor distinto (`==` solo es `true` consigo mismo); un geode sin captura es siempre el mismo valor. `echo(g)` y `"x" + g` dan `<geode>` en los dos modos.
- **Rift (C):** un geode **sin captura** es una vein común y va al C como callback (`qsort(v, n, 8, geode (a: *stone, b: *stone) -> i32 { ... })`). Un geode **con captura** no puede ir al C (el C solo llama la dirección de una función): pasar un geode literal con captura a una `extern` es `[ERRO DE GEODE]` en compilación; si llega al C por una variable, o se vuelve dirección (`as stone`, `fault_addr`), el programa se detiene con `ofs: geode com captura nao pode ir para o C (Rift) nem virar endereco; ...` en stderr y código 1, en los dos modos.
- **Cómo funciona:** el `typeck` encuentra las capturas y saca el cuerpo a una vein `__geode_<vein>_<k>(capturas..., parámetros...)` al final del programa. En el nativo, sin captura el valor es `ptr @__geode_...`; con captura es un entorno en el heap `[adaptador, marca, captura 0, ...]` (8 bytes cada una) y el valor es esa dirección con el bit 0 encendido; la llamada indirecta prueba el bit (solo en los módulos que tienen geode con captura, así que el IR de los demás programas no cambia) y llama `adaptador(entorno, args)`. En el intérprete el valor es la vein del cuerpo con un entorno (array interpretado con las capturas). El entorno vive hasta el fin del programa.
- **Cristalización y Fault Refract:** en un programa con geode, la vein que maneja valores de vein (o llama a quien los maneja) queda interpretada; el cuerpo de un geode es una vein común y puede cristalizar. En `ofs refract`, editar el cuerpo de un geode (mismas capturas y parámetros) se cambia en vivo, incluso en los geodes ya creados; si las capturas cambian, los geodes ya creados siguen con el cuerpo antiguo y los creados después usan el nuevo.
- Pruebas: `ofs/tests/regress/geode_*.ofs`, caso `geode` en `ofs/tests/refract/run.sh`.


### Contratos y tipos con regla (`mineral`, `grain`, `bearing`, `settle`)

Un **`mineral`** es un tipo con regla: un tipo base más una o más **`grain`** (en la regla, `value` es el valor comprobado). Una vein puede tener **`bearing`** (precondición: quien llama la garantiza) y **`settle`** (poscondición: vale para el retorno; `result` es el valor devuelto), tantos como quiera, entre la firma y el cuerpo.

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

- **Compatibilidad:** el mineral es el tipo base para el typeck, el llvmgen y el intérprete. Base → mineral comprueba la regla (`forge x: UserId = e`, `x = e`, `x += e`, `x++`, campo `obj.f = e` de un monolith con campo mineral, argumento de un parámetro mineral, `return` de una vein `-> UserId`); mineral → base es libre. `Array<UserId>`, `*UserId`, `as UserId` y `vein(UserId)->T` valen como el base, sin comprobación.
- **Mineral de mineral:** `mineral Pequeno = UserId grain value < 100` suma las reglas (las del base primero).
- **Forma corta de `grain`:** una llamada sin `value` en los argumentos recibe `value` como 1.er argumento; `contains`/`len` sin vein con ese nombre son las de texto: `contains("@")` = `ofs_str_contains(value, "@")`, `len() >= 2` = `ofs_str_len(value) >= 2`.
- **`result`** solo vale en `settle` (en `bearing` es error). En `settle`, los parámetros valen como a la entrada, aunque el cuerpo los cambie. En una vein sin retorno, `settle` se comprueba en cada `return` y al llegar al final.
- **`mineral`, `grain`, `bearing`, `settle`, `value` y `result` son palabras de contexto**, no reservadas: siguen valiendo como nombres fuera de esas posiciones.

**Cuándo se comprueba la regla**

| Situación | Comprobación |
|---|---|
| Violación probada en compilación: literal, `const` o cuentas con ellos (`forge e: Email = "x"`, `dividir(1, 0)`, `return -1` en `-> UserId`, `settle` que solo depende de `result`) | `[ERRO DE CONTRATO archivo:línea:col]` con la regla y los valores, en los dos modos; también en release |
| En ejecución (`ofs run` y `ofs build` por defecto) | `ofs: [CONTRATO VIOLADO archivo:línea:col]: bearing de 'dividir' falhou: b != 0 (b = 0)` en stderr y **código de salida 1**; el mismo texto en los dos modos |
| Release: `ofs build --release` u `OFS_CONTRACTS=0` | sin comprobación en ejecución (la estática queda); el IR no tiene las comprobaciones |

Un valor probado correcto en compilación no recibe comprobación en ejecución. La falla de contrato no es un `throw`: `tremor`/`catch` no la atrapa. Una salida por `?` (propagación) no comprueba `settle`.

**Casos límite: `ofs assay archivo.ofs`.** Para cada vein con contrato y solo parámetros enteros (`stone`, `iN`, `uN` o mineral de base entera), genera casos en los límites de las reglas: cada comparación de un parámetro con una constante (`b != 0`, `n <= 10`, grain `value > 0`) da el valor de la frontera y ±1; un parámetro sin límite usa -1, 0 y 1 (hasta 64 casos por vein). Los casos fuera del `bearing`/la `grain` solo se listan (`--`); los demás se llaman (`ok`) y `settle` comprueba el resultado. La primera violación se detiene con el mensaje del contrato y código 1. Una regla que depende de algo más que constantes y parámetros deja la vein fuera (`pulada`). Veins de `impl`, parámetros no enteros e intervalos de bucle quedan para después.

```
$ ofs assay metade.ofs
assay metade(n: stone)
  --   metade(-1): fora do bearing n >= 0
  ok   metade(0)
ofs: [CONTRATO VIOLADO metade.ofs:3:5]: settle de 'metade' falhou: result * 2 == n (result = 0, n = 1)
```

**Fault Refract:** cambiar un `bearing`/`settle` es editar la vein (se cambia en vivo); cambiar la `grain` de un mineral usado en una firma es cambio de tipo (cambio rechazado si la vein está en uso).

Pruebas: `ofs/tests/regress/contract_*.ofs`. Pendiente: comprobación de mineral en `Array<Mineral>`, en `as` y en globales con valor no literal; intervalos de bucle en la comprobación estática; `ofs assay` con parámetros no enteros y sin detenerse en la primera violación.

---

## 5. `monolith`, `impl` y layouts

### Estructura

```ofs
monolith Rect {
    w: stone
    h: stone
}
```

Un campo puede ser otro `monolith`. El campo guarda una **referencia** al subobjeto (8 bytes en el layout del sistema); `forge c: Corpo` ya crea el subobjeto a cero, así que `c.pos.x = 1` funciona directamente. `c.pos = v` hace que el campo apunte a `v`; `forge w: Vec3 = c.pos` copia. Igual en nativo y en `ofs run` (prueba: `ofs/tests/regress/monolith_nested.ofs`).

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

### Métodos con `impl`

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

> **Planificado:** `layout` es palabra reservada, pero el parser todavía no acepta `layout` en un `monolith` (da `expected '{' after monolith name`). Hoy todo `monolith` usa el layout nativo. La sintaxis de abajo es la prevista.

```ofs
monolith Header layout packed {
    tag: stone
    flags: stone
}
```

Layouts soportados:

- `native`
- `packed`
- `c`

---

## 6. `namespace`

Use namespace para agrupar funciones y evitar colisiones de nombres.

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

## 7. Arrays y colecciones

```ofs
forge nums = [1, 2, 3]
echo(nums[0])
```

Ejemplo con paso de función:

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

La indexación es **base cero** y tiene **verificación de límites** en tiempo de ejecución:

```ofs
forge nums = [10, 20, 30, 40, 50]
forge primeiro = nums[0]   // 10
nums[2] = 99               // modifica un elemento
```

Fuera de límites (`nums[7]`, `ofs_array_get`/`ofs_array_set`), el programa se detiene con código 1 y un mensaje en stderr, igual en los dos modos:

```
ofs: indice 7 fora do limite do array (tamanho 5)
```

La comprobación vale en `ofs run`, en el `ofs build` predeterminado y en `ofs build --release`: el camino rápido en línea de los arrays (A6) ya compara el índice con el tamaño antes del acceso directo y cae en el runtime cuando está fuera de límites. Costo medido (A5.4, `bench/`): ninguna diferencia perceptible en los 6 programas del benchmark (sort 14 × 13 ms); en un bucle que solo lee y escribe `a[k]` (100 millones de accesos) 137 × 126 ms (~9%). Por eso el release mantiene la comprobación (el IR del release no cambia).

---

## 8. Control de flujo

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

Forma tipo C:

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

### Selección de múltiples caminos

La selección de múltiples ramas se realiza mediante cadenas de `if / else if`:

```ofs
if (code == 200) {
    echo("ok")
} else if (code == 404) {
    echo("nao encontrado")
} else {
    echo("erro")
}
```

O con `match` (sinónimo: `assay`), que compara el valor con cada caso usando `==`; `_` es el caso por defecto:

```ofs
match (code) {
    200 => { echo("ok") }
    404 => { echo("nao encontrado") }
    _ => { echo("erro") }
}
```

### Manejo de errores e interrupción (`throw`)

La instrucción `throw` muestra un mensaje de diagnóstico y termina la ejecución con código de salida 1:

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

> **Nota**: La sintaxis `tremor { ... } catch (e: obsidian) { ... }` es reconocida por la gramática (`NK_TREMOR`), con desenrollado estructurado de pila y captura en tiempo de ejecución previstos para las capas del intérprete y la evolución del runtime.

### `strata` (enumeraciones de variantes)

```ofs
strata Status { Idle, Running, Failed }
```

> **Nota**: una variante vale una etiqueta numérica (`v as stone`) y se puede guardar en un tipo `strata` (`forge c: Status = Idle`, parámetros, retorno, `Array<Status>`). `echo(c)` imprime el nombre (`Idle`) en los dos modos. El payload de `Ok(7)` todavía no se guarda en el nativo (ver propagación `?`).

### Propagación `?`

`expr?` desenvuelve una variante de resultado:

- `Solid(v)?` y `Ore(v)?` valen `v`.
- `Crack(e)?` y `Barren?` salen de la `vein` actual de inmediato (los `anchor` pendientes se ejecutan), propagando el fallo. Las variantes de fallo son las llamadas `Crack` y `Barren`; cualquier otra cuenta como éxito.

El valor propagado es igual en los dos modos:

- En la compilación nativa las variantes son etiquetas `i64` sin payload: la etiqueta es la posición de la variante en el orden de declaración de todas las `strata` del programa (con `strata Outcome { Solid, Crack }` declarada primero, `Crack` = 1). Una `vein ... -> stone` que propaga `Crack` devuelve esa etiqueta, y `echo` imprime `1` en los dos modos. Con retorno `obsidian` se obtiene el texto de la etiqueta; con `crystal`, el número; en `core main` se convierte en el código de salida.
- En el intérprete, si el tipo de retorno declarado es el propio tipo `strata` (`-> Outcome`), se devuelve la variante entera (con payload).
- Como el código nativo no guarda payload, `x?` sobre una expresión que no es un constructor directo (p. ej. `f()?`) vale la etiqueta de la variante de éxito, no su payload.

```ofs
strata Outcome { Solid, Crack }

vein ok() -> stone {
    forge v = Solid(7)?      // 7
    return v
}

vein bad() -> stone {
    forge v = Crack("boom")? // sale aquí devolviendo 1
    return v
}

core main() {
    echo(ok())   // 7
    echo(bad())  // 1
}
```

Prueba: `ofs/tests/regress/question_propagation.ofs`.

### `break` y `continue`

```ofs
cycle (forge i = 0; i < 100; i++) {
    if (i == 50) {
        break       // sale del bucle
    }
    if (i % 2 == 0) {
        continue    // salta los números pares
    }
    echo(i)
}
```

`break` y `continue` funcionan en `cycle` y en `while`. `while` es azúcar sintáctico para un `cycle` solo con condición.

### `seal` y `anchor` (ejecutar al salir del ámbito)

`seal { … }` y `anchor { … }` son **lo mismo** (el mismo token): registran una acción que se ejecuta cuando termina el ámbito actual, en cualquier salida (fin del bloque, `return`, `break`/`shatter`, `continue`/`erode`, propagación `?`). Varias acciones se ejecutan en orden inverso (la última registrada se ejecuta primero). `seal` es el nombre temático ("sella el recurso al salir"); `anchor` sigue siendo válido. Aceptan un bloque o un solo comando.

```ofs
vein copiar(origem: obsidian) -> stone {
    forge f = fopen(origem, "r")
    seal { fclose(f) }          // se ejecuta en cualquier return de abajo
    anchor echo("fim de copiar") // igual que seal; se ejecuta antes de fclose (LIFO)
    if (f == null) { return 1 }
    // ...
    return 0
}
```

- La acción se evalúa **en la salida**, con los valores de ese momento.
- `return`, `break`, `continue` y `throw` **dentro** de `seal`/`anchor` están prohibidos (error `E0042`).
- Funciona igual en nativo y en `ofs run`. Prueba: `ofs/tests/regress/seal_defer.ofs`.

---

## 9. Operadores y casts

### Operadores principales

- aritméticos: `+`, `-`, `*`, `/`, `%`
- comparación: `==`, `!=`, `<`, `<=`, `>`, `>=`
- lógicos: `&&`, `||`, `!`
- bit a bit: `&`, `|`, `^`, `<<`, `>>`

### Cast con `as`

```ofs
forge r: u8 = 255
forge x: u32 = r as u32
forge y: stone = x as stone
forge z: u8 = y as u8
```

También funciona en conversiones entre entero y flotante:

```ofs
forge c: crystal = 10.5
forge n: u8 = c as u8
```

### Precedencia de operadores (de menor a mayor)

| Nivel | Operadores | Asociatividad |
|---|---|---|
| 1 | `\|\|` | Izquierda |
| 2 | `&&` | Izquierda |
| 3 | `==` `!=` | Izquierda |
| 4 | `<` `<=` `>` `>=` | Izquierda |
| 5 | `\|` | Izquierda |
| 6 | `^` | Izquierda |
| 7 | `&` | Izquierda |
| 8 | `<<` `>>` | Izquierda |
| 9 | `+` `-` | Izquierda |
| 10 | `*` `/` `%` | Izquierda |
| 11 | Unarios: `-` `!` `&` `*` `++` `--` `~` | Derecha |

### Casts permitidos

| De | A | Resultado |
|---|---|---|
| `stone` | `crystal` | Entero a float |
| `crystal` | `stone` | Float a entero (trunca) |
| `bool` | `stone` | `true` → 1, `false` → 0 |
| `stone` | `bool` | 0 → `false`, distinto de cero → `true` |
| `bool` | `crystal` | `true` → 1.0, `false` → 0.0 |
| `crystal` | `bool` | 0.0 → `false`, distinto de cero → `true` |

Las conversiones de cadena (`obsidian`) requieren funciones de runtime (`ofs_stone_to_obsidian` etc.).

---

## 10. `attach`

### Biblioteca estándar o paquete instalado

```ofs
attach {core}
attach {window}
attach {terminal-colors}
```

### Archivo local

```ofs
attach {F:modulos/minha_lib.ofs}
```

En Windows también funciona con ruta absoluta:

```ofs
attach {F:G:\projeto\minha_lib.ofs}
```

`attach` incrusta el código OFS del otro archivo en el proceso de compilación. No carga ninguna DLL nativa.

### Dónde busca `attach`

Gana la primera opción que exista:

| Forma | Orden de búsqueda |
|---|---|
| `attach {modulo}` | 1. `$OFS_STDLIB_PATH/modulo.ofs` · 2. la carpeta del archivo que hace el `attach` · 3. cada carpeta de `$OFS_LIB_PATH` (separadas por `:`; en Windows por `;`, como el `PATH`, porque `:` aparece en la letra de unidad `C:\libs`) · 4. `$OFS_LIB_HOME`, o `~/.ofs/libs` (donde instala `src/packages/install_library.sh`) |
| `attach {F:ruta}` | 1. ruta absoluta, o relativa a la carpeta del archivo · 2. `$OFS_STDLIB_PATH/ruta` · 3. y 4. como arriba |

En `attach {modulo}`, el guion se vuelve guion bajo (`terminal-colors` → `terminal_colors.ofs`). Un módulo que no existe en ninguno de esos lugares es **error de compilación**, con la lista de las rutas probadas:

```
[ERRO DE ATTACH app.ofs:2]: modulo 'graficos' nao encontrado
  procurado em (nesta ordem):
    /usr/lib/ofs/stdlib/graficos.ofs
    ./graficos.ofs
    /home/voce/.ofs/libs/graficos.ofs
```

(Antes, `attach {x}` de un módulo inexistente se ignoraba en silencio.)

Un error de sintaxis dentro de un archivo adjunto indica el archivo y su propia línea: `[ERRO SINTATICO lib/util.ofs:4:11]: expected ')'` (antes salía solo `4:11`, la línea del fuente ya expandido).

### Nombres indefinidos

Usar un nombre que no fue declarado (variable, global, constante) es **error de compilación**, en los dos modos, con el archivo y la línea reales y una sugerencia cuando hay un nombre parecido:

```
[ERRO SEMANTICO calc.ofs:5:12]: identificador indefinido 'contadr'
  voce quis dizer 'contador'?
```

Los globales de nivel superior pueden usarse en una `vein` antes de la línea que los declara. Un módulo verificado solo (`ofs check modulo.ofs`) no ve los nombres de quien lo adjunta: verifique el archivo principal. Los tipos incompatibles en una operación (`[AVISO DE TIPO]`) siguen siendo solo un aviso.

---

## 11. `extern vein` y `rift vein`

El **Rift** (la grieta) es la frontera por la que OFS alcanza las bibliotecas nativas del sistema (libc, libm, X11, JNI de Android, kernel32). `extern vein` y `rift vein` declaran una función que vive en una biblioteca nativa. No hay código C en OFS. (La convención de llamada del sistema se conoce técnicamente como ABI C; no involucra código C.)

### `extern vein`

```ofs
extern vein puts(s: obsidian) -> stone
```

### `rift vein`

```ofs
rift vein ofs_window_create(title: obsidian, w: stone, h: stone) -> void abi c
```

### Metadatos de enlace

```ofs
rift vein text_size(s: obsidian) -> stone bind "strlen" abi c
```

### Dirección de función y llamada indirecta (`fault_addr`, `fault_call`)

La forma tipada de pasar una vein a C es el tipo de función (sección 4): declare el parámetro de la `rift vein` con la firma y pase el nombre de la vein, sin `abyss`:

```ofs
rift vein c_qsort(base: *stone, n: stone, size: stone, cmp: vein(*stone, *stone) -> stone) -> void bind "qsort" abi c

core main() {
    forge v: Array<stone> = [42, 7, 19]
    c_qsort(ofs_array_data(v), ofs_array_len(v), 8, crescente)   // crescente: abajo
}
```

El caso crudo sigue existiendo: obtener la dirección de una `vein` como `stone` y llamar a una dirección mediante la convención de llamada del sistema. Sirve para tablas de funciones nativas (JNI) y para código antiguo con el parámetro declarado como `stone` (`ANativeActivity`, `ALooper`, `AChoreographer`).

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
    // ...llena 8 valores...
    abyss {
        c_qsort(buf, 8, 8, fault_addr(crescente))
        forge r = fault_call(fault_addr(crescente), buf, fault_step(buf, 1))
    }
}
```

- `fault_addr(nome)` → `stone`: dirección de la vein. Acepta `vein` del módulo, `ns.vein` y `extern`/`rift vein` (con `bind`, devuelve el símbolo nativo). En IR: `ptrtoint ptr @simbolo to i64`. `fault_addr(f)` con `f` valor de vein vale `f as stone`. Otra cosa es un error semántico.
- `fault_call(endereco, args...)` → `stone`: llamada indirecta mediante la convención de llamada del sistema (`inttoptr` + `call i64`). Argumentos enteros, punteros y `obsidian`; el retorno se lee como `i64` (las funciones `void` devuelven basura, ignórela). Sin `float`/`double`. Cuando el primer argumento tiene tipo de función (`fault_call(f, a, b)` con `f: vein(...)`), la firma se verifica (aridad y argumentos, como en `f(a, b)`) y la llamada es la tipada, con el retorno `R`.
- La vein usada como callback debe tener parámetros compatibles con la convención de llamada del sistema: `stone` para enteros (los enteros de 32 bits llegan con la parte superior indefinida; enmascárelos con `& 4294967295`) y `*stone` para punteros.
- En el intérprete (`ofs run`), `fault_addr` de un `extern`/`rift vein` devuelve la dirección nativa y `fault_addr` de una vein interpretada devuelve un trampolín nativo que reentra en el intérprete (el `qsort` de arriba funciona igual). Ver "En el intérprete (`ofs run`): el Rift".
- Prueba: `ofs/tests/runtime/test_fault_addr_qsort.ofs`.


Campos importantes:

- `bind "..."` define el símbolo nativo real
- `abi c` selecciona la convención de llamada del sistema
- `...` se acepta en una función variádica nativa

### En el intérprete (`ofs run`): el Rift

`ofs run` llama a las mismas funciones nativas de las bibliotecas del sistema que enlazaría la compilación nativa, sin código C y sin libffi:

- **Resolución al cargar.** Cada `extern vein`/`rift vein` llamada por el programa (o pasada a `fault_addr`) se convierte en una dirección con `dlsym(RTLD_DEFAULT, "símbolo")`, usando el `bind` cuando existe. Antes, el intérprete abre `libm` (la compilación enlaza `-lm`) y las bibliotecas de `OFS_FFI_LIBS` (`libfoo.so:/ruta/libbar.so`); si falta algún símbolo, prueba también `libX11`/`libXft` (`ofs build` enlaza ambas cuando existen). Un símbolo ausente da el mismo error del enlazador (`undefined reference to 'x'`) antes de ejecutar, con código de salida 1. Una `extern` declarada y nunca llamada no necesita existir (como en el enlace).
- **Runtime OFS.** `ofscc` se enlaza con `-rdynamic`, así que `ofs_alloc`, `ofs_echo_color` etc. también se encuentran. Las veins definidas en `stdlib/runtime/*.ofs` (incluidas con `attach`) ejecutan la versión nativa enlazada en `ofscc`, como los builtins, cuando la firma cabe en el Rift; `OFS_RUN_RUNTIME=interp` lo desactiva.
- **Conversión.** `stone`, `bool`, enteros (`i8`..`u64`, ajustados al ancho y al signo), punteros y `obsidian` (el `char*` del propio valor) van como `i64`; `crystal` va en registro FP. El retorno sigue el tipo declarado: `char*` pasa a `obsidian` sin copia (como en nativo, donde `obsidian` es el puntero; `NULL` pasa a `null`), `*T` pasa a la dirección (puede volver a la función nativa, compararse con `null`, convertirse con `as` y desreferenciarse), `monolith` al bloque y `Array` al propio `ofs_array`.
- **Trampolines.** La llamada pasa por `__ofs_ffi_dispatch`, generada por `llvmgen` dentro de `ofscc`: un caso con el tipo LLVM exacto por clase de firma (hasta 4 argumentos con cualquier mezcla `stone`/`crystal` y retorno entero o `crystal`; hasta 12 solo enteros; variádicas con 1 a 3 fijos y hasta 4 extras). Otra clase es un error claro.
- **Variádicas** (`printf`, `snprintf`, `open`...): las que `llvmgen` conoce por nombre; los extras van en el tipo declarado (o en el del valor), con la convención correcta (`%al` en x86-64).
- `fault_addr(extern)` devuelve la dirección nativa y `fault_call(direccion, ...)` llama con los argumentos en el tipo del valor (como en nativo). `fault_step` suma sobre un puntero nativo.
- **Memoria cruda (F2).** El intérprete corre nativo, así que un puntero es una dirección real. `*p` y `*p = v` leen y escriben con el ancho del tipo apuntado (`u8`/`i8`/`u16`/`i16`/`u32`/`i32`/`u64`/`i64`/`stone`/`f32`/`f64`/`crystal`/`bool` (4 bytes)/`obsidian`/`*T`), con el tipo estático del `typeck` (el mismo que usa el `llvmgen`); los enteros estrechos quedan con signo extendido (`iN`) o ceros (`uN`), como en el nativo. Un `p` nulo falla como en el nativo (dirección 0). `fault_memcpy`/`fault_memset` usan `memcpy`/`memset` reales, de cualquier tamaño; `fault_step(p, i)` suma `i * sizeof(*p)` con el tipo estático de `p`.
- **`as`** sigue la regla del `llvmgen`: mismo tipo LLVM solo cambia el tipo; `ptrtoint`/`inttoptr` (incluido `obsidian`, que es el propio `char*`); `fptosi`; `sitofp` desde `iN`/`stone` y `uitofp` desde `uN` (`(255 as u8) as crystal` da 255.0); `fpext`/`fptrunc` (`f32` redondea); `trunc` si el origen es más ancho, si no `sext` desde `iN` y `zext` desde `uN`/`bool` (`(0 - 1) as i8 as stone` da -1). `forge x: u8 = 300` y `x = v` en un `u8`/`i32`/`f32` truncan como el `store` del nativo.
- **`&x`.** La dirección de una variable del intérprete es real: en el primer `&x` la variable pasa a vivir en un slot de memoria cruda de 8 bytes (la función nativa puede leerlo y escribirlo: `sscanf(s, "%d", &n)`, `strtol(s, &fin, 10)`, `modf(x, &entera)`), liberado cuando la variable sale de alcance, como la `alloca` del nativo. `&x` de un `monolith` es el propio bloque. Solo existe dirección de variable (el nativo tampoco tiene `&s.campo`/`&a[i]`: use `(&s) as stone + offset`); `&x` de una variable `Array` es un error claro.
- **`monolith` con layout del sistema.** Las instancias viven en memoria cruda con el layout de `llvmgen.struct_layout` (el mismo del código nativo: campos en orden, cada uno alineado a su propio tamaño, hasta 8; `bool` ocupa 4 bytes; `Array` y `monolith` anidado ocupan 8). `s` o `&s` pasado a una función nativa (`memcpy`, `qsort` de structs...) ve los mismos bytes que el build nativo. Como en el nativo, un `monolith` pasado a una vein es por referencia y `forge y: T = x`/`y = x` copian. Un campo `Array` guarda el array del intérprete (la función nativa no debe leerlo).
- **`Array` y strings.** Un `Array` pasado a una función nativa es el propio `ofs_array` nativo (cabecera `[data, len, cap, elem_size]` + datos) de la columna de valores del intérprete; la función nativa escribe los elementos en el lugar y la longitud se sincroniza a la vuelta. Elementos de un solo tipo (enteros/punteros, `crystal`, `bool`, `obsidian` o `monolith`); tipos mezclados son un error claro. Un `Array` devuelto por una función nativa usa el mismo bloque, sin copia. `obsidian` ya es el `char*`. Use `ofs_array_data(a)` para pasar solo los datos (ej.: `qsort`).
- **Callbacks nativos → OFS.** `fault_addr(vein)` de una vein interpretada liga la vein a uno de 16 trampolines nativos (`__ofs_cb_0`..`__ofs_cb_15`, generados por el `llvmgen` dentro del `ofscc`); la biblioteca llama al trampolín, que reentra en el intérprete, incluso dentro de una llamada nativa hecha por el propio programa (`qsort` dentro de `qsort`, `fault_call` de otro callback). La misma vein reutiliza su slot. Hasta 6 parámetros enteros, punteros, `obsidian`, `bool` o `monolith` (por puntero) y retorno entero/puntero. Un callback con `crystal`/`f32` (parámetro o retorno) usa otra tabla, `__ofs_cbx_0`..`__ofs_cbx_15`, que recibe también los registros de punto flotante (hasta 6 enteros y 8 `crystal`/`f32`; ola 4). Más de 16 veins distintas o parámetros más allá de esos límites dan un error claro con código 1 (use `ofs run --native`); en Windows x64 (registros por posición) el callback con `crystal`/`f32` sigue con error claro. El mismo trampolín sirve para un valor de vein pasado en un parámetro `vein(...)` (sección 4), y la cristalización usa los dos para volver a llamar veins interpretadas. `f32` en el Rift (parámetro y retorno, como `sqrtf`/`powf`) funciona en los dos modos.
- `fracture` y `abyss` no cambian la ejecución en los dos modos (el nativo genera el mismo código dentro de ellos); el `typeck` los exige para las operaciones de memoria cruda (ver sección 12, "Reglas"). Un `bedrock` que asigna memoria es también una arena (sección 12); en el intérprete, el `ofs_alloc` escrito en el bloque viene de la arena del runtime y lo que puede salir de ella se decide en compilación, igual que en el nativo. Las celdas y regiones del `bedrock` (stdlib) usan la memoria cruda de arriba.
- Pruebas: `ofs/tests/regress/ffi_*.ofs`, `ptr_widths.ofs`, `tests/parity/cases/` (`ptr_*`, `libc_*`, `monolith_memcpy`, `qsort_fault_addr`, `fault_call_basic`, `bedrock_block`).


### Dirección de la interoperabilidad

`extern vein` es la superficie actual del Rift, más adecuada para símbolos de runtime y bibliotecas nativas del sistema. El roadmap amplía esto a: fronteras con objetos y bibliotecas nativas, puentes hacia otros ecosistemas, embedding de Python y escapes de ensamblador específicos del destino. Esos ítems son roadmap, no recursos implementados hoy.

### Patrón híbrido

Es válido y recomendable mezclar alto y bajo nivel en el mismo archivo:

- lógica de negocio en `vein` comunes;
- llamada a bibliotecas nativas mediante el Rift (`rift vein`);
- trabajo cercano a la máquina en `bedrock`;
- almacenamiento explícito con `bedrock.ofs`.

---

## 12. Bloques de bajo nivel

### `fracture`

Capa de bajo nivel más tipada.

### `abyss`

Capa cruda y sin restricciones.

### `fractal`

Capa especial vinculada a intent/efectos.

### `bedrock`

Vocabulario de OFS para memoria, regiones, lanes, packets e intrinsics `fault_*`. Como bloque, libera las operaciones de bajo nivel (F4) y, cuando el propio bloque asigna memoria, es una **arena** (abajo).

Ejemplo:

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

Un bloque `bedrock { ... }` (también `tectonic bedrock { }` y `bedrock vein f() { }`, que vuelve todo el cuerpo un bloque) en el que **el propio código del bloque asigna memoria** es una arena: todo lo que asigna viene de una región que se libera de una vez a la salida del bloque, en **cualquier** salida (fin, `return`, `shatter`/`erode`, `?`), después de los `seal`/`anchor` del bloque. Sigue liberando las operaciones de bajo nivel de la F4. Un `bedrock` que no asigna nada es solo el bloque de bajo nivel (el mismo código de antes).

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
            ultimo = partes[0]          // el texto sale copiado
        }                               // toda la arena se libera aquí
        i++
    }
    echo(total)
    echo(ultimo)
}
```

- **Qué viene de la arena:** lo que el código escrito en el bloque asigna: concatenación y conversiones a texto, `[...]` y el crecimiento de un `Array` creado en el bloque, `forge m: Monolith`, `ofs_alloc`/`ofs_realloc` y los builtins de texto y de `Array` del runtime. `ofs_free` de un bloque de la arena no hace nada (sale con la región). La arena es **léxica**: una vein llamada desde dentro del bloque asigna en el heap normal (el compilador apaga la arena durante la llamada), y un `Array` de fuera que crece dentro del bloque sigue en el heap. Un `bedrock` dentro de otro abre su propia arena, que se cierra antes que la de fuera.
- **Escape (regla):** un valor que puede estar en la arena y va fuera de ella se trata así: **el texto `obsidian` se copia al heap** automáticamente (solo si está de verdad en una arena viva); **`Array`, `monolith`, puntero y geode** son `[ERRO DE ARENA archivo:línea:col]` en compilación. "Fuera" es: `return` desde dentro del bloque, asignación a una variable declarada fuera del bloque (local, parámetro o global), campo o elemento de un objeto de fuera, `push`/`set` en un `Array` de fuera, argumento de una vein que guarda el parámetro (análisis por vein, la misma idea de la A6), de un valor de vein o de una función desconocida, y el runtime que guarda el puntero (título de la ventana). Copiar un `monolith` solo con campos numéricos a una variable de fuera vale (es una copia por valor). Las funciones C (`extern`/`rift`) no guardan los argumentos (convención del C); si una los guarda, asigne fuera del bedrock. Dentro de `abyss { }` el error no se da: el programador asume el riesgo.

```
[ERRO DE ARENA app.ofs:5:17]: valor alocado na arena do bedrock e guardado fora dele (a arena e liberada na saida do bloco)
  dica: aloque fora do bedrock o que precisa sair dele (so texto obsidian e copiado para fora automaticamente), ou use abyss { } para assumir o risco
```

- **En los dos modos:** la regla se decide en compilación (`ofscc/sediment.ofs`, después del `typeck`), igual para el nativo, el intérprete, la cristalización y la Fault Refract. En el nativo, `ofs_arena_open()` en la entrada y `ofs_arena_close()` como anchor sintético; el runtime (`stdlib/runtime/reservoir.ofs`) saca los bloques de trozos de 64 KB que se duplican (en cero como el `calloc`) y guarda el mayor para la próxima arena. En el intérprete, el `ofs_alloc` escrito en el bloque viene de la misma arena del runtime y el texto copiado afuera pasa por la misma función (`ofs_arena_lift`).
- **Medida** (bucle de 200 mil vueltas con 16 textos en un `Array`, un `monolith` y 1 KB de `ofs_alloc` por vuelta, nativo `-O2`): con `bedrock` 1,9 MB y 0,38 s; el mismo bucle en un bloque común (con `ofs_free` del buffer) 258 MB y 0,37 s.

#### Aviso de fuga

Un `forge p = ofs_alloc(n)` (o `malloc`, `calloc`, `realloc`, `ofs_realloc`) fuera de una arena cuyo puntero nunca llega a un `seal`/`anchor`, `ofs_free`, llamada (dueño), `return`, asignación, campo, `Array` o geode genera un aviso (no error) en stderr, en los dos modos; un `ofs_alloc(n)` suelto, con el resultado descartado, también. `OFS_LEAK_WARN=0` lo silencia.

```
[AVISO DE VAZAMENTO app.ofs:3:24]: 'p' recebe ofs_alloc e nunca chega a um seal/ofs_free, return ou dono (o bloco vaza)
  dica: libere com seal { ofs_free(p) } logo depois, devolva ou guarde o ponteiro, ou aloque dentro de bedrock { } (a arena libera ao sair); OFS_LEAK_WARN=0 silencia
```

Pruebas: `ofs/tests/regress/bedrock_arena.ofs`, `arena_memory.ofs` (20 mil vueltas de 64 KB con `ulimit -v` de 256 MB en los dos modos), `arena_err_*.ofs`, `leak_warn.ofs`.

### Alias `tectonic`

El parser acepta alias textuales como:

- `tectonic fracture`
- `tectonic abyss`
- `tectonic fractal`
- `tectonic safe`
- `tectonic unsafe`
- `tectonic bedrock`

### Reglas de `fracture`, `bedrock`, `tectonic` y `abyss` (F4)

El `typeck` exige un bloque de bajo nivel para las operaciones que tocan memoria cruda. La regla es la misma en `ofs build` y en `ofs run` (viene del `typeck`):

| Operación | `fracture` / `bedrock` / `tectonic` | `abyss` |
|---|---|---|
| `*p` (leer) y `*p = v` (escribir) | libera | libera |
| `&x` | libera | libera |
| `as` con puntero (`n as *u8`, `p as stone`, `s as *u8`, `p as *T`) | libera | libera |
| `fault_step`, `fault_memcpy`, `fault_memset` | libera | libera |
| `fault_call`, `fault_addr` | — | libera |
| `as` entre valor de vein y dirección (`f as stone`, `n as vein(...)`) | — | libera |

- La aritmética de direcciones pasa por estas operaciones: `p + 1` con puntero ya es `[ERRO DE TIPO]`; use `fault_step(p, n)` o `((p as stone) + n) as *T`, ambos dentro del bloque.
- Siguen libres fuera de bloque: llamar `extern`/`rift vein` (incluso con punteros como argumento o retorno, p. ej. `forge buf = ofs_alloc(64)`), comparar un puntero con `null`, guardar y pasar punteros, los intrínsecos de bits (`fault_count`, `fault_cut`, `fault_weave`...) y todo lo de valores de vein tipados (usar el nombre de una vein como valor, guardarlo, pasarlo a C en un parámetro `vein(...)`, llamar `f(x)`).
- `tectonic X` vale como `X` (`tectonic abyss { }` = `abyss`); `tectonic { }` solo vale como `fracture`. Los bloques anidados se suman: dentro de `abyss`, todo está liberado.
- Se puede marcar la vein entera: `fracture vein leer(p: *u8) -> stone { return *p }`, `abyss vein llamar(f: stone) -> stone { return fault_call(f) }` (también en `namespace` e `impl`). El cuerpo vale como el bloque. El código que debe compilar con un `ofscc` anterior a esta regla (el propio compilador, por la semilla, y el stdlib) usa un bloque `fracture { }` dentro de la vein, que todo compilador entiende.
- En el árbol el bloque es un `NK_BLOCK` con el nombre en `val` (`"fracture"`, `"abyss"`...); el `llvmgen` y el intérprete lo ejecutan como un bloque común (mismo IR).

**Migración.** Hasta el **2027-01-09**, una operación fuera de bloque es solo un aviso, en stderr (en `ofs run` el stdout es del programa), y el programa compila igual:

```
[AVISO DE FALHA app.ofs:12:15]: acesso por ponteiro '*p' (leitura ou escrita) fora de bloco de baixo nivel (libera: fracture, bedrock ou abyss)
  dica: fracture { forge v = *p; *p = v + 1 }  ou marque a vein: 'fracture vein f(...)'
[AVISO DE FALHA] 1 operacao(oes) de baixo nivel fora de bloco: viram erro em 2027-01-09; OFS_STRICT_FAULT=1 ja trata como erro
```

Con `OFS_STRICT_FAULT=1` (el modo de las pruebas del repositorio) la misma línea pasa a `[ERRO DE FALHA archivo:línea:col]` y la compilación se detiene con código 1. A partir del 2027-01-09 el modo estricto será el predeterminado. El stdlib, el runtime, el compilador, las herramientas, los ejemplos y las pruebas ya compilan sin ningún aviso.

```ofs
core main() {
    forge x: stone = 10

    fracture {
        shard p: *stone = &x   // p apunta a x
        *p = 42
    }

    echo(x)  // imprime 42
}
```

Pruebas: `ofs/tests/regress/fault_blocks.ofs`, `fault_err_*.ofs`, `fault_warn_default.ofs`.

### Intrínsecos `fault_*`

Los de bits valen en cualquier lugar; `fault_step`, `fault_memcpy` y `fault_memset` piden `fracture`/`bedrock`/`abyss`, y `fault_addr`/`fault_call` piden `abyss` (tabla de arriba):

- `fault_count(stone)` → conteo de bits 1 (popcount)
- `fault_fence()` → barrera de memoria
- `fault_prefetch(*stone)` → precarga de memoria para el destino del puntero
- `fault_trap()` → trampa (trap) de la máquina
- `fault_lead(stone)` → ceros a la izquierda
- `fault_trail(stone)` → ceros a la derecha
- `fault_swap(stone)` → intercambio de bytes
- `fault_spin_left(stone, stone)` / `fault_spin_right(stone, stone)` → rotación de bits
- `fault_step(*stone, stone)` → avance de puntero tipado, en elementos
- `fault_addr(vein)` → dirección de la `vein` como `stone` (ver sección 11)
- `fault_call(endereco, args...)` → llamada indirecta mediante la convención de llamada del sistema (ver sección 11)
- `fault_cut(value, shift, width)` → extrae un campo de bits
- `fault_patch(base, shift, width, insert)` → reemplaza un campo de bits
- `fault_weave(mask, left, right)` → mezcla bits de `left` y `right` según `mask`
- `fault_unreachable()` → marca el camino como inalcanzable (genera `llvm.trap` + `unreachable`)
- `fault_memcpy(dst, src, len)` → copia de memoria en bloque (`llvm.memcpy`); `dst` y `src` son punteros
- `fault_memset(dst, val, len)` → relleno de memoria con el byte `val` (`llvm.memset`); `dst` es puntero

Semántica exacta (igual en el build nativo y en el intérprete `ofs run`):

- Todo en 64 bits, complemento a 2: `fault_count(-1)` = 64; `fault_lead(0)` = `fault_trail(0)` = 64; `fault_lead(-1)` = 0.
- `fault_spin_left`/`fault_spin_right`: rotación de 64 bits (`llvm.fshl`/`llvm.fshr`); la distancia se toma módulo 64 (`fault_spin_right(6, 65)` = 3).
- `fault_weave(mask, left, right)` = `(left & mask) | (right & ~mask)`: el bit viene de `left` donde `mask` tiene 1 y de `right` donde tiene 0.
- `fault_trap()` y `fault_unreachable()` terminan el proceso con SIGILL (el intérprete hace lo mismo con `raise(SIGILL)`).
- `fault_fence()`/`fault_prefetch()` no hacen nada en el intérprete.
- En el intérprete los punteros son direcciones reales: `fault_memcpy`/`fault_memset` copian/rellenan cualquier tamaño, y `fault_step`, `fault_addr` y `fault_call` funcionan como en nativo (ver "En el intérprete (`ofs run`): el Rift").
- Prueba: `ofs/tests/regress/fault_intrinsics.ofs`.

Estos intrínsecos cubren operaciones que el ensamblador ya tiene y le dan a OFS un vocabulario propio, orientado a la máquina.

### `quartz`: ensamblador en línea

`quartz` emite ensamblador directamente en el LLVM IR, con efecto secundario (`sideeffect`) por defecto. Solo se permite en `abyss { ... }` o `abyss vein`; `fracture` y `bedrock` solos no autorizan ensamblador. `asm` sigue siendo el comando CLI `ofs asm`, que genera un archivo `.s`.

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

- Sin salida: `quartz { "nop" }`. Con salida: `quartz { ... } -> variable "=r"`; declare antes la variable escalar. La restricción de salida es opcional (`=r` por defecto) y va en la misma línea que el destino. `+r` lee y escribe el destino; `=&r` marca una salida anticipada, que no puede compartir el registro de una entrada.
- Entradas: `in "r" expresion`, `in "{rdi}" expresion` o `in "m" variable` (memoria de una variable escalar). Una única salida escalar puede vincularse a un registro. `$0` es la salida, cuando existe; las entradas siguientes son `$1`, `$2`, etc. Sin salida, la primera entrada es `$0`.
- Clobbers: ponga `melt "rdx" "memory"` después de las instrucciones y entradas para declarar los registros y la memoria modificados. LLVM no descubre esos efectos por el texto; declare todos los efectos adicionales.
- El dialecto por defecto es AT&T; `quartz att { ... }` es explícito y `quartz intel { ... }` selecciona Intel. Un `$` literal se escribe `$$`: `"shlq $$32, %rdx"`.
- El texto es específico de la arquitectura. `OFSCC_TARGET` selecciona el destino, pero no traduce instrucciones x86-64 a AArch64/Android. Use ensamblador apropiado para el destino.
- En `ofs build`, LLVM compila el fragmento. En `ofs run`, la vein que contiene `quartz` se carga como código nativo en el mismo proceso; vea [Modelo de ejecución](COMPILED_LANGUAGE.md).

### Edición en vivo: `ofs refract` (Fault Refract)

`ofs refract app.ofs` ejecuta el programa interpretado y, mientras está abierto, cambia las veins que usted edita y guarda. La unidad de cambio es la **vein**:

- El cambio ocurre **entre llamadas**: la próxima llamada de la vein usa la versión nueva. Un **bucle** en ejecución en una vein editada (incluido el `while` del `main`) cambia en el próximo encabezado (**OSR**, ola 4): el bucle nuevo continúa con las variables locales como están, y el resto de la vein después de él también es el nuevo; una variable nueva antes del bucle recibe el valor inicial en el momento del cambio. Si una variable antes del bucle cambió de tipo (o el bucle está dentro de `cycle`/`match`/`bedrock`), el bucle termina en la versión antigua y la nueva vale en la próxima llamada. Fuera de un bucle, la llamada en ejecución termina en la versión antigua.
- El **estado** continúa: los globales existentes mantienen su valor (el inicializador no se ejecuta de nuevo), y los arrays y objetos vivos no cambian. Un global nuevo se crea con el valor inicial.
- Una vein **cristalizada** (nativa) o con `quartz` se recompila en un `.so` nuevo, junto con las veins nativas que la llaman; la versión nueva corre interpretada hasta que el `.so` esté listo (las que tienen `quartz`, que no corren interpretadas, se compilan antes del cambio).
- El programa nuevo completo se verifica antes (sintaxis y tipos): con un error, nada cambia.
- Una **firma compatible** cambia incluso con la vein en uso: parámetro nuevo al final con valor por defecto; retorno igual, que era `void`, entero estrecho donde era `stone`, `f32` donde era `crystal`.
- **Un `monolith` con instancias vivas migra**: un campo nuevo recibe el valor por defecto del tipo, un campo eliminado desaparece, el orden puede cambiar y un campo numérico puede cambiar de tipo (el valor se convierte). Las instancias se reescriben en el lugar.
- **Necesitan reinicio** (el cambio se rechaza y el programa sigue con la versión antigua): cambiar la firma de forma incompatible en una vein que una vein en ejecución todavía llama, o de una vein entregada a C como callback o usada como valor; eliminar una vein que algo en ejecución todavía llama; cambiar un campo de `monolith` vivo a un tipo que no convierte (p. ej. `stone` → `obsidian`) o hacer crecer el `monolith` más de 64 bytes; cambiar el tipo de un global; cambiar un `extern`; eliminar o reordenar variantes de `strata` (agregar al final sí se puede).

Ejemplo: `ofs/examples/refract_demo.ofs`. Comando, mensajes y variables: [TOOLS.md](TOOLS.md). Mecanismo: [COMPILER_ARCHITECTURE.md](COMPILER_ARCHITECTURE.md).

### Estrato y permisos por módulo (`stratum`, `claim`, `caprock`)

Seguridad gradual (Idea 4) y permisos por módulo (A5.4), en una sola regla. Cada archivo (el principal y cada `attach`) es un **módulo** y declara arriba, antes del código (comentarios, líneas en blanco y `attach` pueden ir antes o entre estas líneas), **cuánto** bajo nivel usa y **cuál**:

```ofs
// io_crudo.ofs
stratum fault          // opcional: fault es el predeterminado
claim rift, abyss      // lo que usa el módulo

extern vein getenv(name: obsidian) -> obsidian
```

| Estrato | Bajo nivel | Bloques | `claim` |
|---|---|---|---|
| `stratum crust` | ninguno: `*p`, `&x`, `as` con puntero, `fault_*` de memoria, `extern`/`rift vein`, `quartz` son `[ERRO DE ESTRATO]` | `fracture`/`tectonic`/`abyss` son error; `bedrock` vale solo como arena (no libera punteros) | ninguno (`claim` en un archivo `crust` es error) |
| `stratum fault` (predeterminado) | solo dentro del bloque que lo libera (regla de la F4, tabla de arriba) | como en la F4 | pide lo que usa |
| `stratum abyss` | en cualquier parte del archivo, sin bloque | opcionales | vale como `claim rift, abyss, quartz` |

| Permiso | Libera | Ejemplo |
|---|---|---|
| `claim rift` | declarar `extern vein` y `rift vein` (convención de llamada del sistema, `abi c`) | `extern vein fopen(...)` |
| `claim abyss` | bloque `abyss { }` y `abyss vein` | `abyss { forge r = fault_call(f) }` |
| `claim quartz` | `quartz { }` (que sigue exigiendo `abyss`, así que va junto con `claim abyss`) | `claim abyss, quartz` |
| `claim magma` | llamar a `magma.cast` (compilar y cargar código en ejecución; Idea 5). Sin él siempre es error | `claim magma` |

- `fracture`/`bedrock`/`tectonic` no piden claim: el estrato `fault` ya los acepta; el estrato `crust` los rechaza.
- `extern`/`rift vein` de un símbolo del runtime de OFS (nombre que empieza con `ofs_`, p. ej. `ofs_str_len`) no pide `claim rift`: es el propio lenguaje, no código C de afuera.
- **Quien adjunta ve lo que pide el módulo:** `ofs check` lista, por módulo, el estrato y los claims (`[usa sem claim: ...]` y `[claim sem uso: ...]` cuando no coinciden), y `ofs doc` muestra sellos `stratum`/`claim`/`caprock` en la página del módulo.
- **El principal restringe a los adjuntos** con `caprock` (solo en el programa principal; en un módulo es error): `caprock rift, abyss` deja que los adjuntos pidan como máximo eso; `caprock crust` no deja pedir nada. Un adjunto que pide, usa o es `stratum abyss` más allá del caprock es `[ERRO DE PERMISSAO principal:línea]` (siempre error: el caprock es opcional). El runtime (`stdlib/runtime/`) queda fuera del caprock; el propio principal también (sus claims valen para él).
- Relación con los bloques: el estrato dice **dónde** puede aparecer el bajo nivel (en ningún lugar / en bloques / en cualquier lugar), los bloques marcan **el tramo** y el claim dice **qué** poder usa el módulo y es lo que ve quien adjunta. `abyss` nombra el bloque, el permiso y el estrato más profundo: el bloque libera un tramo, `claim abyss` autoriza al módulo a tener ese bloque, `stratum abyss` hace del archivo entero un bloque.
- Forma de comentario: `//! claim rift` (y `//! stratum ...`) significa lo mismo. Es para código que también debe compilar con un `ofscc` anterior a la ola 5: el propio compilador (`ofs/ofscc/`), por la semilla `ofs/dist/ofscc`.
- El driver lee estas líneas como texto (como el `attach`) y no las pasa al lexer; una directiva después del código, una palabra desconocida (`claim lava`) o un `stratum` repetido son `[ERRO DE ESTRATO archivo:línea]`.

**Migración (igual que la F4).** Hasta el **2027-03-16**, el uso sin el claim es solo un aviso en stderr, un mensaje por módulo y permiso, y el programa compila igual:

```
[AVISO DE PERMISSAO app.ofs:3:17]: 'extern vein labs' sem 'claim rift' no topo do arquivo
  dica: acrescente a linha 'claim rift' no topo de app.ofs (quem anexa o modulo ve o que ele pede no ofs check)
[AVISO DE PERMISSAO] 1 uso(s) de baixo nivel sem 'claim' no topo do modulo: viram erro em 2027-03-16; OFS_STRICT_FAULT=1 ja trata como erro
```

Con `OFS_STRICT_FAULT=1` (la misma llave de la F4, modo de las pruebas del repositorio) pasa a `[ERRO DE PERMISSAO archivo:línea:col]`. Las violaciones de estrato (`crust`) y de `caprock` son siempre error, porque solo valen para quien las declara. El stdlib, el runtime, el compilador, las herramientas, los ejemplos y las pruebas del repositorio ya declaran lo que usan y pasan el modo estricto. Pruebas: `ofs/tests/regress/stratum_*.ofs`, `claim_*.ofs` y `stratum_mods/`.

### Compilar y cargar OFS en ejecución: `magma.cast` (Idea 5)

Un programa (en `ofs run` **y** en el binario de `ofs build`) compila un **mod** escrito en OFS mientras se ejecuta y llama a sus veins: plugins, scripts de juego, mods. El mod es un archivo OFS solo con veins (sin `core main`; el `attach` del stdlib vale). Módulo del stdlib: `attach {magma}`.

```ofs
attach {magma}
claim magma                       // cargar código en ejecución es un permiso

core main() {
    forge m = magma.cast("mods/forja.ofs")      // ruta, o el propio fuente (texto con \n)
    if (!magma.cooled(m)) {
        echo(magma.crack(m))                    // el mensaje del compilador; el programa sigue
        return
    }
    forge forjar: vein(stone) -> stone = magma.vent(m, "forjar")
    echo(forjar(10))
    forge erro: vein(crystal) -> stone = magma.vent(m, "forjar")
    if (erro == null) { echo(magma.crack(m)) }  // firma del mod distinta de la pedida
    magma.release(m)
}
```

- `magma.cast(fuente)` → handle `stone` (siempre ≥ 1). Un `fuente` con salto de línea es el propio código; si no, es la ruta del archivo. Los errores (sintaxis, tipo, permiso, carga) no tumban el programa: `magma.cooled(m)` es `false` y `magma.crack(m)` trae el mensaje del compilador.
- `magma.vent(m, "nombre")` → valor de vein **tipado** (F6): el tipo del lugar donde se guarda (`forge f: vein(T) -> R = ...`, `=`, argumento, `return`) es la firma pedida, comprobada en ejecución contra la declarada en el mod. Si no coincide o la vein no existe: `null` y el motivo en `magma.crack(m)`. Sin tipo de destino es `[ERRO DE TIPO]`.
- `magma.veins(m)` lista `nombre: firma`; `magma.release(m)` deja de entregar veins (el código **queda cargado**, misma cautela de la onda 4 de la Refract); `magma.caprock("crust")` es el techo de los próximos mods.
- **Globales:** el mod no ve los globales del programa ni al revés; la comunicación es por argumentos, retornos y valores de vein. El mismo fuente da el mismo `.so` y el mismo módulo; el mod editado es otro `.so` y la versión vieja sigue valiendo.
- **Seguridad:** `magma.cast` pide `claim magma` en el encabezado (siempre error, sin migración); en `stratum crust` es `[ERRO DE ESTRATO]`; `stratum abyss` vale como `claim magma`; un `caprock` sin `magma` impide anexar el módulo. El mod se compila en modo estricto y con el techo de `magma.caprock`.
- **Cómo funciona:** el `ofscc` compila el mod en un proceso aparte (`OFSCC_MODE=cast`: IR + manifiesto `@__magma_manifest`), clang genera el `.so` (`-shared -fPIC -O2 -Wl,-Bsymbolic`) en una caché por hash del IR (`OFS_MAGMA_CACHE` o `~/.cache/ofs/magma`) y se abre con `dlopen`. El mod usa el runtime del programa: `ofs run` ya está enlazado con `-rdynamic` y `ofs build` enlaza con `--export-dynamic` a quien usa `magma.cast`. Compilador: `OFS_MAGMA_OFSCC`, el propio `ofscc` en `ofs run`, o el instalado (como lo encuentra el wrapper); clang: `OFS_MAGMA_CC`. Linux y macOS.
- Ejemplo: `ofs/examples/magma_cast_demo.ofs`. Pruebas: `ofs/tests/regress/magma/` y `magma_err_*.ofs`.

---

## 13. Biblioteca estándar y paquetes

Módulos y paquetes relevantes que ya están presentes en el proyecto:

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

El módulo `fs` expone:

- `fs.exists(path: obsidian) -> bool`
- `fs.size(path: obsidian) -> stone`
- `fs.read(path: obsidian) -> obsidian`
- `fs.write(path: obsidian, content: obsidian) -> bool`
- `fs.append(path: obsidian, content: obsidian) -> bool`

El módulo `net` expone:

- `net.tcp_connect(host: obsidian, port: stone) -> stone`
- `net.tcp_send(fd: stone, data: obsidian) -> stone`
- `net.tcp_recv(fd: stone, max_len: stone) -> obsidian`
- `net.tcp_close(fd: stone) -> void`
- `net.tcp_listen(port: stone, backlog: stone) -> stone`
- `net.tcp_accept(server_fd: stone) -> stone`

El módulo `http` expone:

- `http.get(ip: obsidian, port: stone, host: obsidian, path: obsidian) -> HttpResponse`
- `http.parse_status_code(response_text: obsidian) -> stone`
- `http.extract_body(response_text: obsidian) -> obsidian`

El módulo `odl` expone constructores de HTML: cada función devuelve el fragmento como `obsidian`, y los fragmentos se anidan pasando uno como `children` del otro. El HTML es solo el artefacto generado; el fuente sigue en OFS.

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

El módulo `oes` expone constructores de CSS, en el mismo estilo: cada función devuelve el texto de la regla o declaración.

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

El módulo `oll` expone:

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

El módulo `canvas` expone:

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

El módulo `window` expone:

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

### Módulos de la biblioteca estándar

| Nombre | Descripción |
|---|---|
| `core` | Auxiliares de matemática, predicados, factorial, Fibonacci |
| `math` | Primos, utilidades de dígitos, inversión de números |
| `string` | Utilidades de cadenas: repeat, starts_with, is_empty |
| `io` | prompt, print_separator, print_header |
| `webserver` | Constructor de respuesta HTTP, tipos MIME, auxiliares de log |
| `bedrock` | Celdas, regiones y auxiliares de campos de bits en el heap |
| `bedrock-packet` | Auxiliares de layout de paquete sobre regiones `bedrock` |
| `rift` | Wrappers del Rift (auxiliares de biblioteca nativa) |
| `terminal-colors` | Salida de colores ANSI en la terminal |
| `memory-modes` | Documentación y auxiliares de los modos fracture/abyss/fractal |
| `test-lib` | Utilidades de pruebas unitarias |

### Funciones matemáticas (`stdlib/core.ofs`)

```ofs
vein abs(x: stone) -> stone
vein max(a: stone, b: stone) -> stone
vein min(a: stone, b: stone) -> stone
vein clamp(val: stone, lo: stone, hi: stone) -> stone
```

### Auxiliares de `bedrock.ofs`

- Celdas: `bedrock_cell_new`, `bedrock_cell_read`, `bedrock_cell_write`, `bedrock_cell_add`, `bedrock_cell_sub`, `bedrock_cell_drop`.
- Regiones: `bedrock_region_new`, `bedrock_region_read`, `bedrock_region_write`, `bedrock_region_add`, `bedrock_region_drop`, `bedrock_prefetch`.
- Vistas: `bedrock_view_at`, `bedrock_view_read`, `bedrock_view_write`.
- Campos y lanes: `bedrock_field_cut`, `bedrock_field_patch`, `bedrock_lane8_get/set`, `bedrock_lane16_le/be_get/set`, `bedrock_lane32_le/be_get/set`.
- Barrera: `bedrock_barrier`.

La finalidad de este módulo no es imitar APIs de otro lenguaje, sino hacer que la programación de bajo nivel crezca en el vocabulario propio de OFS. Los auxiliares de lanes dan una superficie con endianness para ventanas de bits orientadas a ABI, sin exigir desplazamientos manuales.

### Auxiliares de `rift.ofs` y `bedrock_packet.ofs`

- `rift.ofs`: `rift_text_size`, `rift_line`, `rift_banner`, `rift_report_line`. Permite consumir bibliotecas nativas del sistema mediante APIs en OFS.
- `bedrock_packet.ofs`: `bedrock_packet_store`, `bedrock_packet_header`, `bedrock_packet_payload`, `bedrock_packet_opcode`, `bedrock_packet_lane`, `bedrock_packet_opcode_be16`, `bedrock_packet_tail_le16`, `bedrock_packet_set_opcode`, `bedrock_packet_set_lane`. Vistas de encabezado y payload sobre regiones `bedrock`.

### Runtime nativo OFS (Stack Magma)

La implementación principal del runtime y de la biblioteca estándar está escrita en OFS (`stdlib/runtime/`), sin runtime en otro lenguaje:

| Subsistema | Módulo | Responsabilidad |
|---|---|---|
| **Magma** | `magma.ofs` | Bootstrap y orquestación de la ejecución del runtime nativo |
| **Reservoir** | `reservoir.ofs` | Asignación y gestión de memoria (`ofs_alloc`, `ofs_free`) |
| **Facet** | `facet.ofs` | Texto y conversión numérica (`ofs_str_*`, `ofs_*_to_*`) |
| **Matrix** | `matrix.ofs` | Colecciones dinámicas y vectores (`ofs_array_*`) |
| **Outlet** | `outlet.ofs` | Entrada y salida de la terminal (`echo_*`, colores ANSI, formato) |
| **Foundation** | `foundation.ofs` | Sistema operativo y entorno (`ofs_getenv`, `ofs_mod`, `ofs_exit`) |
| **Pulse** | `pulse.ofs` | Reloj monotónico de alta precisión y temporizadores (`now_ms`, `sleep_ms`) |
| **Frame** | `frame.ofs` | Gestión de ventanas y superficies nativas |
| **Impulse** | `impulse.ofs` | Eventos de entrada: ratón y teclado |
| **Prism** | `prism.ofs` | Rasterización 2D de píxeles, geometría y superficies |

Todas las primitivas de runtime son compiladas por `ofscc` directamente a LLVM IR nativo.

---

## 14. Comandos del compilador

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

### Compilador self-hosted (`ofscc`) en el estado actual

`ofscc` ya admite optimización y modos de inspección mediante un puente de variables de entorno:

Variables:

- `OFSCC_INPUT`
- `OFSCC_OUTPUT`
- `OFSCC_C_OUT`
- `OFSCC_OPT` (`-O0`, `-O2`, `-O3`)
- `OFSCC_MODE` (`check`, `tokens`, `ast`)

Ejemplo:

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

Paquetes:

```bash
uncover fmt
infuse fmt
infuse {fmt:stable}
reinfuse fmt
```
