# OAL Reference (v1)

OAL (Obsidian Android Language, `.oal`) describes what is **Android-specific** in an OFS app: the manifest, touch screens, styles, actions and lifecycle. Logic stays in `.ofs`. It is a sibling of ODL (document → HTML), OES (style → CSS) and OLL (desktop UI), and plays the CSS/OES role on Android (`style` blocks).

```
app.oal ──oalc──► AndroidManifest.xml
                └► app_oal.ofs ──ofscc (aarch64)──► libapp.so ──► APK
```

- Compiler: `ofs/tools/oalc.ofs` (`OALC_INPUT`, `OALC_OUT_DIR`, `OALC_TARGET=android|desktop`).
- Runtime: `ofs/stdlib/oal_runtime.ofs` (neutral core) + `oal_android.ofs` (backend). You never import OAL itself: the generated file links the runtime and `android.ofs`.
- Build: `ofs build --target android app.oal [-o app.apk] [--install] [--serial ID]` (see [ANDROID.md](ANDROID.md)).

---

## Example

```oal
// logic in OFS: both forms below are equivalent
use "library.ofs"
// attach {F:library.ofs}

app "teste" "com.teste.teste"
  version "0.1.0"
  orientation any
  permission internet

style button.primary
  fill #8b6cff
  ink #101020
  curve 24
  pressed fill #5f45c8
  pressed ink #ffffff

screen library
  bar "teste"
    button.icon "plugins" -> open_plugins
  title "Library"
  text library.summary()
  button.primary "Import game" icon=import -> import_game
  list games from library.games
    item title=item.name sub=item.engine -> open_game(item)
    empty "No games yet."

on back -> go_back
on pause -> quick_save
```

---

## General rules

- One declaration per line; hierarchy by **indentation** (2 spaces or a tab). Comments: `// ...` or `# ...` on their own line.
- Quoted text `"..."` (with `\"`, `\n`); unquoted values are **OFS expressions** (`library.summary()`).
- `key=value` without spaces around `=`.
- Portuguese keywords are accepted too (`tela`, `botao`, `lista`, ...).
- Errors are reported as `oalc: linha N: ...` and nothing is generated.

## Top level

| Declaration | Effect |
|---|---|
| `use "file.ofs"` | includes the logic (path relative to the `.oal`) |
| `attach {F:file.ofs}` | same as `use`, with OFS `attach` syntax |
| `attach {module}` | OFS short form: `module.ofs` next to the `.oal`, else from the stdlib |
| `app "Name" "package"` + properties | `AndroidManifest.xml` |
| `style target` + properties | style |
| `screen name` + widgets | a screen; the first one is the start screen |
| `on event -> action` | lifecycle |

`app` properties: `version "a.b.c"` (versionCode = a*10000+b*100+c), `code N`, `icon "file.png"` (PNG only in v0), `orientation any|portrait|landscape`, `permission a b ...` (`internet` → `android.permission.INTERNET`; `storage` = read + write), `fullscreen`. The `.so` name is the last package segment. Rotation never recreates the activity; it only fires `on rotate`.

## Widgets

| Widget | Form |
|---|---|
| `bar "Title"` | children: `button` |
| `title "text"`, `text "text"` / `text expr()` | |
| `button "label" -> action` | `icon=name`, `.class` |
| `button.icon "name" -> action` | icon-only button |
| `card "title" "sub" -> action` | `title=`, `sub=` |
| `list [id] [from source]` | children: `item`, `empty` |
| `grid [id] [from source] [columns=2] [landscape=4]` | children: `card`, `empty` |
| `row` | side-by-side children |
| `portrait` / `landscape` | children shown only in that orientation |
| `fab "label" -> action` / `fab.icon "name" -> action` | floating button (`corner=`) |
| `dpad -> action(item)` | virtual D-pad (`corner=`) |
| `image expr` | PNG image fitted to the width (`height=` dp; 0 = proportional) |
| `canvas [full] draw=f touch=g` | area drawn by the app: `f(x, y, w, h)` paints into `oal.cv()`, `g(kind, finger, x, y)` gets touches (0 down, 1 move, 2 up); `full` fills the screen below the bar |
| `tabs` | children `tab "label" icon= selected=expr -> action` (tabs / segmented control) |
| `slider "label" value=expr min= max= -> action(item)` | `item` = new value |
| `toggle "label" value=expr -> action(item)` | `item` = 1 (on) or 0 |
| `progress "label" value=expr` | progress bar 0-100 |
| `field "hint" value=expr -> action` | text field look; tap calls the action |
| `keyboard -> action(item)` | on-screen keyboard; `item` = Unicode code, 8 = backspace, 10 = enter |
| `if expr` / `else` | conditional blocks (OFS expression) |

`corner`: `bottom_right` (fab default), `bottom_left` (dpad default), `top_right`, `top_left`, `bottom_row`, `top_row`, `right_column`, `left_column`, or an OFS expression. Bottom corners sit above the `bottom_row`.

Labels, titles, `empty` texts and icon names may be expressions (`button tr("play") icon=play`, `fab.icon player.auto_icon()`); style class plus icon: `fab.ctrl.icon "save"`. `text` and `empty` wrap to several lines (and honor `\n`).

`item`/`card` extras: `image=item.cover` (card cover or item thumbnail), `icon=item.icon`, `badge=item.engine` + `badge_icon=item.icon` (small pill). Images are decoded once and cached (up to 40, shrunk to 480 px); `oal.image_forget(path)` drops a rewritten file.

**Data (`from`):** `list games from library.games` calls `library.games_count()` and, for each `x=item.field` in the template, `library.games_field(i)` (without a template: `library.games_item(i)`). In actions `item` is the index. Call `oal.refresh()` after data changes.

**Icons** (stdlib `icons.ofs`): `back`, `forward`, `reload`, `home`, `plus`, `close`, `lock`, `info`, `star`, `menu`, `hamburger`, `search`, `globe`, `download`/`import`, `history`, `settings`/`config`, `profile`, `stop`, `file`, `bookmark`, `shield`, `plugin`/`plugins`, `save`, `folder`/`load`, `play`, `gamepad`.

## Actions

`function`, `function(args)` / `ns.function(item)`, `@screen` (push), `@back` (pop; closes the app on the first screen), `@quit`, `@portrait` / `@landscape` / `@auto` (force orientation at runtime).

## Lifecycle

`on start|back|pause|resume|rotate|low_memory|frame -> action`. `start` runs once after styles; `frame` runs every frame while `oal.set_animating(true)` (chunked work, animation). Without `on back`, back pops the screen or closes the app.

**Multitouch:** each finger is tracked separately (D-pad held while tapping a `fab`); only the first finger scrolls; `canvas` receives every finger.

## Styles

```oal
style button.primary
  fill #8b6cff        // background
  ink #101020         // text and icon
  curve 24            // radius in dp
  opacity 60          // 0-100
  pressed fill #5f45c8
  pressed ink #ffffff
```

Targets without a class change the type default: `style button`, `style card`, `style list`, `style bar`, `style screen`. Colors: `#rgb`, `#rrggbb` or decimal.

## Runtime API (from `.ofs`)

`oal.refresh()`, `oal.redraw()`, `oal.go(n)`, `oal.back()`, `oal.quit()`, `oal.set_orientation(oal.ORIENT_PORTRAIT|ORIENT_LANDSCAPE|ORIENT_AUTO)`, `oal.set_overlay_opacity(0..100)`, `oal.dpad_dir` / `dpad_x` / `dpad_y`, `oal.width`, `oal.height`, `oal.is_portrait()`, `oal.dp(v)`, `oal.log(text)`, `oal.toast(text)`, `oal.set_animating(bool)` + `oal.now_ms`, `oal.reset_to(n)` / `oal.replace(n)` / `oal.stack_depth()`, `oal.set_text_scale(%)`, `oal.set_overlay_scale(%)`, `oal.set_palette(bg, bar, surface, surface_hi, fg, muted, accent, accent_fg)`, `oal.image_get` / `oal.image_forget`, `oal.cv()` / `oal.font()` / `oal.font_bold()` / `oal._fs(dp)` (for canvas drawing). `app` also accepts `assets "dir"` (copied to the APK `assets/`, read with `android.asset_read`); `icon "x.png"` is copied to `res/mipmap/`.

## Rendering

Software rendering into an RGBA buffer (`canvas` + `raster` + `ttf` + `icons`), copied to the `ANativeWindow`; system Roboto font; layout through the neutral OLL core; sizes in dp; system bar insets respected; drag to scroll.

## Desktop

`ofs run app.oal` opens a phone-sized X11 window (`oal_x11.ofs`). **Experimental and unsupported**; OAL targets Android.

## Limits

No system text input (use `keyboard` + `field`); no CJK font (system Roboto); no gestures (`long_press`, `swipe`, `pinch`) or declarative `intent`/`storage` (use `android.start_intent` and `android.ofs` paths); one `canvas full` per screen; software rendering.
