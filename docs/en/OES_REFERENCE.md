# OES Reference

OES, Obsidian Effect Scripts, is the visual and effect language of OFS.

You write `.oes`, `ofs oes` compiles it to CSS, and the browser receives a standard artifact. OES remains the design source of truth.

## Minimal Example

```oes
gem accent "#29e0bd"
paint .button
  fill "$accent"
  curve "8px"
veil "(max-width: 700px)"
  paint .grid
    grid-cols "1fr"
```

## Main Forms

| Form | Purpose |
|---|---|
| `gem name "value"` | Design token |
| `paint selector` | Visual rule block |
| `fill`, `ink`, `curve`, `space`, `edge` | Common visual aliases |
| `grid-cols`, `grid-rows`, `type-size` | Layout and typography aliases |
| `veil`, `guard`, `strata` | Responsive, feature, and layer groups |
| `pulse`, `rise`, `rest`, `at50` | Animation timeline |
| `raw "css"` | Explicit raw CSS |

Compile with:

```bash
ofs oes theme.oes -o theme.css
```
