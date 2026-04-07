# OFS Low-Level Roadmap

This roadmap defines how OFS can go deeper into low-level programming without turning into C-with-new-keywords or assembly-with-themeing.

## Core Principle

High-level OFS and low-level OFS must coexist in the same file, function, and program.

- High-level code remains the default surface for domain logic.
- Low-level code is opt-in and localized.
- Crossing between the two layers should feel native to OFS, not like escaping the language.
- The compiler must preserve safety and readability outside the explicitly low-level regions.

## Updated Objectives

The low-level evolution of OFS should satisfy all of these objectives at once:

1. OFS must keep its own conceptual identity.
2. Low-level OFS must not require users to think in C first.
3. Low-level OFS must be able to coexist with high-level application code in one source file.
4. Interop with foreign ecosystems must be strong, but optional.
5. The most powerful low-level path should ultimately be expressible in OFS-native concepts, even when inspired by machine-level models.

## Non-Goals

- OFS should not become syntax sugar for C.
- OFS should not require the user to think in registers for normal systems code.
- OFS should not force whole-program unsafe mode just to optimize one hot path.
- OFS should not make its low-level layer depend conceptually on C, C++, Python, C#, or handwritten assembly.

## Current Baseline

Today OFS already has the foundations for hybrid programming:

- `fracture` for typed pointer work.
- `abyss` for unrestricted memory-oriented work.
- `fractal` for lifted-effect regions.
- Native compilation through LLVM IR to object code and executable.
- External runtime calls via `extern vein`.
- Target assembly inspection via `ofs asm`.

This means the language already supports serious mixed-style programming, but the low-level layer is still closer to raw escape hatches than to a first-class OFS subsystem.

## Implemented Set Today

The following parts of the roadmap are already materialized in the repository:

- `bedrock { ... }` typed low-level block
- `rift vein` native interop declaration surface
- `fault_count`, `fault_lead`, `fault_trail`, `fault_swap`
- `fault_spin_left`, `fault_spin_right`, `fault_step`
- `fault_cut`, `fault_patch`, `fault_weave`
- `fault_fence`, `fault_trap`
- `bedrock_cell_*` helpers
- `bedrock_region_*` helpers
- `bedrock_view_*` helpers
- lane helpers such as `bedrock_lane8_get`
- `rift.ofs` wrapper module
- `bedrock_packet.ofs` structured packet/header module

This already gives OFS a meaningful low-level identity that is not reducible to raw C wrappers.

## Direction: OFS-Native Low-Level Layer

The low-level story should grow around OFS-owned concepts instead of importing C mental models directly.

Suggested vocabulary:

- `bedrock`: explicit storage and layout control.
- `fracture`: typed reference and pointer manipulation.
- `abyss`: unrestricted raw operations.
- `fractal`: controlled effect lifting across boundaries.

The important part is not the exact names, but that the programmer feels they are still inside OFS.

## Roadmap

### Phase 1: Inspectable Native Backend

Goal: make native lowering visible without changing the source language model.

- Keep `ofs ir` for LLVM inspection.
- Add `ofs asm` for target assembly inspection.
- Make assembly generation part of the official debugging story.
- Preserve high-level OFS as the source of truth.

Status: available with `ofs ir` and `ofs asm`.

### Phase 2: OFS-Owned Low-Level Library Surface

Goal: stop forcing users to think in raw runtime symbols first.

- Ship OFS stdlib modules that wrap manual allocation and storage primitives.
- Use OFS naming and semantics for low-level tasks.
- Encourage patterns where domain code is high-level and hot/stateful code is bedrock-style.

Status: started with `bedrock.ofs`, `bedrock { ... }`, `fault_*` intrinsics, region helpers, view helpers, and barrier primitives.

### Phase 2.5: Native Low-Level Objects

Goal: define reusable low-level building blocks as first-class OFS objects/modules.

- Heap cells, fixed buffers, packet views, and state blocks.
- OFS-native names for ownership, storage, aliasing, and release.
- Clear theme and vocabulary matching the rest of the language.

Examples of the direction, not finalized syntax:

- `bedrock cell`
- `monolith PacketView bedrock { ... }`
- `vein shard_map(...)`

Status: started through `rift vein` as the first OFS-native interop declaration surface, with `bind`/`abi` metadata and structured low-level modules such as `bedrock_packet.ofs`.

### Phase 3: Explicit Storage Primitives

Goal: let programmers describe memory intentionally inside OFS.

- Add first-class fixed-size storage cells.
- Add explicit buffer regions and slices.
- Add typed views over raw memory.
- Add byte-level operations that stay localized to low-level blocks.

Possible examples:

- `bedrock cell<stone>`
- `bedrock region[256]`
- `fracture view packet as Header`

Status: started with `fault_step`, `fault_cut`, `fault_patch`, `bedrock_region_*`, and `bedrock_view_*` helpers for explicit storage regions and field work.

### Phase 4: Layout and ABI Control

Goal: support systems boundaries without abandoning OFS semantics.

- Stable struct layout controls.
- Endianness-aware read/write helpers.
- Explicit calling convention metadata where needed.
- Safer foreign-boundary declarations for external APIs.

Status: started with `bind`/`abi` metadata on interop declarations, `layout native|packed|c` on `monolith`, endian-aware lane helpers in `bedrock.ofs`, and the first OFS-native interop wrapper module `rift.ofs`.

### Phase 4.5: Foreign Runtime Bridges

Goal: allow OFS to host and call external ecosystems without making them the base abstraction model.

- Native library loading and symbol binding.
- C ABI bridge as the baseline interop layer.
- C++ through explicit bridge surfaces.
- C# bridge through hosting/binding adapters.
- Python bridge through embedding or extension adapters.

Important rule:

Interop must feel like a capability OFS owns, not like OFS dissolving into another language.

Status: foundational work only. OFS already supports symbol-level foreign boundaries through `extern vein` and `rift vein`, but full host/runtime bridges for C++, C#, and Python are still future work.

### Phase 5: OFS Intrinsics

Goal: expose low-level machine capabilities through OFS concepts.

- Bit scans, rotates, popcount, byte swaps.
- Fences, atomics, and cache-aware primitives.
- Prefetch and vector-friendly operations where LLVM can lower them well.

These should be surfaced as OFS intrinsics or stdlib operations, not as random C imports.

Status: started with bit scans, rotates, popcount, byte swap, typed stepping, field extraction/patching, barriers, trap hooks, and prefetch support.

### Phase 6: Inline Native Escape Hatch

Goal: allow truly target-specific operations without making them the default.

- Scoped inline assembly or target snippets.
- Strict isolation inside low-level blocks.
- Explicit clobber and input/output contracts.
- No effect on unrelated high-level code.

This is the point where OFS can go beyond assembly inspection and into authoring machine-adjacent code directly.

Status: future.

### Phase 6.5: OFS Machine-Like Primitives

Goal: let OFS do some things assembly already does, and some things assembly does not expose directly, while staying in OFS terms.

- Explicit bit lanes and field extraction.
- Tagged storage regions.
- Typed trap/assert/fence operations.
- OFS-owned naming for jumps, barriers, memory windows, and control shards.

This is where the language stops merely inspecting assembly and starts owning a deeper low-level model.

Status: future.

### Phase 7: Freestanding and Bare-Metal Profiles

Goal: make OFS viable where there is no hosted runtime.

- Optional no-libc target profile.
- Minimal runtime bootstrap.
- Custom entrypoint support.
- Configurable memory/runtime services.

Status: future.

## Remaining Structural Work

The remaining phases are not blocked by syntax alone. They require larger compiler/runtime subsystems:

- Inline native escape hatch needs a dedicated parsing and lowering model for target-specific snippets.
- Full ABI control needs calling convention metadata, layout guarantees, and safer external binding rules.
- Foreign runtime bridges for C++, C#, and Python need runtime-host integration beyond simple symbol calls.
- Freestanding/bare-metal support needs runtime refactoring and alternative startup/link paths.

These are feasible directions, but they are larger projects than the incremental language/compiler slices already implemented.

## Coexistence Rules

These rules should remain true as the low-level layer grows:

1. High-level logic and low-level logic can appear in the same file.
2. Unsafe power must stay local and explicit.
3. Entering a low-level region must not silently degrade type guarantees elsewhere.
4. Low-level APIs should be callable from normal OFS code without ceremony.
5. The language should encourage moving only the critical path downward, not rewriting everything in a lower style.

## What Makes This a Language Differentiator

The differentiator is not "OFS can call C".

The differentiator is:

- a beginner can start high-level,
- an advanced user can optimize specific subsystems,
- both styles can live together in one coherent OFS codebase,
- and the low-level layer still speaks OFS.

That is a stronger identity than simply looking like C or exposing assembly.

## Interop Summary

The intended relationship is:

- OFS can call foreign code.
- OFS can host foreign runtimes.
- OFS can emit and inspect native code.
- OFS low-level features do not depend on foreign languages for their identity.

The language can borrow useful ideas from assembly, but the resulting model should still be recognizably OFS.

---

## Próximos Passos e Conclusão

### O que falta para o roadmap ser considerado "finalizado"

- **Bridges de runtime estrangeiro completos**: integração real com C++, C#, Python, indo além do símbolo/ABI, com adaptação de tipos e gerenciamento de ciclo de vida.
- **Escape hatch nativo inline**: suporte a assembly inline ou snippets alvo, com contratos explícitos e isolamento seguro.
- **Primitivas machine-like avançadas**: controle de saltos, regiões de memória tagueadas, janelas de controle, e operações que vão além do que o assembly expõe diretamente.
- **Perfil bare-metal/freestanding**: inicialização sem runtime, sem libc, com entrada customizada e serviços mínimos.

### Diretrizes para a evolução

- Sempre priorizar a identidade OFS: nomes, contratos e ergonomia próprios.
- O baixo nível deve ser opt-in, seguro por padrão e nunca degradar o alto nível.
- A interoperabilidade deve ser poderosa, mas nunca dissolver a linguagem em outro ecossistema.
- O roadmap deve ser revisitado conforme a comunidade e os casos reais de uso evoluírem.

### Conclusão

OFS já atingiu um núcleo de baixo nível robusto, com diferenciais reais frente a linguagens tradicionais. As próximas fases são grandes projetos de integração e expansão, mas a base já permite que alto e baixo nível coexistam de forma nativa, segura e idiomática. O futuro do baixo nível na OFS é evoluir sem perder sua essência: poder, clareza e identidade própria.

- [Bare-metal/Freestanding Profile (experimental)](ofs/docs/BAREMETAL_PROFILE.md)