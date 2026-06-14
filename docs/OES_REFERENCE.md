# OES Reference

OES is the visual and effect language of OFS.

```oes
token accent "#29e0bd"
.button
  background "$accent"
  radius "8px"
```

| Form | Result |
|---|---|
| `token name "value"` | CSS custom property |
| `selector` | Rule block |
| `property "value"` | Declaration |
| `media "query"` | Media group |
| `supports "query"` | Feature query |
| `layer "name"` | Cascade layer |
| `motion "name"` | Keyframes |
| `from`, `to`, `at50` | Animation frame |
| `raw "css"` | Explicit raw CSS |

Aliases include `bg`, `radius`, `columns`, `rows`, `size`, `weight`, `align`, and `justify`. `$name` references a token.

Compile with `ofs oes theme.oes -o theme.css`.
