# EdgeTX simulator UI harness

This directory contains the dependency-free host side of the native simulator
automation protocol. The current foundation supports `ping`, `status`,
`describe`, target-filtered key/rotary/touch input, real-LCD frame barriers,
render-complete framebuffer capture, `release-all`, and `stop`. Declarative
flows are added in a later phase of the consolidation plan; `describe`
advertises only commands and capabilities that are usable in the current
target build.

Run a lifecycle probe with Python 3:

```text
python tools/ui-harness/edgetx-ui probe \
  --output build/ui-harness/manual-run \
  build/native/radio/src/targets/simu/simu \
  -- --storage <sdcard-copy> --settings <settings-copy>
```

The client launches with binary pipes, starts independent stdout and stderr
readers, sends `ping`, validates the bounded `describe` result, and polls
`status` until the simulator reports a real first LCD frame. Target identity,
LCD dimensions, command availability, and optional required capabilities are
checked before startup succeeds. Every response is correlated by request ID,
and every shutdown path waits for the child after graceful stop, terminate, or
kill. There is no pipe `select`, shell interpolation, or fixed startup sleep, so
the same lifecycle works on POSIX and Windows.

Primitive calls are immediate; durations are host-side composites:

```python
with SimulatorSession(simulator, output_root) as session:
    session.press("ENTER", duration=0.05)
    session.tap(20, 20, duration=0.05)
    session.drag(((20, 20), (100, 80), (200, 120)), duration=0.2)
    frame = session.wait_next_frame()
```

Key names, LCD bounds, command availability, and rotary/touch capabilities are
validated locally from `describe` before a primitive consumes a request ID.
Every composite attempts its matching release, with `release-all` as the
failure fallback.

RGB565 targets advertise the `capture` capability. The native command waits
for a strictly newer firmware framebuffer and publishes a canonical PPM
without replacing an existing file. The client can validate that PPM directly
or convert it to a dependency-free, independently decoded PNG with SHA-256
metadata:

```python
checkpoint_dir = output_root / "checkpoints"
checkpoint_dir.mkdir(parents=True, exist_ok=True)

with SimulatorSession(simulator, output_root) as session:
    native = session.capture_ppm("checkpoints/home screen.ppm")
    bundle = session.capture_png("checkpoints/model menu.png")
    print(native.display_sequence, bundle.png.sha256)
```

Artifact paths are canonical forward-slash paths below the configured output
root. Their parent directories must already exist, and `.ppm`, `.png`, and
`.capture.json` outputs are never silently overwritten.

`wait_next_frame()` waits for a future firmware LCD notification; it does not
manufacture one on a static screen. `capture_ppm()` does request one safe LVGL
invalidation so static checkpoints can be captured. To synchronize an input,
save `session.read_status().display_sequence` before the input and then call
`session.wait_frame(saved + 1)`.

Run the focused tests from the repository root:

```text
python -m unittest discover -s tools/ui-harness/tests -v
```
