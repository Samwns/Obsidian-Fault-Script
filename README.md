<div align="center">
  <img src="docs/ofs-256.png" width="112" height="112" alt="Obsidian Fault Script icon">

  # Obsidian Fault Script

  **A self-hosted compiled language for native programs and the Web.**

  [![Build](https://github.com/Samwns/Obsidian-Fault-Script/actions/workflows/build.yml/badge.svg)](https://github.com/Samwns/Obsidian-Fault-Script/actions/workflows/build.yml)
  [![Pages](https://github.com/Samwns/Obsidian-Fault-Script/actions/workflows/pages.yml/badge.svg)](https://samwns.github.io/Obsidian-Fault-Script/docs/)
  [![Release](https://img.shields.io/github/v/release/Samwns/Obsidian-Fault-Script?display_name=tag&sort=semver)](https://github.com/Samwns/Obsidian-Fault-Script/releases/latest)
  [![License](https://img.shields.io/badge/license-BSL--1.0-29e0bd)](docs/legal/LICENSE)
  [![LLVM](https://img.shields.io/badge/backend-LLVM-7c5cff)](docs/COMPILER_ARCHITECTURE.md)

  [**Official Site**](https://samwns.github.io/Obsidian-Fault-Script/docs/)
  · [**Documentation**](https://samwns.github.io/Obsidian-Fault-Script/docs/getting-started.html)
  · [**Downloads**](https://github.com/Samwns/Obsidian-Fault-Script/releases/latest)
  · [**Benchmark**](https://samwns.github.io/Obsidian-Fault-Script/docs/benchmark.html)
</div>

---

OFS compiles statically typed source through a self-hosted frontend, emits LLVM
IR, and links a compact native runtime. The same ecosystem includes **ODL** for
browser documents and **OES** for visual rules, responsive behavior, and
motion.

| Layer | Purpose | Source |
|---|---|---|
| **OFS** | Native application logic, types, modules, FFI, and low-level work | `.ofs` |
| **ODL** | Documents, semantic structure, components, Markdown, and interop | `.odl` |
| **OES** | Tokens, layout, responsiveness, transitions, and keyframes | `.oes` |
| **LLVM runtime** | Native executable generation and standard runtime services | C / LLVM IR |

## Quick Start

```bash
ofs check app.ofs
ofs run app.ofs
ofs build app.ofs -o app
```

```ofs
vein greet(name: obsidian) -> obsidian {
    return "hello, " + name
}

core main() {
    echo(greet("OFS"))
}
```

Installers for Debian/Ubuntu, Fedora/RHEL, Arch/Manjaro, macOS ARM64, Windows
x64, and the VS Code extension are published on the
[latest release](https://github.com/Samwns/Obsidian-Fault-Script/releases/latest).

## Native Web

ODL and OES remain the authored source. Browsers receive generated HTML, CSS,
and JavaScript, so sites work on GitHub Pages, CDNs, and ordinary web servers.

```odl
page "OFS Web" "en"
skin "theme.css"
flow
  stage#app
    title1 "OFS Web"
    mark "Built with **ODL**."
```

```oes
gem accent "#29e0bd"
paint .button
  fill "$accent"
  curve "8px"
```

```bash
ofs odl page.odl -o index.html
ofs oes theme.oes -o theme.css
ofs translate legacy.html --to odl
ofs translate legacy.css --to oes
```

ODL supports native Markdown plus explicit HTML, JavaScript modules, Node-style
modules, and PHP template interoperability. The VS Code extension recognizes
`.ofs`, `.odl`, and `.oes`, provides syntax support, and runs ODL sites with
automatic recompilation and live reload.

## Repository Map

| Area | Path |
|---|---|
| Self-hosted compiler | [`ofs/ofscc/`](ofs/ofscc/) |
| Native runtime | [`ofs/runtime/`](ofs/runtime/) |
| Standard library | [`ofs/stdlib/`](ofs/stdlib/) |
| ODL/OES compilers | [`ofs/tools/`](ofs/tools/) |
| Examples | [`ofs/examples/`](ofs/examples/) |
| Website source | [`src/site/src/`](src/site/src/) |
| Benchmark suite | [`src/site/benchmarks/`](src/site/benchmarks/) |
| VS Code extension | [`src/vscode-extension/`](src/vscode-extension/) |
| Packaging | [`src/packaging/`](src/packaging/) |
| Published documentation | [`docs/`](docs/) |

## Documentation

| Guide | Contents |
|---|---|
| [Getting Started](docs/GETTING_STARTED.md) | Installation, first program, packages, Web, and examples |
| [Language Reference](docs/LANGUAGE_REFERENCE.md) | Syntax, types, control flow, modules, pointers, and low-level modes |
| [Standard Library](docs/STANDARD_LIBRARY.md) | Every distributed module and its public surface |
| [Compiler Architecture](docs/COMPILER_ARCHITECTURE.md) | Lexer, parser, type checking, LLVM generation, runtime, and bootstrap |
| [ODL Reference](docs/ODL_REFERENCE.md) | Native document language |
| [OES Reference](docs/OES_REFERENCE.md) | Native visual and effect language |
| [Web UI](docs/WEB_UI.md) | Hosting, live reload, components, and browser artifacts |
| [Performance](docs/PERFORMANCE.md) | Benchmark methodology, current hotspots, and optimization gates |
| [GitHub Recognition](docs/GITHUB_REGISTRATION.md) | Linguist registration and repository detection |

## Benchmark

The reproducible suite measures real processes for CPU, peak RSS, startup,
optimized compilation, artifact size, and output correctness:

```bash
bash src/site/benchmarks/run.sh
```

Results describe the recorded machine and workload, and are published without
turning one microbenchmark into a universal language ranking.

## Build From Source

```bash
clang -O2 -ffunction-sections -fdata-sections \
  -c ofs/runtime/ofs_runtime.c -o /tmp/ofs_runtime.o
ar rcs /tmp/libofs_runtime.a /tmp/ofs_runtime.o
clang -O2 ofs/dist/ofscc.ll /tmp/libofs_runtime.a -lm -o /tmp/ofscc
```

The main branch is verified on Linux, macOS, and Windows. Every push also
regenerates GitHub Pages and publishes versioned release artifacts through the
repository workflows.
