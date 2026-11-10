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

## `ofs fmt`: formatter

`ofs fmt` formats `.ofs` code (a tool written in OFS, `ofs/tools/ofsfmt.ofs`).

```bash
ofs fmt file.ofs        # rewrite in place
ofs fmt src/            # folder, recursive (.ofs only); no target = current folder
ofs fmt --check [paths] # CI: lists what would change and exits with code 1
ofs fmt --diff [paths]  # show the diff, write nothing
ofs fmt --stdin         # read stdin, write stdout (editors)
```

Rules: 4-space indent (continuation lines inside `(`/`[` or after an operator get +4), `{` on the same line, spaces around binary operators and after `,` and `:`, at most one blank line in a row, no trailing spaces, a single `\n` at end of file. The author's line breaks are kept, and comments, strings and `attach` lines are never changed.

Guarantees (tested in `ofs/tests/fmt/run.sh`): formatting twice gives the same file, and the `ofscc` IR of the original and the formatted file is identical (the program does not change). Pending: automatic wrapping above 100 columns and sorting of `attach`.

In VS Code the extension registers `ofs fmt --stdin` as the formatter; to format on save: `"[ofs]": { "editor.formatOnSave": true }`.

## `ofs test`, `ofs twin`, `ofs polish`

```bash
ofs test [--run] [--twin] [folder]   # default: ./tests
ofs twin file.ofs
ofs polish [--check] [--diff] [targets]
```

- `ofs test`: finds `*.ofs` files with a `.expected` next to them, builds and runs each one (native; `--run` uses the interpreter; `--twin` also compares both modes) and diffs the output against `.expected`. Prints `ok`/`FAIL` per file and a summary; exits 1 if any fails. `.ofs` files without `.expected` are skipped.
- `ofs twin`: runs the program interpreted and native in temporary directories and compares stdout, exit code and normalized stderr. Prints `iguais` (same, exit 0) or the first difference (exit 1). `OFS_TEST_TIMEOUT` sets the per-run limit (default 30 s).
- `ofs polish`: alias of `ofs fmt`, same options.

## `ofs quarry`: packages

`ofs quarry` is the package manager (the quarry the libraries are extracted from). It is written in OFS (`ofs/tools/quarry.ofs` + `quarry_json.ofs`) and compiled and cached by the wrapper like `ofs fmt`/`ofs doc`.

```bash
ofs quarry add granito            # range ^<latest>; also granito@1.2.3, granito@^1.2, granito@beta (channel)
ofs quarry remove granito         # drop it from the project and delete orphaned dependencies
ofs quarry list                   # what is installed, with versions
ofs quarry search text            # search names, descriptions and tags in the registry
ofs quarry update [granito]       # raise versions within the ofs.claim ranges
ofs quarry install                # reproduce the project: install what ofs.lode says, verifying sha256
ofs quarry publish [--out dist] [--origin <url>] [--deposit registry.json]
```

**Where it installs:** `$OFS_LIB_HOME`, or `~/.ofs/libs`, which `attach` already searches. Each package goes to `~/.ofs/libs/<package>/` (the `.tar.gz` contents plus `.assay`, with the verified version and hash) and gets a one-line "vein" `~/.ofs/libs/<module>.ofs` (`attach {F:<package>/<main>}`), so `attach {granito}` finds it with no configuration, in both `ofs build` and `ofs run` (hyphens become underscores: `terminal-colors` → `attach {terminal_colors}`). Installation is per user, not per project: projects that need different versions of the same package switch with `ofs quarry install`. If the stdlib has a module with the same name (`core`, `fmt`, `io`...), it wins the search; `quarry` warns and suggests `attach {F:<package>/<main>}`.

**Where it downloads from:** the registry in `OFS_QUARRY_REGISTRY` (or `--registry`): an `http(s)://` URL, `file://` or a path (a `.json` file or a folder with `registry.json`). Default: the repository's `src/packages/registry.json` on GitHub (raw). Each version points to a GitHub release `quarry-<package>-<version>` (published by the `publish-libraries.yml` workflow, with no npm and no `NPM_TOKEN`); if that fails, `quarry` tries `<registry folder>/dist/<file>`. With a local registry the file next to it comes first, so no network is needed. Downloads use the system `curl`: the stdlib `tls.ofs` does not follow redirects (GitHub releases redirect) and needs `-lssl`. Extraction uses `tar`. The sha256 is computed in OFS (`sha256.ofs`).

### Manifest `ofs.claim` and lock `ofs.lode`

The manifest is a key = value file (ODL describes HTML documents, so it does not fit this job). Comments start with `#`:

```ini
name = my-app
version = 0.1.0
description = Example
main = libs/my_app.ofs           # for publish; default libs/<name>.ofs
# optional for publish: author, license, tags = a, b, channel = beta, include = libs, assets

[deps]
granito = ^1.2      # >=1.2.0 <2.0.0  (^0.3 = >=0.3.0 <0.4.0)
basalto = ~1.0.3    # >=1.0.3 <1.1.0
xisto = 1.x         # any 1.*.*
ardosia = 2.0.1     # exact (also >=1.0 <2.0, *)
```

`ofs.lode` is generated, with one section per package (direct and transitive): exact `version`, `sha256` of the `.tar.gz`, the `url` it came from, `main` and `deps`. When the lock covers the manifest, `install` uses only the lock (no registry needed) and **refuses** a package whose sha256 does not match. It also refuses when the registry has a different hash than the lock for the locked version. Registry versions without `sha256` cannot be installed.

**Dependencies between packages:** a package published with `[deps]` in its `ofs.claim` carries those ranges into its registry entry. For each package, resolution picks the highest version that satisfies every requested range (the locked one, if it still fits) and then follows that version's dependencies. If no version satisfies every range, that is a conflict: nothing is written, and the message lists who asked for what.

**Publishing:** run `ofs quarry publish` in the package folder. It builds a reproducible `dist/<name>-<version>.tar.gz` (sorted by name, with zeroed dates and owners and `gzip -n`, so the same source gives the same sha256). The archive holds `include` (default: the folder of `main`), the manifest, `README.md` and `LICENSE`. The command prints the registry entry with its hash and uploads nothing. `--origin` sets the URL (`{name}`, `{version}`, `{file}` are replaced; without it the URL is the file name, relative to the registry). `--deposit registry.json` writes the entry into the registry. `src/packages/build_packages.sh` does this for the 19 libraries in `src/packages/src/`, which still use the old `ofspkg.json` (also accepted).

Test: `ofs/tests/quarry/run.sh` (temporary local registry, temporary `HOME`/`OFS_LIB_HOME`). Pending: package signatures (only the registry/lock sha256 for now), per-project installs, and uploading the release from `publish`.
