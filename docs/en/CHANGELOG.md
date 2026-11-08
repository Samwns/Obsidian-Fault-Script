# Changelog

All notable changes to this project are recorded here.

> **Versioning scheme.** The official versions are the GitHub tags `vMAJOR.MINOR.PATCH` (`v1.0.0` ... `v1.0.178`), with the patch number incremented automatically by the `release.yml` workflow; this is the number `ofs version` shows in an installed package. The labels `1.1.0`, `1.1.1` and `1.2.0` below are historical milestones from April 2026 that **were never tags**; their entries were kept by subject, not by version. The text "Unreleased" covers everything that came after `v1.0.151`.

## [Unreleased]

### Added — `ofs doc` (documentation generated from code) and the site Reference

- **EN** — new **`ofs doc`** command (`ofs/tools/ofsdoc.ofs`, written in OFS): HTML documentation generated from the source (modules, `vein`/`rift vein`/`extern vein`, `monolith`/`impl`, calls/used by, see also, link to the line on GitHub, `search.json` search) and a coverage report (`ofs doc --check` exits 1 if a `.md` cites a function that does not exist). The site wiki was replaced by the **Reference** (`docs/ref/`, obsidian theme): Guide + generated API + Coverage in one navigation; `docs/wiki/` now just redirects. See `docs/en/TOOLS.md`.

### Added / Adicionado — Termux

- **EN** — **Termux**: OFS now runs on the phone (`ofs run`, `ofs build`, tools); the wrapper detects Termux, `ofs-termux-aarch64.tar.gz` package (`src/packaging/termux/build_termux.sh`, attached to the release by CI) and `install_termux.sh`. See `docs/en/GETTING_STARTED.md`.

### Added — hex keyboard (OAL)

- **EN** — OAL: hex layer of the `keyboard`, selected with `oal.kb_layer = 2` (keys 0-9, A-F, backspace, `#` and ↵). Used by the M.O.O.N color picker hex field.

### Added — on-screen gamepad with button styles (OAL)

- OAL: `diamond -> action(item) corner= look=` widget (4 action buttons in a diamond; `PAD_KEYS` keycaps, `PAD_XBOX`, `PAD_PLAY` with vector shapes ✕ ○ □ △ and no logos, `PAD_NINTENDO`), `on key` event (control/keyboard keys forwarded with a filter and without repeats), `oal.pad_connected` (physical controller detected from the event source). `icons.ofs`: `BTN_CROSS/CIRCLE/SQUARE/TRIANGLE`, `KEY_ENTER`. `android.ofs`: `key_source`, `key_handled` (a consumed key does not reach the system); analog sticks no longer act as touch.

### Added — CFF/TTC fonts with fallback, AVI video (M.O.O.N)

- `ttf.ofs`: **CFF** outlines (Type 2 charstrings, local/global subrs, CID fonts with FDArray/FDSelect), **TTC** collections (`load_face`, `load_face_named`, `face_count`), **per-glyph fallback** (`set_fallback`, `set_fallback_file` with lazy loading, `has_glyph`), glyph cache limit (`pool_limit`, `clear_cache`). TrueType text renders bit-identically to before. New `avi.ofs`: AVI reader + Microsoft Video 1 (16-bit) decoder, bit-identical to ffmpeg. `wav.from_pcm`, `audio.position`. Compiler workaround: `ofs_array_get(...) as crystal` loads the element as `double` without converting (go through a function with a `stone` parameter).

### Added — `.moonpack` plugins, WebView, zip, color picker (OAL)

- `zip.ofs`: chunked extraction (`xbegin`/`xstep`/`xstep_ms`/`xpct`/`xabort`) on top of the new resumable inflate in `png.ofs` (`inflate_begin/step/take/abort`); CRC and sha256 are computed while extracting. Installing Godot's 76 MB `.so` in M.O.O.N no longer ANRs on touches during extraction.

- `zip.ofs` (read/extract zip with the `png.ofs` inflate, create stored zip, CRC-32), `sha256.ofs` (SHA-256 and CRC-32), `png.inflate_raw`/`out_ptr`/`out_len`; `android.ofs`: generic JNI (`jni_call`, `jni_new`, `jni_scall`, `jni_static_int_field`, global refs, byte/object arrays); `android_http.ofs` (HTTPS through `HttpURLConnection` via JNI, stepwise, also `file://`; OpenSSL cannot be linked in an APK); `android_webview.ofs` (full-screen WebView in a `Dialog` via JNI, no Java of our own: HTML with the shim injected by `loadDataWithBaseURL`, return channel through `document.title`); `icons.ofs`: `RPGM`, `TYRANO`. OAL: **`colorpicker value=expr -> action(item)`** widget (saturation/value square, hue bar, preview and hex), `oal.set_palette_full` (button, pressed button, border, badge; text on accent/button by contrast), `oal.hsv_to_rgb`/`contrast_ink`; the bar `back` button now sits on the **left**, before the title (`oll.move_child_first`). Compiler caveat: `core` is a reserved word (the function body silently disappears).

### Added — Android (aarch64) and OAL v0

- **Android target**: `OFSCC_TARGET=android` emits `aarch64-unknown-linux-android26`; the `magma` runtime compiles for aarch64 without changes (it only uses libc/bionic). `net.resolve` now reads the bionic `struct addrinfo` (the `ai_addr`/`ai_canonname` order is reversed compared to glibc). Doc: `docs/ANDROID.md` (+ `en/`, `es/`).
- **`ofs build --target android app.ofs|app.oal`**: produces the APK without Gradle/Java (NDK `clang`, `-Wl,-init,__ofs_init_globals`, `aapt2`, `zipalign -P 16`, `apksigner`), with `--install`, `--serial` and `--manifest`. If the installed `ofscc` has no `OFSCC_TARGET`, it builds one from source (`~/.cache/ofs/android/`). The wrapper running from the repo uses the repo's stdlib.
- **`fault_addr(vein)` and `fault_call(endereco, args...)`**: address of a vein as `stone` (`ptrtoint`) and indirect call with C ABI (`inttoptr` + `call`). Enables C callbacks (`qsort`, NativeActivity, ALooper, AChoreographer) and function tables (JNI). Test: `ofs/tests/runtime/test_fault_addr_qsort.ofs`. Self-host fixed point kept (IR gen1 == gen2).
- **stdlib `android.ofs`**: NativeActivity backend **100% OFS, no `.c` at all** (`android_native_app_glue` was removed): exports `ANativeActivity_onCreate`, fills `activity->callbacks` with `fault_addr`, input through `AInputQueue_attachLooper`, frames through `AChoreographer_postFrameCallback`, all on the main thread; lifecycle, touch, back, rotation, system bar areas, RGBA `ANativeWindow`, `set_orientation` via JNI, app paths, log.
- **OAL v0 (Obsidian Android Language, `.oal`)**: `ofs/tools/oalc.ofs` generates `AndroidManifest.xml` + `<name>_oal.ofs`; `app`, `use`/`attach`, `style` (`fill`, `ink`, `curve`, `opacity`, `pressed`), `screen` with `bar`, `button`, `button.icon`, `title`, `text`, `list`/`grid` with `from`, `card`, `row`, `portrait`/`landscape`, `fab`, `dpad`; actions `-> funcao(item)`, `@tela`, `@back`, `@quit`, `@portrait`/`@landscape`/`@auto`; `on back/pause/resume/rotate/low_memory`. Runtime `oal_runtime.ofs` (layout core from `oll.ofs`, `canvas`/`raster`/`ttf`/`icons`) + `oal_android.ofs`; `oal_x11.ofs` is experimental. Doc: `docs/OAL_REFERENCE.md` (+ `en/`, `es/`); test: `ofs/tests/oal/run.sh`.
- **`icons.ofs`**: icons `PLUGIN`, `SAVE`, `FOLDER`, `PLAY`, `HAMBURGER`, `GAMEPAD` and aliases in `by_name` (`plugins`, `import`, `load`, `config`...).
- **stdlib**: new `wav.ofs` (WAV PCM 8/16-bit and Microsoft ADPCM decoder) and `audio_android.ofs` (AAudio output with the mixer on the audio thread); `ttf.ofs` gained multi-line text (`ttf.wrap`, `ttf.draw_wrapped`); `icons.ofs` gained interface icons (`AUTO`, `SKIP`, `EYE`, `ROTATE`, `TRASH`, `VOLUME`, `KEYBOARD`, `JOYSTICK`, `TABS` ...) and engine icons (`WINDOWS`, `RENPY`, `UNITY`, `MOON`, `UNKNOWN`).

- **OAL v1**: widgets `image`, `canvas [full] draw= touch=` (area drawn by the app, with per-finger touches), `tabs`/`tab`, `slider`, `toggle`, `progress`, `field`, `keyboard` (on-screen keyboard), `if`/`else` blocks; `item`/`card` with `image=`, dynamic `icon=`, `badge=`/`badge_icon=`; `text`/`empty` in multiple lines; labels and icons as expressions; `fab.classe.icon`; corners `bottom_row`/`top_row`/`right_column`/`left_column` or an expression; `on start` and `on frame` events; `app` with `assets "folder"` and a PNG icon copied to `res/mipmap/`. Runtime: **per-finger multitouch**, PNG cache, `oal.toast`, `set_animating`, `reset_to`/`replace`, `set_text_scale`, `set_overlay_scale`, `set_palette` (theme), copy to the window only when something changes. Doc: `docs/OAL_REFERENCE.md` (+ `en/`, `es/`).
- **`android.ofs`**: multitouch (`touch_id`, `EV_POINTER_DOWN/UP`, `ptr_*`), JNI helpers (`jni_find_class`, `jni_string`, `jni_call_object`, `jni_call_void1`, `jni_static_bool`, `package_name`, `start_intent`), `is_external_storage_manager`/`open_all_files_settings` (`MANAGE_EXTERNAL_STORAGE`), **immersive full screen** (`set_immersive`, reapplied when focus is regained), drawing in the camera cutout area (`set_cutout_short_edges`), `keep_screen_on`, `asset_read`, `redraw_needed`. The CLI links `-laaudio`. Doc: `docs/ANDROID.md` (+ `en/`, `es/`).
- **`android.ofs` — engine mode** (hosting SDL/Godot inside the `NativeActivity` itself, no Java in the APK): `release_window` (`Window.takeSurface(null)`/`takeInputQueue(null)` on `EV_CREATE`), `set_content_view`, `dex_load_class` (`DexClassLoader`, several `.dex` files separated by `:`), `jni_scall_cls`, `jni_take_error`/`jni_error`, `set_read_only`, `intent_extra`, `self_intent`, `restart_process` (reopens the activity in a new process). Doc: `docs/ANDROID.md` (+ `en/`, `es/`).
- **`icons.ofs`**: 30 new icons (auto, skip, history, eye/hide, rotate, portrait, landscape, trash, edit, image, volume, text, palette, ok, storage, engines `windows`/`renpy`/`unity`/`moon`/`unknown`, keyboard, backspace, shift, pause, joystick, space, arrow, size, tabs) and aliases in `by_name`.

### Fixed — compiler

- **runtime**: `ofs_array_pop` now returns the removed element (it used to return garbage/0 to callers that used the value).
- **driver (`attach`)**: the path of `attach {F:...}` is normalized (`./`, `../`, duplicate slashes) before checking for duplicates, so the same file is no longer included twice.
- **net**: `net.resolve` reads the bionic `struct addrinfo` (Android), where `ai_addr`/`ai_canonname` are in a different order than in glibc.
- **llvmgen**: an `extern vein` defined in the module itself is no longer redeclared (`invalid redefinition of 'ofs_alloc'` in `magma.ofs` and in `test_magma_full`).

### Added — 100% OFS graphics stack (base of Catalyst Browser)

- **`raster.ofs`**: vector rasterizer with antialiasing by signed-area accumulation (font-rs style): `move_to`/`line_to`/`quad_to`/`cubic_to`/`close`, rounded rectangles, circles, capsules (strokes), arcs, rings, and compositing onto the Canvas with alpha.
- **`ttf.ofs`**: TrueType reader (cmap 4/12, simple and composite glyphs, hhea/hmtx) with UTF-8, glyph cache, subpixel positioning (1/4 px), stem darkening, `width`, `ellipsize`, `draw_centered`. Real text (Inter, Noto, DejaVu) with accents.
- **`x11win.ofs`**: native X11 window in pure OFS (Xlib via `extern`): Canvas presentation through `XPutImage`, keyboard events with UTF-8 text and pt-BR dead keys (´ ` ^ ~ ¨ ç), mouse, wheel, resize, close (`WM_DELETE_WINDOW`), cursors (arrow/hand/text), window icon (`_NET_WM_ICON`), `now_ms`/`sleep_ms`.
- **`icons.ofs`**: 20 interface vector icons (back, forward, reload, home, +, close, lock, info, star, menu, search, globe, download, history, gear, profile, stop, file, favorite, shield) with a per-size cache.
- **`hashmap.ofs`**: integer→integer hash table with open addressing and automatic growth.

### Fixed — compiler (`ofscc`), verified by fixed-point bootstrap (vN.ll == vN+1.ll)

- **parser**: `*`, `-` or `&` at the start of a new line now start a new statement. Before, `v = *p` followed by `*p = 0` became `v = *p * p = 0`.
- **parser**: unary operators apply to the complete postfix expression — `!ns.fn(x)` is `!(ns.fn(x))`, not `(!ns).fn(x)` (it generated a call to the nonexistent `@fn`).
- **llvmgen**: `namespace` globals are accessible from outside (`raster._acc`, `cfg.contador = ...`, `cfg.contador++`); before, they generated a GEP over `%unknown`.
- **llvmgen**: globals with a negative literal (`forge x: stone = -1`) were emitted as `0`. This affected the compiler itself (`interpreter___ret_ref`).
- **typeck**: `ofs_array_get`/`ofs_array_pop`/`ofs_array_len`/`ofs_array_push`/`ofs_array_set` without a declared `extern` now have an inferred `Array<T>` type (before `unknown` → pointer → invalid IR).
- **canvas**: `_sin_deg`/`_cos_deg` with angle reduction to [-π/2, π/2] and a 6-term series (a ~20% error near 180° distorted arcs).
- **dist**: `ofs/dist/ofscc`, `ofs/dist/ofscc.ll` and `ofs/dist/stdlib/` were out of sync with the source — the driver used an old `canvas.ofs` without `draw_shadow` (segfault in Catalyst) and an `ofscc` without `stacksave` in loops. Regenerated from the source.
- **runtime (`ofs_str_char_at`)**: called `strlen` on every character (O(n²) in loops over strings). It now caches (pointer, length) of the last string, validated by the NUL terminator. Loop over 164 KB: 0.203 s → 0.014 s. Applied by hand in `ofs/dist/magma.ll` and `ofs/dist/magma_windows.ll` (which have manual adjustments) and mirrored in `stdlib/runtime/facet.ofs`; `magma.o` regenerated.
- **typeck**: string builtins without `extern` (`ofs_str_len`, `ofs_str_char_at`, `ofs_str_eq`, `ofs_str_contains`, `ofs_str_concat`, `ofs_str_substr`, `ofs_stone_to_obsidian`) have an inferred type (before `unknown` → invalid IR in `forge n = ofs_str_len(s)`).
- **typeck/interpreter (`ofs run`)**: `namespace` globals are pre-declared with the mangled name and `ns.global` is rewritten to `ns__global` before the backends — the false "unknown identifier" warning is gone and `ofs run` now reads/writes those globals. The interpreter also initializes globals declared inside `namespace` (before, they stayed null even for the namespace itself).
- **bootstrap**: `ofs/bootstrap/*.ofs` synchronized with `ofs/ofscc/` (it was an old snapshot without `interpreter`, `events`, `reporter`); scripts pointed to `ofs/ofs/bootstrap/ofscc.ofs` (nonexistent — `bootstrap-minimal.sh` failed in Phase 2) and now use `ofs/ofscc/ofscc.ofs`. `OFS_BOOTSTRAP_SELFHOST=1 bootstrap-minimal.sh --validate` passes with validated determinism.

- **driver (`attach {modulo}`)**: the documented short form (`attach {canvas}`, `attach {terminal-colors}`) never loaded the module — only `attach {F:arquivo.ofs}` worked. It now resolves `modulo.ofs` in the stdlib (hyphen → underscore), with priority over a local file of the same name; applies to `build` and `run`.
- **lexer (hex literals)**: `0xFF`, `0x1A1A2E` went raw into the LLVM IR (read as floating point → "floating point constant invalid") and did not work in the interpreter. They are now converted to decimal in the lexer; `0xFFFFFFFFFFFFFFFF == -1` as in C.
- **llvmgen (small integers)**: `u8`/`u16`/`u32`/`i8`/`i16`/`i32` are extended to `i64` (zext/sext) in binary operations and in `echo` — `(r as u32 << 16) | ...` generated invalid IR and `echo(pixel: u32)` called `echo_obsidian` with an integer.
- **examples**: 28 of 33 compile (before, 21). Three remain Windows-only, `attach_local_lib.ofs` (a library without `main`) and `hybrid_fault_console.ofs` (`fault_count` without an implementation in llvmgen).
- **VS Code (1.0.10)**: autocomplete, hover and file map for `fs`, `net`, `http`, `oll`, `ui`, `raster`, `ttf`, `x11win`, `icons`, `hashmap`; snippets `x11win-app` and `hashmap`; bundled toolchain (`bin/linux-x64`: `ofs`, `ofscc`, `magma.o`, stdlib) updated.
- **site/docs**: stdlib pages (PT/EN/ES) regenerated by `build_site.ofs` with the new modules; `docs/en/STANDARD_LIBRARY.md` and `VISUAL_UI.md` (PT/EN) with the 100% OFS graphics stack.

- **lexer (`\r` and `\t` in strings)**: the case existed, but it used the literals `"\r"`/`"\t"`, which depend on the previous compiler; since the seed did not convert them, the error persisted through the bootstrap and `"\r\n"` generated the bytes `\` `r`. HTTP headers in `http.ofs`/`webserver.ofs` came out invalid (servers answered 400). The bytes are now built by code.
- **llvmgen (bool in arrays)**: `ofs_array_set/push(a, i, x || y)` wrote an `i1` into an `i32` slot (invalid IR); the value is now extended.
- **llvmgen**: `ofs_str_char_at_known` is now declared automatically like the other string functions.
- **driver**: `attach {F:/absolute/path.ofs}` accepts an absolute path.
- **net.ofs**: `net.resolve(host)` — DNS via `getaddrinfo` in OFS (`net.is_ipv4`).
- **Catalyst 3.0.0**: rewritten on top of the OFS graphics stack — Chrome-style interface, tabs with their own history, editable omnibox, bookmarks, menu, new HTML/CSS parser, layout with line wrapping, flex, links, scrolling, DNS and HTTP redirects.

- **net.ofs**: `net.recv_all(fd)` (safe binary reception, `net.last_len`), `net.set_timeout` and `net.timeout_s` (15 s default in `tcp_connect`; before, a silent server hung the program forever).
- **Catalyst 3.1.0**: persistent history and downloads in `~/.catalyst` / `~/Downloads` (real binary downloads), `.cai` plugin system (styles, blocking, redirection, search, `ofs://ext/` pages, buttons, shortcuts, home page), `ofs://plugins` page, Google as default search, crystal icon (`assets/catalyst.svg`).

- **llvmgen (`&&` / `||`)**: the native code always evaluated both sides (no short-circuit); `i >= 0 && arr[i]` read outside the array. The right side now runs only when needed, as in the interpreter and in C.
- **parser (shadowing)**: parameters and local variables of functions inside a `namespace` were renamed to the global with the same name (`forge bg = ...` read and overwrote `ns__bg`). Locals now shadow globals.
- **stdlib**: `icons.by_name(nome)`.
- **Catalyst 3.2.0**: redesigned settings (Chrome-style sidebar, Firefox-style search, cards, toggles, options and buttons with icons), light/dark theme, font size, optional bookmarks bar, pt/en/es languages, choice of search engine, new tab with a search box, forms (`<input>`, `<button>`, GET submission) and the `<icon name>` tag in pages.

- **Catalyst 3.3.0**: Catalyst Shield (native ad/tracker blocking: network, EasyList-style cosmetics, heuristics, user filters, per-site allowlist, counter and panel in the omnibox), Essence View (JavaScript-dependent pages shown from JSON-LD, `__NEXT_DATA__`, OpenGraph and links, without running scripts), 6 accent colors, dynamic theme through `theme-color`, font families (Inter, Noto Sans, DejaVu, Serif), persistent bookmarks with the `ofs://bookmarks` manager.

- **stdlib `tls.ofs`**: HTTPS through the system OpenSSL (SNI, certificate and name verification); link with `OFS_LINK_LIBS="-lssl -lcrypto"`.
- **stdlib `png.ofs`**: 100% OFS PNG decoder with its own inflate/zlib (grayscale, RGB, 1–8 bit palette, alpha); pixel-identical decoding in the tests; `draw`, `draw_cover`, `draw_region`.
- **driver `ofs`**: `OFS_LINK_LIBS` variable for extra system libraries at link time.
- **stdlib `x11win.now_ms`**: works without an open window (before, it hung in headless mode).
- **CI**: the `macos-14` runner was retired by GitHub ("configuration 'macos-14-arm64-us-default' is not supported"); `build.yml` and `release.yml` now use `macos-latest`.
- **Catalyst 3.4.0**: HTTPS, HTTP/1.1 with chunked transfer, external CSS with cache, `@media`/`@supports`, selector-indexed cascade (G1: 40 ms CSS), HTML parser 20× faster on pages with inline JS, profiler `CATALYST_PROFILE=1`, long-word wrapping, `flex-shrink`, user export/import (`.catprofile`), free accent and background colors, PNG/gradient wallpaper on the new tab.

### Known issues

- A `monolith`-type global (`forge cv: Canvas` at namespace scope) reserves only one pointer; assigning the whole struct corrupts memory. Use a local variable or pass it as a parameter.
- `stdlib/runtime/frame.ofs` (the `magma.o` window) is a stub; use `x11win.ofs` for real windows.

### Added

- **Reporter Phases 2–5 (ofscc)**:
  - **Phase 2 — i18n**: built-in en/pt/es catalog with automatic locale detection (`OFS_LANG` > `LANG` > `en`) and mirror TOMLs in `ofs/ofscc/locale/` for translators and parity CI.
  - **Phase 3 — Fancy Reporter**: verbs aligned in a fixed column, ANSI symbols, colors with automatic degradation (`NO_COLOR` / `TERM=dumb`).
  - **Phase 4 — animation**: single-thread spinner between stages, `\r` + clear line, can be disabled with `OFS_ANIM=0`.
  - **Phase 5 — Json NDJSON**: extra events (`locale`, `msg_key` + `args[]`, `duration_ms_total`) and a `twin` output stub in NDJSON.
- **CI golden files**: the `golden.yml` workflow re-bootstraps the compiler and compares Plain/Fancy/Json and locale outputs against the freshly built binary.

### Added

- **canvas (painting layer)**: ARGB alpha blending (`argb`, `blend_pixel`), 1px SDF antialiasing (`fill_rounded_rect_aa`, `stroke_rounded_rect_aa`, `fill_circle_aa`, `stroke_arc`), rectangular clip stack (`push_clip`/`pop_clip`/`clear_clips`), layered soft shadow (`draw_shadow`) and vertical gradient (`fill_gradient_v`). `set_pixel` respects clip and normalizes ARGB colors.
- **OLL**: named constants `OLL_WINDOW…OLL_PROGRESS`, `tabs` now uses horizontal layout, `stack`/`container` overlay their children, a dedicated `progress: stone` field in `OllNode` (`value:`/`progress:`), progress bars actually drawn (track + antialiased fill), rendering with antialiased rounded corners and soft shadow via `elevation`, and `#RRGGBBAA` colors with an alpha channel.
- **CI**: render golden (`tests/render/run.sh`) compares the PPM of the scene (`attach {canvas}` + `attach {oll}`) bit by bit in the Linux job.

### Fixed

- **llvmgen**: integer constant `0` in a global with pointer type (e.g. `forge x: Array<T> = []`) emitted `global ptr 0`, rejected by llc — it now emits `global ptr null`.
- **llvmgen (`emit_global_init`)**: globals with the generic type `Array<T>` never passed the `ofs_str_eq(gt, "Array")` test, so the internal buffer was not allocated in `__ofs_init_globals` and every `ofs_array_push` became a no-op (the parser/lexer of the self-hosted compiler lost state). The `Array<` prefix is now recognized and literal elements are initialized. Fixes the bootstrap v3→v4 and the native compilation of `installer_wizard_linux.ofs` in the release.
- **llvmgen (loops)**: each iteration of `while`/`cycle` now wraps the body with `llvm.stacksave`/`llvm.stackrestore` — allocas of variables local to the body no longer accumulate on the stack in unoptimized builds (`llc -O0`). Fixes stack overflow when rendering large rectangles on the Canvas (e.g. Catalyst Browser) and any loop with thousands of iterations.
- **cli (`ofs run --native`)**: the compilation reporter output is redirected to stderr, keeping the program's stdout clean (parity with interpreted `ofs run`).
- **tests/golden**: the Fancy ANSI case explicitly neutralizes `NO_COLOR`/`OFS_LANG`, which leaked from the CI step variables; the seed bootstrap generates valid IR again (82 global array buffers restored).
- **bootstrap**: `ofs_str_substr` without NUL termination, `attach {F:...}` bounds counting and `monolith` allocation sizes fixed.
- **portability**: `snprintf` and `ofs_stone_to_obsidian` rewritten in pure OFS; `crystal` formats as `%.15g` with a `.0` suffix (parity).
- **CI**: `clang -no-pie` (Linux) and `-Wl,-no_pie` (macOS) when linking `ofscc.ll` generated by non-PIE llc; `-Wl,/force:multiple` (Windows).

### Changed

- **License v1.0.151**: adoption of **Apache-2.0** (`LICENSE`, `docs/legal/LICENSE`) and a **Trademark Policy** (`TRADEMARK.md`).
- **`docs/pt/STATUS_REPORT.md`** moved to `src/archive/STATUS_REPORT.md`; the site now points only to the CHANGELOG.

### Self-Hosted Compiler — Milestones 5–6 (Fixed)

- **Milestone 5**: Typechecker no longer OOMs on deep AST paths — hot accessor functions extracted to keep stack pressure bounded during diagnostic walks.
- **Milestone 6a (mine loops)**: Deferred `anchor` statements were emitted at scope-exit in FIFO order, breaking `defer`-style semantics. The LLVM generator now walks the anchor stack in reverse (LIFO) so that the most recently registered anchor runs first, matching Go/Zig conventions.
- **Milestone 6b (assay / strata)**: `strata` variants had no object representation, producing invalid `icmp eq i64` against `ptr` loads and undefined symbols when constructors like `Crack("boom")` were called. Variants are now global `i64` tags registered in `_variant_names`/`_variant_tags` and variant constructors lower to the tag constant.
- **Codegen**: `Array<T>` element loads now infer the right scalar type (`ptr` for `Array<obsidian>`, `double` for `Array<crystal>`, `i1` for `Array<bool>`), avoiding invalid `call i64 @ofs_str_eq(ptr %x)` patterns after reading from a stone-typed slot.
- **Typeck**: `is_variant_name` tightened to strict PascalCase (starts uppercase, contains at least one lowercase, no underscores). This prevents `EXEC_OK`, `NK_CORE_DECL` and other `SCREAMING_SNAKE` constants from being typed as variants (which previously forced a `ptr` alloca and corrupted downstream code).
- **Typeck / Codegen**: `type_node_name` now preserves the full generic spelling (`Array<obsidian>` instead of bare `Array`), enabling downstream consumers (collect_globals, VAR_DECL emission, array indexing) to pick the correct value type.

### Self-Hosted Compiler — Milestones 5–6 (Validated)

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
