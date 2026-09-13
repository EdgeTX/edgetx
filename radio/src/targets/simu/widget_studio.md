# Widget Studio simulator hooks

Widget Studio adds opt-in automation hooks to the native EdgeTX simulator for
developing and testing visual Lua/LVGL components. It uses the real simulator
framebuffer, firmware UI, Lua runtime, themes and fonts; it is not a separate
renderer.

The feature is disabled by default and is compiled only when
`WIDGET_STUDIO=ON`. With the option disabled, the `simu` Lua module,
`--pipe` argument, capture backend, input overrides and reset state are absent.
Nothing in this feature is compiled into radio firmware.

## Build

From the repository root:

```sh
cmake --preset simu -DWIDGET_STUDIO=ON
cmake --build --preset simu
```

The hooks currently target the native color-LCD simulator. They are not an API
for hardware firmware, Companion embedding or the WASM simulator.

## Host command file

Start `simu` with `--pipe <path>`. Despite the historical option name, the path
must identify a regular append-only file, not an operating-system FIFO. The
simulator tolerates a file that does not exist yet and polls only the bytes that
were appended since the previous frame.

Create or truncate the file before starting a new simulator process, then
append newline-delimited commands while it runs. Blank lines and lines whose
first character is `#` are ignored. Both LF and CRLF line endings are accepted.

| Command | Effect |
|---------|--------|
| `exit` | Push an SDL quit event |
| `capture <host path>` | Write the next completed LCD frame as a PNG; the path may contain spaces |
| `key <code> <0\|1>` | Set an `EnumKeys` key up or down |
| `touch <x> <y>` | Press or drag the touch point within the LCD bounds |
| `touchup` | Release the touch point |
| `reset` | Request a full simulator stop/start between frames |
| `reload` | Reload permanent Lua scripts |

The host owns command-file lifecycle. It should append complete lines and must
not rewrite already-consumed bytes while the simulator is running. If the file
shrinks, the reader safely resynchronizes from byte zero.

## Simulator-only Lua module

A color simulator built with the option exposes a global `simu` table:

```lua
simu.setSwitch(name, state)       -- state is clamped to -1, 0 or 1
simu.setAnalog(name, value)       -- ADC value is clamped to 0..4096
simu.armCapture("/SCREENSHOTS/frame.png")
simu.reset()                      -- asynchronous; consumed between frames
```

`setSwitch` and `setAnalog` return `false` when the named input is unknown.
`armCapture` accepts a radio-style path and resolves it through the simulator's
configured SD-card mapping. Telemetry injection intentionally remains on the
existing public `setTelemetryValue` path so tests exercise the normal sensor
registry.

## Deterministic capture

Capture is one-shot. Arming it invalidates the active LVGL screen on the next
UI cycle, which guarantees a flush even when the screen was static. The native
frame-ready callback converts the RGB565 framebuffer to RGB888, writes the PNG
and disarms.

The PNG encoder is linked only into the interactive `simu` executable. Keeping
it out of the shared simulator object library avoids duplicate stb symbols in
radio tests and WASM builds.

## Trust boundary

These are local development hooks. The command file and any host capture paths
must be treated as trusted input; do not expose them to untrusted users or a
network service without an additional validation layer.
