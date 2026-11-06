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

## Límites actuales

Solo `arm64-v8a` y API 26+; render por software; audio solo WAV (PCM/MS-ADPCM); offsets de structs C fijos para aarch64; `fault_call` solo con enteros y punteros; el `magma.o`/`ofscc` instalados son anteriores al objetivo Android (la CLI compila `ofscc` desde el código fuente).
