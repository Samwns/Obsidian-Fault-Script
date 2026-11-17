# OFS: Lenguaje híbrido y compilado nativo

## Pregunta esencial: ¿OFS es interpretado o compilado?

**Respuesta: OFS es HÍBRIDO — compilación nativa AOT vía LLVM e interpretación directa del AST.**

OFS combina lo mejor de los dos paradigmas:
1. **Compilación nativa AOT (`ofs build`)**: genera representación intermedia LLVM IR directamente con su propio compilador self-hosted (`ofscc`), emite código de máquina y enlaza ejecutables mediante los linkers nativos del sistema operativo, sin VM y sin intérprete en el binario final.
2. **Ejecución interpretada instantánea (`ofs run` / `ofs <arquivo.ofs>`)**: ejecuta el árbol sintáctico abstracto (AST) tipado directamente en memoria mediante un intérprete AST integrado escrito en OFS puro (`interpreter.ofs`), ideal para scripts rápidos, prototipado y validación inmediata.


## Modelo de ejecución: interpretado y compilado con la misma semántica

La regla de oro compara `ofs run programa.ofs` con `ofs build programa.ofs` y la ejecución del binario: deben coincidir la salida, el código de salida y los efectos observables. Compartir lexer, parser y verificador de tipos es la base; los tests aportan la evidencia. Ejecute `bash tests/parity/run.sh` y consulte la [matriz de paridad](https://github.com/Samwns/Obsidian-Fault-Script/blob/main/tests/parity/MATRIZ.md), que separa programas iguales, diferencias, errores esperados y casos omitidos. Una diferencia es un bug o una limitación documentada, no una segunda semántica del lenguaje.

`ofs run` evalúa la AST tipada. `extern` y `rift vein` llaman a símbolos nativos resueltos por Rift; los punteros son direcciones reales y los callbacks permiten a C volver a una vein interpretada. `ofs build` emite LLVM IR y enlaza un ejecutable independiente del intérprete. `ofs run --native` compila y ejecuta el programa entero en un directorio temporal.

Con `quartz`, la ejecución es híbrida: el intérprete identifica las veins que contienen ensamblador, pide a `llvmgen` un módulo con el cierre de sus dependencias y carga una biblioteca nativa en el mismo proceso. Las demás veins comienzan interpretadas y pueden promoverse mediante cristalización. Las funciones llamadas por el cierre tienen versiones nativas en el módulo; los valores de veins interpretadas pasados al código nativo usan trampolines de callback. Los globales escalares se sincronizan al entrar y salir del código nativo y en callbacks; textos, arrays y objetos comparten memoria según las representaciones admitidas por Rift. La caché usa el hash del IR y del comando de compilación (`OFS_QUARTZ_CACHE`, o `$XDG_CACHE_HOME/ofs/quartz`, o `~/.cache/ofs/quartz`). `OFS_VERBOSE=1` informa compilación o reutilización. Sin caché se necesita `clang`; `OFS_QUARTZ_CC` selecciona el compilador.

La frontera híbrida hereda los límites del dispatcher FFI: hasta 12 parámetros enteros/punteros o hasta 4 parámetros con `crystal`/`f32`; los callbacks con punto flotante usan los trampolines `__ofs_cbx_K` (fuera de Windows x64). La misma lógica nativa puede acercarse al rendimiento del build una vez compilada, pero interpretar la AST, compilar la biblioteca y cruzar la frontera tienen costes. La equivalencia de rendimiento requiere mediciones que distingan calentamiento y ejecución con caché.

### Cristalización: las veins calientes se vuelven nativas

Por defecto, `ofs run` cuenta las llamadas a cada vein elegible. Al alcanzar `OFS_CRYSTALLIZE_THRESHOLD` (predeterminado `1000`, entero de 1 a 1000000000), genera IR de un único módulo con las veins de firma admitida y sus dependencias. `clang -O2` compila la biblioteca en segundo plano mientras continúa la interpretación; las llamadas siguientes comprueban la finalización y usan despacho nativo. La generación del IR es síncrona. La promoción en lote mantiene una sola copia nativa de los globales. Con `quartz`, el módulo necesario se compila al cargar; las demás veins elegibles obtienen despacho nativo cuando se activa el umbral de calor.

`OFS_CRYSTALLIZE=0` desactiva la promoción opcional y conserva `quartz` híbrido. `OFS_VERBOSE=1` informa promoción y fallos opcionales. Sin `clang`, o si falla la compilación opcional, continúa la interpretación; una vein que requiere `quartz` sí necesita compilación nativa. Las firmas sin trampolín admitido, como `f32` en la frontera, no se promueven como raíces. Un bucle largo dentro de una sola llamada (el `while` de `main`, por ejemplo) cambia de modo en medio (**OSR**, ola 4): después de `OFS_CRYSTALLIZE_THRESHOLD` vueltas el bucle se extrae a una función nativa, compilada en segundo plano, y continúa en ella con las variables vivas transferidas; `bench/ofs/loop.ofs` baja de 18,6 s a 0,34 s (el build nativo tarda 0,41 s). Solo `while` sin `return`/`throw`/`?`/`&`/geode en el cuerpo; `OFS_OSR=0` lo desactiva. La caché evita recompilaciones posteriores, pero no elimina el coste de generar IR y cargar la biblioteca. Compare tiempos en frío, con caché y del binario AOT antes de afirmar equivalencia de velocidad.

### Bajo nivel seguro

El código seguro puede llamar veins tipadas sin bloque de bajo nivel. Use `fracture` (o `bedrock`) para operaciones de puntero, como `&x` y `*p`, y `abyss` para direcciones crudas de funciones, `fault_call` crudo y `quartz`. El verificador de tipos aplica la misma regla en ambos modos: `OFS_STRICT_FAULT=1` convierte las infracciones en errores; la migración predeterminada avisa hasta 2027-01-09. Estos bloques autorizan operaciones, pero no validan automáticamente una dirección ni su vida útil. Los límites de FFI y callbacks están en la [referencia del lenguaje](LANGUAGE_REFERENCE.md).

---

## Arquitectura de ejecución dual

```text
┌─────────────────────────────────────────────────────────────────────┐
│ Código fuente OFS (.ofs)                                            │
└────────────────────────┬────────────────────────────────────────────┘
                         │
                    ┌────▼─────┐
                    │  Lexer   │ -> Análisis léxico (tokens)
                    └────┬─────┘
                         │ Array<Token>
                         ▼
                    ┌──────────┐
                    │  Parser  │ -> Análisis sintáctico (AST puro en OFS)
                    └────┬─────┘
                         │ AST
                         ▼
                    ┌──────────┐
                    │ Typeck   │ -> Verificación y anotación estática de tipos
                    └────┬─────┘
                         │ AST tipado y anotado
         ┌───────────────┴───────────────┐
         │                               │
[ofs run / ofs <arquivo>]         [ofs build <arquivo>]
         │                               │
         ▼                               ▼
┌──────────────────┐            ┌──────────────────┐
│ interpreter.ofs  │            │   llvmgen.ofs    │ -> Emisión de LLVM IR (.ll)
│ Intérprete       │            └────────┬─────────┘
│ AST en OFS puro  │                     │ LLVM IR
└────────┬─────────┘                     ▼
         │                      ┌──────────────────┐
  Ejecución inmediata           │       llc        │ -> Código de máquina (.o)
     en memoria                 └────────┬─────────┘
                                         │ Objeto binario (.o)
                                         ▼
                                ┌──────────────────┐
                                │ ld / ld.lld      │ -> Enlazado con magma.o
                                └────────┬─────────┘
                                         │
                                ┌────────▼─────────┐
                                │ Ejecutable nativo│
                                │ ELF / Mach-O / PE│
                                └──────────────────┘
```

---

## ¿Qué significa el modelo híbrido?

1. **Desarrollo instantáneo**: En modo `ofs run` o `ofs arquivo.ofs`, el código común se ejecuta directamente en memoria; la ejecución híbrida puede compilar veins nativas en el primer uso.
2. **Producción nativa sin VM**: En modo `ofs build`, el ejecutable final no incluye ningún intérprete ni máquina virtual; se ejecuta directamente como código de máquina de la CPU (x86_64, ARM64).
3. **Frontend compartido**: ambos modos usan el mismo lexer, parser y verificador de tipos; la regla de oro y la matriz anterior verifican la paridad de ejecución.
4. **Self-hosted**: Todo el compilador (`ofscc`), incluyendo el parser, el type checker, el generador LLVM y el intérprete, está escrito en OFS.

---

## Comparativa tecnológica

| Característica | OFS (Nativo) | OFS (`run`) | Python 3.14 |
|---|---|---|---|
| **Modelo** | AOT mediante LLVM | AST + cristalización nativa | Bytecode en VM |
| **Inicialización (medida)** | 3 ms | 40 ms | 13 ms |
| **RSS de programa vacío (medido)** | 5,0 MB | n/d | 9,5 MB |
| **Binario vacío** | 37 KB | n/a | n/a |
| **Dependencias** | Bibliotecas del sistema utilizadas por el programa | `ofs`; `clang` para módulos nativos sin caché | Intérprete |

Medido el 2026-11-15 en un Ryzen 5 5600GT (mediana de tres ejecuciones). Resultados y metodología: [bench/RESULTADOS.md](https://github.com/Samwns/Obsidian-Fault-Script/blob/main/bench/RESULTADOS.md). Reproducir con `bash bench/run.sh`. Node.js no fue medido.

---

## Conclusión

OFS es un lenguaje moderno, híbrido y autosuficiente:

- Ejecución inmediata de scripts con el intérprete AST (`ofs run`).
- Compilación directa a código de máquina nativo de alto rendimiento (`ofs build`).
- Frontend 100% escrito en el propio lenguaje OFS.
- Runtime nativo integrado de alto rendimiento (Stack Magma).
