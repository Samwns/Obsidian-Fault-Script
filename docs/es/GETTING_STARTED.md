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
| `ofs <arquivo.ofs>` | Ejecuta el programa directamente (equivale a `ofs run`). |
| `ofs run <arquivo.ofs> [--native]` | Ejecuta en el intérprete (o con `--native` para una compilación temporal). |
| `ofs build <arquivo.ofs> -o <binário>` | Compila el archivo a un ejecutable binario nativo vía LLVM. |
| `ofs check <arquivo.ofs>` | Valida la sintaxis y los tipos sin emitir código. |
| `ofs test <arquivo.ofs>` | Ejecuta las pruebas (assay) del archivo. |
| `ofs twin <arquivo.ofs>` | Ejecuta interpretado y nativo, comparando las salidas. |
| `ofs polish <arquivo.ofs>` | Verifica el estilo y los patrones del código. |
| `ofs language [en\|pt\|es]` | Muestra o cambia el idioma de la CLI. |
| `ofs ir <arquivo.ofs>` / `ofs asm` / `ofs tokens` / `ofs ast` | Artefactos de depuración: LLVM IR, ensamblador, tokens y AST. |
| `ofs ui <arquivo.oll> [-o saída.ppm]` | Renderiza un layout OLL nativo como imagen. |
| `ofs clean` | Limpia los artefactos temporales de compilación. |
| `ofs version` / `ofs update` / `ofs help` | Versión, actualización a la última release y ayuda. |

Variables de entorno reconocidas por el launcher: `OFS_LANG` (idioma en/pt/es), `OFS_PROGRESS` (reporter fancy/plain/json), `OFS_VERBOSE` (logs de enlazado/compilación), `OFS_RUNTIME` y `OFS_STDLIB_PATH` (rutas del runtime y de la biblioteca estándar).

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

> **Nota**: Las expresiones anónimas (*lambdas*) están especificadas en el árbol sintáctico (`NK_LAMBDA`) y en planificación en el roadmap del lenguaje. Actualmente todas las funciones deben tener nombre con `vein`.

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

> **Nota**: La construcción `match` está reservada en la especificación del AST (`NK_MATCH`) y en el lexer, con parser y desvío dedicados planificados en el roadmap.

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
- `bedrock`: Biblioteca estándar para asignaciones explícitas de celdas y regiones de memoria.

```ofs
attach {bedrock}

core main() {
    forge celula = bedrock_cell_new(42)
    echo(bedrock_cell_read(celula))
    bedrock_cell_drop(celula)
}
```

---

## 12. Próximos pasos

- Consulte la [Referencia del lenguaje](LANGUAGE_REFERENCE.md) para detalles exhaustivos de la gramática y los tipos.
- Consulte la [Referencia ODL](ODL_REFERENCE.md) y la [Referencia OES](OES_REFERENCE.md) para desarrollo web.
- Consulte la documentación de la [Biblioteca estándar](STANDARD_LIBRARY.md).
