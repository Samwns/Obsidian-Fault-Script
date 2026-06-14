# OFS Roadmap

## Release Stability

- [ ] Keep GitHub Pages generated from `src/site` and published from `docs`.
- [ ] Keep Auto Release green on Linux, macOS and Windows.
- [ ] Validate `.deb`, `.rpm`, `.pkg.tar.zst`, `.pkg`, `.exe` and `.vsix` after every release.
- [ ] Update workflows to Node 24 actions before GitHub forces the runtime change.

## Repository Layout

- [ ] Keep root limited to `README.md`, hidden Git/GitHub config, `ofs/`, `docs/` and `src/`.
- [ ] Keep generated site output in `docs`.
- [ ] Keep site source in `src/site`.
- [ ] Keep extension source in `src/vscode-extension`.
- [ ] Keep packaging, packages, grammar and release archive under `src`.

## OFS Compiler

- [ ] Replace compiler crashes with diagnostics for missing or invalid attached files.
- [ ] Add parser recovery for malformed source.
- [ ] Add line and column ranges to compiler diagnostics.
- [ ] Add typo suggestions for names and attach targets.
- [ ] Stabilize optimization flags across CLI, packages and VS Code.
- [ ] Add LTO support.
- [ ] Add sanitizer builds for runtime validation.
- [ ] Add profiling output for compiler hotspots.

## ODL and OES

- [ ] Keep `.odl` as authored document source and HTML as generated artifact.
- [ ] Keep `.oes` as authored design/effect source and CSS as generated artifact.
- [ ] Expand native Markdown support.
- [ ] Expand safe raw HTML, JavaScript module, Node-style module and PHP interop.
- [ ] Add visual positioning export from Go Live to OES.
- [ ] Add stricter compiler errors for indentation and invalid blocks.
- [ ] Add more complete HTML-to-ODL and CSS-to-OES translation.

## Site

- [ ] Convert every static documentation block into a real documentation page.
- [ ] Keep a dedicated Downloads page.
- [ ] Document OFS, ODL, OES, CLI, VS Code, packages, webserver, web UI and benchmarks.
- [ ] Keep benchmark charts for CPU, memory, startup, build time and artifact size.
- [ ] Keep Portuguese, English and Spanish UI strings aligned.
- [ ] Keep GitHub Pages deploy green after site generation.

## VS Code Extension

- [ ] Keep `.ofs`, `.odl` and `.oes` icons distinct.
- [ ] Keep diagnostics using the packaged compiler.
- [ ] Keep Go Live for explicit web files only.
- [ ] Keep ODL/OES live rebuild and browser reload.
- [ ] Add commands for ODL/OES translation.
- [ ] Add visual positioning editor support.

## Distribution

- [ ] Keep Linux packages for Debian/Ubuntu, Fedora/RHEL/openSUSE and Arch/Manjaro.
- [ ] Keep macOS ARM64 package.
- [ ] Keep Windows x64 installer.
- [ ] Keep VS Code `.vsix` in releases.
- [ ] Add Homebrew formula.
- [ ] Add AUR package metadata.
- [ ] Add checksum verification instructions.

## GitHub Recognition

- [ ] Keep repository-local Linguist overrides for `.ofs`, `.odl` and `.oes`.
- [ ] Prepare Linguist grammar metadata.
- [ ] Prepare public examples for language recognition.
- [ ] Open upstream Linguist PR when the language has enough public usage.
