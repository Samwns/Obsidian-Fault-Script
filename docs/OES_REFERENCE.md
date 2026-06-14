# OES Reference

OES is the visual and effect language of OFS.

```oes
gem accent "#29e0bd"
paint .button
  fill "$accent"
  curve "8px"
```

| Form | Result |
|---|---|
| `gem name "value"` | CSS custom property |
| `paint selector` | Rule block |
| `property "value"` | Declaration |
| `veil "query"` | Media group |
| `guard "query"` | Feature query |
| `strata "name"` | Cascade layer |
| `pulse "name"` | Keyframes |
| `rise`, `rest`, `at50` | Animation frame |
| `raw "css"` | Explicit raw CSS |

OES keeps compatibility with CSS property names while adding its own easy vocabulary: `fill`, `ink`, `curve`, `space`, `edge`, `flow`, `grid-cols`, `grid-rows`, and `type-size`. `$name` references a token.

The older compatibility forms `token`, plain selectors, `media`, `supports`, `layer`, `motion`, `from`, and `to` remain accepted for migration.

Compile with `ofs oes theme.oes -o theme.css`.
