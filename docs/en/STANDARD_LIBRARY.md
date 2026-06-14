# OFS Standard Library

The standard library lives in `ofs/stdlib/` and is imported with `attach {name}`.

Core modules:

- `math.ofs`: square, cube, range sum, prime checks, digit operations, and numeric palindromes;
- `string.ofs`: repetition, first-character checks, and empty-string checks;
- `io.ofs`: prompts, separators, and terminal headers;
- `terminal_colors.ofs`: ANSI color constants and output helpers.

Web modules:

- `odl.ofs`: document, semantic element, form, code, and table builders;
- `oes.ofs`: properties, rules, media queries, layers, variables, transitions, and keyframes;
- `webserver.ofs`: HTTP constants, responses, MIME types, JSON fragments, logs, and serving loops;
- `webui.ofs`: themes, pages, navigation, heroes, buttons, panels, grids, cards, stacks, and serving.

Native visual modules:

- `canvas.ofs`: experimental 2D drawing;
- `window.ofs`: native window lifecycle;
- `ui.ofs`: immediate-mode text, measurements, boxes, and controls.

System modules:

- `bedrock.ofs`: explicit regions, pointers, and low-level memory primitives;
- `bedrock_packet.ofs`: experimental packet/protocol structures;
- `memory_modes.ofs`: explicit memory-mode experiments;
- `rift.ofs`: experimental C ABI and FFI surface.

Compatibility and testing:

- `ofshtml.ofs`: legacy HTML helpers, superseded by ODL/OES for new projects;
- `test_lib.ofs`: helpers used by language tests and examples.

See the generated `stdlib.html` page for function-level examples and searchable module descriptions.
