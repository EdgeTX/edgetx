# Simulator UI Automation

EdgeTX provides an opt-in automation interface for the native simulator. It
turns a manual simulator reproduction into a repeatable flow that can drive the
real EdgeTX runtime, synchronize with firmware LCD updates, and retain
inspectable evidence after the process exits.

Automation is disabled during normal simulator use. The implementation is also
excluded from physical firmware, WASI, and Emscripten builds.

## Intended use

The interface is designed for:

- repeatable reproduction of simulator UI behavior;
- scripted input and simulator-state transitions;
- framebuffer capture independent of host-window size and scaling;
- lifecycle, restart, and cleanup validation; and
- future UI regression scenarios and visual comparison workflows.

It is not a second UI renderer. Actions run through the existing simulator and
firmware ownership paths, and captures come from the firmware framebuffer.

Protocol version 1 supports one local controller and one native simulator
process. The first complete declarative-flow profile is TX16S (`PCB=X10`,
`PCBREV=TX16S`, 480x272 RGB565). Other native targets can advertise their real
capabilities, but each target needs an explicit host profile, fixture, and smoke
flow before the declarative runner accepts it.

## Architecture

```text
JSON flow / Python session
          |
          | bounded requests and JSON-line responses
          v
native simulator stdio transport
          |
          v
SDL automation executor -------- immediate simulator inputs
          |                       keys, rotary, touch, switches
          |                       and atomic analog overrides
          |
          +---------------------> bounded firmware mailbox
          |                       telemetry and Lua reload
          |
          +---------------------> LCD sequence and snapshot
                                           |
                                           v
                                   capture worker -> PPM
                                           |
                                           v
                                 host PNG, hashes and manifest
```

The Python host owns process lifecycle, request deadlines, timed composite
actions, fixture copies, declarative flows, and artifact verification. The SDL
thread owns immediate simulator input transitions and lifecycle coordination.
Firmware-owned operations cross a bounded mailbox and complete only after the
firmware context reports their result.

LCD notifications publish a display sequence and, when requested, a complete
framebuffer snapshot. The LCD callback performs no filesystem work. A dedicated
worker publishes native PPM artifacts, while PNG encoding and independent image
verification remain in the host harness.

## Activating automation

Launch a native simulator with both automation options:

```text
simu --automation-stdio \
  --automation-output <existing-artifact-directory> \
  --storage <writable-sdcard-copy> \
  --settings <writable-settings-copy>
```

`--automation-stdio` and `--automation-output` must be supplied together. The
output directory must already exist and pass the simulator's validation before
the protocol starts.

Stdout is reserved for protocol responses and transport events. Simulator,
SDL, and diagnostic output is routed to stderr. The host uses binary pipes on
both Windows and POSIX and does not depend on shell interpolation, POSIX
`select`, or fixed startup sleeps.

Closing stdin, losing stdout, or stopping a session releases owned key, touch,
and analog state, cancels pending work, and initiates bounded process shutdown.

## Protocol version 1

Requests are UTF-8 newline-delimited records:

```text
v1 <monotonic-request-id> <command> [arguments...]
```

Request IDs are unsigned 64-bit decimal values and must increase strictly for
the lifetime of the process. Each admitted request receives exactly one
terminal JSON response with the same ID. Uncorrelated transport events that can
still be emitted use a null ID.

The protocol accepts LF and CRLF records. Wire records, responses, queues,
arguments, paths, and asynchronous operations have explicit bounds. `capture`
is the only command whose single argument may contain internal spaces; other
commands use fixed argument boundaries.

### Commands

| Command | Arguments | Completion |
|---|---|---|
| `ping` | none | immediate response |
| `status` | none | consistent session snapshot returned |
| `describe` | none | target commands and capabilities returned |
| `key-down`, `key-up` | canonical key name | input state updated |
| `rotate` | nonzero steps in `-128..128` | rotary input delivered |
| `touch-down`, `touch-move` | in-bounds `x y` | touch state updated |
| `touch-up` | none | active touch released |
| `set-switch` | canonical name and `-1`, `0`, or `1` | target validates and updates the position |
| `set-analog` | canonical name and value in `0..4096` | atomic override published |
| `clear-analog` | canonical name or `all` | requested override cleared |
| `set-telemetry` | `id subId instance value unit precision [name]` | firmware operation completed |
| `reload-lua` | none | requested Lua generation reaches a terminal state |
| `wait-frame` | minimum `display_seq` | requested sequence observed |
| `capture` | safe relative `.ppm` path | fresh artifact published |
| `restart` | none | new epoch and its first LCD frame observed |
| `release-all` | none | keys, touch, and analog overrides released |
| `stop` | none | terminal response flushed and shutdown initiated |

Generic success, error, and transport-event records have these shapes:

```json
{"version":1,"type":"response","id":1,"ok":true,"epoch":1}
{"version":1,"type":"response","id":2,"ok":false,"epoch":1,"error":{"code":"invalid_argument","message":"..."}}
{"version":1,"type":"event","id":null,"epoch":1,"event":{"code":"invalid_record","message":"..."}}
```

`code` is the stable machine-consumable error identifier. `message` is bounded
diagnostic context and should not be parsed as an interface.

`describe` is the authority for the current target. It reports the target and
LCD identity, available commands and capabilities, canonical key and control
names, and advertised numeric bounds. Clients should validate against that
response instead of assuming a target feature.

`status` reports the current session phase, restart epoch, display sequence,
pending asynchronous work, queue counters, active input state, analog
overrides, and Lua state.

Only one asynchronous operation can be active at a time. This includes frame
waits, capture, firmware work, Lua reload, and warm restart. When `stop` is
processed, already admitted requests that have not executed receive
`session_stopping`; no command side effect is allowed after the stop barrier.

## Input and state ownership

Key, rotary, touch, and switch commands reuse the native simulator input paths.
Analog values use atomic overrides read by the existing simulator ADC path.
Timed presses, taps, and drags are composed by the host from explicit down,
move, and up transitions, so native command execution never sleeps to model a
duration.

Telemetry creation and updates run in firmware context and use the complete
sensor identity tuple. Lua reload also runs in firmware context and completes
only after its generation change is observed.

`release-all` releases keys and touch and clears analog overrides. It does not
reset switches. Warm restart releases keys and touch, clears analog overrides,
resets switches, increments the epoch, restarts firmware tasks in the same
process, and completes after the first frame in the new epoch. Model, fixture,
and telemetry state should not be assumed to reset. A cold restart is a
host-side composition that reaps the old process and starts a new process with
fresh fixture copies.

## Frame synchronization and capture

`display_seq` advances only when firmware reports an LCD refresh.
`wait-frame N` completes when the sequence reaches `N`; it does not manufacture
a frame on a static screen.

Capture is available only when the target advertises an RGB565 framebuffer. A
capture records the current sequence, requests one safe LVGL invalidation, and
accepts only a strictly newer complete framebuffer. The snapshot is copied to
worker-owned storage before the LCD callback returns.

The capture worker converts RGB565 to canonical P6 PPM and publishes the file
without replacing an existing destination. The host can validate that PPM,
convert it to PNG using only the Python standard library, decode it
independently, and write a `.capture.json` sidecar containing dimensions,
display sequence, and hashes.

Artifact paths must:

- be valid UTF-8 relative paths;
- remain below the configured output root;
- contain no root, `.` or `..` components;
- use a lowercase `.ppm` extension for native capture;
- refer to an already existing parent directory; and
- not target an existing artifact.

This containment model is for cooperative local development and CI. The output
root is trusted, session-owned state; it is not an operating-system sandbox
against another process running as the same user and mutating the directory
while a run is active.

## Host harness

The standard-library-only Python harness lives under `tools/ui-harness`. From
the repository root, a TX16S smoke can configure, build, run, and verify the
complete flow:

```text
python tools/ui-harness/edgetx-ui smoke \
  --build-dir build/ui-harness/tx16s
```

Run another schema-v1 flow against an existing simulator:

```text
python tools/ui-harness/edgetx-ui run-flow \
  path/to/flow.json path/to/simu
```

Run lifecycle, transport, Lua, restart, capture, fixture, and cleanup stress:

```text
python tools/ui-harness/edgetx-ui harden path/to/simu
```

Flow schemas, static ranges, declared requirements, paths, and step coherence
are validated before the simulator starts. After launch, `describe` verifies
the actual target identity and capability availability before any flow action
executes.

Declarative flow and hardening processes receive a unique writable copy of an
immutable fixture. Direct `probe` and `SimulatorSession` callers own the paths
they provide. A run records a manifest, bounded stderr, captures and metadata,
and all observed protocol records streamed to `protocol.jsonl`. The manifest
represents that evidence by relative path, record count, and SHA-256 instead of
embedding an unbounded transcript.

Host-initiated cleanup attempts input release and graceful stop when the
protocol remains usable, then always reaps the child and joins reader/writer
threads. Timeout or transport failure poisons the session and uses terminate
and kill-and-wait fallbacks when graceful shutdown cannot complete.

The operational command reference, artifact layout, Python session examples,
and troubleshooting guidance are in the
[`tools/ui-harness` README](https://github.com/EdgeTX/edgetx/blob/main/tools/ui-harness/README.md).

## Extending the harness

A new target profile should define:

- PCB and revision identity;
- LCD geometry and pixel depth;
- canonical input names and ranges;
- required and optional capabilities;
- an immutable minimal fixture; and
- one representative smoke flow.

A new native command should define its parser and bounds, capability
advertisement, execution context, completion and cancellation behavior, error
mapping, host-side validation, and native and host tests. Extend protocol v1
compatibly rather than introducing another simulator control surface.

Potential follow-up work includes additional target profiles, image comparison
and masks, a golden-image review policy, CTest or JUnit reporting, richer
telemetry scenarios, adapters over the existing Python session, and eventual
alignment with hardware-in-the-loop workflows.

## Verification

Run the host suite from the repository root:

```text
python -m unittest discover \
  -s tools/ui-harness/tests -p "test_*.py" -v
```

The dedicated CI workflow also runs the host tests on Windows and Ubuntu,
focused native automation tests with AddressSanitizer and
UndefinedBehaviorSanitizer, and a WASI build-isolation check. Fixed pass counts
and pull-request status are intentionally omitted from this page because they
change as coverage evolves.

## Design history

The design consolidates the directions explored in EdgeTX
[PR #7337](https://github.com/EdgeTX/edgetx/pull/7337) and
[PR #7646](https://github.com/EdgeTX/edgetx/pull/7646). PR #7337 by Mateusz
Urban (`onliner10`) established the reusable host harness, CLI, flow, fixture,
and host-side framebuffer-tooling direction. PR #7646 explored Windows-native
control, explicit input transitions, simulator-state injection, Lua reload,
restart, and capture synchronized with a real LCD refresh.

The implementation is a substantial redesign under one bounded protocol and
one cross-platform lifecycle. It does not carry forward the append-only command
file, a simulator-only Lua control API, native PNG encoding, or parallel
automation protocols. Thanks to Mateusz for the original harness direction and
fixture foundation.
