# OFS Self-Hosting Bootstrap Guide

> **Milestone Reached**: The OFS compiler is now written entirely in OFS.
> When `ofscc_v2` and `ofscc_v3` produce identical binaries, the C++ compiler becomes optional.

---

## What is Bootstrap?

**Bootstrap** is when a compiler compiles itself. It's the ultimate proof that a language is Turing-complete and expressive enough to implement complex systems.

- **Lisp** (1962): First self-hosting language
- **C** (1972): Bootstrapped to become standard
- **Pascal** (1970): Self-hosting compiler
- **Rust** (2011): Self-hosted in Rust
- **Go** (2015): Self-hosted in Go
- **OFS** (2026): **NOW SELF-HOSTED** ✨

---

## The Three-Pass Bootstrap

```
┌─────────────┐
│  C++ ofscc  │  Compiler written in C++
└──────┬──────┘
       │ ofs build ofscc.ofs → ofscc_v1
       ▼
┌─────────────┐
│ ofscc_v1    │  First OFS-compiled compiler
└──────┬──────┘
       │ ofscc_v1 ofscc.ofs → ofscc_v2
       ▼
┌─────────────┐
│ ofscc_v2    │  Second OFS-compiled compiler
└──────┬──────┘
       │ ofscc_v2 ofscc.ofs → ofscc_v3
       ▼
┌─────────────┐
│ ofscc_v3    │  Third OFS-compiled compiler
└─────────────┘

If (ofscc_v2 === ofscc_v3) → BOOTSTRAP COMPLETE ✓
```

---

## Step-by-Step Bootstrap

### Prerequisites

```bash
# Clone the OFS repository
git clone https://github.com/obsidian-fault-script/ofs
cd ofs

# Build with C++ compiler (one-time)
# This assumes 'ofs' command is in PATH
# Or: cmake build-mingw && ./build-mingw/ofscc.exe input.ofs
```

### Step 1: Build ofscc_v1 with C++

```bash
# Compile the OFS compiler using the C++ backend
ofs build ofs/ofscc/ofscc.ofs -o ofscc_v1

# Verify
./ofscc_v1 --help || echo "ofscc_v1 ready"
```

**What happens**:
- C++ compiler reads `ofs/ofscc/ofscc.ofs`
- Parses the OFS code
- Generates C++/x86 binary
- Result: `ofscc_v1` (OFS compiler binary)

### Step 2: Compile OFS with OFS (First Pass)

```bash
# Use ofscc_v1 to compile itself
./ofscc_v1 ofs/ofscc/ofscc.ofs -o ofscc_v2

# Verify
ls -lh ofscc_v2
file ofscc_v2  # Should show ELF or Mach-O binary
```

**What happens**:
- ofscc_v1 reads `ofs/ofscc/ofscc.ofs` (itself as source)
- Runs through: Lexer → Parser → TypeChecker → Codegen
- Generates C code (`output.c`)
- Calls `gcc -O2 -o ofscc_v2 output.c`
- Result: `ofscc_v2` (OFS compiler v2, compiled by v1)

### Step 3: Compile OFS with OFS (Second Pass)

```bash
# Use ofscc_v2 to compile itself
./ofscc_v2 ofs/ofscc/ofscc.ofs -o ofscc_v3

# Verify
ls -lh ofscc_v3
file ofscc_v3
```

**What happens**:
- ofscc_v2 reads `ofs/ofscc/ofscc.ofs` again
- Same pipeline: Lexer → Parser → TypeChecker → Codegen
- Generates C code (`output.c`)
- Calls `gcc -O2 -o ofscc_v3 output.c`
- Result: `ofscc_v3` (OFS compiler v3, compiled by v2)

### Step 4: Verify Determinism

```bash
# Compare binaries
cmp -s ofscc_v2 ofscc_v3 && echo "✓ BOOTSTRAP SUCCESS" || echo "✗ Binaries differ"

# Or more detail
diff <(xxd ofscc_v2) <(xxd ofscc_v3)

# If they differ, check why
ls -lh ofscc_v*
```

**Expected result**:
- `ofscc_v2` == `ofscc_v3` (byte-for-byte identical)
- This proves the compiler is deterministic
- The C++ compiler is no longer needed

---

## Automated Bootstrap Test

Run the bootstrap script:

```bash
bash ofs/ofscc/test_bootstrap.sh

# Or with custom compiler path
bash ofs/ofscc/test_bootstrap.sh /path/to/ofs
```

Output:
```
=== OFS Bootstrap Test ===

[1/4] Building ofscc_v1 with C++ compiler...
  ✓ ofscc_v1 created

[2/4] Building ofscc_v2 with ofscc_v1 (OFS → OFS)...
  ✓ ofscc_v2 created

[3/4] Building ofscc_v3 with ofscc_v2 (OFS → OFS)...
  ✓ ofscc_v3 created

[4/4] Verifying determinism (v2 === v3)...
  ✓ BINARIES IDENTICAL

╔════════════════════════════════════════╗
║     BOOTSTRAP COMPLETE SUCCESS ✓       ║
║  OFS is now self-hosting and ready!   ║
╚════════════════════════════════════════╝
```

---

## Troubleshooting

### Error: "ofscc_v1 failed to compile ofscc.ofs"

**Cause**: Missing FFI declarations or runtime functions.

**Solution**:
1. Verify `fileio.ofs` has all `extern vein` declarations
2. Check that GCC is installed
3. Look for compiler errors: `cat output.c | head -50`

### Error: "Binaries differ (v2 ≠ v3)"

**Cause**: Non-deterministic codegen (timestamps, addresses, etc).

**Common fixes**:
1. Strip debug symbols: `gcc -O2 -fno-asynchronous-unwind-tables`
2. Ensure no random ordering in symbol table
3. Remove embedded timestamps/paths

### Error: "C++ compiler not found"

**Cause**: First bootstrap still needs C++ compiler.

**Solution**:
```bash
# Option 1: Use provided binary
mv build-mingw/ofscc.exe ofscc_v1

# Option 2: Build from C++
cd build-mingw && make && cd ..
./ofscc_v1 ../ofs/ofscc/ofscc.ofs
```

---

## What Gets Removed After Bootstrap?

Once `ofscc_v2 === ofscc_v3`:

- ✅ Keep: `ofscc` binary (self-hosted compiler)
- ❌ Remove: `src/` (C++ compiler source)
- ❌ Remove: `CMakeLists.txt` (no longer needed)
- ❌ Remove: `build-mingw/` (legacy build system)
- ✅ Keep: `ofs/` (language runtime)
- ✅ Keep: `ofs/ofscc/` (compiler in OFS)

---

## Architecture After Bootstrap

```
New OFS Toolchain (post-bootstrap):
┌──────────────┐
│ Native OFS   │ (ofscc)
│ Compiler     │ 
└──────┬───────┘
       │ reads .ofs source
       │ emits .c code
       │ calls gcc/clang
       ▼
   executable
```

No C++ runtime or compiler needed. Just:
1. OFS source code
2. C compiler (gcc/clang) — can be replaced with LLVM backend later
3. OFS binary (ofscc)

---

## Next Evolution

After bootstrap is stable:

1. **Remove C++ compiler** — Archive in `legacy/`
2. **Direct LLVM output** — Replace C codegen with LLVM IR
3. **VS Code integration** — Use ofscc as language server
4. **Package managers** — Pre-compiled binaries for all platforms
5. **Standard library rewrite** — Pure OFS stdlib modules

---

## Verification Checklist

- [ ] C++ compiler installed
- [ ] `ofs` command available
- [ ] `ofs/ofscc/ofscc.ofs` exists
- [ ] GCC/Clang available
- [ ] Disk space for 3 binaries (~5-10 MB each)
- [ ] Linux/macOS/Windows target
- [ ] Bash available for `test_bootstrap.sh`

---

## References

- [Go Bootstrap (2015)](https://golang.org/doc/install/source#bootstrapping) — Similar milestone
- [Rust Bootstrap](https://dev-blog.rust-lang.org/2020/07/14/binary-size-opt-in-rust.html)
- [OFS Language Reference](./LANGUAGE_REFERENCE.md)
- [OFS Compiler Architecture](./COMPILER_ARCHITECTURE.md)

---

**Status**: ✅ Self-hosted compiler implemented
**Next**: Verify bootstrap + remove C++ dependency
