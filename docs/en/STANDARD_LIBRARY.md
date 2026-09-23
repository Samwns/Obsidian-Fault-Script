# OFS Standard Library Reference

The OFS standard library modules reside in `ofs/stdlib/`. Installed system modules are imported by name using the `attach` directive:

```ofs
attach {math}
attach {string}
attach {io}
```

During local development, specific files can be imported by relative or absolute file paths using the `F:` prefix:

```ofs
attach {F:../modules/my_module.ofs}
```

---

## 1. Core Modules

### `math.ofs`

Mathematical functions implemented purely in OFS:

- `square(x)`: Computes the square of a number (`x * x`).
- `cube(x)`: Computes the cube of a number (`x * x * x`).
- `sum_range(start, end)`: Calculates the inclusive range sum.
- `is_prime(n)`: Performs prime verification via odd divisor testing.
- `count_digits(n)`: Counts decimal digits in an integer.
- `sum_digits(n)`: Sums decimal digits.
- `reverse_number(n)`: Reverses decimal digits.
- `is_palindrome_number(n)`: Checks whether a number is numerically palindromic.

```ofs
attach {math}

core main() {
    echo(square(12))
    echo(is_prime(97))
}
```

### `string.ofs`

Text manipulation routines built atop runtime primitives:

- `repeat_str(text, count)`: Repeats a string by the specified count.
- `starts_with_char(text, char_code)`: Checks if the first byte matches the given ASCII code.
- `is_empty(text)`: Returns true if the string length is zero.

The native runtime additionally supplies `ofs_str_len`, `ofs_str_char_at`, `ofs_str_substr`, `ofs_str_concat`, `ofs_str_eq`, `ofs_str_contains`, and numeric formatting conversions.

### `io.ofs`

Terminal and console input/output:

- `prompt(message)`: Prints the message and reads a line from standard input.
- `print_separator(character, count)`: Emits a repeating delimiter line.
- `print_header(title)`: Formats and outputs a console section banner.

### `terminal_colors.ofs`

ANSI escape code constants and helpers for styled terminal output on compatible emulators.

---

## 2. Web Stack Modules

### `odl.ofs`

Programmatic API for constructing ODL / HTML semantic document trees:

- **Structural elements**: `document`, `meta`, `css`, `icon`, `script`, `script_module`.
- **Semantic containers**: `div`, `section`, `article`, `nav`, `header`, `main`, `footer`.
- **Typography and text**: `a`, `img`, `p`, `h1`, `h2`, `h3`, `strong`, `span`.
- **Forms and lists**: `ul`, `ol`, `li`, `form`, `label`, `input`, `button`.
- **Tables and code**: `table`, `table_id`, `code_block`.
- **Low-level nodes**: `tag`, `void_tag`, `attr`, `text`.

### `oes.ofs`

Programmatic API for generating OES / CSS styling rules:

- `prop(name, value)`: Emits individual style properties.
- `rule(selector, body)`: Declares selector rule blocks.
- `media(query, body)` / `supports(query, body)` / `layer(name, body)`: Responsive groupings and cascade layers.
- `keyframes(name, frames)` / `frame(step, body)`: Keyframe animation sequences.
- `var(name, value)` / `use(name)`: Token declarations and references.
- `transition(...)` / `animation(...)`: Motion primitives.

### `webserver.ofs`

Native HTTP server and protocol helpers:

- HTTP method constants (`GET`, `POST`, `PUT`, `DELETE`).
- HTTP status codes (200, 201, 400, 404, 500) and descriptions (`status_text`).
- MIME type resolution (`get_mime_type`) for HTML, CSS, JS, JSON, text, ODL, and OES.
- HTTP response encapsulation (`http_response`) with headers.
- Structured JSON fragment emission (`json_string`, `json_number`).
- Request logging utilities (`log_request`).
- Execution loops: `serve_once`, `serve_forever`, `serve_html_once`, `serve_html_forever`.

### `webui.ofs`

Reusable UI component library built on ODL and OES:

- Themes: `theme_dark()`, `theme_light()`.
- Layout components: `page`, `nav`, `hero`, `panel`, `grid`, `card`, `stack`.
- Controls: `button`.
- Server launcher: `serve`.

---

## 3. Native Visual Modules

### `canvas.ofs`

Low-level 2D raster pixel buffer rendering:

- Surface allocation and deallocation (`canvas.create`, `canvas.destroy`).
- Surface clearing (`canvas.clear`).
- Drawing primitives: individual pixels (`set_pixel`), rectangles, lines, and bitmap font text.
- Image export to binary PPM without third-party libraries.

### `window.ofs`

Native desktop window lifecycle and event loops:

- Window creation and teardown (`window.create`, `window.destroy`).
- Event polling (`window.poll`).
- Native platform drivers for X11 on Linux and Win32 on Windows.

### `ui.ofs`

Immediate-mode graphical user interface controls:

- Bounding box primitives (`UiRect`).
- Theme styling (`UiStyle`, `ui.dark()`, `ui.light()`).
- Basic widgets: buttons, labels, and text dimension measurement.

---

## 4. Systems and Low-Level Modules

### `bedrock.ofs`

Explicit memory management:

- Manual cell and buffer allocation (`bedrock_cell_new`, `bedrock_cell_drop`).
- Direct pointer dereferencing and arithmetic.
- Memory arenas and scoped regions.

### `bedrock_packet.ofs`

Binary buffer encoding and decoding for low-level network protocols.

### `rift.ofs`

Foreign Function Interface (FFI) bindings using the standard C ABI.

### `memory_modes.ofs`

Experimental interfaces for explicit memory tracking and access boundary validation.

---

## 5. Support and Compatibility Modules

### `test_lib.ofs`

Testing harness used for language verification and regression testing.

### `ofshtml.ofs`

Legacy HTML helpers maintained for backward compatibility. New web projects should use `odl.ofs`.
