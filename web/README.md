# EdgeTX Web Simulator

Browser-based EdgeTX radio simulator using WebAssembly + WASI threads.

## Prerequisites

- Node.js 20+
- Pre-built `.wasm` modules in `public/` (see [Building WASM Modules](#building-wasm-modules))

## Quick Start

```bash
npm install
npm run dev
```

Open http://localhost:5173 in Chrome, Edge, Safari, or any browser supporting `SharedArrayBuffer` and `Atomics.waitAsync`.

## Building WASM Modules

The build requires [wasi-sdk](https://github.com/WebAssembly/wasi-sdk). The build script resolves it automatically:

1. `$WASI_SDK_PATH` environment variable (if set)
2. `/opt/wasi-sdk/` (default install path, used in CI)
3. Auto-download via `cmake/FetchWasiSDK.cmake` (fetched once, cached across builds)

From the EdgeTX repository root:

```bash
# Build all supported radios
tools/build-wasm-modules.sh

# Build specific radios only
FLAVOR="tx16s;t12;x9dp2019" tools/build-wasm-modules.sh
```

Output `.wasm` files are written to `output/`. Copy them to `web/public/`:

```bash
cp output/*.wasm web/public/
```

### Supported Radios

The radios available in the web UI are defined in `public/radios.json`. This file is **generated** from the authoritative hardware definitions in `radio/src/boards/hw_defs/`. To regenerate it (e.g. after adding a new radio target):

```bash
node web/scripts/gen-radios-json.js
```

The script extracts inputs, switches, trims, keys, and display info from the hw_defs JSON files. Key left/right side placement matches Companion's layout.

## Architecture

```
                    Browser Main Thread
 ┌─────────────────────────────────────────────────────┐
 │                                                     │
 │  App.svelte ──── WasmRunner ──── LcdRenderer        │
 │  (UI, controls)  (loader)        (WebGL canvas)     │
 │       │               │                             │
 │       │          SharedArrayBuffer                  │
 │       │          (analogs, LCD sync)                │
 │       │               │                             │
 │       ▼               ▼                             │
 │  AudioContext    WASM Worker Threads (WASI)         │
 │  (scheduled     ┌──────────────────────────┐        │
 │   playback)     │ worker.ts + FsProxyClient│        │
 │                 └────────────┬─────────────┘        │
 │                    SAB+Atomics (sync I/O)           │
 │                              │                      │
 │                 ┌────────────▼─────────────┐        │
 │                 │ FS Worker (fs-worker.ts) │        │
 │                 │ OpfsBackend (OPFS)       │        │
 │                 └──────────────────────────┘        │
 └─────────────────────────────────────────────────────┘
```

### Lifecycle

1. **Radio selection** — user picks a radio from the dropdown (persisted to localStorage). The FS Worker is spawned and OPFS is scanned for existing data. File uploads are available immediately.
2. **Run** — WASM module is fetched, compiled, and instantiated. Worker threads are spawned with shared memory. The FS Worker's Atomics loop is started. The simulator begins running.
3. **Stop** — firmware shutdown is signalled, worker threads are terminated after a grace period. The FS Worker stays alive for uploads.
4. **Radio switch** — the old instance is torn down (FS Worker included), and a new one is initialized for the selected radio.

### Key Components

- **WasmRunner** — loads WASM, creates shared memory, manages the FS Worker and WASI thread pool. Exposes `initFs()` (spawn FS Worker), `load()` (compile + instantiate WASM), `stopSim()` (terminate threads), and `stopFs()` (terminate FS Worker).
- **FS Worker** (`fs-worker.ts`) — dedicated worker that owns all OPFS state. Serves synchronous filesystem requests from WASM workers via `SharedArrayBuffer` + `Atomics`, and async UI requests (uploads, reads, wipe) via `postMessage`.
- **OpfsBackend** (`opfs-backend.ts`) — in-memory directory tree backed by OPFS `SyncAccessHandle`s. Provides a synchronous Node.js-like filesystem API.
- **FsProxyClient** (`fs-proxy-client.ts`) — worker-side stub that implements `fs.*Sync` methods by writing requests to a shared buffer and blocking on `Atomics.wait` until the FS Worker responds.
- **LcdRenderer** (`lcd-renderer.ts`) — WebGL-based renderer supporting RGB565 (16-bit color), 4-bit grayscale, and 1-bit monochrome (column-major) LCD formats.
- **Audio** — worker threads relay PCM samples via `postMessage`. The main thread schedules them as `AudioBufferSource` nodes for gapless 32 kHz playback.

## Key Files

| File | Description |
|------|-------------|
| `src/App.svelte` | Main UI: radio selector, LCD display, controls, file management |
| `src/lib/wasm-runner.ts` | WASM loader, FS Worker lifecycle, thread management |
| `src/lib/worker.ts` | WASM worker thread entry point (WASI + thread init) |
| `src/lib/fs-worker.ts` | FS Worker: OPFS owner, Atomics dispatch loop, UI file ops |
| `src/lib/opfs-backend.ts` | OPFS-backed synchronous filesystem implementation |
| `src/lib/fs-proxy-client.ts` | Worker-side blocking filesystem proxy |
| `src/lib/fs-proxy-protocol.ts` | Shared protocol constants and serialization |
| `src/lib/lcd-renderer.ts` | LCD framebuffer rendering (RGB565, 4-bit grayscale, 1-bit mono) |
| `public/radios.json` | Radio definitions (generated — do not edit manually) |
| `public/_headers` | COOP/COEP headers for production deployment |
| `scripts/gen-radios-json.js` | Generates `radios.json` from `radio/src/boards/hw_defs/` |

## Browser Requirements

- **SharedArrayBuffer** (requires COOP/COEP headers, configured in `vite.config.ts` and `public/_headers`)
- **Atomics.waitAsync** — Chrome 87+, Safari 16.4+, Edge 87+, Firefox 145+
- **WebAssembly threads** (shared memory)
- **Origin Private File System** (persistent storage across sessions)

## Development

```bash
npm run dev      # Start dev server with HMR
npm run build    # Production build to dist/
npm run preview  # Preview production build
```

### Debugging

- **Filesystem tracing**: Open browser console and run `fsTrace = true` to log all filesystem operations.
- **Trace window**: The simulator UI includes a scrollable trace output showing firmware boot messages and runtime output.

## Production Deployment

The dev server sets the required COOP/COEP headers automatically. For production, `public/_headers` provides them for platforms like Cloudflare Pages. For other hosts, configure your web server to set:

```
Cross-Origin-Embedder-Policy: require-corp
Cross-Origin-Opener-Policy: same-origin
```

### Cloudflare Pages

```bash
npm run build
npx wrangler pages deploy dist --project-name=edgetx-simulator
```
