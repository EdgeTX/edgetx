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

The radios available in the web UI are defined in `public/radios.json`. Each entry specifies the `.wasm` filename, analog inputs, switches, trims, and hardware keys.

Currently configured: TX16S, X10 Express, T15, X9D+ 2019, X-Lite, Jumper T12.

## Architecture

```
┌──────────────────────────────────────────────────┐
│  Browser Main Thread                             │
│  ┌────────────┐  ┌───────────┐  ┌─────────────┐  │
│  │ App.svelte │  │ WasmRunner│  │ FsProxyHost │  │
│  │ (UI)       │──│ (loader)  │──│ (memfs)     │  │
│  └────────────┘  └───────────┘  └─────────────┘  │
│        │              │               ▲          │
│        │         SharedArrayBuffer    │          │
│        │         (analog values)  Atomics.wait   │
│        ▼              │               │          │
│  ┌────────────┐  ┌────┴───────────────┴────┐     │
│  │ LCD Canvas │  │  Worker Threads (WASI)  │     │
│  │ Audio Ctx  │  │  ┌───────┐ ┌─────────┐  │     │
│  └────────────┘  │  │worker │ │FsProxy  │  │     │
│                  │  │.ts    │ │Client   │  │     │
│                  │  └───────┘ └─────────┘  │     │
│                  └─────────────────────────┘     │
└──────────────────────────────────────────────────┘
```

- **WasmRunner** loads and instantiates the WASM module, creates shared memory, and spawns worker threads via `@emnapi/wasi-threads`.
- **Worker threads** each get their own WASI instance with an in-memory filesystem proxy.
- **FsProxyHost/Client** routes filesystem calls from workers to the main thread's `memfs` instance using `SharedArrayBuffer` + `Atomics`.
- **PersistentFS** syncs the in-memory filesystem to/from the browser's Origin Private File System (OPFS) for persistence across sessions.
- **Analog inputs** (sticks, pots, sliders) are shared via a `SharedArrayBuffer` with `Int16Array`, readable by all threads without message passing.
- **Audio** is relayed from worker threads via `postMessage` and played using scheduled `AudioBufferSource` nodes for gapless playback.

## Key Files

| File | Description |
|------|-------------|
| `src/App.svelte` | Main UI: radio selector, LCD display, controls, file management |
| `src/lib/wasm-runner.ts` | WASM loader, memory setup, thread management |
| `src/lib/worker.ts` | Worker thread entry point (WASI + thread init) |
| `src/lib/lcd-renderer.ts` | LCD framebuffer rendering (RGB565, 4-bit grayscale, 1-bit mono) |
| `src/lib/fs-proxy-host.ts` | Main thread filesystem proxy (dispatches to memfs) |
| `src/lib/fs-proxy-client.ts` | Worker-side filesystem proxy (blocking Atomics.wait) |
| `src/lib/fs-proxy-protocol.ts` | Shared protocol constants and serialization |
| `src/lib/persistent-fs.ts` | OPFS persistence layer |
| `public/radios.json` | Radio definitions (inputs, switches, keys, wasm paths) |

## Browser Requirements

- **SharedArrayBuffer** (requires COOP/COEP headers, configured in `vite.config.ts`)
- **Atomics.waitAsync** — Chrome 87+, Safari 16.4+, Edge 87+, Firefox 145+
- **WebAssembly threads** (shared memory)
- **Origin Private File System** (for persistent storage)

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

The dev server automatically sets the required `Cross-Origin-Embedder-Policy` and `Cross-Origin-Opener-Policy` headers. For production, your web server must also set:

```
Cross-Origin-Embedder-Policy: require-corp
Cross-Origin-Opener-Policy: same-origin
```
