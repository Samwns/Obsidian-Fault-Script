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
