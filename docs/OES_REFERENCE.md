# OES Reference / Referencia OES

OES is the visual and effect language of OFS.

OES e a linguagem visual e de efeitos da OFS. Voce escreve `.oes`, a ferramenta OFS compila para CSS, e o navegador recebe um artefato padrao.

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

OES preserva compatibilidade com propriedades CSS, mas adiciona vocabulario proprio e mais facil. Use `gem` para tokens, `paint` para blocos visuais, `veil` para responsividade e `pulse` para animacao.

The older compatibility forms `token`, plain selectors, `media`, `supports`, `layer`, `motion`, `from`, and `to` remain accepted for migration.

As formas antigas continuam aceitas para migracao gradual de CSS existente.

Compile with / Compile com:

```bash
ofs oes theme.oes -o theme.css
```
