# EdgeTX Radio Firmware Architecture

## Guiding Principles

The EdgeTX firmware is undergoing a long-term modernization effort to cleanly
separate **core logic** from **hardware concerns**. The architecture is built
around four key concepts:

| Concept  | What it represents                                     | Goal location          |
|----------|--------------------------------------------------------|------------------------|
| **CPU**  | The CPU architecture and MCU ecosystem                 | `cpu/`                 |
| **Board**| A family of similar hardware boards                    | `boards/`              |
| **Radio**| The specific device a user installs firmware on        | `radios/`              |
| **HAL**  | Hardware abstraction layer interfaces                  | `hal/`                 |

The CPU layer is **conceptually layered** -- it spans the CPU architecture
(ARM), the core variant (Cortex-M4, Cortex-M7), and the MCU family (STM32F4,
STM32H7). Code should be placed at the most general level where it applies --
for example, an ARM SysTick driver belongs to the ARM level, not the STM32
level.

Everything else in `radio/src/` (gui, pulses, telemetry, storage, lua, io,
tasks, etc.) is **generic application code** that must depend only on the HAL
and never on CPU, board, or radio specifics directly.

```
 +---------------------------------------------------------+
 |                  Generic Application Code               |
 |  gui/ pulses/ telemetry/ storage/ lua/ io/ tasks/ ...   |
 +---------------------------------------------------------+
                          |
                     uses HAL only
                          |
                    +-----v------+
                    |    hal/    |   Platform-independent interfaces
                    +-----+------+
                          |
           +--------------+--------------+
           |              |              |
     +-----v------+ +----v-------+ +----v-------+
     |    CPU     | |   Board    | |   Radio    |
     | cpu/       | | boards/    | | radios/    |
     |  arm/      | |  taranis/  | |  x9d-2019/ |
     |  stm32/    | |  horus/    | |  tx16s/    |
     |   f2/      | |  jumper-   | |  t15pro/   |
     |   f4/      | |  h750/     | |  ...       |
     |   h7/      | |            | |            |
     |            | |  rm-h750/  | |            |
     +------------+ +------------+ +------------+
```

## HAL: The Contract

The HAL (`hal/`) defines the interfaces that all hardware-dependent code must
implement. It is the **only** dependency that generic application code should
have on hardware.

HAL interfaces use two patterns:

### VTable (struct of function pointers)

Used for complex subsystems that may have multiple driver implementations or
need opaque per-instance state:

```c
// hal/serial_driver.h
typedef struct {
  void* (*init)(void* hw_def, const etx_serial_init* params);
  void  (*deinit)(void* ctx);
  void  (*sendByte)(void* ctx, uint8_t byte);
  bool  (*txCompleted)(void* ctx);
  // ...
} etx_serial_driver_t;
```

Examples: `adc_driver.h`, `serial_driver.h`, `flash_driver.h`,
`timer_driver.h`, `module_driver.h`, `fatfs_diskio.h`.

### Plain C functions

Used for simpler, singleton hardware features:

```c
// hal/audio_driver.h
void audioSetCurrentVolume(uint8_t vol);
bool isHeadphonePlugged();
```

Examples: `audio_driver.h`, `gpio.h`, `watchdog_driver.h`,
`rotary_encoder.h`, `i2c_driver.h`, `usb_driver.h`.

## CPU Layer

The CPU layer covers the CPU architecture, core variant, and MCU family.
Currently only **ARM Cortex-M on STM32** is supported.

### Goal structure

The CPU layer lives under `cpu/` with STM32-family-specific code nested
under `cpu/stm32/`:

```
cpu/
  arm/                      # ARM-generic code (SysTick, NVIC, fault handlers,
                            #   MPU, FPU init, exception model, etc.)
  stm32/                    # STM32-common code (peripheral HAL/LL drivers
                            #   shared across STM32 families)
    f2/                     # STM32F2-specific code
    f4/                     # STM32F4-specific code
    h7/                     # STM32H7-specific code (including H750)
    h7rs/                   # STM32H7RS-specific code
```

Code must be placed at the **most general level** where it applies. A driver
that works on any ARM Cortex-M (e.g. SysTick, basic NVIC management) belongs
in `arm/`, not in `stm32/`. Only code that uses STM32-specific peripheral
registers belongs in `stm32/` or the family-specific directories.

### Current state

All CPU layer code lives in `cpu/stm32/` (moved from `targets/common/arm/stm32/`)
with insufficient separation between ARM-generic and STM32-specific concerns.
Drivers currently located there include:

- `stm32_adc.cpp` - ADC with DMA
- `stm32_serial_driver.cpp`, `stm32_usart_driver.cpp` - UART/USART
- `stm32_spi.cpp`, `stm32_i2c_driver.cpp` - SPI and I2C
- `stm32_timer.cpp`, `stm32_pulse_driver.cpp` - Timers and PWM
- `stm32_gpio_driver.cpp` - GPIO control
- `audio_dac_driver.cpp` - Audio DAC output
- `flash_driver.cpp` - Internal flash
- `diskio_sdio.cpp`, `diskio_spi.cpp` - SD card interfaces
- `usb_driver.cpp` - USB device
- `rtc_driver.cpp` - Real-time clock
- `watchdog_driver.cpp` - Watchdog timer

As part of the modernization, these should be audited and moved to the correct
level under `cpu/`. Some of these drivers are truly STM32-specific (peripheral
register access), while others operate at the ARM or Cortex-M level and should
be factored out into `cpu/arm/`.

CPU-family variations (F2, F4, H7, H7RS) are handled via subdirectories and
HAL configuration headers (`stm32f4xx_hal_conf.h`, etc.).

To add a **new CPU** (e.g. ESP32, RP2040), one would add a new directory
under `cpu/` and provide driver implementations that satisfy the HAL
interfaces.

## Board Layer

A board family groups radios that share significant hardware design. Boards
implement the "middle layer" between raw CPU drivers and the radio-specific
pin/peripheral assignments.

**Location:** `boards/`

### Goal structure

Every board family should live under `boards/`. Each board directory provides
LCD drivers, touch drivers, backlight, haptic, audio, SDRAM, external flash,
and other peripherals specific to that hardware family:

```
boards/
  generic_stm32/            # Shared logic for all STM32-based boards
  jumper-h750/              # Jumper boards (STM32H750)
  rm-h750/                  # RadioMaster boards (STM32H750)
  taranis/                  # Taranis board family (to be migrated)
  horus/                    # Horus board family (to be migrated)
  st16/                     # ST16 board (to be migrated)
  pl18/                     # PL18 board (to be migrated)
  pa01/                     # PA01 board (to be migrated)
  ...
```

`generic_stm32/` provides common board-level logic used across all STM32
radios: analog input processing, switch debouncing, module port management,
battery monitoring, RGB LEDs, linker scripts, etc.

### Current state

Only the newer board families (`jumper-h750`, `rm-h750`) are properly
extracted into `boards/`. Older board families (Taranis, Horus, ST16, PL18,
PA01) still have their board-level code **mixed into target directories**
under `targets/`. Migrating these into `boards/` is a key part of the
modernization effort.

## Radio Layer

A radio is a specific device (e.g. TX16S, X9D+, T15 Pro).

### Goal structure

Each radio gets its own directory under `radios/`, containing its CMake target
definition and its JSON hardware definition:

```
radios/
  x9d-2019/
    CMakeLists.txt            # CMake target: selects CPU, board, feature flags
    hw_def.json               # Hardware definition (pins, peripherals, etc.)
  tx16s/
    CMakeLists.txt
    hw_def.json
  t15pro/
    CMakeLists.txt
    hw_def.json
  ...
```

Each radio directory is self-contained and lightweight. The `CMakeLists.txt`
composes the radio from existing building blocks:

- Which CPU (e.g. `cpu/stm32f4`)
- Which board family (e.g. `boards/taranis`)
- Radio-specific feature flags (haptic, bluetooth, module types, etc.)

The `hw_def.json` declaratively specifies the radio's hardware configuration:
ADC inputs, GPIO pin mappings, switches, keys, trims, display parameters,
module ports, and feature flags. At build time, Python scripts and Jinja2
templates (`radio/util/hw_defs/`) generate C initialization code (`.inc`
files) from these definitions.

Building a specific radio is a direct CMake target invocation
(e.g. `cmake --build . --target x9d-2019`), not a variable-driven selection
from a shared directory. No `#ifdef` spaghetti to differentiate radios.

### Current state

Today, radios do not have their own directories. They are grouped under
umbrella target directories in `targets/`, selected by the `PCB` CMake
variable and differentiated by `#ifdef` chains:

- `targets/taranis/` - X9D, X9D+, X9D+ 2019, X7, X9Lite, X9Lite-S, XLite,
  XLite-S, XLite-Pro
- `targets/horus/` - X12S
- `targets/st16/`, `targets/pl18/`, `targets/pa01/` - Single-radio targets
- `targets/t15pro/`, `targets/tx15/`, `targets/tx16smk3/` - Already
  close to the goal (one radio per target, linked to a board family)

These target directories mix radio-specific configuration with board-level
code (`board.h`, `board.cpp`, driver files) that should live in `boards/`.
Hardware definitions currently live separately in `boards/hw_defs/*.json`.

### Simulator

**Location:** `simu/`

The simulator provides alternative HAL implementations that replace hardware
with software emulation, allowing the firmware to run on a desktop or in a
browser (via WASM). It implements the same HAL interfaces: simulated ADC,
switches, keys, audio, timers, LCD, FatFS-on-host-filesystem, etc.

The simulator is not a CPU, a board, or a radio -- it is an alternative
**backend** for the HAL that replaces all three with software. It lives as a
peer of `cpu/` and `boards/`:

```
radio/src/
  hal/          # interfaces
  cpu/          # real hardware backend
  boards/       # real hardware backend
  simu/         # simulator backend (replaces cpu + boards)
  radios/       # radio definitions (used by both real and simulated builds)
```

A real firmware build composes `cpu/ + boards/ + radios/<radio>/`. A simulated
build composes `simu/ + radios/<radio>/`.

**Goal: multi-radio simulator binary.** Today, each simulated radio is a
separate compile-time build (selected by `#ifdef`). The long-term goal is a
single simulator binary that can simulate **any radio at runtime**. The radio
selection (LCD dimensions, switch count, input layout, etc.) is driven by the
`hw_def.json` data loaded at startup rather than baked in at compile time.

Most `#ifdef` branches in the firmware exist to produce a small, fast embedded
binary -- constraints that do not apply to the simulator running on desktop
hardware. A multi-radio simulator binary is therefore achievable once the
generic code is fully decoupled from hardware specifics via the HAL.

This also makes the simulator a powerful **validation tool**: if the generic
code compiles once for all radios in a single simulator binary, it proves
there are no remaining `#ifdef RADIO_*` leaks in the generic layer.

## Generic Application Code

All directories in `radio/src/` not listed above contain hardware-independent
logic:

| Directory      | Purpose                                      |
|----------------|----------------------------------------------|
| `gui/`         | User interface (color and B&W variants)      |
| `pulses/`      | RF protocol encoders (PXX, CRSF, DSM, etc.) |
| `telemetry/`   | Telemetry protocol decoders                  |
| `storage/`     | Model/settings persistence                   |
| `lua/`         | Lua scripting engine and API                 |
| `io/`          | Mixer, trainer, serial I/O logic             |
| `tasks/`       | RTOS task definitions                        |
| `os/`          | OS abstraction (FreeRTOS wrappers)           |
| `drivers/`     | Device drivers for external ICs (audio codecs, sensors) |
| `bootloader/`  | Bootloader application code                  |

**This code should call only HAL interfaces**, not CPU or board functions
directly. Violations of this rule are legacy debt to be cleaned up.

## Build System

### Current flow

CMake drives the build with the following flow:

1. **PCB selection** (`-DPCB=TX16S`) routes to the correct target
   `CMakeLists.txt`
2. **Target CMakeLists** sets `CPU_TYPE`, `FLAVOUR`, variant-specific options
3. **CPU-family CMakeLists** enables the correct STM32 peripheral libraries
4. **Board CMakeLists** pulls in the board family code
5. **HAL CMakeLists** triggers code generation from JSON hardware definitions
6. All sources are compiled together into a firmware binary

Key CMake variables:

| Variable           | Example          | Purpose                        |
|--------------------|------------------|--------------------------------|
| `PCB`              | `X9D+`           | Selects the radio model        |
| `TARGET_DIR`       | `taranis`         | Target source directory       |
| `FLAVOUR`          | `x9d+2019`       | Output filename identifier     |
| `CPU_TYPE`         | `STM32F4`        | MCU family                     |
| `CPU_TYPE_FULL`    | `STM32F407xE`    | Full MCU designation           |
| `GUI_DIR`          | `480x272`        | LCD resolution                 |

### Goal

Each radio directory under `radios/` defines a **named CMake target** that can
be built directly (e.g. `cmake --build . --target x9d-2019`). The target
definition in `radios/<radio>/CMakeLists.txt` composes:

- A CPU (e.g. `cpu/stm32/f4`)
- A board family (e.g. `boards/taranis`)
- The colocated `hw_def.json`
- Radio-specific feature flags

This replaces the current `-DPCB=` variable-driven approach and makes radio
configurations explicit, independent, and easy to reason about.

## Current State vs. Goal

### What's done

- HAL interfaces exist for most subsystems (`hal/`)
- JSON hardware definitions cover 50+ radio models
- Code generation from JSON reduces per-radio boilerplate
- Board families exist for newer radios (Jumper H750, RadioMaster H750)
- `generic_stm32` provides shared board logic
- Simulator implements HAL for desktop/WASM testing

### What remains

- **CPU code needs further layering.** It now lives under `cpu/stm32/` but
  ARM-generic and Cortex-M-generic code is still lumped together with
  STM32-specific code. These should be factored out into `cpu/arm/`.
- **Boards** (taranis, horus, st16, pl18, pa01) still have board-level
  code mixed into target directories instead of extracted into `boards/`.
- **Radios lack their own directories.** Multiple radios are grouped under
  umbrella target directories and differentiated by `#ifdef` chains. Each
  radio should get its own directory under `radios/` with a dedicated CMake
  target and colocated JSON hardware definition.
- **Generic code still has `#ifdef` for specific radios** in places where it
  should be calling HAL interfaces instead.
- **Some HAL interfaces are incomplete** -- not all hardware features are
  abstracted yet.
- **`targets/<target>/hal.h`** files contain GPIO definitions that should
  migrate to the per-radio `hw_def.json` files.
- **UI and system control code are not separated.** The `menusTask` (defined
  in `tasks.cpp`, main loop in `main.cpp:perMain()`) is misnamed -- it is
  really the "everything else" task. Beyond running the UI via `guiMain()`,
  it handles: power management and shutdown, USB mode switching, storage
  flushing, trainer port reconfiguration, backlight control, battery
  monitoring, audio volume/headphone detection, Lua script execution, and
  logging. These responsibilities have fundamentally different concerns and
  timing constraints from UI rendering. They should be separated so that
  system-critical control logic (power, storage, trainer) does not share a
  task with high-latency UI operations. How exactly this separation should
  look is still an open question.

### Migration roadmap

This refactoring cannot happen all at once. The steps below are ordered by
dependency -- later steps build on earlier ones. The old `targets/` structure
continues to work for not-yet-migrated radios, so each step can be merged
independently.

#### Step 1: Move CPU code into `cpu/` *(done)*

Moved `targets/common/arm/stm32/` to `cpu/stm32/` and updated all CMake
include paths. Untangling ARM-generic code into `cpu/arm/` can happen later
as a follow-up within `cpu/`.

#### Step 2: Extract board code from `targets/` into `boards/`

Migrate one board family at a time. For each target directory, separate:

- **Board-level code** (LCD driver, touch, backlight, haptic, SDRAM, external
  flash, audio codec init, etc.) → moves to `boards/<family>/`
- **Radio-level configuration** (feature flags, CPU selection, hw_def JSON)
  → stays behind, will move to `radios/` in step 3

Start with simpler single-radio targets (`pa01`, `st16`, `pl18`) to
establish the pattern before tackling multi-radio targets like `taranis`.

#### Step 3: Create `radios/` facade targets

This step does **not** depend on step 2 and can happen early -- even before
any board extraction. Create `radios/<radio>/CMakeLists.txt` files that are
thin wrappers over the existing build system:

```cmake
# radios/x9d-2019/CMakeLists.txt
set(PCB "X9D+" CACHE STRING "")
set(PCBREV "2019" CACHE STRING "")
# ... any other radio-specific flags
include(${CMAKE_SOURCE_DIR}/radio/src/targets/taranis/CMakeLists.txt)
```

This immediately gives the new build interface
(`cmake --build . --target x9d-2019`) without changing any existing build
logic. The facade files are mechanical and could be scripted for all ~50
radios in a day's work.

At this stage, also move each radio's JSON hardware definition from
`boards/hw_defs/<radio>.json` to `radios/<radio>/hw_def.json`.

#### Step 4: Migrate `radios/` from facades to direct composition

As board families are extracted in step 2, the per-radio `CMakeLists.txt`
files can be rewritten to directly compose `cpu/` + `boards/` modules
instead of forwarding to the old target CMakeLists:

```cmake
# radios/x9d-2019/CMakeLists.txt (after migration)
set(CPU "stm32/f4")
set(BOARD "taranis")
set(HW_DEF "${CMAKE_CURRENT_SOURCE_DIR}/hw_def.json")
# radio-specific feature flags
set(HAPTIC ON)
set(BLUETOOTH OFF)
# ...
```

This happens naturally, one board family at a time. Once all radios in a
former target directory have been migrated, the old `targets/<target>/`
directory and its `PCB`/`PCBREV`-driven CMakeLists can be removed.

#### Per-radio compilation during the transition

Until the generic code is fully free of `#ifdef RADIO_*` / `#ifdef PCB_*`,
each radio target must compile its own copy of the core files with its own
preprocessor defines. This is already how things work today with `-DPCB=` --
per-radio CMake targets do not change that.

As each `#ifdef` is removed, the compiled output across radios converges.
The end state is that generic code is compiled **once per MCU architecture**
(e.g. once for Cortex-M4, once for Cortex-M7), since different instruction
sets and compiler flags will always require separate compilation. But the
source code itself is identical -- only the toolchain target differs.

The multi-radio simulator binary is the litmus test: when the simulator can
compile the generic code exactly once for all radios, the `#ifdef` cleanup
is complete.

#### Ongoing (in parallel with steps 1-4)

These can happen at any time and do not block or depend on the steps above:

- **Migrate `hal.h` GPIO defines to `hw_def.json`** -- each define moved
  reduces the amount of radio-specific C code
- **Remove `#ifdef RADIO_*` from generic code** -- each instance requires
  adding or extending a HAL interface, so this is incremental by nature
- **Extend incomplete HAL interfaces** -- as new abstractions are needed
- **Untangle ARM-generic from STM32-specific within `cpu/`** -- factor out
  code that belongs in `cpu/arm/` vs `cpu/stm32/`

### How to contribute to the modernization

When writing new code or modifying existing code:

1. **Never add `#ifdef PCB_*` or `#ifdef RADIO_*` to generic code.** If you
   need hardware-specific behavior, add or extend a HAL interface.
2. **Prefer JSON hardware definitions** over C preprocessor defines for pin
   mappings and peripheral configuration.
3. **When touching older targets**, look for opportunities to extract
   board-level code into `boards/` and replace target-specific logic with HAL
   calls.
4. **Place CPU code at the right level.** If a driver only uses ARM or
   Cortex-M features (not STM32 peripheral registers), it belongs in
   `cpu/arm/`, not under `cpu/stm32/`.
5. **New drivers** should implement HAL interfaces using the VTable pattern
   for complex subsystems or plain C functions for simple ones.
6. **The simulator must remain functional.** Any new HAL interface needs a
   corresponding simulator implementation in `simu/`.
