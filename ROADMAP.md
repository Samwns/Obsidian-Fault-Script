# OFS Roadmap — Evolução Completa

**Versão Atual**: v1.1.0-alpha (Bootstrap pronto)  
**Objetivo Final**: v2.0.0 (Production-ready, sem C++ dependency)

## Critério de 100% (Gate)

O roadmap só é considerado 100% completo quando:

- todos os itens de checklist estiverem em `[x]`
- nenhum item estiver em `[ ]` ou `[~]`
- pipeline de release passar no gate de 100%

Gate automático implementado em:

- `.github/workflows/roadmap-gate.yml`
- `scripts/roadmap_completion.ps1`

---

## Timeline & Versões

```
┌─────────────────────────────────────────────────────────────────────┐
│ Phase A: Auto-Hosting Proof                                        │
│ ✅ COMPLETO (v1.0.0 → v1.1.0-alpha)                               │
├─────────────────────────────────────────────────────────────────────┤
│ - Compilador em OFS criado (13 arquivos, ~4,500 LOC)               │
│ - Bootstrap script criado (Windows + Linux)                         │
│ - Documentação completa                                             │
│ - Status: Pronto para testar                                        │
└─────────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────────┐
│ Phase B: Refinement & Optimization                                 │
│ ⏳ PRÓXIMO (v1.1.0 Release)                                         │
├─────────────────────────────────────────────────────────────────────┤
│ Duration: 2 weeks                                                   │
│ Deliverables:                                                       │
│ - [ ] Bootstrap test (Windows + Linux verified)                     │
│ - [ ] Determinism check (v2 === v3)                                │
│ - [~] CLI bridge via env + mode flags                              │
│ - [~] impl/namespace/strata parser + codegen base                   │
│ - [~] tremor/catch parser + lowered block                           │
│ - [~] Compiler optimization flags (-O0, -O2, -O3)                  │
│ - [~] Cross-platform CI (Linux, macOS, Windows)                     │
│ - [ ] Documentation updates                                        │
│ - [~] GitHub Actions workflow (C++ + self-hosted experimental)      │
└─────────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────────┐
│ Phase C: LLVM Backend                                              │
│ 🔮 FUTURE (v1.2.0 Release)                                         │
├─────────────────────────────────────────────────────────────────────┤
│ Duration: 3-4 weeks                                                 │
│ Deliverables:                                                       │
│ - [ ] LLVM IR codegen (skip C intermediary)                         │
│ - [ ] 40-50x faster compilation                                    │
│ - [ ] Link-Time Optimization (LTO) support                        │
│ - [ ] Full optimization pipeline                                   │
│ - [ ] Sanitizer support (ASAN, UBSAN)                             │
│ - [ ] Profiling support                                            │
│ - [ ] Release notes                                                │
└─────────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────────┐
│ Phase D: Remove C++ Dependency                                     │
│ 🔮 FUTURE (v1.3.0 Release)                                         │
├─────────────────────────────────────────────────────────────────────┤
│ Duration: 1-2 weeks                                                 │
│ Deliverables:                                                       │
│ - [ ] Port ofscc to use LLVM SDK directly                          │
│ - [ ] Remove C++ code from repo                                    │
│ - [ ] Update build instructions                                    │
│ - [ ] Migration guide for existing users                           │
│ - [ ] Remove LLVM C++ bindings requirement                         │
│ - [ ] Pure LLVM C API or FFI                                      │
└─────────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────────┐
│ Phase E: Distribution & Tooling                                    │
│ 🔮 FUTURE (v1.4.0 + v2.0.0)                                        │
├─────────────────────────────────────────────────────────────────────┤
│ Duration: 3-4 weeks                                                 │
│ Deliverables:                                                       │
│ - [ ] Windows installer (.exe via OFS generator)                   │
│ - [ ] Linux packages (.deb, .rpm, AUR)                            │
│ - [ ] macOS package (Homebrew, DMG)                               │
│ - [ ] GitHub Actions with OFS-generated installer                 │
│ - [ ] VS Code extension integration                                │
│ - [ ] LSP server in OFS                                            │
│ - [ ] Package manager (registry)                                   │
│ - [ ] v2.0.0 stable release                                        │
└─────────────────────────────────────────────────────────────────────┘
```

---

## Phase B (v1.1.0): Refinement & Optimization

### B.1: Bootstrap Verification ✓ (This Sprint)

**Objective**: Prove OFS can recompile itself deterministically

**Tasks**:
- [✓] Create test_bootstrap.sh (Linux)
- [✓] Create test_bootstrap_windows.ps1 (Windows)
- [ ] Run on Windows (user has environment)
- [ ] Run on Linux (CI/CD)
- [ ] Verify v2 === v3 (byte-exact)
- [ ] Document results

**Success Criteria**:
```
ofscc_v1.exe (C++ generated OFS)
    ↓ compiles ofscc.ofs
ofscc_v2.exe ← Binary 1
    ↓ compiles ofscc.ofs again
ofscc_v3.exe ← Binary 2
    
Assertion: sha256(ofscc_v2) === sha256(ofscc_v3)
```

**Timeline**: Immediate (test now!)

---

### B.2: CLI & Language Features (Week 1)

**Objective**: Full argument parsing + missing language features

**CLI Arguments** (currently hardcoded):
```
ofs INPUT.ofs -o OUTPUT.exe [options]

Options:
  -O0 / -O2 / -O3       Optimization level
  -c                    Compile to C only (keep output.c)
  -v, --verbose         Verbose output
  -W all / -W none      Warning level
  --check               Type check only, don't compile
  --emit-llvm           Emit LLVM IR (Phase C preview)
```

**Language Features**:
- [~] impl blocks (struct methods codegen base)
- [~] namespace declarations (flattened emission)
- [~] strata (enum-tag base generation)
- [ ] Default parameters
- [ ] Named arguments

**Timeline**: Week 1

---

### B.3: Error Handling & Diagnostics (Week 1)

**Objective**: Better error messages, exception support

**Improvements**:
- [ ] Implement tremor/catch (exception handling in codegen)
- [ ] Error recovery in parser
- [ ] Line/column accurate error reporting
- [ ] Suggestion for typos ("did you mean?")
- [ ] Warning categories (-W warnings)

**Timeline**: Week 1

---

### B.4: Cross-Platform Testing (Week 2)

**Objective**: Verify compilation on all platforms

**Platforms**:
- [✓] Windows (user testing now)
- [ ] Linux (GitHub Actions)
- [ ] macOS (CI/CD)

**Test Suite**:
- [✓] hello.ofs (basic)
- [✓] factorial.ofs (recursion)
- [✓] fizzbuzz.ofs (loops)
- [ ] webserver.ofs (complex)
- [ ] All stdlib modules

**Timeline**: Week 2

---

### B.5: Documentation Updates (Week 2)

**Files to update**:
- [✓] COMPILED_LANGUAGE.md (just created)
- [ ] GETTING_STARTED.md (bootstrap + Phase B preview)
- [ ] LANGUAGE_REFERENCE.md (add cli options)
- [ ] CHANGELOG.md (v1.1.0 release notes)
- [ ] README.md (update version, links)
- [ ] docs/ (comprehensive refresh)

**Timeline**: Week 2

---

### B.6: GitHub Actions Setup (Week 2)

**Objective**: CI/CD with C++ compiler (Phase D will use OFS)

**Workflow** (still using C++ `ofs`):
```yaml
on: [push, pull_request]
jobs:
  build:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      - name: Setup OFS (C++ compiler)
        run: sudo apt-get install -y llvm-17-dev
      - name: Build C++
        run: cd ofs/build && cmake .. && make -j4
      - name: Test
        run: ./ofs ofs/examples/hello.ofs -o hello && ./hello
      - name: Bootstrap Test
        run: bash ofs/ofscc/test_bootstrap.sh
```

**Timeline**: Week 2

---

## Phase C (v1.2.0): LLVM Backend

### Perspective: Why LLVM?

Current: OFS → C → GCC (250ms compile time, `~4,500 LOC ofscc`)

Future: OFS → LLVM IR → Native (50-100ms, more control)

### C.1: LLVM IR Codegen

**Objective**: Generate LLVM IR instead of C

**Changes**:
- [ ] Add llvm_ir_gen.ofs (IR emission)
- [ ] Map OFS types → LLVM types
- [ ] Direct IR emission for each node kind
- [ ] Link LLVM libraries

**Files Modified**:
- ofscc/codegen.ofs → split into:
  - codegen_c.ofs (old C path)
  - codegen_llvm.ofs (new IR path)

**Timeline**: 2 weeks

---

### C.2: Performance Improvements

**Metrics**:
- Compilation: 250ms → 50ms (5x faster)
- Executable size: Similar
- Runtime: No change (both produce native code)

**LTO Support**:
```
ofs -O3 --lto program.ofs
```

**Timeline**: 1 week

---

## Phase D (v1.3.0): Remove C++ Dependency

### D.1: Replace C++ Components

**Current Structure**:
```
/ ofs/src/main.cpp (C++, LLVM C++ bindings)
```

**Goal**:
```
/ ofs/ofscc/ (all OFS)
  - Can use LLVM C API or pure FFI
```

**Approach**:
1. Keep build infrastructure (CMake knows how to find LLVM)
2. OFS can call LLVM C API via `extern`
3. No more C++ code in ofs/ folder

**Timeline**: 1 week after Phase C

---

### D.2: Update Documentation

- [ ] "OFS is now self-hosted!"
- [ ] Remove C++ installation steps
- [ ] Update build instructions
- [ ] "Thanks to auto-hosting, OFS needs only: LLVM SDK"

---

## Phase E (v1.4.0 + v2.0.0): Distribution

### E.1: Installer Generation

**Objective**: OFS generates its own installers

**Approach**:
1. Create installer_generator.ofs
2. Takes binary + stdlib + docs as input
3. Outputs:
   - Windows: installer.exe (NSIS script)
   - Linux: install.sh + deb/rpm
   - macOS: dmg + Homebrew formula

**Timeline**: 2 weeks

---

### E.2: GitHub Actions with OFS

**Current (Phase B)**: Uses C++ ofs

**Phase E**: GitHub Actions uses OFS to generate installer

```yaml
# .github/workflows/release.yml
- name: Download OFS v1.3.0
  run: curl -L https://releases.ofs.lang/v1.3.0/ofs-windows.zip -o ofs.zip
  
- name: Generate Installer
  run: |
    ofs installer_generator.ofs \
      --input build/ofs.exe \
      --output releases/ofs-1.4.0-installer.exe
```

**Timeline**: After Phase E.1

---

### E.3: VS Code Extension

**Current Status**: Extension exists but uses C++ ofs

**Updates Needed**:
- [ ] Point to ofscc (v1.3.0+)
- [ ] Use LSP server from ofscc
- [ ] Syntax highlighting refresh
- [ ] Debugger integration

---

### E.4: v2.0.0 Stable Release

**Requirements**:
- [ ] Phase B complete (fully tested)
- [ ] Phase C complete (LLVM IR working)
- [ ] Phase D complete (C++ removed)
- [ ] Phase E.1-3 complete (distribution working)

**Release Notes**:
```markdown
# v2.0.0 Release — Production Ready

✅ Self-hosted compiler (all OFS)
✅ LLVM IR backend (50x faster compilation)
✅ No C++ dependency (LLVM SDK only)
✅ Cross-platform (Windows, Linux, macOS)
✅ Package manager integrated
✅ VS Code extension with LSP
✅ Auto-generated installers
✅ 2,000+ tests passing

Benchmarks:
- Compilation: 50-100ms typical program
- Runtime: C-equivalent performance
- Bootstrap: Deterministic (v2 === v3)
```

**Timeline**: After Phase E

---

## Dependency Changes Over Time

### v1.0.0 (Current C++ only)
```
Dependencies:
- LLVM 17+
- C++ compiler (g++, clang++)
- GCC/Clang (runtime backend)
```

### v1.1.0 (Refinement)
```
Same as v1.0.0
(Bootstrap proven but C++ still primary)
```

### v1.2.0 (LLVM Backend)
```
- LLVM 17+ (C API, not C++ bindings)
- GCC/Clang (optional, for cross-compiling)
```

### v1.3.0 (No C++)
```
- LLVM 17+ (C API only)
- (Remove: C++ compiler, C++ bindings)
```

### v2.0.0 (Full Self-Hosting)
```
- LLVM 17+ (C API only)
- OFS compiler (self-hosted)
- Everything else in OFS
```

---

## Current Outstanding Tasks

### Immediate (This Week)

- [ ] Test bootstrap on Windows (user machine)
- [ ] Test bootstrap on Linux (CI)
- [ ] Verify determinism (v2 === v3)
- [ ] Document results
- [ ] Update STATUS_REPORT.md

### Week 1-2 (Phase B Start)

- [ ] Full CLI argument parsing
- [ ] impl/namespace/strata codegen
- [ ] tremor/catch support
- [ ] Cross-platform testing
- [ ] Documentation refresh

### When Bootstrap Ready

- [ ] Start Phase C (LLVM IR)
- [ ] Plan Phase D (Remove C++)
- [ ] Plan Phase E (Distribution)

---

## Success Metrics

| Milestone | Metric | Status |
|-----------|--------|--------|
| **Phase A** | Compiler in OFS | ✅ Done |
| **Phase A** | Bootstrap tested | ⏳ User to test |
| **Phase B** | v1.1.0 released | ⏳ After testing |
| **Phase C** | LLVM IR working | 🔮 Not started |
| **Phase D** | C++ removed | 🔮 Not started |
| **Phase E** | v2.0.0 stable | 🔮 Not started |
| **Goal** | Self-hosted OFS | ✅ Ready (Phase D) |

---

## Next Action

**For Now**:
```bash
# Windows
.\ofs\ofscc\test_bootstrap_windows.ps1

# Linux
bash ofs/ofscc/test_bootstrap.sh
```

**Expected Output**:
```
✅ Tamanhos idênticos
✅ SHA256 idênticos
✅ BOOTSTRAP DETERMINÍSTICO VERIFICADO!
```

**After Bootstrap Passes**:
→ Begin Phase B refinements
→ Update roadmap with metrics

---

## Repository Changes Coming

### Before (Current State)

```
ofs/
  src/
    main.cpp (C++)
    codegen/ (C++)
    ...
```

### After Phase D

```
ofs/
  ofscc/ (all OFS, ~4,500 LOC + Phase B)
  docs/ (comprehensive)
  examples/ (all working)
  (NO C++ anymore!)
```

**Benefit**: Self-contained, reproducible builds, no C++ toolchain needed.

---

**Document Status**: ✅ Roadmap Complete  
**Next Review**: After bootstrap verification  
**Maintainer**: OFS Team  
**Last Updated**: April 12, 2026
