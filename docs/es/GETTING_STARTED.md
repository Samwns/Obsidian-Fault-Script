# Guía de inicio de OFS

OFS (Obsidian Fault Script) es un lenguaje de programación híbrido y self-hosted, que incluye un intérprete AST integrado para ejecución instantánea (`ofs run`) y un backend LLVM para generar ejecutables binarios nativos de alto rendimiento (`ofs build`).

El ecosistema incluye el compilador, la biblioteca estándar, herramientas de línea de comandos y los lenguajes de dominio web ODL y OES.

---

## 1. Instalación

### Paquetes precompilados

Hay instaladores oficiales disponibles para varias distribuciones y sistemas operativos:

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
  Ejecute el script instalador incluido en el release de GitHub.
- **Windows (x64)**:
  Ejecute el instalador `ofs-windows-x64-installer-v*.exe`.
- **VS Code**:
  Instale la extensión oficial `ofs-vscode-extension.vsix` para soporte de sintaxis, resaltado y recarga dinámica (*Go Live*).

### Compilación desde el código fuente (Bootstrap)

Para compilar el compilador directamente desde el repositorio:

```bash
git clone https://github.com/Samwns/Obsidian-Fault-Script.git
cd Obsidian-Fault-Script
bash ofs/bootstrap/scripts/bootstrap-minimal.sh
```

El binario del compilador se generará en `ofs/dist/ofs` (o `ofs/dist/ofscc`).

---

### Termux (Android)

OFS corre en el propio teléfono, dentro de Termux (aarch64).

**Instalar** (en Termux):

```bash
curl -fsSL https://raw.githubusercontent.com/Samwns/Obsidian-Fault-Script/main/src/packaging/termux/install_termux.sh | bash
# o, con el paquete local:  bash install_termux.sh ofs-termux-aarch64.tar.gz
```

El instalador ejecuta `pkg install -y clang`, descarga la última release `ofs-termux-aarch64.tar.gz`, la extrae en `$PREFIX` (`bin/ofs` y `lib/ofs/`) y prueba `ofs --version`.

**Usar:** `ofs run hello.ofs` (interpretado), `ofs build hello.ofs -o hello && ./hello` (nativo aarch64, enlaza con el `clang` de Termux). Las herramientas están en `$PREFIX/lib/ofs/tools/` (`ofs run $PREFIX/lib/ofs/tools/odlc.ofs`).

**Limitaciones:** sin ventanas X11 por ahora (ventanas `.oll`/`.oal`); generar un APK en el teléfono (`ofs build --target android`) queda para después.

## 2. Comandos de la línea de comandos (CLI)

El ejecutable `ofs` expone los siguientes subcomandos:

| Comando | Descripción |
|---|---|
| `ofs <archivo.ofs>` | Ejecuta el programa directamente (equivale a `ofs run`). |
| `ofs run <archivo.ofs> [--native]` | Ejecuta en el intérprete, que cristaliza solo las veins calientes a nativo (o, con `--native`, compila el programa entero en un temporal). Capítulo 12. |
| `ofs build <archivo.ofs> -o <binário>` | Compila el archivo a un ejecutable binario nativo vía LLVM (`--release` quita las comprobaciones de contrato en ejecución). |
| `ofs check <archivo.ofs>` | Valida la sintaxis y los tipos sin emitir código. |
| `ofs refract <archivo.ofs>` | Ejecuta y cambia en vivo las veins que edites y guardes; `--remote` lo hace en una app Android. Capítulo 13. |
| `ofs assay <archivo.ofs>` | Casos de borde de las veins con contrato (`bearing`/`settle`). Capítulo 14. |
| `ofs fossil <archivo.ofs>` | Graba una ejecución para reproducirla y volver en el tiempo (`ofs fossil replay`). Capítulo 19. |
| `ofs quarry <add\|list\|install\|...>` | Gestor de paquetes. Capítulo 19. |
| `ofs fmt` / `ofs polish` | Formatea el código (`--check` y `--diff` para CI). |
| `ofs test [carpeta]` | Ejecuta los `.ofs` que tengan un `.expected` al lado y compara la salida. |
| `ofs twin <archivo.ofs>` | Ejecuta interpretado y nativo, comparando las salidas. |
| `ofs doc [destino]` | Genera la documentación HTML a partir de los comentarios del código. |
| `ofs language [en\|pt\|es]` | Muestra o cambia el idioma de la CLI. |
| `ofs ir <archivo.ofs>` / `ofs asm` / `ofs tokens` / `ofs ast` | Artefactos de depuración: LLVM IR, ensamblador, tokens y AST. |
| `ofs ui <archivo.oll> [-o salida.ppm]` | Renderiza un layout OLL nativo como imagen. |
| `ofs clean` | Limpia los artefactos temporales de compilación. |
| `ofs version` / `ofs update` / `ofs help` | Versión, actualización a la última release y ayuda. |

Variables de entorno reconocidas por el launcher: `OFS_LANG` (idioma en/pt/es), `OFS_PROGRESS` (reporter fancy/plain/json), `OFS_VERBOSE` (logs de enlazado/compilación), `OFS_RUNTIME` y `OFS_STDLIB_PATH` (rutas del runtime y de la biblioteca estándar), `OFS_CRYSTALLIZE=0` (desactiva la promoción automática a nativo en `ofs run`) y `OFS_CONTRACTS=0` (quita las comprobaciones de contrato en ejecución). Referencia completa de las herramientas: [TOOLS.md](TOOLS.md).

---

## 3. Primer programa

Cree un archivo llamado `hello.ofs`:

```ofs
core main() {
    echo("Olá, Mundo via OFS!")
}
```

Ejecución directa en el intérprete AST:

```bash
ofs hello.ofs
# o: ofs run hello.ofs
```

O compilación a un ejecutable binario nativo:

```bash
ofs build hello.ofs -o hello
./hello
```

Salida:
```text
Olá, Mundo via OFS!
```

---

## 4. Tipos de datos

OFS es de tipado estático. El compilador ofrece inferencia de tipos en las declaraciones de variables locales (`forge`).

### Tipos fundamentales

- `stone`: Entero de 64 bits con signo.
- `crystal`: Punto flotante de 64 bits (IEEE 754).
- `obsidian`: Secuencia de caracteres UTF-8 (*string*).
- `bool`: Booleano (`true` o `false`).
- `void`: Ausencia de valor de retorno.

### Enteros de ancho fijo

- Sin signo: `u8`, `u16`, `u32`, `u64`
- Con signo: `i8`, `i32`

Ejemplo de declaración y conversión explícita:

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

## 5. Funciones

Las funciones se definen con la palabra clave `vein`. El punto de entrada nativo del programa es la función `core main()`.

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

Las funciones pueden componer llamadas entre sí e invocar otras rutinas:

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

> **Nota**: Las funciones anónimas que capturan variables son `geode (x: stone) -> stone { return x + base }` (closures; ver la sección "`geode`" de la [Referencia](LANGUAGE_REFERENCE.md)). El valor tiene el tipo `vein(stone) -> stone` y se pasa, guarda y llama como cualquier vein.

---

## 6. Estructuras de datos (`monolith`)

OFS usa la palabra clave `monolith` para definir tipos compuestos y bloques `impl` para asociar métodos:

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

## 7. Módulos e importaciones (`attach`)

La declaración `attach` importa módulos de la biblioteca estándar instalada o archivos locales:

```ofs
attach {math}
attach {string}
attach {io}
```

Para importar archivos locales relativos o absolutos, use el prefijo `F:`:

```ofs
attach {F:./modulos/auxiliar.ofs}
```

---

## 8. Control de flujo

### Condicionales (`if` / `else`)

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

### Bucles de repetición (`while`)

```ofs
core main() {
    forge i: stone = 0
    while (i < 5) {
        echo(i)
        i = i + 1
    }
}
```

### Selección de múltiples caminos

Las ramificaciones con múltiples condiciones usan `if / else if / else`:

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

Cuando todas las condiciones comparan el mismo valor, `match` (sinónimo: `assay`) es más corto; `_` es el caso por defecto:

```ofs
core main() {
    forge status_code = 404
    match (status_code) {
        200 => { echo("Éxito") }
        404 => { echo("No encontrado") }
        _ => { echo("Otro código") }
    }
}
```

### Manejo y validación de errores (`throw`)

La instrucción `throw` muestra un mensaje de fallo y termina el proceso con código de error 1:

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

> **Nota**: La estructura `tremor { ... } catch (...) { ... }` está modelada en la gramática (`NK_TREMOR`), con desenrollado estructurado de pila previsto para las próximas etapas del runtime y del intérprete.

---

## 9. El Rift: bibliotecas nativas del sistema (`rift` / `extern vein`)

El **Rift** (la grieta) es la frontera por la que OFS alcanza las bibliotecas nativas del sistema (libc, libm, X11, JNI de Android, kernel32). `rift vein` y `extern vein` declaran una función que vive en una biblioteca nativa, llamada con la convención de llamada del sistema. No hay código C en OFS: el compilador y el runtime están escritos en OFS.

> La convención de llamada del sistema se conoce técnicamente como ABI C (`abi c`); no involucra código C.

```ofs
extern vein puts(s: obsidian) -> stone
rift vein strlen(s: obsidian) -> stone bind "strlen" abi c

core main() {
    puts("Mensaje enviado con puts de libc")
}
```

Funciona igual en `ofs run` y en `ofs build`. Un módulo que declara `extern`/`rift vein` pide `claim rift` al principio (mira el capítulo 17, que también muestra callbacks y `f32`/`crystal` por el Rift).

---

## 10. Stack web: ODL y OES

El ecosistema OFS cuenta con dos lenguajes de dominio específico (DSL) dedicados a construir interfaces web estáticas y dinámicas:

### ODL (Obsidian Document Language)

Estructura semántica para documentos web compilada a HTML5:

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

Compilación de ODL (la herramienta `odlc` forma parte de la biblioteca estándar de herramientas y se ejecuta con el propio `ofs`):

```bash
# en el repositorio OFS (o con las tools instaladas en /usr/share/ofs/tools)
ODLC_INPUT=documento.odl ODLC_OUTPUT=public/index.html ofs run ofs/tools/odlc.ofs
```

### OES (Obsidian Effect Scripts)

Definición de estilos, variables, reglas responsivas y animaciones compilada a CSS:

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

Compilación de OES (mismo patrón, usando la herramienta `oesc`):

```bash
OESC_INPUT=estilo.oes OESC_OUTPUT=public/estilo.css ofs run ofs/tools/oesc.ofs
```

### Servidor HTTP nativo (`webserver`)

La biblioteca estándar incluye un servidor HTTP nativo integrado al runtime:

```ofs
attach {webserver}

core main() {
    forge html = "<h1>Servidor Nativo OFS</h1><p>Executando sem dependências externas.</p>"
    webserver.serve_html_forever(8080, html)
}
```

---

## 11. Recursos de bajo nivel

OFS admite manipulación directa de memoria y contextos específicos cuando es necesario:

- `fracture { ... }`: Bloque con contexto de punteros tipados.
- `abyss { ... }`: Frontera explícita para operaciones de bajo nivel de máquina.
- `bedrock`: Biblioteca estándar para asignaciones explícitas de celdas y regiones de memoria (y, como bloque, la arena del capítulo 15).

```ofs
attach {bedrock}

core main() {
    forge celula = bedrock_cell_new(42)
    echo(bedrock_cell_read(celula))
    bedrock_cell_drop(celula)
}
```

Los capítulos 15 y 17 muestran cómo hacerlo con seguridad (`seal`, arena `bedrock { }`, `fracture`, `abyss`, `stratum`/`claim`/`caprock`).

---

## 12. `ofs run` vs `ofs build`: rápido de escribir, rápido de ejecutar

Los dos comandos aceptan el mismo programa y dan la misma salida. La diferencia es cuándo ocurre el trabajo:

- **`ofs run app.ofs`** interpreta el programa al instante: no genera binario, ideal para escribir y probar.
- **`ofs build app.ofs -o app`** genera un ejecutable nativo (LLVM) que distribuyes y ejecutas sin el `ofs`.

No necesitas elegir `ofs build` solo por velocidad: `ofs run` **se acelera solo**. Cuenta las llamadas de cada `vein`; cuando una se "calienta" (1000 llamadas por defecto), se compila a código nativo en segundo plano (**cristalización**) y las llamadas siguientes la usan. Un bucle largo dentro de `main` también pasa a nativo en mitad de la ejecución. Sin `clang`, el programa sigue interpretado, solo que más lento.

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
ofs run fib.ofs                       # 2178309, cristaliza solo (unos 0,06 s aquí)
OFS_CRYSTALLIZE=0 ofs run fib.ofs     # 2178309, solo interpretado (unos 1,5 s aquí)
ofs build fib.ofs -o fib && ./fib     # 2178309, nativo
```

Usa `OFS_CRYSTALLIZE=0` para desactivar la promoción (útil para depurar) y `OFS_VERBOSE=1` para ver cuándo ocurre. Los tiempos varían según la máquina; las mediciones comparables están en [PERFORMANCE.md](PERFORMANCE.md). El modelo completo está en [COMPILED_LANGUAGE.md](COMPILED_LANGUAGE.md).

---

## 13. Edición en vivo: `ofs refract`

`ofs refract app.ofs` ejecuta el programa y observa el archivo. Cada vez que editas una `vein` y guardas, se cambia **con el programa abierto**, sin reiniciar y sin perder las variables globales.

```ofs
claim rift
extern vein usleep(us: stone) -> stone

vein msg() -> obsidian { return "hola" }

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
# hola 0
# hola 1
# (cambia "hola" por "ey" en el editor y guarda)
# ofs refract: 1 vein refratada (msg)
# ey 2
```

El bucle de `main` continúa donde estaba: solo cambia la `vein` editada. Si el archivo tiene un error, el diagnóstico aparece y la versión en ejecución sigue. Los cambios que no se pueden hacer en vivo (por ejemplo, cambiar el tipo de un global) se rechazan con un mensaje que dice qué requiere reinicio.

**En el celular (Android):** `ofs refract --remote app.oal` mantiene la app **nativa ejecutándose en el dispositivo** y envía solo las veins que editaste (por `adb`). Mira [ANDROID.md](ANDROID.md) para preparar la app y [TOOLS.md](TOOLS.md) para las reglas y variables. Referencia del lenguaje: [Edición en vivo](LANGUAGE_REFERENCE.md).

---

## 14. Contratos: `mineral`, `grain`, `bearing`, `settle`

Los contratos quitan del código el `if` de validación repetido. Un **`mineral`** es un tipo con regla (`grain`); una `vein` puede exigir algo a quien la llama (`bearing`) y prometer algo sobre el retorno (`settle`, donde `result` es el valor devuelto).

```ofs
mineral Edad = stone grain value >= 0 grain value <= 150

vein dividir(a: stone, b: stone) -> stone
    bearing b != 0
    settle result * b + a % b == a
{
    return a / b
}

core main() {
    forge i: Edad = 30         // comprueba la regla
    echo(dividir(17, 5))       // 3
    forge zero = 0
    echo(dividir(1, zero))     // viola el bearing
}
```

Salida: `3` y después, en stderr y con código de salida 1:

```text
ofs: [CONTRATO VIOLADO contrato.ofs:4:12]: bearing de 'dividir' falhou: b != 0 (b = 0)
```

Cuando la violación se puede probar en compilación (`dividir(1, 0)`, `forge i: Edad = 200`), el compilador avisa con `[ERRO DE CONTRATO]` antes de ejecutar.

- **`ofs assay archivo.ofs`** llama a las veins con contrato en los valores de frontera de las reglas y muestra qué pasa:

```text
$ ofs assay mitad.ofs
assay mitad(n: stone)
  --   mitad(-1): fora do bearing n >= 0
  ok   mitad(0)
  ok   mitad(1)
assay: 2 caso(s) ok em 1 vein(s); 1 fora das regras de entrada
```

- **`ofs build --release`** (u `OFS_CONTRACTS=0`) quita las comprobaciones de **ejecución** del binario; las de compilación se quedan.

Referencia: [Contratos y tipos con regla](LANGUAGE_REFERENCE.md) y [`ofs assay`](TOOLS.md).

---

## 15. Limpieza segura: `seal` y la arena `bedrock { }`

### `seal`: se ejecuta al salir del ámbito

`seal { ... }` (o `anchor`, es lo mismo) registra una acción que se ejecuta **cuando el ámbito termina, por cualquier camino**: fin del bloque, `return`, `shatter`/`erode`. Sirve para cerrar archivos, liberar memoria y deshacer estado sin repetir la limpieza en cada `return`.

```ofs
vein procesar(nombre: obsidian) -> stone {
    echo("abre " + nombre)
    seal { echo("cierra " + nombre) }
    if (nombre == "x") { return 1 }
    echo("trabajando en " + nombre)
    return 0
}

core main() {
    echo(procesar("a"))
    echo(procesar("x"))
}
```

```text
abre a
trabajando en a
cierra a
0
abre x
cierra x
1
```

Varios `seal` se ejecutan en orden inverso (el último registrado va primero). Referencia: [`seal` y `anchor`](LANGUAGE_REFERENCE.md).

### `bedrock { }`: una arena de memoria

Un bloque `bedrock { ... }` que **asigna** (texto, `Array`, `ofs_alloc`) usa una **arena**: todo lo que asigna se libera de una vez cuando el bloque termina. Es la forma más simple de procesar muchos datos temporales sin perder memoria.

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
            ultimo = partes[0]        // el texto sale copiado
        }                             // toda la arena se libera aquí
        i++
    }
    echo(total)    // 6890
    echo(ultimo)   // item999
}
```

Regla de oro: **el texto `obsidian` se copia** fuera de la arena automáticamente; un `Array`, `monolith` o puntero que "se escapa" del bloque es `[ERRO DE ARENA]` en compilación, con la pista de asignar fuera del bloque. También existe el aviso `[AVISO DE VAZAMENTO]` para un `ofs_alloc` que nunca se libera. Referencia: [Arena `bedrock { }`](LANGUAGE_REFERENCE.md).

---

## 16. Veins como valores: `vein(T) -> R` y `geode`

El nombre de una `vein` es un **valor** con tipo de función, escrito con la misma palabra: `vein(stone) -> stone`. Se puede guardar en una variable, `Array` o campo, pasar como argumento y devolver.

```ofs
vein doble(x: stone) -> stone { return x * 2 }
vein triple(x: stone) -> stone { return x * 3 }

vein aplicar(f: vein(stone) -> stone, x: stone) -> stone { return f(x) }

core main() {
    forge f: vein(stone) -> stone = doble
    echo(f(21))                 // 42
    echo(aplicar(triple, 4))    // 12
    forge ops: Array<vein(stone) -> stone> = [doble, triple]
    echo(ops[1](10))            // 30
}
```

Un **`geode`** es una vein sin nombre que **captura** variables del ámbito de fuera (lo que otros lenguajes llaman closure o lambda). La captura es una copia del valor en el momento en que nace el geode.

```ofs
vein sumador(base: stone) -> vein(stone) -> stone {
    return geode (x: stone) -> stone { return x + base }
}

core main() {
    forge mas10 = sumador(10)
    forge mas1 = sumador(1)
    echo(mas10(5))    // 15
    echo(mas1(5))     // 6
}
```

Referencia: [Tipos de función y valores de vein](LANGUAGE_REFERENCE.md) y [`geode`](LANGUAGE_REFERENCE.md).

---

## 17. Bajo nivel con barandillas

OFS deja tocar memoria cruda y ensamblador, pero separa **dónde** y **qué**, y cada módulo declara lo que usa.

### Bloques `fracture` y `abyss`

`*p` (leer/escribir por puntero) y `&x` (dirección) solo valen dentro de un bloque de bajo nivel. `fracture { }` es el bloque tipado para punteros; `abyss { }` es el crudo y sin restricciones (direcciones de función, `fault_call`, `quartz`).

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

Fuera del bloque, el mismo código es un error de compilación. Reglas completas: [Reglas de `fracture`, `bedrock`, `tectonic` y `abyss`](LANGUAGE_REFERENCE.md).

### `stratum`, `claim` y `caprock`: permisos por módulo

Al principio del archivo, el módulo declara su **estrato** (cuánto bajo nivel usa) y sus **claims** (cuál). Quien anexa el módulo ve lo que pide, y el programa principal puede imponer un techo con `caprock`.

```ofs
// io_crudo.ofs
claim rift                 // este módulo declara extern vein

extern vein getenv(name: obsidian) -> obsidian
vein casa() -> obsidian { return getenv("HOME") }
```

```ofs
// principal.ofs
caprock crust              // ningún módulo anexado puede pedir bajo nivel
attach {F:io_crudo.ofs}

core main() { echo(1) }
```

Al ejecutar el principal: `[ERRO DE PERMISSAO ...]: o modulo io_crudo.ofs pede 'rift', mas o caprock do programa principal libera so: nada (caprock crust)`. Y un archivo con `stratum crust` no acepta `&x` ni `extern`:

```text
[ERRO DE ESTRATO stratum.ofs:5:24]: endereco '&x' num arquivo 'stratum crust' (sem baixo nivel)
```

Referencia: [Estrato y permisos por módulo](LANGUAGE_REFERENCE.md).

### El Rift: funciones nativas del sistema

El **Rift** es la grieta por la que OFS llama a las bibliotecas nativas del sistema (libc, libm, X11, JNI de Android). No hay código C en OFS: el compilador y el runtime están escritos en OFS. Declara la función con `rift vein` (o `extern vein`) y pide `claim rift`. Funciona igual en `ofs run` y en `ofs build`.

```ofs
claim rift

rift vein strlen(s: obsidian) -> stone bind "strlen" abi c
extern vein puts(s: obsidian) -> stone

core main() {
    echo(strlen("obsidiana"))    // 9
    puts("escrito por la libc")
}
```

Una `vein` o un `geode` **sin captura** puede entregarse al sistema como callback (por ejemplo, el comparador de `qsort`); `f32` y `crystal` también cruzan el Rift. Referencia: [`extern vein` y `rift vein`](LANGUAGE_REFERENCE.md).

### `quartz`: ensamblador en línea

`quartz { ... }` escribe instrucciones de máquina dentro de la `vein`, en AT&T por defecto. Solo vale en `abyss` y es específico de la arquitectura (el ejemplo es x86-64).

```ofs
claim abyss, quartz

abyss vein sumar(a: stone, b: stone) -> stone {
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
    echo(sumar(20, 22))    // 42
}
```

En `ofs run`, la vein con `quartz` pasa a nativa automáticamente (necesita `clang` en la primera ejecución). Referencia: [`quartz`](LANGUAGE_REFERENCE.md).

---

## 18. Cargar OFS en ejecución: `magma.cast`

Un programa puede compilar un **mod** escrito en OFS mientras corre y llamar a sus veins: plugins, scripts de juego. Como entra código nuevo en el proceso, el archivo necesita `claim magma`.

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
        echo(magma.crack(m))       // mensaje del compilador
        return
    }
    forge forjar: vein(stone) -> stone = magma.vent(m, "forjar")
    echo(forjar(10))               // 101
    magma.release(m)
}
```

Un error en el mod no tumba el programa: `magma.cooled` da `false` y `magma.crack` trae el mensaje. Referencia: [`magma.cast`](LANGUAGE_REFERENCE.md). Ejemplo del repositorio: `ofs/examples/magma_cast_demo.ofs`.

---

## 19. Herramientas del día a día

Todas las herramientas de abajo son programas escritos en OFS que `ofs` compila la primera vez y guarda en caché. La lista completa está en [TOOLS.md](TOOLS.md).

### `ofs fmt` y `ofs polish`: formatear

Arregla sangría y espacios sin cambiar el programa. `ofs polish` es el mismo comando con otro nombre.

```bash
ofs fmt feo.ofs          # reescribe en el lugar
ofs fmt --diff feo.ofs   # solo muestra la diferencia
ofs fmt --check src/     # en CI: lista lo que cambiaría y sale con 1
```

```ofs
// antes                           // después
vein suma(n:stone)->stone {        vein suma(n: stone) -> stone {
  forge t=0                            forge t = 0
  ...                                  ...
```

### `ofs test` y `ofs twin`: comprobar

`ofs test [carpeta]` ejecuta cada `x.ofs` que tenga un `x.expected` al lado y compara la salida. `ofs twin x.ofs` ejecuta interpretado **y** nativo y confirma que dan el mismo resultado.

```bash
printf '55\n' > cuenta.expected    # la salida esperada de cuenta.ofs
ofs test .        # ok    cuenta.ofs  /  resumo: 1 ok, 0 falha(s), 1 teste(s)
ofs twin cuenta.ofs                # iguais: cuenta.ofs
```

### `ofs doc`: documentación del código

Comenta cada declaración con `//` justo encima y `ofs doc` genera un sitio de referencia (una página por módulo, búsqueda y cobertura).

```ofs
// Suma los n primeros números.
// @param n cuántos números sumar
// @returns la suma de 1 a n
vein suma(n: stone) -> stone { ... }
```

```bash
ofs doc cuenta.ofs -o docs_api   # genera docs_api/index.html
ofs doc --check                  # solo la cobertura y las funciones citadas en los .md que no existen
```

### `ofs fossil`: grabar una ejecución y volver en el tiempo

`ofs fossil` ejecuta el programa y graba, en un `.fossil`, todo lo que viene de fuera y cambia cada vez (reloj, aleatorio, entrada, archivos, red). `replay` repite la ejecución con los valores grabados, sin tocar el mundo, y deja **parar en cualquier punto**.

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
ofs fossil rel.ofs                      # 383, true  (graba rel.fossil)
ofs fossil replay rel.fossil            # 383, true  (lo mismo, siempre)
ofs fossil replay rel.fossil --to L7    # para en la línea 7 y muestra locales y globales
ofs fossil replay rel.fossil --dig      # modo interactivo, con paso hacia atrás
```

(El número aleatorio cambia en cada grabación; el replay devuelve siempre el grabado.)

### `ofs quarry`: paquetes

`ofs quarry` instala bibliotecas de la comunidad para que `attach` las encuentre sin configuración.

```bash
ofs quarry search colores            # busca en el registro
ofs quarry add terminal-colors       # instala y anota en ofs.claim / ofs.lode
ofs quarry list                      # lo que está instalado
ofs quarry install                   # reproduce el proyecto desde ofs.lode (comprueba el sha256)
```

```ofs
attach {terminal_colors}    // usa el paquete instalado (el guion pasa a guion bajo)
```

El manifiesto `ofs.claim` y la publicación (`ofs quarry publish`) están en [TOOLS.md](TOOLS.md).

---

## 20. Próximos pasos

- Consulte la [Referencia del lenguaje](LANGUAGE_REFERENCE.md) para detalles exhaustivos de la gramática y los tipos.
- Consulte la [Referencia ODL](ODL_REFERENCE.md) y la [Referencia OES](OES_REFERENCE.md) para desarrollo web.
- Consulte la documentación de la [Biblioteca estándar](STANDARD_LIBRARY.md).
- Vea todas las herramientas (`fmt`, `doc`, `refract`, `fossil`, `quarry`, ...) en [TOOLS.md](TOOLS.md), el modelo `ofs run`/`ofs build` en [COMPILED_LANGUAGE.md](COMPILED_LANGUAGE.md) y la app Android en [ANDROID.md](ANDROID.md).
- Los ejemplos probados de cada recurso están en `ofs/tests/regress/` y `ofs/examples/`.
