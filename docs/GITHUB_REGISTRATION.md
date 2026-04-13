# Registrando OFS no GitHub

## GitHub Linguist (Detector de Linguagem)

O arquivo `.gitattributes` foi configurado para registrar `.ofs`:

```
*.ofs linguist-language=OFS
*.ofs linguist-detectable=true
```

## Próximos Passos

### 1. Fazer Pull Request ao Linguist

O GitHub Linguist é o projeto que detecta linguagens de programação. Para registrar OFS:

1. Fork: https://github.com/github/linguist
2. Clone localmente
3. Edite `languages.yml`:

```yaml
OFS:
  type: compiled
  color: "#2A0845"
  extensions:
  - .ofs
  tm_scope: source.ofs
  ace_mode: text
  codemirror_mode: null
  codemirror_mime_type: null
  aliases:
  - obsidian-fault-script
```

4. Abra PR com título: "Add OFS (Obsidian Fault Script) language support"

### 2. GitHub Topics (Etiquetas)

Adicione ao `package.json`:
```json
"keywords": [
  "programming-language",
  "compiler",
  "self-hosted",
  "obsidian",
  "ofs"
]
```

### 3. Awesome Lists (Descoberta)

Submeta em:
- https://github.com/aalhour/awesome-compilers
- https://github.com/sindresorhus/awesome (seção "Programming Languages")

**PR Template:**
```markdown
## Language: Obsidian Fault Script (OFS)

- **Type**: Compiled, Self-hosted
- **GitHub**: https://github.com/Samwns/Obsidian-Fault-Script
- **Description**: Statically-typed language designed to be simple at first and powerful as projects grow
- **Extension**: `.ofs`
- **Architecture**: Written in pure OFS (4500 LOC self-hosted compiler)
```

### 4. Documentação no README

Adicione badge:
```markdown
[![OFS Badge](https://img.shields.io/badge/Language-OFS-2A0845)](https://github.com/Samwns/Obsidian-Fault-Script)
```

## Verificação

Após registro no Linguist, o repositório deve mostrar:
- GitHub reconhece `.ofs` como linguagem própria
- Badge de estatísticas de linguagem aparece no repo

## Status

- ✅ `.gitattributes` configurado
- ⏳ PR ao Linguist (manual)
- ⏳ Awesome Lists (manual)
- ✅ Repositório removido de C++ e 100% OFS
