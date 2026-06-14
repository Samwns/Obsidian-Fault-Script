# GitHub Linguist Contribution Plan

## Proposed Entry

```yaml
OFS:
  type: programming
  color: "#2A0845"
  extensions:
    - ".ofs"
  tm_scope: source.ofs
  ace_mode: text
  aliases:
    - obsidian-fault-script
```

The `language_id` must be generated inside the Linguist fork with
`script/update-ids`; it must not be invented manually.

## Grammar

https://github.com/Samwns/obsidian-fault-script-grammar

The grammar is MIT licensed and uses the `source.ofs` scope.

## Samples

Representative source should come from:

- `ofs/ofscc/` for the self-hosted compiler
- `src/site/src/` for ODL/OES site generation
- `ofs/examples/bedrock_gateway.ofs` for low-level features

## Acceptance Gate

GitHub Linguist currently requires at least 2,000 `.ofs` files indexed in the
last year, excluding forks, with reasonable distribution across public
repositories and users. A global PR should only be opened after that threshold
is met. Until then, `.gitattributes` provides repository-local recognition.

Search audit on June 14, 2026:

- `extension:ofs`: 364 indexed files
- `extension:ofs -user:Samwns`: 133 indexed files
