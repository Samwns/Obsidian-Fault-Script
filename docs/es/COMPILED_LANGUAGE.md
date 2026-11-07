# OFS: Lenguaje híbrido y compilado nativo

## Pregunta esencial: ¿OFS es interpretado o compilado?

**Respuesta: OFS es HÍBRIDO — compilación nativa AOT vía LLVM e interpretación directa del AST.**

OFS combina lo mejor de los dos paradigmas:
1. **Compilación nativa AOT (`ofs build`)**: genera representación intermedia LLVM IR directamente con su propio compilador self-hosted (`ofscc`), emite código de máquina y enlaza ejecutables mediante los linkers nativos del sistema operativo, sin VM y sin intérprete en el binario final.
2. **Ejecución interpretada instantánea (`ofs run` / `ofs <arquivo.ofs>`)**: ejecuta el árbol sintáctico abstracto (AST) tipado directamente en memoria mediante un intérprete AST integrado escrito en OFS puro (`interpreter.ofs`), ideal para scripts rápidos, prototipado y validación inmediata.

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

1. **Desarrollo instantáneo**: En modo `ofs run` o `ofs arquivo.ofs`, el código se ejecuta directamente en memoria sin el retraso de invocar `llc` y el `linker`.
2. **Producción nativa sin VM**: En modo `ofs build`, el ejecutable final no incluye ningún intérprete ni máquina virtual; se ejecuta directamente como código de máquina de la CPU (x86_64, ARM64).
3. **Frontend unificado**: Tanto el intérprete como el generador LLVM comparten el mismo lexer, el mismo parser y el mismo verificador estático de tipos, lo que garantiza la coherencia semántica.
4. **Self-hosted**: Todo el compilador (`ofscc`), incluyendo el parser, el type checker, el generador LLVM y el intérprete, está escrito en OFS.

---

## Comparativa tecnológica

| Característica | OFS (Nativo) | OFS (Interpretado) | Python | Node.js |
|---|---|---|---|---|
| **Modelo** | Máquina nativa AOT | AST Walk en memoria | Bytecode en VM | Motor JIT (V8) |
| **Ejecución** | Directo en la CPU | Intérprete en OFS | CPython | V8 C++ |
| **Inicialización** | ~2 a 10 ms | ~5 a 15 ms | ~40 a 100 ms | ~50 a 150 ms |
| **Memoria base** | Ligera (~1 a 4 MB) | Ligera (~4 a 8 MB) | Moderada (~20 MB)| Elevada (~50 MB) |
| **Dependencias** | Ninguna (binario) | Compilador `ofs` | Intérprete | Runtime de Node |

---

## Conclusión

OFS es un lenguaje moderno, híbrido y autosuficiente:

- Ejecución inmediata de scripts con el intérprete AST (`ofs run`).
- Compilación directa a código de máquina nativo de alto rendimiento (`ofs build`).
- Frontend 100% escrito en el propio lenguaje OFS.
- Runtime nativo integrado de alto rendimiento (Stack Magma).
