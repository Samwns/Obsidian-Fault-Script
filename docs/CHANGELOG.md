# Changelog

All notable changes to this project are recorded here.

> **Esquema de versão.** As versões oficiais são as tags `vMAJOR.MINOR.PATCH` do GitHub (`v1.0.0` ... `v1.0.181`), com o patch incrementado automaticamente pelo workflow `release.yml`; é esse número que `ofs version` mostra num pacote instalado. O `OFS_VERSION="1.0.169"` do wrapper `src/packaging/linux/ofs` é só o valor de checkout local (o release troca `__OFS_VERSION__`), por isso fica atrás da última tag. Os rótulos `1.1.0`, `1.1.1` e `1.2.0` mais abaixo são marcos históricos de abril de 2026 que **nunca foram tags**; as entradas foram mantidas por ordem de assunto, não de versão. O texto "Unreleased" cobre tudo o que veio depois da `v1.0.151`.

## [Unreleased]

### Added / Adicionado — Entrada de texto pelo teclado do sistema (`android_input.ofs`, `oal.edit_open`)

- **PT** — novo stdlib `android_input.ofs` (Dialog + `EditText` por JNI, sem Java próprio) e `oal.edit_open`/`edit_state`/`edit_text`: teclado padrão do Android (Gboard) com cursor, rolagem, colar e autocompletar nos campos de texto; o `keyboard` da OAL continua como alternativa. Documentado em `docs/OAL_REFERENCE.md` e `docs/ANDROID.md`.
- **EN** — new stdlib `android_input.ofs` (Dialog + JNI `EditText`, no Java of our own) and `oal.edit_open`/`edit_state`/`edit_text`: the Android default keyboard (Gboard) with cursor, scrolling, paste and autocomplete in text fields; OAL `keyboard` remains as an alternative. Documented in `docs/OAL_REFERENCE.md` and `docs/ANDROID.md`.
- **ES** — nuevo stdlib `android_input.ofs` (Dialog + `EditText` por JNI, sin Java propio) y `oal.edit_open`/`edit_state`/`edit_text`: teclado predeterminado de Android (Gboard) con cursor, desplazamiento, pegar y autocompletar en los campos de texto; el `keyboard` de OAL sigue como alternativa. Documentado en `docs/OAL_REFERENCE.md` y `docs/ANDROID.md`.

### Added / Adicionado — OAL: barra de navegação inferior, menu suspenso, bottom sheet e segurar (long press)

- **PT** — **oalc + `oal_runtime`**: novos widgets genéricos `navbar` (abas embaixo com ícone e rótulo; o conteúdo e os `fab` respeitam a altura da barra) e `popup menu|sheet` (menu suspenso ⋮ ou bottom sheet, com fundo escurecido e tocar fora fecha); atributo `long=ação(item)` em `card`/`item` (segurar ~0,45 s, cancelado ao arrastar) e `bar "" logo=ícone`. As pilhas de `fab` no topo encolhem para não cobrir o D-pad/losango do mesmo lado. Documentado em `docs/OAL_REFERENCE.md`.
- **EN** — **oalc + `oal_runtime`**: new generic widgets `navbar` (bottom tabs with icon and label; content and `fab`s respect its height) and `popup menu|sheet` (⋮ drop-down or bottom sheet, dimmed scrim, tap outside closes); `long=action(item)` attribute on `card`/`item` (long press ~0.45 s, cancelled by dragging) and `bar "" logo=icon`. Top `fab` stacks shrink so they do not cover the D-pad/diamond on the same side. Documented in `docs/OAL_REFERENCE.md`.
- **ES** — **oalc + `oal_runtime`**: nuevos widgets genéricos `navbar` (pestañas abajo con icono y etiqueta; el contenido y los `fab` respetan su altura) y `popup menu|sheet` (menú desplegable ⋮ o bottom sheet, fondo oscurecido, tocar fuera cierra); atributo `long=acción(item)` en `card`/`item` (pulsación larga ~0,45 s, se cancela al arrastrar) y `bar "" logo=icono`. Las pilas de `fab` superiores se encogen para no tapar el D-pad/rombo del mismo lado. Documentado en `docs/OAL_REFERENCE.md`.

### Fixed / Corrigido — compilador e runtime (`core`, `as crystal`, `ofs_str_char_at`, `impl`)

- **PT** — **parser**: `core` como nome de função (`vein core()`, também dentro de `namespace`) descartava o corpo em silêncio; agora é erro de compilação claro ("`core` e palavra reservada…"), e **qualquer erro sintático encerra o `ofscc` com código 1** (antes as mensagens saíam no stdout e o build seguia). O exemplo `showcase.ofs`, que usava um bloco `if (...): ... obsid` inexistente, foi corrigido. **llvmgen**: `ofs_array_get(a, i) as crystal` (e `a[i] as T`) lê o elemento no tipo de `Array<T>` e converte (`sitofp` para `stone`), como `(x: stone) as crystal`; antes os bits do inteiro eram lidos como `double` (o contorno via função com parâmetro `stone` não é mais necessário). **runtime (`facet.ofs`)**: o cache do `ofs_str_char_at` comparava a string com `==` entre `obsidian`, que vira `strcmp` (O(n) por chamada; laço sobre 1 MB = ANR no Android, cujo runtime é compilado deste fonte); agora compara ponteiros, O(1) amortizado. O índice é de byte. **typeck/interpretador**: `r.metodo(args)` de `impl T` vira `T__metodo(r, args)`; antes o build nativo gerava `@r__metodo` indefinido (`09_impl_methods`, agora no teste diferencial). Testes: `ofs/tests/regress/run.sh`. Ponto fixo do self-host mantido (IR gen1 == gen2).
- **EN** — **parser**: `core` as a function name (`vein core()`, also inside a `namespace`) silently dropped the body; it is now a clear compile error ("`core` e palavra reservada…"), and **any syntax error makes `ofscc` exit with code 1** (messages used to go to stdout while the build went on). The `showcase.ofs` example, which used a nonexistent `if (...): ... obsid` block, was fixed. **llvmgen**: `ofs_array_get(a, i) as crystal` (and `a[i] as T`) loads the element with the `Array<T>` type and converts it (`sitofp` for `stone`), like `(x: stone) as crystal`; the integer bits used to be read as a `double` (the workaround through a function with a `stone` parameter is no longer needed). **runtime (`facet.ofs`)**: the `ofs_str_char_at` cache compared the string with `==` between `obsidian` values, which is `strcmp` (O(n) per call; a loop over 1 MB caused an ANR on Android, whose runtime is compiled from this source); it now compares pointers, amortized O(1). The index is a byte index. **typeck/interpreter**: `r.method(args)` from `impl T` becomes `T__method(r, args)`; native builds used to emit an undefined `@r__method` (`09_impl_methods`, now in the differential test). Tests: `ofs/tests/regress/run.sh`. Self-host fixed point kept (IR gen1 == gen2).
- **ES** — **parser**: `core` como nombre de función (`vein core()`, también dentro de un `namespace`) descartaba el cuerpo en silencio; ahora es un error de compilación claro ("`core` e palavra reservada…"), y **cualquier error sintáctico termina `ofscc` con código 1** (antes los mensajes salían por stdout y el build seguía). Se corrigió el ejemplo `showcase.ofs`, que usaba un bloque `if (...): ... obsid` inexistente. **llvmgen**: `ofs_array_get(a, i) as crystal` (y `a[i] as T`) lee el elemento con el tipo de `Array<T>` y lo convierte (`sitofp` para `stone`), como `(x: stone) as crystal`; antes los bits del entero se leían como `double` (ya no hace falta pasar por una función con parámetro `stone`). **runtime (`facet.ofs`)**: la caché de `ofs_str_char_at` comparaba la cadena con `==` entre `obsidian`, que es `strcmp` (O(n) por llamada; un bucle sobre 1 MB causaba ANR en Android, cuyo runtime se compila desde este fuente); ahora compara punteros, O(1) amortizado. El índice es de byte. **typeck/intérprete**: `r.metodo(args)` de `impl T` pasa a `T__metodo(r, args)`; el build nativo generaba `@r__metodo` indefinido (`09_impl_methods`, ahora en la prueba diferencial). Pruebas: `ofs/tests/regress/run.sh`. Punto fijo del self-host mantenido (IR gen1 == gen2).

### Added — teclado hex (OAL)

- **PT** — OAL: camada de teclado hex do `keyboard`, com `oal.kb_layer = 2` (teclas 0-9, A-F, apagar, `#` e ↵). Usada no hex do seletor de cor do M.O.O.N.
- **EN** — OAL: hex layer of the `keyboard`, selected with `oal.kb_layer = 2` (keys 0-9, A-F, backspace, `#` and ↵). Used by the M.O.O.N color picker hex field.
- **ES** — OAL: capa hex del `keyboard`, con `oal.kb_layer = 2` (teclas 0-9, A-F, borrar, `#` y ↵). Usada en el campo hex del selector de color de M.O.O.N.

### Added — gamepad na tela com estilos de botões (OAL)

- **PT** — OAL: widget `diamond -> ação(item) corner= look=` (4 botões de ação em losango; estilos `PAD_KEYS` teclas, `PAD_XBOX`, `PAD_PLAY` com formas vetoriais ✕ ○ □ △ sem logotipos, `PAD_NINTENDO`), evento `on key` (teclas de controle/teclado repassadas com filtro e sem repetição), `oal.pad_connected` (controle físico detectado pela fonte do evento). `icons.ofs`: `BTN_CROSS/CIRCLE/SQUARE/TRIANGLE`, `KEY_ENTER`. `android.ofs`: `key_source`, `key_handled` (a tecla consumida não vai ao sistema) e analógico não vira toque.
- **EN** — OAL: `diamond -> action(item) corner= look=` widget (4 face buttons in a diamond; `PAD_KEYS` keycaps, `PAD_XBOX`, `PAD_PLAY` vector cross/circle/square/triangle with no logos, `PAD_NINTENDO`), `on key` event (filtered, repeat-free controller/keyboard keys), `oal.pad_connected` (physical gamepad detected from the event source). `icons.ofs`: `BTN_CROSS/CIRCLE/SQUARE/TRIANGLE`, `KEY_ENTER`. `android.ofs`: `key_source`, `key_handled` (a consumed key does not reach the system); analog sticks no longer act as touch.

### Added — fontes CFF/TTC com fallback, vídeo AVI (M.O.O.N)

- **PT** — `ttf.ofs`: outlines **CFF** (Type 2 charstrings, subrs locais/globais, fontes CID com FDArray/FDSelect), coleções **TTC** (`load_face`, `load_face_named`, `face_count`), **fallback por glifo** (`set_fallback`, `set_fallback_file` com carga preguiçosa, `has_glyph`), limite do cache de glifos (`pool_limit`, `clear_cache`). Texto TrueType sai igual bit a bit ao de antes. Novo `avi.ofs`: leitor AVI + decodificador Microsoft Video 1 (16 bits), igual bit a bit ao ffmpeg. `wav.from_pcm`, `audio.position`. Contorno do compilador: `ofs_array_get(...) as crystal` lê o elemento como `double` sem converter (passe por uma função com parâmetro `stone`).
- **EN** — `ttf.ofs`: **CFF** outlines (Type 2 charstrings, local/global subrs, CID fonts with FDArray/FDSelect), **TTC** collections (`load_face`, `load_face_named`, `face_count`), **per-glyph fallback** (`set_fallback`, lazily loaded `set_fallback_file`, `has_glyph`), glyph cache limit (`pool_limit`, `clear_cache`). TrueType text renders bit-identically to before. New `avi.ofs`: AVI reader + Microsoft Video 1 (16-bit) decoder, bit-identical to ffmpeg. `wav.from_pcm`, `audio.position`. Compiler workaround: `ofs_array_get(...) as crystal` loads the element as `double` without converting (go through a function with a `stone` parameter).

### Added — plugins `.moonpack`, WebView, zip, seletor de cor (OAL)

- **PT** — `zip.ofs`: extração em blocos (`xbegin`/`xstep`/`xstep_ms`/`xpct`/`xabort`) sobre o novo inflate retomável do `png.ofs` (`inflate_begin/step/take/abort`); CRC e sha256 calculados enquanto extrai. Instalar o `.so` de 76 MB do Godot no M.O.O.N deixou de gerar ANR com toques durante a extração.
- **EN** — `zip.ofs`: chunked extraction (`xbegin`/`xstep`/`xstep_ms`/`xpct`/`xabort`) on top of the new resumable inflate in `png.ofs` (`inflate_begin/step/take/abort`); CRC and sha256 are computed while extracting. Installing Godot's 76 MB `.so` in M.O.O.N no longer ANRs on touches during extraction.


- **PT** — `zip.ofs` (ler/extrair zip com o inflate do `png.ofs`, criar zip sem compressão, `crc32`), `sha256.ofs` (SHA-256 e CRC-32), `png.inflate_raw`/`out_ptr`/`out_len`; `android.ofs`: JNI genérico (`jni_call`, `jni_new`, `jni_scall`, `jni_static_int_field`, referências globais, arrays de bytes/objetos); `android_http.ofs` (HTTPS por `HttpURLConnection` via JNI, em passos, também `file://`; o OpenSSL não é ligável num APK); `android_webview.ofs` (WebView em tela cheia num `Dialog`, via JNI, sem Java próprio: HTML com shim injetado por `loadDataWithBaseURL`, canal de volta pelo `document.title`); `icons.ofs`: `RPGM`, `TYRANO`. OAL: widget **`colorpicker value=expr -> ação(item)`** (quadrado saturação/valor, barra de matiz, prévia e hex), `oal.set_palette_full` (botão, botão pressionado, borda, selo; texto sobre destaque/botão por contraste), `oal.hsv_to_rgb`/`contrast_ink`; o botão `back` da barra agora fica no **lado esquerdo**, antes do título (`oll.move_child_first`). Cuidado do compilador: `core` é palavra reservada (o corpo da função some em silêncio).
- **EN** — `zip.ofs` (read/extract zip with the `png.ofs` inflate, create stored zip, CRC-32), `sha256.ofs` (SHA-256 and CRC-32), `png.inflate_raw`/`out_ptr`/`out_len`; `android.ofs`: generic JNI (`jni_call`, `jni_new`, `jni_scall`, `jni_static_int_field`, global refs, byte/object arrays); `android_http.ofs` (HTTPS through `HttpURLConnection` via JNI, stepwise, also `file://`; OpenSSL cannot be linked in an APK); `android_webview.ofs` (full-screen WebView in a `Dialog` via JNI, no Java of our own: HTML with the shim injected by `loadDataWithBaseURL`, return channel through `document.title`); `icons.ofs`: `RPGM`, `TYRANO`. OAL: **`colorpicker value=expr -> action(item)`** widget (saturation/value square, hue bar, preview and hex), `oal.set_palette_full` (button, pressed button, border, badge; text on accent/button by contrast), `oal.hsv_to_rgb`/`contrast_ink`; the bar `back` button now sits on the **left**, before the title (`oll.move_child_first`). Compiler caveat: `core` is a reserved word (the function body silently disappears).
- **ES** — `zip.ofs` (leer/extraer zip con el inflate de `png.ofs`, crear zip sin compresión, CRC-32), `sha256.ofs` (SHA-256 y CRC-32), `png.inflate_raw`/`out_ptr`/`out_len`; `android.ofs`: JNI genérico (`jni_call`, `jni_new`, `jni_scall`, `jni_static_int_field`, referencias globales, arrays de bytes/objetos); `android_http.ofs` (HTTPS con `HttpURLConnection` por JNI, por pasos, también `file://`; OpenSSL no se puede enlazar en un APK); `android_webview.ofs` (WebView a pantalla completa en un `Dialog` por JNI, sin Java propio: HTML con el shim inyectado por `loadDataWithBaseURL`, canal de vuelta por `document.title`); `icons.ofs`: `RPGM`, `TYRANO`. OAL: widget **`colorpicker value=expr -> acción(item)`** (cuadrado saturación/valor, barra de matiz, vista previa y hex), `oal.set_palette_full` (botón, botón pulsado, borde, sello; texto sobre acento/botón por contraste), `oal.hsv_to_rgb`/`contrast_ink`; el botón `back` de la barra ahora va a la **izquierda**, antes del título (`oll.move_child_first`). Aviso del compilador: `core` es palabra reservada (el cuerpo de la función desaparece en silencio).

### Added — Android (aarch64) e OAL v0

- **Alvo Android**: `OFSCC_TARGET=android` emite `aarch64-unknown-linux-android26`; o runtime `magma` compila para aarch64 sem mudanças (só usa a libc/bionic). `net.resolve` agora lê `struct addrinfo` do bionic (ordem `ai_addr`/`ai_canonname` invertida em relação à glibc). Doc: `docs/ANDROID.md` (+ `en/`, `es/`).
- **`ofs build --target android app.ofs|app.oal`**: gera o APK sem Gradle/Java (`clang` do NDK, `-Wl,-init,__ofs_init_globals`, `aapt2`, `zipalign -P 16`, `apksigner`), com `--install`, `--serial` e `--manifest`. Se o `ofscc` instalado não tiver `OFSCC_TARGET`, compila um do fonte (`~/.cache/ofs/android/`). O wrapper rodando do repo usa a stdlib do repo.
- **`fault_addr(vein)` e `fault_call(endereco, args...)`**: endereço de uma vein como `stone` (`ptrtoint`) e chamada indireta com ABI C (`inttoptr` + `call`). Permite callbacks C (`qsort`, NativeActivity, ALooper, AChoreographer) e tabelas de funções (JNI). Teste: `ofs/tests/runtime/test_fault_addr_qsort.ofs`. Ponto fixo do self-host mantido (IR gen1 == gen2).
- **stdlib `android.ofs`**: backend NativeActivity **100% OFS, sem nenhum `.c`** (o `android_native_app_glue` saiu): exporta `ANativeActivity_onCreate`, preenche `activity->callbacks` com `fault_addr`, entrada por `AInputQueue_attachLooper`, frames por `AChoreographer_postFrameCallback`, tudo na thread principal; ciclo de vida, toque, voltar, rotação, áreas das barras do sistema, `ANativeWindow` RGBA, `set_orientation` via JNI, caminhos do app, log.
- **OAL v0 (Obsidian Android Language, `.oal`)**: `ofs/tools/oalc.ofs` gera `AndroidManifest.xml` + `<nome>_oal.ofs`; `app`, `use`/`attach`, `style` (`fill`, `ink`, `curve`, `opacity`, `pressed`), `screen` com `bar`, `button`, `button.icon`, `title`, `text`, `list`/`grid` com `from`, `card`, `row`, `portrait`/`landscape`, `fab`, `dpad`; ações `-> funcao(item)`, `@tela`, `@back`, `@quit`, `@portrait`/`@landscape`/`@auto`; `on back/pause/resume/rotate/low_memory`. Runtime `oal_runtime.ofs` (núcleo de layout do `oll.ofs`, `canvas`/`raster`/`ttf`/`icons`) + `oal_android.ofs`; `oal_x11.ofs` é experimental. Doc: `docs/OAL_REFERENCE.md` (+ `en/`, `es/`); teste: `ofs/tests/oal/run.sh`.
- **`icons.ofs`**: ícones `PLUGIN`, `SAVE`, `FOLDER`, `PLAY`, `HAMBURGER`, `GAMEPAD` e apelidos em `by_name` (`plugins`, `import`, `load`, `config`...).
- **stdlib**: novos `wav.ofs` (decodificador WAV PCM 8/16 bits e Microsoft ADPCM) e `audio_android.ofs` (saída AAudio com mixer na thread de áudio); `ttf.ofs` ganhou texto em várias linhas (`ttf.wrap`, `ttf.draw_wrapped`); `icons.ofs` ganhou ícones de interface (`AUTO`, `SKIP`, `EYE`, `ROTATE`, `TRASH`, `VOLUME`, `KEYBOARD`, `JOYSTICK`, `TABS` ...) e de motor (`WINDOWS`, `RENPY`, `UNITY`, `MOON`, `UNKNOWN`).

- **OAL v1**: widgets `image`, `canvas [full] draw= touch=` (área desenhada pelo app, com toques por dedo), `tabs`/`tab`, `slider`, `toggle`, `progress`, `field`, `keyboard` (teclado na tela), blocos `if`/`else`; `item`/`card` com `image=`, `icon=` dinâmico, `badge=`/`badge_icon=`; `text`/`empty` em várias linhas; rótulos e ícones como expressões; `fab.classe.icon`; cantos `bottom_row`/`top_row`/`right_column`/`left_column` ou expressão; eventos `on start` e `on frame`; `app` com `assets "pasta"` e ícone PNG copiado para `res/mipmap/`. Runtime: **multitoque por dedo**, cache de PNG, `oal.toast`, `set_animating`, `reset_to`/`replace`, `set_text_scale`, `set_overlay_scale`, `set_palette` (tema), cópia para a janela só quando algo muda. Doc: `docs/OAL_REFERENCE.md` (+ `en/`, `es/`).
- **`android.ofs`**: multitoque (`touch_id`, `EV_POINTER_DOWN/UP`, `ptr_*`), ajudantes JNI (`jni_find_class`, `jni_string`, `jni_call_object`, `jni_call_void1`, `jni_static_bool`, `package_name`, `start_intent`), `is_external_storage_manager`/`open_all_files_settings` (`MANAGE_EXTERNAL_STORAGE`), **tela cheia imersiva** (`set_immersive`, reaplicada ao recuperar o foco), desenho na área do recorte da câmera (`set_cutout_short_edges`), `keep_screen_on`, `asset_read`, `redraw_needed`. A CLI linka `-laaudio`. Doc: `docs/ANDROID.md` (+ `en/`, `es/`).
- **`android.ofs` — modo motor** (hospedar SDL/Godot na própria `NativeActivity`, sem Java no APK): `release_window` (`Window.takeSurface(null)`/`takeInputQueue(null)` em `EV_CREATE`), `set_content_view`, `dex_load_class` (`DexClassLoader`, vários `.dex` com `:`), `jni_scall_cls`, `jni_take_error`/`jni_error`, `set_read_only`, `intent_extra`, `self_intent`, `restart_process` (reabre a activity num processo novo). Doc: `docs/ANDROID.md` (+ `en/`, `es/`).
- **`icons.ofs`**: 30 ícones novos (auto, pular, histórico, olho/esconder, girar, retrato, paisagem, lixeira, editar, imagem, volume, texto, paleta, ok, armazenamento, motores `windows`/`renpy`/`unity`/`moon`/`unknown`, teclado, apagar, shift, pausa, joystick, espaço, seta, tamanho, abas) e apelidos em `by_name`.

### Fixed — compilador

- **runtime**: `ofs_array_pop` agora devolve o elemento removido (antes devolvia lixo/0 para quem usava o valor).
- **driver (`attach`)**: o caminho de `attach {F:...}` é normalizado (`./`, `../`, barras duplicadas) antes de checar duplicidade, então o mesmo arquivo não é incluído duas vezes.
- **net**: `net.resolve` lê `struct addrinfo` do bionic (Android), onde `ai_addr`/`ai_canonname` estão em ordem diferente da glibc.
- **llvmgen**: um `extern vein` definido no próprio módulo não é mais redeclarado (`invalid redefinition of 'ofs_alloc'` no `magma.ofs` e no `test_magma_full`).

### Added — stack gráfica 100% OFS (base do Catalyst Browser)

- **`raster.ofs`**: rasterizador vetorial com antialiasing por acumulação de área (estilo font-rs): `move_to`/`line_to`/`quad_to`/`cubic_to`/`close`, retângulos arredondados, círculos, cápsulas (traços), arcos, anéis, e composição no Canvas com alfa.
- **`ttf.ofs`**: leitor TrueType (cmap 4/12, glifos simples e compostos, hhea/hmtx) com UTF-8, cache de glifos, posicionamento subpixel (1/4 px), escurecimento de hastes, `width`, `ellipsize`, `draw_centered`. Texto real (Inter, Noto, DejaVu) com acentos.
- **`x11win.ofs`**: janela X11 nativa em OFS puro (Xlib via `extern`): apresentação do Canvas por `XPutImage`, eventos de teclado com texto UTF-8 e teclas mortas pt-BR (´ ` ^ ~ ¨ ç), mouse, roda, resize, fechar (`WM_DELETE_WINDOW`), cursores (seta/mão/texto), ícone da janela (`_NET_WM_ICON`), `now_ms`/`sleep_ms`.
- **`icons.ofs`**: 20 ícones vetoriais de interface (voltar, avançar, recarregar, casa, +, fechar, cadeado, info, estrela, menu, busca, globo, download, histórico, engrenagem, perfil, parar, arquivo, favorito, escudo) com cache por tamanho.
- **`hashmap.ofs`**: tabela hash inteiro→inteiro com endereçamento aberto e crescimento automático.

### Fixed — compilador (`ofscc`), verificado por bootstrap com ponto fixo (vN.ll == vN+1.ll)

- **parser**: `*`, `-` ou `&` no início de uma nova linha agora iniciam um novo comando. Antes, `v = *p` seguido de `*p = 0` virava `v = *p * p = 0`.
- **parser**: operadores unários aplicam-se à expressão pós-fixa completa — `!ns.fn(x)` é `!(ns.fn(x))`, não `(!ns).fn(x)` (gerava chamada para `@fn` inexistente).
- **llvmgen**: globais de `namespace` acessíveis de fora (`raster._acc`, `cfg.contador = ...`, `cfg.contador++`); antes geravam GEP sobre `%unknown`.
- **llvmgen**: globais com literal negativo (`forge x: stone = -1`) eram emitidos como `0`. Afetava o próprio compilador (`interpreter___ret_ref`).
- **typeck**: `ofs_array_get`/`ofs_array_pop`/`ofs_array_len`/`ofs_array_push`/`ofs_array_set` sem `extern` declarado agora têm tipo inferido de `Array<T>` (antes `unknown` → ponteiro → IR inválido).
- **canvas**: `_sin_deg`/`_cos_deg` com redução de ângulo a [-π/2, π/2] e série de 6 termos (erro de ~20% perto de 180° deformava arcos).
- **dist**: `ofs/dist/ofscc`, `ofs/dist/ofscc.ll` e `ofs/dist/stdlib/` estavam dessincronizados do fonte — o driver usava um `canvas.ofs` antigo sem `draw_shadow` (segfault no Catalyst) e um `ofscc` sem `stacksave` nos loops. Regenerados a partir do fonte.
- **runtime (`ofs_str_char_at`)**: chamava `strlen` a cada caractere (O(n²) em laços sobre strings). Agora guarda em cache (ponteiro, comprimento) da última string, validado pelo terminador NUL. Laço sobre 164 KB: 0,203 s → 0,014 s. Aplicado à mão em `ofs/dist/magma.ll` e `ofs/dist/magma_windows.ll` (que têm ajustes manuais) e espelhado em `stdlib/runtime/facet.ofs`; `magma.o` regenerado.
- **typeck**: builtins de string sem `extern` (`ofs_str_len`, `ofs_str_char_at`, `ofs_str_eq`, `ofs_str_contains`, `ofs_str_concat`, `ofs_str_substr`, `ofs_stone_to_obsidian`) têm tipo inferido (antes `unknown` → IR inválido em `forge n = ofs_str_len(s)`).
- **typeck/interpreter (`ofs run`)**: globais de `namespace` são pré-declarados com o nome mangled e `ns.global` é reescrito para `ns__global` antes dos backends — some o falso aviso "identificador desconhecido" e `ofs run` passa a ler/escrever esses globais. O interpretador também passa a inicializar globais declarados dentro de `namespace` (antes ficavam nulos até para o próprio namespace).
- **bootstrap**: `ofs/bootstrap/*.ofs` sincronizado com `ofs/ofscc/` (era um retrato antigo sem `interpreter`, `events`, `reporter`); scripts apontavam para `ofs/ofs/bootstrap/ofscc.ofs` (inexistente — `bootstrap-minimal.sh` falhava na Fase 2) e agora usam `ofs/ofscc/ofscc.ofs`. `OFS_BOOTSTRAP_SELFHOST=1 bootstrap-minimal.sh --validate` passa com determinismo validado.

- **driver (`attach {modulo}`)**: a forma curta documentada (`attach {canvas}`, `attach {terminal-colors}`) nunca carregava o módulo — só `attach {F:arquivo.ofs}` funcionava. Agora resolve `modulo.ofs` na stdlib (hífen → underscore), com prioridade sobre arquivo local homônimo; vale para `build` e `run`.
- **lexer (literais hex)**: `0xFF`, `0x1A1A2E` iam crus para o LLVM IR (lidos como ponto flutuante → "floating point constant invalid") e não funcionavam no interpretador. Agora viram decimal no lexer; `0xFFFFFFFFFFFFFFFF == -1` como em C.
- **llvmgen (inteiros pequenos)**: `u8`/`u16`/`u32`/`i8`/`i16`/`i32` são estendidos para `i64` (zext/sext) em operações binárias e no `echo` — `(r as u32 << 16) | ...` gerava IR inválido e `echo(pixel: u32)` chamava `echo_obsidian` com inteiro.
- **examples**: 28 de 33 compilam (antes 21). Restam 3 exclusivos de Windows, `attach_local_lib.ofs` (biblioteca sem `main`) e `hybrid_fault_console.ofs` (`fault_count` sem implementação no llvmgen).
- **VS Code (1.0.10)**: autocompletar, hover e mapa de arquivos com `fs`, `net`, `http`, `oll`, `ui`, `raster`, `ttf`, `x11win`, `icons`, `hashmap`; snippets `x11win-app` e `hashmap`; toolchain empacotado (`bin/linux-x64`: `ofs`, `ofscc`, `magma.o`, stdlib) atualizado.
- **site/docs**: páginas de stdlib (PT/EN/ES) regeneradas pelo `build_site.ofs` com os novos módulos; `docs/en/STANDARD_LIBRARY.md` e `VISUAL_UI.md` (PT/EN) com a stack gráfica 100% OFS.

- **lexer (`\r` e `\t` em strings)**: o caso existia, mas usava os literais `"\r"`/`"\t"`, que dependem do compilador anterior; como a semente não os convertia, o erro se perpetuava no bootstrap e `"\r\n"` gerava os bytes `\` `r`. Cabeçalhos HTTP de `http.ofs`/`webserver.ofs` saíam inválidos (servidores respondiam 400). Agora os bytes são construídos por código.
- **llvmgen (bool em arrays)**: `ofs_array_set/push(a, i, x || y)` gravava um `i1` num slot `i32` (IR inválido); o valor agora é estendido.
- **llvmgen**: `ofs_str_char_at_known` passa a ser declarada automaticamente como as demais funções de string.
- **driver**: `attach {F:/caminho/absoluto.ofs}` aceita caminho absoluto.
- **net.ofs**: `net.resolve(host)` — DNS via `getaddrinfo` em OFS (`net.is_ipv4`).
- **Catalyst 3.0.0**: reescrito sobre a stack gráfica OFS — interface estilo Chrome, abas com histórico próprio, omnibox editável, favoritos, menu, parser HTML/CSS novo, layout com quebra de linha, flex, links, rolagem, DNS e redirecionamentos HTTP.

- **net.ofs**: `net.recv_all(fd)` (recepção binária segura, `net.last_len`), `net.set_timeout` e `net.timeout_s` (15 s padrão em `tcp_connect`; antes um servidor mudo travava o programa para sempre).
- **Catalyst 3.1.0**: histórico e downloads persistentes em `~/.catalyst` / `~/Downloads` (downloads binários reais), sistema de plugins `.cai` (estilos, bloqueio, redirecionamento, busca, páginas `ofs://ext/`, botões, atalhos, página inicial), página `ofs://plugins`, Google como busca padrão, ícone de cristal (`assets/catalyst.svg`).

- **llvmgen (`&&` / `||`)**: o código nativo avaliava sempre os dois lados (sem curto-circuito); `i >= 0 && arr[i]` lia fora do array. Agora o lado direito só roda quando necessário, como no interpretador e em C.
- **parser (sombreamento)**: parâmetros e variáveis locais de funções num `namespace` eram renomeados para o global homônimo (`forge bg = ...` lia e sobrescrevia `ns__bg`). Agora locais sombreiam globais.
- **stdlib**: `icons.by_name(nome)`.
- **Catalyst 3.2.0**: configurações redesenhadas (barra lateral estilo Chrome, busca estilo Firefox, cartões, interruptores, opções e botões com ícones), tema claro/escuro, tamanho de fonte, barra de favoritos opcional, idiomas pt/en/es, escolha do mecanismo de busca, nova aba com caixa de pesquisa, formulários (`<input>`, `<button>`, envio GET) e a tag `<icon name>` nas páginas.

- **Catalyst 3.3.0**: Catalyst Shield (bloqueio nativo de anúncios/rastreadores: rede, cosmético estilo EasyList, heurística, filtros do usuário, liberação por site, contador e painel na omnibox), Visão Essência (páginas dependentes de JavaScript exibidas a partir de JSON-LD, `__NEXT_DATA__`, OpenGraph e links, sem executar scripts), 6 cores de destaque, tema dinâmico por `theme-color`, famílias de fonte (Inter, Noto Sans, DejaVu, Serif), favoritos persistentes com gerenciador `ofs://bookmarks`.

- **stdlib `tls.ofs`**: HTTPS via OpenSSL do sistema (SNI, verificação de certificado e de nome); linkar com `OFS_LINK_LIBS="-lssl -lcrypto"`.
- **stdlib `png.ofs`**: decodificador PNG 100% OFS com inflate/zlib próprio (cinza, RGB, paleta 1–8 bits, alfa); decodificação idêntica pixel a pixel nos testes; `draw`, `draw_cover`, `draw_region`.
- **driver `ofs`**: variável `OFS_LINK_LIBS` para bibliotecas extras do sistema no link.
- **stdlib `x11win.now_ms`**: funciona sem janela aberta (antes travava no modo headless).
- **CI**: runner `macos-14` foi aposentado pelo GitHub ("configuration 'macos-14-arm64-us-default' is not supported"); `build.yml` e `release.yml` passam a usar `macos-latest`.
- **Catalyst 3.4.0**: HTTPS, HTTP/1.1 com chunked, CSS externo com cache, `@media`/`@supports`, cascata indexada por seletor (G1: CSS de 40 ms), parser HTML 20× mais rápido em páginas com JS inline, perfilador `CATALYST_PROFILE=1`, quebra de palavras longas, `flex-shrink`, usuários com exportar/importar (`.catprofile`), cores livres de destaque e fundo, papel de parede PNG/gradiente na nova aba.

### Known issues

- Global de tipo `monolith` (`forge cv: Canvas` em escopo de namespace) reserva só um ponteiro; atribuir a struct inteira corrompe memória. Use variável local ou passe por parâmetro.
- `stdlib/runtime/frame.ofs` (janela do `magma.o`) é um stub; use `x11win.ofs` para janelas reais.

### Added

- **Reporter Fases 2–5 (ofscc)**:
  - **Fase 2 — i18n**: catálogo embutido en/pt/es com detecção automática de locale (`OFS_LANG` > `LANG` > `en`) e TOMLs espelho em `ofs/ofscc/locale/` para tradutores e CI de paridade.
  - **Fase 3 — Reporter Fancy**: verbos alinhados em coluna fixa, símbolos ANSI, cores com degradação automática (`NO_COLOR` / `TERM=dumb`).
  - **Fase 4 — animação**: spinner single-thread entre estágios, `\r` + clear line, desligável via `OFS_ANIM=0`.
  - **Fase 5 — Json NDJSON**: eventos extras (`locale`, `msg_key` + `args[]`, `duration_ms_total`) e stub de saída `twin` em NDJSON.
- **CI golden files**: workflow `golden.yml` re-bootstrapa o compilador e compara saídas Plain/Fancy/Json e de locale contra binário recém-gerado.

### Added

- **canvas (camada de pintura)**: mistura alfa ARGB (`argb`, `blend_pixel`), antialiasing por SDF de 1px (`fill_rounded_rect_aa`, `stroke_rounded_rect_aa`, `fill_circle_aa`, `stroke_arc`), pilha de clip retangular (`push_clip`/`pop_clip`/`clear_clips`), sombra suave em camadas (`draw_shadow`) e gradiente vertical (`fill_gradient_v`). `set_pixel` respeita clip e normaliza cores ARGB.
- **OLL**: constantes nomeadas `OLL_WINDOW…OLL_PROGRESS`, `tabs` agora usa layout horizontal, `stack`/`container` sobrepõem os filhos, campo `progress: stone` próprio no `OllNode` (`value:`/`progress:`), barras de progresso desenhadas de fato (trilha + preenchimento AA), render com bordas arredondadas antialiased e sombra suave por `elevation`, e cores `#RRGGBBAA` com canal alfa.
- **CI**: golden de render (`tests/render/run.sh`) compara o PPM da cena (`attach {canvas}` + `attach {oll}`) bit a bit no job Linux.

### Fixed

- **llvmgen**: constante inteira `0` em global com tipo ponteiro (ex.: `forge x: Array<T> = []`) emitia `global ptr 0`, rejeitado pelo llc — agora emite `global ptr null`.
- **llvmgen (`emit_global_init`)**: globais com tipo genérico `Array<T>` nunca passavam no teste `ofs_str_eq(gt, "Array")`, então o buffer interno não era alocado no `__ofs_init_globals` e todo `ofs_array_push` virava no-op (parser/lexer do compilador auto-hospedado perdiam estado). Agora o prefixo `Array<` é reconhecido e os elementos literais são inicializados. Corrige o bootstrap v3→v4 e a compilação nativa de `installer_wizard_linux.ofs` no release.
- **llvmgen (loops)**: cada iteração de `while`/`cycle` agora envolve o corpo com `llvm.stacksave`/`llvm.stackrestore` — allocas de variáveis locais ao corpo não acumulam pilha em builds não-otimizados (`llc -O0`). Corrige stack-overflow renderizando retângulos grandes no Canvas (ex.: Catalyst Browser) e em qualquer loop com milhares de iterações.
- **cli (`ofs run --native`)**: saída do reporter de compilação redirecionada para stderr, deixando o stdout do programa limpo (paridade com `ofs run` interpretado).
- **tests/golden**: caso Fancy ANSI neutraliza explicitamente `NO_COLOR`/`OFS_LANG`, que vazavam das variáveis do step de CI; o bootstrap do seed volta a gerar IR válido (82 buffers de arrays globais restaurados).
- **bootstrap**: `ofs_str_substr` sem terminação NUL, contagem de limites de `attach {F:...}` e tamanhos de allocação de `monolith` corrigidos.
- **portabilidade**: `snprintf` e `ofs_stone_to_obsidian` reescritos em OFS puro; `crystal` formata como `%.15g` com sufixo `.0` (paridade).
- **CI**: `clang -no-pie` (Linux) e `-Wl,-no_pie` (macOS) ao linkar `ofscc.ll` gerado por llc non-PIE; `-Wl,/force:multiple` (Windows).

### Changed

- **Licença v1.0.151**: adoção de **Apache-2.0** (`LICENSE`, `docs/legal/LICENSE`) e **Política de Marca** (`TRADEMARK.md`).
- **`docs/pt/STATUS_REPORT.md`** movido para `src/archive/STATUS_REPORT.md`; site passa a apontar apenas para o CHANGELOG.

### Self-Hosted Compiler — Marcos 5–6 (Fixed)

- **Marco 5**: Typechecker no longer OOMs on deep AST paths — hot accessor functions extracted to keep stack pressure bounded during diagnostic walks.
- **Marco 6a (mine loops)**: Deferred `anchor` statements were emitted at scope-exit in FIFO order, breaking `defer`-style semantics. The LLVM generator now walks the anchor stack in reverse (LIFO) so that the most recently registered anchor runs first, matching Go/Zig conventions.
- **Marco 6b (assay / strata)**: `strata` variants had no object representation, producing invalid `icmp eq i64` against `ptr` loads and undefined symbols when constructors like `Crack("boom")` were called. Variants are now global `i64` tags registered in `_variant_names`/`_variant_tags` and variant constructors lower to the tag constant.
- **Codegen**: `Array<T>` element loads now infer the right scalar type (`ptr` for `Array<obsidian>`, `double` for `Array<crystal>`, `i1` for `Array<bool>`), avoiding invalid `call i64 @ofs_str_eq(ptr %x)` patterns after reading from a stone-typed slot.
- **Typeck**: `is_variant_name` tightened to strict PascalCase (starts uppercase, contains at least one lowercase, no underscores). This prevents `EXEC_OK`, `NK_CORE_DECL` and other `SCREAMING_SNAKE` constants from being typed as variants (which previously forced a `ptr` alloca and corrupted downstream code).
- **Typeck / Codegen**: `type_node_name` now preserves the full generic spelling (`Array<obsidian>` instead of bare `Array`), enabling downstream consumers (collect_globals, VAR_DECL emission, array indexing) to pick the correct value type.

### Self-Hosted Compiler — Marcos 5–6 (Validated)

- `ofs/tests/interpreter/10_geological_flow.ofs` → prints `1 2 3 4 5 40 30 8 50 7 20 10` (exact LIFO anchor ordering across nested scopes and loops).
- `ofs/tests/interpreter/11_assay_basic.ofs` → prints `20`.
- `ofs/tests/interpreter/12_outcome_lode.ofs` → prints `7` and `2` (strata variants used in `assay`).
- `ofs/tests/interpreter/13_question_propagation.ofs` → prints `0` and `1` (basic `?` propagation semantics).

## [Unreleased — track 1 & low-level]
### Added
- Track 1 language update documented and available in the compiler: small integers (`u8`, `u16`, `u32`, `u64`, `i8`, `i32`), `impl`, namespaces, function values/lambdas, and `window` runtime bridge.
- `window.ofs` standard-library module and `window` package entry for native window/input access.
- `canvas.ofs` standard-library module and `canvas` package entry for pixel-buffer drawing on top of `window`.
- VS Code extension grammar, snippets, hover docs, and README updated to cover Track 1 syntax and the canvas/window workflow.
- New `fault_unreachable()` intrinsic: marks the current code path as unreachable; lowers to `llvm.trap` followed by an LLVM `unreachable` instruction, letting the optimizer eliminate dead branches.
- New `fault_memcpy(dst, src, len)` intrinsic: lowers to `llvm.memcpy` for bulk memory copies inside `fracture`/`abyss`/`bedrock` blocks.
- New `fault_memset(dst, val, len)` intrinsic: lowers to `llvm.memset` for bulk memory initialisation inside low-level blocks.
- Variadic `...` syntax on `extern vein` declarations: `extern vein printf(fmt: obsidian, ...) -> stone` now compiles correctly. The `ELLIPSIS` token (`...`) was added to the lexer and parser.
- Implemented `ofs_webserver_serve_once` and `ofs_webserver_serve_forever` in the runtime (POSIX sockets on Linux/macOS; Winsock on Windows). These were previously declared in the header but had no implementation, causing linker errors in any program that called them.

### Changed
- `match` statement codegen is now fully implemented. Previously only the `default` arm was generated; all `case` arms now compile to a chain of compare-and-branch instructions. String arms use `ofs_str_eq`, numeric arms use integer/float comparison.
- Inline assembly (`asm "..."`) is now correctly lowered to LLVM `InlineAsm`. Previously the `gen_inline_asm` handler existed but was never called from `gen_expr`, so all `asm` statements were silently discarded. Input operands are now forwarded with default register constraints.
- `fault_step` symbol type in the built-in symbol table corrected from `stone` to `*stone` (shard of stone), matching what `check_call` already returned at call sites.
- `register_builtin_symbols` rewritten for clarity: each intrinsic now has its exact return type recorded in a table rather than a catch-all `stone` fallback.

### Fixed
- Windows updater now resolves the latest release tag more robustly by following redirects and falling back to the GitHub API when needed.
- **Critical**: String equality (`==`) and inequality (`!=`) on `obsidian` values now use `ofs_str_eq` (content comparison). Previously they compared raw pointer addresses, so two string literals with equal content could compare as unequal.
- **Critical**: `asm "..."` expressions were never lowered to IR (the dispatch case was missing in `gen_expr`). Fixed by adding the `InlineAsmExpr` branch to `gen_expr` and to `check_expr` in the semantic analyser.
- Semantic check for `void` function returning a value now raises a proper error (`void function cannot return a value`) instead of silently producing invalid LLVM IR that fails verifier checks.
- `gen_return` now has a codegen-level safety guard: if a void function somehow passes semantic analysis with a return value expression, codegen evaluates the expression for side effects and emits `ret void`, preventing an LLVM verifier crash.
- `examples/baremetal_minimal.ofs` corrected: `vein main()` (void, no core entry) with `return 42` was replaced with a proper `core main()` + helper function structure.
- `gen_inline_asm` previously ignored `inputs` entirely (empty constraint string) and used only the first output name as the constraint. Now builds the constraint string from outputs, per-input `"r"` constraints, and clobber sections.

### Added
- New `ofs asm <file.ofs>` command to emit target-native assembly from the current LLVM backend.
- Low-level roadmap document focused on OFS-native coexistence between high-level and low-level code in the same program.
- `bedrock` standard-library module with OFS-first heap cell helpers for manual state handling.
- VS Code extension command to emit native assembly from the active OFS file.
- Expanded language direction docs covering OFS-native low-level goals and foreign interop roadmap.
- New `bedrock { ... }` typed low-level block in the language.
- New `rift vein` declaration form for OFS-native foreign interop boundaries.
- New `fault_*` intrinsics for machine-like low-level operations with OFS-native naming.
- New `fault_step` intrinsic for typed pointer stepping.
- New `bedrock_region_*` helpers for explicit low-level storage regions.
- New `rift.ofs` module for OFS-native wrappers around foreign runtime boundaries.
- New `fault_cut` and `fault_patch` intrinsics for field extraction and patching.
- New `bedrock_view_*` helpers for OFS-native low-level views over regions.
- New `fault_fence` and `fault_trap` intrinsics for machine-level barrier/trap hooks.
- New `fault_prefetch` intrinsic and `bedrock_prefetch` helper for cache-aware pointer staging in low-level OFS code.
- New lane helpers such as `bedrock_lane8_get` and `bedrock_lane8_set`.
- New endian-aware lane helpers such as `bedrock_lane16_le_get`, `bedrock_lane16_be_get`, `bedrock_lane32_le_set`, and `bedrock_lane32_be_set`.
- New `bedrock_packet.ofs` module for packet/header views over low-level regions.
- New packet helpers for endian-aware header windows such as `bedrock_packet_opcode_be16` and `bedrock_packet_tail_le16`.
- New `bind`/`abi` metadata on `rift vein` and `extern vein` declarations for more explicit interop boundaries.
- New `layout native|packed|c` support on `monolith` declarations.

## [1.2.0] - 2026-04-13 — C++ Compiler Removed 

### Major Milestone: Pure OFS Implementation

**C++ compiler source code removed from repository. Language is now 100% self-hosted.**

All language implementation and compilation pipeline now solely uses OFS.

### Removed

- `ofs/src/` - Legacy C++ compiler source (main.cpp, lexer, parser, codegen, semantic, runtime, ast)
- `ofs/CMakeLists.txt` - CMake build configuration
- All C++ dependencies from codebase

### Repository Structure (Simplified)

```
ofs/
  ├── ofscc/           # OFS compiler written in OFS (~4500 LOC)
  ├── stdlib/          # OFS standard library
  ├── examples/        # Example programs
  ├── tests/           # Test suite
  └── docs/            # Documentation
```

### Notes

- Compiler distributed via releases as `ofs` binary
- Users install via: `sudo mv ofs/bin/ofscc /usr/local/bin/ofs`
- Language bootstrapping: `bash ofs/bootstrap/scripts/bootstrap-minimal.sh`
- No C++ runtime or compiler needed to build OFS

---


## [1.1.1] - 2024-12-XX — Automated Native-Only Releases

### Major Milestone: CI/CD Automation & Zero C++ Dependencies

**Full automation pipeline achieved! Releases now generated in ~30 seconds using only the native OFS compiler.**

### Added - CI/CD & Release Automation

- **GitHub Actions Workflow** (`.github/workflows/release-native.yml`):
  - Multi-platform jobs: Linux, macOS, Windows
  - Automatic triggers: Git tags (`v*`) + manual `workflow_dispatch`
  - Per-platform release scripts (native-only, no C++ build step)
  - VS Code extension auto-build integration
  - Checksums (SHA256) + version metadata auto-generated

- **Bootstrap Scripts** (`ofs/bootstrap/scripts/`):
  - `bootstrap-minimal.sh`: Native-only compiler recompile (~2 sec, deterministic)
  - `release-native-only.sh`: Linux/macOS release src/packaging (~5 sec)
  - `release-native-only-windows.ps1`: Windows release src/packaging (~5 sec)
  - Built-in determinism validation (compiles twice, verifies identical binaries)

- **Documentation Updates**:
  - `docs/GETTING_STARTED.md` — Complete rewrite for native compiler
  - `docs/BOOTSTRAP.md` — Updated with bootstrap-minimal.sh workflow
  - `ofs/docs/README_BAREMETAL.md` — Native compiler instructions
  - `ofs/docs/BAREMETAL_PROFILE.md` — Native compiler instructions
  - `README.md` — Updated all commands from `ofs build` to native approach

- **VS Code Extension** (v1.0.8):
  - Updated display name: "Obsidian Fault Script (Native)"
  - Highlights: "(C++-free)" and "self-hosted native compiler"
  - Auto-detection of native compiler in `ofs/dist/ofscc`

### Performance Improvements

- **Release generation**: 5-15 minutes → ~30 seconds (300x faster!)
- **Bootstrap verification**: Full determinism check in 2-5 seconds
- **Total CI/CD time**: 15+ minutes → ~30 seconds

### Removed

- C++ compilation step from release pipeline
- LLVM/Clang dependencies from CI/CD
- CMake build requirements for releases
- Old `ofs` command references (replaced with native approach)

### Technical Details

**New Release Pipeline:**
```
Git Tag → GitHub Actions → (Linux/macOS/Windows parallel bootstrap-minimal) 
→ (platform-specific release scripts) → Consolidated artifacts + GitHub Release
```

**Zero Dependencies:**
- No C++ compiler needed for releases
- No CMake required
- Bash + native GCC/Clang only

---


## [1.1.0] - 2026-04-12 — Self-Hosted Compiler

### Major Milestone: OFS Compiler is Self-Hosting

**The OFS compiler is now written entirely in OFS!**

After implementing a complete compiler pipeline in pure OFS (lexer, parser, type checker, code generator), the language has reached a critical milestone: it can compile itself. This is the same achievement as Go (2015), Rust (2011), and C (1972).

### Added - Self-Hosting Compiler

- **`ofs/ofscc/`** — Complete OFS compiler written in OFS (~4,500 LOC, 13 files)
  - `chars.ofs` — ASCII constants and character classification
  - `tokens.ofs` — Token types (80+ kinds) + keyword lookup
  - `lexer.ofs` — Lexical analyzer (tokenizer)
  - `nodes.ofs` — AST node kinds (60+)
  - `ast.ofs` — AST pool + node management
  - `symbols.ofs` — Symbol table definitions
  - `parser.ofs` — Recursive descent + Pratt parser
  - `typeck.ofs` — Type checker with scope stack
  - `codegen.ofs` — C code generator
  - `fileio.ofs` — File I/O + FFI declarations
  - `ofscc.ofs` — Main driver (6-phase pipeline)
  - Test files: `test_lexer_units.ofs`, `test_lexer_parser.ofs`
  - Bootstrap script: `test_bootstrap.sh`

- **Bootstrap verification**:
  - `ofs build ofs/ofscc/ofscc.ofs -o ofscc_v1` (C++ → OFS)
  - `./ofscc_v1 ofs/ofscc/ofscc.ofs -o ofscc_v2` (OFS → OFS pass 1)
  - `./ofscc_v2 ofs/ofscc/ofscc.ofs -o ofscc_v3` (OFS → OFS pass 2)
  - When `ofscc_v2 === ofscc_v3`, C++ compiler becomes optional

- **Documentation**:
  - `docs/BOOTSTRAP.md` — Complete bootstrap guide (step-by-step)
  - `docs/COMPILER_ARCHITECTURE.md` — Compiler internals (phases, data structures)
  - `ofs/ofscc/README.md` — Compiler project README + testing instructions

### Changed

- **C++ compiler now marked as legacy**
  - `src/` remains for build compatibility
  - Future versions may remove C++ entirely
  - All new development uses OFS compiler

- **Build system**:
  - `ofs build` still available for bootstrap
  - New path: `ofscc input.ofs -o output`
  - Both compilers work in parallel (for now)

### Added - Phase B implementation increment (2026-04-12)

- **Parser/Type Checker/Codegen upgrades in `ofs/ofscc/`**
  - `tremor/catch` parsing and lowered codegen block
  - `monolith` parsing + C struct emission
  - `impl` parsing + method flattening (`Type__method`)
  - `namespace` parsing + nested member emission
  - `strata` parsing + enum-tag codegen base

- **CLI bridge for `ofscc`**
  - `OFSCC_INPUT`, `OFSCC_OUTPUT`, `OFSCC_C_OUT`, `OFSCC_MODE`, `OFSCC_OPT`
  - support for build optimization levels `-O0`, `-O2`, `-O3`

- **CI/CD expansion**
  - `build.yml` now includes macOS build
  - fixed Linux determinism stat command (`stat -c%s`)
  - Windows LLVM install no longer pins exact Chocolatey version

- **Self-hosted release workflow (Phase D/E prep)**
  - `.github/workflows/selfhosted-release.yml`
  - downloads bootstrap compiler from Releases
  - compiles `ofscc.ofs` and generates installer assets with OFS tooling

- **VS Code extension update**
  - autodetects self-hosted binaries (`ofscc`, `ofscc_v2`) in workspace
  - run command adapts when compiler is `ofscc`

### Technical Details

**Compiler Architecture**:
- **Phase 1: Lexer** — Tokenizes source (80 token kinds)
- **Phase 2: Parser** — Recursive descent + Pratt precedence
- **Phase 3: Type Checker** — Scope stack + type inference
- **Phase 4: Codegen** — Emits valid C code
- **Phase 5: C Compilation** — Calls `gcc -O2`
- **Phase 6: Output** — Native executable

**Features Supported**:
- All primitive types (stone, crystal, obsidian, bool, u8-u64, i32)
- Functions, variables, constants
- Arrays, monoliths (structs)
- Control flow (if/else, while, cycle, match, break, continue, return)
- Operators (arithmetic, logical, bitwise, comparison)
- String literals with escape sequences
- Comments (//, /* */)
-  Type inference (basic)
-  Error messages (simple)

**Limitations (Addressed in v1.2)**:
- Command-line argument parsing
- impl/namespace/strata (parsing only)
- Full error recovery
- Diagnostics beyond stderr

### Performance

| Phase | Time |
|---|---|
| Lexer | <1ms |
| Parser | ~10ms |
| Type Check | ~5ms |
| Codegen | ~10ms |
| GCC | ~100ms |
| **Total** | ~150ms |

### Files Changed

```
Created:
  docs/BOOTSTRAP.md
  docs/COMPILER_ARCHITECTURE.md
  ofs/ofscc/*.ofs (13 files)
  ofs/ofscc/*.sh (test scripts)
  ofs/ofscc/README.md

Modified:
  (Updated below in "Changed" section)
```

### Next Steps (v1.2)

- [ ] Implement command-line argument parsing
- [ ] Add full impl/namespace/strata support
- [ ] Improve error messages with line/column
- [ ] Optimize generated C code
- [ ] Port to Linux/macOS/Windows
- [ ] VS Code extension using ofscc

### Breaking Changes

None. The C++ compiler is still the default. Opt-in to OFS compiler:

```bash
# Old (still works)
ofs build input.ofs -o output

# New (opt-in)
./ofscc input.ofs -o output
```

---



## [1.0.151] - 2026-10-01 — Apache-2.0 License & Trademark Policy

### Changed

- License updated to **Apache-2.0** (Apache License, Version 2.0), aligning OFS with major systems languages (Rust/LLVM), providing explicit patent protection and attribution requirements.
- **Trademark Policy** added (`TRADEMARK.md`) protecting official project names ("Obsidian Fault Script", "OFS") and branding against unauthorized commercial re-use and confusing forks.

## [1.0.148] - 2026-09-30 — Hybrid Interpreter Backend & Differential Testing

### Added

- **AST Interpreter Backend (`interpreter.ofs`)**: OFS is now a true hybrid language. Running `ofs <file.ofs>` or `ofs run <file.ofs>` directly evaluates the typed AST without invoking LLVM, Clang, or linker phases, providing sub-millisecond execution for scripting and prototyping.
- **Value Monolith & Dynamic Heap**: Full runtime support for primitives (`stone`, `crystal`, `obsidian`, `bool`), handles for `Array<T>`, heap-allocated `monolith` structs with value-copy semantics, and pointers with the `Location` table.
- **Lexical Call Frame Scoping**: Frame base tracking (`_current_frame_base` and `_globals_count`) ensuring that local variables in caller functions never leak or shadow global declarations in callees.
- **Method Dispatch via `impl`**: Support for methods declared in `impl Type { vein method(self: Type) ... }` called directly on instances (`instance.method()`).
- **Differential Test Suite**: Automated differential runner `ofs/tests/interpreter/test_differential.sh` with 9 end-to-end test suites asserting 1-to-1 output equivalence between interpreted mode and native LLVM builds.
- **CI Integration**: Added differential test validation to GitHub Actions workflows on Linux and macOS.

### Changed

- `ofs run <file.ofs>` defaults to instant interpreted execution; `ofs run --native <file.ofs>` is available for temporary compilation + execution.
- Windows PowerShell runner `src/packaging/windows/ofs.ps1` updated with interpreted mode and `--native` flag for complete parity across Linux, macOS, and Windows.

---


## [1.0.101] - 2026-05-31 — Native Installers Release Hardening

### Fixed

- Fixed LLVM 14 opaque pointer handling in CI and release builds.
- Fixed Bash 3/macOS wrapper behavior when LLVM IR flags are empty.
- Fixed example checks so relative `attach {F:...}` paths run from each example directory.
- Fixed Windows release src/packaging collision by using a dedicated `windows-dist` staging directory.

### Release

- Release automation now produces native installers for Debian/Ubuntu, Fedora, Arch, macOS, and Windows.
- VS Code extension src/packaging is included in the release assets.
- Windows CI validates the native compiler binary and installer package; Linux/macOS continue to run full example and stdlib smoke checks.

---


## [1.0.3] - 2026-04-01
### Added
- New keywords: `match`, `case`, `default`, `const`, `strata`, `tremor`, `catch`, `throw`.
- New operators: `<<`, `>>`, `%=`.
- Bitwise operator support in binary expressions (`&`, `|`, `^`, `<<`, `>>`).
- New AST nodes for:
  - Pattern matching (`MatchStmt`, `MatchArm`)
  - Immutable variables (`ConstStmt`)
  - Enumerations (`StrataDecl`)
  - Error handling (`TremorStmt`, `ThrowStmt`)

### Changed
- Parser updated to recognise `strata`, `const`, `match/case/default`, `tremor/catch`, `throw`.
- Semantic analyser with stubs and basic validation for new constructs.
- Codegen stubs for new constructs and bitwise/shift expression support.
- Lexer and parser tests extended for new tokens and syntax.
- Release CI/CD improved for Linux/Windows/macOS with better LLVM detection.

### Fixed
- Windows workflow: no longer fails when a newer LLVM is already installed and a downgrade is attempted.
- Linux workflow: `LLVM_DIR` now set explicitly to avoid picking up an older LLVM on the runner.
- CMake: LLVM backend chosen by architecture (x86_64 vs arm64).
- Codegen: compatibility with LLVM 15/17+ for `Reloc::Model` (`llvm::Optional` vs `std::optional`).


## [1.0.2] - 2026-04-01
### Added
- Automated release pipeline with binaries for Linux, Windows, and macOS.


## [1.0.1] - 2026-04-01
### Changed
- README simplified for quick installation and usage.


## [1.0.0] - 2026-04-01
### Added
- First stable release of the OFS language (lexer, parser, semantic analysis, LLVM codegen, runtime, and examples).

