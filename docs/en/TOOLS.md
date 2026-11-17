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
- **Stratum and permissions** (A5.4 + Idea 4): `stratum ...`, `claim ...` and `caprock ...` badges at the top of the module page, read from the file header (also in the `//! claim` form); `stratum abyss` gets the "asks for everything" badge. In `--dump`, one `modulo|stratum ...|claim ...` line per module that declares something.
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

## `ofs assay` and `ofs build --release`: contracts

```bash
ofs assay file.ofs             # boundary cases for veins with contracts
ofs build --release app.ofs    # binary without run-time contract checks
```

- `ofs assay`: for each vein with `bearing`/`settle` (or a `mineral` parameter) and only integer parameters, calls the vein at the rule boundaries (the boundary value and ±1; a parameter with no bound uses -1, 0 and 1; up to 64 cases per vein), interpreted. Cases outside the `bearing`/`grain` are listed with `--` and do not run; the others print `ok` after `settle` checks the result. The first violation stops with `ofs: [CONTRATO VIOLADO ...]` on stderr and exit code 1. With no eligible vein it says so and exits 0. The program's `core main` does not run.
- Run-time contract checks: on for `ofs run` and default `ofs build`; `ofs build --release` (or `OFS_CONTRACTS=0` in any mode) removes them. The static check (`[ERRO DE CONTRATO]`) always applies. Rules in `docs/en/LANGUAGE_REFERENCE.md`, section 4.
- `ofs doc` shows contracts: a vein page gets a **Contracts** table (`bearing`/`settle`) and each `mineral` is listed among the types, with its base in the signature and its `grain` rules in the same table.

## `ofs check`: module stratum and permissions

After checking, `ofs check` lists every module that declares or uses low-level code (the main program and each `attach`), with its stratum and claims (rule in `docs/en/LANGUAGE_REFERENCE.md`, section 12, "Stratum and per-module permissions"):

```
Estrato e permissoes (stratum / claim) por modulo:
  app.ofs (principal): stratum fault (padrao), claim abyss
  stdlib/fs.ofs: stratum fault (padrao), claim rift
  plugins/raw.ofs: stratum fault (padrao), sem claim  [usa sem claim: rift]
  caprock do principal: os anexados podem pedir rift, abyss
```

`[usa sem claim: ...]` is what triggers `[AVISO DE PERMISSAO]` (an error with `OFS_STRICT_FAULT=1`); `[claim sem uso: ...]` is a claim the module does not use. It is not printed with `OFS_PROGRESS=json` (the extension's JSON does not change).

## `ofs refract`: live editing (Fault Refract)

`ofs refract` runs the program interpreted and applies every saved edit **without restarting and without losing state**: the edited vein replaces the old one on its next call, and globals, arrays and objects keep the values they already had.

```bash
ofs refract app.ofs            # run and watch app.ofs and its local attaches
ofs refract --quiet app.ofs    # no swap notices (errors and refusals are still shown)
ofs refract ofs/examples/refract_demo.ofs   # example: edit desenhar/atualizar and save
```

- **Watches** the main file and local `attach` files (the installed stdlib and the runtime are excluded). The check runs between vein calls, every `OFS_REFRACT_PERIOD` ms (default 200), and only recompiles after the content stays the same for one period (the editor may still be writing).
- **Unsaved edits (lava).** In VS Code the change applies **without saving**: with the extension (setting `ofs.refract.liveEdits`, on by default), ~250 ms after the last keystroke the editor content goes to a draft, the *lava*, at `${XDG_RUNTIME_DIR:-/tmp}/ofs-refract-<uid>/lava/<absolute path of the file without the leading slash>` (on Windows, `%TEMP%\ofs-refract\lava\C\...`). For each watched file, Refract uses whichever changed last: the lava or the file on disk. Saving, closing the file or undoing everything deletes the lava and Refract goes back to the disk; a lava that already existed when Refract started only counts if it changes. Nothing to configure: Refract creates the folder (0700, owned by the user) and leaves `pulse/<pid>` while it runs, and the extension only writes the lava if there is a `pulse` of a live process (with no Refract running, nothing is written). `OFS_REFRACT_DRAFT=0` turns the lava off (in Refract; in the extension, `ofs.refract.liveEdits: false`). Outside VS Code you have to **save** (or turn on the editor's auto save: for Refract it is the same as saving).
- **Half-typed code.** With the source coming from the lava, the code goes through invalid states while you type: the running version keeps going and the error **only shows if the content stays still with the error for ~1 s**, once (the same error is not repeated; `erro de compilacao no rascunho do editor (acima); a versao em execucao continua`). When it compiles again, the swap is normal. The usual stability rule (recompile only after the content stays the same for a period) still applies.
- **Compile errors** (syntax, type, attach): the diagnostic goes to stderr and the running version keeps going. Fix it and save again.
- **Only what changed** is swapped: each vein is compared with the running version. New veins and new globals are added (a new global starts with its initial value; existing globals keep their value, even if the initializer changes in the text).
- **Native veins** (crystallized or using `quartz`) are recompiled into a new `.so` in the background, together with the native veins that call them; until the `.so` is ready, the new version runs interpreted. Veins with `quartz` are compiled before the swap (they cannot run interpreted).
- **A compatible signature is accepted** even while the vein is in use (wave 4): old parameters equal and first, a new trailing parameter **with a default value** (`vein versao(k: stone = 7)`), the same return, a return that used to be `void`, a narrow integer where it was `stone` or `f32` where it was `crystal`. Old calls get the default. Message: `a assinatura de 'versao' mudou de ()->stone para (stone)->stone: compativel (1 parametro novo com padrao: k)`.
- **A `monolith` with live instances migrates** (wave 4): a new field gets the type's default, a removed field disappears, the order may change and a numeric field may change type with its value converted (`stone`↔`crystal`↔narrow integers↔`bool`). Instances are rewritten in place (pointers to them stay valid): under Refract every instance is allocated with 64 spare bytes. Message: `monolith 'Ponto' migrado ao vivo (2 instancias); campo(s) novo(s) com o valor padrao: z; convertido(s): y stone->crystal`.
- **Refusals** (the program keeps the old version and the message says what needs a restart): an incompatible signature change of a vein still called by a running vein (or handed to C as a callback, or used as a value), a removed vein still called by a running vein, a live `monolith` field changed to a type that does not convert (e.g. `stone` → `obsidian`) or growing beyond the slack, a global that changed type, an `extern` with a new signature, `strata` variants removed or reordered.
- **Loops switch in the middle (OSR, wave 4).** A running loop in an edited vein (including the `while` in `main`, which never returns) moves to the new version at its next header, with the local variables as they are; the rest of the vein after the loop is new too. A new variable before the loop gets its initial value at switch time, a removed one disappears; if one changed type, or the loop is inside `cycle`/`match`/`bedrock`, the loop finishes on the old version (message `o laco de 'main' segue na versao antiga: ...`). A loop that went native through OSR comes back to the interpreter at its header when the file changes (the native loop asks every 65536 iterations) and later goes native again with the new version. Outside a loop, the running vein finishes on the old version and the new one applies on the next call.

stderr messages (short, prefixed with `ofs refract:`): `observando app.ofs; salve (ou so edite, no VS Code) para refratar`, `2 veins refratadas (desenhar, atualizar)`, `1 vein nova (pulsar)`, `1 global novo (bonus)`, `recristalizando em segundo plano: desenhar`, `desenhar: cristalizada(s) de novo (.so novo)`, `laco de 'main' continua na versao nova (OSR)`, `variavel(is) nova(s) antes do laco, com o valor inicial: extra`, `erro de compilacao (acima); a versao em execucao continua`, `troca recusada: ...`.

Variables: `OFS_REFRACT_PERIOD` (ms, 10 to 60000), `OFS_REFRACT_DRAFT=0` (ignore the lava), `OFS_REFRACT_QUIET=1` (same as `--quiet`), plus the crystallization ones (`OFS_CRYSTALLIZE`, `OFS_CRYSTALLIZE_THRESHOLD`, `OFS_OSR=0` to turn native OSR off, `OFS_QUARTZ_CACHE`, `OFS_VERBOSE=1`). Without the wrapper: `OFS_REFRACT=1 OFSCC_MODE=run OFSCC_INPUT=app.ofs ofscc`. Available on Linux and macOS (the native part needs an `ofscc` linked with `-rdynamic`); for the phone, see `ofs refract --remote` below.

Test: `ofs/tests/refract/run.sh` (interpreted swap, crystallized vein swap, compile error, refused signature, new vein and global, `--quiet` wrapper, contracts, geode; wave 4: OSR in the `main` loop, OSR of a native loop, `monolith` migration, accepted compatible signature; lava: a draft swaps the vein with the disk untouched, a draft with an error neither swaps nor speaks before ~1 s, deleting the draft goes back to the disk, `OFS_REFRACT_DRAFT=0`, and the PC side of `--remote` without a device).

### `ofs refract --remote`: on the phone (L3)

```bash
ofs refract --remote app.oal               # or an app.ofs with android_event
ofs refract --remote app.oal --serial <id>
ofs build --target android app.oal --refract --install   # just the debug APK
```

The app runs **natively on the device** and the PC sends only the edited veins: the `--refract` build adds one dispatch cell per vein and the `geophone` agent (abstract socket `ofs-refract.<package>`, reached through `adb forward`); on every save the PC diffs vein by vein against the manifest the app sent, applies the safety rules, compiles the patch with the NDK clang (`-shared -fPIC`, linked against the app's lib) and sends it; the app `dlopen`s it, swaps the cells between two main-thread events and replies. Same process, globals at the same address. Messages (prefix `refract:`): `conectado a <pacote> (pid N, geracao G, ...)`, `alteradas: painel.texto`, `novas: painel.rotulo`, `religadas (chamam uma vein sem celula): ...`, `geracao 2 aplicada no app em 3 ms: ...`, `recusada: a assinatura de 'painel.somar' mudou (()->void -> ()->stone): ...`, `o programa nao compila; o app segue como estava`, `o app recusou o remendo: dlopen: ...`. Details, rules, protocol and security (the agent only exists in `--refract` builds and only accepts the `adb` uid, root or the app itself) in [ANDROID.md](ANDROID.md#live-editing-on-the-device-fault-refract-l3). Device test: `ofs/tests/refract/android.sh` (skips without a device; same PID before and after, counter kept, new vein, refusal, compile error). The lava also applies here: the PC reads the editor draft of the watched files (the `.oal` and the local `.ofs` files) with the same rule and the same ~1 s silence for typing errors.

## `ofs fossil`: record a run and travel back in time

`ofs fossil` runs the program interpreted and records into a `.fossil` file everything that comes from outside and is not deterministic. `ofs fossil replay` runs it again handing back the recorded values instead of those calls, without touching the real world, and the output comes out identical. With `--to`, `--back` and `--dig` you can stop at any point of the run and go back in time.

```bash
ofs fossil app.ofs                       # run and record app.fossil (-o other.fossil)
ofs fossil replay app.fossil             # replay: same output, no stdin, clock, files...
ofs fossil replay app.fossil --to 120    # stop before step 120 and show locals and globals
ofs fossil replay app.fossil --to L42    # first time line 42 of the main file runs
ofs fossil replay app.fossil --to util.ofs:7   # a line of a local attach
ofs fossil replay app.fossil --to weigh  # entry into the vein weigh
ofs fossil replay app.fossil --to L42 --back 10   # 10 steps before that point
ofs fossil replay app.fossil --back 5    # 5 steps before the end
ofs fossil replay app.fossil --dig       # interactive mode (below)
```

**What is recorded.** Every external call goes through the interpreter's Rift (`extern vein`, `rift vein` and the runtime's native veins such as `ofs_read_line`), and each symbol has a policy:

- **world** (recorded; not called on replay): input (`read`, `fgets`, `fread`, `getchar`, `ofs_read_line`, `recv`, `SSL_read`, `getrandom`), clock (`time`, `clock_gettime`, `gettimeofday`, `clock`, `localtime(_r)`, `pulse.now_ms`, `sleep`/`usleep`, which do not wait on replay), randomness (`rand`, `random`, `drand48`, `srand`, `arc4random`...), environment (`getenv`, `ofs_getenv`), process (`getpid`), files and network (`fopen`, `fclose`, `fseek`, `open`, `close`, `stat`, `socket`, `connect`, `accept`, `system`, `remove`...). It records the return value and **the bytes C wrote into the program's buffer** (`read` → `r` bytes, `fgets` → the text, `fread` → `r * size`, `clock_gettime` → 16 bytes...). On replay the bytes go into the current buffer (the address changes between runs; the argument is the same). Text returned by C (`getenv`, `ofs_read_line`) comes back as a copy; `FILE*`/`DIR*` come back as an opaque value that only serves the recorded calls.
- **output** (`write`, `fwrite`, `fputs`, `fputc`, `fprintf`, `fflush`, `printf`, `puts`, `putchar`, `send`, `SSL_write`): records the return value and a hash of the data. On replay, console output (fd 1/2, `stdout`/`stderr`) really runs; file or network output becomes a **checked no-op**: if the data changed, `ofs fossil: aviso: a saida de 'fputs' (evento 13) difere da gravada`.
- **everything else** (pure C functions, `strlen`, `snprintf`, `malloc`, `memcpy`, math): really runs in both modes. `OFS_FOSSIL_WORLD="my_fn:other"` treats your own functions as world (return value only).
- `echo` belongs to the interpreter and repeats by itself. Command-line arguments: the language does not read `argv` yet, so there is nothing to record.

**Crystallization and OSR are off** during `ofs fossil` and replay: native code does not go through the external-call hook nor count steps. Veins with `quartz` stay native (warning: external calls made inside them are not recorded).

**Format** (version 1, text with raw payloads prefixed by their size; see `ofs/ofscc/fossil.ofs`):

```
OFS-FOSSIL 1
S <source hash> <n>\n<program path>
E <symbol> <rk> <value> [<float bits> [<output hash>]]   one call
<n>\n<bytes>                     returned text (rk 1) or new block (rk 3)
M <arg> <n>\n<bytes>             memory C wrote into the argument
F <steps> <events> <exit code>   end
```

`rk`: 0 = raw value, 1 = text, 2 = the argument itself (`fgets` returns the buffer), 3 = new memory block (`localtime`). Trailing zeros of an `E` line are omitted. A program doing 200 thousand `rand()` calls records 3.9 MB.

**Changed program.** The header stores the hash of the content of the main file and the local `attach`es. If it changed: `ofs fossil: aviso: o programa mudou desde a gravacao (o hash do fonte difere); o replay pode divergir`. If the program makes a different external call than the recorded one, replay stops with exit code 3: `o replay divergiu no evento 1 (passo 2): a gravacao tem 'time', o programa chamou 'rand'`. At the end, the step count is checked against the recording. Without `OFSCC_INPUT`, replay uses the path in the header.

**Going back in time.** A *step* is one executed statement (in `main`, in called veins and in the global initializers), counted the same way while recording and replaying. `--to` stops **before** the step and shows where it is and the variables (vein locals and globals declared in the program's files; arrays and `monolith`s expanded):

```
fossil: passo 28 | mundo.ofs:22 | vein pesar
  22 |     acumulado = acumulado + peso
  locais:
    nome: obsidian = "samuel"
    sorte: stone = 769
    peso: stone = 1369
  globais:
    acumulado: stone = 0
```

Program output is hidden up to that point. To go back, replay keeps **layers**: every `--layer N` steps (default 2000) the whole process is duplicated with `fork` (copy-on-write: variables, interpreter heap, raw memory and C memory) and the copy sleeps. Going to an earlier step wakes the closest layer before it, which re-executes (silently) up to the step; the current timeline ends. There are at most 32 layers: beyond that, every other one is dropped and the interval doubles. The process the shell waits for becomes a supervisor that only receives the exit code (on Linux it is a "subreaper", so it notices if the run crashes: `ofs fossil: o replay caiu (sinal 9)`).

**Interactive mode** (`--dig`, or `-i`): starts at step 1 (or at `--to`) and reads commands from the terminal (the command words are Portuguese, themed like the rest of the language; the one-letter shortcuts are the usual debugger ones):

| command | shortcut | what |
|---|---|---|
| `segue [n]` | `n` | moves forward n steps showing the program output |
| `cava [n]` | `b` | goes back n steps in time ("dig") |
| `ir <target>` | `g` | goes to a step (earlier or later), `L<line>`, `file.ofs:<line>` or vein (line and vein search forward) |
| `ver <name>` | `p` | prints a variable (local, then global) |
| `estrato` | `e` | locals and globals |
| `onde` | `w` | step, file:line, vein and the line text |
| `sai` | `q` | quits |

At the end of the program the mode goes on (`fossil: fim do programa (passo 47, codigo 0)`) and `cava`/`ir <step>` still work; a runtime error also lands at the prompt.

**Cost** (Ryzen 5 5600GT, `bench/ofs/fib.ofs` interpreted, `fib(35)`, 74.6 million steps): plain 6.21 s (before this change: 6.42 s, so the interpreter without recording did not get slower); recording 6.40 s (+3%); replay 6.43 s. With many external calls the cost is per call: 200 thousand `rand()` take 0.05 s plain, 0.32 s recording (each event hits the disk right away, so a crashing program leaves the recording up to the crash) and 0.14 s with `OFS_FOSSIL_SYNC=0` (written at the end); replay takes 0.12 s.

Variables (without the wrapper): `OFS_FOSSIL=record|replay`, `OFS_FOSSIL_FILE=file.fossil`, `OFS_FOSSIL_TO`, `OFS_FOSSIL_BACK`, `OFS_FOSSIL_DIG=1`, `OFS_FOSSIL_LAYER`, `OFS_FOSSIL_SYNC=0`, `OFS_FOSSIL_WORLD`; e.g. `OFS_FOSSIL=record OFS_FOSSIL_FILE=app.fossil OFSCC_MODE=run OFSCC_INPUT=app.ofs ofscc`. Recording and replay work wherever `ofs run` works; going back in time (`--back`, `--dig`, `cava`) needs `fork` (Linux, Android/Termux, macOS; on Windows only forward `--to`).

**VS Code:** the commands `OFS: Fossil: Go to a Point of a Recording`, `Fossil: Next Step` and `Fossil: Previous Step` run `ofs fossil replay --to`, show the state in the "OFS Fossil" channel and highlight the step's line in the editor. A debug adapter (DAP) with step-back on top of `--dig` is the next step.

Known limits: `scanf`/`fscanf` (outputs through variadic pointers) and functions that fill structures outside the table do not restore memory (use `read`/`fgets` + `sscanf`, which is pure); UI apps (X11/Android events) are not recorded; calls through a vein value coming from C (`call_native_sig`) really run.

Test: `ofs/tests/fossil/run.sh` (records a program with stdin, clock, randomness, environment, a file and a `rift vein` with `bind`; replays with all of it changed and checks identical output and the file not rewritten; `--to` by step, line, `file:line` and vein with the variable values; `--back` with layers every 7 and 2000 steps; `--dig` with `ir`/`ver`/`segue`/`cava`/`estrato`; changed program warns and diverges; no layer left behind).

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

**Dependencies between packages:** a package published with `[deps]` in its `ofs.claim` carries those ranges into its registry entry. For each package, resolution picks the highest version that satisfies every requested range (the locked one, if it still fits) and then follows that version's dependencies. If no version satisfies every range, that is a conflict: nothing is written, and the message lists who asked for what (`conflito: nenhuma versao de basalto satisfaz ^1.0 (pedido por granito@1.0.0); ^2.0 (pedido por xisto@1.0.0)`).

**Publishing:** run `ofs quarry publish` in the package folder. It builds a reproducible `dist/<name>-<version>.tar.gz` (sorted by name, with zeroed dates and owners and `gzip -n`, so the same source gives the same sha256). The archive holds `include` (default: the folder of `main`), the manifest, `README.md` and `LICENSE`. The command prints the registry entry with its hash and uploads nothing. `--origin` sets the URL (`{name}`, `{version}`, `{file}` are replaced; without it the URL is the file name, relative to the registry). `--deposit registry.json` writes the entry into the registry. `src/packages/build_packages.sh` does this for the 19 libraries in `src/packages/src/`, which still use the old `ofspkg.json` (also accepted).

Test: `ofs/tests/quarry/run.sh` (temporary local registry, temporary `HOME`/`OFS_LIB_HOME`). Pending: package signatures (only the registry/lock sha256 for now), per-project installs, and uploading the release from `publish`.
