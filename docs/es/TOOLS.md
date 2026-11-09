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
