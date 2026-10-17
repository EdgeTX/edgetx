# Standalone Mambo board tests

Run without the radio, RF firmware, an RTOS or private calibration files:

```sh
cmake -S tools/mambo-tests -B /tmp/mambo-board-tests
cmake --build /tmp/mambo-board-tests
ctest --test-dir /tmp/mambo-board-tests --output-on-failure
```

The production encoder decoder and board polling helper are exercised against
register fakes: APB2 timer rate, EXTI release, interrupt-mask restoration,
direction, bounce, stable samples, ENTER suppression and off-detent startup.
These are host tests, not physical timing measurements.

`mambo-support` retains the standalone board and external CRSF support, direct
Hall input/calibration, Lua rotary navigation events and encoder polling fix.
Internal stock-RF integration, its startup/SD/IRQ handling, runtime harness and
diagnostics live on the separate `mambo-tracer` branch. Branch separation does
not change the firmware currently installed on the radio.
