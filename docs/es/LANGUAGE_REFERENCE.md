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

> **Nota**: Las expresiones anónimas de función (`vein(...) -> ...` en línea) están especificadas en el árbol sintáctico (`NK_LAMBDA`) y planificadas para la hoja de ruta de evolución del lenguaje. Actualmente, todas las funciones deben declararse con nombre mediante `vein`.

---

## 5. `monolith`, `impl` y layouts

### Estructura

```ofs
monolith Rect {
    w: stone
    h: stone
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

> **Nota**: `match` y `case` están reservados en la gramática y en el AST (`NK_MATCH`), con soporte completo en el parser previsto en la hoja de ruta de evolución del compilador.

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

> **Nota**: `strata` se analiza sintáctica y semánticamente, con generación directa para el backend LLVM prevista en la hoja de ruta de tipos algebraicos.

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

---

## 11. `extern vein` y `rift vein`

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

OFS no tiene un tipo "puntero a función", pero puede obtener la dirección de una `vein` como `stone` y llamar a una dirección con la ABI de C. Sirve para callbacks de bibliotecas C (`qsort`, `ANativeActivity`, `ALooper`, `AChoreographer`) y para tablas de funciones de C (JNI).

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
    c_qsort(buf, 8, 8, fault_addr(crescente))
    forge r = fault_call(fault_addr(crescente), buf, fault_step(buf, 1))
}
```

- `fault_addr(nome)` → `stone`: dirección de la vein. Acepta `vein` del módulo, `ns.vein` y `extern`/`rift vein` (con `bind`, devuelve el símbolo de C). En IR: `ptrtoint ptr @simbolo to i64`. Fuera de una vein, es un error semántico.
- `fault_call(endereco, args...)` → `stone`: llamada indirecta con la ABI de C (`inttoptr` + `call i64`). Argumentos enteros, punteros y `obsidian`; el retorno se lee como `i64` (las funciones `void` devuelven basura, ignórela). Sin `float`/`double`.
- La vein usada como callback debe tener parámetros compatibles con la ABI de C: `stone` para enteros (los enteros de 32 bits llegan con la parte superior indefinida; enmascárelos con `& 4294967295`) y `*stone` para punteros.
- Solo en la compilación nativa (`ofs build`); el intérprete (`ofs run`) no implementa `fault_*`.
- Prueba: `ofs/tests/runtime/test_fault_addr_qsort.ofs`.


Campos importantes:

- `bind "..."` define el símbolo nativo real
- `abi c` define la ABI de C
- `...` se acepta en una función variádica nativa

---

## 12. Bloques de bajo nivel

### `fracture`

Capa de bajo nivel más tipada.

### `abyss`

Capa cruda y sin restricciones.

### `fractal`

Capa especial vinculada a intent/efectos.

### `bedrock`

Vocabulario de OFS para memoria, regiones, lanes, packets e intrinsics `fault_*`.

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

### Alias `tectonic`

El parser acepta alias textuales como:

- `tectonic fracture`
- `tectonic abyss`
- `tectonic fractal`
- `tectonic safe`
- `tectonic unsafe`
- `tectonic bedrock`

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

### Precedencia de operadores (de menor a mayor)

| Nivel | Operadores          | Asociatividad |
|-------|---------------------|---------------|
| 1     | `\|\|`              | Izquierda     |
| 2     | `&&`                | Izquierda     |
| 3     | `==` `!=`           | Izquierda     |
| 4     | `<` `<=` `>` `>=`   | Izquierda     |
| 5     | `|`                 | Izquierda     |
| 6     | `^`                 | Izquierda     |
| 7     | `&`                 | Izquierda     |
| 8     | `<<` `>>`           | Izquierda     |
| 9     | `+` `-`             | Izquierda     |
| 10    | `*` `/` `%`         | Izquierda     |
| 11    | Unarios: `-` `!` `&` `*` `++` `--` `~` | Derecha |

---
## Control Flow

### if / else

```ofs
if (condition) {
    // then block
}

if (condition) {
    // then block
} else {
    // else block
}

if (x > 100) {
    echo("big")
} else if (x > 50) {
    echo("medium")
} else {
    echo("small")
}
```

### cycle (bucle estilo C)

```ofs
cycle (forge i = 0; i < 10; i++) {
    echo(i)
}
```

**Sintaxis:**
```
cycle (<init>; <condition>; <step>) {
    <body>
}
```

### cycle (bucle por rango)

```ofs
forge nums = [1, 2, 3, 4, 5]
cycle (n in nums) {
    echo(n)
}
```

**Sintaxis:**
```
cycle (<variable> in <collection>) {
    <body>
}
```

### break / continue

```ofs
cycle (forge i = 0; i < 100; i++) {
    if (i == 50) {
        break       // salir del bucle
    }
    if (i % 2 == 0) {
        continue    // omitir los números pares
    }
    echo(i)
}
```

### while (bucle solo con condición)

```ofs
forge count: stone = 10
while (count > 0) {
    echo(count)
    count -= 1
}
```

**Sintaxis:**
```
while (<condition>) {
    <body>
}
```

`while` es azúcar sintáctico para un `cycle` con solo una condición. `break` y `continue` funcionan dentro de los bloques `while`.

---

## Conversión de tipos (as)

Use la palabra clave `as` para conversiones de tipo explícitas:

```ofs
forge x: stone = 42
forge y: crystal = x as crystal    // entero a flotante

forge pi: crystal = 3.14
forge rounded: stone = pi as stone // flotante a entero (trunca)

forge flag: bool = true
forge num: stone = flag as stone   // bool a entero (0 o 1)
```

### Casts permitidos

| Desde     | A         | Resultado               |
|-----------|-----------|-------------------------|
| `stone`   | `crystal` | Entero a flotante       |
| `crystal` | `stone`   | Flotante a entero (trunca) |
| `bool`    | `stone`   | `true` → 1, `false` → 0 |
| `stone`   | `bool`    | 0 → `false`, distinto de cero → `true` |
| `bool`    | `crystal` | `true` → 1.0, `false` → 0.0 |
| `crystal` | `bool`    | 0.0 → `false`, distinto de cero → `true` |

Nota: Los casts a cadena (`obsidian`) requieren funciones de runtime (`ofs_stone_to_obsidian`, etc.).

---

## Importaciones

Use `attach` para incorporar al ámbito módulos de la biblioteca estándar, paquetes o archivos locales.

### Attach de biblioteca (stdlib / paquete)

```ofs
attach {core}
attach {math}
attach {webserver}
attach {bedrock}
attach {terminal-colors}
attach {memory-modes}
```

El nombre dentro de `{}` corresponde a un módulo conocido de la biblioteca estándar o a un paquete instalado. El compilador resuelve el archivo automáticamente desde el directorio de la biblioteca estándar, `OFS_STDLIB_PATH` o `OFS_LIB_PATH`.

### Attach de archivo

```ofs
attach {F:helpers.ofs}
attach {F:../shared/utils.ofs}
```

Use el prefijo `F:` para referenciar un archivo `.ofs` concreto mediante una ruta relativa al archivo actual.

Las declaraciones de `attach` deben aparecer en el nivel superior, antes o entre otras declaraciones. El preprocesador las resuelve antes de que se ejecuten el lexer y el parser, por lo que cada módulo se incrusta exactamente una vez (la deduplicación es automática).

### Módulos de la biblioteca estándar

| Nombre | Descripción |
|------|-------------|
| `core` | Auxiliares matemáticos, predicados, factorial, fibonacci |
| `math` | Primos, utilidades de dígitos, inversión de números |
| `string` | Utilidades de cadenas: repeat, starts_with, is_empty |
| `io` | prompt, print_separator, print_header |
| `webserver` | Constructor de respuestas HTTP, tipos MIME, auxiliares de registro |
| `bedrock` | Celdas de heap de bajo nivel, regiones, auxiliares de campos de bits |
| `bedrock-packet` | Auxiliares de layout de paquetes sobre regiones de bedrock |
| `rift` | Envoltorios de runtime externo (auxiliares de ABI de C) |
| `terminal-colors` | Salida de colores ANSI en terminal |
| `memory-modes` | Documentación y auxiliares para los modos fracture/abyss/fractal |
| `test-lib` | Utilidades para pruebas unitarias |

---

## Funciones externas

Declare funciones externas de C para FFI (Foreign Function Interface):

```ofs
extern vein ofs_pow(base: crystal, exp: crystal) -> crystal
extern vein ofs_sqrt(val: crystal) -> crystal
extern vein ofs_str_len(s: obsidian) -> stone

core main() {
    forge result = ofs_pow(2.0, 10.0)
    echo(result)  // prints 1024
}
```

Todas las funciones del runtime de OFS pueden accederse mediante declaraciones `extern`. Esto permite programar a bajo y alto nivel en el mismo lenguaje.

### rift vein

OFS ahora también admite `rift vein` como forma de declaración orientada a la interoperabilidad nativa:

```ofs
rift vein strlen(text: obsidian) -> stone

core main() {
    echo(strlen("fault"))
}
```

`rift vein` se reduce al mismo pipeline de llamadas externas que `extern vein`, pero presenta la interoperabilidad como un límite nativo de OFS en lugar de enmarcar la característica solo como "extern C".

También puede añadir metadatos explícitos de interoperabilidad:

```ofs
rift vein text_size(text: obsidian) -> stone bind "strlen" abi c
```

- `bind "symbol"` define el nombre del símbolo de enlace externo.
- `abi c` declara la ABI prevista.
- `abi system` está reservado para llamadas en el límite de plataforma/sistema.

### Dirección de interoperabilidad

`extern vein` es la superficie FFI actual. Hoy en día es más adecuada para símbolos de runtime y compatibles con C.

La hoja de ruta amplía esto hacia capas de interoperabilidad nativa más ricas para:

- límites de objetos y bibliotecas de C,
- capas de puente de C++,
- puentes de host/runtime de C#,
- incrustación de Python o bindings de host,
- escapes de ensamblador específicos del destino.

Estos son elementos de la hoja de ruta, no todos son funciones implementadas hoy.

---

## Estructuras (monolith)

Las estructuras se definen con la palabra clave `monolith`:

```ofs
monolith Player {
    name: obsidian
    hp:   stone
    speed: crystal
}
```

También puede declarar un modo de layout explícito:

```ofs
monolith Header layout packed {
    tag: stone
    flags: stone
}
```

Modos de layout soportados actualmente:

- `layout native`
- `layout packed`
- `layout c`

`layout packed` se reduce a un layout de struct empaquetado de LLVM. `layout c` se acepta como intención explícita orientada a la ABI y actualmente se reduce igual que `native` hasta que se amplíe el subsistema completo de ABI/layout.

### Creación de instancias

```ofs
forge hero: Player
hero.name = "Obsidian Knight"
hero.hp = 100
hero.speed = 1.5
```

### Acceso a campos

Use la notación de punto para acceder a los campos:

```ofs
echo(hero.name)     // "Obsidian Knight"
echo(hero.hp)       // 100
hero.hp -= 10       // recibe daño
```

### Paso a funciones

```ofs
vein greet(p: Player) -> void {
    echo(p.name)
    echo(p.hp)
}

core main() {
    forge hero: Player
    hero.name = "Knight"
    hero.hp = 100
    hero.speed = 1.5
    greet(hero)
}
```

---

## Punteros (shard / fracture)

Las operaciones con punteros solo están permitidas dentro de bloques `fracture` (seguro), `bedrock` (de bajo nivel tipado) o `abyss` (inseguro).

### fracture (bloque de punteros seguro)

```ofs
core main() {
    forge x: stone = 10

    fracture {
        shard p: *stone = &x   // p apunta a x
        *p = 42                // desreferenciar y asignar
    }

    echo(x)  // prints 42
}
```

**Reglas clave:**
- Los punteros solo pueden crearse y usarse dentro de bloques `fracture`
- El compilador verifica los tipos de todas las operaciones con punteros
- `&x` toma la dirección de `x`
- `*p` desreferencia el puntero `p`

### Tipos de puntero

```ofs
shard p: *stone       // puntero a stone (i64*)
shard q: *crystal     // puntero a crystal (f64*)
shard s: *obsidian    // puntero a obsidian (i8**)
```

### bedrock (bloque de bajo nivel tipado)

`bedrock` es el bloque de bajo nivel nativo de OFS para trabajo tipado cercano a la máquina.

```ofs
core main() {
    forge x: stone = 10

    bedrock {
        shard p: *stone = &x
        *p = *p + fault_spin_left(1, 3)
    }

    echo(x)
}
```

Use `bedrock` cuando quiera control de bajo nivel e intrínsecos parecidos a los de la máquina, manteniendo las reglas tipadas de OFS.

---

## Colecciones (Arrays)

### Literales de array

```ofs
forge nums = [10, 20, 30, 40, 50]
forge names = ["Alice", "Bob", "Charlie"]
```

Todos los elementos deben ser del mismo tipo. El tipo del array se infiere a partir de los elementos.

### Indexación

```ofs
forge first = nums[0]    // 10
forge last = nums[4]     // 50
nums[2] = 99             // modificar elemento
```

La indexación de arrays es **basada en cero** y con **verificación de límites** en tiempo de ejecución.

### Iteración

```ofs
// Por rango
cycle (n in nums) {
    echo(n)
}

// Por índice
cycle (forge i = 0; i < 5; i++) {
    echo(nums[i])
}
```

---

## Memoria insegura (abyss)

El bloque `abyss` permite operaciones de memoria crudas e inseguras:

```ofs
abyss {
    // Raw memory access — no type checking
    // Use with extreme caution
}
```

**Reglas:**
- La verificación de tipos se relaja dentro de los bloques `abyss`
- Se permite el direccionamiento directo de memoria
- Úselo solo cuando sea absolutamente necesario
- Prefiera `fracture` para las operaciones con punteros

### Alias de directiva: tectonic

Puede usar `tectonic` como directiva de prefijo para los modos de memoria/efectos.
Es una sintaxis alias que se asigna al mismo comportamiento de runtime/semántico:

- `tectonic fracture { ... }` -> igual que `fracture { ... }`
- `tectonic safe { ... }` -> igual que `fracture { ... }`
- `tectonic abyss { ... }` -> igual que `abyss { ... }`
- `tectonic unsafe { ... }` -> igual que `abyss { ... }`
- `tectonic fractal { ... }` -> igual que `fractal { ... }`
- `tectonic bedrock { ... }` -> igual que `bedrock { ... }`

Ejemplo:

```ofs
core main() {
    forge x: stone = 10

    tectonic fracture {
        shard p: *stone = &x
        *p = 20
    }

    tectonic safe {
        shard q: *stone = &x
        *q = *q + 1
    }

    tectonic abyss {
        shard raw: *stone = &x
        *raw = *raw + 1
    }

    tectonic unsafe {
        shard raw2: *stone = &x
        *raw2 = *raw2 + 1
    }

    tectonic fractal {
        echo("effect-lifted mode")
    }

    tectonic bedrock {
        shard native: *stone = &x
        *native = *native + 1
    }
}
```

---

## Funciones integradas

### echo

Función de impresión polimórfica: funciona con cualquier tipo:

```ofs
echo(42)              // prints: 42
echo(3.14)            // prints: 3.14
echo("hello")         // prints: hello
echo(true)            // prints: true
```

`echo` selecciona automáticamente la función de impresión de runtime correcta según el tipo del argumento.

### Intrínsecos fault_*

Dentro de `bedrock`, `fracture` o `abyss`, OFS expone intrínsecos parecidos a los de la máquina con nombres nativos de OFS:

```ofs
bedrock {
    forge x: stone = 0xF0F0
    echo(fault_count(x))
    echo(fault_lead(x))
    echo(fault_trail(x))
    echo(fault_swap(x))
    echo(fault_spin_left(x, 4))
    echo(fault_spin_right(x, 4))
    echo(fault_weave(0xFF00, 0xAAAA, 0x5555))
}
```

Intrínsecos disponibles:

- `fault_count(stone)` -> conteo de población (bits a 1)
- `fault_fence()` -> emite una barrera de memoria a nivel de máquina
- `fault_prefetch(*stone)` -> solicita una captación previa a nivel de máquina para el destino de un puntero
- `fault_trap()` -> emite una trampa a nivel de máquina
- `fault_lead(stone)` -> cuenta los ceros a la izquierda
- `fault_trail(stone)` -> cuenta los ceros a la derecha
- `fault_swap(stone)` -> intercambio de bytes
- `fault_spin_left(stone, stone)` -> rotación a la izquierda
- `fault_spin_right(stone, stone)` -> rotación a la derecha
- `fault_step(*stone, stone)` -> avance tipado de punteros en elementos
- `fault_addr(vein)` -> dirección de la vein como `stone` (ver FFI)
- `fault_call(endereco, args...)` -> llamada indirecta con ABI de C (ver FFI)
- `fault_cut(value, shift, width)` -> extrae un campo de bits
- `fault_patch(base, shift, width, insert)` -> reemplaza un campo de bits
- `fault_weave(mask, left, right)` -> mezcla bits de `left` y `right` usando `mask`
- `fault_unreachable()` -> marca el camino actual como inalcanzable, se reduce a IR `llvm.trap` + `unreachable`
- `fault_memcpy(dst, src, len)` -> copia de memoria en bloque (se reduce a `llvm.memcpy`); `dst` y `src` deben ser punteros
- `fault_memset(dst, val, len)` -> relleno de memoria en bloque con el byte `val` (se reduce a `llvm.memset`); `dst` debe ser un puntero

Estas funciones están diseñadas para cubrir operaciones que el ensamblador ya tiene, y al mismo tiempo dar a OFS espacio para su propio vocabulario orientado a la máquina.

### asm (escape de ensamblador en línea) — planeado

> **Estado: planeado.** El `asm` en línea todavía **no está disponible**: el lexer no reconoce la palabra `asm` y el backend LLVM no genera `InlineAsm`. El ejemplo muestra el diseño previsto y **no compila hoy**. Por ahora use los intrínsecos `fault_*`.

Cuando ningún intrínseco `fault_*` cubre su necesidad, `asm` emitirá una cadena de ensamblador sin procesar directamente en ese punto del código. Solo se permitirá dentro de un bloque `bedrock`, `fracture` o `abyss`.

```ofs
bedrock {
    asm "nop"     // planeado
    asm "int3"    // planeado
}
```

**Restricciones:**
- Planeado: el argumento debe ser un literal de cadena.
- La sintaxis de operandos de entrada/salida (restricciones completas estilo AT&T) todavía no es analizable; use los intrínsecos `fault_*` cuando necesite enlazar operandos.
- Planeado: genera una llamada de ensamblador en línea sin valor de retorno en LLVM IR, reenviando la cadena de ensamblador textualmente.
- Planeado: la expresión `asm` se reducirá a `InlineAsm` de LLVM, despachada en el análisis semántico y en la generación de código. Todavía no está implementado.

---

## Comentarios

```ofs
// Comentario de una sola línea

/* Comentario
   de varias líneas */

forge x = 42  // comentario en línea
```

---

## Terminación de sentencias

Las sentencias terminan con **salto de línea** o **punto y coma**. Ambos son válidos:

```ofs
forge x = 10
forge y = 20

// O con punto y coma
forge x = 10; forge y = 20
```

Los saltos de línea dentro de `()`, `[]` y `{}` se ignoran (continuación implícita de línea):

```ofs
forge result = add(
    10,
    20
)
```

---

## Biblioteca estándar

La biblioteca estándar se divide entre auxiliares orientados a la aplicación y auxiliares nativos de bajo nivel de OFS en desarrollo.

Ejemplos:

- `terminal_colors.ofs` para la presentación en terminal
- `webserver.ofs` para auxiliares HTTP
- `memory_modes.ofs` para las descripciones de los modos de memoria
- `bedrock.ofs` para auxiliares de estado respaldado por heap de forma explícita, al estilo nativo de OFS
- `rift.ofs` para envoltorios de límite externo nativos de OFS
- `bedrock_packet.ofs` para vistas de paquete/cabecera construidas sobre regiones de bajo nivel

### Módulo Bedrock

`bedrock.ofs` es la semilla actual de la programación de bajo nivel nativa de OFS.

Proporciona auxiliares como:

- `bedrock_cell_new(initial)`
- `bedrock_cell_read(cell)`
- `bedrock_cell_write(cell, value)`
- `bedrock_cell_add(cell, delta)`
- `bedrock_cell_sub(cell, delta)`
- `bedrock_cell_drop(cell)`
- `bedrock_region_new(len)`
- `bedrock_region_read(region, index)`
- `bedrock_region_write(region, index, value)`
- `bedrock_region_add(region, index, delta)`
- `bedrock_prefetch(region, index)`
- `bedrock_region_drop(region)`
- `bedrock_view_at(region, start, offset)`
- `bedrock_view_read(region, start, offset)`
- `bedrock_view_write(region, start, offset, value)`
- `bedrock_field_cut(value, shift, width)`
- `bedrock_field_patch(base, shift, width, insert)`
- `bedrock_lane8_get(value, lane)`
- `bedrock_lane8_set(value, lane, insert)`
- `bedrock_lane16_le_get(value, lane)`
- `bedrock_lane16_be_get(value, lane)`
- `bedrock_lane16_le_set(value, lane, insert)`
- `bedrock_lane16_be_set(value, lane, insert)`
- `bedrock_lane32_le_get(value, lane)`
- `bedrock_lane32_be_get(value, lane)`
- `bedrock_lane32_le_set(value, lane, insert)`
- `bedrock_lane32_be_set(value, lane, insert)`
- `bedrock_barrier()`

El propósito de este módulo no es imitar directamente las APIs de C, sino dejar que la programación de bajo nivel crezca con el vocabulario de OFS.

Estos auxiliares de lanes también dan a OFS una superficie consciente del endianness para ventanas de bits orientadas a la ABI, sin obligar al usuario a recurrir a desplazamientos crudos en todas partes.

### Módulo Rift

`rift.ofs` es la primera superficie de interoperabilidad de la biblioteca estándar construida sobre `rift vein`.

Proporciona envoltorios como:

- `rift_text_size(text)`
- `rift_line(text)`
- `rift_banner(title)`
- `rift_report_line(label, value)`

El objetivo es que los límites externos puedan consumirse mediante APIs orientadas a OFS, en lugar de exponer solo declaraciones crudas por todas partes.

### Módulo Bedrock Packet

`bedrock_packet.ofs` es el primer módulo estructurado de bajo nivel con estilo de objetos construido sobre regiones de bedrock.

Proporciona auxiliares como:

- `bedrock_packet_store(region, slot, header, payload)`
- `bedrock_packet_header(region, slot)`
- `bedrock_packet_payload(region, slot)`
- `bedrock_packet_opcode(header)`
- `bedrock_packet_lane(header)`
- `bedrock_packet_opcode_be16(header)`
- `bedrock_packet_tail_le16(header)`
- `bedrock_packet_set_opcode(header, opcode)`
- `bedrock_packet_set_lane(header, lane)`

Esto forma parte de la dirección en la que la programación de bajo nivel de OFS adquiere sus propios objetos/módulos reutilizables, en lugar de exponer solo operaciones crudas.

### Patrón de ejemplo híbrido

Es válido y recomendable mezclar OFS de alto y bajo nivel en el mismo archivo:

- lógica de negocio en funciones `vein` normales,
- interoperabilidad externa en el límite mediante `rift vein`,
- trabajo cercano a la máquina en `bedrock`,
- auxiliares explícitos de almacenamiento de `bedrock.ofs`.

OFS incluye una biblioteca estándar escrita en el propio OFS (`stdlib/core.ofs`):

### Funciones matemáticas

```ofs
vein abs(x: stone) -> stone         // valor absoluto
vein max(a: stone, b: stone) -> stone  // máximo
vein min(a: stone, b: stone) -> stone  // mínimo
vein clamp(val: stone, lo: stone, hi: stone) -> stone  // limitar a un rango
```

### Runtime nativo de OFS (Stack Magma)

La implementación principal del runtime y de la biblioteca estándar de OFS está escrita en OFS (`stdlib/runtime/`), sin una implementación de runtime en otro lenguaje:

| Subsistema | Módulo | Responsabilidad |
|---|---|---|
| **Magma** | `magma.ofs` | Bootstrap y orquestador de ejecución del runtime nativo |
| **Reservoir** | `reservoir.ofs` | Asignación y gestión de memoria (`ofs_alloc`, `ofs_free`) |
| **Facet** | `facet.ofs` | Operaciones de texto y conversión numérica (`ofs_str_*`, `ofs_*_to_*`) |
| **Matrix** | `matrix.ofs` | Colecciones dinámicas y vectores (`ofs_array_*`) |
| **Outlet** | `outlet.ofs` | Entrada y salida de terminal (`echo_*`, colores ANSI, formato) |
| **Foundation** | `foundation.ofs` | Abstracciones del sistema operativo y del entorno (`ofs_getenv`, `ofs_mod`, `ofs_exit`) |
| **Pulse** | `pulse.ofs` | Reloj monotónico de alta precisión y temporizadores (`now_ms`, `sleep_ms`) |
| **Frame** | `frame.ofs` | Gestión de ventanas y superficies nativas |
| **Impulse** | `impulse.ofs` | Eventos de entrada: ratón y teclado |
| **Prism** | `prism.ofs` | Rasterización 2D de píxeles, geometría y superficies |

Todas las primitivas de runtime son compiladas directamente por `ofscc` a LLVM IR nativo, lo que garantiza que los programas de OFS se ejecuten sobre el propio runtime nativo sin depender de implementaciones en C.

---

## Palabras clave

Lista completa de palabras clave reservadas:

| Palabra clave | Token            | Propósito                  |
|------------|------------------|----------------------------|
| `core`     | `KW_CORE`        | Función de entrada         |
| `vein`     | `KW_VEIN`        | Definición de función      |
| `forge`    | `KW_FORGE`       | Declaración de variable    |
| `monolith` | `KW_MONOLITH`   | Definición de struct       |
| `fracture` | `KW_FRACTURE`   | Bloque de punteros seguro  |
| `abyss`    | `KW_ABYSS`      | Bloque de memoria insegura |
| `cycle`    | `KW_CYCLE`       | Bucle                      |
| `if`       | `KW_IF`          | Condicional                |
| `else`     | `KW_ELSE`        | Rama else                  |
| `return`   | `KW_RETURN`      | Retorno desde función      |
| `echo`     | `KW_ECHO`        | Impresión (integrada)      |
| `in`       | `KW_IN`          | Bucle por rango            |
| `shard`    | `KW_SHARD`       | Tipo puntero               |
| `true`     | `KW_TRUE`        | Booleano verdadero         |
| `false`    | `KW_FALSE`       | Booleano falso             |
| `null`     | `KW_NULL`        | Valor nulo                 |
| `break`    | `KW_BREAK`       | Salir del bucle            |
| `continue` | `KW_CONTINUE`    | Omitir iteración           |
| `stone`    | `KW_STONE`       | Tipo entero (i64)          |
| `crystal`  | `KW_CRYSTAL`     | Tipo flotante (f64)        |
| `obsidian` | `KW_OBSIDIAN`    | Tipo cadena (i8*)          |
| `bool`     | `KW_BOOL`        | Tipo booleano (i1)         |
| `void`     | `KW_VOID`        | Tipo void                  |
| `attach`   | `KW_IMPORT`      | Adjuntar módulo            |
| `while`    | `KW_WHILE`       | Bucle solo con condición   |
| `extern`   | `KW_EXTERN`      | Función externa            |
| `rift`     | `KW_RIFT`        | Declaración de interop nativa |
| `as`       | `KW_AS`          | Cast de tipo              |
| `match`    | `KW_MATCH`       | Coincidencia de patrones   |
| `case`     | `KW_CASE`        | Rama de match              |
| `default`  | `KW_DEFAULT`     | Rama de respaldo de match  |
| `const`    | `KW_CONST`       | Variable inmutable         |
| `strata`   | `KW_STRATA`      | Declaración de enum        |
| `tremor`   | `KW_TREMOR`      | Bloque de manejo de errores |
| `catch`    | `KW_CATCH`       | Manejador de errores       |
| `throw`    | `KW_THROW`       | Lanzar valor de error      |
| `tectonic` | `KW_TECTONIC`    | Prefijo de directiva de modo |
| `bedrock`  | `KW_BEDROCK`     | Bloque de bajo nivel tipado |
| `asm`      | `KW_ASM`         | Escape de ensamblador en línea (planeado, aún no disponible) |
