# Unified Simulator UI Automation Plan

**Status:** Proposed implementation plan for a collaborative draft pull request

**Related work:**

- [#7337 — Add TX16S UI harness](https://github.com/EdgeTX/edgetx/pull/7337),
  authored by `onliner10`
- [#7646 — Add opt-in Widget Studio automation hooks](https://github.com/EdgeTX/edgetx/pull/7646),
  authored by `bultodepapas`

This proposal combines the strongest ideas from both pull requests into one
generic, bounded, cross-platform simulator automation facility. It is not a
mechanical merge of the two patches. The intent is to converge on one protocol,
one execution path, and one small host harness while preserving authorship and
inviting both contributors to shape the result.

No existing pull request should be closed until the replacement has an agreed
contract and demonstrates the required behavior.

## 1. Executive decision

The consolidated implementation will use #7337 as the conceptual foundation:

- generic simulator UI automation rather than widget-specific tooling;
- a host-driven harness and declarative scenarios;
- explicit status and error responses;
- key, rotary, and touch input;
- framebuffer-based screenshots; and
- normal simulator behavior unless automation is explicitly requested.

It will incorporate the following requirements proven useful by #7646:

- native Windows support;
- safe handling of partial input and paths containing spaces;
- frame-boundary reset and Lua reload;
- switch, analog, and telemetry injection for widget scenarios;
- a redraw request for capture of an otherwise static screen; and
- synchronized state handoff between the host-facing simulator loop and the
  firmware/UI execution context.

The result will not preserve every implementation choice from either pull
request. In particular, the first consolidated pull request will not add a
simulator-only Lua API, a Widget Studio build option, an in-process PNG encoder,
an MCP server, or an unrelated native-test build switch.

## 2. Problem statement

EdgeTX UI changes currently rely heavily on manual simulator navigation and
host-window screenshots. Those workflows are difficult to reproduce because
they depend on local window scale, focus, timing, input method, and model/SD
state.

The two existing pull requests independently address that problem, but keeping
both would create competing contracts:

- two command transports;
- two command grammars;
- different capture completion semantics;
- different target and platform coverage;
- overlapping changes in `arg_parser.*` and `sdl_simu.cpp`; and
- two possible places for future tooling to add input and state controls.

The combined design must provide one stable boundary that can support CLI,
tests, and optional future adapters without adding a second UI framework or
allowing host threads to mutate LVGL objects directly.

## 3. Goals

The first implementation pull request will:

1. provide an opt-in native-simulator automation mode on Linux and Windows;
2. accept versioned, correlated commands and return one visible result for
   every accepted command;
3. execute UI and simulator mutations on the designated simulator/UI context;
4. support key, rotary, and complete touch transitions;
5. support bounded switch, analog, and telemetry test input;
6. provide reset and Lua reload without blocking the SDL event loop;
7. provide a render-complete capture checkpoint with framebuffer metadata;
8. provide one small standard-library-only Python harness and one deterministic
   smoke scenario; and
9. prove normal simulator behavior is unchanged when automation is disabled.

## 4. Non-goals

The first pull request will not:

- redesign any EdgeTX screen or widget;
- change firmware behavior on physical radios;
- add a public or simulator-only Lua `simu` table;
- add a Widget Studio-specific build mode;
- add a general reactive UI framework;
- add an MCP server or AI-agent-specific API;
- add native PNG encoding or a new image dependency to `simu`;
- add golden-image approval automation;
- add a general telemetry simulation framework beyond the minimum bounded
  value injection required by the pilot scenario;
- disable native tests or change their default build policy;
- change Companion or WebAssembly behavior; or
- promise support for every target in the first slice.

MCP, richer visual diffing, additional targets, and alternate transports can be
follow-up work after the simulator protocol is accepted.

## 5. Source comparison and disposition

### 5.1 Capabilities

| Capability | #7337 | #7646 | Consolidated decision |
|---|---|---|---|
| Generic UI automation intent | Yes | Widget-oriented | Keep the generic #7337 scope |
| Runtime opt-in | `--automation-stdio` | `WIDGET_STUDIO` plus `--pipe` | Keep runtime opt-in; remove product-specific build flag |
| Linux control transport | stdin/stdout | append-only file | Use non-blocking stdin/stdout |
| Windows control transport | Not implemented | append-only file works | Implement redirected stdin on Windows with the same protocol |
| Structured success/error | JSON response | none | Keep and strengthen correlated responses |
| Request correlation/version | none | none | Add protocol version and request ID |
| Status | Yes | no | Keep and extend |
| Key input | press/long press | down/up by key code | Expose down/up primitives; implement press timing in the host |
| Rotary input | Yes | no explicit command | Keep |
| Touch input | combined timed touch | down/move/up primitives | Keep explicit down/move/up primitives |
| Blocking waits | SDL delays | none | Remove from simulator; host waits or uses `wait-idle` |
| Reset | no | asynchronous request | Keep asynchronous frame-boundary reset |
| Lua reload | no | yes | Keep as an explicit command |
| Switch/analog injection | no | Lua-side helpers | Keep capability through host protocol, not Lua API |
| Telemetry injection | no | command support | Keep one bounded host command |
| Screenshot format | PPM, host converts to PNG | native PNG through stb | Keep PPM/raw framebuffer path; no new encoder dependency |
| Static-screen capture | immediate copy | invalidates then waits for flush | Keep redraw scheduling and reply only after capture completion |
| CLI harness | Yes | project-specific visual kit | Keep a reduced generic CLI |
| Declarative flows | Yes | project-specific scenes | Keep one generic scenario format |
| MCP adapter | Yes | no | Defer until the core protocol is accepted |
| Test build switch | `EDGE_TX_BUILD_TESTS` | no | Drop as unrelated |
| Simulator Lua API | no | `simu` table | Drop from the consolidated core |

### 5.2 Code-level treatment

| Existing area | Treatment | Reason |
|---|---|---|
| #7337 command names and host service model | Adapt | Useful generic starting point, but blocking durations move to the host |
| #7337 `select()` plus `std::getline()` reader | Replace | Buffered C++ input can strand commands and partial lines can block |
| #7337 JSON response helpers | Reuse concept | Structured visible errors are required; add version and request ID |
| #7337 framebuffer copy and PPM writer | Refactor and keep | Reuses `simuLcdCopy` and avoids an additional encoder dependency |
| #7337 Python CLI and flow runner | Reduce and keep | Retain build/start/run/capture essentials using the Python standard library |
| #7337 MCP layer | Remove from first slice | It is an adapter, not part of the simulator contract |
| #7337 large checked-in settings fixture | Minimize | Keep only the deterministic state required by the smoke scenario |
| #7646 append-only command-file poller | Do not carry forward | Its Windows requirement remains, but a second transport is unnecessary if stdin is implemented correctly |
| #7646 partial-line and bounded-read behavior | Reimplement in transport buffer | These are required correctness properties for both platforms |
| #7646 synchronized reset/capture state | Refactor and keep | Cross-context ownership must be explicit |
| #7646 capture-triggered LVGL invalidation | Keep as a UI-context operation | Static screens otherwise may never produce a new flush |
| #7646 native PNG/stb source | Remove | Host-side conversion is sufficient and keeps the simulator smaller |
| #7646 `api_simu.*` and Lua registration | Remove | Avoid a second control surface and simulator-only script coupling |
| #7646 analog overrides and telemetry primitives | Retain behind protocol executor | They enable real widget scenarios without a Lua-only test API |

### 5.3 Existing review feedback incorporated

The consolidated design treats the current review findings as requirements,
not as comments to revisit after implementation:

- screenshot paths must preserve spaces;
- stdin polling must not mix an OS readiness check with buffered `std::cin`;
- raw byte chunks must be assembled before UTF-8 lines are decoded;
- partial input must never block the SDL loop;
- SDL events must be fully initialized;
- key, touch, and wait durations must not use `SDL_Delay` in the simulator
  command handler; and
- the host wrapper must close pipes and reap the child after normal exit,
  timeout, terminate, and kill paths.

## 6. Proposed architecture

```text
Python CLI / test runner
          |
          | versioned request lines and JSON responses
          v
platform stdin adapter
  POSIX: non-blocking read
  Win32: PeekNamedPipe + ReadFile
          |
          v
bounded byte buffer -> bounded command queue
          |
          | consumed by SDL/main loop
          v
automation protocol parser and executor
          |
          +--> key / rotary / touch
          +--> switch / analog / telemetry
          +--> Lua reload / simulator reset
          +--> redraw request / capture checkpoint
          |
          v
structured response with request ID and frame ID
```

The transport only reads bytes and assembles complete lines. It does not mutate
simulator or UI state. The SDL/main-loop executor validates and dispatches
commands. LVGL invalidation occurs only in the existing UI execution context.

### 6.1 Components

1. **Transport adapter**
   - reads redirected stdin without blocking the SDL loop;
   - accumulates bytes rather than decoding arbitrary chunks;
   - handles LF, CRLF, multiple lines per read, partial UTF-8 bytes, EOF, and
     broken pipes;
   - never mixes `select()` with `std::cin`; and
   - pushes complete lines into a bounded queue.

2. **Protocol parser**
   - validates protocol version, request ID, command, arity, numeric ranges,
     and trailing data;
   - preserves the remainder of a capture line as the path, including spaces;
   - returns explicit parse errors; and
   - has no dependency on a new JSON parser.

3. **Command executor**
   - runs from the SDL/main loop;
   - uses primitive input state transitions rather than sleeps;
   - schedules reset, reload, redraw, and capture work at safe boundaries; and
   - emits exactly one terminal response per request.

4. **Capture coordinator**
   - associates a capture request with a request ID and minimum frame ID;
   - requests invalidation for a static screen on the UI context;
   - waits for the corresponding framebuffer flush;
   - writes a deterministic PPM or raw framebuffer artifact;
   - reports dimensions, depth, frame ID, output path, and any write failure;
   - permits only one capture in progress in the first version; and
   - returns `busy` rather than silently replacing an armed capture.

5. **Host harness**
   - configures and starts one reference simulator target;
   - sends correlated commands and enforces response timeouts;
   - implements timed presses as `key-down`, host delay, `key-up`;
   - runs declarative scenarios;
   - records command/result metadata next to artifacts; and
   - always closes stdin/stdout and reaps the child process.

## 7. Protocol contract

### 7.1 Request grammar

Requests are UTF-8, newline-terminated records:

```text
v1 <request-id> <command> [arguments...]
```

`request-id` is an unsigned 64-bit decimal value chosen by the host. Commands
and fixed tokens use ASCII. For `capture`, everything after the command's first
separator is treated as the path, so spaces are preserved.

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
v1 11 reload-lua
v1 12 wait-idle 184
v1 13 capture C:\EdgeTX Artifacts\home.ppm
v1 14 stop
```

The simulator does not parse arbitrary JSON requests because the native
simulator currently has no suitable JSON dependency. Responses remain JSON so
the host can consume them directly without fragile text matching.

### 7.2 Response schema

Success:

```json
{"version":1,"id":13,"ok":true,"result":{"frame":185,"path":"C:\\EdgeTX Artifacts\\home.ppm","width":480,"height":272,"depth":16}}
```

Failure:

```json
{"version":1,"id":13,"ok":false,"error":{"code":"capture_busy","message":"capture request 12 is still pending"}}
```

Every syntactically recoverable request receives one response. An overlong or
malformed line receives an error when its request ID can be recovered; otherwise
the simulator emits a protocol error event with `id: null`.

### 7.3 Initial command set

| Command | Arguments | Completion point |
|---|---|---|
| `ping` | none | Parsed and dispatched |
| `status` | none | State snapshot captured |
| `key-down` / `key-up` | key name | Input state updated |
| `rotate` | signed steps | Rotary event queued |
| `touch-down` / `touch-move` | x, y | Touch state updated |
| `touch-up` | none | Release state updated |
| `set-switch` | name, -1/0/1 | Override validated and stored |
| `set-analog` | name, 0..4096 | Override validated and stored |
| `set-telemetry` | id, sub-ID, instance, value, unit, precision | Value accepted by telemetry path |
| `reload-lua` | none | Reload request accepted at safe boundary |
| `reset` | none | Restart completed and new session epoch reported |
| `wait-idle` | minimum frame ID | No pending command/capture work and frame condition met |
| `capture` | output path | Requested frame written successfully |
| `stop` | none | Quit event accepted; response flushed before exit |

`press`, `long-press`, and arbitrary-duration `wait` are host helpers, not
simulator commands. This prevents `SDL_Delay` from blocking rendering and input.

### 7.4 Status result

`status` reports at least:

- protocol version;
- simulator running state;
- target identifier;
- LCD width, height, and depth;
- current frame ID;
- session epoch, incremented after reset;
- whether a capture or reset is pending;
- command-queue depth;
- transport overflow count; and
- Lua availability.

## 8. Bounds and failure behavior

Initial limits are explicit compile-time constants and covered by tests:

| Resource | Initial bound | Overflow behavior |
|---|---:|---|
| Input line | 16 KiB | Discard through newline and emit `line_too_long` |
| Buffered partial input | 16 KiB | Same as overlong line |
| Pending command queue | 64 records | Emit `queue_full`; do not block SDL |
| Commands executed per SDL iteration | 8 | Leave remainder queued for next iteration |
| Capture requests in flight | 1 | Emit `capture_busy` |
| Capture path | 4 KiB | Emit `path_too_long` |
| Response line | 16 KiB | Emit minimal `response_too_large` and count failure |

These are simulator-host limits, not firmware budgets. They still prevent an
automation client from causing unbounded memory growth or starving rendering.

Unknown commands, unsupported targets, invalid coordinates, invalid key names,
out-of-range values, unavailable Lua, output-file failures, EOF, and process
shutdown are all visible states. No command is silently ignored.

## 9. Execution and ownership rules

- The input adapter may run in the SDL thread or perform OS polling, but it may
  only append bytes and queue complete records.
- Only the SDL/main-loop executor calls simulator input functions.
- Only the established UI execution context invalidates LVGL objects.
- Reset is represented as a request consumed between simulator iterations.
- Lua reload uses the existing interpreter state transition; no host thread
  touches the Lua state.
- Capture state owns its request ID, output path, minimum frame, and terminal
  result until completion.
- Reset cancels a pending capture with an explicit `session_reset` error.
- Stop flushes its response before the SDL quit event terminates the process.
- State shared across contexts is protected by the smallest existing
  synchronization primitive practical for the native simulator.

## 10. Proposed repository layout

The implementation should remain concentrated and avoid a new framework:

```text
radio/src/targets/simu/
  automation_protocol.h
  automation_protocol.cpp
  automation_transport.h
  automation_transport.cpp
  arg_parser.cpp                 # --automation-stdio
  arg_parser.h
  sdl_simu.cpp                   # pump queue and emit responses
  simulib.cpp/.h                 # bounded state/reset/input hooks

radio/src/tests/
  simu_automation.cpp            # parser, queue, and state tests

tools/ui-harness/
  edgetx-ui                      # standard-library Python entry point
  README.md
  flows/tx16s-smoke.json
  fixtures/tx16s/                # minimum deterministic fixture only
```

If extracting transport and protocol code creates more plumbing than it
removes, they may be combined into one narrowly named pair. The testable parser
and queue must not remain as a large static block inside `sdl_simu.cpp`.

## 11. Implementation sequence

### Phase 0 — Agreement and attribution

1. Publish this plan in a draft replacement pull request.
2. Thank and invite `onliner10`, the author of #7337, to review or collaborate.
3. Ask maintainers to confirm that one consolidated replacement is preferable
   to evolving either existing pull request.
4. Record retained ideas and code provenance in the pull-request description.
5. Preserve Git authorship when code is reused; use contributor-approved
   `Co-authored-by` trailers when substantial code is rewritten.

**Exit:** contributors and maintainers agree on one protocol direction. Neither
existing pull request needs to be closed merely to complete this phase.

### Phase 1 — Protocol and queue

1. Add a pure parser/result model with no SDL or LVGL dependency.
2. Add explicit bounds and error codes.
3. Add the 64-record bounded queue and eight-command pump budget.
4. Add protocol unit tests for valid, invalid, partial, overlong, and unknown
   input.

**Exit:** parser and queue tests pass independently of the interactive
simulator.

### Phase 2 — Cross-platform transport

1. Implement POSIX non-blocking `read()` into the bounded byte buffer.
2. Implement redirected Win32 stdin using `PeekNamedPipe` and `ReadFile`.
3. Handle multiple commands, partial lines, CRLF, EOF, and broken pipes.
4. Keep response serialization common across platforms.

**Exit:** the same transport test vectors pass on Linux and Windows; polling
never blocks an SDL iteration.

### Phase 3 — Input and lifecycle commands

1. Wire key down/up and rotary events.
2. Wire touch down/move/up with target bounds.
3. Add named switch and analog overrides.
4. Add the bounded telemetry command.
5. Add asynchronous Lua reload and reset.
6. Report a new session epoch after reset.

**Exit:** functional assertions prove each state transition and reset/reload
completion without fixed simulator-side delays.

### Phase 4 — Deterministic capture

1. Introduce a monotonic frame/checkpoint ID.
2. Arm one capture with its request and minimum frame IDs.
3. Request a full-screen invalidation from the UI context when necessary.
4. Complete capture only after the corresponding framebuffer flush.
5. Write PPM from `simuLcdCopy` and return dimensions/depth/frame metadata.
6. Return explicit errors for busy state and file failures.

**Exit:** twenty repeated captures of an unchanged deterministic checkpoint
produce identical bytes and do not use sleeps as a correctness condition.

### Phase 5 — Minimal host harness

1. Add a dependency-free Python process/session wrapper.
2. Add monotonic request IDs and timeout diagnostics.
3. Implement host-side press and long-press helpers.
4. Add one minimal TX16S-class fixture and smoke flow.
5. Capture command logs and metadata next to images.
6. Guarantee process cleanup after success, failure, timeout, and forced kill.

**Exit:** one command builds/starts the simulator, executes the smoke flow,
captures checkpoints, and exits cleanly on Linux and Windows.

### Phase 6 — Review hardening

1. Run the normal native test and firmware build matrix.
2. Verify automation-disabled behavior and command-line help.
3. Measure per-iteration automation polling cost when idle and under load.
4. Confirm no firmware, Companion, or WASM source receives automation-only
   definitions or dependencies.
5. Update this document from proposal to the accepted contract.

**Exit:** all applicable CI is green and the pull request contains one coherent
simulator-testing capability.

## 12. Test matrix

### 12.1 Protocol and transport

- empty and whitespace-only lines;
- unsupported protocol version;
- missing, invalid, maximum, and repeated request IDs;
- unknown command;
- missing, extra, negative, overflowed, and non-numeric arguments;
- one command split across every byte boundary;
- several commands in one OS read;
- LF and CRLF;
- UTF-8 path split across read boundaries;
- path containing spaces;
- 16 KiB exact boundary and one byte over;
- queue at 63, 64, and 65 commands;
- stdin EOF and broken pipe;
- simulator stop with queued and pending work; and
- response serialization of quotes, backslashes, and control characters.

### 12.2 Input

- key down/up and invalid key;
- host-composed short and long press;
- positive, negative, zero, and excessive rotary steps;
- touch down/move/up;
- repeated down, move without down, and repeated up;
- coordinates at every edge and just outside the LCD;
- switch positions -1, 0, and 1;
- analog values 0, midpoint, 4096, and outside range; and
- valid and rejected telemetry metadata.

### 12.3 Lifecycle and capture

- Lua available and unavailable;
- reload during an idle screen and live widget refresh;
- reset with no pending work;
- reset with queued commands;
- reset while capture is pending;
- capture on a changing screen;
- capture on a static screen requiring invalidation;
- second capture while one is pending;
- missing/unwritable output directory;
- path containing spaces;
- twenty deterministic captures;
- deliberate one-pixel change detected by the host; and
- clean stop, timeout termination, forced kill, and child reaping.

### 12.4 Target and platform

The first required target is one TX16S-class color simulator. The pull request
must run protocol/parser tests on every native test platform available in CI and
must exercise the interactive smoke scenario on Linux and Windows where the CI
environment supports SDL execution. Additional resolutions are follow-up work
unless adding one requires no new protocol behavior.

## 13. Acceptance criteria

The consolidated pull request is ready for non-draft review only when:

- maintainers have confirmed the consolidation direction;
- each accepted command has one correlated terminal response;
- malformed and overflow input has visible bounded behavior;
- the SDL loop contains no automation `SDL_Delay` or blocking stdin read;
- Linux and Windows share one command grammar and result schema;
- UI mutation remains in the established UI context;
- reset, reload, and capture have explicit completion semantics;
- static-screen capture completes without arbitrary sleeps;
- deterministic capture passes twenty consecutive runs;
- the smoke scenario covers key, rotary, touch, state injection, reload, and
  capture;
- process resources are released on all exit paths;
- automation-disabled simulator behavior is unchanged;
- no physical-radio firmware size change is attributable to the harness;
- documentation identifies trust boundaries and unsupported targets; and
- reused work from both original pull requests is credited.

## 14. Pull-request and collaboration strategy

The replacement starts as a draft from a fresh `upstream/main` branch. Its
first commit contains this plan so that scope is reviewable before more code is
added. Implementation commits follow the phases above and remain independently
reviewable.

The draft description will:

- thank `onliner10` for #7337 and link its design and implementation;
- link #7646 and explain why a single replacement is proposed;
- invite `onliner10`, `pfeerick`, and the simulator maintainers to review the
  contract early;
- explicitly state that the new work is a collaboration, not an attempt to
  replace attribution; and
- list which capabilities were kept, redesigned, deferred, or dropped.

Once the replacement reaches behavioral parity and the direction is accepted:

- the author of #7646 can close it as superseded with a link to the replacement;
- the author of #7337 decides whether to close, rebase, or continue that pull
  request; and
- no automated or unilateral action is taken on another contributor's branch.

## 15. Risks and controls

| Risk | Control |
|---|---|
| A third PR increases confusion | Open as a clearly labeled draft replacement and cross-link both originals immediately |
| Contribution appears appropriated | Invite the original author before implementation, preserve authorship, and document provenance |
| Automation blocks the SDL loop | Non-blocking OS reads, bounded pump budget, and no simulator-side duration waits |
| Host thread mutates UI state | Queue commands and execute them only from the SDL/UI-owned path |
| Capture returns a stale frame | Redraw request, frame ID, flush checkpoint, and completion response |
| Static screen never flushes | UI-context invalidation specifically tied to the pending capture |
| Input grows without bound | Fixed input, partial-line, queue, path, and response limits |
| Two transports diverge | One stdin protocol with platform adapters and common test vectors |
| Lua/widget tooling becomes simulator-only | Host protocol owns test control; no new Lua API |
| Scope expands into a full testing platform | Defer MCP, golden management, rich diffs, and multiple targets |
| Build complexity spreads to firmware | Keep code native-simulator-only and verify firmware-size stability |

## 16. Rollback and follow-up

Automation remains opt-in. If the host process or protocol fails, the simulator
can still run normally without the automation flag. The feature can be removed
without changing model data, Lua APIs, firmware behavior, or persisted radio
settings.

Potential follow-ups, each requiring a separate decision, are:

- an MCP adapter over the accepted host harness;
- CTest/JUnit registration and visual-diff artifacts;
- PNG conversion and comparison in host tooling;
- additional color and monochrome targets;
- more complete telemetry scenario APIs;
- alternate transport for environments where redirected stdin is unavailable;
- hardware smoke-test vocabulary aligned with simulator scenarios; and
- performance and memory probes built on the same checkpoint IDs.

These follow-ups must reuse the accepted protocol rather than adding parallel
simulator control surfaces.
