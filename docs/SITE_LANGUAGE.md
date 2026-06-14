# Benitoite and Serpentine

Benitoite and Serpentine are the OFS web authoring layer.

The names follow the mineral theme of Obsidian Fault Script:
Benitoite is the structural markup layer, and Serpentine is the style layer that flows around it.

| Layer | Purpose | Browser artifact |
|---|---|---|
| Benitoite | document structure and components | HTML |
| Serpentine | tokens, rules, layout, motion | CSS |
| OFS functions | reusable components and page generation | static site files |
| webserver | local preview/server output | HTTP response |

## Minimal Page

```ofs
attach {benitoite}
attach {serpentine}

vein page() -> obsidian {
    forge head = benitoite.css("site.css")
    forge body = benitoite.h1("OFS Web")
    return benitoite.document("pt", "OFS Web", "site em OFS", head, body)
}
```

## Style

```ofs
attach {serpentine}

vein style() -> obsidian {
    forge s = ""
    s = s + serpentine.rule(":root", serpentine.var("accent", "#29e0bd"))
    s = s + serpentine.rule(".button", serpentine.prop("background", serpentine.use("accent")))
    return s
}
```

The official site in `ofs/site/` is the reference implementation.
