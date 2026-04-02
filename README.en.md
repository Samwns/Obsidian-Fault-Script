# Obsidian Fault Script (OFS)

A compiled, statically typed language designed to help people learn programming with fast feedback.
Write simple code, run it with one command, and move into lower-level concepts when you are ready.

## Start in 5 minutes

1. Download the latest version from [Releases](https://github.com/Samwns/Obsidian-Fault-Script/releases/latest).
2. Install the binary for your platform.
3. (Optional) Install the VS Code extension for highlight, diagnostics, and Run button support.

Release files:

| Platform | File |
|---|---|
| Linux (x64) | `ofs-linux-x64-installer.tar.gz` |
| Windows (x64) | `ofs-windows-x64-installer.exe` |
| macOS (ARM) | `ofs-macos-arm64-installer.pkg` |
| VS Code | `ofs-vscode-extension.vsix` |

Quick install:

```bash
# Linux
tar -xzf ofs-linux-x64-installer.tar.gz
chmod +x install.sh
./install.sh

# macOS
sudo installer -pkg ofs-macos-arm64-installer.pkg -target /

# VS Code
code --install-extension ofs-vscode-extension.vsix
```

On Windows, run `ofs-windows-x64-installer.exe`.

## Your first program

Create `hello.ofs`:

```ofs
core main() {
    echo("Hello, OFS!")
}
```

Run:

```bash
ofs hello.ofs
```

## Beginner track (dynamic)

Choose a level and move at your own pace:

1. Level 1: Variables and types
2. Level 2: Decisions and loops
3. Level 3: Functions and structure
4. Level 4: Memory and performance

Level 1 (15 min):

```ofs
core main() {
    forge name: obsidian = "Nina"
    forge age: stone = 17
    forge height: crystal = 1.68
    echo(name)
    echo(age)
    echo(height)
}
```

Quick challenge:
- Change the values and print a sentence with name and age.

Level 2 (20 min):

```ofs
core main() {
    forge score: stone = 8

    if (score >= 7) {
        echo("Passed")
    } else {
        echo("Review content")
    }

    cycle (forge i = 1; i <= 3; i++) {
        echo(i)
    }
}
```

Quick challenge:
- Print numbers from 1 to 10 and the total sum.

Level 3 (20 min):

```ofs
vein sum(a: stone, b: stone) -> stone {
    return a + b
}

core main() {
    forge result = sum(12, 30)
    echo(result)
}
```

Quick challenge:
- Create a function that receives a name and returns a greeting.

Level 4 (advanced, when you want):
- `fracture` and `shard` for safe pointers
- `extern` to call C functions
- `attach` to reuse modules

## Essential commands

```bash
ofs file.ofs                    # run directly
ofs check file.ofs              # validate types and syntax
ofs build file.ofs -o app       # build native binary
ofs ast file.ofs                # show syntax tree
ofs ir file.ofs                 # show LLVM IR
ofs help                        # help
```

## Ready-to-run examples

Examples are in `ofs/examples/`:

- `hello.ofs`
- `fizzbuzz.ofs`
- `calculator.ofs`
- `showcase.ofs`
- `packages_demo.ofs`

Run full showcase:

```bash
ofs ofs/examples/showcase.ofs
```

## Packages and libraries

Sample libraries in this project:

- `terminal-colors`
- `memory-modes`

Local flow:

```bash
./packages/build_packages.sh
./packages/install_library.sh packages/dist/terminal-colors.tar.gz
./packages/install_library.sh packages/dist/memory-modes.tar.gz
export OFS_LIB_PATH="$HOME/.ofs/libs"
ofs ofs/examples/packages_demo.ofs
```

## Documentation

- [Getting started](docs/GETTING_STARTED.md)
- [Language reference](docs/LANGUAGE_REFERENCE.md)
- [OFS beginner journey](OFS_JORNADA_INICIANTE.md)
- [Changelog](docs/CHANGELOG.md)

## Contributing (local build)

Requirements:

- CMake 3.20+
- C++17 compiler
- LLVM 17+

```bash
git clone https://github.com/Samwns/Obsidian-Fault-Script.git
cd Obsidian-Fault-Script/ofs
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

## License

MIT
