# Compilation Options

This page documents the CMake options available when building EdgeTX firmware. Most options have sensible defaults set by the target's CMakeLists.txt — you only need to override them when you have a specific reason to.

## Target Selection

These are the most important options to set, as they determine which radio you are building for.

### `PCB`

Selects the radio target.

| Value | Radio |
|-------|-------|
| `X7` | FrSky Taranis Q X7 / X7S and Jumper / RadioMaster variants (see `PCBREV`) |
| `X9LITE` | FrSky Taranis X9 Lite |
| `X9LITES` | FrSky Taranis X9 Lite S |
| `XLITE` | FrSky X-Lite |
| `XLITES` | FrSky X-Lite S |
| `X9D` | FrSky Taranis X9D |
| `X9D+` | FrSky Taranis X9D+ (see `PCBREV`) |
| `X9E` | FrSky Taranis X9E |
| `X10` | FrSky Horus X10 / X10S and RadioMaster / Jumper variants (see `PCBREV`) |
| `X12S` | FrSky Horus X12S |
| `PL18` | Flysky PL18 and variants (see `PCBREV`) |
| `TX15` | RadioMaster TX15 |
| `TX16SMK3` | RadioMaster TX16S Mark III |
| `T15PRO` | Jumper T15 Pro |
| `ST16` | Siyi ST16 |
| `PA01` | FrSky PA01 |

### `PCBREV`

Selects a specific variant within a `PCB` target. Valid values depend on the selected `PCB`:

=== "X7"
    | Value | Radio |
    |-------|-------|
    | `TX12` | RadioMaster TX12 |
    | `TX12MK2` | RadioMaster TX12 Mark II |
    | `ZORRO` | RadioMaster Zorro |
    | `BOXER` | RadioMaster Boxer |
    | `MT12` | RadioMaster MT12 |
    | `GX12` | RadioMaster GX12 |
    | `POCKET` | RadioMaster Pocket |
    | `T8` | RadioMaster T8 |
    | `COMMANDO8` | RadioMaster Commando 8 |
    | `T12` | Jumper T12 |
    | `T12MAX` | Jumper T12 Max |
    | `TPRO` | Jumper T-Pro |
    | `TPROV2` | Jumper T-Pro V2 |
    | `TPROS` | Jumper T-Pro S |
    | `T14` | Jumper T14 |
    | `T20` | Jumper T20 |
    | `T20V2` | Jumper T20 V2 |
    | `TLITE` | Jumper T-Lite |
    | `LR3PRO` | Jumper LR3 Pro |
    | `BUMBLEBEE` | Jumper Bumblebee |
    | `ACCESS` | FrSky Q X7 Access |
    | `V12` | FrSky Q X7 V1.2 |
    | `V14` | FrSky Q X7 V1.4 |

=== "X9D+"
    | Value | Radio |
    |-------|-------|
    | `2014` | X9D+ (original, default) |
    | `2019` | X9D+ 2019 |

=== "X10"
    | Value | Radio |
    |-------|-------|
    | `STD` | FrSky Horus X10 (default) |
    | `EXPRESS` | FrSky Horus X10 Express |
    | `T16` | RadioMaster TX16S |
    | `TX16S` | RadioMaster TX16S Mark II |
    | `F16` | Flyzone F16 |
    | `V16` | VolantexRC V16 |
    | `T18` | Jumper T18 |
    | `T15` | Jumper T15 |

=== "PL18"
    | Value | Radio |
    |-------|-------|
    | `PL18` | Flysky PL18 (default) |
    | `PL18EV` | Flysky PL18EV / Paladin EV |
    | `PL18U` | Flysky PL18U |
    | `EL18` | Flysky EL18 |
    | `NV14` | Flysky NV14 |
    | `NB4P` | Flysky NB4+ |

### `TRANSLATIONS`

Firmware language. Default: `EN`

Valid values: `CN` `CZ` `DA` `DE` `EN` `ES` `FI` `FR` `HE` `HU` `IT` `JP` `KO` `NL` `PL` `PT` `RU` `SE` `SK` `TW` `UA`

---

## Feature Options

These options enable or disable firmware features. Defaults are typically set by the target, so you only need to override them if you want to add or remove a feature for a specific build.

| Option | Default | Description |
|--------|---------|-------------|
| `AUTOSOURCE` | ON | Automatic source detection in menus |
| `AUTOSWITCH` | ON | Automatic switch detection in menus |
| `BLUETOOTH` | varies | Bluetooth module support |
| `BOOTLOADER` | ON | Include bootloader |
| `CURVES` | ON | Curves support |
| `DANGEROUS_MODULE_FUNCTIONS` | OFF | Enable RangeCheck / Bind / Module OFF functions |
| `DBLKEYS` | ON | Double key shortcuts |
| `DISK_CACHE` | ON | SD card disk cache (color screen targets) |
| `FAI` | OFF | Competition mode — disables telemetry |
| `FLIGHT_MODES` | ON | Flight modes |
| `FLYSKY_GIMBAL` | varies | Flysky serial gimbal support |
| `GVARS` | ON | Global variables |
| `GUI` | ON | GUI |
| `HELI` | ON | Helicopter mixer menu |
| `INTERNAL_GPS` | varies | Internal GPS support |
| `KCX_BTAUDIO` | OFF | KCX BT audio emitter support |
| `LUA` | ON | Lua scripting |
| `LUA_COMPILER` | ON | Pre-compile and cache Lua scripts on the radio |
| `LUA_MIXER` | ON | Lua mixer / model scripts |
| `MODULE_PROTOCOL_D8` | ON | FrSky D8 module protocol |
| `MODULE_PROTOCOL_FCC` | ON | FCC module protocol |
| `MODULE_PROTOCOL_FLEX` | OFF | Non-certified FLEX module protocol |
| `MODULE_PROTOCOL_LBT` | ON | EU / LBT module protocol |
| `MODULE_SIZE_STD` | ON | Standard size external module bay |
| `MULTIMODULE` | ON | DIY Multiprotocol TX Module |
| `OVERRIDE_CHANNEL_FUNCTION` | ON | OverrideChannel function |
| `PPM_CENTER_ADJUSTABLE` | ON | PPM centre adjustable |
| `PPM_LIMITS_SYMETRICAL` | OFF | PPM limits symmetrical |
| `RAS` | ON | RAS (SWR) measurement |
| `SPACEMOUSE` | OFF | SpaceMouse support (Horus only) |
| `TEMPLATES` | OFF | Model templates menu |
| `UNEXPECTED_SHUTDOWN` | ON | Unexpected shutdown warning screen |
| `USB_SERIAL` | ON | USB serial (CDC) |
| `WATCHDOG` | ON | Hardware watchdog |

---

## Hardware Configuration

These string options configure hardware behaviour and are normally set by the target.

### `PWR_BUTTON`

How the power button works. Default is target-specific.

| Value | Description |
|-------|-------------|
| `PRESS` | Momentary press to power on/off (most radios) |
| `SWITCH` | Toggle switch |

### `PPM_UNIT`

PPM display unit. Default is target-specific.

| Value | Description |
|-------|-------------|
| `PERCENT_PREC1` | Percentage with one decimal place (e.g. 100.0%) |
| `PERCENT_PREC0` | Percentage with no decimal places (e.g. 100%) |
| `US` | Microseconds |

### `DEFAULT_MODE`

Default sticks mode. Leave blank to use the firmware default.

| Value | Description |
|-------|-------------|
| `` | Use firmware default |
| `1` | Mode 1 |
| `2` | Mode 2 |
| `3` | Mode 3 |
| `4` | Mode 4 |

### `INTERNAL_GPS_BAUDRATE`

Baud rate for the internal GPS, where supported. Default: `9600`

### `LUA_SCRIPT_LOAD_MODE`

Controls how Lua scripts are loaded and compiled. Leave blank to use the firmware default (`bt` on radio, `T` on simulator/debug builds). Accepts a combination of the following flags:

| Flag | Description |
|------|-------------|
| `b` | Load binary (pre-compiled) scripts |
| `t` | Load text scripts |
| `T` | Pre-compile scripts |
| `x` | Execute on load |
| `c` | Compile only |
| `d` | Decompress |

### `POPUP_LEVEL`

Controls the verbosity of popup notifications. Default: `2` (set to `3` for `FRSKY_RELEASE` builds).

---

## Debug & Tracing

These options are for development and debugging. All default to `OFF`.

| Option | Description |
|--------|-------------|
| `ASTERISK` | Show asterisk icon to identify test-only firmware |
| `DEBUG_SEGGER_RTT` | Route debug output to Segger RTT |
| `DEBUG_WINDOWS` | Trace UI window activity |
| `DEBUG_YAML` | Trace YAML parser activity |
| `DEBUG_LABELS` | Trace label activity |
| `JITTER_MEASURE` | Enable ADC jitter measurement |
| `SEMIHOSTING` | Enable debugger semihosting |
| `TEST_BUILD_WARNING` | Show a warning banner on test builds |
| `TRACE_AUDIO` | Trace audio subsystem |
| `TRACE_LUA_INTERNALS` | Trace Lua VM internals |
| `UI_PERF_MONITOR` | Overlay frame rate and CPU usage on screen |
| `WARNINGS_AS_ERRORS` | Treat compiler warnings as errors (`-Werror`) |

---

## Simulator Options

| Option | Default | Description |
|--------|---------|-------------|
| `SIMU_DISKIO` | OFF | Enable disk I/O simulation using a `sdcard.image` file |
| `SIMU_LUA_COMPILER` | ON | Pre-compile and cache Lua scripts in the simulator |
| `SIMU_TARGET` | ON | Build the simulator target |
| `DISABLE_COMPANION` | OFF | Skip building Companion and simulators |

---

## Release Build Options

These options are used to produce official manufacturer-specific firmware releases and are not needed for general development builds.

| Option | Manufacturer |
|--------|-------------|
| `FRSKY_RELEASE` | FrSky (also sets `POPUP_LEVEL=3`) |
| `IMRC_RELEASE` | IMRC |
| `TBS_RELEASE` | Team BlackSheep |
| `JUMPER_RELEASE` | Jumper |
| `RADIOMASTER_RELEASE` | RadioMaster |
| `RADIOMASTER_RTF_RELEASE` | RadioMaster RTF |
| `IFLIGHT_RELEASE` | iFlight |
| `BETAFPV_RELEASE` | BetaFPV |

---

## Compiler & Build Flags

| Option | Default | Description |
|--------|---------|-------------|
| `OPT` | `s` | Optimisation level (`0`, `1`, `2`, `s`) |
| `FIRMWARE_C_FLAGS` | `` | Additional C compiler flags |
| `FIRMWARE_CXX_FLAGS` | `` | Additional C++ compiler flags |
| `WARNINGS_AS_ERRORS` | OFF | Treat all compiler warnings as errors |
| `VERBOSE_CMAKELISTS` | OFF | Show extra information when processing CMake files |
