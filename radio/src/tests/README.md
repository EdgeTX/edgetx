# EdgeTX Radio Unit Tests

Unit tests for the EdgeTX radio firmware, using [Google Test](https://github.com/google/googletest) (GTest v1.14.0).

## Overview

Tests run against the firmware compiled in **native (simulator) mode**, so no physical hardware is required.
All hardware calls (ADC, switches, audio, LCD) are stubbed out by the simulator library (`simulib.h`).

**AddressSanitizer (ASAN)** is enabled by default for all test builds, catching memory errors
(buffer overflows, use-after-free, memory leaks) automatically.

## Coverage areas

| Test file | What it covers |
|-----------|---------------|
| `mixer.cpp` | Mixer engine: trims, curves, weights, throttle, flight mode transitions |
| `switches.cpp` | Logical switches, edge detection, flex switch inputs |
| `frsky.cpp` | FrSky telemetry protocol: Hub, SPORT, GPS, cell voltage, altitude |
| `timers.cpp` | Timer modes (ON, THR, THR_REL, THR_START, OFF), countdown, reset |
| `crossfire.cpp` | Crossfire frame construction, CRC-8, telemetry parsing |
| `lua.cpp` | Lua script execution, panic protection, state persistence |
| `functions.cpp` | Special functions: global variables, flight reset |
| `module_ports.cpp` | Serial port and module configuration |
| `lcd.cpp` | LCD drawing primitives (128×64 and 212×64 targets) |
| `lcd_480x272.cpp` | Color LCD rendering (480×272) |
| `colorlcd.cpp` | Color LCD widget rendering |
| `model_audio.cpp` | Model audio trigger filenames |
| `yaml.cpp` | YAML model serialization round-trip |
| `eeprom.cpp` | EEPROM read/write |
| `model.cpp` | Model default initialization |
| `sources.cpp` | Source value mapping |
| `test_*.cpp` | Translation string compilation (18 languages) |

## Building

Tests are **not** built by the default CMake target. Use the explicit targets:

```bash
# From the edgetx repo root, first configure a native build:
cmake -B build/native <options>

# Then build and run tests for that target:
cmake --build build/native --target tests-radio

# Or build the executable without running:
cmake --build build/native --target gtests-radio
```

In practice the easiest way is to use the provided script, which configures and runs tests for
a specific radio target:

```bash
# Example: run tests for TX16S
FLAVOR=tx16s ./tools/commit-tests.sh
```

## Running

```bash
# Run all tests (terse output — one '+' per passing test):
./gtests-radio

# Run all tests with standard GTest verbose output:
./gtests-radio --verbose

# Run a specific test suite:
./gtests-radio --gtest_filter=Timers.*

# Run a specific test:
./gtests-radio --gtest_filter=Timers.timerAbsolute

# Run multiple suites (wildcard, separated by colon):
./gtests-radio --gtest_filter=Timers.*:FrSky.*

# Exclude a suite:
./gtests-radio --gtest_filter=-FlexSwitches.*

# List all available tests without running them:
./gtests-radio --gtest_list_tests
```

## Adding new tests

### Simple test (no shared setup)

```cpp
#include "gtests.h"

TEST(MyFeature, myScenario)
{
  MODEL_RESET();
  MIXER_RESET();
  // ... set up state ...
  EXPECT_EQ(expected, actual);
}
```

### Test using the shared fixture

`EdgeTxTest` calls `SYSTEM_RESET()`, `MODEL_RESET()`, `MIXER_RESET()`, `setModelDefaults()`, and
`RADIO_RESET()` before each test. Inherit from it when your test needs clean radio + model state:

```cpp
#include "gtests.h"

class MyFeatureTest : public EdgeTxTest {};

TEST_F(MyFeatureTest, myScenario)
{
  // State is already clean — just configure what you need:
  g_model.timers[0].mode = TMRMODE_ON;
  // ...
}
```

### Parameterized test

Use `TEST_P` when the same behaviour should be verified across multiple inputs:

```cpp
#include "gtests.h"

struct MyParams {
  int input;
  int expected;
};

class MyParamTest : public EdgeTxTest,
                    public ::testing::WithParamInterface<MyParams> {};

TEST_P(MyParamTest, checkOutput)
{
  auto p = GetParam();
  anaSetFiltered(0, p.input);
  evalMixes(1);
  EXPECT_EQ(p.expected, channelOutputs[0]);
}

INSTANTIATE_TEST_SUITE_P(Inputs, MyParamTest, ::testing::Values(
  MyParams{-1024, -CHANNEL_MAX},
  MyParams{    0,            0},
  MyParams{+1024, +CHANNEL_MAX}
));
```

### New test file

1. Create `radio/src/tests/myfeature.cpp` — it will be picked up automatically by the glob in
   `CMakeLists.txt`.
2. Include `"gtests.h"` at the top.
3. Use `#if defined(SOME_DEFINE)` guards when the feature is not present on all targets.
4. Add your test file to this table in the README.

## Test helpers reference

### Reset macros

| Macro | What it resets |
|-------|----------------|
| `SYSTEM_RESET()` | General settings, all switches to up position |
| `MODEL_RESET()` | Model struct, ADC inputs, mixer first-run flag |
| `MIXER_RESET()` | Channel outputs, trims, flight mode state, logical switches |
| `TELEMETRY_RESET()` | Telemetry sensors and streaming data |
| `RADIO_RESET()` | Physical switch types (varies by radio hardware) |

### Simulator inputs

```cpp
anaSetFiltered(chan, val);       // Set analog input (stick/pot), range -1024..+1024
anaResetFiltered();              // Reset all analog inputs to zero
simuSetSwitch(idx, pos);        // Set switch position: -1=up, 0=mid, 1=down
setTrimValue(flightMode, ch, val); // Set trim value
```

### String assertion helpers

```cpp
EXPECT_ZSTREQ("hello", zstring);  // Compare C string with zero-padded fixed-size string
EXPECT_STRNEQ("hello", nstring);  // Compare C string with null-padded fixed-size string
```

## Translation completeness

The `test_*.cpp` files verify that each language file compiles (i.e., defines every string
referenced in `string_list.h`). For richer analysis — identifying strings identical to English
(likely untranslated), orphaned strings, and coverage metrics — use the Python script:

```bash
# Summary table for all languages:
python3 tools/check-translations.py --summary-only

# Detailed report for one language:
python3 tools/check-translations.py --lang fr

# Show strings that are identical to English (likely untranslated):
python3 tools/check-translations.py --lang de --show-identical

# Exit non-zero if any identical strings exist (useful for CI gates):
python3 tools/check-translations.py --lang fr --strict
```

The script exits with code 1 if any language is missing a string that exists in English
(which would also cause a C++ compile failure).

## Coverage reporting

Coverage instrumentation uses gcov/lcov and is **mutually exclusive with ASAN** (enable one or the other).
Requires `lcov` and `genhtml` (`apt install lcov`).

```bash
# Configure with coverage enabled (ASAN is automatically disabled):
cmake -B build/coverage <options> -DENABLE_COVERAGE=ON

# Build and generate an HTML report in build/coverage/coverage_html/:
cmake --build build/coverage --target coverage-radio
```

Open `build/coverage/coverage_html/index.html` in a browser to browse line-by-line coverage.

## CI

Tests run in CI for every pull request and push to `main`, covering 19+ radio target
configurations. Firmware builds are gated on all tests passing. See
`.github/workflows/build_fw.yml` and `tools/commit-tests.sh`.
