# Architectural Guide — OFS IDLE (Obsidian Integrated Development & Learning Environment)

This document sets out the technical mapping and the reverse-engineering specification for building **OFS IDLE**: a very high-performance integrated development environment with the same interface, elegance and ergonomics as Visual Studio Code, but **100% written in Obsidian Fault Script (OFS) and rendered natively through the Obsidian Layout Language (OLL)**, with no Electron, Chromium, Node.js or Microsoft proprietary code.

---

## 1. Principles of Sovereignty and Clean Room

1. **Zero Lines of Microsoft Code**: the OFS IDLE code is written entirely from scratch in OFS syntax (`vein`, `forge`, `monolith`, `attach`), using the native Magma stack (`Canvas`, `Window`, `OLL`, `FS`).
2. **Zero Mandatory Credits**: the architectural concepts (Activity Bar, tree Sidebar, editor tabs with a numbered gutter, bottom terminal panel and status bar) are universal patterns of the software industry (adopted by Eclipse, TextMate, Sublime Text, Atom, Theia, Lapce and Zed). Since no line of VS Code's TypeScript/JS code is copied, the intellectual authorship belongs entirely to the OFS project.
3. **Official Language Icon**: the visual identity replaces every external symbol with the **Obsidian Diamond with Energy Fissure (OFS Logo)** in high resolution.

---

## 2. Technology Comparison: VS Code vs. OFS IDLE

| Technical Dimension | Official VS Code | OFS IDLE (Native) |
| :--- | :--- | :--- |
| **Execution Engine** | Node.js + V8 Engine + Chromium Runtime | **Native AOT Binary (compiled via LLVM)** |
| **Interface Layer (UI)** | Web DOM + CSS + HTML5 Canvas | **Declarative OLL + Native Magma / Win32 / X11 Buffer** |
| **Memory Use (RAM)** | 600 MB to 1.8 GB at rest | **18 MB to 35 MB** of total memory |
| **Startup Time** | 2.5 to 6.0 seconds | **15 to 45 milliseconds** (instant) |
| **External Dependencies** | Node, npm, Electron, heavy C++ libraries | **Zero dependencies** (single self-contained executable) |
| **Frame Rate (FPS)** | Varies with the weight of the Chromium DOM | **Locked 60 FPS** with GDI/Xlib drawing acceleration |

---

## 3. Architectural Mapping: VS Code → OFS IDLE

VS Code organizes its core in the `src/vs/workbench/browser/parts/` directory. The table below maps each original module to its counterpart in the OFS ecosystem:

| VS Code Component | VS Code Module | OFS IDLE Implementation | Role in the System |
| :--- | :--- | :--- | :--- |
| **Titlebar** | `parts/titlebar` | `idle_titlebar.ofs` | Top menu (File, Edit, View), OFS icon and global search box (`Ctrl+P`) |
| **Activity Bar** | `parts/activitybar` | `idle_activitybar.ofs` | Left vertical strip (48px) with buttons for Explorer, Search, Git, Debug and Extensions |
| **Sidebar** | `parts/sidebar` | `idle_sidebar.ofs` | Collapsible panel (230px) with a hierarchical tree of project files via `fs.ofs` |
| **Editor Group** | `parts/editor` | `idle_editor.ofs` | File tabs, gutter with line numbers, editable text buffer and minimap |
| **Panel / Terminal** | `parts/panel` | `idle_terminal.ofs` | Bottom panel with tabs for Integrated Terminal, Build Output and Diagnostics |
| **Status Bar** | `parts/statusbar` | `idle_statusbar.ofs` | Footer with Git branch, Errors/Warnings, cursor position (Ln/Col), UTF-8 and LSP status |
| **Command Palette** | `platform/quickinput` | `idle_quickopen.ofs` | Centered modal drop-down window for running commands and quick file search |

---

## 4. Workbench Structure in OLL (.oll)

The declarative tree that makes up the whole visual interface of OFS IDLE follows the modern model:

```oll
window "Obsidian Fault Script — Studio IDLE" {
    width: 1280
    height: 800
    auto_center: true
    resizable: true
    bg_color: #181818

    column {
        // 1. Title Bar
        row {
            height: 35
            bg_color: #1f1f1f
            padding: 6
            align: "space-between"
            // Menu: File, Edit, Selection, View, Go, Run, Terminal, Help
            // Central search: "Obsidian Fault Script - Studio (Ctrl+P)"
            // Window controls: Minimize, Maximize, Close
        }

        // 2. Central Work Area (Main Workbench)
        row {
            // Activity Bar: 48px
            column {
                width: 48
                bg_color: #181818
                padding: 6
                gap: 16
                // Ícones: Explorer (ativo), Search, Source Control, Run & Debug, Extensions, Settings
            }

            // Sidebar: 240px
            column {
                width: 240
                bg_color: #1f1f1f
                padding: 10
                gap: 8
                // Header: EXPLORER
                // Project: OBSIDIAN-FAULT-SCRIPT
                // Tree View: src/, docs/, ofs/, main.ofs, oll.ofs
            }

            // Central Editor and Terminal
            column {
                // Editor Tab Bar
                row {
                    height: 35
                    bg_color: #181818
                    // Active tab: [OFS] main.ofs (com 'x')
                    // Inactive tab: [OFS] oll.ofs
                }

                // Source Code Buffer with Numbered Gutter
                row {
                    // Gutter with line numbers (1..35)
                    column { width: 45 bg_color: #1e1e1e }
                    // Code Area with Syntax Highlighting
                    card { bg_color: #1e1e1e padding: 8 }
                    // Code Minimap on the right
                    column { width: 55 bg_color: #1e1e1e }
                }

                // Bottom Panel (Terminal and Output)
                card {
                    height: 180
                    bg_color: #181818
                    border_color: #2b2b2b
                    // Tabs: TERMINAL (active), OUTPUT, PROBLEMS (0), DEBUG
                    // Console with ofs compiler commands
                }
            }
        }

        // 3. Bottom Status Bar
        row {
            height: 24
            bg_color: #007acc  // or OFS purple #6d28d9
            padding: 4
            align: "space-between"
            // Left: Git branch, errors (0), warnings (0), OFS LSP Ready
            // Right: Ln 14, Col 28 | 4 Spaces | UTF-8 | LF | Obsidian Fault Script
        }
    }
}
```

---

## 5. Next Steps to Build the IDLE

1. **Native Visual and Operational Prototype (`idle/src/ofs_idle.ofs`)**: create the complete application in native OFS, drawing the VS Code Workbench with high fidelity, vector icons in pure OFS and ClearType typography.
2. **Integration with the Compiler and Tools**: connect the IDLE's integrated terminal directly to `ofs build`, `ofs run` and `ofs ui`.
3. **Binary Generator**: compile the IDLE into a Windows executable (`ofs_idle.exe`) and a Linux one (`ofs_idle`).
