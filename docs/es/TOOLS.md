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
