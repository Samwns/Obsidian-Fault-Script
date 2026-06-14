# ODL and OES

ODL means **Obsidian Document Language**. It is the structural document layer.

OES means **Obsidian Effect Scripts**. It defines visual rules, responsive behavior, transitions, and animation.

| Layer | Purpose | Browser artifact |
|---|---|---|
| ODL | document structure and components | HTML |
| OES | tokens, rules, layout, motion | CSS |
| OFS functions | reusable components and page generation | static site files |
| webserver | local preview/server output | HTTP response |

## Minimal Page

```ofs
attach {odl}
attach {oes}

vein page() -> obsidian {
    forge head = odl.css("site.css")
    forge body = odl.h1("OFS Web")
    return odl.document("pt", "OFS Web", "site em OFS", head, body)
}
```

## Style

```ofs
attach {oes}

vein style() -> obsidian {
    forge s = ""
    s = s + oes.rule(":root", oes.var("accent", "#29e0bd"))
    s = s + oes.rule(".button", oes.prop("background", oes.use("accent")))
    return s
}
```

The official site in `website/src/` is the reference implementation. Generated files live in `website/public/`.
