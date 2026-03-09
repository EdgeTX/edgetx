import { WASIThreads } from '@emnapi/wasi-threads';
import type { WASIInstance } from '@emnapi/wasi-threads';
import { WASI } from '@tybys/wasm-util';
import { Volume, createFsFromVolume } from 'memfs-browser';

export interface SimulatorExports {
  memory: WebAssembly.Memory;
  malloc: (size: number) => number;
  free: (ptr: number) => void;

  simuInit: () => void;
  simuStart: (tests: number) => void;
  simuStop: () => void;
  simuIsRunning: () => number;
  simuFatfsSetPaths: (sdPath: number, settingsPath: number) => void;

  simuSetKey: (key: number, state: number) => void;
  simuSetTrim: (trim: number, state: number) => void;
  simuSetSwitch: (swtch: number, state: number) => void;
  simuSetTrimValue: (idx: number, value: number) => void;
  simuTouchDown: (x: number, y: number) => void;
  simuTouchUp: () => void;
  simuRotaryEncoderEvent: (steps: number) => void;

  simuLcdChanged: () => number;
  simuLcdCopy: (buf: number, maxLen: number) => number;
  simuLcdGetWidth: () => number;
  simuLcdGetHeight: () => number;
  simuLcdGetDepth: () => number;
  simuLcdFlushed: () => void;

  simuGetCapability: (cap: number) => number;
  simuAudioGetVolume: () => number;
}

export type TraceCallback = (text: string) => void;
export type AudioCallback = (samples: Int16Array) => void;

/** Parse the WASM binary import section to find the memory import limits. */
function getMemoryImport(bytes: Uint8Array): { initial: number; maximum: number } {
  // Minimal WASM binary parser – just enough to find the memory import.
  let off = 8; // skip magic + version

  function readU32Leb(): number {
    let result = 0, shift = 0;
    while (true) {
      const b = bytes[off++];
      result |= (b & 0x7f) << shift;
      if ((b & 0x80) === 0) return result;
      shift += 7;
    }
  }

  function readName(): string {
    const len = readU32Leb();
    const s = new TextDecoder().decode(bytes.subarray(off, off + len));
    off += len;
    return s;
  }

  while (off < bytes.length) {
    const sectionId = bytes[off++];
    const sectionLen = readU32Leb();
    const sectionEnd = off + sectionLen;

    if (sectionId !== 2) { // not Import section
      off = sectionEnd;
      continue;
    }

    const count = readU32Leb();
    for (let i = 0; i < count; i++) {
      const mod = readName();
      const name = readName();
      const kind = bytes[off++];
      if (kind === 2) {
        // Memory import: flags, initial, [maximum]
        const flags = readU32Leb();
        const initial = readU32Leb();
        const maximum = (flags & 1) ? readU32Leb() : 65536;
        if (mod === 'env' && name === 'memory') {
          return { initial, maximum };
        }
      } else if (kind === 0) { readU32Leb(); } // func: typeidx
      else if (kind === 1) { readU32Leb(); readU32Leb(); readU32Leb(); } // table: reftype + limits
      else if (kind === 3) { off += 2; } // global: valtype + mut
      else if (kind === 4) { readU32Leb(); } // tag: typeidx
    }
    break;
  }
  // Fallback if not found
  return { initial: 256, maximum: 32768 };
}

export class WasmRunner {
  private wasiThreads: WASIThreads;
  private _exports: SimulatorExports | null = null;

  private analogValues = new Int16Array(32);
  private onTrace: TraceCallback;
  private onAudio: AudioCallback;

  constructor(onTrace: TraceCallback, onAudio: AudioCallback) {
    this.onTrace = onTrace;
    this.onAudio = onAudio;

    const wasi = new WASI({
      version: 'preview1',
      fs: createFsFromVolume(Volume.fromJSON({ '/': null })) as any,
      preopens: { '/': '/' },
      print: (s: string) => this.onTrace(s + '\n'),
      printErr: (s: string) => this.onTrace(s + '\n'),
    });

    this.wasiThreads = new WASIThreads({
      wasi: wasi as WASIInstance,
      reuseWorker: { size: 4, strict: true },
      waitThreadStart: typeof window === 'undefined' ? 1000 : false,
      onCreateWorker: () => {
        const worker = new Worker(new URL('./worker.ts', import.meta.url), {
          type: 'module',
        });
        worker.addEventListener('message', (e) => {
          if (e.data?.type === 'trace') {
            this.onTrace(e.data.text);
          }
        });
        return worker;
      },
    });
  }

  get exports(): SimulatorExports | null {
    return this._exports;
  }

  async load(wasmPath: string): Promise<void> {
    const response = await fetch(wasmPath);
    if (!response.ok) {
      throw new Error(`Failed to fetch WASM: ${response.statusText}`);
    }

    // Read the first stream chunk to parse memory import requirements,
    // then compile+instantiate the full module via streaming.
    const clone = response.clone();
    const reader = response.body!.getReader();
    const { value: headerBytes } = await reader.read();
    reader.cancel();
    const { initial, maximum } = getMemoryImport(headerBytes!);
    const memory = new WebAssembly.Memory({ initial, maximum, shared: true });

    const readCStr = (ptr: number): string => {
      const view = new Uint8Array(memory.buffer);
      let end = ptr;
      while (view[end] !== 0) end++;
      return new TextDecoder('utf-8').decode(view.subarray(ptr, end));
    };

    const wasi = this.wasiThreads.wasi;
    const { module, instance } = await WebAssembly.instantiateStreaming(
      clone,
      {
        wasi_snapshot_preview1:
          wasi.wasiImport as WebAssembly.ModuleImports,
        wasi: { ...this.wasiThreads.getImportObject().wasi },
        env: {
          memory,
          simuGetAnalog: (idx: number): number => {
            return this.analogValues[idx] ?? 0;
          },
          simuQueueAudio: (buf: number, len: number): void => {
            const view = new Int16Array(memory.buffer, buf, len / 2);
            this.onAudio(view);
          },
          simuTrace: (ptr: number): void => {
            this.onTrace(readCStr(ptr));
          },
        },
      }
    );

    this._exports = instance.exports as unknown as SimulatorExports;

    this.wasiThreads.initialize(instance, module, memory);
    await this.wasiThreads.preloadWorkers();
  }

  setAnalog(index: number, value: number): void {
    if (index >= 0 && index < this.analogValues.length) {
      this.analogValues[index] = value;
    }
  }

  /** Copy LCD framebuffer from WASM memory into a host-side Uint8Array */
  copyLcd(size: number): Uint8Array | null {
    const ex = this._exports;
    if (!ex) return null;

    const wasmBuf = ex.malloc(size);
    if (!wasmBuf) return null;

    const copied = ex.simuLcdCopy(wasmBuf, size);
    if (copied === 0) {
      ex.free(wasmBuf);
      return null;
    }

    const mem = new Uint8Array(ex.memory.buffer, wasmBuf, copied);
    const result = new Uint8Array(copied);
    result.set(mem);
    ex.free(wasmBuf);
    return result;
  }
}
