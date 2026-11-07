# OFS no Android (aarch64)

O OFS gera apps Android nativos **100% em OFS**: o APK leva uma `lib<nome>.so` compilada do seu código e da stdlib, sem Java, sem Gradle e sem nenhum arquivo `.c`. Só as bibliotecas do sistema (libc/bionic, libandroid, liblog) são chamadas, via `extern vein`/`rift vein`, como a libc no desktop.

Testado num Moto G8 Power (arm64-v8a, Android 11) em 2026-11-06.

---

## Requisitos

| Item | Versão usada | Para quê |
|---|---|---|
| Android SDK | `platforms/android-*` com `android.jar` | `aapt2 link` |
| build-tools | 37.0.0 (`aapt2`, `zipalign`, `apksigner`) | empacotar e assinar |
| NDK | 29.0.14206865 | `clang --target=aarch64-linux-android26`, sysroot com libandroid/liblog |
| `keytool` (JDK) | qualquer | gerar a chave de debug, se não existir |
| `adb` (platform-tools) | qualquer | `--install` |

Defina `ANDROID_SDK` (ou `ANDROID_HOME`/`ANDROID_SDK_ROOT`) com a pasta que contém `ndk/`, `build-tools/` e `platforms/`. O padrão é `~/Android/Sdk`. A CLI escolhe a versão mais nova de cada um.

O `ofscc` precisa entender `OFSCC_TARGET` (versões a partir desta). Se o `ofscc` instalado for anterior, a CLI compila um do fonte do repositório (`OFS_SOURCE=<repo>/ofs`) e guarda em `~/.cache/ofs/android/`.

---

## Gerar o APK

```bash
# a partir de um .oal (manifesto gerado pelo oalc)
ofs build --target android app.oal -o app-debug.apk

# a partir de um .ofs que define android_event (manifesto: --manifest, ou
# AndroidManifest.xml ao lado do fonte, ou um gerado com org.ofs.<nome>)
ofs build --target android jogo.ofs --manifest AndroidManifest.xml

# instalar e abrir
ofs build --target android app.oal --install --serial 0064380838
```

O que a CLI faz (`src/packaging/linux/ofs`, função `android_build`):

1. `.oal` → `oalc` → `AndroidManifest.xml` + `<nome>_oal.ofs` (ver [OAL_REFERENCE.md](OAL_REFERENCE.md));
2. `OFSCC_TARGET=android ofscc` → LLVM IR com `target triple = "aarch64-unknown-linux-android26"` do app e do runtime `magma` (o runtime fica em cache);
3. `clang` do NDK → `lib<nome>.so` com `-shared -Wl,-init,__ofs_init_globals -u ANativeActivity_onCreate -Wl,-z,max-page-size=16384 -landroid -llog -lm`;
4. `aapt2 link` + `zip` da `lib/arm64-v8a/` + `zipalign -P 16` + `apksigner` (chave `~/.android/debug.keystore` ou uma gerada no cache).

O nome da `.so` vem de `android.app.lib_name` no manifesto. Arquivos intermediários ficam em `build/android/<nome>/` ao lado do fonte (ou em `OFS_ANDROID_BUILD`).

---

## Como o app roda (stdlib `android.ofs`)

- **Entrada:** `android.ofs` exporta `vein ANativeActivity_onCreate(...)`, que o `NativeActivity` acha por `dlsym`. Ela preenche `activity->callbacks` com o endereço de veins OFS usando `fault_addr` (ver [LANGUAGE_REFERENCE.md](LANGUAGE_REFERENCE.md)).
- **Uma thread só:** tudo roda na thread principal do Android, sem pthread. O runtime magma não é thread-safe, então isso é intencional.
  - ciclo de vida: callbacks `onStart/Resume/Pause/Stop/Destroy`, `onWindowFocusChanged`, `onNativeWindow*`, `onInputQueue*`, `onContentRectChanged`, `onConfigurationChanged`, `onLowMemory`;
  - entrada: `AInputQueue_attachLooper` no looper da thread principal, com callback OFS;
  - frames: `AChoreographer_postFrameCallback` (API 24+), reagendado só quando há pedido (`android.request_frame()` ou `android.continuous = true`).
  - Trabalho longo deve ser dividido em pedaços, um por `EV_FRAME`.
- **Contrato:** o programa define `vein android_event(ev: stone) -> void` e lê o estado em `android.*`:

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
| `EV_TOUCH_DOWN/UP/MOVE/CANCEL`, `EV_POINTER_DOWN/UP` (outros dedos) | `touch_id`, `touch_x`, `touch_y`, `touch_count`; todos os dedos em `ptr_count`, `ptr_id[]`, `ptr_x[]`, `ptr_y[]` |
| `EV_BACK` (tecla voltar solta), `EV_KEY` | `key_code`, `key_action` |
| `EV_FOCUS`, `EV_CONFIG`, `EV_LOW_MEMORY` | `has_focus` |

Funções: `lock()`/`unlock_post()` (buffer RGBA_8888 do `ANativeWindow`: `buf_w`, `buf_h`, `buf_stride`, `buf_bits`), `fill_rect`, `blit(pixels, w, h)` (copia um buffer do `Canvas`), `request_frame()`, `finish()`, `set_orientation(ORIENT_REQ_*)` (JNI `setRequestedOrientation`), `call_activity_void_int(nome, valor)`, `internal_data_path()`, `external_data_path()`, `log(texto)`, `log_tag(tag, texto)`, `s32(v)`, `peek`/`poke`.

**Multitoque:** cada evento vale para o dedo `touch_id` (`AMotionEvent_getPointerId`). O primeiro dedo gera `EV_TOUCH_DOWN`, os seguintes `EV_POINTER_DOWN`; ao soltar, `EV_POINTER_UP` (ainda há dedos) ou `EV_TOUCH_UP` (o último). `EV_TOUCH_MOVE` chega uma vez por dedo.

**JNI (thread principal, pela tabela do `JNIEnv` com `fault_call`):** `jni_env()`, `jni_activity()`, `jni_fn(i)`, `jni_find_class(nome)`, `jni_string(s)`, `jni_to_text(jstring)`, `jni_delete(ref)`, `jni_check()` (limpa exceção), `jni_args(a, b, c, d)` (jvalue), `jni_static_bool(classe, método)`, `package_name()`, `start_intent(ação, uri)`.

- `android.is_external_storage_manager()` — `Environment.isExternalStorageManager()` (Android 11+; antes devolve true).
- `android.open_all_files_settings()` — abre `Settings.ACTION_MANAGE_APP_ALL_FILES_ACCESS_PERMISSION` com `package:<pacote>` (com alternativas). No `.oal`: `permission android.permission.MANAGE_EXTERNAL_STORAGE`.
- `android.keep_screen_on(bool)` — `FLAG_KEEP_SCREEN_ON`.
- `android.set_immersive(bool)` — tela cheia imersiva (esconde status e navegação; `setSystemUiVisibility` com `IMMERSIVE_STICKY` via JNI); reaplicada sozinha quando a janela recupera o foco.
- `android.set_cutout_short_edges(bool)` — Android 9+: desenhar também na faixa do recorte da câmera (`layoutInDisplayCutoutMode = SHORT_EDGES`), sem a faixa preta em paisagem.
- `android.jni_call_object(obj, nome, assinatura)` / `jni_call_void1(obj, nome, assinatura, arg)` — chamadas genéricas sem argumento / com 1 argumento.
- `android.asset_read("arquivo")` — lê `assets/` do APK (`AAssetManager`); tamanho em `android.asset_len`.
- `android.redraw_needed` — a janela foi criada/redimensionada/redesenhada: o programa deve copiar o quadro de novo (o backend da OAL só copia quando algo mudou).

**Áudio (`audio_android.ofs` + `wav.ofs`):** `audio.open()` abre um stream AAudio 44,1 kHz estéreo i16 com callback; `audio.play(canal, wav.load(...) ou wav.decode(ptr, n), loop)`, `stop`, `stop_all`, `set_volume(canal, %)`, `set_master(%)`, `playing(canal)`, `pause`/`resume`/`close`, `pump()` uma vez por quadro. O mixer roda na thread de áudio e só toca memória crua (nada de Array/strings); o PCM trocado é liberado com atraso seguro. `wav.ofs` decodifica PCM 8/16 bits e **Microsoft ADPCM** (saída idêntica à do ffmpeg). A CLI linka com `-laaudio`.

O botão voltar é entregue como `EV_BACK` e **não** fecha o app sozinho: chame `android.finish()` (a OAL faz isso por padrão).

---

### Entrada de texto pelo teclado do sistema (`android_input.ofs`)

`ainput.open(título, texto, multilinha)` abre um `android.app.Dialog` com um `EditText` criado por JNI (sem Java próprio): teclado padrão (Gboard), cursor, colar e autocompletar. Sem listeners: os botões são `ToggleButton` (alternam ao toque; `ainput.poll()` lê `isChecked()`), e voltar/tocar fora fecha o Dialog (`isShowing()`). `poll()` devolve 0 aberto, 1 OK, 2 cancelado; `ainput.text()` lê o texto. A OAL expõe isso como `oal.edit_open`.

## Limites atuais

- Só `arm64-v8a` (aarch64) e API 26+. Sem x86/armv7.
- Render em software (`ANativeWindow_lock`), sem OpenGL/Vulkan.
- Áudio: só WAV (PCM/MS‑ADPCM); sem Ogg/MP3; sem reamostragem de qualidade (passo inteiro 16.16).
- Sem `onSaveInstanceState` (o app salva o próprio estado no `on pause`).
- Os offsets de structs C (`ANativeActivity`, `ANativeWindow_Buffer`, `ARect`, `struct stat`/`statfs`, tabela do `JNIEnv`) estão fixos para aarch64.
- Inteiros C de 32 bits chegam como `stone` com o topo indefinido: use `android.s32()`.
- `fault_call` só passa inteiros e ponteiros (sem `float`/`double`).
- O `ofs/dist/magma.o` e o `ofscc` instalados ainda são de antes do alvo Android: até o próximo release, a CLI compila o `ofscc` do fonte.

## JNI genérico, WebView, HTTP (stdlib)

- **JNI genérico** (`android.ofs`): `jni_call(obj, nome, assinatura, a..f)` (o retorno vem da assinatura: `V`, `Z`, `I`, `J`, objeto), `jni_new(classe, assinatura, a..d)`, `jni_scall(classe, nome, assinatura, a..d)` (estático), `jni_static_int_field`, `jni_global`/`jni_delete_global`, `jni_new_byte_array`/`jni_get_bytes`/`jni_set_bytes`, `jni_new_object_array`/`jni_set_object_elem`; `android.jni_failed` diz se houve exceção Java. Só na thread principal.
- **`android_webview.ofs`**: `webview.open(base_url, html)` abre uma `WebView` (JS, DOM storage, acesso a arquivos, mídia sem gesto) num `android.app.Dialog` em tela cheia imersiva; `poll()` (o botão voltar fecha o Dialog), `eval(js)`, `title()`, `pause(on)`, `close()`. **Por que Dialog:** a `NativeActivity` toma a superfície da janela principal (`takeSurface`), então uma `View` adicionada à janela dela não é desenhada; o Dialog tem janela própria e recebe toque direto (sem passar pela `AInputQueue`). Sem subclasse Java: o shim entra como `<script>` no HTML (`loadDataWithBaseURL("file:///pasta/", html)`), e o retorno do JS para o OFS vai pelo `document.title` (o Chromium corta o título em 4096 caracteres e colapsa espaços/controles: mande em pedaços e com `encodeURIComponent`). Com a janela principal coberta o `EV_FRAME` continua chegando. `ofs_str_char_at` é O(n): em textos grandes leia bytes por ponteiro.
- **`android_http.ofs`**: `ahttp.open(url)`/`read(buf)`/`close()` e `get_text(url)` por `HttpURLConnection` (HTTPS com verificação de certificado e redirecionamentos; o OpenSSL do `tls.ofs` não é ligável num APK). Bloqueia até ~4 s por chamada na thread principal (use passos curtos por quadro; o `StrictMode` é liberado). Precisa de `permission INTERNET` no `.oal`. Aceita `file:///...` e caminhos absolutos.

## Modo motor: hospedar outro código/View na NativeActivity (stdlib `android.ofs`)

Para rodar um motor de jogo (SDL, Godot) **dentro** do app, sem instalar APK e sem Java no APK (`hasCode=false`):

- `release_window()`: chama `getWindow().takeSurface(null)` e `takeInputQueue(null)`. **Chame em `EV_CREATE`** (a janela só é anexada depois do `onResume`). A partir daí a `NativeActivity` não reserva mais a superfície nem a fila de entrada: `set_content_view(view)` (ou o `setContentView` de um adaptador Java) desenha normalmente e recebe toque/teclado pelo Java. O OFS deixa de receber `EV_WINDOW_*` e toques; o ciclo de vida continua chegando. Testado no Android 11 (Moto G8 Power). No `NativeActivity.java` do AOSP o `onCreate` faz `takeSurface(this)`/`takeInputQueue(this)` e só depois chama o `ANativeActivity_onCreate`; o `ViewRootImpl` lê esses callbacks ao anexar a janela, então o `null` vale.
- `dex_load_class(dex, pasta_libs, "pacote.Classe")`: `DexClassLoader` (pai: o class loader da activity) e `loadClass`; devolve referência **global** da classe ou 0 (motivo em `jni_error`). `dex` aceita vários arquivos separados por `:`. O `.dex` deve estar na área interna do app e, no Android 14+, ser somente leitura (`set_read_only(caminho)`). `pasta_libs` é o caminho de busca do `System.loadLibrary` desse loader.
- `jni_scall_cls(classe, nome, assinatura, a..d)`: método estático numa `jclass` já obtida (ex.: de `dex_load_class`).
- `jni_take_error()`: se há exceção Java pendente, guarda o `toString()` em `jni_error`, imprime a pilha no logcat (`System.err`) e limpa.
- `intent_extra(chave)`: `getIntent().getStringExtra(chave)` ("" se não houver).
- `self_intent(chaves, valores)` / `restart_process(chaves, valores)`: `Intent` para esta mesma activity com extras `String` e `FLAG_ACTIVITY_NEW_TASK|CLEAR_TASK`; `restart_process` faz `startActivity` e encerra o processo (`Process.killProcess(myPid)`): o sistema abre a activity num processo novo (~60 ms no Moto G8). Usado pelo M.O.O.N para entrar e sair do "modo motor" (um motor por processo).
