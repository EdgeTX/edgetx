import { ThreadMessageHandler, WASIThreads } from '@emnapi/wasi-threads';
import { WASI } from '@tybys/wasm-util';
import { FsProxyClient } from './fs-proxy-client';

// Catch all errors for debugging
globalThis.addEventListener('error', (e) => {
  postMessage({ type: 'trace', text: `[worker error] ${e.message} at ${e.filename}:${e.lineno}\n` });
});
globalThis.addEventListener('unhandledrejection', (e) => {
  postMessage({ type: 'trace', text: `[worker rejection] ${e.reason}\n` });
});

// Shared analog values buffer, received from main thread before thread start
let analogValues: Int16Array | null = null;

// LCD sync buffer, received from main thread before thread start
let lcdSync: Int32Array | null = null;

// Filesystem proxy client, received from main thread before thread start
let fsClient: FsProxyClient | null = null;
let wakeBuffer: SharedArrayBuffer | null = null;

const handler = new ThreadMessageHandler({
  async onLoad({ wasmModule, wasmMemory }) {
    const post = (s: string) => postMessage({ type: 'trace', text: s + '\n' });

    if (!fsClient) {
      throw new Error('No fs-channel received before thread start');
    }
    const fs = fsClient;

    let wasi: WASI;
    try {
      wasi = new WASI({
        version: 'preview1',
        fs: fs as any,
        preopens: { '/': '/' },
        print: post,
        printErr: post,
      });
    } catch (e: any) {
      post('[worker] WASI init error: ' + (e?.stack ?? e?.message ?? e));
      throw e;
    }
    const wasiThreads = new WASIThreads({
      wasi: wasi as any,
      childThread: true,
    });

    const instance = await WebAssembly.instantiate(wasmModule, {
      env: {
        memory: wasmMemory,
        simuGetAnalog: (idx: number): number => analogValues?.[idx] ?? 0,
        simuQueueAudio: (buf: number, len: number): void => {
          const samples = new Int16Array(wasmMemory.buffer, buf, len / 2);
          // Copy and relay to main thread for playback
          postMessage({ type: 'audio', samples: new Int16Array(samples) });
        },
        simuTrace: (_ptr: number): void => {},
        simuLcdNotify: (): void => {
          if (lcdSync) {
            Atomics.add(lcdSync, 0, 1);
            Atomics.notify(lcdSync, 0);
          }
        },
      },
      wasi_snapshot_preview1: wasi.wasiImport,
      wasi: { ...wasiThreads.getImportObject().wasi },
    });

    const initialized = wasiThreads.initialize(
      instance,
      wasmModule,
      wasmMemory
    );

    return { module: wasmModule, instance: initialized };
  },
});

globalThis.onmessage = function (e) {
  if (e.data?.type === 'analog-buffer') {
    analogValues = new Int16Array(e.data.buffer);
    return;
  }
  if (e.data?.type === 'lcd-sync') {
    lcdSync = new Int32Array(e.data.buffer);
    return;
  }
  if (e.data?.type === 'wake-buffer') {
    wakeBuffer = e.data.buffer;
    return;
  }
  if (e.data?.type === 'fs-channel') {
    fsClient = new FsProxyClient(e.data.ctrlBuffer, e.data.dataBuffer, wakeBuffer ?? undefined);
    return;
  }
  handler.handle(e);
};
