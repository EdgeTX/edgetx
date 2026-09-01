# Unified Simulator UI Automation Plan

**Status:** Proposed implementation contract for collaborative review

**Phase 0 evidence:** [baseline and gate record](simulator-ui-automation-phase-0.md)

**Current state (2026-08-31):** Phases 1–8 are implemented and the local Phase 8
hardening matrix passes. Contract review G0 is still open, so this document
remains a proposed contract and the pull request remains draft. G0 is required
before the pull request leaves draft or is merged, not before implementation and
verification commits.

**Working rule:** keep one branch and one draft pull request, implement the
smallest useful vertical pieces, preserve the best requirements from both source
pull requests, and invite both authors to review before merge.

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

The intended result is one initiative, one protocol, and one implementation
draft with reviewable phase commits. Split it only if maintainers explicitly
request a smaller review surface; never create parallel control APIs.

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
| `simulcd.cpp::simuLcdBuf` | Holds the assembled simulator framebuffer at LCD notification time | Copy this as the capture source after a newer notification |
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

### 2.5 Phase 0 provenance ledger

This ledger records expected ownership before implementation. It distinguishes
code reuse from requirement reuse so that credit is explicit without assigning
authorship for code that has not been imported.

| Retained component | Source | Reuse class | Expected implementation ownership and credit status |
|---|---|---|---|
| Opt-in stdio direction, key mapping, framebuffer capture, and simulator argument-parser starting point | #7337: `arg_parser.*`, `sdl_simu.cpp` | Algorithm and implementation reference; code may be imported only in a coherent commit | Preserve Mateusz Urban's authorship if a coherent block is imported. If substantially rewritten, the consolidation author owns the new commit and requests approval before adding a `Co-authored-by` trailer. Approval is currently pending. |
| Cross-platform host session, CLI shape, declarative flow, fixture layout, and host-side PPM conversion | #7337: `tools/ui-harness/edgetx_ui/*`, launchers, fixtures, and flow | Design and test ideas retained; the Phase 3 session is a substantial rewrite and imports no coherent source block. MCP and root dependency files remain excluded | Credit #7337 and Mateusz Urban in the commit and PR. The rewritten commit remains owned by the consolidation author; any co-authorship trailer still requires contributor approval. |
| Explicit key/touch transitions, Windows-native transport requirement, capture-after-refresh requirement, Lua reload, and warm restart | #7646: `arg_parser.*`, `sdl_simu.cpp`, `simulib.*` | Requirements and selected implementation reference | Implemented by the consolidation author unless a source commit is retained. The append-file transport and direct state mutation are not reused. |
| Static LVGL invalidation and capture coordination | #7646: `LvglWrapper.cpp`, `simu_capture.cpp` | Algorithm retained; implementation redesigned | Rewritten by the consolidation author to use a display sequence, preallocated snapshot, and worker-owned file I/O. |
| Switch, analog, and telemetry injection | #7646: `api_simu.*`, `simulib.*` | Requirements retained; original Lua surface dropped | Rewritten by the consolidation author behind the common protocol and firmware mailbox. No Lua `simu` table code is imported. |

The ledger is updated if review changes what is imported. A PR link is always
required even when reuse is requirement-only.

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
- defend an automation output root that another process concurrently replaces
  or rewires through symlinks/reparse points;
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

## 4. Proposed design decisions

Status is explicit and review-driven. `Proposed` does not become `accepted` by
age, passing CI, or absence of comments. Update each row only when the relevant
contributor and maintainer response is linked in the Phase 0 record.

| ID | Status | Decision | Rationale |
|---|---|---|---|
| D01 | Proposed | Use one generic `--automation-stdio` runtime mode | Keeps #7337's reusable scope and leaves normal runs unchanged |
| D02 | Proposed | Require `--automation-output <directory>` with automation mode | Constrains writes to an explicit artifact root |
| D03 | Proposed | Use UTF-8 newline-delimited text requests and JSON-line responses | Small native parser; robust host parsing |
| D04 | Proposed | Requests are not JSON | Avoids adding a native JSON parser dependency |
| D05 | Proposed | Request IDs are strictly increasing unsigned 64-bit values per process | Correlation without an unbounded duplicate-ID set |
| D06 | Proposed | Protocol stdout contains JSON records only; diagnostics stay on stderr | Prevents logs from being mistaken for replies |
| D07 | Proposed | Timed press/touch/wait actions are composed by the host | Keeps sleeps out of the SDL command path |
| D08 | Proposed | `display_seq` increments only in `simuLcdNotify` | Distinguishes firmware LCD refresh from SDL/ImGui redraw |
| D09 | Proposed | Capture means the first full LCD refresh after arming | Gives a precise freshness boundary |
| D10 | Proposed | Capture output is PPM; PNG conversion remains in Python | Avoids another native image dependency |
| D11 | Proposed | Only one asynchronous operation is active in protocol v1 | Makes cancellation, restart, and response ownership unambiguous |
| D12 | Proposed | `restart` means warm `simuStop`/`simuStart`; the CLI exposes cold process restart separately | Avoids claiming a stronger reset than EdgeTX provides |
| D13 | Proposed | Lua and telemetry work crosses a bounded firmware mailbox | Prevents SDL-thread mutation of firmware-owned state |
| D14 | Proposed | Fixtures are immutable templates copied into a unique run directory | Prevents tracked-file mutation and cross-run contamination |
| D15 | Proposed | The reference Python client serializes commands | Simplifies v1 while preserving IDs for diagnostics and later adapters |
| D16 | Proposed | The simulator assumes a cooperative local peer that drains stdout | Keeps v1 transport small; output backpressure is stress-tested |
| D17 | Proposed | New behavior is native-simulator-only and runtime-dormant | No physical firmware feature or size impact |
| D18 | Proposed | Unknown scenario fields and unsupported capabilities are errors | Prevents typo-driven false-positive tests |
| D19 | Proposed | One dedicated capture worker writes PPM artifacts | Keeps both the LCD callback and SDL loop free of capture file I/O |

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
are compiled under the single `SIMU_AUTOMATION` native-simulator CMake boundary,
not by a new user-visible product option.

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

Performance budgets use paired runs from the same implementation commit,
machine, toolchain, build type, target, fixture, window state, and power mode.
Phase 0 records only the automation-disabled `upstream/main` baseline because
the enabled mode does not exist yet. Phase 2 records both disabled and enabled-
but-idle runs before its exit gate.

The measurement protocol is fixed as follows:

- use `SDL_GetPerformanceCounter` and `SDL_GetPerformanceFrequency`;
- report `loop_work`, measured before deliberate frame pacing, separately from
  start-to-start `loop_period`;
- discard a five-second warm-up, then sample for 30 seconds and at least 1,000
  completed iterations;
- run three independent processes and report every run plus the median p50,
  p95, p99, and maximum;
- record CPU, logical-core count, OS, compiler, SDL version, build type, target,
  display configuration, and whether the window was visible or virtual; and
- time dependency acquisition, CMake configuration, clean compilation, and
  incremental compilation separately instead of reporting one mixed duration.

After Phase 2 adds the runtime mode:

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
normal flows. That run root is trusted session-owned state: no other process may
modify, replace, or reparse it while the simulator is running. Path containment
rejects accidental escape and unsafe client input; it is not a sandbox boundary
against a concurrent process running as the same OS user.

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
7. send `describe` and validate its schema, target, LCD, commands, and required
   capabilities; and
8. send `status` until `Ready` with a non-zero epoch and first-frame
   sequence, or timeout.

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
- Hook call sites use `#if defined(SIMU_AUTOMATION)`, preventing an unresolved
  bridge or behavior change in firmware, WASI, and Emscripten builds.
- Nothing is added to physical firmware targets.
- Nothing is added to `wasi-module`.
- No new FetchContent dependency is introduced.
- Existing native tests remain enabled by default.
- Building `--target simu` remains the harness build path.

## 11. Detailed implementation phases

Each phase is independently reviewable. A phase does not start by hiding failed
exit criteria from the previous phase.

### Phase 0 — Agreement, baseline, and provenance

#### 0.1 Freeze source and contract status

- Pin the exact `upstream/main` commit and fetch timestamp.
- Record all submodule commits and verify a clean checkout.
- Pin toolchain versions and the Linux container by immutable digest.
- Record hashes for downloaded archives even when an existing upstream
  `FetchContent` declaration does not specify `URL_HASH`.
- Give every D01–D19 row an explicit `proposed`, `accepted`, or `objected`
  review status; silence is not acceptance.

- Review D01–D19 with `onliner10` and simulator maintainers.
- Confirm `--automation-stdio` and `--automation-output` naming.
- Confirm stdout/stderr separation.
- Confirm warm versus cold restart terminology.
- Confirm that MCP and the Lua `simu` table remain deferred.

#### 0.2 Confirm provenance

Maintain the ledger in section 2.5 and record, per retained component:

- source PR and original file;
- whether code, algorithm, test idea, or requirement is reused;
- expected author of the implementation commit; and
- whether a `Co-authored-by` trailer has contributor approval.

#### 0.3 Establish the supported baseline matrix

Linux is the authoritative correctness and isolation environment. Use a native
clone inside the official EdgeTX container, checked out at the pinned commit;
do not bind-mount a Windows linked worktree whose `.git` file contains host
paths. Record:

- clean native radio tests;
- the TX16S simulator configuration and clean build;
- representative TX16S firmware and WASM artifact sizes where the official
  image exposes those targets; and
- hashes for produced baseline artifacts.

Windows is the authoritative native simulator environment. Use the versions
documented by EdgeTX, including the official SDL VC development archive and its
CMake package files. Record:

- TX16S simulator configuration and clean build;
- normal command-line help;
- ordinary launch and clean shutdown; and
- toolchain and artifact hashes.

The existing radio-test executable is not a Phase 0 Windows requirement while
it depends on POSIX headers. New pure protocol tests still become cross-platform
before Phase 8.

#### 0.4 Measure only observable Phase 0 baselines

- Run the timing protocol in section 6.8 with automation disabled.
- If `upstream/main` does not emit the existing SDL-loop timing samples, use an
  uncommitted measurement-only probe, publish its complete diff and hash, keep
  measurement buffers preallocated, and restore a byte-identical checkout
  before recording product artifacts.
- Run three clean simulator builds after dependency acquisition and report the
  median; also record one no-change incremental build.
- Record dependency acquisition separately from configure and compile time.
- Record the physical firmware and WASM sizes for later comparison.
- Do not report enabled-but-idle overhead until Phase 2 implements the mode.
- Do not claim binary isolation on pre-feature `upstream/main`; perform the
  meaningful post-implementation symbol audit in Phase 8.

#### 0.5 Apply two distinct exit gates

**Technical exit 0T:** source and tools are pinned, supported Linux and Windows
commands are reproducible, required baseline checks pass, measurements include
their method and limitations, and the provenance ledger is complete.

**Contract gate G0:** no unresolved architectural objection remains to one
protocol, and contributor/maintainer responses are recorded. G0 may remain
pending after 0T; it must not be described as accepted by absence of comments.

**Deliverables:** decision-status table, provenance ledger, environment and
command manifest, baseline log, and artifact hashes.

The dated Phase 0 baseline and gate record is maintained in
[`simulator-ui-automation-phase-0.md`](simulator-ui-automation-phase-0.md).

**Exit:** 0T passes before Phase 1 implementation begins. G0 passes before this
pull request leaves draft or is merged. Original PRs remain open until the
replacement demonstrates the retained behavior and their authors have had a
reasonable opportunity to review it.

### Phase 1 — Pure protocol, bounds, and state model

**Status (2026-08-17):** implemented in this draft. The pure protocol and state
tests pass with the maintained Linux `gtests-radio` target (122/122 total tests),
and `simu` builds without adding runtime activation or a new dependency.

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

**Status (2026-08-17):** implemented in this draft. The full native simulator
builds on Linux; the transport also compiles with Windows Clang and MinGW; and
the maintained Linux ASan suite passes 129/129. Process probes cover malformed
input, `ping`, provisional unsupported commands, flushed `stop`, EOF, broken
peers, 63/64/65 queue boundaries, and the eight-record pump budget. The same
four-record protocol vector is byte-identical on both platforms (SHA-256
`15276a945f8875ac6854fd22b08c5bfef2aef96df280e63a9b968d7158adab79`).
Only `ping` and `stop` execute in this phase; recognized later-phase commands
return `unsupported_command`.

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

#### 2.5 Use a temporary protocol probe

Before the full host exists, use a disposable validation probe that:

- launches the simulator;
- sends `ping` and malformed records;
- verifies IDs and EOF behavior; and
- runs on both platforms without Python `select`.

Do not retain the probe as a second API. Phase 3 replaces it with the reusable
cross-platform session client.

**Tests:** T01–T16, B01–B04.

**Exit:** identical byte vectors and response JSON pass on Linux and Windows;
idle automation adds no blocking read to the SDL loop.

### Phase 3 — Cross-platform Python session foundation

**Status (2026-08-17): implementation complete and validated locally.**
Slice 3A implements the dependency-free binary session and portable process
lifecycle. Slice 3B connects the existing LCD notification to the guarded
session state, adds bounded `status` and `describe` results, and makes startup
wait for a real first frame. Discovery validates the protocol schema, target,
LCD dimensions, implemented command set, and caller-required capabilities.
Capability and input-name lists remain deliberately empty until their matching
commands are usable.

The 29 focused tests pass with warnings treated as errors on Windows Python
3.11 and the official Linux image's Python 3.10. They include 100 process
lifecycle cycles per host and an explicit forced `kill`-then-`wait` fallback.
The maintained native TX16S suite passes 131/131. The real Linux TX16S
simulator passes 100 readiness/stop cycles with no orphan process, and the real
Windows TX16S simulator completes the same handshake once with paths containing
spaces and exit code zero. The Windows full build used the installed Clang 22
with SDL's documented prefetch guard; the protocol and Win32 transport compile
without that compatibility flag.

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

**Implementation status:** implemented and locally verified on the single
consolidation branch. Maintainer review remains pending; current-head GitHub
Actions checks passed at the final-audit checkpoint.
Phase 4 deliberately excludes capture, switches, analog inputs, telemetry, Lua,
restart, and scenario files. Capture is now implemented by Phase 5; the other
features remain in Phases 6–7.

#### 4.0 Research record and resulting constraints

The Phase 4 design was checked against the following primary sources on
2026-08-17. This is an implementation record, not a request to copy either
source pull request wholesale.

| # | Primary source investigated | Phase 4 conclusion |
|---:|---|---|
| 1 | [EdgeTX PR #7337](https://github.com/EdgeTX/edgetx/pull/7337) | Retain canonical host actions and host-side duration control; reject its simulator-side `SDL_Delay` command execution because it stalls the SDL loop. |
| 2 | [EdgeTX PR #7646](https://github.com/EdgeTX/edgetx/pull/7646) | Retain direct use of simulator helpers and explicit touch transitions; replace numeric public key IDs and append-file transport with the consolidated protocol. |
| 3 | [Current EdgeTX `simulib.cpp`](https://github.com/EdgeTX/edgetx/blob/main/radio/src/targets/simu/simulib.cpp) | Execute through `simuSetKey`, `simuRotaryEncoderEvent`, `simuTouchDown`, and `simuTouchUp`; do not add a parallel input state. |
| 4 | [Current EdgeTX `sdl_simu.cpp`](https://github.com/EdgeTX/edgetx/blob/main/radio/src/targets/simu/sdl_simu.cpp) | Filter keys with the target support mask and preserve framebuffer-pixel touch coordinates. |
| 5 | [SDL `SDL_PushEvent`](https://wiki.libsdl.org/SDL2/SDL_PushEvent) | Do not synthesize keyboard or pointer events: pushed device events do not update SDL device state. |
| 6 | [SDL `SDL_Event`](https://wiki.libsdl.org/SDL2/SDL_Event) | If a future command must create an SDL event, zero-initialize the entire union before setting fields. Phase 4 needs no synthetic SDL input event. |
| 7 | [SDL `SDL_PollEvent`](https://wiki.libsdl.org/SDL2/SDL_PollEvent) | Keep protocol execution non-blocking and on the existing main/video loop. |
| 8 | [SDL `SDL_PeepEvents`](https://wiki.libsdl.org/SDL2/SDL_PeepEvents) | Do not add a second SDL queue-draining path for automation. |
| 9 | [SDL `SDL_KeyboardEvent`](https://wiki.libsdl.org/SDL2/SDL_KeyboardEvent) | Avoid an incomplete fake keyboard transition and call the native EdgeTX key helper directly. |
| 10 | [SDL `SDL_MouseButtonEvent`](https://wiki.libsdl.org/SDL2/SDL_MouseButtonEvent) | Public touch coordinates remain LCD framebuffer coordinates, independent of window scaling and mouse button layout. |
| 11 | [SDL `SDL_MouseMotionEvent`](https://wiki.libsdl.org/SDL2/SDL_MouseMotionEvent) | Represent drag as ordered touch state updates, not relative host-window mouse motion. |
| 12 | [SDL `SDL_TouchFingerEvent`](https://wiki.libsdl.org/SDL2/SDL_TouchFingerEvent) | Do not expose SDL's normalized touch coordinate convention; the EdgeTX helper consumes integer LCD pixels. |
| 13 | [LVGL input-device overview](https://docs.lvgl.io/8/overview/indev.html) | Persist pointer pressed/released state and the latest coordinate across reads; rotary input is an immediate signed difference. |
| 14 | [LVGL input-device porting guide](https://docs.lvgl.io/8.0/porting/indev.html) | Preserve strict pointer `down -> move* -> up` ordering and explicit released state. |
| 15 | [Python `time`](https://docs.python.org/3/library/time.html#time.monotonic) | Measure composite deadlines with `time.monotonic()`; allow `sleep()` overshoot instead of pretending durations are exact. |
| 16 | [Python `subprocess`](https://docs.python.org/3/library/subprocess.html#subprocess.Popen) | Keep the existing binary-pipe, `shell=False`, owned-process lifecycle for all new actions. |

The combined design therefore keeps the best part of each proposal: #7337's
portable host composition and #7646's direct simulator primitives. It removes
four avoidable liabilities: native duration sleeps, fake SDL input events,
target-specific numeric IDs in the public contract, and a second transport or
branch.

#### 4.1 Freeze the Phase 4 wire contract

All primitive requests use the existing `v1 <id> <command> ...` record and
produce exactly one correlated terminal JSON response. Durations never cross
the wire.

| Command | Arguments and bounds | Native completion | Success result |
|---|---|---|---|
| `key-down` | one canonical name advertised by `describe.keys` | `simuSetKey(key, true)` has run | none |
| `key-up` | one currently-down canonical name | `simuSetKey(key, false)` has run | none |
| `rotate` | signed non-zero integer in `[-128, 128]` | `simuRotaryEncoderEvent(steps)` has run | none |
| `touch-down` | `x y`, with `0 <= x < lcd.width` and `0 <= y < lcd.height` | `simuTouchDown(x, y)` has run | none |
| `touch-move` | bounded `x y` while touch is down | the latest position was passed through `simuTouchDown(x, y)` | none |
| `touch-up` | no arguments while touch is down | `simuTouchUp()` has run | none |
| `wait-frame` | unsigned minimum display sequence | the real LCD sequence is at least the minimum | `{"display_seq": N}` |
| `release-all` | no arguments; idempotent | every owned key and touch is released | none |

Failure mapping is stable and machine-readable:

- a key not listed by the target returns `unsupported_target`;
- duplicate key-down/unmatched key-up return `key_already_down` and
  `key_not_down`;
- duplicate touch-down or move/up without a down return
  `touch_already_down` and `touch_not_down`;
- invalid coordinates or rotary magnitude return `out_of_range`;
- a second asynchronous operation or input during one returns
  `operation_busy`;
- a command absent from discovery returns `unsupported_command`;
- stop cancellation of a pending barrier returns `session_stopping` for the
  barrier before the successful `stop` response.

#### 4.2 Subphase A — target discovery and native executor hooks

1. Define one canonical key table containing `MENU`, `EXIT`, `ENTER`,
   `PAGEUP`, `PAGEDN`, `UP`, `DOWN`, `LEFT`, `RIGHT`, `PLUS`, `MINUS`,
   `MODEL`, `TELE`, `SYS`, `SHIFT`, and `BIND`.
2. Build `describe.keys` by applying `keyIsSupported` to that table; never
   advertise an enum value merely because it compiles.
3. Advertise `key-down`, `key-up`, and `release-all` only when at least one
   supported key is executable; advertise rotary/touch commands only under
   their existing target feature definitions.
4. Inject small native callbacks into `AutomationStdio`. The transport owns
   validation and state; `sdl_simu.cpp` owns the name-to-target-key mapping.
5. Set `capabilities.rotary` and `capabilities.touch` only when their commands
   and callbacks are both usable. Do not change unrelated capability flags.

#### 4.3 Subphase B — key, rotary, and touch state machines

Key ownership is a set of canonical names. Touch ownership is one boolean plus
the latest framebuffer coordinate. State changes are committed under the
session mutex before invoking the short, non-failing native helper on the SDL
thread. No command calls `SDL_Delay`, waits on firmware, or creates an SDL
event.

`release-all` first snapshots owned inputs, clears protocol ownership, and then
releases the native keys/touch exactly once. It is legal while a frame barrier
is pending so failure cleanup cannot be blocked by the operation it is cleaning
up. `stop` runs the same release path.

#### 4.4 Subphase C — real LCD sequence and asynchronous barrier

`simuLcdNotify` is the only sequence producer. Under the existing session
mutex, it performs a saturating increment and tests the one optional pending
barrier. When the minimum is reached it claims the request once and publishes
its POD fields into a one-slot completion mailbox.

The LCD callback never serializes JSON, writes stdout, touches SDL, performs
file I/O, sleeps, or waits for the host. `AutomationStdio::pump` drains the
completion queue on the SDL loop and owns protocol output. This preserves
thread ownership and prevents stdout backpressure from blocking firmware/UI
notification code.

Barrier rules:

1. `minimum <= current display_seq` completes immediately with the current
   sequence.
2. A larger minimum arms `AsyncOperation::WaitFrame` with request ID and epoch;
   no provisional response is emitted.
3. Only a notification in the same epoch may complete it.
4. Stop atomically cancels and clears it, queues one `session_stopping` terminal
   response, releases inputs, then acknowledges stop.
5. Sequence wrap is forbidden: at `UINT64_MAX` the published value saturates.

#### 4.5 Subphase D — validated host primitives and composites

Add dependency-free methods to `SimulatorSession`:

- primitives: `key_down`, `key_up`, `rotate`, `touch_down`, `touch_move`,
  `touch_up`, `release_all`, `wait_frame`, and a fresh `read_status`;
- composites: `press`, `long_press`, `tap`, `drag`, and `wait_next_frame`.

Every primitive validates discovery and bounds locally before consuming a
request ID. `press`, `long_press`, and `tap` put their matching release in a
`finally` path. `drag` validates the full point list before touch-down, uses a
monotonic schedule across the requested total duration, emits ordered moves,
and always attempts touch-up. If the matching release fails, the host attempts
idempotent `release-all` while preserving the primary exception.

`wait_next_frame` obtains a fresh status snapshot and requests
`display_seq + 1`; cached startup status is insufficient. Host timeouts remain
deadlines, not simulator sleeps. It does not force an LCD invalidation, so a
static screen may legitimately time out. To synchronize an input, read status
before the input and then call `wait_frame(previous_sequence + 1)`; Phase 5 is
responsible for forced capture freshness. A timed-out request poisons that
session for further commands because its eventual response cannot be safely
re-correlated; shutdown then owns process cleanup.

#### 4.6 Subphase E — verification matrix

| IDs | Required proof |
|---|---|
| I01–I04 | supported key succeeds; unknown key, duplicate down, and unmatched up return their exact errors |
| I05–I08 | rotary `-128`, `128`, zero, and one-beyond bounds are distinguished between execution and parser rejection |
| I09–I14 | touch down/move/up ordering, all four framebuffer edges, first outside coordinate, and release-all cleanup |
| I15–I18 | press/tap/drag release on success and failure; host rejects absent target capabilities before writing |
| F01–F03 | sequence starts at first LCD notification, ignores SDL redraws, and saturates without wrap |
| F04–F06 | current sequence completes immediately; next sequence completes only after notify; stop/timeout cannot create a late second terminal response |

Validation runs in increasing cost order: native protocol/state tests, Python
unit and fake-process lifecycle tests, incremental simulator build, then a real
simulator probe for discovery plus representative key/rotary/touch/barrier
commands. Normal interactive mode and WASM/firmware isolation remain unchanged.

**Exit:** all I01–I18 and F01–F06 behaviors are covered, target discovery is
truthful, key/rotary/touch scenarios contain no simulator-side duration wait,
and a host can prove it observed a real LCD refresh without any write from the
LCD callback.

#### 4.7 Implementation evidence (2026-08-17)

| Layer | Result |
|---|---|
| Host protocol/session | 37 Python tests passed, including composites, strict frame result decoding, local validation, release fallback, lifecycle, and poisoned timeout cleanup |
| Native protocol/state | 22 focused `SimuAutomation*` GoogleTests passed under ASan |
| Windows target | TX16S `simu.exe` compiled and linked with Clang 22 |
| Real simulator primitives | Dynamic keys, rotary, touch, duplicate-key error, release cleanup, and graceful stop passed through real binary pipes |
| Real LCD barrier | Snapshot `1`, input, then `wait-frame 2` completed at sequence `2`; later status observed sequence `3` |
| Stop cancellation | A pipelined unreachable barrier and stop produced exactly `session_stopping` for the wait, one successful stop response, and process exit `0` |

No extra branch, transport, package dependency, synthetic SDL input event, or
simulator-side duration command was introduced.

### Phase 5 — Render-complete capture

**Implementation status (2026-08-18):** implemented and locally verified on
the consolidation branch. Focused native and Python suites, the real Windows
TX16S simulator, and current-head GitHub Actions checks pass; maintainer review
remains pending.

Phase 5 is intentionally one narrow feature: a `capture` request returns only
after a fresh firmware framebuffer has been published as a complete,
deterministic artifact. It does not add scenario parsing, baseline comparison,
switch/analog injection, telemetry, Lua control, or restart behavior.

#### 5.0 Research and option review

The implementation was rechecked against the two source PRs, the current
simulator display path, and primary specifications before choosing the design.

| Source | Relevant constraint | Phase 5 consequence |
|---|---|---|
| [EdgeTX #7337](https://github.com/EdgeTX/edgetx/pull/7337) | Demonstrates a small PPM producer and dependency-free Python PNG encoder | Keep PPM as the native interchange and adapt the host conversion, but remove immediate/live-buffer capture and unrestricted host paths |
| [EdgeTX #7646](https://github.com/EdgeTX/edgetx/pull/7646) | Demonstrates static-screen invalidation and RGB565 capture | Keep the invalidation idea, but move encoding and file I/O out of `simuLcdNotify` and do not add a native PNG dependency |
| [EdgeTX simulator LCD driver](https://github.com/EdgeTX/edgetx/blob/main/radio/src/targets/simu/simulcd.cpp) | `simuLcdNotify` follows the simulator framebuffer update | Treat that notification, not an SDL repaint, as the freshness boundary |
| [LVGL v8 drawing](https://docs.lvgl.io/8/overview/drawing.html) | LVGL renders invalid areas and calls the display flush callback after rendering | Force one invalidation for an otherwise static screen, then wait for the normal flush path |
| [LVGL v8 style invalidation](https://docs.lvgl.io/8.2/overview/style.html) | `lv_obj_invalidate(lv_scr_act())` is the supported simple-redraw request | Consume the request only from the firmware/UI-owned LVGL loop |
| [LVGL v8 display interface](https://docs.lvgl.io/8.3/porting/display.html) | Full refresh and flush completion are display-driver concepts | Capture the assembled simulator framebuffer after its final notification instead of taking an independent LVGL snapshot |
| [SDL 2 `SDL_LockTexture`](https://wiki.libsdl.org/SDL2/SDL_LockTexture) | Locked streaming-texture pixels are write-only and need not preserve old data | Reject SDL texture readback as the capture source |
| [Netpbm PPM specification](https://netpbm.sourceforge.net/doc/ppm.html) | Minimal P6 is header plus top-to-bottom RGB triples with max value 255 | Emit one canonical header and an exact `width * height * 3` raster |
| [PNG Third Edition](https://www.w3.org/TR/png-3/) | A PNG stream is signature, IHDR, IDAT, and IEND with CRCs and a zlib stream | Keep the small standard-library Python encoder and validate its chunks by decoding them in tests |
| [Python `zlib`](https://docs.python.org/3/library/zlib.html) | The standard library exposes zlib compression and CRC-32 | No Pillow or native encoder dependency is needed |
| [Python `hashlib`](https://docs.python.org/3/library/hashlib.html) | SHA-256 is guaranteed by CPython | Record reproducible PPM and PNG digests in a deterministic sidecar manifest |
| [Python `json`](https://docs.python.org/3/library/json.html) | Stable separators and sorted keys are available | Sidecar metadata can be byte-stable and contain no timestamp |
| [POSIX `open`](https://pubs.opengroup.org/onlinepubs/9799919799/functions/open.html) | `O_CREAT | O_EXCL` performs atomic create-if-absent, including against symlinks | Create the temporary artifact without truncating any existing path |
| [POSIX `link`](https://pubs.opengroup.org/onlinepubs/009695399/functions/link.html) | A new hard link is created atomically and fails with `EEXIST` | Publish the already-closed temporary file without replacing the final name |
| [POSIX `rename`](https://pubs.opengroup.org/onlinepubs/9799919799/functions/rename.html) | Rename is atomic but normally replaces an existing regular file | Do not use plain POSIX rename for protocol v1's no-overwrite contract |
| [Win32 `CreateFileW`](https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-createfilew) | `CREATE_NEW` fails when the name already exists | Match the POSIX exclusive temporary-file behavior with Unicode paths |
| [Win32 `MoveFileExW`](https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-movefileexw) | Moving without `MOVEFILE_REPLACE_EXISTING` preserves no-overwrite behavior | Publish the closed temporary file without replacing the final name |
| [Win32 file naming](https://learn.microsoft.com/en-us/windows/win32/fileio/naming-a-file) | Device names such as `CON` and `NUL`, control bytes, and reserved punctuation do not identify ordinary files | Reject reserved final names before calling `CreateFileW` |
| [C++ condition variables](https://eel.is/c++draft/thread.condition) | Predicate waits coordinate a worker without polling | Use one persistent worker created only for an active automation session |

The open questions and selected answers are:

| Question | Options considered | Decision |
|---|---|---|
| Capture source | SDL texture, LVGL snapshot, or `simuLcdBuf` | `simuLcdBuf` at the newer `simuLcdNotify`; SDL readback has the wrong contract and an LVGL snapshot duplicates the display path |
| Static screen | Return the current buffer, sleep, or invalidate | Invalidate once in LVGL context and require a strictly newer `display_seq` |
| Encoding location | Native PNG, native PPM, or raw protocol bytes | Native PPM on a worker; Python converts to PNG with the standard library |
| Artifact publication | Direct write, replacing rename, or exclusive publish | Exclusive temporary create, full write/close/size check, then atomic no-replace publish |
| Worker model | Work in LCD callback, one thread per capture, or one session worker | One session worker; callback work is bounded to sequence accounting and one framebuffer copy |
| Additional LCD depths | Convert all current depths or prove one reference target first | Protocol discovery remains generic; v1 capture is enabled only for tested RGB565 (`LCD_DEPTH == 16`) targets |
| Metadata scope | Full run manifest now or capture-local metadata | Add deterministic capture-local metadata now; the run-wide log/manifest remains Phase 7 |

#### 5.1 Safe artifact-path contract

The simulator receives the canonical `--automation-output` root at startup.
Before reserving the asynchronous slot, `capture` must:

1. enforce the 1024-byte UTF-8 protocol limit;
2. reject empty, absolute, rooted, NUL-containing, `.` and `..` paths;
3. require the exact lowercase `.ppm` extension;
4. require an existing directory as the parent;
5. canonicalize that parent and prove component-wise containment below the
   configured output root;
6. reject an existing final entry, including a symlink; and
7. reject Win32 device names and reserved filename characters on Windows; and
8. preserve valid internal spaces and UTF-8 names unchanged.

The host creates parent directories. The simulator neither creates arbitrary
parents nor silently normalizes an unsafe name. Path failures are terminal and
must not arm invalidation or consume the asynchronous slot.

The configured root and its descendants are private to the active harness run
and must not be changed concurrently by another process. Descriptor-relative
containment against a hostile same-user filesystem actor is outside protocol
v1's threat model; callers needing that boundary must provide an OS sandbox or a
private filesystem namespace.

#### 5.2 Static-screen invalidation handoff

- `capture` records the current `display_seq`, arms the capture coordinator,
  then raises one atomic invalidation flag.
- `LvglWrapper::run()` consumes that flag immediately before
  `lv_timer_handler()` and calls `lv_obj_invalidate(lv_scr_act())`.
- The hook is compiled only for native `SIMU`, has no effect without an active
  request, and does not change normal simulator redraw behavior.
- Repeated calls collapse to one flag; the one-slot protocol state already
  prevents concurrent capture requests.

#### 5.3 Fresh snapshot handoff

- `simuLcdNotify` first advances the process display sequence.
- A capture may consume only a notification whose sequence is greater than
  `armed_after_seq` and whose epoch still matches.
- The notification copies exactly `LCD_W * LCD_H * sizeof(pixel_t)` bytes from
  `simuLcdBuf` into a preallocated, coordinator-owned RGB565 buffer.
- The callback performs no allocation, color conversion, filesystem call,
  JSON serialization, or protocol output.
- A condition variable wakes the writer. The SDL loop later drains exactly one
  completion and claims the matching protocol asynchronous operation.

#### 5.4 Deterministic PPM writer and publication

The worker emits the minimal Netpbm P6 form:

```text
P6
<width> <height>
255
<width * height RGB bytes>
```

RGB565 channels use bit replication, not floating-point scaling:

- red and blue: `(v << 3) | (v >> 2)`;
- green: `(v << 2) | (v >> 4)`.

Publication is a transaction:

1. exclusively create a request-specific temporary sibling;
2. write the canonical header and every converted scanline;
3. flush, close, and verify the exact file size;
4. recheck cancellation before the commit point;
5. atomically publish without replacing the final name; and
6. remove the temporary entry on every failure or cancellation path.

Open, write, close, size, and publish errors remain distinct human-readable
messages under `capture_failed`; a final-name collision remains
`artifact_exists`.

#### 5.5 Protocol and cancellation semantics

Success is emitted only after the final artifact exists:

```json
{"display_seq":186,"path":"checkpoints/home screen.ppm","width":480,"height":272,"depth":16,"bytes":391695}
```

- `capture` uses the existing one-operation slot and reports
  `async_operation: "capture"` while active.
- A second asynchronous command receives `operation_busy`.
- Stop before the writer commit point returns one terminal
  `capture_cancelled` response, removes any temporary file, and then returns
  the stop response.
- Stop after the commit point drains the capture's real terminal result before
  stopping; it never reports cancellation for an artifact already published.
- Teardown joins the worker before simulator-owned framebuffer memory can be
  destroyed.

#### 5.6 Host PNG conversion and capture metadata

The dependency-free Python client adds:

- strict decoding of the one-image, max-value-255 P6 subset;
- exact raster-length and dimension checks;
- PNG encoding with filter type 0, zlib compression, and verified chunk CRCs;
- SHA-256 for both files; and
- a deterministic `<name>.capture.json` sidecar with schema version, epoch,
  display sequence, target dimensions/depth, paths, byte counts, and hashes.

`SimulatorSession.capture_ppm()` returns the validated protocol metadata.
`SimulatorSession.capture_png()` performs native capture, local conversion,
independent PNG decode verification, hash calculation, and sidecar creation.
Neither file format contains a timestamp.

#### 5.7 Implementation subphases

1. Add capture metadata to `Response` serialization and strict Python decode.
2. Add the isolated capture coordinator, path validator, RGB conversion, and
   platform publication backend.
3. Connect output-root configuration, target capability discovery, capture
   dispatch, completion draining, cancellation, and teardown.
4. Add the single native LVGL invalidation hook and framebuffer callback copy.
5. Add Python PPM/PNG/manifest helpers and session composites.
6. Run pure protocol tests, capture unit tests, fake-process tests, sanitizer
   tests, a native target build, and a real simulator capture loop.

#### 5.8 Phase 5 verification matrix

| IDs | Required proof |
|---|---|
| C01–C05 | valid relative path with spaces; reject absolute/rooted/traversal/wrong extension; reject missing parent and existing/symlink final |
| C06–C09 | invalidation is one-shot; capture requires a newer sequence; one-slot busy behavior; stop cancellation has one terminal response and no partial artifact |
| C10–C13 | RGB565 black/white/primary known values; canonical P6 header; exact byte count; exclusive atomic publication |
| C14–C16 | injected open/write/close-or-publish failures remain distinct and clean the temporary sibling |
| C17–C18 | 20 static captures have identical PPM hashes; one deliberate visible pixel/state change changes the hash |
| H15–H18 | strict Python capture-result decode; PPM rejection cases; PNG chunk/decode verification; deterministic sidecar contents and SHA-256 |

#### 5.9 Exit evidence

Phase 5 is complete only when all of the following are recorded:

- focused native and Python tests pass;
- the representative TX16S simulator compiles and links with no native PNG
  dependency;
- a real static-screen capture returns a sequence newer than the arm point;
- 20 repeated captures of that checkpoint are byte-identical;
- one deliberate visible change produces a different PPM hash;
- paths containing spaces work through real binary pipes on Windows and in the
  portable test suite used by Linux CI;
- no `.tmp-*` artifact remains after success, injected failure, cancellation,
  or shutdown; and
- automation-disabled simulator, firmware, Companion, and WASM scopes remain
  unchanged.

Recorded local evidence (2026-08-18):

| Check | Result |
|---|---|
| Focused native suite | 34/34 protocol, state, path, writer, invalidation, freshness, cancellation, failure, and determinism tests passed under AddressSanitizer |
| Host suite | 49/49 protocol, strict PPM/PNG, metadata, fake-process, timeout, and lifecycle tests passed |
| Representative build | Windows TX16S `simu.exe` compiled and linked; capture adds no native PNG dependency |
| Real native capture | 20/20 static frames were byte-identical (`SHA-256 4649614358c3354bd16427256553c1226601096c6beb6df0696edc5e092fab13`) with strictly increasing sequences `3..22` |
| Visible change | An `ENTER` interaction produced a different framebuffer hash (`566ea1a3f202fde8e308267929451f23e4fa137fc836a4a6dd77fce7d09ce375`) |
| Host artifacts | A Unicode-and-space PNG decoded as 480×272; manifest hashes matched; no native or host temporary artifact remained |
| Build isolation | Capture sources are native-only, the LVGL hook is guarded by native `SIMU`, and Emscripten does not compile the coordinator |

**Tests:** C01–C18 and H15–H18.

**Exit:** render-complete PPM and PNG artifacts are fresh, contained,
deterministic, independently decodable, and produced without blocking the LCD
notification on filesystem work.

### Phase 6 — State injection and lifecycle

**Implementation status (2026-08-18):** implemented and locally verified on
the consolidation branch. Maintainer review and post-push CI remain before the
pull request can be considered merge-ready.

This phase deliberately follows transport and capture because it crosses three
ownership boundaries: synchronous SDL inputs, firmware-owned model/Lua work,
and process lifecycle. It does not add the Phase 7 flow language, check in a
large fixture, expose a second Lua API, or change physical firmware behavior.

#### 6.0 Research, alternatives, and final decisions

The implementation was re-audited against the two source proposals and primary
documentation before code was changed:

| Source | Finding used in Phase 6 |
|---|---|
| [PR #7337](https://github.com/EdgeTX/edgetx/pull/7337) | Preserve its generic host-session direction and process cleanup discipline; do not restore its target-specific or blocking command shortcuts |
| [PR #7646](https://github.com/EdgeTX/edgetx/pull/7646) | Keep its reset, reload, and state-injection requirements; drop the append-file transport and simulator-only Lua table |
| [SDL simulator loop](https://github.com/EdgeTX/edgetx/blob/main/radio/src/targets/simu/sdl_simu.cpp) | Switch mutation and warm-restart coordination belong on the SDL owner thread |
| [Simulator lifecycle](https://github.com/EdgeTX/edgetx/blob/main/radio/src/targets/simu/simulib.cpp) | `simuStop()` joins tasks, while `simuStart()` intentionally preserves selected static state; a warm restart is not clean isolation |
| [Native task shutdown](https://github.com/EdgeTX/edgetx/blob/main/radio/src/tasks.cpp) | Queue reset is safe only after the firmware tasks have been joined |
| [Firmware periodic path](https://github.com/EdgeTX/edgetx/blob/main/radio/src/main.cpp) | Consume bounded firmware requests immediately before UI work and publish results after the UI cycle |
| [Switch simulator driver](https://github.com/EdgeTX/edgetx/blob/main/radio/src/targets/simu/switch_driver.cpp) | Reuse `simuSetSwitch`; canonical discovery must still filter unsupported hardware/type combinations |
| [Telemetry sensor path](https://github.com/EdgeTX/edgetx/blob/main/radio/src/telemetry/telemetry_sensors.cpp) | Reuse `setTelemetryValue(PROTOCOL_TELEMETRY_LUA, ...)` in firmware context rather than duplicate sensor storage logic |
| [Lua state machine](https://github.com/EdgeTX/edgetx/blob/main/radio/src/lua/interface.cpp) | Completion must observe `INTERPRETER_RUNNING` or `INTERPRETER_PANIC`; posting the reload flag alone is not success |
| [C++ atomic ordering](https://eel.is/c%2B%2Bdraft/atomics.order) | Publish SPSC slots with release stores and consume them with acquire loads; the ADC override itself is one lock-free packed atomic |
| [Python subprocess lifecycle](https://docs.python.org/3/library/subprocess.html) | Launch with an argument vector and binary pipes, then always wait/reap after graceful stop, terminate, or kill |
| [Python `copytree`](https://docs.python.org/3/library/shutil.html#shutil.copytree) | A cold restart receives new settings and SD-card directory trees, not the previous process's writable paths |
| [Python temporary directories](https://docs.python.org/3/library/tempfile.html) | Allocate a unique run root and remove it if copying or relaunch fails |

The rejected alternatives are intentional:

| Decision | Selected | Rejected | Reason |
|---|---|---|---|
| Switch surface | Existing stdio command plus `simuSetSwitch` | New Lua `simu` table | One capability-discovered control plane is easier to test and maintain |
| ADC synchronization | Enabled bit plus `0..4096` in one atomic word | Session mutex in every ADC sample | Sampling remains allocation-free and non-blocking |
| Firmware crossing | Fixed-capacity SPSC request/completion queues | SDL calling telemetry or writing `luaState` | Model and interpreter mutations retain their current owner |
| Lua completion | Monotonic generation plus observed terminal state | Delay or immediate acknowledgement | A response proves the requested reload, not merely its scheduling |
| Automation boot | Existing `simuStart(false)` no-splash/no-calibration/no-checks mode | Declaring the splash framebuffer ready | The firmware periodic loop must own commands before startup readiness is reported; normal simulator startup stays unchanged |
| Reset semantics | Warm protocol restart and separate cold host restart | Calling both operations “reset” | Their isolation guarantees are materially different |
| Cold restart return | A newly started `SimulatorSession` | Reusing a closed process object in place | Old reader threads, IDs, diagnostics, and protocol failure state cannot leak |

#### 6.1 Synchronous switch control

1. Build discovery from canonical hardware names and supported switch types.
2. Advertise only present toggle, two-position, and three-position controls.
3. Resolve the advertised canonical name on the SDL thread.
4. Accept `-1` and `1` for toggle/two-position controls; accept `-1`, `0`, and
   `1` for three-position controls.
5. Keep the automation value authoritative over the ImGui slider so the next
   redraw cannot silently overwrite a successful command.
6. Restore the simulator's up/default position during warm restart.

Unknown names return `unsupported_target`; a valid name with an invalid
position returns `out_of_range`. No switch command crosses the firmware
mailbox.

#### 6.2 Lock-free analog overrides

Discovery enumerates canonical `ADC_INPUT_MAIN` and configured
`ADC_INPUT_FLEX` names and maps each to its flattened ADC index. Each index owns
one `std::atomic<uint32_t>`:

```text
bit 31       bits 12..0
enabled      value 0..4096
```

- `set-analog` release-stores the complete packed word and replaces any prior
  value.
- `simuGetAnalog` acquire-loads once; an enabled word wins, otherwise the
  existing ImGui/default calculation runs unchanged.
- `clear-analog <name>` and `clear-analog all` store the disabled word.
- `release-all`, warm restart, protocol stop, and teardown clear every word.
- Status counts enabled overrides from atomic snapshots; no counter can drift
  from the actual slots.

The build asserts that the 32-bit atomic representation is always lock-free on
the supported native simulator toolchains.

#### 6.3 Bounded firmware mailbox

`AutomationFirmwareMailbox` contains independent 16-entry SPSC rings for
requests and completions. The SDL thread is the request producer/completion
consumer; the firmware periodic thread is the request consumer/completion
producer.

- Slots are fixed POD records: command, request ID, epoch, reload generation,
  and validated telemetry fields.
- Producer slot writes happen-before the release publication index; consumers
  acquire that index before copying a slot.
- `simuAutomationBeforeUi()` consumes at most two operations per `perMain()`.
- `simuAutomationAfterUi()` publishes telemetry only after that UI cycle and
  publishes Lua only after a stable matching state is observed.
- Only one protocol asynchronous operation can be admitted, so capacity is a
  protection boundary rather than a throughput target.
- A completion with the wrong epoch cannot claim the active request; it is
  discarded and increments `stale_completion_count`.
- Queue reset occurs only while the firmware tasks are stopped/joined.

#### 6.4 Telemetry injection

`set-telemetry` validates before mailbox admission:

- ID `1..65535`, sub-ID `0..7`, instance `0..255`;
- signed 32-bit value;
- stored telemetry unit `UNIT_RAW..UNIT_MAX`;
- precision `0..2`; and
- optional `[A-Za-z0-9_-]{1,TELEM_LABEL_LEN}` label.

The firmware hook calls the existing `PROTOCOL_TELEMETRY_LUA` path. On a valid
sensor index it initializes the model sensor exactly as the Lua API does and
marks only the copied run model dirty. An explicit automation request
temporarily enables allocation around that call and restores the interactive
`allowNewSensors` setting afterward, so the Telemetry screen's Discover toggle
cannot make a test nondeterministic. Reinjecting an existing tuple updates its
value without allocating another slot. A genuinely full sensor table returns a
visible terminal failure. Protocol v1 deliberately has no partial “delete
sensor” operation: a cold process restart from the immutable template is the
cleanup boundary.

#### 6.5 Generation-observed Lua reload

The SDL thread assigns a monotonically increasing nonzero generation and posts
it with the request ID and epoch. The firmware hook sets the existing permanent
script reload state, then lets the normal `luaTask()` path run. A successful
terminal response is:

```json
{"generation":3,"state":"running"}
```

`panic` produces `lua_panic` with the same generation. A build without Lua does
not advertise the command and returns `lua_unavailable` if a request reaches
the mailbox defensively. Status reports `unavailable`, `not_observed`,
`reloading`, `running`, or `panic` from an atomic firmware-published snapshot.

#### 6.6 Warm restart state machine

`restart` follows one explicit path:

1. Require `Ready`, no pending frame/capture/firmware work, and empty mailbox
   request/completion queues.
2. Reserve the asynchronous slot and enter `Restarting`.
3. Release keys/touch, clear analog overrides, and reset automation switches.
4. Return control to the SDL coordinator, call `simuStop()`, and wait for its
   task joins.
5. Reset mailbox storage only after the join, call `simuStart(false)` to retain
   deterministic automation startup, and verify the runtime reports started.
6. Increment the epoch without resetting the process-monotonic
   `display_seq`.
7. Complete only on the first subsequent `simuLcdNotify()` and return its new
   epoch and sequence.

Any other asynchronous command during steps 2–7 receives `operation_busy`.
Stop/EOF during a restart still follows normal process teardown; it cannot
pretend that a first-frame barrier completed.

#### 6.7 Cold host restart

`SimulatorSession.restart_process(fixture_root, runs_root)` is the authoritative
isolation operation:

1. validate immutable `fixture_root/settings` and `fixture_root/sdcard` trees
   and the destination root before closing the healthy session;
2. stop and fully reap the old child and join both reader threads;
3. allocate one unique directory below `runs_root`;
4. copy both trees and create a new `artifacts` output root;
5. replace or append the simulator's `--settings` and `--storage` arguments;
6. construct a new session with the same timeouts and discovery expectations;
7. launch and wait for normal first-frame readiness; and
8. remove the newly allocated directory if copy or startup fails.

The method returns the new session; the old object remains closed and cannot be
requested again. A successful run directory is retained because it contains
the writable fixture and evidence, while incomplete restart directories are
removed.

#### 6.8 Implementation order and verification matrix

Implementation is intentionally incremental inside the same branch and PR:

1. tighten protocol validation and add strict Lua/restart result decoding;
2. implement switch discovery/control and packed analog overrides;
3. add and unit-test the SPSC mailbox plus the two small `perMain()` hooks;
4. connect telemetry and generation-observed Lua reload;
5. connect warm restart and preserve process-monotonic display sequence;
6. add the cold host restart API and failure cleanup; and
7. run focused native/host tests, a representative TX16S build, and real
   simulator checks.

| IDs | Required proof |
|---|---|
| V01–V04 | two-position valid/rejected-neutral, three-position all states, absent switch rejection |
| V05–V08 | analog set, replace, clear-one/all, and unoverridden fallback |
| V09–V12 | telemetry tuple/range validation, real sensor visibility, and writes confined to a copied fixture |
| L01–L04 | Lua unavailable, running, panic, and generation correlation |
| L05–L08 | warm restart first-frame success, epoch increment, monotonic display sequence, stale completion rejection |
| L09–L12 | restart rejection during frame/capture/mailbox work plus key/touch/analog cleanup |
| L13–L16 | cold restart uses a new PID and fixture, fully reaps the old child, removes failed run roots, and leaks no override/telemetry state |

Recorded local evidence (2026-08-18):

| Check | Result |
|---|---|
| Focused native suite | 38/38 `SimuAutomation*` tests passed under AddressSanitizer, covering protocol, capture/state, mailbox, analog, telemetry, and restart behavior |
| Host suite | 57/57 Python protocol/session tests passed, including validation and cold-restart cleanup failures |
| Representative build | The Windows TX16S native simulator compiled and linked; normal startup remains unchanged and automation uses deterministic startup |
| Real state injection | Discovery reported 8 switches and 13 analogs; two-/three-position switch rules, analog set/replace/clear, telemetry create/update, and observed Lua generation 1 all passed |
| Warm lifecycle | Epoch advanced from 1 to 2 and the process-monotonic display sequence advanced from 2 to 4 before completion |
| Cold lifecycle | The replacement session used a new PID and freshly copied settings/SD-card trees after the old child was reaped |
| Build isolation | Phase 6 runtime code remains behind native simulator guards; no physical firmware or WASM source path was changed |

**Tests:** V01–V12 and L01–L16.

**Exit:** the implemented Phase 6 behavior satisfies the scoped local exit
criteria. Maintainer review and repository CI remain release gates for the
consolidated pull request.

### Phase 7 — Scenario, fixture, and developer UX

**Implementation status (2026-08-31):** implemented and locally verified on
the consolidation branch. The Phase 7 head passed repository CI; the pull
request remains draft because contract review G0 is still open.

The dependency-free host implementation lives in `edgetx_ui/flow.py`. It keeps
schema validation and step interpretation separate from `SimulatorSession`,
validates the complete scenario before process launch, and records every
session request/response without changing the v1 wire protocol.

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

The checked-in TX16S template retains the three settings files contributed in
#7337, migrated to the current settings schema and protected as CRLF on every
platform because the validated settings checksum depends on those exact bytes.
An empty SD-card tree is copied beside settings for every unique run.

Recorded local evidence (2026-08-31):

| Check | Result |
|---|---|
| Scenario tests | Q01–Q10 passed: strict/unknown/range rejection, capability discovery, fixture isolation, unique roots, hashes, failure diagnostics, documented smoke, and nonzero CLI failure |
| Full host suite | 67/67 Python tests passed |
| One-command smoke | Configure/build/run completed on Windows TX16S and exited zero |
| Real flow | 11/11 actions passed, 36 protocol records were correlated, and clean stop returned process code 0 |
| Evidence bundle | Verified PPM, independently decoded PNG, capture sidecar, protocol log, bounded stderr, manifest, fixture/flow hashes, and immutable source fixture |

**Tests:** Q01–Q10.

**Exit:** one documented command builds, runs the smoke scenario, produces
verified artifacts, and exits nonzero on any failed step.

### Phase 8 — Hardening, CI, and review readiness

**Implementation status (2026-08-31):** technical implementation and the local
hardening matrix are complete. The pull request remains draft. Phase 8.6 and the
final exit remain externally blocked by contract review G0; repository CI for
the Phase 8 head must also pass after push.

The hardening runner is available as `edgetx-ui harden`. It creates an isolated
fixture copy and unique artifact directory, runs the configured lifecycle,
transport, Lua, restart, and capture loops, and writes one machine-readable JSON
report. Any failed gate produces a nonzero exit code while retaining partial
evidence and cleanup results.

#### 8.1 Run native correctness matrix

- pure parser/state unit tests on Linux and Windows;
- normal radio tests in their supported Linux environment;
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

Slow-reader testing showed that synchronous stdout writes could block the SDL
loop. The native transport now owns a dedicated output writer thread with a
bounded 128-record FIFO and a 64-record high-water mark. The SDL pump stops
admitting work at that mark, observes writer failure without blocking, and uses
cooperative flush-and-stop semantics. POSIX writes handle `EAGAIN`; Windows
teardown cancels a blocked synchronous write before joining the writer.

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
- no automation symbols or sections in post-implementation firmware/WASM
  artifacts, verified with the appropriate binary inspection tools; and
- normal simulator behavior unchanged.

#### 8.6 Update proposal to accepted contract

Replace future tense, record final deviations and rationale, and link test/CI
evidence.

This step is intentionally not complete. Absence of an objection is not
acceptance: the document remains proposed until G0 records an explicit review
outcome. No implementation result may be used to bypass that gate.

Recorded hardening evidence (2026-08-31):

| Check | Result |
|---|---|
| Host correctness | 71/71 Python tests passed on Windows and 71/71 passed in the clean Linux container; Python byte-compilation and workflow YAML parsing also passed |
| Native correctness | 173/173 radio tests passed with AddressSanitizer in the clean Linux container; exact 1,024-byte capture-path and 16-KiB request/response boundaries and their one-byte overflow cases are covered |
| Real TX16S lifecycle | 100/100 process start/stop cycles reaped every child; the full run completed with no fixture mutation or temporary artifacts |
| Real TX16S transport | 10,000/10,000 pings completed with correlated IDs; delayed stdout draining for 500 ms preserved 64 ordered responses, and the 64/65 boundary produced 64 correlated replies plus one `queue_full` event |
| Real TX16S async lifecycle | 20/20 Lua reload generations and 20/20 warm restart epochs completed; no stale completion crossed an epoch |
| Deterministic capture | 20/20 static PPM/RGB captures were byte-identical; a deliberate rotary mutation produced a different framebuffer |
| WASM isolation | TX16S WASI artifact built successfully (5,233,532 bytes) and contained zero automation markers |
| Physical firmware isolation | TX16S ARM firmware built successfully (1,612,948-byte BIN); BIN and ELF contained zero automation strings and the ELF contained zero automation symbols |
| Build isolation | `SIMU_AUTOMATION` excludes the complete automation surface from firmware, WASI, and Emscripten source builds; the host test workflow covers Ubuntu and Windows without adding a root Python dependency project |

The failed first visual-mutation attempt was retained as diagnostic evidence. A
held Enter key did not visibly change the real TX16S framebuffer, so the gate
was corrected to use a rotary mutation. The reduced verification and the full
rerun then passed. This changes only the visual hardening action, not the wire
protocol or product behavior.

**Exit:** the technical local checks are green. The replacement is not ready to
leave draft until repository CI for the Phase 8 head is green and G0 records an
explicit contract-review outcome.

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

### 12.8 Hardening-runner tests

| ID range | Coverage |
|---|---|
| R01 | 10,000 correlated requests and monotonic response IDs |
| R02 | 20 Lua reload generations and 20 warm restart epochs without stale completions |
| R03 | 20 identical static captures, deliberate visual change, fixture and artifact isolation |
| R04 | complete machine-readable report, process reaping, cleanup gates, and nonzero failure exit |

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

1. Use this branch and this draft pull request for the consolidated plan and
   implementation; do not create a second replacement branch or a branch per
   phase.
2. Keep commits small and reviewable, using the phase order as guidance rather
   than as bureaucracy.
3. Keep the pull request in draft while the contract and implementation evolve;
   require contributor/maintainer review before marking it ready or merging it.
4. Use disposable local build directories or container clones for matrix work,
   not additional remote branches.
5. Split at a phase boundary only when a maintainer explicitly requests it;
   retain one protocol and one tracking document if that happens.
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
| A third PR increases confusion | One draft, cross-links, one decision table | Do not mark ready or merge until direction is reviewed |
| Contribution appears appropriated | Provenance table and author invitation | Review attribution before importing code |
| Protocol scope becomes a testing platform | Version 1 command list and non-goals | Defer adapters/goldens/multiple targets |
| Windows works natively but Python fails | Binary reader threads, no pipe `select` | Windows lifecycle test in Phase 3 |
| SDL loop stalls on input | Raw bounded reads and pump budget | Frame-time baseline and stress |
| SDL loop stalls on output | Bounded response writer queue, high-water admission control, and slow-reader stress | T13–T16 and the Phase 8 64/65 boundary probe pass |
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
| Large patch becomes unreviewable | Phase commits in one implementation draft | Split only when a maintainer explicitly requests it |

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
