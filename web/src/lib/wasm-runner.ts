import { WASIThreads } from '@emnapi/wasi-threads';
import type { WASIInstance } from '@emnapi/wasi-threads';
import { WASI } from '@tybys/wasm-util';
import { CTRL_BUFFER_SIZE, DATA_BUFFER_SIZE, WAKE_BUFFER_SIZE } from './fs-proxy-protocol';

/**
 * Minimal fs stub for the main thread's WASI instance.
 * The main thread only needs enough to satisfy preopen setup — all real
 * filesystem I/O happens in worker threads via FsProxyClient → FS Worker.
 */
const stubFs = (() => {
  let nextFd = 3;
  const dirStat = () => ({
    dev: 0n, ino: 0n, mode: 0o040755n, nlink: 1n,
    uid: 0n, gid: 0n, rdev: 0n, size: 0n,
    blksize: 4096n, blocks: 0n,
    atimeMs: 0n, mtimeMs: 0n, ctimeMs: 0n, birthtimeMs: 0n,
    atimeNs: 0n, mtimeNs: 0n, ctimeNs: 0n, birthtimeNs: 0n,
    atime: new Date(0), mtime: new Date(0), ctime: new Date(0), birthtime: new Date(0),
    isFile: () => false, isDirectory: () => true, isSymbolicLink: () => false,
    isCharacterDevice: () => false, isBlockDevice: () => false,
    isSocket: () => false, isFIFO: () => false,
  });
  return {
    openSync() { return nextFd++; },
    closeSync() {},
    fstatSync() { return dirStat(); },
    statSync() { return dirStat(); },
    lstatSync() { return dirStat(); },
    readdirSync() { return []; },
    readSync() { return 0; },
    writeSync(_fd: number, _b: any, _o: number, len: number) { return len; },
    mkdirSync() {}, renameSync() {}, rmdirSync() {}, unlinkSync() {},
    linkSync() {}, symlinkSync() {},
    readlinkSync(p: string) { return p; },
    realpathSync(p: string) { return p; },
    ftruncateSync() {}, futimesSync() {}, utimesSync() {},
    fdatasyncSync() {}, fsyncSync() {},
  };
})();

export interface SimulatorExports {
  memory: WebAssembly.Memory;
  malloc: (size: number) => number;
  free: (ptr: number) => void;

  simuInit: () => void;
  simuStart: (tests: number) => void;
  simuStop: () => void;
  simuIsRunning: () => number;
  simuFatfsSetPaths: (sdPath: number, settingsPath: number) => void;
  simuCreateDefaults: () => void;

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

  simuIsChannelUsed: (channel: number) => number;
  simuGetChannelsUsed: () => number;
  simuGetMixCount: () => number;

  simuGetNumCustomSwitches: () => number;
  simuGetCustomSwitchState: (idx: number) => number;
  simuGetCustomSwitchColor: (idx: number) => number;
}

export type TraceCallback = (text: string) => void;
export type AudioCallback = (samples: Int16Array) => void;

/** Derive a short radio key from a wasm filename. */
export function radioKeyFromWasm(wasmFile: string): string {
  const base = wasmFile.replace(/.*\//, '').replace(/\.wasm$/, '');
  const m = base.match(/^edgetx-(.+)-simulator$/);
  return m ? m[1] : base;
}

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
  private wasiThreads!: WASIThreads;
  private _exports: SimulatorExports | null = null;

  private analogBuffer = new SharedArrayBuffer(32 * 2);
  private analogValues = new Int16Array(this.analogBuffer);
  /** LCD sync: Int32[0] = frame sequence number, incremented by firmware on each refresh. */
  private lcdSyncBuffer = new SharedArrayBuffer(4);
  private lcdSync = new Int32Array(this.lcdSyncBuffer);
  private onTrace: TraceCallback;
  private onAudio: AudioCallback;
  private fsWorker: Worker | null = null;
  private wakeBuffer: SharedArrayBuffer | null = null;
  private nextFsReqId = 0;
  /** Persistent WASM-side buffer for simuLcdCopy (allocated on first use). */
  private wasmLcdBuf = 0;
  private wasmLcdBufSize = 0;
  /** Set to true (or type `fsTrace = true` in browser console) to log fs proxy ops. */
  fsTrace = false;

  constructor(onTrace: TraceCallback, onAudio: AudioCallback) {
    this.onTrace = onTrace;
    this.onAudio = onAudio;
  }

  get exports(): SimulatorExports | null {
    return this._exports;
  }

  get hasFsWorker(): boolean {
    return this.fsWorker !== null;
  }

  /**
   * Spawn the FS Worker and scan OPFS for existing content.
   * This enables file uploads/reads before WASM is loaded.
   */
  async initFs(radioKey: string): Promise<{ hasContent: boolean }> {
    this.fsWorker = new Worker(new URL('./fs-worker.ts', import.meta.url), { type: 'module' });
    this.fsWorker.addEventListener('message', (e) => {
      if (e.data?.type === 'trace') this.onTrace(e.data.text);
    });
    this.wakeBuffer = new SharedArrayBuffer(WAKE_BUFFER_SIZE);

    const hasContent = await new Promise<boolean>((resolve, reject) => {
      const onMsg = (e: MessageEvent) => {
        if (e.data.type === 'ready') {
          this.fsWorker!.removeEventListener('message', onMsg);
          this.fsWorker!.removeEventListener('error', onErr);
          resolve(e.data.hasContent);
        }
      };
      const onErr = (e: ErrorEvent) => {
        this.fsWorker!.removeEventListener('message', onMsg);
        reject(new Error(e.message));
      };
      this.fsWorker!.addEventListener('message', onMsg);
      this.fsWorker!.addEventListener('error', onErr, { once: true });
      this.fsWorker!.postMessage({ type: 'init', radioKey, wakeBuffer: this.wakeBuffer });
    });

    return { hasContent };
  }

  /**
   * Load and instantiate the WASM module.  Requires initFs() first.
   */
  async load(wasmPath: string): Promise<void> {
    if (!this.fsWorker) throw new Error('initFs() must be called before load()');

    // --- 1. Create main-thread WASI (stub fs — real I/O happens in workers) ---
    const wasi = new WASI({
      version: 'preview1',
      fs: stubFs as any,
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
        worker.postMessage({ type: 'analog-buffer', buffer: this.analogBuffer });
        worker.postMessage({ type: 'lcd-sync', buffer: this.lcdSyncBuffer });
        // Create FS channel for this worker and register with FS Worker
        const ctrlBuffer = new SharedArrayBuffer(CTRL_BUFFER_SIZE);
        const dataBuffer = new SharedArrayBuffer(DATA_BUFFER_SIZE);
        this.fsWorker!.postMessage({ type: 'channel', ctrlBuffer, dataBuffer });
        worker.postMessage({ type: 'wake-buffer', buffer: this.wakeBuffer });
        worker.postMessage({ type: 'fs-channel', ctrlBuffer, dataBuffer });
        worker.addEventListener('message', (e) => {
          if (e.data?.type === 'trace') {
            this.onTrace(e.data.text);
          } else if (e.data?.type === 'audio') {
            this.onAudio(e.data.samples);
          }
        });
        return worker;
      },
    });

    // --- 3. Start FS Worker event loop (before preloadWorkers, which triggers FS calls) ---
    this.fsWorker.postMessage({ type: 'start' });

    // --- 4. Load and compile WASM ---
    const response = await fetch(wasmPath);
    if (!response.ok) {
      throw new Error(`Failed to fetch WASM: ${response.statusText}`);
    }

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

    const wasiObj = this.wasiThreads.wasi;
    const { module, instance } = await WebAssembly.instantiateStreaming(
      clone,
      {
        wasi_snapshot_preview1:
          wasiObj.wasiImport as WebAssembly.ModuleImports,
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
          simuLcdNotify: (): void => {
            Atomics.add(this.lcdSync, 0, 1);
            Atomics.notify(this.lcdSync, 0);
          },
        },
      }
    );

    this._exports = instance.exports as unknown as SimulatorExports;

    this.wasiThreads.initialize(instance, module, memory);
    await this.wasiThreads.preloadWorkers();
  }

  // --- FS Worker message helpers ---

  private fsMessage(type: string, payload: Record<string, any> = {}, transfer: Transferable[] = []): Promise<any> {
    return new Promise((resolve, reject) => {
      if (!this.fsWorker) return reject(new Error('FS Worker not running'));
      const id = ++this.nextFsReqId;
      const handler = (e: MessageEvent) => {
        if (e.data.id === id) {
          this.fsWorker!.removeEventListener('message', handler);
          if (e.data.error) reject(new Error(e.data.error));
          else resolve(e.data);
        }
      };
      this.fsWorker.addEventListener('message', handler);
      this.fsWorker.postMessage({ ...payload, type, id }, { transfer });
    });
  }

  async fsReadTextFile(path: string): Promise<string | null> {
    try {
      const result = await this.fsMessage('readTextFile', { path });
      return result.text;
    } catch {
      return null;
    }
  }

  async fsWriteFile(path: string, data: ArrayBuffer): Promise<void> {
    await this.fsMessage('writeFile', { path, data }, [data]);
  }

  async fsWipe(): Promise<void> {
    await this.fsMessage('wipe');
  }

  async fsListFiles(basePath = '/'): Promise<string[]> {
    const result = await this.fsMessage('listFiles', { basePath });
    return result.files;
  }

  /** Terminate WASM worker threads but keep the FS Worker alive. */
  stopSim(): void {
    if (this.wasiThreads) {
      this.wasiThreads.terminateAllThreads();
    }
    this._exports = null;
    // Reset LCD buffer pointer — it belonged to the old WASM memory
    this.wasmLcdBuf = 0;
    this.wasmLcdBufSize = 0;
  }

  /** Stop the FS Worker and clean up. */
  async stopFs(): Promise<void> {
    if (!this.fsWorker) return;
    await new Promise<void>((resolve) => {
      const handler = (e: MessageEvent) => {
        if (e.data.type === 'stopped') {
          this.fsWorker!.removeEventListener('message', handler);
          resolve();
        }
      };
      this.fsWorker!.addEventListener('message', handler);
      this.fsWorker!.postMessage({ type: 'stop' });
    });
    this.fsWorker.terminate();
    this.fsWorker = null;
  }

  setAnalog(index: number, value: number): void {
    if (index >= 0 && index < this.analogValues.length) {
      this.analogValues[index] = value;
    }
  }

  /** Allocate a C string in WASM linear memory and return its pointer. */
  private allocCStr(s: string): number {
    const ex = this._exports!;
    const encoded = new TextEncoder().encode(s);
    const ptr = ex.malloc(encoded.length + 1);
    if (!ptr) throw new Error('malloc failed');
    // Re-derive view after malloc (buffer may have grown)
    const view = new Uint8Array(ex.memory.buffer);
    view.set(encoded, ptr);
    view[ptr + encoded.length] = 0;
    return ptr;
  }

  /** Tell the firmware where the SD card / settings directories are. */
  setFatfsPaths(sdPath: string, settingsPath: string): void {
    const ex = this._exports;
    if (!ex) return;

    const sdPtr = this.allocCStr(sdPath);
    const settingsPtr = this.allocCStr(settingsPath);
    ex.simuFatfsSetPaths(sdPtr, settingsPtr);
    ex.free(sdPtr);
    ex.free(settingsPtr);
  }

  /**
   * Wait for the firmware to signal a new LCD frame.
   * Returns true if a frame is ready, false on timeout.
   * Uses Atomics.waitAsync so the main thread is not blocked.
   */
  async waitForLcdFrame(timeout = 100): Promise<boolean> {
    const current = Atomics.load(this.lcdSync, 0);
    const result = Atomics.waitAsync(this.lcdSync, 0, current, timeout);
    if (result.async) {
      const status = await result.value;
      return status === 'ok';
    }
    // 'not-equal' means the value already changed — frame is ready
    return true;
  }

  /** Copy LCD framebuffer from WASM memory into a host-side Uint8Array */
  copyLcd(size: number): Uint8Array | null {
    const ex = this._exports;
    if (!ex) return null;

    // Allocate persistent WASM buffer on first use or if size changed
    if (!this.wasmLcdBuf || this.wasmLcdBufSize < size) {
      if (this.wasmLcdBuf) ex.free(this.wasmLcdBuf);
      this.wasmLcdBuf = ex.malloc(size);
      this.wasmLcdBufSize = this.wasmLcdBuf ? size : 0;
      if (!this.wasmLcdBuf) return null;
    }

    const copied = ex.simuLcdCopy(this.wasmLcdBuf, size);
    if (copied === 0) return null;

    const mem = new Uint8Array(ex.memory.buffer, this.wasmLcdBuf, copied);
    const result = new Uint8Array(copied);
    result.set(mem);
    return result;
  }
}
