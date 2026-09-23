# OES Reference

OES (Obsidian Effect Scripts) is the styling, effects, and animation language of the OFS ecosystem.

You write `.oes` source files, `ofs oes` compiles them into standard CSS, and the browser consumes the resulting stylesheet. OES serves as the design system's source of truth.

---

## Minimal Example

```oes
gem accent "#a978ff"
gem surface "#120d1f"

paint .button
  fill "$accent"
  ink "#13071f"
  curve "8px"
  transition "transform .2s ease"

veil "(max-width: 700px)"
  paint .grid
    grid-cols "1fr"

pulse "appear"
  rise
    opacity "0"
  rest
    opacity "1"
```

---

## Typed Syntax and Explicit Scope

Along with the whitespace-sensitive syntax, typed tokens and scoped blocks can be declared with braces:

```oes
gem accent: Color = "#a978ff"
gem spacing: Size = "16px"

paint .button
  fill(accent)
  curve("8px")
  transition("transform .2s ease")

  veil("(max-width: 700px)") {
    grid-cols("1fr")
    space(spacing)
  }
end
```

---

## Syntax Rules

- The `gem` keyword declares reusable design tokens mapped to CSS custom properties (`--name: value;`).
- `gem name: Type = "value"` defines typed design tokens.
- The `paint` keyword opens a visual styling block for a selector.
- Style properties are indented within the `paint` block.
- Braces `{}` and `end` delimiters provide explicit block scoping.
- `$token` references are emitted as `var(--token)`.
- `veil` specifies responsive media queries (`@media`).
- `guard` specifies browser feature queries (`@supports`).
- `strata` defines cascade layers (`@layer`).
- `pulse` defines animation timeline keyframes (`@keyframes`).

---

## Core Directives and Properties

| Directive / Property | Equivalent CSS | Purpose |
|---|---|---|
| `gem name "value"` | `:root { --name: value; }` | Design token definition |
| `paint selector` | `selector { ... }` | Selector rule block |
| `fill "value"` | `background: value;` | Background fill |
| `ink "value"` | `color: value;` | Text foreground color |
| `curve "value"` | `border-radius: value;` | Corner border radius |
| `space "value"` | `gap: value;` | Layout gap / spacing |
| `edge "value"` | `border: value;` | Border definition |
| `flow "value"` | `display: value;` | Display and layout mode |
| `grid-cols "value"` | `grid-template-columns: value;` | Grid column template |
| `grid-rows "value"` | `grid-template-rows: value;` | Grid row template |
| `type-size "value"` | `font-size: value;` | Typography font size |
| `veil "query"` | `@media query { ... }` | Responsive breakpoint |
| `guard "query"` | `@supports query { ... }` | Feature detection query |
| `strata "name"` | `@layer name { ... }` | Cascade layer grouping |
| `pulse "name"` | `@keyframes name { ... }` | Animation keyframe definition |
| `rise` / `rest` | `from { ... }` / `to { ... }` | Initial and terminal animation frames |
| `at50` / `atNN` | `50% { ... }` | Percentage-based timeline waypoint |
| `raw "css"` | Unwrapped | Direct unescaped CSS emission |

---

## Backward Compatibility

OES accepts standard CSS property names whenever a specific alias is absent. Legacy keywords such as `token`, flat selectors, `media`, `supports`, `layer`, `motion`, `from`, and `to` are preserved for backward compatibility.

---

## Compilation

Compile an OES stylesheet to CSS:

```bash
ofs oes theme.oes -o public/theme.css
```
