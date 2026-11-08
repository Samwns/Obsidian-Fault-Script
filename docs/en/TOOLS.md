# OFS tools

## `ofs doc`: documentation generated from the code

`ofs doc` reads `.ofs` files and generates a static HTML reference (like `rustdoc` or `godoc`): one page per module, with function cards, search and a coverage report. The tool is written in OFS (`ofs/tools/ofsdoc.ofs`) and works on any OFS project.

```bash
ofs doc                          # the repo's ofs/stdlib, ofs/tools and ofs/ofscc -> docs_api/
ofs doc my_folder -o docs_api/   # any folder (recursive) or a single .ofs
ofs doc --check                  # check only: prints coverage, generates no HTML
```

Options: `-o <dir>` (output, default `docs_api`), `--check`, `--docs <dir>` (the project's `.md` files; default `docs/` if present), `--title <name>`, `--repo <GitHub URL>` (links "Source" to the line on GitHub), `--lang pt|en|es` (UI language; default `OFS_LANG`). To view it: `cd docs_api && python3 -m http.server 8765`.

On the site, `build_site.ofs` runs the same generator on every build and merges the API with the Guide into a single navigation in `docs/ref/` (Reference).

### What each module shows

- `namespace`, `monolith` (fields and types), `impl` (methods), `vein`, `rift vein` (badge **exported · abi c**), `extern vein` (badge **external: libc/NDK**), `core` and constants/globals.
- Full signature: parameters, types and return type. `monolith` names in types are links.
- **Calls** (known functions used in the body) and **Used by** (the inverse, across all analyzed code).
- **See also**: sections of the `.md` files in `docs/` that mention the function or module.
- **Source**: link to the line on GitHub.
- Functions starting with `_` are internal: they sit in a collapsed section and do not count towards coverage.

### How to comment code

A declaration's comment is the contiguous `//` lines immediately above it (no blank line in between), or a `/** ... */` block. The first block of the file, separated by a blank line, describes the module.

```ofs
// hashmap: integer -> integer hash table (module comment).
//
//   forge m = hashmap.new(64)
//   hashmap.set(m, 42, 7)

namespace hashmap {
    // Creates a table with initial capacity `cap`.
    //
    // The first paragraph is the summary (shown in search and indexes).
    // Lines indented by 2+ spaces become a code block;
    // `name` becomes a link when it exists in the code.
    // @param cap initial capacity (a power of 2 is best)
    // @returns the table id
    // @example
    //   forge m = hashmap.new(64)
    vein new(cap: stone) -> stone {
        return _alloc(cap)
    }
}
```

| Tag | Effect |
|---|---|
| `@param name text` | description in the parameters table |
| `@returns text` | description of the return value |
| `@example` | the rest of the comment becomes a code block |
| `` `name` `` | link to the function, type or module, if it exists |
| `// ──── title ────` / `// ----` | decorative line, ignored |

Limitations: the `{` of `namespace`, `impl`, `monolith` and `vein` bodies must be on the same line as the declaration (the stdlib style).

### Coverage ("what is left to document")

The **Coverage** page (and the output of `ofs doc --check`) shows:

1. how many public functions (`vein`/`rift vein` without `_`) have a comment, per module and in total, with the percentage and the list of what is missing;
2. functions **cited in `docs/*.md` that do not exist in the code**: `module.function(` when `module` is a known module or namespace, and a name with `_` quoted in backticks with an opening parenthesis that exists nowhere. `fault_*` names, functions defined as examples in the `.md` files' own code blocks and example signatures (`` `f(x) -> T` ``) do not count.

`ofs doc --check` exits with code **1** if a cited function does not exist, for use in CI:

```bash
ofs doc --check || exit 1
```

### Test

`ofs/tests/ofsdoc/run.sh` compares the extractor with `ofs/tests/ofsdoc/expected.txt` (names, signatures, comments, calls/used by) and checks the exit code of `--check`.
