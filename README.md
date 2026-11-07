<div align="center">
  <img src="docs/ofs-256.png" width="112" height="112" alt="Obsidian Fault Script icon">

  # Obsidian Fault Script

  **A self-hosted compiled language for native programs and the Web.**

  [![Build](https://github.com/Samwns/Obsidian-Fault-Script/actions/workflows/build.yml/badge.svg)](https://github.com/Samwns/Obsidian-Fault-Script/actions/workflows/build.yml)
  [![Pages](https://github.com/Samwns/Obsidian-Fault-Script/actions/workflows/pages.yml/badge.svg)](https://samwns.github.io/Obsidian-Fault-Script/docs/)
  [![Release](https://img.shields.io/github/v/release/Samwns/Obsidian-Fault-Script?display_name=tag&sort=semver)](https://github.com/Samwns/Obsidian-Fault-Script/releases/latest)
  [![License: Apache 2.0](https://img.shields.io/badge/License-Apache_2.0-blue.svg)](LICENSE)
  [![LLVM](https://img.shields.io/badge/backend-LLVM-7c5cff)](docs/COMPILER_ARCHITECTURE.md)
  [![Support on Ko-fi](https://img.shields.io/badge/Ko--fi-Support-FF5E5B?logo=kofi&logoColor=white)](https://ko-fi.com/samwns)

  [**Official Site**](https://samwns.github.io/Obsidian-Fault-Script/docs/)
  · [**Documentation**](https://samwns.github.io/Obsidian-Fault-Script/docs/getting-started.html)
  · [**Downloads**](https://github.com/Samwns/Obsidian-Fault-Script/releases/latest)
  · [**Benchmark**](https://samwns.github.io/Obsidian-Fault-Script/docs/benchmark.html)
  · [**Trademark Policy**](TRADEMARK.md)
  · [**Sponsor / Apoiar**](https://ko-fi.com/samwns)
</div>

<br>

<div align="center">
  <a href="https://ko-fi.com/samwns" target="_blank" rel="noopener noreferrer">
    <img src="https://storage.ko-fi.com/cdn/kofi3.png?v=3" height="36" alt="Apoie no Ko-fi / Support on Ko-fi" />
  </a>
</div>

## Android (aarch64) e OAL

O OFS também gera APKs para Android (aarch64):

```sh
ofs build --target android app.oal -o app-debug.apk
```

- **APK 100% OFS:** sem Java e sem C no código do app. O `NativeActivity` é exportado por `android.ofs`, e a chamada à API do Android (JNI) sai de `fault_addr`/`fault_call`.
- **OAL (Obsidian Android Language, `.oal`):** sub-linguagem que descreve o que é próprio do Android: manifesto, telas de toque, estilos, ações e ciclo de vida. A lógica continua em `.ofs`. Ela é irmã da ODL, da OES e da OLL.
- **Exemplo real:** o [M.O.O.N](Moon/README.md), um executor de visual novels para Android escrito inteiramente em OFS, com a interface em OAL.

Documentação:
- [docs/ANDROID.md](docs/ANDROID.md): requisitos, build, instalação e como o app roda.
- [docs/OAL_REFERENCE.md](docs/OAL_REFERENCE.md): referência da OAL v1.