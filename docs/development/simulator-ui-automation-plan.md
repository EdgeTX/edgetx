# Unified Simulator UI Automation Plan

**Status:** Proposed implementation contract for collaborative review

**Related pull requests:**

- [#7337 — Add TX16S UI harness](https://github.com/EdgeTX/edgetx/pull/7337),
  authored by `onliner10`
- [#7646 — Add opt-in Widget Studio automation hooks](https://github.com/EdgeTX/edgetx/pull/7646),
  authored by `bultodepapas`

This document proposes one generic simulator automation facility built from the
strongest ideas in both pull requests. It is not a mechanical merge. Code is
retained only where its behavior, ownership, and testability fit the unified
contract.

No existing pull request should be closed until the replacement direction is
accepted, the relevant behavior is demonstrated, and each author has had a
reasonable opportunity to review the result.

## 1. Executive decision

The consolidated design will use #7337 as the product and tooling foundation:

- generic simulator automation instead of Widget Studio-specific behavior;
- one opt-in runtime mode;
- a host-side CLI and declarative scenarios;
- structured success and error responses;
- key, rotary, touch, and framebuffer capture primitives; and
- unchanged normal simulator behavior when automation is disabled.

It will incorporate requirements demonstrated by #7646:

- Windows-native command transport;
- explicit key and touch transitions rather than timed simulator commands;
- switch, analog, and telemetry state injection;
- Lua reload and simulator task restart;
- capture synchronized with an LCD refresh;
- forced invalidation for a static LVGL screen; and
- explicit coordination between the SDL thread and firmware/UI context.

The implementation will deliberately exclude:

- the `WIDGET_STUDIO` build option;
- the simulator-only Lua `simu` table;
- the append-only command file as the primary transport;
- PNG encoding and disk I/O inside the LCD refresh callback;
- `SDL_Delay` inside command execution;
- the MCP adapter in the first implementation;
- a root `pyproject.toml` or `uv.lock` solely for this tool;
- the unrelated `EDGE_TX_BUILD_TESTS` option; and
- direct mutation of Lua, telemetry, or LVGL state from the SDL thread.

The intended result is one initiative and one protocol. It may be delivered as
reviewable commits or stacked pull requests if maintainers request a smaller
review surface, but it must not create parallel control APIs.

## 2. Evidence and current integration seams

The plan is based on the current simulator architecture, not only on the two
patches.

### 2.1 Existing EdgeTX ownership points

| Existing symbol or file | Current behavior | Consequence for the unified design |
|---|---|---|
| `sdl_simu.cpp::main` | Owns SDL setup and the approximately 60 Hz host loop | Transport polling, command pumping, and warm restart coordination belong here |
| `sdl_simu.cpp::handleEvents` | Drains SDL events, then redraws | Automation work must have a bounded per-iteration budget |
| `sdl_simu.cpp::handleKeyEvents` | Calls existing simulator input setters from the SDL thread | Automation should reuse the same key/rotary path |
| `sdl_simu.cpp::redraw` | Copies the firmware LCD into the host texture and renders ImGui | Host redraw is not the same event as a firmware LCD frame |
| `sdl_simu.cpp::simuGetAnalog` | Converts ImGui stick/pot state into ADC values | Analog automation must override this path, not write `adcValues` once |
| `simulib.cpp::simuStart` | Starts firmware tasks | A task restart completes only after startup and a subsequent LCD frame |
| `simulib.cpp::simuStop` | Calls `task_shutdown_all`, which joins native tasks | A task restart may pause the SDL loop; it is not a non-blocking or cold reset |
| `simulib.cpp::simuSetKey` | Updates native simulator key state | Canonical key validation must happen before this assertion-backed API |
| `switch_driver.cpp::simuSetSwitch` | Updates switch state and asserts the index | Resolve and validate canonical switch names before dispatch |
| `simulib.cpp::simuTouchDown` / `simuTouchUp` | Feed the native touch state | The protocol must impose a valid down/move/up state machine |
| `simulib.cpp::simuLcdCopy` | Copies the current firmware framebuffer | Retain this as the capture source |
| `simulcd.cpp::lcdRefresh` / `simuRefreshLcd` | Call `simuLcdNotify` after a firmware LCD refresh | Increment the protocol display sequence and take the capture snapshot here |
| `LvglWrapper::run` | Runs LVGL in the firmware/UI context | A static-screen invalidation request must be consumed here |
| `main.cpp::perMain` / `guiMain` | Own firmware/UI periodic work and invoke Lua | Firmware-only automation mailbox work can be consumed here |
| `interface.cpp::luaTask` | Owns the Lua interpreter state machine | Lua reload completion must be observed here, not guessed by the SDL thread |
| `telemetry_sensors.cpp::setTelemetryValue` | Mutates telemetry/model state | Telemetry injection must execute in firmware context |
| `radio/src/tests/CMakeLists.txt` | Builds all native radio tests with simulator objects | Pure protocol/state tests can reuse the existing native test target |

### 2.2 What #7337 contributes

Keep or adapt:

- the generic `--automation-stdio` direction;
- the target registry and host session abstraction;
- named key mapping;
- framebuffer capture through `simuLcdCopy`;
- PPM-to-PNG conversion in the host;
- a declarative JSON scenario; and
- a minimal CLI entry point.

Redesign before reuse:

- add protocol versioning and monotonic request IDs;
- separate simulator protocol output from simulator diagnostics;
- replace `select()` plus buffered `std::cin` with raw-byte reads;
- implement the Windows native side;
- replace Python `select.select` on child pipes with reader threads;
- move press, long-press, touch duration, and wait timing to the host;
- wait for a firmware LCD refresh before capture;
- move image writing off the LCD callback;
- validate every argument and supported target capability;
- preserve partial UTF-8 bytes until a complete line is available;
- copy checked-in fixtures to a run directory before launching; and
- close pipes and reap the process on every exit path.

Drop from the first implementation:

- the MCP server;
- the native-test build toggle;
- the root Python dependency project and lockfile; and
- any fixture content not proven necessary for deterministic startup.

### 2.3 What #7646 contributes

Keep or adapt:

- explicit key down/up and touch down/move/up semantics;
- partial-line accumulation as a correctness requirement;
- reset requested at a safe loop boundary;
- Lua permanent-script reload;
- named switch and analog injection;
- the use of the existing telemetry path;
- capture armed for a later LCD refresh; and
- LVGL invalidation so a static screen produces a refresh.

Redesign before reuse:

- replace the append-only file with the common stdio protocol;
- add replies, correlation, bounds, and error propagation;
- reject a second capture instead of silently replacing the first;
- copy the framebuffer in `simuLcdNotify` but perform disk I/O elsewhere;
- distinguish a warm simulator task restart from a cold process restart;
- clear analog overrides during release/restart/stop;
- execute telemetry and Lua transitions in firmware context;
- avoid a mutex on every high-frequency ADC read;
- validate rather than silently clamp invalid inputs; and
- runtime-gate behavior instead of source-specific `WIDGET_STUDIO` definitions.

Drop:

- `api_simu.cpp`, `api_simu.h`, and Lua registration changes;
- `simu_capture.cpp`'s in-process PNG encoder;
- `stb_image_write` as a new simulator dependency;
- `--pipe` as the main transport; and
- direct file writing from `simuLcdNotify`.

### 2.4 Review findings promoted to requirements

The following are acceptance requirements, not optional cleanup:

- paths containing spaces must remain intact;
- no OS readiness check may be combined with buffered `std::cin`;
- a partial request must never block the SDL loop;
- UTF-8 must be decoded only after complete byte records are assembled;
- SDL events must be zero-initialized;
- simulator commands must not sleep to model durations;
- Python must not use `select.select` for Windows subprocess pipes;
- protocol stdout and diagnostic stderr must not be merged;
- all child pipes must be closed;
- a killed child must still be waited on; and
- a checked-in fixture must never be used as the simulator's writable settings
  directory.

## 3. Goals, scope, and success definition

### 3.1 Goals

Protocol version 1 will provide:

1. an opt-in native simulator automation mode on Linux and Windows;
2. one request grammar and one JSON response schema on both platforms;
3. bounded raw-byte input with visible parse and overflow failures;
4. strictly correlated terminal responses;
5. capability discovery for the selected simulator target;
6. key, rotary, and complete touch transitions;
7. canonical switch and analog control with explicit clearing;
8. bounded telemetry injection in firmware context;
9. Lua reload with observable completion;
10. a documented warm simulator task restart;
11. a host-driven cold process restart;
12. a monotonic firmware display sequence;
13. capture of the first complete LCD refresh after a request;
14. static-screen invalidation in the LVGL context;
15. one dependency-free Python client and CLI;
16. one deterministic TX16S-class smoke scenario; and
17. unchanged behavior when automation is not requested.

### 3.2 Non-goals

The first accepted implementation will not:

- redesign an EdgeTX screen or widget;
- change model or EEPROM formats;
- add a public or simulator-only Lua control API;
- add a network listener;
- add an MCP or AI-agent adapter;
- add golden-image approval policy;
- add a general telemetry simulator;
- support concurrent automation clients;
- guarantee hostile-client resistance;
- support every color and monochrome target;
- alter Companion or WASM behavior;
- change native-test defaults; or
- make a warm task restart equivalent to a new process.

### 3.3 Definition of success

The initiative is successful when a fresh checkout can build the reference
simulator, start it with a copied fixture, discover its capabilities, drive a
mixed input/state scenario, reload Lua, capture deterministic framebuffer
artifacts, and stop without leaked processes on Linux and Windows.

A feature being present is insufficient. Every asynchronous operation must have
a named completion point and a timeout test.

## 4. Frozen design decisions

| ID | Decision | Rationale |
|---|---|---|
| D01 | Use one generic `--automation-stdio` runtime mode | Keeps #7337's reusable scope and leaves normal runs unchanged |
| D02 | Require `--automation-output <directory>` with automation mode | Constrains writes to an explicit artifact root |
| D03 | Use UTF-8 newline-delimited text requests and JSON-line responses | Small native parser; robust host parsing |
| D04 | Requests are not JSON | Avoids adding a native JSON parser dependency |
| D05 | Request IDs are strictly increasing unsigned 64-bit values per process | Correlation without an unbounded duplicate-ID set |
| D06 | Protocol stdout contains JSON records only; diagnostics stay on stderr | Prevents logs from being mistaken for replies |
| D07 | Timed press/touch/wait actions are composed by the host | Keeps sleeps out of the SDL command path |
| D08 | `display_seq` increments only in `simuLcdNotify` | Distinguishes firmware LCD refresh from SDL/ImGui redraw |
| D09 | Capture means the first full LCD refresh after arming | Gives a precise freshness boundary |
| D10 | Capture output is PPM; PNG conversion remains in Python | Avoids another native image dependency |
| D11 | Only one asynchronous operation is active in protocol v1 | Makes cancellation, restart, and response ownership unambiguous |
| D12 | `restart` means warm `simuStop`/`simuStart`; the CLI exposes cold process restart separately | Avoids claiming a stronger reset than EdgeTX provides |
| D13 | Lua and telemetry work crosses a bounded firmware mailbox | Prevents SDL-thread mutation of firmware-owned state |
| D14 | Fixtures are immutable templates copied into a unique run directory | Prevents tracked-file mutation and cross-run contamination |
| D15 | The reference Python client serializes commands | Simplifies v1 while preserving IDs for diagnostics and later adapters |
| D16 | The simulator assumes a cooperative local peer that drains stdout | Keeps v1 transport small; output backpressure is stress-tested |
| D17 | New behavior is native-simulator-only and runtime-dormant | No physical firmware feature or size impact |
| D18 | Unknown scenario fields and unsupported capabilities are errors | Prevents typo-driven false-positive tests |
| D19 | One dedicated capture worker writes PPM artifacts | Keeps both the LCD callback and SDL loop free of capture file I/O |

## 5. Architecture and ownership

```text
Python CLI / scenario runner
  ├─ immutable fixture -> per-run writable copy
  ├─ stdin writer
  ├─ stdout binary reader thread -> JSON response dispatcher
  └─ stderr binary reader thread -> bounded diagnostic ring
                 |
                 | v1 request records / JSON response records
                 v
native stdin adapter
  POSIX: non-blocking read()
  Win32: PeekNamedPipe() + ReadFile()
                 |
                 v
bounded byte assembler -> bounded request queue
                 |
                 | maximum eight commands per SDL iteration
                 v
SDL automation session / executor
  ├─ key / rotary / touch / switch
  ├─ atomic analog overrides
  ├─ firmware mailbox -> telemetry / Lua
  ├─ warm restart state machine
  └─ capture coordinator
                 |
                 +-> LVGL invalidation request (firmware/UI context)
                 +-> simuLcdNotify() display_seq + framebuffer snapshot
                 +-> capture writer -> PPM artifact
```

### 5.1 Component responsibilities

#### `AutomationLineBuffer`

- owns raw bytes read from stdin;
- recognizes LF and CRLF;
- preserves partial lines and partial UTF-8 sequences;
- discards an overlong record through its next newline;
- creates a transport error record when possible; and
- never calls simulator, SDL, Lua, or LVGL APIs.

#### `AutomationProtocol`

- parses version, request ID, command, and arguments;
- enforces strictly increasing request IDs;
- validates UTF-8 and numeric overflow;
- preserves the remainder of a capture record as one relative path;
- serializes valid JSON, including every control character; and
- contains no SDL or firmware dependency.

#### `AutomationSession`

- owns request and response lifecycle;
- pumps a bounded number of commands per SDL iteration;
- exposes current capabilities and status;
- owns one asynchronous operation slot;
- records session epoch and display sequence;
- rejects incompatible operations while busy; and
- guarantees one terminal response for each admitted request.

#### `AutomationFirmwareMailbox`

- is a bounded SDL-producer/firmware-consumer queue;
- carries only operations that must run in firmware context;
- initially supports telemetry injection and Lua reload;
- returns completion records to the SDL session;
- is drained with a small budget from the existing periodic firmware path; and
- is inactive unless the runtime automation session exists.

#### `AutomationCapture`

- validates a relative artifact path under the configured output root;
- arms a request against the current `display_seq`;
- requests LVGL invalidation;
- takes a framebuffer snapshot in `simuLcdNotify`;
- owns one worker thread created only in automation mode;
- sends the worker an owned snapshot through a one-job queue;
- performs PPM encoding and filesystem work only on that worker;
- posts a bounded completion record back to the SDL session;
- uses a temporary file followed by an atomic rename;
- reports the captured sequence, dimensions, depth, byte count, and path; and
- never overwrites an existing artifact in protocol v1.

#### Python `Session`

- starts subprocess readers before the first request;
- uses binary pipes, never text-mode chunk decoding;
- assigns monotonic request IDs;
- validates every JSON response and ID;
- exposes host composite actions;
- fails pending requests on EOF or child exit;
- retains bounded recent stderr for diagnostics; and
- owns graceful stop, terminate, kill, wait, close, and thread join.

### 5.2 Thread and context ownership

| Operation | Owner | Allowed work | Forbidden work |
|---|---|---|---|
| Raw stdin polling | SDL loop | Read bounded bytes and assemble records | Blocking read or simulator mutation |
| Parse and dispatch | SDL loop | Validate and call existing simulator input APIs | Sleep or wait for firmware work |
| Key / rotary / touch / switch | SDL loop | Reuse current simulator setters | Direct LVGL or Lua access |
| Analog override write | SDL loop | Atomic packed override update | Per-read heap allocation |
| Analog override read | ADC conversion path | Atomic load, then normal fallback | Taking the session mutex on every sample |
| Telemetry injection | Firmware periodic context | Call the existing telemetry/model path | SDL-thread model mutation |
| Lua reload transition | Firmware/Lua context | Set and observe interpreter state | SDL-thread write of `luaState` |
| LVGL invalidation | Firmware/UI context | Invalidate the active screen | SDL-thread LVGL call |
| LCD notification | Firmware display context | Increment sequence and copy a snapshot | File I/O or PNG encoding |
| PPM write | Dedicated capture worker | Encode the owned snapshot and rename | Access a live LVGL buffer or emit protocol responses |
| Warm restart | SDL loop between iterations | Stop joined tasks, clear session state, start tasks | Claim cold-process semantics |
| Host timing | Python main thread | Monotonic waits and composite actions | Simulator `SDL_Delay` |

### 5.3 Cross-context invariants

- A request ID has exactly one owner until its terminal response is queued.
- A pending capture owns its request ID, epoch, target path, minimum display
  sequence, snapshot buffer, and terminal state.
- `restart` is admitted only while `Ready`; while another asynchronous
  operation is active it returns `operation_busy` and does not preempt it.
- Stop, EOF, or broken transport cancels capture, frame wait, and
  firmware-mailbox work with one visible terminal error when output remains
  available.
- All active keys and touch state are released before restart or stop.
- Analog overrides are cleared on `release-all`, warm restart, and process stop.
- A telemetry or Lua completion from an older epoch is ignored and counted.
- `display_seq` is monotonic for the life of the process and does not reset when
  the epoch changes.
- The first ready session uses epoch 1; zero is reserved for pre-readiness
  diagnostics.
- No command may hold a shared-state lock while calling firmware, SDL, LVGL, or
  filesystem code.

### 5.4 Exact simulator bridge hooks

The implementation adds only these cross-context calls outside the simulator
automation files:

1. `perMain()` calls a SIMU-only `simuAutomationBeforeUi()` after periodic
   housekeeping and before `MainWindow::run()`. It consumes at most two
   firmware-mailbox operations, so telemetry is visible to that UI cycle and a
   Lua reload can enter the existing interpreter state machine.
2. `perMain()` calls `simuAutomationAfterUi()` after `guiMain()`. It publishes
   completed telemetry work and stable Lua reload state for the matching
   generation.
3. `LvglWrapper::run()` calls `simuConsumeLcdInvalidateRequest()` immediately
   before `lv_timer_handler()`. Only that UI-owned call may invoke
   `lv_obj_invalidate(lv_scr_act())`.
4. The existing `simuLcdNotify()` host callback increments `display_seq` and,
   when capture is armed, copies the complete framebuffer into preallocated
   owned storage. It signals the capture worker and returns.

All four hooks are no-ops without an active runtime automation session. They
are compiled under `#if defined(SIMU) && !defined(__wasm__)` and the existing
native simulator CMake boundary, not by a new user-visible product option.

## 6. Protocol version 1

### 6.1 Activation

The reference launch form is:

```text
simu --automation-stdio \
     --automation-output <absolute-run-artifact-directory> \
     --storage <run-sdcard-copy> \
     --settings <run-settings-copy>
```

Rules:

- `--automation-output` is valid only with `--automation-stdio`.
- The output directory must exist and be a directory.
- Automation startup fails before `simuStart` if stdin is unsuitable, the output
  root cannot be canonicalized, or stdout cannot be opened.
- In automation mode, stdout is reserved for protocol JSON lines.
- SDL, TRACE, and harness diagnostics go to stderr.
- EOF on stdin cancels pending operations, releases inputs, and requests a clean
  simulator exit. No response is possible after EOF.
- Broken stdout is treated as loss of the controlling peer and also requests
  exit.

### 6.2 Request record grammar

```text
v1 <request-id> <command> [arguments...]\n
```

- Records are UTF-8.
- There is no leading whitespace.
- Horizontal ASCII space separates fixed tokens.
- A trailing `\r` is removed before parsing.
- `request-id` is decimal, in `1..UINT64_MAX`, and greater than every previously
  admitted ID in the current process.
- Once a valid `v1 <request-id>` prefix is recognized, that ID is consumed even
  if command parsing later fails; a client must never reuse it.
- Blank records are ignored and are not admitted requests.
- Fixed command and enum tokens are ASCII.
- For `capture`, the complete remainder after the command separator is the
  relative path, including internal spaces.
- NUL, invalid UTF-8, missing newline beyond the record limit, and numeric
  overflow are errors.

Examples:

```text
v1 1 ping
v1 2 status
v1 3 key-down ENTER
v1 4 key-up ENTER
v1 5 rotate -2
v1 6 touch-down 120 80
v1 7 touch-move 130 82
v1 8 touch-up
v1 9 set-switch SA 1
v1 10 set-analog AIL 2048
v1 11 clear-analog AIL
v1 12 set-telemetry 61696 0 1 115 1 1 RSSI
v1 13 reload-lua
v1 14 wait-frame 185
v1 15 capture checkpoints/home screen.ppm
v1 16 restart
v1 17 release-all
v1 18 stop
```

### 6.3 Response records

Success:

```json
{"version":1,"type":"response","id":15,"ok":true,"epoch":1,"result":{"display_seq":186,"path":"checkpoints/home screen.ppm","width":480,"height":272,"depth":16,"bytes":391695}}
```

Failure:

```json
{"version":1,"type":"response","id":15,"ok":false,"epoch":1,"error":{"code":"artifact_exists","message":"capture target already exists"}}
```

Uncorrelated transport event:

```json
{"version":1,"type":"event","id":null,"epoch":1,"event":{"code":"invalid_record","message":"request id could not be recovered"}}
```

Rules:

- Every admitted request receives exactly one terminal response.
- Parse failures receive a response when a valid request ID can be recovered.
- The reference host rejects a response with an unknown, repeated, or mismatched
  ID.
- JSON strings escape quotes, backslashes, and all bytes below U+0020.
- JSON is emitted as valid UTF-8 and one object per line.
- A response is flushed before `stop` causes the SDL quit event.
- Diagnostic text never appears on stdout.

### 6.4 Status and discovery

`status` returns at least:

- protocol version;
- process running state;
- target/flavour identifier;
- LCD width, height, and depth;
- current epoch and `display_seq`;
- current asynchronous operation;
- request queue depth;
- firmware mailbox depth;
- line and queue overflow counters;
- active key/touch state;
- count of analog overrides;
- Lua state as a stable protocol enum;
- whether the target supports rotary, touch, switch, analog, telemetry, Lua,
  capture, and warm restart; and
- the canonical output-root status without exposing unrelated filesystem paths.

`describe` returns bounded canonical names and ranges for:

- supported keys;
- physical switches and their valid positions;
- ADC inputs;
- LCD coordinates and pixel depth; and
- optional command capabilities.

If the bounded description would exceed the response limit, startup must fail
for that target rather than emit truncated JSON.

### 6.5 Core command contract

| Command | Arguments and validation | Terminal completion |
|---|---|---|
| `ping` | none | Request dispatch |
| `status` | none | Consistent session snapshot collected |
| `describe` | none | Capability description serialized |
| `key-down` | supported canonical key; key currently up | Key state updated |
| `key-up` | supported canonical key; key currently down | Key state updated |
| `rotate` | non-zero signed steps in `-128..128`; target supports rotary | Existing rotary event updated |
| `touch-down` | in-bounds x/y; no active touch | Touch state becomes active |
| `touch-move` | in-bounds x/y; touch active | Position update delivered through the existing touch path |
| `touch-up` | no arguments; touch active | Touch state released |
| `set-switch` | canonical default switch name; valid target position | Existing switch state updated |
| `set-analog` | canonical ADC name; value `0..4096` | Atomic override published |
| `clear-analog` | canonical ADC name or `all` | Override cleared |
| `set-telemetry` | validated ID tuple, int32 value, unit, precision, optional bounded name | Firmware mailbox operation completed |
| `reload-lua` | no arguments; target has Lua; session idle | Interpreter reaches `running` or `panic` for the requested reload generation |
| `wait-frame` | minimum `display_seq` greater than or equal to current value | Sequence reaches minimum or the operation times out at the host |
| `capture` | safe relative `.ppm` path under output root | Fresh snapshot is written and atomically renamed |
| `restart` | no arguments; session idle | Tasks restarted, epoch incremented, and first new LCD frame observed |
| `release-all` | none | Keys, touch, and analog overrides are clear |
| `stop` | none | Response flushed and zero-initialized SDL quit event accepted |

### 6.6 Host-only composite actions

These are scenario actions, not simulator commands:

| Host action | Expansion |
|---|---|
| `press(key, hold_ms)` | `key-down`, monotonic host wait, `key-up` |
| `long-press(key, hold_ms)` | Same primitives with a longer validated hold |
| `tap(x, y, hold_ms)` | `touch-down`, host wait, `touch-up` |
| `drag(points, step_ms)` | `touch-down`, ordered `touch-move` calls, `touch-up` |
| `wait-ms(ms)` | Host monotonic wait only |
| `wait-next-frame(timeout)` | Snapshot status, then `wait-frame(current + 1)` |
| `restart-process` | Graceful stop, process reap, relaunch from a fresh fixture copy |
| `capture-png(name)` | Native PPM capture followed by host PNG conversion |

Hold durations are bounded by the host schema. A `finally` path always attempts
the matching release if a composite action fails.

### 6.7 Error code taxonomy

Stable codes are machine-consumable. Messages are diagnostic and may improve.

| Category | Codes |
|---|---|
| Record | `invalid_utf8`, `line_too_long`, `invalid_record`, `unsupported_version` |
| ID | `invalid_id`, `id_not_monotonic` |
| Command | `unknown_command`, `missing_argument`, `extra_argument`, `invalid_argument`, `out_of_range` |
| Capability | `unsupported_command`, `unsupported_target`, `unsupported_lcd_depth` |
| Input state | `key_already_down`, `key_not_down`, `touch_already_down`, `touch_not_down` |
| Capacity | `queue_full`, `firmware_queue_full`, `operation_busy`, `response_too_large` |
| Lifecycle | `session_stopping`, `restart_failed`, `lua_unavailable`, `lua_panic` |
| Capture | `unsafe_path`, `path_too_long`, `artifact_exists`, `capture_failed`, `capture_cancelled` |
| Transport | `stdin_closed`, `stdout_closed`, `io_error` |
| Internal | `invariant_violation`, `internal_error` |

### 6.8 Bounds and backpressure

| Resource | Initial bound | Behavior at limit |
|---|---:|---|
| OS bytes read per SDL iteration | 4096 | Continue next iteration |
| Request record | 16 KiB | Discard through newline; `line_too_long` |
| Partial record buffer | 16 KiB | Enter discard mode until newline |
| Pending request queue | 64 records | `queue_full` for fully read records |
| Commands executed per SDL iteration | 8 | Leave remainder queued |
| Firmware mailbox | 16 operations | `firmware_queue_full` |
| Active asynchronous operation | 1 | `operation_busy` |
| Capture relative path | 1024 UTF-8 bytes | `path_too_long` |
| Response line | 16 KiB | Minimal `response_too_large` response |
| Host recent stderr | 200 lines / 256 KiB | Drop oldest line |
| Scenario steps | 1000 | Reject scenario before launch |
| Host command timeout | command-specific, maximum 60 s | Fail request and clean session |
| Capture writer | one job | Second capture rejected |

The reference host begins stdout/stderr readers before sending `ping` and sends
one command at a time. This satisfies D16 for normal operation. Hardening tests
must pause the reader deliberately; if the SDL loop exceeds the agreed latency
budget, a bounded response-writer queue becomes mandatory before non-draft
review.

Performance budgets are measured against an automation-disabled run on the
same machine and build:

- idle automation polling adds no more than 1 ms to p95 SDL-loop duration;
- a normal serialized command does not create an SDL iteration over 50 ms;
- the TX16S framebuffer copy in `simuLcdNotify` remains below 5 ms at p99; and
- capture encoding time is excluded from SDL timing because it runs only on the
  dedicated worker.

Warm restart is measured separately because `task_shutdown_all()` deliberately
joins firmware tasks.

### 6.9 Session state machine

```text
Starting
  ├─ ping / status ─> allowed (epoch 0)
  └─ first LCD notification ─> Ready (epoch 1)

Ready
  ├─ wait-frame ─> WaitingForFrame ─> Ready
  ├─ capture ─> AwaitingCaptureFlush ─> WritingCapture ─> Ready
  ├─ set-telemetry ─> WaitingForFirmware ─> Ready
  ├─ reload-lua ─> ReloadingLua ─> Ready
  ├─ restart ─> RestartStopping ─> RestartStarting
  |                └─ first LCD notification ─> Ready (epoch + 1)
  └─ stop / EOF / broken peer ─> Stopping ─> Stopped

Any asynchronous state
  ├─ status / ping ─> allowed
  ├─ incompatible mutation ─> operation_busy
  └─ stop / transport loss ─> cancel operation, release inputs, stop
```

The reference host serializes commands, but the simulator state machine still
defends itself against invalid sequences.

## 7. Deterministic capture contract

### 7.1 Artifact containment

At startup the simulator canonicalizes `--automation-output`. A capture path:

- must be relative;
- must be valid UTF-8;
- may contain internal spaces;
- must end in `.ppm`;
- must not contain a root name, root directory, NUL, `.`, or `..` component;
- must remain below the canonical output root after resolution;
- must have an existing parent directory created by the host; and
- must not already exist.

The simulator never creates arbitrary parent directories and never overwrites a
capture. The host creates a unique run root, so this does not inconvenience
normal flows.

### 7.2 Capture sequence

1. The SDL executor validates the request and records `armed_after_seq`.
2. It reserves the single asynchronous slot and snapshot buffer.
3. It raises a simulator LCD invalidation request.
4. `LvglWrapper::run` consumes that flag in firmware/UI context and invalidates
   the active screen.
5. LVGL performs a complete refresh through the normal display driver.
6. `simuLcdNotify` increments `display_seq`.
7. If the new sequence is greater than `armed_after_seq`, the callback copies
   `simuLcdBuf` into the owned capture buffer.
8. The callback marks the snapshot ready and returns without filesystem work.
9. The capture writer converts the owned RGB565 snapshot to PPM in a temporary
   file.
10. It closes the file, verifies the byte count, and renames it to the requested
    final path.
11. The SDL executor emits the terminal response with metadata.

### 7.3 Pixel and file semantics

Version 1 requires the reference TX16S-class RGB565 framebuffer. It writes PPM
P6 with deterministic RGB565-to-RGB888 expansion. Other LCD depths are reported
through discovery but capture returns `unsupported_lcd_depth` until they have
their own tested conversion.

The native artifact contains no timestamp or host metadata. The host writes a
separate JSON manifest and may convert PPM to PNG. Repeating the same state must
therefore produce byte-identical PPM data.

### 7.4 Capture failure and cancellation

- A second capture receives `operation_busy`.
- `restart` is rejected while capture is pending; stop or transport loss
  cancels a capture awaiting a flush.
- A snapshot already being written completes or removes its temporary file;
  shutdown joins the capture worker before process teardown continues.
- File-open, short-write, close, and rename failures are distinct diagnostic
  messages under `capture_failed`.
- No terminal success is emitted until the final artifact exists.
- A late LCD notification from a previous epoch cannot complete a new capture.

## 8. State injection and lifecycle semantics

### 8.1 Keys, touch, and rotary

- Canonical key names come from a fixed protocol mapping and are filtered by
  `keysGetSupported`.
- Duplicate down and unmatched up are errors, not silent no-ops.
- `release-all` is idempotent and is also called by cleanup.
- Rotary steps are bounded to avoid overflow; larger motion is composed by the
  host.
- Touch coordinates use framebuffer coordinates, independent of host-window
  scale.
- Move without down and repeated up are visible flow errors.

### 8.2 Switches

- Protocol names are canonical hardware defaults such as `SA`, not localized or
  model-custom labels.
- The resolver iterates existing switch definitions and checks target presence.
- A two-position switch rejects position `0`.
- A three-position switch accepts `-1`, `0`, and `1`.
- A warm restart restores the simulator's documented default switch state.

### 8.3 Analog overrides

Each ADC input uses one packed atomic value containing an enabled flag and the
`0..4096` value. This avoids a mutex in the ADC sampling path.

- `set-analog` replaces the current override.
- `clear-analog` restores normal ImGui/simulator input.
- `clear-analog all` clears every override.
- `release-all`, warm restart, and stop clear all overrides.
- Discovery exposes canonical ADC names from the HAL mapping.

### 8.4 Telemetry

`set-telemetry` enters the firmware mailbox and runs through the existing
`PROTOCOL_TELEMETRY_LUA` sensor path in firmware context. It validates:

- ID `1..65535`;
- sub-ID `0..7`;
- instance `0..255`;
- signed 32-bit value;
- a known unit;
- supported precision; and
- an optional bounded sensor name.

The optional sensor name is one ASCII token matching
`[A-Za-z0-9_-]{1,TELEM_LABEL_LEN}`; spaces and locale-dependent labels are not
accepted in protocol v1.

If a model sensor must be initialized, changes occur only in the run's copied
settings. Telemetry cleanup for v1 is a cold process restart; the protocol does
not invent partial sensor deletion semantics.

### 8.5 Lua reload

The SDL thread posts a reload generation to the firmware mailbox. The firmware
periodic path:

1. accepts the generation when Lua is available;
2. transitions to `INTERPRETER_RELOAD_PERMANENT_SCRIPTS`;
3. lets the existing `luaTask` state machine load scripts; and
4. publishes completion only when the same generation reaches
   `INTERPRETER_RUNNING` or `INTERPRETER_PANIC`.

The response reports generation and final stable state. A request accepted into
the mailbox is not yet a successful reload.

### 8.6 Warm restart versus cold restart

`restart` is a warm simulator task restart:

1. reject if another asynchronous operation owns the slot;
2. release key, touch, and analog state;
3. verify the firmware mailbox is idle and discard stale completion records;
4. call `simuStop` between SDL iterations;
5. call `simuStart`;
6. increment `epoch`;
7. wait for the first LCD notification in the new epoch; and
8. return the new epoch and `display_seq`.

Because `simuStart` intentionally preserves some native static state, this is
not a clean test isolation boundary. The host `restart-process` action is the
authoritative cold reset: stop/reap the process, make a fresh writable fixture
copy, relaunch, and wait for readiness.

## 9. Host harness contract

### 9.1 Process lifecycle

The Python host uses `subprocess.Popen` with argument arrays and binary pipes:

- `stdin=PIPE`;
- `stdout=PIPE` for protocol only;
- `stderr=PIPE` for diagnostics;
- `text=False` and no implicit chunk decoding; and
- no shell interpolation.

Startup:

1. create a unique run directory;
2. copy fixture templates;
3. create the artifact tree;
4. launch the simulator;
5. start stdout and stderr reader threads;
6. send `ping`;
7. send `status` until `Ready` or timeout; and
8. validate target dimensions and required capabilities.

There is no fixed startup `sleep`.

Shutdown escalation:

1. attempt `release-all`;
2. send `stop` and wait for its response;
3. close stdin;
4. wait for the child;
5. on timeout, terminate and wait;
6. on a second timeout, kill and wait;
7. close stdout and stderr in `finally`;
8. join reader threads; and
9. report exit code and retained diagnostics.

Every path, including an already-dead child, executes pipe closure and `wait`.

### 9.2 Response reader

A blocking binary reader thread is portable across POSIX and Windows child
pipes. It:

- accumulates bytes;
- splits on newline bytes;
- preserves a partial trailing record;
- decodes only a complete line as UTF-8;
- parses JSON;
- routes by request ID into a thread-safe queue; and
- fails the session on malformed protocol output.

The stderr reader uses the same byte discipline but stores text in a bounded
ring. It never participates in response matching.

### 9.3 Scenario schema

One strict JSON representation is accepted. Equivalent alternate shapes are
not supported.

```json
{
  "schema": 1,
  "target": "tx16s",
  "requires": ["rotary", "touch", "analog", "telemetry", "lua", "capture"],
  "steps": [
    {"action": "wait-ready", "timeout_ms": 10000},
    {"action": "press", "key": "ENTER", "hold_ms": 120},
    {"action": "wait-next-frame", "timeout_ms": 3000},
    {"action": "set-switch", "name": "SA", "position": 1},
    {"action": "set-analog", "name": "AIL", "value": 2048},
    {"action": "set-telemetry", "id": 61696, "sub_id": 0, "instance": 1, "value": 115, "unit": 1, "precision": 1, "name": "RSSI"},
    {"action": "reload-lua", "timeout_ms": 10000},
    {"action": "capture", "name": "home", "format": "png"}
  ]
}
```

Validation occurs before process launch:

- unknown top-level keys or step keys fail;
- every action has one canonical shape;
- numbers are range checked;
- artifact names are normalized but not silently changed;
- required capabilities are compared with `describe`; and
- the maximum step count is enforced.

### 9.4 Fixture policy

- Checked-in fixtures are read-only templates.
- Every run copies them under `build/ui-harness/runs/<run-id>/settings`.
- SD-card content and settings are separate copies.
- The simulator receives only run-copy paths.
- The host records a hash of the source fixture.
- After proving which fields are required, unused generated content is removed.
- Fixture minimization must not rely on an unreviewed assumption that missing
  YAML fields are harmless.

### 9.5 Artifact manifest

Each run produces:

```text
build/ui-harness/runs/<run-id>/
  settings/
  sdcard/
  artifacts/
    checkpoints/
      home.ppm
      home.png
  manifest.json
  protocol.jsonl
  stderr.log
```

`manifest.json` records:

- schema version;
- EdgeTX commit;
- target and dimensions;
- platform and Python version;
- fixture hash;
- start/end time;
- exit code;
- every request ID, command, response, epoch, and display sequence;
- artifact SHA-256 values; and
- failure step and recent diagnostics when applicable.

Timestamps live in the manifest, not in native image bytes.

## 10. Proposed repository and build layout

```text
radio/src/targets/simu/
  automation_protocol.h
  automation_protocol.cpp
  automation_session.h
  automation_session.cpp
  automation_transport.h
  automation_transport.cpp
  automation_capture.h
  automation_capture.cpp
  automation_bridge.h
  automation_bridge.cpp
  arg_parser.cpp
  arg_parser.h
  sdl_simu.cpp
  simulib.cpp
  simulib.h

radio/src/gui/colorlcd/
  LvglWrapper.cpp                 # tiny SIMU-only invalidation hook

radio/src/
  main.cpp                        # tiny SIMU-only firmware mailbox hook

radio/src/tests/
  simu_automation.cpp

tools/ui-harness/
  edgetx-ui
  README.md
  PROTOCOL.md
  edgetx_ui/
    __init__.py
    cli.py
    protocol.py
    session.py
    flow.py
    ppm.py
  tests/
    test_protocol.py
    test_flow.py
    test_ppm.py
  flows/
    tx16s-smoke.json
  fixtures/
    tx16s/
      settings/
      sdcard/
```

This is the recommended first implementation split. A maintainer-requested
rename is acceptable, but the following boundaries are mandatory:

- byte assembly and protocol parsing remain testable without SDL;
- capture writing is separate from `simuLcdNotify`;
- host subprocess management is separate from flow interpretation; and
- `sdl_simu.cpp` does not become a second monolithic 200-line command parser.

### 10.1 CMake constraints

- No new user-visible build option is required.
- `automation_session`, `automation_transport`, and `automation_capture` are
  added only to the native interactive `simu` executable.
- `automation_protocol` is compiled into both `simu` and `gtests-radio` without
  an SDL dependency.
- `automation_bridge` is added to native simulator objects only when
  `NOT WASI`, so `main.cpp`, `LvglWrapper.cpp`, and tests resolve the four
  SIMU-only bridge hooks.
- Hook call sites use `#if defined(SIMU) && !defined(__wasm__)`, preventing an
  unresolved bridge or behavior change in firmware and WASM builds.
- Nothing is added to physical firmware targets.
- Nothing is added to `wasi-module`.
- No new FetchContent dependency is introduced.
- Existing native tests remain enabled by default.
- Building `--target simu` remains the harness build path.

## 11. Detailed implementation phases

Each phase is independently reviewable. A phase does not start by hiding failed
exit criteria from the previous phase.

### Phase 0 — Agreement, baseline, and provenance

#### 0.1 Freeze the contract

- Review D01–D19 with `onliner10` and simulator maintainers.
- Confirm `--automation-stdio` and `--automation-output` naming.
- Confirm stdout/stderr separation.
- Confirm warm versus cold restart terminology.
- Confirm that MCP and the Lua `simu` table remain deferred.

#### 0.2 Record provenance

Create a PR table listing, per retained component:

- source PR and original file;
- whether code, algorithm, test idea, or requirement is reused;
- expected author of the implementation commit; and
- whether a `Co-authored-by` trailer has contributor approval.

#### 0.3 Establish baseline

Record on Linux and Windows where available:

- clean `upstream/main` native tests;
- TX16S simulator build command and time;
- idle SDL-loop frame time;
- p50, p95, and p99 SDL-loop duration with automation disabled and enabled but
  idle;
- normal simulator command-line help;
- physical firmware size for a representative target; and
- absence of automation symbols in firmware/WASM outputs.

**Deliverables:** accepted decision table, provenance table, baseline log.

**Exit:** no unresolved architectural objection to one protocol. Original PRs
remain open.

### Phase 1 — Pure protocol, bounds, and state model

#### 1.1 Add protocol data types

Implement:

- request/response models;
- stable error enum;
- command enum;
- target capability model;
- session epoch and display sequence types; and
- asynchronous-operation enum.

#### 1.2 Add raw byte assembly

Implement and test:

- LF and CRLF;
- multiple records in one feed;
- every split position of representative UTF-8 input;
- discard-through-newline after overflow;
- EOF with and without a partial record; and
- fixed memory bounds.

#### 1.3 Add strict parser and JSON serializer

Implement:

- version and monotonic ID validation;
- exact arity checks;
- overflow-safe integer conversion;
- capture remainder parsing;
- UTF-8 validation;
- complete JSON escaping; and
- response-size enforcement.

#### 1.4 Add pure session state tests

Model legal and illegal transitions for:

- key and touch state;
- one asynchronous operation;
- cancellation;
- restart epoch change; and
- exactly-once terminal response ownership.

**Tests:** P01–P18, S01–S12.

**Exit:** `gtests-radio` passes parser/state tests with no SDL window and no new
external dependency.

### Phase 2 — Native transport and runtime activation

#### 2.1 Add command-line activation

Add and test:

- `--automation-stdio`;
- required `--automation-output` dependency;
- help text;
- invalid combinations;
- output-root canonicalization; and
- automation-disabled defaults.

#### 2.2 Implement POSIX input

Use raw non-blocking `read` with:

- no `std::cin`;
- a 4096-byte per-iteration budget;
- EAGAIN/EWOULDBLOCK handling;
- EOF and error distinction; and
- no change to SDL frame pacing.

#### 2.3 Implement Win32 input

Use redirected standard input with:

- `GetStdHandle` and handle validation;
- `GetFileType`;
- `PeekNamedPipe` to determine available bytes;
- bounded `ReadFile`;
- broken-pipe handling; and
- no console-only assumption.

#### 2.4 Reserve protocol stdout

- Emit protocol JSON only on stdout.
- Route automation diagnostics to stderr.
- Flush complete response lines.
- Ignore SIGPIPE or handle its platform equivalent.
- Exit cleanly when the controlling pipe disappears.

#### 2.5 Add a temporary protocol probe

Before the full host exists, add a test helper that:

- launches the simulator;
- sends `ping` and malformed records;
- verifies IDs and EOF behavior; and
- runs on both platforms without Python `select`.

The helper evolves into the Phase 3 session code; it is not a second API.

**Tests:** T01–T16, B01–B04.

**Exit:** identical byte vectors and response JSON pass on Linux and Windows;
idle automation adds no blocking read to the SDL loop.

### Phase 3 — Cross-platform Python session foundation

#### 3.1 Implement binary response readers

- Start stdout and stderr threads before first request.
- Preserve partial bytes across reads.
- Route protocol responses by ID.
- Bound stderr memory.
- Fail on malformed stdout.

#### 3.2 Implement request lifecycle

- Generate strictly increasing IDs.
- Serialize one request at a time.
- Attach command-specific timeouts.
- Detect child exit while waiting.
- Include request ID and recent stderr in errors.

#### 3.3 Implement process cleanup

Cover:

- clean `stop`;
- simulator error response;
- request timeout;
- child crash;
- terminate escalation;
- kill escalation;
- pipe closure; and
- final `wait` and thread join.

#### 3.4 Implement readiness

Replace startup sleeps with:

- `ping`;
- `status`;
- first-frame readiness; and
- capability validation.

**Tests:** H01–H14.

**Exit:** 100 start/ping/stop cycles leave no child process, open pipe warning,
or zombie on Linux and Windows.

### Phase 4 — Core input and display barriers

#### 4.1 Add discovery and status

Implement `status` and `describe` with bounded response output.

#### 4.2 Add key and rotary primitives

- Reuse existing key and rotary functions.
- Filter target-supported keys.
- Enforce key state transitions.
- Bound rotary steps.
- Zero-initialize every synthetic SDL event.

#### 4.3 Add touch primitives

- Use framebuffer coordinates.
- Enforce down/move/up order.
- Validate every edge and out-of-bounds coordinate.
- Guarantee cleanup release.

#### 4.4 Add display sequence

- Increment only from `simuLcdNotify`.
- Publish it atomically.
- Implement `wait-frame` as an asynchronous operation.
- Keep SDL redraw counters out of the protocol.

#### 4.5 Add host composite actions

Implement validated `press`, `long-press`, `tap`, `drag`,
`wait-next-frame`, and cleanup-on-failure.

**Tests:** I01–I18, F01–F06.

**Exit:** key, rotary, and touch scenarios complete without simulator-side
duration waits, and a host can wait for a real LCD refresh.

### Phase 5 — Render-complete capture

#### 5.1 Add safe artifact paths

Implement canonical containment, extension check, parent check, no-overwrite
policy, length limit, and spaces.

#### 5.2 Add invalidation handoff

- SDL raises a request.
- Firmware/UI consumes it.
- LVGL invalidates the active screen.
- Default simulator behavior remains unchanged with no request.

#### 5.3 Add snapshot handoff

- Arm against `display_seq`.
- Copy only after a newer full LCD notification.
- Copy into owned storage.
- Perform no file I/O in `simuLcdNotify`.

#### 5.4 Add deterministic PPM writer

- RGB565-to-RGB888 conversion tests;
- temporary-file write;
- complete byte-count validation;
- close and atomic rename; and
- cleanup of partial artifacts.

#### 5.5 Add host PNG conversion and metadata

Reuse the useful #7337 host-side concept with standard-library code and verify
dimensions, PPM hash, PNG decode, and manifest contents.

**Tests:** C01–C18.

**Exit:** 20 repeated captures of one static checkpoint are byte-identical;
one deliberate pixel change changes the hash; paths containing spaces work on
Linux and Windows.

### Phase 6 — State injection and lifecycle

This is deliberately after the core transport/capture path because it crosses
more ownership boundaries.

#### 6.1 Add switch control

Resolve canonical names, validate switch type, and test all valid positions.

#### 6.2 Add atomic analog overrides

Add set, replace, clear-one, clear-all, cleanup, and fallback behavior without a
per-sample mutex.

#### 6.3 Add firmware mailbox

Implement bounded request/completion queues, per-iteration budget, epoch tags,
and stale-completion rejection.

#### 6.4 Add telemetry injection

Execute the existing telemetry/model path from firmware context and verify a
real widget-visible sensor value using only the writable run fixture.

#### 6.5 Add Lua reload generation

Post a generation, observe existing Lua state transitions, return running or
panic, and test missing/broken scripts.

#### 6.6 Add warm restart state machine

Require an idle asynchronous slot, release state, purge stale completions,
stop/join tasks, start, increment epoch, wait for the first new LCD frame, and
report failure visibly.

#### 6.7 Add cold host restart

Reap the old process, create a new fixture copy, relaunch, and prove that
telemetry and overrides do not leak.

**Tests:** V01–V12, L01–L16.

**Exit:** state injection is visible in the UI, Lua completion is observed rather
than assumed, and warm/cold restart semantics are demonstrably different.

### Phase 7 — Scenario, fixture, and developer UX

#### 7.1 Finalize strict flow schema

Validate all steps before launch and reject unknown data.

#### 7.2 Minimize the TX16S fixture

Start from the proven #7337 fixture, preserve attribution, and remove content
only when startup and smoke tests prove it unnecessary.

#### 7.3 Add one representative smoke flow

The flow must cover:

- readiness;
- key press;
- rotary input;
- touch input;
- switch and analog injection;
- telemetry injection;
- Lua reload;
- wait for a real display frame;
- static-screen capture;
- input release; and
- clean stop.

#### 7.4 Add artifacts and diagnostics

Write manifest, protocol log, bounded stderr log, PPM, PNG, hashes, and failed
step.

#### 7.5 Document use and troubleshooting

Document build, run, protocol, output layout, unsupported targets, timeouts,
Windows behavior, and cleanup.

**Tests:** Q01–Q10.

**Exit:** one documented command builds, runs the smoke scenario, produces
verified artifacts, and exits nonzero on any failed step.

### Phase 8 — Hardening, CI, and review readiness

#### 8.1 Run native correctness matrix

- parser/state unit tests on Linux and Windows;
- normal radio tests;
- simulator builds;
- automation-disabled CLI test; and
- representative firmware build/size comparison.

#### 8.2 Run transport stress

- 10,000 requests;
- every relevant byte split;
- 64/65 queue boundary;
- slow stdout reader;
- stdin EOF;
- broken stdout;
- malformed UTF-8;
- maximum path/record/response; and
- forced child crash.

The supported-client stress case must satisfy the performance budgets in
section 6.8. If delayed stdout draining causes an iteration over 50 ms, Phase 8
adds a bounded writer queue and reruns T13–T16 before review.

#### 8.3 Run lifecycle stress

- 100 process start/stop cycles;
- 20 Lua reloads;
- 20 warm restarts;
- restart requests during each asynchronous state, which must return
  `operation_busy` without preemption;
- capture write failure;
- no leftover temporary artifacts; and
- no stale completion crossing epochs.

#### 8.4 Run deterministic visual smoke

- 20 identical static captures;
- changing-screen capture;
- one-pixel mutation;
- unique artifact directories; and
- Linux/Windows hash comparison where pixel format is expected to match.

#### 8.5 Verify scope isolation

- no Lua `simu` table;
- no `WIDGET_STUDIO` option;
- no append-only transport;
- no MCP;
- no root Python dependency project;
- no native PNG dependency;
- no automation code in firmware/WASM; and
- normal simulator behavior unchanged.

#### 8.6 Update proposal to accepted contract

Replace future tense, record final deviations and rationale, and link test/CI
evidence.

**Exit:** all mandatory checks are green and the replacement is ready to leave
draft.

## 12. Test catalogue

### 12.1 Protocol tests

| ID range | Coverage |
|---|---|
| P01–P04 | valid records, LF/CRLF, multiple records, blank record |
| P05–P08 | version, missing ID, zero/max/overflow ID, non-monotonic ID |
| P09–P12 | unknown command, missing/extra arguments, numeric syntax/range |
| P13–P15 | valid UTF-8, split multi-byte path, invalid UTF-8 |
| P16–P18 | exact line limit, one byte over, complete JSON escaping |

### 12.2 Session-state and build-isolation tests

| ID range | Coverage |
|---|---|
| S01–S04 | request ownership, one terminal response, duplicate completion, release |
| S05–S08 | legal async transitions, busy rejection, stop cancellation, timeout cleanup |
| S09–S12 | epoch increment, stale completion, queue purge, stop state |
| B01–B02 | CLI flag combinations and Linux/Windows simulator build |
| B03–B04 | automation-disabled behavior and firmware/WASM isolation |

### 12.3 Transport and host tests

| ID range | Coverage |
|---|---|
| T01–T04 | one-byte feeds, partial line, multi-line read, read budget |
| T05–T08 | POSIX EAGAIN/EOF/error and Windows empty/broken pipe |
| T09–T12 | 63/64/65 queue records and bounded pump fairness |
| T13–T16 | stdout separation, broken peer, stop flush, disabled mode |
| H01–H04 | startup readiness, response correlation, stderr isolation, crash |
| H05–H08 | timeout, graceful stop, terminate, kill-and-wait |
| H09–H12 | pipe closure, reader join, 100 lifecycle cycles, no warning |
| H13–H14 | fixture copy isolation and nonzero flow failure exit |

### 12.4 Input and frame tests

| ID range | Coverage |
|---|---|
| I01–I04 | supported/unsupported key, duplicate down, unmatched up |
| I05–I08 | rotary negative/positive/zero/out-of-range |
| I09–I14 | touch state order, edges, outside bounds, cleanup |
| I15–I18 | composite press/tap failure releases and target capability errors |
| F01–F03 | display sequence only changes on LCD notification |
| F04–F06 | wait current/next sequence, timeout/cancellation |

### 12.5 Capture tests

| ID range | Coverage |
|---|---|
| C01–C05 | relative path, spaces, absolute/traversal, extension, existing file |
| C06–C09 | static invalidation, newer sequence, one capture, stop cancellation |
| C10–C13 | RGB565 known colors, header, byte count, atomic rename |
| C14–C16 | open/write/close failure and temporary cleanup |
| C17–C18 | 20 identical hashes and deliberate one-pixel difference |

### 12.6 State and lifecycle tests

| ID range | Coverage |
|---|---|
| V01–V04 | switch 2/3-position validation and absent switch |
| V05–V08 | analog set/replace/clear/fallback |
| V09–V12 | telemetry tuple/range, real sensor visibility, copied-fixture write |
| L01–L04 | Lua unavailable/running/panic/generation |
| L05–L08 | warm restart success, first frame, epoch, stale completion |
| L09–L12 | restart rejected while wait/capture/mailbox is active and input cleanup |
| L13–L16 | cold restart isolation, child reap, no temp artifacts, failure |

### 12.7 Scenario and developer-UX tests

| ID range | Coverage |
|---|---|
| Q01–Q03 | strict schema, unknown fields, pre-launch range validation |
| Q04–Q06 | required capabilities, immutable fixture copy, unique run directory |
| Q07–Q08 | manifest/protocol/artifact hashes and failed-step diagnostics |
| Q09–Q10 | documented one-command smoke and nonzero failure exit |

## 13. Requirement traceability

| Requirement | Source | Design decision | Phase | Proof |
|---|---|---|---|---|
| Generic reusable harness | #7337 | D01 | 0–3 | B01, H01 |
| Windows control | #7646 requirement | D03, native Win32 adapter | 2–3 | T05–T08, H01–H14 |
| Partial-line safety | Both PRs/review | Line buffer | 1–2 | P13–P18, T01–T04 |
| Correlated errors | #7337 improved | D05–D06 | 1 | P05–P18 |
| Non-blocking durations | #7337 review | D07 | 4 | I15–I18 |
| Key/rotary/touch | #7337 + #7646 | Core commands | 4 | I01–I18 |
| State injection | #7646 | Firmware ownership/atomics | 6 | V01–V12 |
| Lua reload | #7646 | Generation completion | 6 | L01–L04 |
| Warm restart | #7646 | D12 | 6 | L05–L12 |
| Cold deterministic reset | #7337 host model | Host process restart | 6 | L13–L16 |
| Fresh static capture | #7646 | D08–D10 | 5 | C06–C18 |
| Safe paths with spaces | #7337 review | Output root containment | 5 | C01–C05 |
| Process cleanup | #7337 review | Host lifecycle contract | 3 | H05–H12 |
| Small native dependency surface | Both | D10, D17 | 5, 8 | scope audit |
| Attribution | Collaboration requirement | provenance policy | 0, PR strategy | reviewed commit history |

## 14. Final acceptance criteria

The implementation may leave draft only when all of the following are true:

- one protocol and one runtime activation path exist;
- maintainers have reviewed or accepted the consolidation direction;
- every admitted request has one correlated terminal response;
- request IDs, bounds, UTF-8, paths, and JSON escaping are tested;
- Linux and Windows use the same grammar and host API;
- neither native nor Python input handling depends on pipe-incompatible
  `select` behavior;
- no automation command uses `SDL_Delay`;
- no partial input blocks the SDL loop;
- idle polling and supported-client stress remain inside the section 6.8
  performance budgets;
- stdout contains protocol only and stderr is drained separately;
- key/touch state is released on every cleanup path;
- analog overrides never leak across release/restart;
- telemetry and Lua operations run in firmware context;
- Lua reload reports actual stable completion;
- warm restart reports a new epoch and first new LCD frame;
- cold restart uses a new process and fixture copy;
- static-screen capture is tied to a newer LCD notification;
- `simuLcdNotify` performs no filesystem work;
- 20 repeated checkpoint captures are byte-identical;
- paths containing spaces pass on Linux and Windows;
- fixture templates remain unmodified after tests;
- stop, crash, timeout, terminate, and kill all reap the child;
- the scenario runner exits nonzero on failure;
- normal simulator behavior and help remain valid without automation flags;
- no physical firmware or WASM artifact contains the feature;
- no unrelated build option or dependency is added;
- retained work from both PRs is visibly credited; and
- the final document records any approved deviation from this contract.

## 15. Pull-request, commit, and attribution strategy

### 15.1 Pull-request structure

1. Keep this documentation pull request focused on scope and contract.
2. After direction is accepted, branch the replacement implementation from a
   fresh `upstream/main`.
3. Keep the implementation draft until Phase 8.
4. Use the phase order as the default commit order.
5. If reviewers request smaller changes, split at phase boundaries into stacked
   pull requests while retaining one protocol and one tracking issue/document.
6. Do not merge a temporary second transport or compatibility API merely to
   reduce rebasing work.

### 15.2 Recommended commit sequence

1. `simu: add bounded automation protocol core and tests`
2. `simu: add cross-platform stdio transport`
3. `tools: add cross-platform simulator session client`
4. `simu: add input primitives and display sequencing`
5. `simu: add render-complete framebuffer capture`
6. `simu: add state injection and lifecycle operations`
7. `tools: add strict TX16S smoke scenario and fixtures`
8. `docs: finalize simulator automation contract`

Each commit must build and its tests must pass. Fixups should be folded before
final review unless preserving an author's imported commit is more important.

### 15.3 Attribution rules

- Thank `onliner10` prominently in the replacement description.
- Link both source pull requests in the document and implementation PR.
- Preserve original Git authorship when importing a coherent block that can
  remain a reviewable commit.
- If substantial code is rewritten, use `Co-authored-by` only with the
  contributor's approval.
- If only an idea or requirement is retained, credit it in the provenance table
  and PR description rather than manufacturing code authorship.
- Do not close, force-push, or otherwise modify another contributor's branch.
- #7646 may be closed by its author only after replacement parity is clear.
- The author of #7337 decides whether that PR is closed, rebased, or retained.

## 16. Risks, controls, and decision gates

| Risk | Control | Decision gate |
|---|---|---|
| A third PR increases confusion | Draft, cross-links, one decision table | Do not implement until direction is visible |
| Contribution appears appropriated | Provenance table and author invitation | Review attribution before importing code |
| Protocol scope becomes a testing platform | Version 1 command list and non-goals | Defer adapters/goldens/multiple targets |
| Windows works natively but Python fails | Binary reader threads, no pipe `select` | Windows lifecycle test in Phase 3 |
| SDL loop stalls on input | Raw bounded reads and pump budget | Frame-time baseline and stress |
| SDL loop stalls on output | Cooperative-reader invariant and slow-reader stress | Add response writer queue if latency budget fails |
| Capture is stale | Newer `display_seq` and forced invalidation | C06–C09 |
| LCD callback blocks firmware | Snapshot only; writer elsewhere | Static analysis and failure-injection test |
| Capture writes outside run directory | Canonical relative containment | C01–C05 |
| Analog read becomes expensive | Packed atomic override | Measure ADC path; no mutex |
| Telemetry races model state | Firmware mailbox | Thread/context review before Phase 6 |
| Lua reply means only “requested” | Generation and stable-state completion | L01–L04 |
| Warm restart is mistaken for clean state | Explicit epoch and cold host action | Documentation and isolation tests |
| Fixture is modified in Git | Copy-on-run policy | Dirty-worktree assertion after smoke |
| Async operation replies twice | Single owner state machine | S01–S12 |
| Late completion crosses restart | Epoch-tagged mailbox/completion | L05–L12 |
| Firmware size changes | Native-only build boundaries | Representative size comparison |
| Large patch becomes unreviewable | Phase commits or stacked PRs | Split only at architecture boundaries |

## 17. Rollback and recovery

Automation is opt-in and has no persisted protocol state. If the implementation
must be removed:

- remove the runtime flags and simulator-only sources;
- remove the two small firmware/UI simulator hooks;
- remove the host tool and fixtures;
- retain no model-format, Lua-API, firmware, Companion, or WASM compatibility
  obligation; and
- keep generated artifacts under ignored build directories.

During development, a phase that fails its exit criteria is reverted or revised
before the next phase. It is not hidden behind another build option.

## 18. Deferred follow-up work

Each item requires a separate proposal after protocol v1 is accepted:

- MCP adapter over the Python client;
- JUnit/CTest integration;
- golden-image review and approval workflow;
- richer image diffing and masks;
- monochrome and additional color target capture;
- concurrent or pipelined clients;
- a bounded asynchronous response writer if stress data requires it;
- alternate transport for environments without redirected stdio;
- richer telemetry scenario vocabulary;
- hardware-in-the-loop commands aligned with the same scenario model; and
- performance/memory probes keyed by epoch and `display_seq`.

Follow-ups must consume the accepted protocol or extend it compatibly. They must
not introduce a second simulator control surface.
