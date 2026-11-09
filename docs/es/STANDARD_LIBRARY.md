# Referencia de la biblioteca estándar de OFS

Los módulos de la biblioteca estándar de OFS están ubicados en el directorio `ofs/stdlib/`. Los módulos instalados en el sistema se importan por nombre con la directiva `attach`:

```ofs
attach {math}
attach {string}
attach {io}
```

Durante el desarrollo o en proyectos locales, un archivo específico puede importarse mediante ruta relativa o absoluta usando el prefijo `F:`:

```ofs
attach {F:../modulos/meu_modulo.ofs}
```

---

## 1. Módulos fundamentales

### `math.ofs`

Operaciones y funciones matemáticas implementadas en OFS puro:

- `square(x)`: Devuelve el cuadrado del número (`x * x`).
- `cube(x)`: Devuelve el cubo del número (`x * x * x`).
- `sum_range(inicio, fim)`: Calcula la suma del intervalo inclusivo de enteros.
- `is_prime(n)`: Comprueba la primalidad verificando divisores impares.
- `count_digits(n)`: Devuelve la cantidad de dígitos del entero.
- `sum_digits(n)`: Devuelve la suma de los dígitos decimales del número.
- `reverse_number(n)`: Invierte el orden de los dígitos decimales.
- `is_palindrome_number(n)`: Comprueba si el número es idéntico a su inverso.

```ofs
attach {math}

core main() {
    echo(square(12))
    echo(is_prime(97))
}
```

### `string.ofs`

Rutinas auxiliares para manipular cadenas sobre las primitivas del runtime:

- `repeat_str(texto, vezes)`: Devuelve la cadena repetida el número de veces indicado.
- `starts_with_char(texto, codigo_char)`: Comprueba si el primer carácter de la cadena coincide con el código ASCII indicado.
- `is_empty(texto)`: Devuelve verdadero si la cadena tiene longitud cero.

El runtime nativo ofrece además las rutinas `ofs_str_len`, `ofs_str_char_at`, `ofs_str_substr`, `ofs_str_concat`, `ofs_str_eq`, `ofs_str_contains` y conversiones numéricas.

### `strbuf.ofs`

Búfer de texto cuya capacidad se duplica al llenarse (anexar es O(1) amortizado). Úselo cuando el texto se arma en muchos pasos o pasa por varias funciones:

- `strbuf.create(capacidad)`: crea un `StrBuf` vacío.
- `strbuf.append(b, texto)`, `strbuf.append_char(b, codigo_byte)`, `strbuf.append_int(b, n)`: anexan al final.
- `strbuf.to_obsidian(b)`: devuelve una **copia** del contenido (la cadena no cambia si el búfer sigue creciendo).
- `strbuf.length(b)`, `strbuf.clear(b)` (lo vacía y conserva la capacidad), `strbuf.release(b)` (libera la memoria).

```ofs
attach {strbuf}

core main() {
    forge b: StrBuf = strbuf.create(64)
    strbuf.append(b, "total: ")
    strbuf.append_int(b, 42)
    echo(strbuf.to_obsidian(b))
    strbuf.release(b)
}
```

Por ahora solo en código nativo (usa memoria cruda del runtime; `ofs run` aún no alcanza el Rift).

**Cadenas y memoria.** Las cadenas no tienen GC ni conteo de referencias: `t = s` hace que ambas variables apunten al mismo texto. El compilador nativo solo reutiliza o libera memoria cuando demuestra que nadie más ve el valor: `s = s + x` / `s += x` en una variable local que nunca se copia a otro lugar crecen en el lugar; las partes intermedias de `a + b + c` y un `f(a + b)` cuyo parámetro no se guarda se liberan; los arrays locales que no escapan se liberan al final de su ámbito. Si la variable se copia, se guarda en un campo/array o se pasa a una función que la guarda, todo funciona como antes (una copia nueva en cada `+`).

### `io.ofs`

Entrada y salida en la terminal:

- `prompt(mensagem)`: Imprime el mensaje indicado y lee una línea de la entrada estándar (stdin).
- `print_separator(caractere, tamanho)`: Imprime una línea repetida con el carácter delimitador.
- `print_header(titulo)`: Imprime un encabezado con formato en la terminal.

### `terminal_colors.ofs`

Declaración de constantes de control ANSI y utilidades para dar formato a texto con colores en emuladores de terminal compatibles.

---

## 2. Módulos de la stack web

### `odl.ofs`

API programática en OFS para construir y generar árboles de documentos ODL/HTML:

- **Nivel estructural**: `document`, `meta`, `css`, `icon`, `script`, `script_module`.
- **Contenedores semánticos**: `div`, `section`, `article`, `nav`, `header`, `main`, `footer`.
- **Tipografía y contenido**: `a`, `img`, `p`, `h1`, `h2`, `h3`, `strong`, `span`.
- **Formularios y listas**: `ul`, `ol`, `li`, `form`, `label`, `input`, `button`.
- **Tablas y bloques de código**: `table`, `table_id`, `code_block`.
- **Primitivas de bajo nivel**: `tag`, `void_tag`, `attr`, `text`.

### `oes.ofs`

API programática para crear hojas de estilo y reglas OES/CSS:

- `prop(nome, valor)`: Emisión de propiedades de estilo individuales.
- `rule(seletor, corpo)`: Declaración de bloques selectores con propiedades.
- `media(consulta, corpo)` / `supports(consulta, corpo)` / `layer(nome, corpo)`: Agrupaciones condicionales y capas.
- `keyframes(nome, frames)` / `frame(etapa, corpo)`: Definición de líneas de tiempo de animación.
- `var(nome, valor)` / `use(nome)`: Declaración y uso de tokens visuales.
- `transition(...)` / `animation(...)`: Auxiliares de movimiento.

### `webserver.ofs`

Servidor HTTP de alto rendimiento y utilidades de protocolo:

- Constantes de métodos HTTP (`GET`, `POST`, `PUT`, `DELETE`).
- Definiciones de códigos de estado HTTP (200, 201, 400, 404, 500) y textos de estado (`status_text`).
- Detección y asignación de tipos MIME (`get_mime_type`) para HTML, CSS, JS, JSON, texto, ODL y OES.
- Construcción de respuestas HTTP (`http_response`) con las cabeceras adecuadas.
- Emisión de fragmentos JSON estructurados (`json_string`, `json_number`).
- Registro estructurado de solicitudes (`log_request`).
- Bucles de ejecución: `serve_once`, `serve_forever`, `serve_html_once`, `serve_html_forever`.

### `webui.ofs`

Biblioteca de componentes para interfaces web construida sobre ODL y OES:

- Temas predeterminados: `theme_dark()`, `theme_light()`.
- Componentes de layout: `page`, `nav`, `hero`, `panel`, `grid`, `card`, `stack`.
- Controles: `button`.
- Servidor rápido: `serve`.

---

## 3. Módulos gráficos nativos

### `canvas.ofs`

API para renderizado gráfico 2D directo sobre un búfer de píxeles raster:

- Creación y liberación de superficies raster (`canvas.create`, `canvas.destroy`).
- Limpieza de superficie (`canvas.clear`).
- Dibujo de primitivas: píxeles (`set_pixel`), rectángulos sólidos y vacíos, líneas y texto bitmap.
- Exportación de imágenes al formato PPM binario sin dependencias externas.

### `window.ofs`

Gestión de ventanas nativas del sistema operativo:

- Creación y destrucción de ventanas (`window.create`, `window.destroy`).
- Gestión del bucle de eventos y mensajes (`window.poll`).
- Integración nativa con X11 en Linux y Win32 en Windows.

### `ui.ofs`

Controles de interfaz de usuario en modo inmediato:

- Estructuras de cajas delimitadoras (`UiRect`).
- Estilización temática (`UiStyle`, `ui.dark()`, `ui.light()`).
- Controles básicos: botones, etiquetas y medición tipográfica.

---

### `raster.ofs`
Rasterizador vectorial con antialiasing (acumulación de área firmada). Trayectorias (`move_to`, `line_to`, `quad_to`, `cubic_to`, `close`), formas (`add_round_rect`, `add_circle`, `add_capsule`, `add_arc_stroke`) y atajos (`fill_round_rect`, `fill_circle`, `stroke_line`, `stroke_round_rect`). Compone en el Canvas respetando el alfa y el clip.

### `ttf.ofs`
Fuentes TrueType en OFS puro: `ttf.load(caminho)`, `ttf.draw(cv, fonte, tamanho, x, linha_base, texto, cor)`, `ttf.width`, `ttf.ascent`/`descent`/`line_height`, `ttf.ellipsize`, `ttf.draw_centered`. UTF-8, glifos compuestos (acentos), caché y posicionamiento subpíxel. Texto en varias líneas: `ttf.wrap(fonte, tamanho, texto, largura_max)` divide por palabras y devuelve `Array<obsidian>` (respeta `\n`); `ttf.draw_wrapped(cv, fonte, tamanho, x, y, largura_max, altura_linha, texto, cor, max_chars)` dibuja el texto dividido desde la parte superior `y` y devuelve el número de líneas (`max_chars < 0` dibuja todo; útil para el efecto de máquina de escribir). Contornos **CFF** (OpenType/PostScript, charstrings Type 2 con subrutinas locales/globales y fuentes CID con FDArray/FDSelect, sugerencias ignoradas) además de `glyf`. **Colecciones TTC**: `ttf.load_face(caminho, i)`, `ttf.load_face_named(caminho, "CJK JP")` (busca en el nombre de la cara), `ttf.face_count`. **Fallback por glifo**: `ttf.set_fallback(fonte)` o `ttf.set_fallback_file(caminho, nome_da_face)` (se carga solo cuando falta el primer glifo); `draw`/`width`/`wrap`/`ellipsize` usan el fallback cuando la fuente no tiene el carácter; `ttf.has_glyph(fonte, cp)`. La caché de glifos rasterizados se reinicia al superar `ttf.pool_limit` píxeles (`ttf.clear_cache()`). Ej.: Roboto + `/system/fonts/NotoSansCJK-Regular.ttc` en Android para japonés y flechas.

### `x11win.ofs`
Ventana X11 nativa: `open`, `present(canvas)`, `poll()` → `EV_KEY`/`EV_MOUSE_DOWN`/`EV_MOUSE_UP`/`EV_MOUSE_MOVE`/`EV_WHEEL`/`EV_RESIZE`/`EV_CLOSE`, campos `ev_x`, `ev_y`, `ev_button`, `ev_key` (keysym), `ev_text` (UTF-8, con teclas muertas pt-BR), `ev_mods`; `set_cursor`, `set_icon`, `set_title`, `now_ms`, `sleep_ms`.

### `icons.ofs`
Iconos vectoriales de 24×24: `icons.draw(cv, icons.BACK, x, y, tamanho, cor)` e `icons.draw_centered`. Constantes `BACK`, `FORWARD`, `RELOAD`, `HOME`, `PLUS`, `CLOSE`, `LOCK`, `INFO`, `STAR`, `MENU`, `SEARCH`, `GLOBE`, `DOWNLOAD`, `HISTORY`, `SETTINGS`, `PROFILE`, `STOP`, `FILE`, `BOOKMARK`, `SHIELD`, `PLUGIN`, `SAVE`, `FOLDER`, `PLAY`, `HAMBURGER`, `GAMEPAD`; `icons.by_name("plugins")` acepta nombres y alias (`import`, `load`, `config`...). Iconos de interfaz añadidos para OAL y Moon: `AUTO`, `SKIP`, `BACKLOG`, `EYE`, `EYE_OFF`, `ROTATE`, `PORTRAIT`, `LANDSCAPE`, `TRASH`, `EDIT`, `IMAGE`, `VOLUME`, `TEXT`, `PALETTE`, `CHECK`, `STORAGE`, `KEYBOARD`, `BACKSPACE`, `SHIFT`, `PAUSE`, `JOYSTICK`, `SPACE`, `CHEVRON`, `RESIZE`, `TABS`. Iconos de **motor/engine** de juegos: `WINDOWS` (ventana de programa de PC), `RENPY` (globo de diálogo: novela visual), `UNITY` (cubo isométrico: motor 3D), `MOON`, `UNKNOWN`. `icons.by_name` también acepta nombres en portugués (`lixeira`, `retrato`, `paisagem`, `teclado`, `joystick`...).

### `android.ofs`
Backend de Android en OFS puro (NativeActivity, aarch64, API 26+): exporta `ANativeActivity_onCreate`, conecta los callbacks con `fault_addr`, entrada mediante `AInputQueue_attachLooper`, frames mediante `AChoreographer`. El programa define `android_event(ev)`. Funciones: `lock`/`unlock_post`/`fill_rect`/`blit` (ANativeWindow RGBA), `request_frame`, `finish`, `set_orientation` (JNI), `internal_data_path`, `external_data_path`, `log`/`log_tag`. Ver [ANDROID.md](ANDROID.md).

### `oal_runtime.ofs`, `oal_android.ofs`, `oal_x11.ofs`
Runtime de la OAL: widgets táctiles (barra, botón, icono, lista, cuadrícula, tarjeta, `fab`, D-pad), estilos con estado presionado y opacidad, desplazamiento, navegación, retrato/paisaje, dibujados en `canvas`/`raster`/`ttf`/`icons` sobre el núcleo de layout de `oll.ofs`. `oal_android.ofs` es el backend de Android; `oal_x11.ofs` (ventana de móvil en escritorio) es experimental. Ver [OAL_REFERENCE.md](OAL_REFERENCE.md).

### `wav.ofs`
Decodificador WAV (RIFF) en OFS puro, sin dependencia de plataforma: `wav.load(caminho)` (`-1` si falla), `wav.decode(ptr, tamanho)`, `wav.pcm(s)` (puntero a las muestras i16 intercaladas), `wav.frames`, `wav.channels`, `wav.rate`, `wav.free`. Admite PCM de 8/16 bits y Microsoft ADPCM. `wav.from_pcm(ptr, quadros, canais, hz)` registra un PCM i16 que ya está en memoria.

### `audio_android.ofs`
Salida de sonido con AAudio (Android 8+): `audio.open()` (44,1 kHz, estéreo, i16), `audio.play(canal, som, loop)`, `stop`, `stop_all`, `set_volume(canal, 0-100)`, `set_master`, `playing`, `pause`/`resume`/`close` y `audio.pump()` una vez por cuadro. El mezclador corre en el hilo de audio y solo usa memoria cruda; la CLI enlaza con `-laaudio` mediante `ofs build --target android`. Ver [ANDROID.md](ANDROID.md). `audio.position(canal)`: cuadros ya reproducidos del sonido del canal (reloj para sincronizar el video).

### `avi.ofs`
Lector AVI (RIFF/OpenDML) y decodificador **Microsoft Video 1** (CRAM/MSVC, 16 bits) en OFS puro: `avi.open(caminho)`, `codec` (FourCC), `supported`, `width`/`height`/`frames`/`us_per_frame`, `avi.decode(v, quadro, dst)` (0xRRGGBB por píxel; secuencial, cuadros delta), `audio_format`/`audio_channels`/`audio_rate`/`audio_bits`, `read_audio` (PCM de todos los bloques `##wb`) + `audio_bytes`, `close`. Cada cuadro se lee del disco en el momento. La salida es idéntica bit a bit a la de ffmpeg. Otros códecs (Xvid/MPEG-4, Indeo) no se decodifican.

### `hashmap.ofs`
Tabla hash entero→entero: `hashmap.new(cap)`, `set`, `get(m, chave, padrao)`, `has`, `count`, `clear`.

### `tls.ofs`
Conexiones HTTPS usando el OpenSSL del sistema: `tls.connect(fd, host)` (SNI + verificación de certificado y nombre), `tls.send`, `tls.recv_all` (`tls.last_len`), `tls.close`, `tls.last_error`. Enlazar con `OFS_LINK_LIBS="-lssl -lcrypto"`.

### `png.ofs`
Decodificador PNG en OFS puro (inflate/zlib propio): `png.load(caminho)`, `png.width`, `png.height`, `png.draw(cv, img, x, y, w, h)`, `png.draw_cover`, `png.draw_region`. Píxeles en `0xAARRGGBB`.

### `zip.ofs`
Leer y crear `.zip` en OFS puro (inflate de `png.ofs`): `zip.open(caminho)`, `count`, `name(i)`, `size(i)`, `is_dir(i)`, `find(nome)`, `read(i)` (`zip.data_ptr`/`data_len`, `free_data`), `read_text(nome)`, `extract(i, destino)`, `extract_all(pasta)` (rechaza `..` y rutas absolutas; verifica el CRC), `close`; escritura sin compresión: `create`, `add_file`, `add_text`, `add_bytes`, `finish`. Sin zip64/cifrado. `png.inflate_raw` expone el deflate crudo.
**Extracción por bloques** (no bloquea la interfaz): `zip.xbegin(i, destino, hash)` abre la entrada; llame a `zip.xstep(bytes)` o `zip.xstep_ms(ms)` (uno por cuadro) hasta que devuelva `1` (escrito, CRC verificado, `zip.x_sha` con el sha256 si `hash`) o `-1` (`zip.last_error`; el archivo parcial se elimina); `zip.xpct()` devuelve 0..100 y `zip.xabort()` cancela. El inflate reanuda desde donde se detuvo (`png.inflate_begin/inflate_step/inflate_take/inflate_abort`) y tiene estado propio, por lo que `png.load` puede ejecutarse entre los pasos.

### `sha256.ofs`
`sha256.file(caminho)` y `sha256.bytes(ptr, n)` (64 hex en minúsculas; bloques de 64 KB), `sha256.crc32_update(crc, ptr, n)`.

### `android_http.ofs`, `android_webview.ofs`
HTTP/HTTPS mediante `HttpURLConnection` vía JNI y WebView a pantalla completa en un `Dialog`, sin Java propio. Ver [ANDROID.md](ANDROID.md). `icons.ofs` incorporó `RPGM` y `TYRANO`; `oal_runtime.ofs` incorporó `colorpicker`, `set_palette_full`, `hsv_to_rgb`.

## 4. Módulos de bajo nivel y sistema

### `bedrock.ofs`

Primitivas de gestión explícita de memoria:

- Asignación y liberación manual de bloques de memoria y celdas (`bedrock_cell_new`, `bedrock_cell_drop`).
- Operaciones de lectura y escritura directa en direcciones de memoria.
- Creación y reciclaje de regiones y arenas de asignación.

### `bedrock_packet.ofs`

Estructuras utilitarias para armar y analizar paquetes de datos binarios en protocolos de red.

### `rift.ofs`

Capa del Rift: declarar y llamar funciones nativas de bibliotecas del sistema con la convención de llamada del sistema (conocida técnicamente como ABI C; no involucra código C).

### `memory_modes.ofs`

Declaración de interfaces experimentales para controlar patrones de acceso a búferes y asignaciones del sistema.

---

## 5. Módulos de apoyo y compatibilidad

### `test_lib.ofs`

Módulo auxiliar para validar pruebas unitarias y ejemplos del lenguaje.

### `ofshtml.ofs`

Módulo heredado mantenido por compatibilidad hacia atrás con código que usaba generadores HTML anteriores a ODL. Los proyectos nuevos deben usar `odl.ofs`.
