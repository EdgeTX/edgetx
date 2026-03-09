import { ThreadMessageHandler, WASIThreads } from '@emnapi/wasi-threads';
import { WASI } from '@tybys/wasm-util';
import { Volume, createFsFromVolume } from 'memfs-browser';

// Shared analog values buffer, received from main thread before thread start
let analogValues: Int16Array | null = null;

const handler = new ThreadMessageHandler({
  async onLoad({ wasmModule, wasmMemory }) {
    const post = (s: string) => postMessage({ type: 'trace', text: s + '\n' });
    const wasi = new WASI({
      version: 'preview1',
      fs: createFsFromVolume(Volume.fromJSON({ '/': null })) as any,
      preopens: { '/': '/' },
      print: post,
      printErr: post,
    });
    const wasiThreads = new WASIThreads({
      wasi: wasi as any,
      childThread: true,
    });

    const instance = await WebAssembly.instantiate(wasmModule, {
      env: {
        memory: wasmMemory,
        simuGetAnalog: (idx: number): number => analogValues?.[idx] ?? 0,
        simuQueueAudio: (_buf: number, _len: number): void => {},
        simuTrace: (_ptr: number): void => {},
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
  handler.handle(e);
};
