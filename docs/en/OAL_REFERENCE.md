# OAL Reference (v1)

OAL (Obsidian Android Language, `.oal`) describes what is **Android-specific** in an OFS app: the manifest, touch screens, styles, actions and lifecycle. Logic stays in `.ofs`. It is a sibling of ODL (document → HTML), OES (style → CSS) and OLL (desktop UI), and also plays the CSS/OES role on Android (`style` block).

```
app.oal ──oalc──► AndroidManifest.xml
                └► app_oal.ofs ──ofscc (aarch64)──► libapp.so ──► APK
```

- Compiler: `ofs/tools/oalc.ofs` (`OALC_INPUT`, `OALC_OUT_DIR`, `OALC_TARGET=android|desktop`).
- Runtime: `ofs/stdlib/oal_runtime.ofs` (neutral core) + `oal_android.ofs` (backend). You never import OAL itself: the generated file already links the runtime and `android.ofs`.
- Build: `ofs build --target android app.oal [-o app.apk] [--install] [--serial ID]` (see [ANDROID.md](ANDROID.md)).

---

## Example

```oal
// logic in OFS: both forms below have the same effect
use "library.ofs"
// attach {F:library.ofs}

app "M.O.O.N" "com.fixedseat.moon"
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
  bar "M.O.O.N"
    button.icon "plugins" -> open_plugins
  title "Library"
  text library.summary()
  button.primary "Import game" icon=import -> import_game
  list games from library.games
    item title=item.name sub=item.engine -> open_game(item)
    empty "No games yet."

screen plugins
  bar "Plugins"
    button.icon "back" -> @back
  list from library.plugins
    item title=item.name sub=item.state
    empty "No plugins registered."

on back -> go_back
on pause -> quick_save
```

```ofs
// library.ofs
namespace library {
    vein games_count() -> stone { return 0 }
    vein games_name(i: stone) -> obsidian { return "" }
    vein games_engine(i: stone) -> obsidian { return "" }
    vein summary() -> obsidian { return "0 games installed" }
}
vein open_plugins() -> void { oal.go(1) }
vein go_back() -> void { oal.back() }
```

---

## General rules

- One declaration per line; the hierarchy is given by **indentation** (2 spaces or a tab).
- Comments: `// ...` or `# ...` on their own line.
- Quoted text `"..."` (with `\"`, `\n`). Outside quotes, a value is an **OFS expression** (`library.summary()`).
- `key=value` without spaces around the `=` (`title=item.name`, `corner=bottom_left`).
- Portuguese words are accepted too: `tela`, `barra`, `botao`, `texto`, `titulo`, `lista`, `grade`, `cartao`, `linha`, `vazio`, `retrato`, `paisagem`, `estilo`, `usa`, `ao`.
- Errors are reported as `oalc: linha N: ...` and nothing is generated.

## Top level

| Declaration | Effect |
|---|---|
| `use "file.ofs"` | includes the logic (path relative to the `.oal`) |
| `attach {F:file.ofs}` | same as `use`, with the OFS `attach` syntax |
| `attach {module}` | OFS short form: `module.ofs` next to the `.oal`, else from the stdlib (hyphen becomes `_`) |
| `app "Name" "package"` + properties | generates `AndroidManifest.xml` |
| `style target` + properties | style (see below) |
| `screen name` + widgets | a screen; the first one is the start screen |
| `on event -> action` | lifecycle |

### `app`

| Property | Example | Manifest |
|---|---|---|
| `version "a.b.c"` | `version "0.1.0"` | `versionName`; `versionCode = a*10000 + b*100 + c` |
| `code N` | `code 7` | explicit `versionCode` |
| `icon "file.png"` | | `@mipmap/ic_launcher` (PNG only; the CLI copies it to `res/mipmap/`) |
| `orientation any\|portrait\|landscape` | | `screenOrientation` (`sensorLandscape` for landscape) |
| `permission a b ...` | `permission internet vibrate` | `android.permission.INTERNET`... (`storage` = read + write; a dotted name passes as is) |
| `fullscreen` | | `NoTitleBar.Fullscreen` theme |
| `assets "folder"` | `assets "../android/assets"` | the folder goes to the APK `assets/` (read with `android.asset_read`) |

The `.so` name (`android.app.lib_name`) is the last segment of the package. The activity always declares orientation/size `configChanges`: rotating does **not** recreate the app, it only fires `on rotate`.

## Widgets (`screen`)

| Widget | Form | Generates |
|---|---|---|
| `bar "Title"` | children: `button` | fixed bar at the top |
| `title "text"` | | title |
| `text "text"` / `text expr()` | | secondary text |
| `button "label" -> action` | `icon=name`, `.class` | button |
| `button.icon "name" -> action` | | icon-only button |
| `card "title" "sub" -> action` | `title=`, `sub=` | card |
| `list [id] [from source]` | children: `item`, `empty` | list |
| `grid [id] [from source] [columns=2] [landscape=4]` | children: `card`, `empty` | grid with columns per orientation |
| `row` | children | side-by-side widgets |
| `portrait` / `landscape` | children | shown only in that orientation |
| `fab "label" -> action` / `fab.icon "name" -> action` | `corner=` | floating button (layer on top, does not scroll) |
| `dpad -> action(item)` | `corner=` | virtual D-pad (floating layer) |
| `diamond -> action(item)` | `corner=`, `look=` | 4 action buttons in a diamond (floating layer); `item` = tapped button (`oal.SLOT_TOP/RIGHT/BOTTOM/LEFT`); `look` = `oal.PAD_KEYS` (keycaps), `PAD_XBOX` (A B X Y), `PAD_PLAY` (✕ ○ □ △ as vectors, no logos), `PAD_NINTENDO` (mirrored A B X Y) |
| `image expr` | `height=` (dp; 0 = proportional) | PNG image (path) fitted to the width |
| `canvas [full] draw=f touch=g` | `height=` | area drawn by the app (see below) |
| `tabs` | children: `tab "label" icon= selected=expr -> action` | tabs / segments side by side |
| `slider "label" value=expr min= max= -> action(item)` | `icon=` | slider; `item` = new value |
| `colorpicker value=expr -> action(item)` | | touch color picker: saturation/value square, hue bar, preview and hex; `value`/`item` = `0xRRGGBB`; the action runs while dragging |
| `toggle "label" value=expr -> action(item)` | `icon=` | switch; `item` = 1 (on) or 0 |
| `progress "label" value=expr` | | progress bar 0-100 |
| `field "hint" value=expr -> action` | `icon=` | text field (tap calls the action) |
| `keyboard -> action(item)` | | on-screen keyboard; `item` = Unicode code, 8 = backspace, 10 = enter. Hex layer (`oal.kb_layer = 2`): keys 0-9, A-F, backspace, `#` (abc) and ↵. |

**System text input (`oal.edit_open`).** `oal.edit_open(title, text, multiline) -> bool` opens an Android Dialog with a native `EditText` (stdlib `android_input.ofs`, JNI only): the device's default keyboard (Gboard), cursor, text scrolling, paste and autocomplete. OK/Cancel buttons (`oal.edit_ok_label`/`edit_cancel_label`). The result arrives in `on frame`: `oal.edit_state` 2 = confirmed, 3 = cancelled, with `oal.edit_text`; call `oal.edit_done()` when handled. Returns `false` off Android: use `keyboard`, which stays available as an alternative (e.g. gamepad).

**Color picker (`colorpicker`).** Reusable touch widget: drag in the square (saturation × value) and on the hue bar; the preview and the `#RRGGBB` code appear below. `value=` is the current color (`0xRRGGBB`, integer) and `-> action(item)` receives the new color on every move. Typed hex is up to the app (a `field` + `keyboard` calling the same action). Fixed height (~280 dp). Theme: `oal.set_palette_full(bg, bar, surface, surface_hi, text, muted, accent, button, button_pressed, border, badge)`; text over accent/button is chosen by contrast (`oal.contrast_ink`). A bar button with the `back` icon is placed on the **left side**, before the title.

| Widget | Form | Generates |
|---|---|---|
| `if expr` / `else` | children | conditional blocks (OFS expression) |

`corner`: `bottom_right` (`fab` default), `bottom_left` (`dpad` default), `top_right`, `top_left`, `bottom_row` (centered row at the bottom), `top_row`, `right_column`, `left_column` (centered columns) or an **OFS expression** (`corner=app.dpad_side()`). Bottom corners sit above the `bottom_row`.

Labels, titles, `empty` texts and icon names may be expressions: `button tr("play") icon=play`, `fab.icon player.auto_icon()`, `bar app.name()`. Class and icon together: `fab.ctrl.icon "save"`.

`text` wraps to several lines at the screen width (honors `\n`); the `empty` notice too.

### Navigation: bottom bar, drop-down menu, bottom sheet and long press

| Widget / attribute | Effect |
| --- | --- |
| `navbar` with `tab "label" icon=name selected=expr -> action` children | fixed bottom navigation bar (icon + label; the `selected` tab is highlighted in the theme accent). Content scrolls above it and `fab`s move up with it. |
| `popup menu -> close` / `popup sheet title=expr -> close`, children `item title=... icon=... [show=cond] -> action` | `menu`: drop-down under the bar, on the right (⋮); `sheet`: bottom sheet with a title. A dimmed scrim covers the screen; tapping outside calls the close action. Show it with `if app.menu_open()`. Class: `item.perigo`. |
| `card ... long=action(item)` / `item ... long=action(item)` | long press (~0.45 s) on the card/item calls the action (a normal tap still uses `-> action`). Moving the finger cancels it. |
| `bar "" logo=name` | an `icons.ofs` icon in place of the bar title text. |

Runtime: `oal.nav_begin/nav_tab/nav_end`, `oal.popup_begin(sheet, title, close)/popup_item/popup_end`, `oal.use_long(action)`, `oal.bar_logo(name)`. The screen stack and back button belong to the app (`on back`); `oal.reset_to(screen)` switches tabs clearing the stack. Top `fab` stacks (`top_left`/`top_right`) shrink so they do not cover the D-pad/diamond on the same side.

### Lists and grids with data (`from`)

`list games from library.games` calls user functions by name:

- `library.games_count() -> stone`;
- for each attribute `x=item.field` of the template, `library.games_field(i) -> obsidian`;
- without a template, `library.games_item(i)`.

The template is the `item` child (list) or `card` child (grid). `empty "text"` appears when the count is 0. In the action, `item` is the index (`-> open(item)`). Without `from`, the children are fixed items: `item "title" "sub" -> action`.

Extra attributes of `item`/`card` (literal or `item.field`):

| Attribute | Effect |
|---|---|
| `image=item.cover` | card cover (PNG, cropped to fill) or thumbnail on the left of the item |
| `icon=item.icon` | icon (name from `icons.ofs`); on a card without an image it becomes the "cover" |
| `badge=item.engine` / `badge_icon=item.icon` | badge (icon + short text) in the corner of the card or on the right of the item |

Images live in a cache (up to 40, shrunk to 480 px); `oal.image_forget(path)` drops a rewritten image.

After changing the data, call `oal.refresh()` to rebuild the screen.

### Icons

`button.icon "name"`, `fab.icon "name"` and `icon=name` use the vector icons from the stdlib `icons.ofs`: `back`, `forward`, `reload`, `home`, `plus`, `close`, `lock`, `info`, `star`, `menu` (three dots), `hamburger`, `search`, `globe`, `download`/`import`, `history`, `settings`/`config`, `profile`, `stop`, `file`, `bookmark`, `shield`, `plugin`/`plugins`, `save`, `folder`/`load`, `play`, `gamepad`.

## Actions (`->`)

| Action | Effect |
|---|---|
| `function` | calls `function()` from the `.ofs` |
| `function(args)` / `ns.function(item)` | calls with arguments (`item` = item index) |
| `@screen` | opens the screen (push) |
| `@back` | goes back one screen; on the first one, closes the app |
| `@quit` | closes the app |
| `@portrait` / `@landscape` / `@auto` | forces the orientation at runtime |

## Lifecycle (`on`)

`on start|back|pause|resume|rotate|low_memory|frame|key -> action`. `key` (key/button): the runtime only forwards control keys (D-pad, A/B/X/Y, START/SELECT, Enter, Space, Esc, M, H); the app reads `oal.key_code` (Android codes) and held-key repeats are filtered. The system handles the others (volume etc.). Keys from a **physical gamepad** (gamepad source) or its analog stick set `oal.pad_connected = true` (cleared on resume), to hide the virtual gamepad. `start` runs once, after the styles. `frame` runs every frame while `oal.set_animating(true)` (chunked work without freezing the screen, animations, player clock). Without `on back`, back pops the screen or closes the app. With `on back`, the app decides (call `oal.back()` or `oal.quit()`).

## Styles (`style`)

```oal
style button.primary       // class: used as button.primary / fab.primary / dpad.primary
  fill #8b6cff             // background
  ink #101020              // text and icon
  curve 24                 // radius in dp
  opacity 60               // 0-100
  pressed fill #5f45c8     // pressed state
  pressed ink #ffffff
```

Targets without a class change the type default: `style button`, `style card`, `style list`, `style bar` (bar color), `style screen` (`fill` = app background, `ink` = text). `style name` without a type creates the class `name`. Colors: `#rgb`, `#rrggbb` or decimal.

## Runtime API (for the `.ofs`)

| Call | Effect |
|---|---|
| `oal.refresh()` / `oal.redraw()` | rebuilds / redraws |
| `oal.go(n)`, `oal.back()`, `oal.quit()` | navigation (screen index in file order) |
| `oal.set_orientation(oal.ORIENT_PORTRAIT\|ORIENT_LANDSCAPE\|ORIENT_AUTO)` | forces the orientation (JNI `setRequestedOrientation`) |
| `oal.pad_label(style, slot)`, `oal.pad_shape(style, slot)` | text (A/B/Enter...) or icon (`icons.BTN_CROSS/CIRCLE/SQUARE/TRIANGLE`) of each `diamond` button |
| `oal.set_overlay_opacity(0..100)` | opacity of the floating layer (`fab`, `dpad`) |
| `oal.dpad_dir`, `oal.dpad_x`, `oal.dpad_y` | direction (`DIR_NONE/UP/RIGHT/DOWN/LEFT`) and vector (-100..100) of the D-pad |
| `oal.width`, `oal.height`, `oal.is_portrait()`, `oal.dp(v)` | size in px and dp → px conversion |
| `oal.log(text)` | logcat (tag `oal`) |
| `oal.toast(text)` | short message on top of everything (disappears by itself) |
| `oal.set_animating(bool)`, `oal.now_ms` | continuous frames (`on frame`) and clock in ms |
| `oal.reset_to(n)`, `oal.replace(n)`, `oal.stack_depth()` | navigation without pushing / emptying the stack |
| `oal.set_text_scale(%)`, `oal.set_overlay_scale(%)` | text size; `fab`/`dpad` size |
| `oal.set_palette(bg, bar, surface, surface_hi, text, muted, accent, accent_text)` | whole theme at runtime |
| `oal.image_get(path)`, `oal.image_forget(path)` | decoded PNG cache |
| `oal.cv()`, `oal.font()`, `oal.font_bold()`, `oal._fs(dp)` | for drawing inside a `canvas` |
| `oal.kb_shift`, `oal.kb_layer` | on-screen keyboard state |

### `canvas`

```oal
screen player
  canvas full draw=player.paint touch=player.touch
  fab.icon "menu" corner=bottom_row -> open_menu
```

The `.ofs` defines `paint(x, y, w, h)` (draws into `oal.cv()` in the given area) and `touch(kind, finger, x, y)` (`kind` 0 down, 1 move, 2 up; coordinates relative to the canvas). `full` fills the whole area below the bar (does not scroll; `fab`/`dpad` stay on top). Call `oal.redraw()` when the drawing changes. oalc generates `oal_app_paint`/`oal_app_touch`.

The D-pad calls the action on every direction change, with `item` = direction, and with `DIR_NONE` on release.

## Multitouch

Each finger is tracked separately (`android.touch_id`): you can hold the D-pad and tap a `fab` at the same time. Only the first finger scrolls the content. Buttons fire on release; slider and D-pad follow the drag; the `canvas` receives every finger.

## Rendering

Everything is drawn in software into an RGBA buffer (`canvas` + `raster` + `ttf` + `icons`) and copied to the `ANativeWindow`. The font is the system Roboto. Layout uses the neutral OLL core (`oll.new_node`/`add_child`/`layout_node`); sizes are in dp (density read from `AConfiguration`). The system bar area (`onContentRectChanged`) is respected in portrait and landscape. Content scrolls by dragging.

## Desktop

`ofs run app.oal` opens the app in a phone-sized X11 window (`oal_x11.ofs`). It is **experimental and unsupported**: it only serves to check the layout; OAL targets Android.

## Limits

- Text: system text field via `oal.edit_open` (`keyboard` + `field` as an alternative); no CJK fonts (the system Roboto has no Japanese).
- Gestures: only tap, drag (scroll) and long press (`long=`); no `swipe`/`pinch`. No declarative `intent` and `storage` (use `android.start_intent` and the `android.ofs` paths).
- Layers: content (scrolls), bar, floating and `canvas full`; one `canvas full` per screen.
- Software rendering: screens with many large images are slower the first time (PNG decoding in OFS).
