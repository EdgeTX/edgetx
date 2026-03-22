# EdgeTX Developer Documentation

Welcome to the EdgeTX developer wiki!

![EdgeTX Logo](assets/images/EdgeTX_logo.png)

EdgeTX is an open-source firmware for RC radio transmitters. This documentation covers technical topics for developers and contributors: how to build EdgeTX from source, hardware reference material, hardware modification guides, and internal software architecture.

## What's in this wiki

- **Contributing** — Getting started with Git, the GitHub workflow, and a command reference for common Git operations.
- **Building EdgeTX** — Step-by-step build instructions for every supported platform (Linux, macOS, Windows, Docker, and GitHub CodeSpaces), plus a reference for all CMake compilation options.
- **Hardware Reference** — Radio hardware specifications, DMA mappings and IRQ usage tables for specific radios.
- **Hardware Mods** — Community hardware modification guides for FlySky gimbals, FRM303 module, PL18 / Paladin EV, NB4+, and radio recovery (unbricking).
- **Development** — Internal architecture documentation covering CLI commands, customisable switches, control inputs, external module protocols, YAML storage, and mixer synchronisation.

## Quick links

| Section | Description |
|---|---|
| [Contributing](contributing/getting-started-with-git.md) | Start here if you are new to Git or GitHub |
| [Building — CodeSpaces](building/codespaces.md) | Build in a web browser with no local setup |
| [Building — Linux (Ubuntu 24.04)](building/linux-ubuntu-24.04.md) | Recommended local build environment |
| [Building — Windows](building/windows.md) | Native Windows build using Visual Studio + Ninja |
| [Unbrick your radio](mods/unbrick.md) | Recover from a failed flash using STM32CubeProgrammer |
| [Radio Specifications](hardware/radio-specs.md) | Hardware specs for supported radios |
