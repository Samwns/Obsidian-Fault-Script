# OFS Native Visual Library

OFS provides native graphical user interface layers authored purely in OFS using the `window` and `canvas` modules.

```ofs
attach {ui}

core main() {
    forge cv = canvas.create(96, 64)
    forge style = ui.dark()
    forge button = ui.rect(12, 14, 22, 16)

    window.create("OFS UI", 96, 64)
    while (window.poll()) {
        canvas.clear(cv, style.bg)
        ui.draw_button(cv, button, style, ui.contains(button, input.mouse_x(), input.mouse_y()), false)
        ui.draw_plus_icon(cv, button.x + 7, button.y + 4, style.text)
        ui.draw_text(cv, 12, 36, "OFS UI", style.text, 1)
        canvas.present(cv)
    }
    window.destroy()
    canvas.destroy(cv)
}
```

---

## Native Visual Layers

The OFS ecosystem offers two complementary approaches for native interface development:

1. **Immediate Mode (`canvas`, `window`, `ui`)**: Low-level procedural rendering directly onto raster pixel buffers with polling-based event loops.
2. **Declarative Mode (`oll`)**: Obsidian Layout Language (`.oll`), specifying component hierarchies, modern styling tokens, and automatic frame rendering.

---

## OLL — Obsidian Layout Language (Declarative UI)

The OLL DSL decouples interface layout from backend program logic:

```oll
window "System Panel" {
    width: 640
    height: 480
    bg_color: #1e1e2e
    padding: 24

    column {
        gap: 16
        header "Resource Monitor" { fg_color: #cba6f7 }

        card {
            bg_color: #313244
            border_color: #45475a
            border_width: 1
            padding: 16

            row {
                label "CPU" { fg_color: #89b4fa }
                badge "45%" { fg_color: #a6e3a1 }
            }
            progress { value: 45 bg_color: #45475a fg_color: #a6e3a1 }
        }

        row {
            gap: 12
            button "Refresh" { bg_color: #89b4fa fg_color: #11111b on_click: "refresh" }
            button "Close" { bg_color: #f38ba8 fg_color: #11111b on_click: "close" }
        }
    }
}
```

### Running OLL Applications

```bash
# Launch interactive native desktop application via X11 or Win32:
ofs-ui app.oll

# Or generate a static raster preview image in PPM format:
ofs ui app.oll -o preview.ppm
```

---

## Supported Capabilities

- `UiRect`: Layout bounding box geometry primitive.
- `UiStyle`: Typed styling container for color palettes, borders, and fills.
- Built-in default themes: `ui.dark()` and `ui.light()`.
- OLL: Declarative element hierarchy (`window`, `column`, `row`, `card`, `header`, `badge`, `progress`, `button`, `label`, `input`, `checkbox`).
- Window metrics: `auto_center` (automatic display centering) and `resizable` (window sizing lock).
- Full box model: `padding`, `gap`, and `margin`.
- Interactive control states: `idle`, `hover`, `active`, and `disabled`.
- Hexadecimal color values (`#1e1e2e`, `#89b4fa`).
- Core widgets and icons: buttons, checkboxes, sliders, progress bars, add, close, checkmark, and menu icons.
- Built-in 5x7 bitmap font rendering for alphanumeric and punctuation characters.
- Mouse pointer event handling via `input.mouse_x`, `input.mouse_y`, and `input.is_down`.
- Native platform drivers for Win32 on Windows and X11 on Linux.

## Pure-OFS graphics stack

Besides `canvas`/`window`/OLL, the stdlib ships a graphics stack written entirely in OFS, with no C runtime:

| Module | Role |
|---|---|
| `x11win.ofs` | Real X11 window, UTF-8 keyboard with dead keys, mouse, wheel, resize, cursors |
| `raster.ofs` | Antialiased vector drawing (paths, curves, rounded corners, strokes) |
| `ttf.ofs` | TrueType text (Inter, Noto, DejaVu) with accents and glyph cache |
| `icons.ofs` | Vector UI icons |

```ofs
attach {x11win}
attach {ttf}
attach {icons}

core main() {
    if (!x11win.open("App", 800, 500)) { return }
    forge font = ttf.load("/usr/share/fonts/rsms-inter-fonts/Inter-Regular.ttf")
    forge cv: Canvas = canvas.create(800, 500)
    while (x11win.is_open()) {
        forge ev = x11win.poll()
        while (ev != 0) { ev = x11win.poll() }
        canvas.clear(cv, 1776415)
        raster.fill_round_rect(cv, 24.0, 24.0, 320.0, 64.0, 14.0, 2829617)
        icons.draw(cv, icons.HOME, 40, 44, 24, 9090296)
        ttf.draw(cv, font, 18, 76, 63, "Hello, OFS!", 15264493)
        x11win.present(cv)
        x11win.sleep_ms(16)
    }
}
```

Note: `window.ofs` relies on the `magma.o` runtime, whose window is a stub; use `x11win.ofs` for real windows.
