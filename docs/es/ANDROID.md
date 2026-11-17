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
# a partir de un .oal (manifiesto generado por oalc)
ofs build --target android app.oal -o app-debug.apk

# a partir de un .ofs que define android_event (manifiesto: --manifest, o
# AndroidManifest.xml junto al fuente, o uno generado con org.ofs.<nombre>)
ofs build --target android juego.ofs --manifest AndroidManifest.xml

# instalar y abrir
ofs build --target android app.oal --install --serial 0064380838
```

Lo que hace la CLI (`src/packaging/linux/ofs`, función `android_build`):

1. `.oal` → `oalc` → `AndroidManifest.xml` + `<nombre>_oal.ofs` (ver [OAL_REFERENCE.md](OAL_REFERENCE.md));
2. `OFSCC_TARGET=android ofscc` → LLVM IR con `target triple = "aarch64-unknown-linux-android26"` de la app y del runtime `magma` (el runtime queda en caché);
3. `clang` del NDK → `lib<nombre>.so` con `-shared -Wl,-init,__ofs_init_globals -u ANativeActivity_onCreate -Wl,-z,max-page-size=16384 -landroid -llog -lm`;
4. `aapt2 link` + `zip` de `lib/arm64-v8a/` + `zipalign -P 16` + `apksigner` (clave `~/.android/debug.keystore` o una generada en la caché).

El nombre de la `.so` viene de `android.app.lib_name` en el manifiesto. Los archivos intermedios quedan en `build/android/<nombre>/` junto al fuente (o en `OFS_ANDROID_BUILD`).

## Edición en vivo en el dispositivo (Fault Refract L3)

Con el celular por USB (depuración USB activa), la app corre en el dispositivo y cada edición guardada en el PC entra en ella **sin reinstalar ni reiniciar el proceso**: las veins editadas se cambian en su próxima llamada y los globales conservan sus valores.

```bash
ofs refract --remote app.oal                   # build --refract, instala, abre, conecta y observa
ofs refract --remote app.oal --serial 0064380838
ofs build --target android app.oal --refract --install   # solo el APK con celdas y agente
```

Ejemplo: `ofs/examples/refract_android/` (un contador por segundo; edite `painel.texto()` en `painel.ofs` y guarde). Salida en el PC:

```text
refract: conectado a org.ofs.refract.demo (pid 16109, geracao 0, porta 7341 -> localabstract:ofs-refract.org.ofs.refract.demo)
refract: alteradas: painel.texto
refract: geracao 1 aplicada no app em 1 ms: painel__texto (6272 bytes, pid 16109)
```

**Cómo funciona.**

1. **Build `--refract`.** Las veins del usuario (el archivo principal y los `attach` locales; la stdlib queda fuera) reciben una **celda de despacho** exportada (`@__ofs_cell.<vein>`, un puntero) y cada llamada directa a ellas lee la celda. El `.so` lleva el manifiesto (`@__ofs_refract_manifest`: hash del árbol tipado y firma de cada vein, tipos de los globales, layout de los `monolith`, `extern`, variantes) y el agente `geophone` (stdlib `geophone.ofs`, anexado por el `ofscc`). La lib recibe `-soname lib<nombre>.so` para que el parche enlace contra ella. **Sin `--refract` nada de esto existe**: el IR es idéntico al de antes.
2. **Agente (`geophone`).** Arranca como constructor de la lib, en el hilo principal, y escucha en el socket Unix abstracto `ofs-refract.<paquete>`. El socket entra en el looper del hilo principal (`ALooper_addFd`), así que el parche se aplica **entre dos eventos** y el runtime (que no es thread-safe) no ve otro hilo. Log: `adb logcat -s ofs-refract`.
3. **PC (`ofs refract --remote`).** Compila con `--refract`, instala y abre, `adb forward tcp:7341 localabstract:ofs-refract.<paquete>`, conecta, pide el manifiesto y observa el `.oal` y los `.ofs` del usuario. Al guardar: `oalc` (si es `.oal`), `ofscc` en modo parche (`OFSCC_REFRACT=patch`: diff vein a vein contra el manifiesto de la app y las reglas de abajo), `clang` del NDK (`--target=aarch64-linux-android26 -shared -fPIC -Wl,-Bsymbolic -Wl,--no-undefined`, enlazado contra la `lib<nombre>.so` de la app), envía el `.so` y muestra la respuesta de la app.
4. **En la app.** El `.so` va a `code_cache/ofs-refract/`, se abre con `dlopen` y `__ofs_refract_install` escribe en las celdas la dirección de las veins nuevas. Los globales del parche son los de la app (`external`, misma dirección): el estado continúa. Después el agente llama a `oal.refresh()` (apps OAL) o pide un cuadro.

**Qué entra y qué se rechaza.** Vein cambiada con la misma firma: cambiada. Vein nueva: va en el parche (sin celda en la app); ella y quien la llama se reenvían en cada parche siguiente. Vein eliminada: queda en la app, sin uso. `monolith` nuevo, `extern` nueva y variantes añadidas al final: aceptados. **Rechazo** (la app sigue con la versión anterior; el mensaje dice el motivo y pide reinstalar): firma nueva de una vein con celda, global nuevo o con tipo nuevo, layout cambiado de un `monolith` existente (la app nativa no migra instancias), `extern` con firma nueva, variantes eliminadas o reordenadas. Error de compilación: no se envía nada. Un parche que usa un símbolo que la app no tiene (un `attach` nuevo de la stdlib) falla al enlazar en el PC, con el nombre del símbolo. Cambios en la cabecera del `.oal` (`app`/`version`/`permission`) piden reinstalar.

**Límites.** Una vein en ejecución termina en la versión antigua (sin OSR en nativo). Un valor de vein guardado (callback entregado a C, `fault_addr`) apunta a la versión del build: el PC avisa. El `.so` de cada parche queda cargado hasta que termina el proceso. Solo aarch64/API 26+.

**Seguridad.** El agente solo existe en builds `--refract` (nunca en el build normal ni en el de release). El socket es abstracto y local: desde fuera del dispositivo solo lo alcanza `adb`, y el agente solo acepta conexiones con el uid de `adb` (shell, 2000), root o la propia app (`SO_PEERCRED`); las demás se rechazan y se registran. Cualquiera con acceso a `adb` puede inyectar código en la app: es la misma frontera de confianza de `adb`/`run-as`; no distribuya un APK `--refract`.

**Protocolo v1** (líneas de texto terminadas en `\n`): al conectar la app manda `OFS-REFRACT 1 <paquete> <pid> <generación>`; `MANIFEST` → las líneas del manifiesto y `END`; `PATCH <generación> <n>` seguido de `n` bytes del `.so` → `OK <generación> <ms> <veins>` o `ERR <generación> <motivo>`; `PING` → `PONG`; `BYE` cierra. Manifiesto: un registro por línea, campos separados por tab (`V <símbolo> <hash> <firma> <1|0 celda>`, `G <global> <tipo>`, `M <monolith> <campos>`, `X <extern> <forma>`, `S - <variantes>`); la app junta los registros de cada parche (clave = los dos primeros campos), así el PC siempre compara con lo que está corriendo.

Variables: `OFS_REFRACT_PORT` (puerto en el PC, 7341), `OFS_REFRACT_PERIOD` (ms entre verificaciones, 300), `OFS_REFRACT_ONCE=1` (sale tras el primer parche aplicado o rechazado). Prueba en el dispositivo: `ofs/tests/refract/android.sh` (se omite sin dispositivo).

## Windows

El mismo comando funciona en Windows (`src/packaging/windows/ofs.ps1`, `Invoke-AndroidBuild`; mismos pasos que en Linux). Requisitos además de OFS:

| Elemento | Dónde / cómo instalar |
|---|---|
| LLVM (`clang` en el PATH) | `winget install LLVM.LLVM` (compila `oalc` en el host) |
| Android SDK | por defecto `%LOCALAPPDATA%\Android\Sdk` (Android Studio) o `ANDROID_SDK`/`ANDROID_HOME`/`ANDROID_SDK_ROOT` |
| NDK | `ndk\<versión>\toolchains\llvm\prebuilt\windows-x86_64\bin\clang.exe` (`sdkmanager "ndk;29.0.14206865"`; o `ANDROID_NDK`) |
| build-tools | `build-tools\<versión>\aapt2.exe`, `zipalign.exe`, `apksigner.bat` (`sdkmanager "build-tools;37.0.0"`) |
| `platforms` | `platforms\android-*\android.jar` (`sdkmanager "platforms;android-34"`) |
| JDK 17+ | `keytool`, `jar` y `apksigner`; se busca en `JAVA_HOME`, `Program Files\Android\Android Studio\jbr`, Adoptium/Microsoft/Zulu o PATH |

```powershell
ofs build --target android app.oal -o app-debug.apk
ofs build --target android app.oal --install --serial <dispositivo>
```

Diferencias con Linux: la caché (`ofscc` con `OFSCC_TARGET` si el instalado es antiguo, `oalc.exe`, `magma-aarch64.o`) está en `%LOCALAPPDATA%\ofs\android\`; la clave de debug es `%USERPROFILE%\.android\debug.keystore` (o una generada en la caché con `keytool`); `lib/` se guarda sin compresión con el `jar` del JDK (sin `zip`). El instalador incluye `tools\`, `stdlib\runtime\` y las fuentes de `ofscc\`; `OFS_SOURCE=<repo>\ofs` usa un checkout. Los errores indican qué falta y el comando `sdkmanager`. Estado: verificado por revisión contra el flujo de Linux y por un job de CI informativo (`android-windows` en `build.yml`).

---

## Modelo de ejecución (stdlib `android.ofs`)

- **Entrada:** `android.ofs` exporta `vein ANativeActivity_onCreate(...)`, que `NativeActivity` encuentra por `dlsym`. Llena `activity->callbacks` con la dirección de veins OFS usando `fault_addr` (ver [LANGUAGE_REFERENCE.md](LANGUAGE_REFERENCE.md)).
- **Un solo hilo:** todo corre en el hilo principal de Android, sin pthread. El runtime magma no es thread-safe, así que es intencional.
  - ciclo de vida: callbacks `onStart/Resume/Pause/Stop/Destroy`, `onWindowFocusChanged`, `onNativeWindow*`, `onInputQueue*`, `onContentRectChanged`, `onConfigurationChanged`, `onLowMemory`;
  - entrada: `AInputQueue_attachLooper` en el looper del hilo principal, con callback OFS;
  - cuadros: `AChoreographer_postFrameCallback` (API 24+), reprogramado solo cuando hay pedido (`android.request_frame()` o `android.continuous = true`).
  - El trabajo largo debe dividirse en trozos, uno por `EV_FRAME`.
- **Contrato:** el programa define `vein android_event(ev: stone) -> void` y lee el estado en `android.*`:

```ofs
attach {F:android.ofs}

vein android_event(ev: stone) -> void {
    if (ev == android.EV_FRAME) {
        if (android.lock()) {
            android.fill_rect(0, 0, android.buf_w, android.buf_h, 1052704)
            android.unlock_post()
        }
    }
    if (ev == android.EV_TOUCH_DOWN) { android.request_frame() }
    if (ev == android.EV_BACK) { android.finish() }
}
```

| Eventos | Estado |
|---|---|
| `EV_CREATE`, `EV_START`, `EV_RESUME`, `EV_PAUSE`, `EV_STOP`, `EV_DESTROY` | `activity`, `sdk_version`, `resumed` |
| `EV_WINDOW_CREATED`, `EV_WINDOW_DESTROYED`, `EV_RESIZE`, `EV_ROTATE`, `EV_FRAME` | `window`, `win_w`, `win_h`, `orientation`, `content_*`, `frame_ns` |
| `EV_TOUCH_DOWN/UP/MOVE/CANCEL`, `EV_POINTER_DOWN/UP` (otros dedos) | `touch_id`, `touch_x`, `touch_y`, `touch_count`; todos los dedos en `ptr_count`, `ptr_id[]`, `ptr_x[]`, `ptr_y[]` |
| `EV_BACK` (tecla atrás soltada), `EV_KEY` | `key_code`, `key_action` |
| `EV_FOCUS`, `EV_CONFIG`, `EV_LOW_MEMORY` | `has_focus` |

Funciones: `lock()`/`unlock_post()` (buffer RGBA_8888 del `ANativeWindow`: `buf_w`, `buf_h`, `buf_stride`, `buf_bits`), `fill_rect`, `blit(pixels, w, h)` (copia un buffer del `Canvas`), `request_frame()`, `finish()`, `set_orientation(ORIENT_REQ_*)` (JNI `setRequestedOrientation`), `call_activity_void_int(nombre, valor)`, `internal_data_path()`, `external_data_path()`, `log(texto)`, `log_tag(tag, texto)`, `s32(v)`, `peek`/`poke`.

**Multitáctil:** cada evento vale para el dedo `touch_id` (`AMotionEvent_getPointerId`). El primer dedo genera `EV_TOUCH_DOWN`, los siguientes `EV_POINTER_DOWN`; al soltar, `EV_POINTER_UP` (aún hay dedos) o `EV_TOUCH_UP` (el último). `EV_TOUCH_MOVE` llega una vez por dedo.

**JNI (hilo principal, por la tabla del `JNIEnv` con `fault_call`):** `jni_env()`, `jni_activity()`, `jni_fn(i)`, `jni_find_class(nombre)`, `jni_string(s)`, `jni_to_text(jstring)`, `jni_delete(ref)`, `jni_check()` (limpia la excepción), `jni_args(a, b, c, d)` (jvalue), `jni_static_bool(clase, método)`, `package_name()`, `start_intent(acción, uri)`.

- `android.is_external_storage_manager()`: `Environment.isExternalStorageManager()` (Android 11+; antes devuelve true).
- `android.open_all_files_settings()`: abre `Settings.ACTION_MANAGE_APP_ALL_FILES_ACCESS_PERMISSION` con `package:<paquete>` (con alternativas). En el `.oal`: `permission android.permission.MANAGE_EXTERNAL_STORAGE`.
- `android.keep_screen_on(bool)`: `FLAG_KEEP_SCREEN_ON`.
- `android.set_immersive(bool)`: pantalla completa inmersiva (oculta barras de estado y navegación; `setSystemUiVisibility` con `IMMERSIVE_STICKY` vía JNI); se reaplica sola cuando la ventana recupera el foco.
- `android.set_cutout_short_edges(bool)`: Android 9+: dibujar también en la franja del recorte de la cámara (`layoutInDisplayCutoutMode = SHORT_EDGES`), sin la franja negra en horizontal.
- `android.jni_call_object(obj, nombre, firma)` / `jni_call_void1(obj, nombre, firma, arg)`: llamadas genéricas sin argumento / con 1 argumento.
- `android.asset_read("archivo")`: lee `assets/` del APK (`AAssetManager`); tamaño en `android.asset_len`.
- `android.redraw_needed`: la ventana fue creada/redimensionada/redibujada: el programa debe copiar el cuadro de nuevo (el backend de OAL solo copia cuando algo cambió).

**Audio (`audio_android.ofs` + `wav.ofs`):** `audio.open()` abre un stream AAudio 44,1 kHz estéreo i16 con callback; `audio.play(canal, wav.load(...) o wav.decode(ptr, n), loop)`, `stop`, `stop_all`, `set_volume(canal, %)`, `set_master(%)`, `playing(canal)`, `pause`/`resume`/`close`, `pump()` una vez por cuadro. El mixer corre en el hilo de audio y solo toca memoria cruda (nada de Array/strings); el PCM reemplazado se libera con un retraso seguro. `wav.ofs` decodifica PCM de 8/16 bits y **Microsoft ADPCM** (salida idéntica a la de ffmpeg). La CLI enlaza `-laaudio`.

El botón atrás se entrega como `EV_BACK` y **no** cierra la app solo: llama a `android.finish()` (la OAL lo hace por defecto).

---

### Entrada de texto con el teclado del sistema (`android_input.ofs`)

`ainput.open(título, texto, multilínea)` abre un `android.app.Dialog` con un `EditText` creado por JNI (sin Java propio): teclado predeterminado (Gboard), cursor, pegar y autocompletar. Sin listeners: los botones son `ToggleButton` (alternan al tocar; `ainput.poll()` lee `isChecked()`), y volver/tocar fuera cierra el Dialog (`isShowing()`). `poll()` devuelve 0 abierto, 1 OK, 2 cancelado; `ainput.text()` lee el texto. OAL lo expone como `oal.edit_open`.

## Límites actuales

- Solo `arm64-v8a` (aarch64) y API 26+. Sin x86/armv7.
- Render por software (`ANativeWindow_lock`), sin OpenGL/Vulkan.
- Audio: solo WAV (PCM/MS-ADPCM); sin Ogg/MP3; sin remuestreo de calidad (paso entero 16.16).
- Sin `onSaveInstanceState` (la app guarda su propio estado en `on pause`).
- Los offsets de structs C (`ANativeActivity`, `ANativeWindow_Buffer`, `ARect`, `struct stat`/`statfs`, tabla del `JNIEnv`) están fijos para aarch64.
- Los enteros C de 32 bits llegan como `stone` con la parte alta indefinida: usa `android.s32()`.
- `fault_call` solo pasa enteros y punteros (sin `float`/`double`).
- El `ofs/dist/magma.o` y el `ofscc` instalados aún son anteriores al objetivo Android: hasta el próximo release, la CLI compila el `ofscc` desde el código fuente.

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
