# OFS on Android (aarch64)

OFS builds native Android apps **100% in OFS**: the APK carries a `lib<name>.so` compiled from your code and the stdlib, with no Java, no Gradle and no `.c` files. Only system libraries (libc/bionic, libandroid, liblog) are called, through `extern vein`/`rift vein`, the same way libc is used on the desktop.

Tested on a Moto G8 Power (arm64-v8a, Android 11) on 2026-11-06.

---

## Requirements

| Item | Version used | Purpose |
|---|---|---|
| Android SDK | `platforms/android-*` with `android.jar` | `aapt2 link` |
| build-tools | 37.0.0 (`aapt2`, `zipalign`, `apksigner`) | packaging and signing |
| NDK | 29.0.14206865 | `clang --target=aarch64-linux-android26`, sysroot with libandroid/liblog |
| `keytool` (JDK) | any | debug key generation |
| `adb` (platform-tools) | any | `--install` |

Set `ANDROID_SDK` (or `ANDROID_HOME`/`ANDROID_SDK_ROOT`) to the folder that contains `ndk/`, `build-tools/` and `platforms/` (default `~/Android/Sdk`). The newest version of each is used.

`ofscc` must understand `OFSCC_TARGET` (this version onward). If the installed `ofscc` is older, the CLI builds one from the repository source (`OFS_SOURCE=<repo>/ofs`) and caches it in `~/.cache/ofs/android/`.

---

## Building the APK

```bash
ofs build --target android app.oal -o app-debug.apk          # manifest from oalc
ofs build --target android game.ofs --manifest AndroidManifest.xml
ofs build --target android app.oal --install --serial <device>
```

Steps (`src/packaging/linux/ofs`, `android_build`):

1. `.oal` → `oalc` → `AndroidManifest.xml` + `<name>_oal.ofs` (see [OAL_REFERENCE.md](OAL_REFERENCE.md));
2. `OFSCC_TARGET=android ofscc` → LLVM IR (`aarch64-unknown-linux-android26`) for the app and the `magma` runtime (cached);
3. NDK `clang` → `lib<name>.so` with `-shared -Wl,-init,__ofs_init_globals -u ANativeActivity_onCreate -Wl,-z,max-page-size=16384 -landroid -llog -lm`;
4. `aapt2 link` + zip `lib/arm64-v8a/` + `zipalign -P 16` + `apksigner`.

The `.so` name comes from `android.app.lib_name` in the manifest. Intermediate files go to `build/android/<name>/` next to the source (or `OFS_ANDROID_BUILD`).

---

## Runtime model (stdlib `android.ofs`)

- **Entry:** `android.ofs` exports `vein ANativeActivity_onCreate(...)` and fills `activity->callbacks` with OFS vein addresses via `fault_addr` (see [LANGUAGE_REFERENCE.md](LANGUAGE_REFERENCE.md)).
- **Single thread:** everything runs on the Android main thread, no pthread (the magma runtime is not thread-safe). Input uses `AInputQueue_attachLooper`; frames use `AChoreographer_postFrameCallback` (API 24+), posted only on demand (`android.request_frame()` or `android.continuous = true`). Split long work across `EV_FRAME`s.
- **Contract:** the program defines `vein android_event(ev: stone) -> void` and reads state from `android.*` (`EV_CREATE`…`EV_DESTROY`, `EV_WINDOW_*`, `EV_RESIZE`, `EV_ROTATE`, `EV_FRAME`, `EV_TOUCH_*`, `EV_BACK`, `EV_KEY`, `EV_FOCUS`, `EV_CONFIG`, `EV_LOW_MEMORY`).

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

**Multitouch:** every touch event refers to finger `touch_id` (`AMotionEvent_getPointerId`); extra fingers arrive as `EV_POINTER_DOWN`/`EV_POINTER_UP`, `EV_TOUCH_MOVE` comes once per finger, and all fingers are in `ptr_count`/`ptr_id[]`/`ptr_x[]`/`ptr_y[]`.

**JNI helpers** (main thread, `JNIEnv` table via `fault_call`): `jni_env`, `jni_activity`, `jni_fn`, `jni_find_class`, `jni_string`, `jni_to_text`, `jni_delete`, `jni_check`, `jni_args`, `jni_static_bool`, `package_name()`, `start_intent(action, uri)`; `is_external_storage_manager()` (Android 11+ `MANAGE_EXTERNAL_STORAGE`), `open_all_files_settings()` (opens `Settings.ACTION_MANAGE_APP_ALL_FILES_ACCESS_PERMISSION`), `keep_screen_on(bool)`, `asset_read(name)` (+ `asset_len`), `redraw_needed`.

**Immersive / cutout:** `set_immersive(bool)` hides status and navigation bars (`setSystemUiVisibility` + `IMMERSIVE_STICKY`, re-applied when focus returns); `set_cutout_short_edges(bool)` (Android 9+) draws into the camera cutout area, so landscape has no black strip; generic `jni_call_object(obj, name, sig)` and `jni_call_void1(obj, name, sig, arg)`.

**Audio** (`audio_android.ofs` + `wav.ofs`): AAudio 44.1 kHz stereo i16 stream with a data callback; `audio.open`, `play(channel, sound, loop)`, `stop`, `stop_all`, `set_volume`, `set_master`, `playing`, `pause`/`resume`/`close`, `pump()` once per frame. The mixer runs on the audio thread touching raw memory only; replaced PCM is freed after a safe delay. `wav.ofs` decodes PCM 8/16-bit and Microsoft ADPCM (bit-identical to ffmpeg). The CLI links `-laaudio`.

Functions: `lock`/`unlock_post`, `fill_rect`, `blit`, `request_frame`, `finish`, `set_orientation(ORIENT_REQ_*)` (JNI `setRequestedOrientation`), `call_activity_void_int`, `internal_data_path`, `external_data_path`, `log`, `log_tag`, `s32`, `peek`/`poke`. The back key arrives as `EV_BACK` and does **not** close the app by itself: call `android.finish()` (OAL does it by default).

---

## Current limits

- `arm64-v8a` only, API 26+.
- Software rendering (`ANativeWindow_lock`), no GL/Vulkan.
- Audio: WAV only (PCM / MS-ADPCM), no Ogg/MP3; no `onSaveInstanceState` (save your state in `on pause`).
- C struct offsets (`ANativeActivity`, `ANativeWindow_Buffer`, `ARect`, `JNIEnv` table) are fixed for aarch64.
- 32-bit C ints arrive as `stone` with undefined upper bits: use `android.s32()`.
- `fault_call` passes integers and pointers only.
- The installed `ofs/dist/magma.o` and `ofscc` predate the Android target; until the next release the CLI builds `ofscc` from source.

## Generic JNI, WebView, HTTP (stdlib)

- **Generic JNI** (`android.ofs`): `jni_call(obj, name, signature, a..f)` (the return kind comes from the signature: `V`, `Z`, `I`, `J`, object), `jni_new(class, signature, a..d)`, `jni_scall(class, name, signature, a..d)` (static), `jni_static_int_field`, `jni_global`/`jni_delete_global`, `jni_new_byte_array`/`jni_get_bytes`/`jni_set_bytes`, `jni_new_object_array`/`jni_set_object_elem`; `android.jni_failed` tells whether a Java exception happened. Main thread only.
- **`android_webview.ofs`**: `webview.open(base_url, html)` opens a `WebView` (JS, DOM storage, file access, media without gesture) in a full-screen immersive `android.app.Dialog`; `poll()` (Back closes the Dialog), `eval(js)`, `title()`, `pause(on)`, `close()`. **Why a Dialog:** `NativeActivity` takes the main window surface (`takeSurface`), so a `View` added to its window is not drawn; the Dialog has its own window and gets touch directly (not through `AInputQueue`). No Java subclass: the shim goes in as a `<script>` in the HTML (`loadDataWithBaseURL("file:///folder/", html)`), and the JS-to-OFS return channel is `document.title` (Chromium cuts titles at 4096 characters and collapses whitespace/control chars: send in chunks and with `encodeURIComponent`). `EV_FRAME` keeps arriving while the main window is covered. `ofs_str_char_at` is O(n): read bytes by pointer on large texts.
- **`android_http.ofs`**: `ahttp.open(url)`/`read(buf)`/`close()` and `get_text(url)` over `HttpURLConnection` (HTTPS with certificate checking and redirects; the OpenSSL of `tls.ofs` cannot be linked in an APK). Blocks up to ~4 s per call on the main thread (use short per-frame steps; `StrictMode` is relaxed). Needs `permission INTERNET` in the `.oal`. Accepts `file:///...` and absolute paths.

## Engine mode: hosting other code/Views in the NativeActivity (stdlib `android.ofs`)

To run a game engine (SDL, Godot) **inside** the app, without installing an APK and without Java in the APK (`hasCode=false`):

- `release_window()`: calls `getWindow().takeSurface(null)` and `takeInputQueue(null)`. **Call it in `EV_CREATE`** (the window is attached only after `onResume`). From then on the `NativeActivity` no longer claims the surface or the input queue: `set_content_view(view)` (or a Java adapter's `setContentView`) draws normally and gets touch/keys through Java. OFS stops receiving `EV_WINDOW_*` and touches; lifecycle events still arrive. Tested on Android 11 (Moto G8 Power). In AOSP's `NativeActivity.java`, `onCreate` calls `takeSurface(this)`/`takeInputQueue(this)` before `ANativeActivity_onCreate`; `ViewRootImpl` reads those callbacks when the window is attached, so `null` wins.
- `dex_load_class(dex, libs_dir, "pkg.Class")`: `DexClassLoader` (parent: the activity's class loader) + `loadClass`; returns a **global** class reference or 0 (reason in `jni_error`). `dex` accepts several files separated by `:`. The `.dex` must live in the app's internal storage and, on Android 14+, be read-only (`set_read_only(path)`). `libs_dir` is that loader's `System.loadLibrary` search path.
- `jni_scall_cls(cls, name, signature, a..d)`: static method on an already obtained `jclass` (e.g. from `dex_load_class`).
- `jni_take_error()`: if a Java exception is pending, stores its `toString()` in `jni_error`, prints the stack to logcat (`System.err`) and clears it.
- `intent_extra(key)`: `getIntent().getStringExtra(key)` ("" if missing).
- `self_intent(keys, values)` / `restart_process(keys, values)`: `Intent` for this same activity with `String` extras and `FLAG_ACTIVITY_NEW_TASK|CLEAR_TASK`; `restart_process` calls `startActivity` and kills the process (`Process.killProcess(myPid)`): the system opens the activity in a fresh process (~60 ms on a Moto G8). M.O.O.N uses it to enter and leave "engine mode" (one engine per process).
