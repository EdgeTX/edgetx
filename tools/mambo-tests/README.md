# Standalone Mambo board tests

Run without the radio, RF firmware, an RTOS or private calibration files:

```sh
cmake -S tools/mambo-tests -B /tmp/mambo-board-tests
cmake --build /tmp/mambo-board-tests
ctest --test-dir /tmp/mambo-board-tests --output-on-failure
```

Requires a C++17 compiler, CMake 3.20+ and Python 3. The eight suites exercise
production code using only synthetic data and register/GPIO fakes:

- Encoder: APB2 timer rate, EXTI release, interrupt-mask restoration,
  direction, bounce, stable samples, ENTER suppression and off-detent startup.
- Hall sensors: initialization/readback, conversion rearming, data-ready and
  transport failures, freshness, timestamp wrapping and projection bounds.
- Calibration: both file checksums, signed parameters, record corruption,
  invalid geometry, and comparisons with independent double-precision math.
- Trim ladder: nominal resistor ratios, idle, shorted and boundary inputs.
- External bay: all 256 persisted protocol values, model-controlled Off/CRSF,
  power-off and emergency-shutdown inhibition.
- Power button: active-high Mambo and unchanged active-low behavior.
- Battery: full filtered-ADC range, calibration and absent-input behavior.

These are host tests, not physical timing measurements. The calibration
fixtures are artificial and must never be installed on a radio.

`verify_firmware.py` checks an application ELF and its matching binary against
the flash/SRAM/CCM reservations and verifies the real external CRSF UART/DMA
interrupt vectors and board/bay symbols in the default Mambo build. It requires
`pyelftools` (included in the repository's Python dependencies). It never opens
USB or writes to a radio. `.github/workflows/mambo.yml` runs the host tests and
builds/checks default Mambo without adding a nightly/release target.

`mambo-support` retains the standalone board and external CRSF support, direct
Hall input/calibration, Lua rotary navigation events and encoder polling fix.
Internal stock-RF integration, its startup/SD/IRQ handling, runtime harness and
diagnostics live on the separate `mambo-tracer` branch. Branch separation does
not change the firmware currently installed on the radio.
