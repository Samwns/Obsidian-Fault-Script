# Herramientas de OFS

## `ofs doc`: documentación generada del código

`ofs doc` lee archivos `.ofs` y genera una referencia HTML estática (como `rustdoc` o `godoc`): una página por módulo, con tarjetas de función, búsqueda y un informe de cobertura. La herramienta está escrita en OFS (`ofs/tools/ofsdoc.ofs`) y funciona en cualquier proyecto OFS.

```bash
ofs doc                          # ofs/stdlib, ofs/tools y ofs/ofscc del repositorio -> docs_api/
ofs doc mi_carpeta -o docs_api/  # cualquier carpeta (recursivo) o un solo .ofs
ofs doc --check                  # solo verifica: imprime la cobertura, no genera HTML
```

Opciones: `-o <carpeta>` (salida, por defecto `docs_api`), `--check`, `--docs <carpeta>` (los `.md` del proyecto; por defecto `docs/` si existe), `--title <nombre>`, `--repo <URL de GitHub>` (enlaza "Fuente" a la línea en GitHub), `--lang pt|en|es` (idioma de la interfaz; por defecto `OFS_LANG`). Para verlo: `cd docs_api && python3 -m http.server 8765`.

En el sitio, `build_site.ofs` ejecuta el mismo generador en cada build y une la API con la Guía en una sola navegación en `docs/ref/` (Referencia).

### Qué muestra cada módulo

- `namespace`, `monolith` (campos y tipos), `impl` (métodos), `vein`, `rift vein` (sello **exportada · abi c**), `extern vein` (sello **externa: libc/NDK**), `core` y constantes/globales.
- Firma completa: parámetros, tipos y retorno. Los nombres de `monolith` en los tipos son enlaces.
- **Llama a** (funciones conocidas usadas en el cuerpo) y **Usado por** (lo inverso, en todo el código analizado).
- **Véase también**: secciones de los `.md` de `docs/` que citan la función o el módulo.
- **Fuente**: enlace a la línea en GitHub.
- **Estrato y permisos** (A5.4 + Idea 4): sellos `stratum ...`, `claim ...` y `caprock ...` arriba de la página del módulo, leídos de la cabecera del archivo (también en la forma `//! claim`); `stratum abyss` recibe el sello "pide todo". En `--dump`, una línea `modulo|stratum ...|claim ...` por módulo que declara algo.
- Las funciones que empiezan con `_` son internas: van en una sección plegada y no cuentan para la cobertura.

### Cómo comentar el código

El comentario de una declaración son las líneas `//` contiguas inmediatamente encima (sin línea en blanco en medio), o un bloque `/** ... */`. El primer bloque del archivo, separado por una línea en blanco, describe el módulo.

```ofs
// hashmap: tabla hash entero -> entero (comentario del módulo).
//
//   forge m = hashmap.new(64)
//   hashmap.set(m, 42, 7)

namespace hashmap {
    // Crea una tabla con capacidad inicial `cap`.
    //
    // El primer párrafo es el resumen (aparece en la búsqueda y los índices).
    // Las líneas con 2+ espacios de sangría se vuelven bloque de código;
    // `nombre` se vuelve enlace cuando existe en el código.
    // @param cap capacidad inicial (una potencia de 2 es mejor)
    // @returns el identificador de la tabla
    // @example
    //   forge m = hashmap.new(64)
    vein new(cap: stone) -> stone {
        return _alloc(cap)
    }
}
```

| Marca | Efecto |
|---|---|
| `@param nombre texto` | descripción en la tabla de parámetros |
| `@returns texto` | descripción del retorno |
| `@example` | el resto del comentario se vuelve bloque de código |
| `` `nombre` `` | enlace a la función, tipo o módulo, si existe |
| `// ──── título ────` / `// ----` | línea decorativa, ignorada |

Limitaciones: las llaves `{` de `namespace`, `impl`, `monolith` y del cuerpo de `vein` deben estar en la misma línea que la declaración (estilo del stdlib).

### Cobertura ("qué falta documentar")

La página **Cobertura** (y la salida de `ofs doc --check`) muestra:

1. cuántas funciones públicas (`vein`/`rift vein` sin `_`) tienen comentario, por módulo y en total, con el porcentaje y la lista de lo que falta;
2. las funciones **citadas en `docs/*.md` que no existen en el código**: `modulo.funcion(` cuando `modulo` es un módulo o namespace conocido, y un nombre con `_` citado entre comillas invertidas con paréntesis de apertura que no existe en ningún lugar. Los nombres `fault_*`, las funciones definidas como ejemplo en los bloques de código de los propios `.md` y las firmas de ejemplo (`` `f(x) -> T` ``) no cuentan.

`ofs doc --check` sale con código **1** si hay una función citada inexistente, para usar en CI:

```bash
ofs doc --check || exit 1
```

### Prueba

`ofs/tests/ofsdoc/run.sh` compara el extractor con `ofs/tests/ofsdoc/expected.txt` (nombres, firmas, comentarios, llama a/usado por) y verifica el código de salida de `--check`.

## `ofs fmt`: formateador

`ofs fmt` formatea código `.ofs` (herramienta escrita en OFS, `ofs/tools/ofsfmt.ofs`).

```bash
ofs fmt archivo.ofs     # reescribe en el sitio
ofs fmt src/            # carpeta, recursivo (solo .ofs); sin destino = carpeta actual
ofs fmt --check [rutas] # CI: lista lo que cambiaría y sale con código 1
ofs fmt --diff [rutas]  # muestra el diff, no escribe
ofs fmt --stdin         # lee la entrada estándar, escribe en la salida (editores)
```

Reglas: sangría de 4 espacios (las líneas de continuación dentro de `(`/`[` o tras un operador reciben +4), `{` en la misma línea, espacio alrededor de operadores binarios y después de `,` y `:`, como máximo una línea en blanco seguida, sin espacios al final de línea, un `\n` al final del archivo. Se mantienen los saltos de línea del autor, y los comentarios, las cadenas y las líneas `attach` nunca se modifican.

Garantías (probadas en `ofs/tests/fmt/run.sh`): formatear dos veces da el mismo archivo y el IR de `ofscc` del original y del formateado es idéntico (el programa no cambia). Pendiente: salto automático de líneas de más de 100 columnas y ordenación de `attach`.

En VS Code la extensión registra `ofs fmt --stdin` como formateador; para formatear al guardar: `"[ofs]": { "editor.formatOnSave": true }`.

## `ofs test`, `ofs twin`, `ofs polish`

```bash
ofs test [--run] [--twin] [carpeta]   # por defecto: ./tests
ofs twin archivo.ofs
ofs polish [--check] [--diff] [destinos]
```

- `ofs test`: busca los `*.ofs` con un `.expected` al lado, compila y ejecuta cada uno (nativo; `--run` usa el interpretado; `--twin` también compara ambos modos) y compara la salida con el `.expected`. Imprime `ok`/`FAIL` por archivo y un resumen; sale con 1 si alguno falla. Los `.ofs` sin `.expected` se ignoran.
- `ofs twin`: ejecuta el programa interpretado y nativo en directorios temporales y compara stdout, código de salida y stderr normalizado. Imprime `iguais` (iguales, código 0) o la primera diferencia (código 1). `OFS_TEST_TIMEOUT` fija el límite por ejecución (30 s por defecto).
- `ofs polish`: alias de `ofs fmt`, mismas opciones.

## `ofs assay` y `ofs build --release`: contratos

```bash
ofs assay archivo.ofs          # casos limite de las veins con contrato
ofs build --release app.ofs    # binario sin las comprobaciones de contrato en ejecucion
```

- `ofs assay`: para cada vein con `bearing`/`settle` (o parámetro `mineral`) y solo parámetros enteros, llama a la vein en los límites de las reglas (el valor de la frontera y ±1; un parámetro sin límite usa -1, 0 y 1; hasta 64 casos por vein), interpretado. Los casos fuera del `bearing`/`grain` se listan con `--` y no se ejecutan; los demás muestran `ok` después de que `settle` compruebe el resultado. La primera violación se detiene con `ofs: [CONTRATO VIOLADO ...]` en stderr y código 1. Sin ninguna vein elegible, lo dice y sale con 0. El `core main` del programa no se ejecuta.
- Comprobaciones de contrato en ejecución: activas en `ofs run` y en `ofs build` por defecto; `ofs build --release` (u `OFS_CONTRACTS=0` en cualquier modo) las quita. La comprobación estática (`[ERRO DE CONTRATO]`) vale siempre. Reglas en `docs/es/LANGUAGE_REFERENCE.md`, sección 4.
- `ofs doc` muestra los contratos: la página de la vein gana la tabla **Contratos** (`bearing`/`settle`) y cada `mineral` aparece entre los tipos, con la base en la firma y las `grain` en la misma tabla.

## `ofs check`: estrato y permisos de los módulos

Después de verificar, `ofs check` lista cada módulo que declara o usa bajo nivel (el principal y cada `attach`), con el estrato y los claims (regla en `docs/es/LANGUAGE_REFERENCE.md`, sección 12, "Estrato y permisos por módulo"):

```
Estrato e permissoes (stratum / claim) por modulo:
  app.ofs (principal): stratum fault (padrao), claim abyss
  stdlib/fs.ofs: stratum fault (padrao), claim rift
  plugins/cru.ofs: stratum fault (padrao), sem claim  [usa sem claim: rift]
  caprock do principal: os anexados podem pedir rift, abyss
```

`[usa sem claim: ...]` es lo que produce `[AVISO DE PERMISSAO]` (error con `OFS_STRICT_FAULT=1`); `[claim sem uso: ...]` es un claim que el módulo no usa. No aparece con `OFS_PROGRESS=json` (el JSON de la extensión no cambia).

## `ofs refract`: edición en vivo (Fault Refract)

`ofs refract` ejecuta el programa interpretado y aplica cada edición guardada **sin reiniciar y sin perder el estado**: la vein editada reemplaza a la antigua en su próxima llamada, y globales, arrays y objetos conservan los valores que ya tenían.

```bash
ofs refract app.ofs            # ejecuta y observa app.ofs y sus attach locales
ofs refract --quiet app.ofs    # sin avisos de cambio (errores y rechazos se siguen mostrando)
ofs refract ofs/examples/refract_demo.ofs   # ejemplo: edite desenhar/atualizar y guarde
```

- **Observa** el archivo principal y los `attach` locales (la stdlib instalada y el runtime quedan fuera). La comprobación ocurre entre llamadas de vein, cada `OFS_REFRACT_PERIOD` ms (200 por defecto), y solo recompila cuando el contenido queda igual durante un período (el editor puede estar guardando).
- **Error de compilación** (sintaxis, tipo, attach): el diagnóstico va a stderr y la versión en ejecución continúa. Corrija y guarde de nuevo.
- **Solo cambia lo que cambió**: cada vein se compara con la versión en ejecución. Entran veins y globales nuevos (un global nuevo empieza con su valor inicial; los existentes conservan su valor aunque el inicializador cambie en el texto).
- **Veins nativas** (cristalizadas o con `quartz`) se recompilan en un `.so` nuevo en segundo plano, junto con las veins nativas que las llaman; hasta que el `.so` esté listo, la versión nueva se ejecuta interpretada. Las veins con `quartz` se compilan antes del cambio (no pueden interpretarse).
- **Una firma compatible se acepta** aunque la vein esté en uso (ola 4): parámetros antiguos iguales y al principio, un parámetro nuevo al final **con valor por defecto** (`vein versao(k: stone = 7)`), el mismo retorno, un retorno que era `void`, un entero estrecho donde era `stone` o `f32` donde era `crystal`. Las llamadas antiguas reciben el valor por defecto. Mensaje: `a assinatura de 'versao' mudou de ()->stone para (stone)->stone: compativel (1 parametro novo com padrao: k)`.
- **Un `monolith` con instancias vivas migra** (ola 4): un campo nuevo recibe el valor por defecto del tipo, un campo eliminado desaparece, el orden puede cambiar y un campo numérico puede cambiar de tipo convirtiendo el valor (`stone`↔`crystal`↔enteros estrechos↔`bool`). Las instancias se reescriben en su lugar (los punteros a ellas siguen valiendo): en la Refract cada instancia nace con 64 bytes de holgura. Mensaje: `monolith 'Ponto' migrado ao vivo (2 instancias); campo(s) novo(s) com o valor padrao: z; convertido(s): y stone->crystal`.
- **Rechazos** (el programa sigue con la versión antigua y el mensaje dice qué requiere reinicio): firma incompatible de una vein que una vein en ejecución todavía llama (o entregada a C como callback, o usada como valor), vein eliminada que alguien en ejecución todavía llama, campo de `monolith` vivo que cambia a un tipo que no se convierte (p. ej. `stone` → `obsidian`) o que crece más allá de la holgura, global que cambió de tipo, `extern` con firma nueva, variantes de `strata` eliminadas o reordenadas.
- **Los bucles cambian en medio (OSR, ola 4).** Un bucle en ejecución en una vein editada (incluido el `while` de `main`, que nunca retorna) pasa a la versión nueva en su próxima cabecera, con las variables locales como están; el resto de la vein después del bucle también es el nuevo. Una variable nueva antes del bucle recibe su valor inicial en el momento del cambio, una eliminada desaparece; si una cambió de tipo, o el bucle está dentro de `cycle`/`match`/`bedrock`, el bucle termina en la versión antigua (mensaje `o laco de 'main' segue na versao antiga: ...`). Un bucle que pasó a nativo por OSR vuelve al intérprete por la cabecera cuando el archivo cambia (el bucle nativo pregunta cada 65536 vueltas) y después vuelve a ser nativo con la versión nueva. Fuera de un bucle, la vein en ejecución termina en la versión antigua y la nueva vale en la próxima llamada.

Mensajes en stderr (cortos, con prefijo `ofs refract:`): `observando app.ofs; salve para refratar`, `2 veins refratadas (desenhar, atualizar)`, `1 vein nova (pulsar)`, `1 global novo (bonus)`, `recristalizando em segundo plano: desenhar`, `desenhar: cristalizada(s) de novo (.so novo)`, `laco de 'main' continua na versao nova (OSR)`, `variavel(is) nova(s) antes do laco, com o valor inicial: extra`, `erro de compilacao (acima); a versao em execucao continua`, `troca recusada: ...`.

Variables: `OFS_REFRACT_PERIOD` (ms, 10 a 60000), `OFS_REFRACT_QUIET=1` (igual que `--quiet`), y las de la cristalización (`OFS_CRYSTALLIZE`, `OFS_CRYSTALLIZE_THRESHOLD`, `OFS_OSR=0` para desactivar la OSR nativa, `OFS_QUARTZ_CACHE`, `OFS_VERBOSE=1`). Sin el wrapper: `OFS_REFRACT=1 OFSCC_MODE=run OFSCC_INPUT=app.ofs ofscc`. Disponible en Linux y macOS (la parte nativa necesita un `ofscc` enlazado con `-rdynamic`); para el celular, ver `ofs refract --remote` abajo.

Prueba: `ofs/tests/refract/run.sh` (cambio interpretado, cambio de vein cristalizada, error de compilación, firma rechazada, vein y global nuevos, wrapper `--quiet`, contratos, geode; ola 4: OSR en el bucle de `main`, OSR de un bucle nativo, migración de `monolith`, firma compatible aceptada).

### `ofs refract --remote`: en el celular (L3)

```bash
ofs refract --remote app.oal               # o un app.ofs con android_event
ofs refract --remote app.oal --serial <id>
ofs build --target android app.oal --refract --install   # solo el APK de depuración
```

La app corre **nativa en el dispositivo** y el PC envía solo las veins editadas: el build `--refract` pone una celda de despacho por vein y el agente `geophone` (socket abstracto `ofs-refract.<paquete>`, alcanzado con `adb forward`); en cada guardado el PC compara vein a vein con el manifiesto que mandó la app, aplica las reglas de seguridad, compila el parche con el clang del NDK (`-shared -fPIC`, enlazado contra la lib de la app) y lo envía; la app hace `dlopen`, cambia las celdas entre dos eventos del hilo principal y responde. Mismo proceso, globales en la misma dirección. Mensajes (prefijo `refract:`): `conectado a <pacote> (pid N, geracao G, ...)`, `alteradas: painel.texto`, `novas: painel.rotulo`, `religadas (chamam uma vein sem celula): ...`, `geracao 2 aplicada no app em 3 ms: ...`, `recusada: a assinatura de 'painel.somar' mudou (()->void -> ()->stone): ...`, `o programa nao compila; o app segue como estava`, `o app recusou o remendo: dlopen: ...`. Detalles, reglas, protocolo y seguridad (el agente solo existe en builds `--refract` y solo acepta el uid de `adb`, root o la propia app) en [ANDROID.md](ANDROID.md#edición-en-vivo-en-el-dispositivo-fault-refract-l3). Prueba en el dispositivo: `ofs/tests/refract/android.sh` (se omite sin dispositivo; mismo PID antes y después, contador conservado, vein nueva, rechazo, error de compilación).

## `ofs fossil`: grabar una ejecución y volver en el tiempo

`ofs fossil` ejecuta el programa interpretado y graba en un archivo `.fossil` todo lo que viene de fuera y no es determinista. `ofs fossil replay` lo ejecuta de nuevo devolviendo los valores grabados en lugar de esas llamadas, sin tocar el mundo real, y la salida sale idéntica. Con `--to`, `--back` y `--dig` se puede parar en cualquier punto de la ejecución y volver en el tiempo.

```bash
ofs fossil app.ofs                       # ejecuta y graba app.fossil (-o otro.fossil)
ofs fossil replay app.fossil             # reproduce: misma salida, sin leer entrada, reloj, archivos...
ofs fossil replay app.fossil --to 120    # para antes del paso 120 y muestra locales y globales
ofs fossil replay app.fossil --to L42    # primera vez que se ejecuta la linea 42 del principal
ofs fossil replay app.fossil --to util.ofs:7   # linea de un attach local
ofs fossil replay app.fossil --to pesar  # entrada en la vein pesar
ofs fossil replay app.fossil --to L42 --back 10   # 10 pasos antes de ese punto
ofs fossil replay app.fossil --back 5    # 5 pasos antes del final
ofs fossil replay app.fossil --dig       # modo interactivo (abajo)
```

**Qué se graba.** Toda llamada externa pasa por el Rift del intérprete (`extern vein`, `rift vein` y las veins nativas del runtime, como `ofs_read_line`), y cada símbolo tiene una política:

- **mundo** (grabado; en el replay no se llama): entrada (`read`, `fgets`, `fread`, `getchar`, `ofs_read_line`, `recv`, `SSL_read`, `getrandom`), reloj (`time`, `clock_gettime`, `gettimeofday`, `clock`, `localtime(_r)`, `pulse.now_ms`, `sleep`/`usleep`, que en el replay no esperan), aleatorio (`rand`, `random`, `drand48`, `srand`, `arc4random`...), entorno (`getenv`, `ofs_getenv`), proceso (`getpid`), archivos y red (`fopen`, `fclose`, `fseek`, `open`, `close`, `stat`, `socket`, `connect`, `accept`, `system`, `remove`...). Graba el retorno y **los bytes que C escribió en el buffer del programa** (`read` → `r` bytes, `fgets` → el texto, `fread` → `r * tamaño`, `clock_gettime` → 16 bytes...). En el replay los bytes van al buffer actual (la dirección cambia de una ejecución a otra; el argumento es el mismo). El texto devuelto por C (`getenv`, `ofs_read_line`) vuelve como copia; `FILE*`/`DIR*` vuelven como valor opaco que solo sirve a las llamadas grabadas.
- **salida** (`write`, `fwrite`, `fputs`, `fputc`, `fprintf`, `fflush`, `printf`, `puts`, `putchar`, `send`, `SSL_write`): graba el retorno y un hash de los datos. En el replay, la salida a la consola (fd 1/2, `stdout`/`stderr`) se ejecuta de verdad; a archivo o red se vuelve un **no-op comprobado**: si los datos cambiaron, `ofs fossil: aviso: a saida de 'fputs' (evento 13) difere da gravada`.
- **lo demás** (funciones puras de C, `strlen`, `snprintf`, `malloc`, `memcpy`, matemática): se ejecuta de verdad en los dos modos. `OFS_FOSSIL_WORLD="mi_fn:otra"` trata funciones propias como mundo (solo el retorno).
- `echo` es del intérprete y se repite solo. Argumentos de línea de comandos: el lenguaje todavía no lee `argv`, así que no hay nada que grabar.

**La cristalización y la OSR quedan apagadas** durante `ofs fossil` y el replay: el código nativo no pasa por el gancho de las llamadas externas ni cuenta pasos. Las veins con `quartz` siguen nativas (aviso: las llamadas externas hechas dentro de ellas no se graban).

**Formato** (versión 1, texto con cargas brutas prefijadas por el tamaño; ver `ofs/ofscc/fossil.ofs`):

```
OFS-FOSSIL 1
S <hash del fuente> <n>\n<ruta del programa>
E <simbolo> <rk> <valor> [<bits float> [<hash de la salida>]]   una llamada
<n>\n<bytes>                     texto devuelto (rk 1) o bloque nuevo (rk 3)
M <arg> <n>\n<bytes>             memoria escrita por C en el argumento
F <pasos> <eventos> <codigo>     fin
```

`rk`: 0 = valor bruto, 1 = texto, 2 = el propio argumento (`fgets` devuelve el buffer), 3 = bloque de memoria nuevo (`localtime`). Los ceros del final de la línea `E` se omiten. Un programa con 200 mil `rand()` graba 3,9 MB.

**Programa modificado.** La cabecera guarda el hash del contenido del principal y de los `attach` locales. Si cambió: `ofs fossil: aviso: o programa mudou desde a gravacao (o hash do fonte difere); o replay pode divergir`. Si el programa hace otra llamada externa distinta de la grabada, el replay para con código 3: `o replay divergiu no evento 1 (passo 2): a gravacao tem 'time', o programa chamou 'rand'`. Al final se compara el número de pasos con el grabado. Sin `OFSCC_INPUT`, el replay usa la ruta de la cabecera.

**Volver en el tiempo.** Un *paso* es un statement ejecutado (los del `main`, de las veins llamadas y de la inicialización de los globales), contado igual al grabar y al reproducir. `--to` para **antes** del paso y muestra dónde está y las variables (locales de la vein y globales declarados en los archivos del programa; arrays y `monolith` por dentro):

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

La salida del programa no aparece hasta ese punto. Para volver, el replay guarda **capas**: cada `--layer N` pasos (por defecto 2000) el proceso entero se duplica con `fork` (copia bajo demanda: variables, heap del intérprete, memoria cruda y la de C) y la copia duerme. Ir a un paso anterior despierta la capa más cercana antes de él, que reejecuta (sin salida) hasta el paso; la línea de tiempo actual termina. Hay como máximo 32 capas: al pasar de eso se queda una sí y otra no, y el intervalo se duplica. El proceso que espera el shell se vuelve un supervisor que solo recibe el código de salida (en Linux es "subreaper", así que nota si la ejecución se cae: `ofs fossil: o replay caiu (sinal 9)`).

**Modo interactivo** (`--dig`, o `-i`): empieza en el paso 1 (o en el `--to`) y lee comandos del terminal (las palabras son en portugués, con el tema del lenguaje; los atajos de una letra son los de siempre):

| comando | atajo | qué |
|---|---|---|
| `segue [n]` | `n` | avanza n pasos mostrando la salida del programa |
| `cava [n]` | `b` | vuelve n pasos en el tiempo ("cavar") |
| `ir <destino>` | `g` | va a un paso (antes o después), `L<linea>`, `archivo.ofs:<linea>` o vein (linea y vein buscan hacia adelante) |
| `ver <nombre>` | `p` | muestra una variable (local y después global) |
| `estrato` | `e` | locales y globales |
| `onde` | `w` | paso, archivo:linea, vein y el texto de la linea |
| `sai` | `q` | termina |

Al final del programa el modo sigue (`fossil: fim do programa (passo 47, codigo 0)`) y `cava`/`ir <paso>` siguen funcionando; un error de ejecución también cae en el prompt.

**Costo** (Ryzen 5 5600GT, `bench/ofs/fib.ofs` interpretado, `fib(35)`, 74,6 millones de pasos): puro 6,21 s (antes de este cambio: 6,42 s, o sea, el intérprete sin grabación no se volvió más lento); grabando 6,40 s (+3%); replay 6,43 s. Con muchas llamadas externas el costo es por llamada: 200 mil `rand()` tardan 0,05 s puro, 0,32 s grabando (cada evento va al disco en el momento, para que un programa que se cae deje la grabación hasta la caída) y 0,14 s con `OFS_FOSSIL_SYNC=0` (graba al final); el replay tarda 0,12 s.

Variables (sin el wrapper): `OFS_FOSSIL=record|replay`, `OFS_FOSSIL_FILE=arch.fossil`, `OFS_FOSSIL_TO`, `OFS_FOSSIL_BACK`, `OFS_FOSSIL_DIG=1`, `OFS_FOSSIL_LAYER`, `OFS_FOSSIL_SYNC=0`, `OFS_FOSSIL_WORLD`; p. ej.: `OFS_FOSSIL=record OFS_FOSSIL_FILE=app.fossil OFSCC_MODE=run OFSCC_INPUT=app.ofs ofscc`. Grabar y reproducir funcionan donde funciona `ofs run`; volver en el tiempo (`--back`, `--dig`, `cava`) necesita `fork` (Linux, Android/Termux, macOS; en Windows solo `--to` hacia adelante).

**VS Code:** los comandos `OFS: Fossil: Go to a Point of a Recording`, `Fossil: Next Step` y `Fossil: Previous Step` ejecutan `ofs fossil replay --to`, muestran el estado en el canal "OFS Fossil" y marcan la línea del paso en el editor. Un adaptador de depuración (DAP) con paso hacia atrás sobre el `--dig` es el siguiente paso.

Límites conocidos: `scanf`/`fscanf` (salidas por punteros variádicos) y funciones que llenan estructuras fuera de la tabla no restauran la memoria (use `read`/`fgets` + `sscanf`, que es puro); las apps de UI (eventos X11/Android) no se graban; las llamadas por valor de vein que viene de C (`call_native_sig`) se ejecutan de verdad.

Prueba: `ofs/tests/fossil/run.sh` (graba un programa con entrada estándar, reloj, aleatorio, entorno, archivo y `rift vein` con `bind`; reproduce con todo cambiado y comprueba la salida idéntica y el archivo no reescrito; `--to` por paso, línea, `archivo:linea` y vein con el valor de las variables; `--back` con capas cada 7 y 2000 pasos; `--dig` con `ir`/`ver`/`segue`/`cava`/`estrato`; programa modificado avisa y diverge; no queda ninguna capa).

## `ofs quarry`: paquetes

`ofs quarry` es el gestor de paquetes (la cantera de donde se extraen las bibliotecas). Está escrito en OFS (`ofs/tools/quarry.ofs` + `quarry_json.ofs`), y el wrapper lo compila y lo guarda en caché como `ofs fmt`/`ofs doc`.

```bash
ofs quarry add granito            # rango ^<última>; también granito@1.2.3, granito@^1.2, granito@beta (canal)
ofs quarry remove granito         # lo quita del proyecto y borra las dependencias huérfanas
ofs quarry list                   # lo instalado, con versión
ofs quarry search texto           # busca en nombre, descripción y tags del registro
ofs quarry update [granito]       # sube versiones dentro de los rangos de ofs.claim
ofs quarry install                # reproduce el proyecto: instala lo que dice ofs.lode, verificando el sha256
ofs quarry publish [--out dist] [--origin <url>] [--deposit registry.json]
```

**Dónde instala:** en `$OFS_LIB_HOME`, o en `~/.ofs/libs`, donde `attach` ya busca. Cada paquete va a `~/.ofs/libs/<paquete>/` (el contenido del `.tar.gz` más `.assay`, con la versión y el hash verificados) y recibe una "veta" de una línea, `~/.ofs/libs/<modulo>.ofs` (`attach {F:<paquete>/<main>}`). Así `attach {granito}` lo encuentra sin configuración, tanto en `ofs build` como en `ofs run` (el guion pasa a guion bajo: `terminal-colors` → `attach {terminal_colors}`). La instalación es por usuario, no por proyecto: dos proyectos que necesiten versiones distintas del mismo paquete alternan con `ofs quarry install`. Si el stdlib tiene un módulo con el mismo nombre (`core`, `fmt`, `io`...), ese gana la búsqueda; `quarry` avisa y sugiere `attach {F:<paquete>/<main>}`.

**De dónde descarga:** del registro en `OFS_QUARRY_REGISTRY` (o `--registry`): una URL `http(s)://`, `file://` o una ruta (archivo `.json` o carpeta con `registry.json`). Por defecto: `src/packages/registry.json` del repositorio en GitHub (raw). Cada versión apunta a una release de GitHub `quarry-<paquete>-<versión>` (la publica el workflow `publish-libraries.yml`, sin npm ni `NPM_TOKEN`); si falla, prueba `<carpeta del registro>/dist/<archivo>`. Con un registro local, el archivo de al lado va primero, así que no hace falta red. La descarga usa el `curl` del sistema (el `tls.ofs` del stdlib no sigue redirecciones y requiere `-lssl`), y la extracción usa `tar`. El sha256 se calcula en OFS (`sha256.ofs`).

### Manifiesto `ofs.claim` y lock `ofs.lode`

El manifiesto es un archivo clave = valor (el ODL describe documentos HTML, no sirve para esto). Comentarios con `#`:

```ini
name = meu-app
version = 0.1.0
description = Exemplo
main = libs/meu_app.ofs          # para publish; por defecto libs/<nombre>.ofs
# opcionales para publish: author, license, tags = a, b, channel = beta, include = libs, assets

[deps]
granito = ^1.2      # >=1.2.0 <2.0.0  (^0.3 = >=0.3.0 <0.4.0)
basalto = ~1.0.3    # >=1.0.3 <1.1.0
xisto = 1.x         # cualquier 1.*.*
ardosia = 2.0.1     # exacta (también >=1.0 <2.0, *)
```

`ofs.lode` se genera: una sección por paquete (directos y transitivos) con `version` exacta, `sha256` del `.tar.gz`, `url` de donde vino, `main` y `deps`. `install` usa solo el lock cuando este cubre el manifiesto (no necesita el registro) y **rechaza** el paquete si el sha256 no coincide; lo mismo vale si el registro trae, para la versión fijada, un hash distinto del lock. Una versión del registro sin `sha256` no es instalable.

**Dependencias entre paquetes:** un paquete publicado con `[deps]` en su `ofs.claim` lleva esos rangos a la entrada del registro. La resolución elige, para cada paquete, la mayor versión que satisface todos los rangos pedidos (la del lock, si todavía sirve) y sigue sus dependencias. Si ninguna versión satisface todos, hay conflicto: no se escribe nada y el mensaje lista quién pidió qué (`conflito: nenhuma versao de basalto satisfaz ^1.0 (pedido por granito@1.0.0); ^2.0 (pedido por xisto@1.0.0)`).

**Publicar:** `ofs quarry publish` (en la carpeta del paquete) genera `dist/<nombre>-<versión>.tar.gz` reproducible (orden por nombre, fechas y dueños en cero, `gzip -n`: el mismo fuente da el mismo sha256) con `include` (por defecto: la carpeta del `main`), el manifiesto, `README.md` y `LICENSE`, y muestra la entrada de registro con el hash. No envía nada. `--origin` define la URL (`{name}`, `{version}`, `{file}` se sustituyen; sin él la URL es el nombre del archivo, relativo al registro) y `--deposit registry.json` escribe la entrada en el registro. `src/packages/build_packages.sh` hace esto para las 19 bibliotecas de `src/packages/src/` (que todavía usan el antiguo `ofspkg.json`, también aceptado).

Prueba: `ofs/tests/quarry/run.sh` (registro local temporal; `HOME`/`OFS_LIB_HOME` temporales). Pendiente: firma de paquetes (hoy solo sha256 del registro/lock), instalación por proyecto y el envío de la release desde `publish`.
