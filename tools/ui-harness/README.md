# EdgeTX simulator UI harness

This directory contains the dependency-free host side of the native simulator
automation protocol. It supports strict declarative flows, copied fixtures,
target-filtered input and state injection, real-LCD frame barriers,
render-complete framebuffer capture, Lua reload, restart, `release-all`, and
clean process shutdown. `describe` advertises only commands and capabilities
that are usable in the current target build.

## One-command TX16S smoke

From a compiler environment that can already build the native EdgeTX simulator,
the following command configures, incrementally builds, runs the checked-in
scenario, verifies its artifacts, and returns nonzero on any failure:

```text
python tools/ui-harness/edgetx-ui smoke --build-dir build/ui-harness/tx16s
```

On Windows, run it from an x64 Visual Studio developer shell with `SDL2_DIR`
set to an SDL2 CMake package. On Linux, install the normal EdgeTX native build
dependencies or run it in the official `ghcr.io/edgetx/edgetx-dev` image. The
initial target is TX16S (`PCB=X10`, `PCBREV=TX16S`, 480x272 RGB565); other
targets are rejected before launch until they have an explicit schema profile
and fixture.

To use an already-built simulator:

```text
python tools/ui-harness/edgetx-ui smoke build/native/radio/src/targets/simu/simu
```

Use `run-flow` for another schema-v1 JSON scenario:

```text
python tools/ui-harness/edgetx-ui run-flow path/to/flow.json path/to/simu
```

## TX16S hardening gate

Phase 8 lifecycle and visual stress is available as one reproducible command.
Options precede the simulator path; arguments after `--` are passed through to
the simulator:

```text
python tools/ui-harness/edgetx-ui harden \
  --runs build/ui-harness/hardening \
  --report build/ui-harness/hardening/windows.json \
  build/native/radio/src/targets/simu/simu
```

The contractual defaults are 100 fresh process start/stop cycles, 10,000
correlated pings, 20 Lua reloads, 20 warm restarts, and 20 static captures.
Use `--lifecycle-cycles`, `--ping-count`, `--lua-reloads`,
`--warm-restarts`, and `--captures` to tune a local or gate run. To keep reports
and artifact sets bounded, lifecycle cycles, Lua reloads, warm restarts, and
captures are capped at 1,000 each; pings are capped at 1,000,000 (including the
100,000-exchange gate).

`--report` is published with exclusive-create semantics and fails if the target
already exists. Pass `--force` only when intentional replacement of that report
has been explicitly chosen; a publication failure still leaves diagnostic
evidence inside the unique run directory.

Every process receives a unique writable copy of the TX16S fixture. The stable
JSON report records process reaping, reader/writer-thread shutdown, Lua generations,
restart epochs and display sequences, stale-completion counters, canonical PPM
and decoded-RGB SHA-256 values, fixture integrity, streaming protocol-evidence
path/count/SHA-256 metadata, and leftover temporary
artifacts. Any incomplete count, changing static capture, unchanged deliberate
visual mutation, stale completion, modified fixture, temporary artifact, or
unreaped child makes the command exit nonzero while preserving its evidence.

Every flow is completely validated before process launch. Unknown fields,
duplicate JSON keys, unsupported actions or targets, out-of-range values, more
than 1000 steps, unsafe artifact names, and missing capabilities are failures.
Command and startup timeouts are positive and capped at 60 seconds.

## Fixtures and output

`tools/ui-harness/fixtures/tx16s` is an immutable template derived from the
fixture contributed by Mateusz Urban (`onliner10`) in EdgeTX PR #7337 and
migrated to the current settings schema. Each execution creates a unique tree:

```text
build/ui-harness/runs/tx16s-smoke-<unique>/
  settings/                    # writable fixture copy
  sdcard/                      # writable fixture copy
  artifacts/checkpoints/
    home.ppm
    home.png
    home.capture.json
  manifest.json
  protocol.jsonl
  stderr.log
```

The manifest records the EdgeTX commit, host platform, Python version, target,
LCD, fixture and flow hashes, all steps and protocol exchanges, termination
state, artifact SHA-256 values, and the exact failed step. Simulator stderr is
kept separately and bounded by the session client. PPM, PNG, metadata, protocol,
and manifest output is UTF-8 or canonical binary data and is never written into
the fixture template.

The unique run root is trusted, session-owned state. Do not modify or replace
its directories, symlinks, or Windows reparse points while a run is active.
Containment rejects unsafe protocol paths but is not an OS sandbox against a
concurrent process running as the same user.

Failed runs are intentionally preserved for diagnosis. Successful and failed
run directories can be removed after their evidence is no longer needed; the
harness never reuses or overwrites them. It always attempts protocol `stop`,
then terminate and kill-and-wait if necessary, and releases owned inputs on
composite-action failures.

## Troubleshooting

- `unsupported target`, LCD, command, or capability means the simulator build
  does not match the flow; rebuild the TX16S native simulator.
- `output root is not ready` means the artifacts directory was not accepted.
- A request timeout poisons that session by design. Inspect `manifest.json`,
  `protocol.jsonl`, and `stderr.log`; increase a schema timeout only when the
  operation legitimately needs it.
- Windows uses binary subprocess pipes and separate reader threads; no POSIX
  `select` behavior is assumed. Ensure `SDL2.dll` is on `PATH` beside the
  simulator or in the developer shell.
- Checked-in fixture YAML is forced to CRLF because the current TX16S settings
  checksum was produced by the simulator's Windows writer; Git preserves that
  representation on Linux and Windows.

## Session API

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
