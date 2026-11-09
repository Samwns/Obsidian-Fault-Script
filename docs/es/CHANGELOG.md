# Registro de cambios

Todos los cambios notables de este proyecto se registran aquí.

> **Esquema de versiones.** Las versiones oficiales son las etiquetas `vMAJOR.MINOR.PATCH` de GitHub (`v1.0.0` ... `v1.0.178`), con el parche incrementado automáticamente por el flujo de trabajo `release.yml`; ese es el número que muestra `ofs version` en un paquete instalado. Las etiquetas `1.1.0`, `1.1.1` y `1.2.0` que aparecen más abajo son hitos históricos de abril de 2026 que **nunca fueron etiquetas**; las entradas se mantuvieron por tema, no por versión. El texto "Unreleased" cubre todo lo que llegó después de `v1.0.151`.

## [Unreleased]

### Corregido — compilador, memoria y paridad (2026-11-08/09)

- `a[i] = v` y `a[i] op= v` ahora guardan en el binario nativo (antes la asignación desaparecía). `xs[i]` con `Array<obsidian>` como parámetro o campo se lee como texto. Una palabra reservada usada como nombre (`anchor`, `core`…) da un error claro. `snprintf` y otras funciones C variádicas se declaran con `...` (un `crystal` a veces salía como basura). Intrínsecos `fault_count`, `fault_lead`, `fault_trail`, `fault_spin_*`, `fault_weave`, `fault_memcpy`, `fault_memset`, `fault_unreachable` implementados (nativo e interpretado). `?` con la misma semántica en los dos modos. Un error de ejecución del intérprete sale con código 1.
- Memoria: la concatenación de cadenas crece en el lugar cuando el compilador prueba que es seguro, los arrays y cadenas locales se liberan al final del ámbito, `a[i]` en línea. Benchmark `strings` 223 ms / 520 MB → 15 ms / 6 MB; `alloc` 116 ms / 97 MB → 17 ms / 5 MB; `sort` 86 → 15 ms; compilar el propio compilador 197 → 45 MB. El intérprete (`ofs run`) dejó de perder ~3 KB por llamada: `fib(35)` interpretado corre con 2,3 MB. El runtime imprime `ofs: sem memoria` cuando falla una asignación de memoria.

### Añadido — `ofs fmt`, `strbuf` y verificación de paridad

- **`ofs fmt`** (`ofs/tools/ofsfmt.ofs`): formateador con `--check`, `--diff`, `--stdin` y carpetas recursivas; conserva los comentarios y no cambia el IR generado; formatear al guardar en VS Code. **`strbuf`** en el stdlib (búfer de texto que crece). **`tests/parity/run.sh`**: ejecuta los programas en los dos modos (`ofs run` × nativo) y genera `tests/parity/MATRIZ.md`. **`bench/run.sh`** mide el compilador del repositorio con límite de memoria; el sitio lee `bench/results.json`.

### Added — `ofs doc` (documentación generada del código) y la Referencia del sitio

- **ES** — nuevo comando **`ofs doc`** (`ofs/tools/ofsdoc.ofs`, escrito en OFS): documentación HTML generada del código fuente (módulos, `vein`/`rift vein`/`extern vein`, `monolith`/`impl`, llama a/usado por, véase también, enlace a la línea en GitHub, búsqueda `search.json`) e informe de cobertura (`ofs doc --check` sale con 1 si un `.md` cita una función inexistente). La wiki del sitio fue reemplazada por la **Referencia** (`docs/ref/`, tema obsidiana): Guía + API generada + Cobertura en una sola navegación; `docs/wiki/` solo redirige. Ver `docs/es/TOOLS.md`.

### Added / Adicionado — Termux

- **ES** — **Termux**: OFS ahora corre en el teléfono (`ofs run`, `ofs build`, herramientas); el wrapper detecta Termux, paquete `ofs-termux-aarch64.tar.gz` (`src/packaging/termux/build_termux.sh`, adjuntado a la release por CI) e `install_termux.sh`. Ver `docs/es/GETTING_STARTED.md`.

### Added — hex keyboard (OAL)

- **ES** — OAL: capa hex del `keyboard`, con `oal.kb_layer = 2` (teclas 0-9, A-F, borrar, `#` y ↵). Usada en el campo hex del selector de color de M.O.O.N.

### Added — gamepad en pantalla con estilos de botones (OAL)

- OAL: widget `diamond -> acción(item) corner= look=` (4 botones de acción en rombo; estilos `PAD_KEYS` teclas, `PAD_XBOX`, `PAD_PLAY` con formas vectoriales ✕ ○ □ △ sin logotipos, `PAD_NINTENDO`), evento `on key` (teclas de control/teclado reenviadas con filtro y sin repetición), `oal.pad_connected` (control físico detectado por la fuente del evento). `icons.ofs`: `BTN_CROSS/CIRCLE/SQUARE/TRIANGLE`, `KEY_ENTER`. `android.ofs`: `key_source`, `key_handled` (la tecla consumida no llega al sistema) y el analógico ya no actúa como toque.

### Added — fuentes CFF/TTC con fallback, video AVI (M.O.O.N)

- `ttf.ofs`: contornos **CFF** (charstrings Type 2, subrutinas locales/globales, fuentes CID con FDArray/FDSelect), colecciones **TTC** (`load_face`, `load_face_named`, `face_count`), **fallback por glifo** (`set_fallback`, `set_fallback_file` con carga diferida, `has_glyph`), límite de la caché de glifos (`pool_limit`, `clear_cache`). El texto TrueType sale idéntico bit a bit al anterior. Nuevo `avi.ofs`: lector AVI + decodificador Microsoft Video 1 (16 bits), idéntico bit a bit a ffmpeg. `wav.from_pcm`, `audio.position`. Workaround del compilador: `ofs_array_get(...) as crystal` lee el elemento como `double` sin convertir (pase por una función con parámetro `stone`).

### Added — plugins `.moonpack`, WebView, zip, selector de color (OAL)

- `zip.ofs`: extracción por bloques (`xbegin`/`xstep`/`xstep_ms`/`xpct`/`xabort`) sobre el nuevo inflate reanudable de `png.ofs` (`inflate_begin/step/take/abort`); CRC y sha256 se calculan mientras se extrae. Instalar el `.so` de 76 MB de Godot en M.O.O.N ya no genera ANR con los toques durante la extracción.

- `zip.ofs` (leer/extraer zip con el inflate de `png.ofs`, crear zip sin compresión, CRC-32), `sha256.ofs` (SHA-256 y CRC-32), `png.inflate_raw`/`out_ptr`/`out_len`; `android.ofs`: JNI genérico (`jni_call`, `jni_new`, `jni_scall`, `jni_static_int_field`, referencias globales, arrays de bytes/objetos); `android_http.ofs` (HTTPS con `HttpURLConnection` por JNI, por pasos, también `file://`; OpenSSL no se puede enlazar en un APK); `android_webview.ofs` (WebView a pantalla completa en un `Dialog` por JNI, sin Java propio: HTML con el shim inyectado por `loadDataWithBaseURL`, canal de vuelta por `document.title`); `icons.ofs`: `RPGM`, `TYRANO`. OAL: widget **`colorpicker value=expr -> acción(item)`** (cuadrado saturación/valor, barra de matiz, vista previa y hex), `oal.set_palette_full` (botón, botón pulsado, borde, sello; texto sobre acento/botón por contraste), `oal.hsv_to_rgb`/`contrast_ink`; el botón `back` de la barra ahora va a la **izquierda**, antes del título (`oll.move_child_first`). Aviso del compilador: `core` es palabra reservada (el cuerpo de la función desaparece en silencio).

### Added — Android (aarch64) y OAL v0

- **Destino Android**: `OFSCC_TARGET=android` emite `aarch64-unknown-linux-android26`; el runtime `magma` compila para aarch64 sin cambios (solo usa libc/bionic). `net.resolve` ahora lee el `struct addrinfo` de bionic (el orden de `ai_addr`/`ai_canonname` es el inverso al de glibc). Doc: `docs/ANDROID.md` (+ `en/`, `es/`).
- **`ofs build --target android app.ofs|app.oal`**: genera el APK sin Gradle/Java (`clang` del NDK, `-Wl,-init,__ofs_init_globals`, `aapt2`, `zipalign -P 16`, `apksigner`), con `--install`, `--serial` y `--manifest`. Si el `ofscc` instalado no tiene `OFSCC_TARGET`, compila uno desde el código fuente (`~/.cache/ofs/android/`). El wrapper que corre desde el repo usa la stdlib del repo.
- **`fault_addr(vein)` y `fault_call(endereco, args...)`**: dirección de una vein como `stone` (`ptrtoint`) y llamada indirecta con la convención de llamada del sistema (`inttoptr` + `call`). Permite callbacks nativos (`qsort`, NativeActivity, ALooper, AChoreographer) y tablas de funciones (JNI). Prueba: `ofs/tests/runtime/test_fault_addr_qsort.ofs`. Punto fijo del self-host mantenido (IR gen1 == gen2).
- **stdlib `android.ofs`**: backend NativeActivity **100% OFS, sin ningún `.c`** (se eliminó el `android_native_app_glue`): exporta `ANativeActivity_onCreate`, rellena `activity->callbacks` con `fault_addr`, entrada por `AInputQueue_attachLooper`, frames por `AChoreographer_postFrameCallback`, todo en el hilo principal; ciclo de vida, toque, atrás, rotación, áreas de las barras del sistema, `ANativeWindow` RGBA, `set_orientation` por JNI, rutas de la app, log.
- **OAL v0 (Obsidian Android Language, `.oal`)**: `ofs/tools/oalc.ofs` genera `AndroidManifest.xml` + `<nombre>_oal.ofs`; `app`, `use`/`attach`, `style` (`fill`, `ink`, `curve`, `opacity`, `pressed`), `screen` con `bar`, `button`, `button.icon`, `title`, `text`, `list`/`grid` con `from`, `card`, `row`, `portrait`/`landscape`, `fab`, `dpad`; acciones `-> funcao(item)`, `@tela`, `@back`, `@quit`, `@portrait`/`@landscape`/`@auto`; `on back/pause/resume/rotate/low_memory`. Runtime `oal_runtime.ofs` (núcleo de layout de `oll.ofs`, `canvas`/`raster`/`ttf`/`icons`) + `oal_android.ofs`; `oal_x11.ofs` es experimental. Doc: `docs/OAL_REFERENCE.md` (+ `en/`, `es/`); prueba: `ofs/tests/oal/run.sh`.
- **`icons.ofs`**: iconos `PLUGIN`, `SAVE`, `FOLDER`, `PLAY`, `HAMBURGER`, `GAMEPAD` y alias en `by_name` (`plugins`, `import`, `load`, `config`...).
- **stdlib**: nuevos `wav.ofs` (decodificador WAV PCM de 8/16 bits y Microsoft ADPCM) y `audio_android.ofs` (salida AAudio con el mezclador en el hilo de audio); `ttf.ofs` ganó texto en varias líneas (`ttf.wrap`, `ttf.draw_wrapped`); `icons.ofs` ganó iconos de interfaz (`AUTO`, `SKIP`, `EYE`, `ROTATE`, `TRASH`, `VOLUME`, `KEYBOARD`, `JOYSTICK`, `TABS` ...) y de motor (`WINDOWS`, `RENPY`, `UNITY`, `MOON`, `UNKNOWN`).

- **OAL v1**: widgets `image`, `canvas [full] draw= touch=` (área dibujada por la app, con toques por dedo), `tabs`/`tab`, `slider`, `toggle`, `progress`, `field`, `keyboard` (teclado en pantalla), bloques `if`/`else`; `item`/`card` con `image=`, `icon=` dinámico, `badge=`/`badge_icon=`; `text`/`empty` en varias líneas; etiquetas e iconos como expresiones; `fab.classe.icon`; esquinas `bottom_row`/`top_row`/`right_column`/`left_column` o expresión; eventos `on start` y `on frame`; `app` con `assets "carpeta"` e icono PNG copiado a `res/mipmap/`. Runtime: **multitoque por dedo**, caché de PNG, `oal.toast`, `set_animating`, `reset_to`/`replace`, `set_text_scale`, `set_overlay_scale`, `set_palette` (tema), copia a la ventana solo cuando algo cambia. Doc: `docs/OAL_REFERENCE.md` (+ `en/`, `es/`).
- **`android.ofs`**: multitoque (`touch_id`, `EV_POINTER_DOWN/UP`, `ptr_*`), auxiliares JNI (`jni_find_class`, `jni_string`, `jni_call_object`, `jni_call_void1`, `jni_static_bool`, `package_name`, `start_intent`), `is_external_storage_manager`/`open_all_files_settings` (`MANAGE_EXTERNAL_STORAGE`), **pantalla completa inmersiva** (`set_immersive`, reaplicada al recuperar el foco), dibujo en el área del recorte de la cámara (`set_cutout_short_edges`), `keep_screen_on`, `asset_read`, `redraw_needed`. La CLI enlaza con `-laaudio`. Doc: `docs/ANDROID.md` (+ `en/`, `es/`).
- **`android.ofs` — modo motor** (alojar SDL/Godot en la propia `NativeActivity`, sin Java en el APK): `release_window` (`Window.takeSurface(null)`/`takeInputQueue(null)` en `EV_CREATE`), `set_content_view`, `dex_load_class` (`DexClassLoader`, varios `.dex` separados por `:`), `jni_scall_cls`, `jni_take_error`/`jni_error`, `set_read_only`, `intent_extra`, `self_intent`, `restart_process` (reabre la activity en un proceso nuevo). Doc: `docs/ANDROID.md` (+ `en/`, `es/`).
- **`icons.ofs`**: 30 iconos nuevos (auto, saltar, historial, ojo/ocultar, girar, retrato, paisaje, papelera, editar, imagen, volumen, texto, paleta, ok, almacenamiento, motores `windows`/`renpy`/`unity`/`moon`/`unknown`, teclado, borrar, shift, pausa, joystick, espacio, flecha, tamaño, pestañas) y alias en `by_name`.

### Fixed — compilador

- **runtime**: `ofs_array_pop` ahora devuelve el elemento eliminado (antes devolvía basura/0 a quien usaba el valor).
- **driver (`attach`)**: la ruta de `attach {F:...}` se normaliza (`./`, `../`, barras duplicadas) antes de comprobar duplicados, así el mismo archivo ya no se incluye dos veces.
- **net**: `net.resolve` lee el `struct addrinfo` de bionic (Android), donde `ai_addr`/`ai_canonname` están en un orden distinto al de glibc.
- **llvmgen**: un `extern vein` definido en el propio módulo ya no se vuelve a declarar (`invalid redefinition of 'ofs_alloc'` en `magma.ofs` y en `test_magma_full`).

### Added — stack gráfico 100% OFS (base de Catalyst Browser)

- **`raster.ofs`**: rasterizador vectorial con antialiasing por acumulación de área (estilo font-rs): `move_to`/`line_to`/`quad_to`/`cubic_to`/`close`, rectángulos redondeados, círculos, cápsulas (trazos), arcos, anillos, y composición sobre el Canvas con alfa.
- **`ttf.ofs`**: lector TrueType (cmap 4/12, glifos simples y compuestos, hhea/hmtx) con UTF-8, caché de glifos, posicionamiento subpíxel (1/4 px), oscurecimiento de trazos, `width`, `ellipsize`, `draw_centered`. Texto real (Inter, Noto, DejaVu) con acentos.
- **`x11win.ofs`**: ventana X11 nativa en OFS puro (Xlib mediante `extern`): presentación del Canvas por `XPutImage`, eventos de teclado con texto UTF-8 y teclas muertas pt-BR (´ ` ^ ~ ¨ ç), ratón, rueda, redimensionado, cierre (`WM_DELETE_WINDOW`), cursores (flecha/mano/texto), icono de ventana (`_NET_WM_ICON`), `now_ms`/`sleep_ms`.
- **`icons.ofs`**: 20 iconos vectoriales de interfaz (atrás, adelante, recargar, inicio, +, cerrar, candado, info, estrella, menú, búsqueda, globo, descarga, historial, engranaje, perfil, detener, archivo, favorito, escudo) con caché por tamaño.
- **`hashmap.ofs`**: tabla hash entero→entero con direccionamiento abierto y crecimiento automático.

### Fixed — compilador (`ofscc`), verificado por bootstrap con punto fijo (vN.ll == vN+1.ll)

- **parser**: `*`, `-` o `&` al inicio de una nueva línea ahora inician un nuevo comando. Antes, `v = *p` seguido de `*p = 0` se convertía en `v = *p * p = 0`.
- **parser**: los operadores unarios se aplican a la expresión postfija completa — `!ns.fn(x)` es `!(ns.fn(x))`, no `(!ns).fn(x)` (generaba una llamada a `@fn` inexistente).
- **llvmgen**: los globales de `namespace` son accesibles desde fuera (`raster._acc`, `cfg.contador = ...`, `cfg.contador++`); antes generaban un GEP sobre `%unknown`.
- **llvmgen**: los globales con literal negativo (`forge x: stone = -1`) se emitían como `0`. Afectaba al propio compilador (`interpreter___ret_ref`).
- **typeck**: `ofs_array_get`/`ofs_array_pop`/`ofs_array_len`/`ofs_array_push`/`ofs_array_set` sin `extern` declarado ahora tienen tipo inferido de `Array<T>` (antes `unknown` → puntero → IR inválido).
- **canvas**: `_sin_deg`/`_cos_deg` con reducción de ángulo a [-π/2, π/2] y serie de 6 términos (un error de ~20% cerca de 180° deformaba los arcos).
- **dist**: `ofs/dist/ofscc`, `ofs/dist/ofscc.ll` y `ofs/dist/stdlib/` estaban desincronizados del fuente — el driver usaba un `canvas.ofs` antiguo sin `draw_shadow` (segfault en Catalyst) y un `ofscc` sin `stacksave` en los bucles. Regenerados desde el fuente.
- **runtime (`ofs_str_char_at`)**: llamaba a `strlen` en cada carácter (O(n²) en bucles sobre cadenas). Ahora guarda en caché (puntero, longitud) de la última cadena, validado por el terminador NUL. Bucle sobre 164 KB: 0,203 s → 0,014 s. Aplicado a mano en `ofs/dist/magma.ll` y `ofs/dist/magma_windows.ll` (que tienen ajustes manuales) y reflejado en `stdlib/runtime/facet.ofs`; `magma.o` regenerado.
- **typeck**: los builtins de cadenas sin `extern` (`ofs_str_len`, `ofs_str_char_at`, `ofs_str_eq`, `ofs_str_contains`, `ofs_str_concat`, `ofs_str_substr`, `ofs_stone_to_obsidian`) tienen tipo inferido (antes `unknown` → IR inválido en `forge n = ofs_str_len(s)`).
- **typeck/interpreter (`ofs run`)**: los globales de `namespace` se predeclaran con el nombre mangled y `ns.global` se reescribe a `ns__global` antes de los backends — desaparece la falsa advertencia "identificador desconocido" y `ofs run` pasa a leer/escribir esos globales. El intérprete también pasa a inicializar los globales declarados dentro de `namespace` (antes quedaban nulos incluso para el propio namespace).
- **bootstrap**: `ofs/bootstrap/*.ofs` sincronizado con `ofs/ofscc/` (era una foto antigua sin `interpreter`, `events`, `reporter`); los scripts apuntaban a `ofs/ofs/bootstrap/ofscc.ofs` (inexistente — `bootstrap-minimal.sh` fallaba en la Fase 2) y ahora usan `ofs/ofscc/ofscc.ofs`. `OFS_BOOTSTRAP_SELFHOST=1 bootstrap-minimal.sh --validate` pasa con determinismo validado.

- **driver (`attach {modulo}`)**: la forma corta documentada (`attach {canvas}`, `attach {terminal-colors}`) nunca cargaba el módulo — solo funcionaba `attach {F:archivo.ofs}`. Ahora resuelve `modulo.ofs` en la stdlib (guion → guion bajo), con prioridad sobre un archivo local del mismo nombre; vale para `build` y `run`.
- **lexer (literales hexadecimales)**: `0xFF`, `0x1A1A2E` iban sin procesar al LLVM IR (leídos como punto flotante → "floating point constant invalid") y no funcionaban en el intérprete. Ahora se convierten a decimal en el lexer; `0xFFFFFFFFFFFFFFFF == -1` como en C.
- **llvmgen (enteros pequeños)**: `u8`/`u16`/`u32`/`i8`/`i16`/`i32` se extienden a `i64` (zext/sext) en operaciones binarias y en `echo` — `(r as u32 << 16) | ...` generaba IR inválido y `echo(pixel: u32)` llamaba a `echo_obsidian` con un entero.
- **examples**: 28 de 33 compilan (antes 21). Quedan 3 exclusivos de Windows, `attach_local_lib.ofs` (biblioteca sin `main`) y `hybrid_fault_console.ofs` (`fault_count` sin implementación en llvmgen).
- **VS Code (1.0.10)**: autocompletado, hover y mapa de archivos con `fs`, `net`, `http`, `oll`, `ui`, `raster`, `ttf`, `x11win`, `icons`, `hashmap`; snippets `x11win-app` y `hashmap`; toolchain empaquetado (`bin/linux-x64`: `ofs`, `ofscc`, `magma.o`, stdlib) actualizado.
- **sitio/docs**: páginas de stdlib (PT/EN/ES) regeneradas por `build_site.ofs` con los nuevos módulos; `docs/en/STANDARD_LIBRARY.md` y `VISUAL_UI.md` (PT/EN) con el stack gráfico 100% OFS.

- **lexer (`\r` y `\t` en cadenas)**: el caso existía, pero usaba los literales `"\r"`/`"\t"`, que dependen del compilador anterior; como la semilla no los convertía, el error se perpetuaba en el bootstrap y `"\r\n"` generaba los bytes `\` `r`. Las cabeceras HTTP de `http.ofs`/`webserver.ofs` salían inválidas (los servidores respondían 400). Ahora los bytes se construyen por código.
- **llvmgen (bool en arrays)**: `ofs_array_set/push(a, i, x || y)` escribía un `i1` en un slot `i32` (IR inválido); el valor ahora se extiende.
- **llvmgen**: `ofs_str_char_at_known` ahora se declara automáticamente como las demás funciones de cadenas.
- **driver**: `attach {F:/ruta/absoluta.ofs}` acepta ruta absoluta.
- **net.ofs**: `net.resolve(host)` — DNS mediante `getaddrinfo` en OFS (`net.is_ipv4`).
- **Catalyst 3.0.0**: reescrito sobre el stack gráfico de OFS — interfaz estilo Chrome, pestañas con historial propio, omnibox editable, favoritos, menú, nuevo parser HTML/CSS, layout con salto de línea, flex, enlaces, desplazamiento, DNS y redirecciones HTTP.

- **net.ofs**: `net.recv_all(fd)` (recepción binaria segura, `net.last_len`), `net.set_timeout` y `net.timeout_s` (15 s por defecto en `tcp_connect`; antes un servidor mudo bloqueaba el programa para siempre).
- **Catalyst 3.1.0**: historial y descargas persistentes en `~/.catalyst` / `~/Downloads` (descargas binarias reales), sistema de plugins `.cai` (estilos, bloqueo, redirección, búsqueda, páginas `ofs://ext/`, botones, atajos, página de inicio), página `ofs://plugins`, Google como buscador predeterminado, icono de cristal (`assets/catalyst.svg`).

- **llvmgen (`&&` / `||`)**: el código nativo evaluaba siempre ambos lados (sin cortocircuito); `i >= 0 && arr[i]` leía fuera del array. Ahora el lado derecho solo se ejecuta cuando hace falta, como en el intérprete y en C.
- **parser (sombreado)**: los parámetros y las variables locales de funciones dentro de un `namespace` se renombraban al global homónimo (`forge bg = ...` leía y sobrescribía `ns__bg`). Ahora las locales sombrean a los globales.
- **stdlib**: `icons.by_name(nome)`.
- **Catalyst 3.2.0**: configuración rediseñada (barra lateral estilo Chrome, búsqueda estilo Firefox, tarjetas, interruptores, opciones y botones con iconos), tema claro/oscuro, tamaño de fuente, barra de favoritos opcional, idiomas pt/en/es, elección del motor de búsqueda, nueva pestaña con caja de búsqueda, formularios (`<input>`, `<button>`, envío GET) y la etiqueta `<icon name>` en las páginas.

- **Catalyst 3.3.0**: Catalyst Shield (bloqueo nativo de anuncios/rastreadores: red, cosmético estilo EasyList, heurística, filtros del usuario, excepciones por sitio, contador y panel en la omnibox), Visión Esencia (páginas que dependen de JavaScript mostradas a partir de JSON-LD, `__NEXT_DATA__`, OpenGraph y enlaces, sin ejecutar scripts), 6 colores de acento, tema dinámico mediante `theme-color`, familias tipográficas (Inter, Noto Sans, DejaVu, Serif), favoritos persistentes con gestor `ofs://bookmarks`.

- **stdlib `tls.ofs`**: HTTPS mediante el OpenSSL del sistema (SNI, verificación de certificado y de nombre); enlazar con `OFS_LINK_LIBS="-lssl -lcrypto"`.
- **stdlib `png.ofs`**: decodificador PNG 100% OFS con inflate/zlib propio (gris, RGB, paleta de 1–8 bits, alfa); decodificación idéntica píxel a píxel en las pruebas; `draw`, `draw_cover`, `draw_region`.
- **driver `ofs`**: variable `OFS_LINK_LIBS` para bibliotecas extra del sistema en el enlace.
- **stdlib `x11win.now_ms`**: funciona sin ventana abierta (antes se bloqueaba en modo headless).
- **CI**: el runner `macos-14` fue retirado por GitHub ("configuration 'macos-14-arm64-us-default' is not supported"); `build.yml` y `release.yml` pasan a usar `macos-latest`.
- **Catalyst 3.4.0**: HTTPS, HTTP/1.1 con chunked, CSS externo con caché, `@media`/`@supports`, cascada indexada por selector (G1: CSS de 40 ms), parser HTML 20× más rápido en páginas con JS inline, perfilador `CATALYST_PROFILE=1`, salto de palabras largas, `flex-shrink`, usuarios con exportar/importar (`.catprofile`), colores libres de acento y fondo, papel tapiz PNG/degradado en la nueva pestaña.

### Known issues

- Un global de tipo `monolith` (`forge cv: Canvas` en ámbito de namespace) reserva solo un puntero; asignar la estructura completa corrompe la memoria. Use una variable local o páselo por parámetro.
- `stdlib/runtime/frame.ofs` (la ventana de `magma.o`) es un stub; use `x11win.ofs` para ventanas reales.

### Added

- **Reporter Fases 2–5 (ofscc)**:
  - **Fase 2 — i18n**: catálogo integrado en/pt/es con detección automática de locale (`OFS_LANG` > `LANG` > `en`) y TOML espejo en `ofs/ofscc/locale/` para traductores y CI de paridad.
  - **Fase 3 — Reporter Fancy**: verbos alineados en columna fija, símbolos ANSI, colores con degradación automática (`NO_COLOR` / `TERM=dumb`).
  - **Fase 4 — animación**: spinner de un solo hilo entre etapas, `\r` + borrado de línea, desactivable con `OFS_ANIM=0`.
  - **Fase 5 — Json NDJSON**: eventos extra (`locale`, `msg_key` + `args[]`, `duration_ms_total`) y stub de salida `twin` en NDJSON.
- **Archivos golden de CI**: el flujo `golden.yml` vuelve a arrancar el compilador (bootstrap) y compara las salidas Plain/Fancy/Json y de locale contra el binario recién generado.

### Added

- **canvas (capa de pintura)**: mezcla alfa ARGB (`argb`, `blend_pixel`), antialiasing por SDF de 1 px (`fill_rounded_rect_aa`, `stroke_rounded_rect_aa`, `fill_circle_aa`, `stroke_arc`), pila de recorte rectangular (`push_clip`/`pop_clip`/`clear_clips`), sombra suave en capas (`draw_shadow`) y degradado vertical (`fill_gradient_v`). `set_pixel` respeta el recorte y normaliza los colores ARGB.
- **OLL**: constantes con nombre `OLL_WINDOW…OLL_PROGRESS`, `tabs` ahora usa layout horizontal, `stack`/`container` superponen a sus hijos, campo `progress: stone` propio en `OllNode` (`value:`/`progress:`), barras de progreso dibujadas de verdad (pista + relleno AA), render con bordes redondeados antialiased y sombra suave mediante `elevation`, y colores `#RRGGBBAA` con canal alfa.
- **CI**: golden de render (`tests/render/run.sh`) compara el PPM de la escena (`attach {canvas}` + `attach {oll}`) bit a bit en el job de Linux.

### Fixed

- **llvmgen**: la constante entera `0` en un global con tipo puntero (p. ej. `forge x: Array<T> = []`) emitía `global ptr 0`, rechazado por llc — ahora emite `global ptr null`.
- **llvmgen (`emit_global_init`)**: los globales con tipo genérico `Array<T>` nunca pasaban la prueba `ofs_str_eq(gt, "Array")`, así que el buffer interno no se reservaba en `__ofs_init_globals` y cada `ofs_array_push` era un no-op (el parser/lexer del compilador autohospedado perdía el estado). Ahora se reconoce el prefijo `Array<` y se inicializan los elementos literales. Corrige el bootstrap v3→v4 y la compilación nativa de `installer_wizard_linux.ofs` en el release.
- **llvmgen (bucles)**: cada iteración de `while`/`cycle` ahora envuelve el cuerpo con `llvm.stacksave`/`llvm.stackrestore` — las allocas de variables locales al cuerpo ya no se acumulan en la pila en compilaciones sin optimizar (`llc -O0`). Corrige el desbordamiento de pila al renderizar rectángulos grandes en el Canvas (p. ej. Catalyst Browser) y en cualquier bucle con miles de iteraciones.
- **cli (`ofs run --native`)**: la salida del reporter de compilación se redirige a stderr, dejando limpia la stdout del programa (paridad con `ofs run` interpretado).
- **tests/golden**: el caso Fancy ANSI neutraliza explícitamente `NO_COLOR`/`OFS_LANG`, que se filtraban desde las variables del paso de CI; el bootstrap de la semilla vuelve a generar IR válido (82 buffers de arrays globales restaurados).
- **bootstrap**: corregidos `ofs_str_substr` sin terminación NUL, el conteo de límites de `attach {F:...}` y los tamaños de asignación de `monolith`.
- **portabilidad**: `snprintf` y `ofs_stone_to_obsidian` reescritos en OFS puro; `crystal` se formatea como `%.15g` con sufijo `.0` (paridad).
- **CI**: `clang -no-pie` (Linux) y `-Wl,-no_pie` (macOS) al enlazar `ofscc.ll` generado por llc non-PIE; `-Wl,/force:multiple` (Windows).

### Changed

- **Licencia v1.0.151**: adopción de **Apache-2.0** (`LICENSE`, `docs/legal/LICENSE`) y **Política de marca** (`TRADEMARK.md`).
- **`docs/pt/STATUS_REPORT.md`** movido a `src/archive/STATUS_REPORT.md`; el sitio apunta solo al CHANGELOG.

### Self-Hosted Compiler — Hitos 5–6 (Fixed)

- **Hito 5**: el typechecker ya no se queda sin memoria en rutas profundas del AST — las funciones de acceso frecuentes se extrajeron para mantener acotada la presión de pila durante los recorridos de diagnóstico.
- **Hito 6a (bucles de mina)**: las sentencias `anchor` diferidas se emitían al salir del ámbito en orden FIFO, rompiendo la semántica tipo `defer`. El generador LLVM ahora recorre la pila de anchors en orden inverso (LIFO) para que el anchor registrado más recientemente se ejecute primero, como en las convenciones de Go/Zig.
- **Hito 6b (assay / strata)**: las variantes de `strata` no tenían representación de objeto, lo que producía `icmp eq i64` inválidos contra cargas `ptr` y símbolos indefinidos al llamar constructores como `Crack("boom")`. Ahora las variantes son etiquetas globales `i64` registradas en `_variant_names`/`_variant_tags`, y los constructores de variantes se reducen a la constante de la etiqueta.
- **Codegen**: las cargas de elementos de `Array<T>` ahora infieren el tipo escalar correcto (`ptr` para `Array<obsidian>`, `double` para `Array<crystal>`, `i1` para `Array<bool>`), evitando patrones inválidos `call i64 @ofs_str_eq(ptr %x)` tras leer de un slot de tipo stone.
- **Typeck**: `is_variant_name` se restringió a PascalCase estricto (empieza en mayúscula, contiene al menos una minúscula, sin guiones bajos). Así `EXEC_OK`, `NK_CORE_DECL` y otras constantes SCREAMING_SNAKE no se tipan como variantes (lo que antes forzaba una alloca `ptr` y corrompía el código posterior).
- **Typeck / Codegen**: `type_node_name` ahora conserva la grafía genérica completa (`Array<obsidian>` en lugar de `Array` a secas), lo que permite a los consumidores posteriores (collect_globals, emisión de VAR_DECL, indexación de arrays) elegir el tipo de valor correcto.

### Self-Hosted Compiler — Hitos 5–6 (Validated)

- `ofs/tests/interpreter/10_geological_flow.ofs` → imprime `1 2 3 4 5 40 30 8 50 7 20 10` (orden LIFO exacto de los anchors entre ámbitos anidados y bucles).
- `ofs/tests/interpreter/11_assay_basic.ofs` → imprime `20`.
- `ofs/tests/interpreter/12_outcome_lode.ofs` → imprime `7` y `2` (variantes de strata usadas en `assay`).
- `ofs/tests/interpreter/13_question_propagation.ofs` → imprime `0` y `1` (semántica básica de propagación con `?`).

## [Unreleased — track 1 & low-level]
### Added
- Actualización del lenguaje Track 1 documentada y disponible en el compilador: enteros pequeños (`u8`, `u16`, `u32`, `u64`, `i8`, `i32`), `impl`, namespaces, valores de función/lambdas y el puente de runtime `window`.
- Módulo de biblioteca estándar `window.ofs` y entrada de paquete `window` para el acceso nativo a ventana/entrada.
- Módulo de biblioteca estándar `canvas.ofs` y entrada de paquete `canvas` para dibujo en búfer de píxeles sobre `window`.
- Gramática, snippets, documentación de hover y README de la extensión de VS Code actualizados para cubrir la sintaxis de Track 1 y el flujo canvas/window.
- Nuevo intrínseco `fault_unreachable()`: marca la ruta de código actual como inalcanzable; se reduce a `llvm.trap` seguido de la instrucción `unreachable` de LLVM, permitiendo que el optimizador elimine ramas muertas.
- Nuevo intrínseco `fault_memcpy(dst, src, len)`: se reduce a `llvm.memcpy` para copias masivas de memoria dentro de bloques `fracture`/`abyss`/`bedrock`.
- Nuevo intrínseco `fault_memset(dst, val, len)`: se reduce a `llvm.memset` para inicialización masiva de memoria dentro de bloques de bajo nivel.
- Sintaxis variádica `...` en las declaraciones `extern vein`: `extern vein printf(fmt: obsidian, ...) -> stone` ahora compila correctamente. Se añadió el token `ELLIPSIS` (`...`) al lexer y al parser.
- Implementados `ofs_webserver_serve_once` y `ofs_webserver_serve_forever` en el runtime (sockets POSIX en Linux/macOS; Winsock en Windows). Antes estaban declarados en la cabecera pero sin implementación, lo que causaba errores de enlace en cualquier programa que los llamara.

### Changed
- La generación de código de la sentencia `match` ahora está completa. Antes solo se generaba la rama `default`; todas las ramas `case` se compilan ahora como una cadena de instrucciones de comparación y salto. Las ramas de cadenas usan `ofs_str_eq`, las numéricas comparación de enteros/flotantes.
- El ensamblador en línea (`asm "..."`) ahora se reduce correctamente a `InlineAsm` de LLVM. Antes existía el manejador `gen_inline_asm`, pero nunca lo llamaba `gen_expr`, así que todas las sentencias `asm` se descartaban en silencio. Los operandos de entrada ahora se reenvían con restricciones de registro por defecto.
- El tipo del símbolo `fault_step` en la tabla de símbolos integrada se corrigió de `stone` a `*stone` (shard de stone), coincidiendo con lo que ya devolvía `check_call` en los puntos de llamada.
- `register_builtin_symbols` reescrito para mayor claridad: cada intrínseco ahora tiene su tipo de retorno exacto registrado en una tabla, en lugar de un `stone` genérico por defecto.

### Fixed
- El actualizador de Windows ahora resuelve la última etiqueta de release de forma más robusta, siguiendo redirecciones y recurriendo a la API de GitHub cuando hace falta.
- **Crítico**: la igualdad (`==`) y desigualdad (`!=`) de valores `obsidian` ahora usan `ofs_str_eq` (comparación de contenido). Antes comparaban direcciones de puntero, por lo que dos literales de cadena con el mismo contenido podían resultar distintos.
- **Crítico**: las expresiones `asm "..."` nunca se reducían a IR (faltaba el caso de despacho en `gen_expr`). Se corrigió añadiendo la rama `InlineAsmExpr` en `gen_expr` y en `check_expr` del analizador semántico.
- La comprobación semántica de una función `void` que devuelve un valor ahora lanza un error apropiado (`void function cannot return a value`) en lugar de generar en silencio IR inválido que falla en los verificadores de LLVM.
- `gen_return` ahora tiene una protección a nivel de codegen: si una función `void` llegara a pasar el análisis semántico con una expresión de retorno, codegen evalúa la expresión por sus efectos y emite `ret void`, evitando un fallo del verificador de LLVM.
- Corregido `examples/baremetal_minimal.ofs`: `vein main()` (void, sin entrada core) con `return 42` se sustituyó por una estructura apropiada de `core main()` + función auxiliar.
- `gen_inline_asm` antes ignoraba por completo `inputs` (cadena de restricciones vacía) y usaba solo el primer nombre de salida como restricción. Ahora construye la cadena de restricciones a partir de las salidas, restricciones `"r"` por entrada y secciones de clobber.

### Added
- Nuevo comando `ofs asm <file.ofs>` para emitir ensamblador nativo del destino desde el backend LLVM actual.
- Documento de hoja de ruta de bajo nivel centrado en la coexistencia nativa de OFS entre código de alto y bajo nivel en el mismo programa.
- Módulo de biblioteca estándar `bedrock` con auxiliares de celdas de heap de primer nivel OFS para el manejo manual de estado.
- Comando de la extensión de VS Code para emitir ensamblador nativo desde el archivo OFS activo.
- Documentación de dirección del lenguaje ampliada, que cubre los objetivos de bajo nivel nativos de OFS y la hoja de ruta de interoperabilidad externa.
- Nuevo bloque tipado de bajo nivel `bedrock { ... }` en el lenguaje.
- Nueva forma de declaración `rift vein` para los límites de interoperabilidad externa nativos de OFS.
- Nuevos intrínsecos `fault_*` para operaciones de bajo nivel parecidas a las de máquina, con nombres nativos de OFS.
- Nuevo intrínseco `fault_step` para el avance tipado de punteros.
- Nuevos auxiliares `bedrock_region_*` para regiones de almacenamiento explícitas de bajo nivel.
- Nuevo módulo `rift.ofs` con envoltorios nativos de OFS alrededor de los límites de runtime externo.
- Nuevos intrínsecos `fault_cut` y `fault_patch` para extracción y reemplazo de campos.
- Nuevos auxiliares `bedrock_view_*` para vistas de bajo nivel sobre regiones.
- Nuevos intrínsecos `fault_fence` y `fault_trap` para ganchos de barrera/trampa a nivel de máquina.
- Nuevo intrínseco `fault_prefetch` y auxiliar `bedrock_prefetch` para la preparación de punteros con caché en código OFS de bajo nivel.
- Nuevos auxiliares de carriles como `bedrock_lane8_get` y `bedrock_lane8_set`.
- Nuevos auxiliares de carriles sensibles al endianness como `bedrock_lane16_le_get`, `bedrock_lane16_be_get`, `bedrock_lane32_le_set` y `bedrock_lane32_be_set`.
- Nuevo módulo `bedrock_packet.ofs` para vistas de paquete/cabecera sobre regiones de bajo nivel.
- Nuevos auxiliares de paquete para ventanas de cabecera sensibles al endianness, como `bedrock_packet_opcode_be16` y `bedrock_packet_tail_le16`.
- Nuevos metadatos `bind`/`abi` en las declaraciones `rift vein` y `extern vein` para límites de interoperabilidad más explícitos.
- Nuevo soporte de `layout native|packed|c` en las declaraciones `monolith`.
## [1.2.0] - 2026-04-13 — Compilador C++ eliminado

### Hito principal: implementación puramente OFS

**El código fuente del compilador C++ se eliminó del repositorio. El lenguaje ahora es 100% self-hosted.**

Toda la implementación del lenguaje y el pipeline de compilación usan únicamente OFS.

### Removed

- `ofs/src/` - Código fuente heredado del compilador C++ (main.cpp, lexer, parser, codegen, semantic, runtime, ast)
- `ofs/CMakeLists.txt` - configuración de compilación CMake
- Todas las dependencias de C++ del código base

### Estructura del repositorio (simplificada)

```
ofs/
  ├── ofscc/           # Compilador de OFS escrito en OFS (~4500 LOC)
  ├── stdlib/          # Biblioteca estándar de OFS
  ├── examples/        # Programas de ejemplo
  ├── tests/           # Suite de pruebas
  └── docs/            # Documentación
```

### Notas

- El compilador se distribuye en los releases como el binario `ofs`
- Instalación para usuarios: `sudo mv ofs/bin/ofscc /usr/local/bin/ofs`
- Arranque del lenguaje: `bash ofs/bootstrap/scripts/bootstrap-minimal.sh`
- No se necesita runtime ni compilador de C++ para compilar OFS

---


## [1.1.1] - 2024-12-XX — Releases automatizados solo nativos

### Hito principal: automatización de CI/CD y cero dependencias de C++

**¡Pipeline de automatización completo! Los releases se generan en ~30 segundos usando solo el compilador nativo de OFS.**

### Added - CI/CD y automatización de releases

- **Flujo de GitHub Actions** (`.github/workflows/release-native.yml`):
  - Trabajos multiplataforma: Linux, macOS, Windows
  - Disparadores automáticos: etiquetas Git (`v*`) + `workflow_dispatch` manual
  - Scripts de release por plataforma (solo nativos, sin paso de compilación C++)
  - Integración automática de build de la extensión de VS Code
  - Checksums (SHA256) + metadatos de versión generados automáticamente

- **Scripts de bootstrap** (`ofs/bootstrap/scripts/`):
  - `bootstrap-minimal.sh`: recompilación del compilador solo nativo (~2 s, determinista)
  - `release-native-only.sh`: empaquetado de release Linux/macOS (~5 s)
  - `release-native-only-windows.ps1`: empaquetado de release Windows (~5 s)
  - Validación de determinismo integrada (compila dos veces, verifica que los binarios son idénticos)

- **Actualizaciones de documentación**:
  - `docs/GETTING_STARTED.md` — Reescritura completa para el compilador nativo
  - `docs/BOOTSTRAP.md` — Actualizado con el flujo de bootstrap-minimal.sh
  - `ofs/docs/README_BAREMETAL.md` — Instrucciones del compilador nativo
  - `ofs/docs/BAREMETAL_PROFILE.md` — Instrucciones del compilador nativo
  - `README.md` — Todos los comandos actualizados de `ofs build` al enfoque nativo

- **Extensión de VS Code** (v1.0.8):
  - Nombre visible actualizado: "Obsidian Fault Script (Native)"
  - Destacados: "(C++-free)" y "self-hosted native compiler"
  - Detección automática del compilador nativo en `ofs/dist/ofscc`

### Performance Improvements

- **Generación de release**: 5-15 minutos → ~30 segundos (¡300x más rápido!)
- **Verificación de bootstrap**: comprobación completa de determinismo en 2-5 segundos
- **Tiempo total de CI/CD**: 15+ minutos → ~30 segundos

### Removed

- Paso de compilación de C++ del pipeline de release
- Dependencias de LLVM/Clang en CI/CD
- Requisitos de compilación CMake para los releases
- Referencias al comando `ofs` antiguo (sustituidas por el enfoque nativo)

### Technical Details

**Nuevo pipeline de release:**
```
Etiqueta Git → GitHub Actions → (bootstrap-minimal en paralelo para Linux/macOS/Windows)
→ (scripts de release por plataforma) → Artefactos consolidados + GitHub Release
```

**Cero dependencias:**
- No se necesita compilador de C++ para los releases
- No se requiere CMake
- Solo Bash + GCC/Clang nativos

---


## [1.1.0] - 2026-04-12 — Compilador autohospedado

### Hito principal: el compilador de OFS es self-hosting

**¡El compilador de OFS ahora está escrito enteramente en OFS!**

Tras implementar un pipeline de compilación completo en OFS puro (lexer, parser, verificador de tipos, generador de código), el lenguaje alcanzó un hito crítico: puede compilarse a sí mismo. Es el mismo logro que Go (2015), Rust (2011) y C (1972).

### Added - Compilador autohospedado

- **`ofs/ofscc/`** — Compilador completo de OFS escrito en OFS (~4.500 LOC, 13 archivos)
  - `chars.ofs` — Constantes ASCII y clasificación de caracteres
  - `tokens.ofs` — Tipos de token (más de 80 tipos) + búsqueda de palabras clave
  - `lexer.ofs` — Analizador léxico (tokenizador)
  - `nodes.ofs` — Tipos de nodo del AST (más de 60)
  - `ast.ofs` — Pool del AST + gestión de nodos
  - `symbols.ofs` — Definiciones de la tabla de símbolos
  - `parser.ofs` — Descenso recursivo + Pratt parser
  - `typeck.ofs` — Verificador de tipos con pila de ámbitos
  - `codegen.ofs` — Generador de código C
  - `fileio.ofs` — E/S de archivos + declaraciones FFI
  - `ofscc.ofs` — Driver principal (pipeline de 6 fases)
  - Archivos de prueba: `test_lexer_units.ofs`, `test_lexer_parser.ofs`
  - Script de bootstrap: `test_bootstrap.sh`

- **Verificación del bootstrap**:
  - `ofs build ofs/ofscc/ofscc.ofs -o ofscc_v1` (C++ → OFS)
  - `./ofscc_v1 ofs/ofscc/ofscc.ofs -o ofscc_v2` (OFS → OFS pasada 1)
  - `./ofscc_v2 ofs/ofscc/ofscc.ofs -o ofscc_v3` (OFS → OFS pasada 2)
  - Cuando `ofscc_v2 === ofscc_v3`, el compilador de C++ pasa a ser opcional

- **Documentación**:
  - `docs/BOOTSTRAP.md` — Guía completa de bootstrap (paso a paso)
  - `docs/COMPILER_ARCHITECTURE.md` — Internals del compilador (fases, estructuras de datos)
  - `ofs/ofscc/README.md` — README del proyecto del compilador + instrucciones de pruebas

### Changed

- **El compilador C++ ahora está marcado como heredado**
  - `src/` se mantiene por compatibilidad de compilación
  - Las versiones futuras podrían eliminar C++ por completo
  - Todo el desarrollo nuevo usa el compilador OFS

- **Sistema de compilación**:
  - `ofs build` sigue disponible para el bootstrap
  - Nueva ruta: `ofscc input.ofs -o output`
  - Ambos compiladores funcionan en paralelo (por ahora)

### Added - Incremento de implementación Fase B (2026-04-12)

- **Mejoras de Parser/Verificador de tipos/Codegen en `ofs/ofscc/`**
  - Parseo de `tremor/catch` y bloque de codegen rebajado
  - Parseo de `monolith` + emisión de struct C
  - Parseo de `impl` + aplanado de métodos (`Type__method`)
  - Parseo de `namespace` + emisión de miembros anidados
  - Parseo de `strata` + base de codegen de etiquetas de enum

- **Puente CLI para `ofscc`**
  - `OFSCC_INPUT`, `OFSCC_OUTPUT`, `OFSCC_C_OUT`, `OFSCC_MODE`, `OFSCC_OPT`
  - soporte para los niveles de optimización `-O0`, `-O2`, `-O3`

- **Expansión de CI/CD**
  - `build.yml` ahora incluye build de macOS
  - corregido el comando stat de determinismo en Linux (`stat -c%s`)
  - la instalación de LLVM en Windows ya no fija una versión exacta de Chocolatey

- **Flujo de release autohospedado (preparación Fase D/E)**
  - `.github/workflows/selfhosted-release.yml`
  - descarga el compilador de bootstrap desde Releases
  - compila `ofscc.ofs` y genera los activos del instalador con las herramientas de OFS

- **Actualización de la extensión de VS Code**
  - detecta automáticamente los binarios autohospedados (`ofscc`, `ofscc_v2`) en el workspace
  - el comando de ejecución se adapta cuando el compilador es `ofscc`

### Technical Details

**Arquitectura del compilador**:
- **Fase 1: Lexer** — Tokeniza el código fuente (80 tipos de token)
- **Fase 2: Parser** — Descenso recursivo + precedencia Pratt
- **Fase 3: Verificador de tipos** — Pila de ámbitos + inferencia de tipos
- **Fase 4: Codegen** — Emite código C válido
- **Fase 5: Compilación C** — Invoca `gcc -O2`
- **Fase 6: Salida** — Ejecutable nativo

**Características soportadas**:
- Todos los tipos primitivos (stone, crystal, obsidian, bool, u8-u64, i32)
- Funciones, variables, constantes
- Arrays, monoliths (structs)
- Control de flujo (if/else, while, cycle, match, break, continue, return)
- Operadores (aritméticos, lógicos, a nivel de bits, comparación)
- Literales de cadena con secuencias de escape
- Comentarios (//, /* */)
-  Inferencia de tipos (básica)
-  Mensajes de error (simples)

**Limitaciones (abordadas en v1.2)**:
- Análisis de argumentos de línea de comandos
- impl/namespace/strata (solo parseo)
- Recuperación completa de errores
- Diagnósticos más allá de stderr

### Performance

| Fase | Tiempo |
|---|---|
| Lexer | <1ms |
| Parser | ~10ms |
| Verificación de tipos | ~5ms |
| Codegen | ~10ms |
| GCC | ~100ms |
| **Total** | ~150ms |

### Files Changed

```
Creados:
  docs/BOOTSTRAP.md
  docs/COMPILER_ARCHITECTURE.md
  ofs/ofscc/*.ofs (13 archivos)
  ofs/ofscc/*.sh (scripts de prueba)
  ofs/ofscc/README.md

Modificados:
  (Actualizado abajo en la sección "Changed")
```

### Next Steps (v1.2)

- [ ] Implementar el análisis de argumentos de línea de comandos
- [ ] Añadir soporte completo de impl/namespace/strata
- [ ] Mejorar los mensajes de error con línea/columna
- [ ] Optimizar el código C generado
- [ ] Portar a Linux/macOS/Windows
- [ ] Extensión de VS Code usando ofscc

### Breaking Changes

Ninguno. El compilador C++ sigue siendo el predeterminado. Optar por el compilador OFS:

```bash
# Antiguo (sigue funcionando)
ofs build input.ofs -o output

# Nuevo (opt-in)
./ofscc input.ofs -o output
```

---



## [1.0.151] - 2026-10-01 — Licencia Apache-2.0 y política de marca

### Changed

- Licencia actualizada a **Apache-2.0** (Apache License, Version 2.0), alineando OFS con los principales lenguajes de sistemas (Rust/LLVM), con protección explícita de patentes y requisitos de atribución.
- Se añadió la **Política de marca** (`TRADEMARK.md`), que protege los nombres oficiales del proyecto ("Obsidian Fault Script", "OFS") y la marca frente a reutilizaciones comerciales no autorizadas y bifurcaciones confusas.

## [1.0.148] - 2026-09-30 — Backend de intérprete híbrido y pruebas diferenciales

### Added

- **Backend de intérprete AST (`interpreter.ofs`)**: OFS es ahora un lenguaje híbrido real. Ejecutar `ofs <file.ofs>` o `ofs run <file.ofs>` evalúa directamente el AST tipado sin invocar las fases de LLVM, Clang o el linker, ofreciendo ejecución en sub-milisegundos para scripting y prototipado.
- **Value Monolith y heap dinámico**: soporte completo en runtime para primitivos (`stone`, `crystal`, `obsidian`, `bool`), manejadores para `Array<T>`, structs `monolith` en heap con semántica de copia por valor, y punteros con la tabla `Location`.
- **Ámbito lexico de marcos de llamada**: seguimiento de la base del marco (`_current_frame_base` y `_globals_count`) que garantiza que las variables locales de las funciones llamadoras nunca se filtren ni oculten las declaraciones globales de las funciones llamadas.
- **Despacho de métodos mediante `impl`**: soporte para métodos declarados en `impl Type { vein method(self: Type) ... }` llamados directamente sobre instancias (`instance.method()`).
- **Suite de pruebas diferenciales**: ejecutor automático `ofs/tests/interpreter/test_differential.sh` con 9 suites de prueba de extremo a extremo que verifican la equivalencia 1 a 1 de la salida entre el modo interpretado y los builds nativos de LLVM.
- **Integración CI**: validación de pruebas diferenciales añadida a los flujos de GitHub Actions en Linux y macOS.

### Changed

- `ofs run <file.ofs>` ejecuta por defecto de forma interpretada e instantánea; `ofs run --native <file.ofs>` está disponible para compilación temporal + ejecución.
- El ejecutor de PowerShell para Windows `src/packaging/windows/ofs.ps1` se actualizó con el modo interpretado y el flag `--native` para la paridad completa entre Linux, macOS y Windows.

---


## [1.0.101] - 2026-05-31 — Endurecimiento de los instaladores nativos

### Fixed

- Corregido el manejo de punteros opacos de LLVM 14 en CI y en los builds de release.
- Corregido el comportamiento del wrapper de Bash 3/macOS cuando los flags de LLVM IR están vacíos.
- Corregidas las comprobaciones de ejemplos para que las rutas relativas de `attach {F:...}` se ejecuten desde el directorio de cada ejemplo.
- Corregida la colisión de empaquetado del release de Windows usando un directorio de staging dedicado `windows-dist`.

### Release

- La automatización del release ahora produce instaladores nativos para Debian/Ubuntu, Fedora, Arch, macOS y Windows.
- El empaquetado de la extensión de VS Code se incluye en los activos del release.
- El CI de Windows valida el binario del compilador nativo y el paquete del instalador; Linux/macOS siguen ejecutando las pruebas de humo completas de ejemplos y stdlib.

---


## [1.0.3] - 2026-04-01
### Added
- Nuevas palabras clave: `match`, `case`, `default`, `const`, `strata`, `tremor`, `catch`, `throw`.
- Nuevos operadores: `<<`, `>>`, `%=`.
- Soporte de operadores a nivel de bits en expresiones binarias (`&`, `|`, `^`, `<<`, `>>`).
- Nuevos nodos del AST para:
  - Pattern matching (`MatchStmt`, `MatchArm`)
  - Variables inmutables (`ConstStmt`)
  - Enumeraciones (`StrataDecl`)
  - Manejo de errores (`TremorStmt`, `ThrowStmt`)

### Changed
- El parser reconoce ahora `strata`, `const`, `match/case/default`, `tremor/catch`, `throw`.
- Analizador semántico con stubs y validación básica para las nuevas construcciones.
- Stubs de codegen para las nuevas construcciones y soporte de expresiones de bits/desplazamiento.
- Pruebas de lexer y parser ampliadas para los nuevos tokens y la nueva sintaxis.
- CI/CD de release mejorado para Linux/Windows/macOS con mejor detección de LLVM.

### Fixed
- Flujo de Windows: ya no falla cuando hay una versión más nueva de LLVM instalada y se intenta un downgrade.
- Flujo de Linux: `LLVM_DIR` ahora se fija explícitamente para evitar tomar una versión antigua de LLVM del runner.
- CMake: el backend de LLVM se elige según la arquitectura (x86_64 vs arm64).
- Codegen: compatibilidad con LLVM 15/17+ para `Reloc::Model` (`llvm::Optional` vs `std::optional`).


## [1.0.2] - 2026-04-01
### Added
- Pipeline de release automatizado con binarios para Linux, Windows y macOS.


## [1.0.1] - 2026-04-01
### Changed
- README simplificado para una instalación y uso rápidos.


## [1.0.0] - 2026-04-01
### Added
- Primera versión estable del lenguaje OFS (lexer, parser, análisis semántico, codegen LLVM, runtime y ejemplos).
