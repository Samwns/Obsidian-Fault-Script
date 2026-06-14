# Obsidian Fault Script

OFS e uma linguagem compilada experimental com compilador self-hosted, runtime nativo, geracao LLVM IR e uma stack web propria com ODL para documentos e OES para efeitos.

## Links

- Documentacao: https://samwns.github.io/Obsidian-Fault-Script/
- Downloads e releases: https://github.com/Samwns/Obsidian-Fault-Script/releases
- Benchmark: https://samwns.github.io/Obsidian-Fault-Script/benchmark.html
- Web / ODL / OES: https://samwns.github.io/Obsidian-Fault-Script/web.html

## Exemplo rapido

```ofs
vein add(a: stone, b: stone) -> stone {
    return a + b
}

core main() {
    echo(add(20, 22))
}
```

```bash
ofs check app.ofs
ofs run app.ofs
ofs build app.ofs -o app
```

## Web

ODL e OES sao fontes do projeto. HTML e CSS sao artefatos gerados para hospedagem em navegadores, GitHub Pages, CDN ou servidor proprio.

```bash
ofs odl page.odl -o index.html
ofs oes site.oes -o site.css
```
