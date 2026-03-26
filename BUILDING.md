# Building EdgeTX

This guide covers how to build EdgeTX firmware, the SDL simulator, and the
Companion application from source. For platform-specific environment setup
(installing toolchains, Qt, etc.), see the guides in [`docs/building/`](docs/building/).

## Prerequisites

| Component | Required for | Notes |
|-----------|-------------|-------|
| **CMake >= 3.21** | Everything | |
| **arm-none-eabi-gcc** | Firmware | 14.2.rel1 recommended |
| **Python >= 3.10** | Firmware, Companion | Code generation at build time |
| **Qt 6** | Companion | 6.9+ recommended |
| **SDL2** | Simulator | |
| **Ninja** (optional) | Everything | Faster than Make |

### Python dependencies

The build invokes Python scripts for code generation (YAML parsers, hardware
definitions, bitmap encoding, etc.). Install the required packages with
[uv](https://docs.astral.sh/uv/):

```bash
uv sync
source .venv/bin/activate   # CMake needs the venv activated to find Python
```

The virtual environment must stay activated when running CMake, so that the
build can find the installed packages. On Windows, use `.venv\Scripts\activate`
instead.

## Quick start with CMake Presets

The repository ships CMake presets for all supported radios and build types.
These work in VSCode (CMake Tools), CLion, and the command line.

### Firmware

```bash
# Configure for a specific radio
cmake --preset tx16s

# Build
cmake --build build/tx16s --target firmware
```

Replace `tx16s` with any supported radio preset. Run `cmake --list-presets` to
see all available presets. A few examples:

| Preset | Radio |
|--------|-------|
| `tx16s` | RadioMaster TX16S |
| `tx16smk3` | RadioMaster TX16S MK3 |
| `boxer` | RadioMaster Boxer |
| `pocket` | RadioMaster Pocket |
| `zorro` | RadioMaster Zorro |
| `t20` | Jumper T20 |
| `t14` | Jumper T14 |
| `tpro` | Jumper T-Pro |
| `x9dp2019` | FrSky X9D+ 2019 |
| `x10` | FrSky X10 |
| `nv14` | FlySky NV14 |
| `pl18` | FlySky PL18 |


### Simulator

```bash
cmake --preset simu -DPCB=X10 -DPCBREV=TX16S
cmake --build build/simu --target simu
```

### Companion

Companion embeds a radio simulator that uses WASM plugins — one per radio
target. With presets, this requires two build directories: one for the WASM
plugins and one for Companion itself.

Build the WASM plugins for all radios:

```bash
tools/build-wasm-modules.sh

# or limit to specific radios
FLAVOR="tx16s;boxer" tools/build-wasm-modules.sh
```

Then build Companion:

```bash
cmake --preset companion
cmake --build build/companion --target companion --parallel
```

When developing for a specific radio, you can build just that one WASM plugin
using the superbuild instead (see [Manual CMake](#manual-cmake-without-presets)
below):

```bash
cmake -S . -B build-tx16s -DPCB=X10 -DPCBREV=TX16S
cmake --build build-tx16s --target wasi-module --parallel
cmake --build build-tx16s --target companion --parallel
```

### Tests

```bash
cmake --preset simu -DPCB=X10 -DPCBREV=TX16S
cmake --build build/simu --target tests-radio
```

## Manual CMake (without presets)

If you prefer to invoke CMake directly, the superbuild takes care of toolchain
selection and sub-build configuration automatically:

```bash
# configure
cmake -S . -B build-tx16s -DPCB=X10 -DPCBREV=TX16S

# build firmware
cmake --build build-tx16s --target firmware --parallel

# build simulator (same build directory)
cmake --build build-tx16s --target simu --parallel

# run tests
cmake --build build-tx16s --target tests-radio --parallel
```

## Key CMake variables

| Variable | Default | Description |
|----------|---------|-------------|
| `PCB` | (required) | Target radio family (X7, X10, PL18, ...) |
| `PCBREV` | (varies) | Radio variant within a PCB family (TX16S, BOXER, ...) |
| `CMAKE_BUILD_TYPE` | | `Release` or `Debug` (native builds only; firmware ELF always includes debug symbols, stripped for .bin/.uf2) |
| `TRANSLATIONS` | EN | Firmware language (EN, FR, DE, ES, ...) |
| `DEFAULT_MODE` | | Stick mode 1-4 |
| `LUA` | ON | Lua scripting support |

For the full list, see [`docs/building/compilation-options.md`](docs/building/compilation-options.md).

## FetchContent dependency cache

Downloaded third-party dependencies (googletest, yaml-cpp, imgui, etc.) are
cached in `.cache/fetchcontent/` at the repository root. This cache is shared
across all build directories so that switching targets or doing clean rebuilds
does not re-download dependencies.

To override the cache location, set the `FETCHCONTENT_BASE_DIR` environment
variable or pass it as a CMake variable:

```bash
export FETCHCONTENT_BASE_DIR=~/edgetx-deps
cmake --preset tx16s
```

## CI builds

CI uses the scripts in `tools/`:

| Script | Purpose |
|--------|---------|
| `build-gh.sh` | Firmware release builds (all targets) |
| `commit-tests.sh` | Debug builds with warnings-as-errors |
| `build-companion.sh` | Companion app (Linux, macOS, Windows) |
| `build-wasm-modules.sh` | WebAssembly simulator modules |
| `build-common.sh` | Shared helpers and target definitions |

These scripts use `FLAVOR` environment variables and are tailored for GitHub
Actions. For local development, prefer the CMake presets above.
