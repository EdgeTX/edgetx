# Building EdgeTX

This section contains step-by-step build instructions for every supported platform. Whether you want to build a custom firmware binary, run the EdgeTX simulator, or contribute to development, pick the guide that matches your environment.

## Available build guides

| Guide | Platform |
|---|---|
| [In a Web Browser (CodeSpaces)](codespaces.md) | Any OS — no local setup required |
| [Windows](windows.md) | Windows — Visual Studio 2022 + Ninja |
| [Windows (Docker)](windows-docker.md) | Windows 10 — Docker Desktop |
| [Linux (Ubuntu 22.04)](linux-ubuntu-22.04.md) | Ubuntu 22.04 (bare-metal, VM, or WSL2) |
| [Linux (Ubuntu 24.04)](linux-ubuntu-24.04.md) | Ubuntu 24.04 (bare-metal, VM, or WSL2) |
| [Linux (WSL / Ubuntu 20.04)](linux-wsl.md) | Windows 10/11 — WSL2 with Ubuntu 20.04 |
| [macOS (Sequoia / Tahoe)](macos-sequoia.md) | macOS 15 (Sequoia) and macOS 26 (Tahoe) |
| [macOS (Sonoma)](macos-sonoma.md) | macOS 14.1 (Sonoma) |
| [macOS (Catalina)](macos-catalina.md) | macOS 10.15 (Catalina) |
| [Compilation Options](compilation-options.md) | Reference list of all CMake build flags |
| [Qt Creator (Obsolete)](qt-creator.md) | Historical — no longer maintained |

## Notes

- The document here is meant to help you develop or test changes to EdgeTX on your PC, not to build flight/radio safe version of binaries.
- For a list of all available CMake build options, see [Compilation Options](compilation-options.md).
- For radio-specific build flags (`PCB`, `PCBREV`), refer to the build scripts at [https://github.com/EdgeTX/edgetx/tree/main/tools](https://github.com/EdgeTX/edgetx/tree/main/tools).
