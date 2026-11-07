# OFS en Android (aarch64)

OFS genera apps Android nativas **100% en OFS**: el APK lleva una `lib<nombre>.so` compilada de tu código y de la stdlib, sin Java, sin Gradle y sin archivos `.c`. Solo se llaman bibliotecas del sistema (libc/bionic, libandroid, liblog) mediante `extern vein`/`rift vein`, igual que la libc en el escritorio.

Probado en un Moto G8 Power (arm64-v8a, Android 11) el 2026-11-06.

---

## Requisitos

| Elemento | Versión usada | Para qué |
|---|---|---|
| Android SDK | `platforms/android-*` con `android.jar` | `aapt2 link` |
| build-tools | 37.0.0 (`aapt2`, `zipalign`, `apksigner`) | empaquetar y firmar |
| NDK | 29.0.14206865 | `clang --target=aarch64-linux-android26` |
| `keytool` (JDK) | cualquiera | clave de debug |
| `adb` | cualquiera | `--install` |

Define `ANDROID_SDK` (o `ANDROID_HOME`/`ANDROID_SDK_ROOT`) con la carpeta que contiene `ndk/`, `build-tools/` y `platforms/` (por defecto `~/Android/Sdk`). Si el `ofscc` instalado no entiende `OFSCC_TARGET`, la CLI compila uno desde el código fuente (`OFS_SOURCE=<repo>/ofs`) en `~/.cache/ofs/android/`.

---

## Generar el APK

```bash
ofs build --target android app.oal -o app-debug.apk
ofs build --target android juego.ofs --manifest AndroidManifest.xml
ofs build --target android app.oal --install --serial <dispositivo>
```

Pasos: `.oal` → `oalc` → `AndroidManifest.xml` + `<nombre>_oal.ofs` ([OAL_REFERENCE.md](OAL_REFERENCE.md)); `OFSCC_TARGET=android ofscc` → LLVM IR aarch64 del app y del runtime `magma`; `clang` del NDK → `lib<nombre>.so` (`-shared -Wl,-init,__ofs_init_globals -u ANativeActivity_onCreate -landroid -llog -lm`); `aapt2` + `zipalign -P 16` + `apksigner`.

---

## Modelo de ejecución (stdlib `android.ofs`)

- `android.ofs` exporta `vein ANativeActivity_onCreate(...)` y llena `activity->callbacks` con direcciones de veins OFS mediante `fault_addr`.
- Un solo hilo (el principal de Android), sin pthread: entrada con `AInputQueue_attachLooper`, frames con `AChoreographer_postFrameCallback`, pedidos con `android.request_frame()`.
- El programa define `vein android_event(ev: stone) -> void` y lee el estado en `android.*` (`EV_CREATE`…`EV_DESTROY`, `EV_WINDOW_*`, `EV_ROTATE`, `EV_FRAME`, `EV_TOUCH_*`, `EV_BACK`…).
- Funciones: `lock`/`unlock_post`, `fill_rect`, `blit`, `request_frame`, `finish`, `set_orientation` (JNI `setRequestedOrientation`), `internal_data_path`, `external_data_path`, `log`, `s32`.
- **Multitáctil:** cada evento es del dedo `touch_id`; los demás dedos llegan como `EV_POINTER_DOWN`/`EV_POINTER_UP`; todos en `ptr_count`, `ptr_id[]`, `ptr_x[]`, `ptr_y[]`.
- **JNI:** `jni_find_class`, `jni_string`, `jni_static_bool`, `package_name()`, `start_intent(accion, uri)`, `is_external_storage_manager()`, `open_all_files_settings()` (`Settings.ACTION_MANAGE_APP_ALL_FILES_ACCESS_PERMISSION`), `keep_screen_on`, `asset_read` (`assets/` del APK).
- **Pantalla completa / recorte:** `set_immersive(bool)` (oculta barras con `IMMERSIVE_STICKY`, se reaplica al recuperar el foco), `set_cutout_short_edges(bool)` (dibuja en el recorte de la cámara, sin franja negra en horizontal), `jni_call_object`, `jni_call_void1`.
- **Audio:** `audio_android.ofs` (AAudio con callback, 4 canales, volumen, loop) y `wav.ofs` (PCM y Microsoft ADPCM). La CLI enlaza `-laaudio`.
- El botón atrás llega como `EV_BACK` y **no** cierra la app solo: llama `android.finish()` (la OAL lo hace por defecto).

```ofs
attach {F:android.ofs}

vein android_event(ev: stone) -> void {
    if (ev == android.EV_FRAME) {
        if (android.lock()) {
            android.fill_rect(0, 0, android.buf_w, android.buf_h, 1052704)
            android.unlock_post()
        }
    }
    if (ev == android.EV_BACK) { android.finish() }
}
```

### Entrada de texto con el teclado del sistema (`android_input.ofs`)

`ainput.open(título, texto, multilínea)` abre un `android.app.Dialog` con un `EditText` creado por JNI (sin Java propio): teclado predeterminado (Gboard), cursor, pegar y autocompletar. Sin listeners: los botones son `ToggleButton` (alternan al tocar; `ainput.poll()` lee `isChecked()`), y volver/tocar fuera cierra el Dialog (`isShowing()`). `poll()` devuelve 0 abierto, 1 OK, 2 cancelado; `ainput.text()` lee el texto. OAL lo expone como `oal.edit_open`.

## Límites actuales

Solo `arm64-v8a` y API 26+; render por software; audio solo WAV (PCM/MS-ADPCM); offsets de structs C fijos para aarch64; `fault_call` solo con enteros y punteros; el `magma.o`/`ofscc` instalados son anteriores al objetivo Android (la CLI compila `ofscc` desde el código fuente).

## JNI genérico, WebView, HTTP (stdlib)

- **JNI genérico** (`android.ofs`): `jni_call(obj, nombre, firma, a..f)` (el tipo de retorno sale de la firma: `V`, `Z`, `I`, `J`, objeto), `jni_new(clase, firma, a..d)`, `jni_scall(clase, nombre, firma, a..d)` (estático), `jni_static_int_field`, `jni_global`/`jni_delete_global`, `jni_new_byte_array`/`jni_get_bytes`/`jni_set_bytes`, `jni_new_object_array`/`jni_set_object_elem`; `android.jni_failed` indica si hubo excepción Java. Solo en el hilo principal.
- **`android_webview.ofs`**: `webview.open(base_url, html)` abre un `WebView` (JS, DOM storage, acceso a archivos, medios sin gesto) en un `android.app.Dialog` a pantalla completa inmersiva; `poll()` (Atrás cierra el Dialog), `eval(js)`, `title()`, `pause(on)`, `close()`. **Por qué un Dialog:** `NativeActivity` toma la superficie de la ventana principal (`takeSurface`), así que una `View` añadida a su ventana no se dibuja; el Dialog tiene ventana propia y recibe el toque directamente (sin pasar por `AInputQueue`). Sin subclase Java: el shim entra como `<script>` en el HTML (`loadDataWithBaseURL("file:///carpeta/", html)`) y el retorno del JS al OFS va por `document.title` (Chromium corta el título en 4096 caracteres y colapsa espacios/controles: envía en trozos y con `encodeURIComponent`). `EV_FRAME` sigue llegando con la ventana principal tapada. `ofs_str_char_at` es O(n): en textos grandes lee bytes por puntero.
- **`android_http.ofs`**: `ahttp.open(url)`/`read(buf)`/`close()` y `get_text(url)` sobre `HttpURLConnection` (HTTPS con verificación de certificado y redirecciones; el OpenSSL de `tls.ofs` no se puede enlazar en un APK). Bloquea hasta ~4 s por llamada en el hilo principal (usa pasos cortos por cuadro; `StrictMode` se relaja). Necesita `permission INTERNET` en el `.oal`. Acepta `file:///...` y rutas absolutas.

## Modo motor: alojar otro código/View en la NativeActivity (stdlib `android.ofs`)

Para ejecutar un motor de juego (SDL, Godot) **dentro** de la app, sin instalar APK y sin Java en el APK (`hasCode=false`):

- `release_window()`: llama a `getWindow().takeSurface(null)` y `takeInputQueue(null)`. **Llámela en `EV_CREATE`** (la ventana se adjunta solo después de `onResume`). Desde ahí la `NativeActivity` ya no reserva la superficie ni la cola de entrada: `set_content_view(view)` (o el `setContentView` de un adaptador Java) dibuja normalmente y recibe toque/teclado por Java. OFS deja de recibir `EV_WINDOW_*` y toques; el ciclo de vida sigue llegando. Probado en Android 11 (Moto G8 Power). En el `NativeActivity.java` de AOSP, `onCreate` hace `takeSurface(this)`/`takeInputQueue(this)` antes de `ANativeActivity_onCreate`; `ViewRootImpl` lee esos callbacks al adjuntar la ventana, así que el `null` vale.
- `dex_load_class(dex, carpeta_libs, "paquete.Clase")`: `DexClassLoader` (padre: el class loader de la activity) + `loadClass`; devuelve referencia **global** de la clase o 0 (motivo en `jni_error`). `dex` acepta varios archivos separados por `:`. El `.dex` debe estar en el almacenamiento interno de la app y, en Android 14+, ser de solo lectura (`set_read_only(ruta)`). `carpeta_libs` es la ruta de búsqueda de `System.loadLibrary` de ese loader.
- `jni_scall_cls(clase, nombre, firma, a..d)`: método estático sobre una `jclass` ya obtenida (p. ej. de `dex_load_class`).
- `jni_take_error()`: si hay una excepción Java pendiente, guarda su `toString()` en `jni_error`, imprime la pila en logcat (`System.err`) y la limpia.
- `intent_extra(clave)`: `getIntent().getStringExtra(clave)` ("" si no hay).
- `self_intent(claves, valores)` / `restart_process(claves, valores)`: `Intent` para esta misma activity con extras `String` y `FLAG_ACTIVITY_NEW_TASK|CLEAR_TASK`; `restart_process` hace `startActivity` y termina el proceso (`Process.killProcess(myPid)`): el sistema abre la activity en un proceso nuevo (~60 ms en un Moto G8). M.O.O.N lo usa para entrar y salir del "modo motor" (un motor por proceso).
