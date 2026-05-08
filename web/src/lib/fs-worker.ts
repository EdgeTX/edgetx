/**
 * Dedicated FS Worker — owns all OPFS state and serves filesystem
 * requests from WASM workers (via SharedArrayBuffer + Atomics) and
 * the main thread (via its own async channel).
 *
 * Lifecycle:
 *   1. Main thread posts 'init' → backend scans OPFS, posts 'ready'
 *   2. Main thread posts 'channel' × N → worker stores channels
 *   3. Main thread posts 'start' → worker enters Atomics loop
 *   4. Main thread posts 'stop' → worker exits loop, posts 'stopped'
 */

import { OpfsBackend } from './opfs-backend';
import {
  FsOp,
  IDX_REQUEST_FLAG, IDX_RESPONSE_FLAG, IDX_OPCODE,
  IDX_ARG1, IDX_ARG2, IDX_ARG3, IDX_ARG4,
  IDX_RESULT, IDX_ERROR_CODE, IDX_DATA_LEN,
  DATA_BUFFER_SIZE, STAT_SIZE,
  errorStringToCode, serializeStat,
} from './fs-proxy-protocol';

interface Channel {
  ctrl: Int32Array;
  data: Uint8Array;
}

let backend: OpfsBackend;
let channels: Channel[] = [];
let wake: Int32Array;
let running = false;

const encoder = new TextEncoder();
const decoder = new TextDecoder();

function trace(msg: string) {
  self.postMessage({ type: 'trace', text: `[fs-worker] ${msg}\n` });
}

// --- Message handler (init / channel registration / lifecycle) ---

self.onmessage = async (e: MessageEvent) => {
  switch (e.data.type) {
    case 'init': {
      wake = new Int32Array(e.data.wakeBuffer);
      backend = new OpfsBackend();
      await backend.init(e.data.radioKey);
      trace(`init done, radioKey="${e.data.radioKey}", hasContent=${backend.hasContent}`);
      self.postMessage({ type: 'ready', hasContent: backend.hasContent });
      break;
    }
    case 'channel': {
      channels.push({
        ctrl: new Int32Array(e.data.ctrlBuffer),
        data: new Uint8Array(e.data.dataBuffer),
      });
      trace(`channel registered (total: ${channels.length})`);
      break;
    }
    case 'start': {
      if (!running) {
        running = true;
        trace('mainLoop starting');
        mainLoop();
      } else {
        trace('mainLoop already running, ignoring start');
      }
      break;
    }
    case 'stop': {
      running = false;
      if (wake) {
        Atomics.store(wake, 0, 1);
        Atomics.notify(wake, 0);
      }
      break;
    }

    // --- UI operations (used by main thread before/after sim runs) ---

    case 'readTextFile': {
      try {
        const data = await backend.readFile(e.data.path);
        const text = new TextDecoder().decode(data);
        self.postMessage({ type: 'readTextFileDone', id: e.data.id, text });
      } catch (err: any) {
        self.postMessage({ type: 'readTextFileDone', id: e.data.id, error: err?.message ?? 'read error' });
      }
      break;
    }
    case 'writeFile': {
      try {
        await backend.writeFile(e.data.path, new Uint8Array(e.data.data));
        self.postMessage({ type: 'writeFileDone', id: e.data.id });
      } catch (err: any) {
        self.postMessage({ type: 'writeFileDone', id: e.data.id, error: err?.message ?? 'write error' });
      }
      break;
    }
    case 'wipe': {
      await backend.wipe();
      self.postMessage({ type: 'wiped', id: e.data.id });
      break;
    }
    case 'listFiles': {
      const files = backend.listFiles(e.data.basePath ?? '/');
      self.postMessage({ type: 'listFilesDone', id: e.data.id, files });
      break;
    }
  }
};

// --- Main loop ---

async function mainLoop() {
  trace(`mainLoop entered, channels=${channels.length}`);
  while (running) {
    const asyncWork = runSyncBatch();
    if (asyncWork) {
      await asyncWork();
    } else {
      // Yield to let the event loop process messages (stop, etc.)
      await new Promise<void>(resolve => setTimeout(resolve, 0));
    }
  }
  trace('mainLoop exiting, closing handles');
  backend.closeAll();
  self.postMessage({ type: 'stopped' });
}

/**
 * Process requests synchronously until an async operation is needed
 * or we've been idle long enough to yield for the event loop.
 */
function runSyncBatch(): (() => Promise<void>) | null {
  let idleCount = 0;

  while (running) {
    let didWork = false;

    for (const ch of channels) {
      if (Atomics.load(ch.ctrl, IDX_REQUEST_FLAG) !== 0) {
        const asyncNeeded = dispatch(ch);
        if (asyncNeeded) return asyncNeeded;
        didWork = true;
      }
    }

    if (didWork) {
      idleCount = 0;
    } else {
      idleCount++;
      if (idleCount >= 2) return null; // yield after ~10ms idle
      Atomics.wait(wake, 0, 0, 5);
      Atomics.store(wake, 0, 0);
    }
  }
  return null;
}

// --- Request dispatch ---

function dispatch(ch: Channel): (() => Promise<void>) | null {
  const opcode = Atomics.load(ch.ctrl, IDX_OPCODE) as FsOp;
  const a1 = Atomics.load(ch.ctrl, IDX_ARG1);
  const a2 = Atomics.load(ch.ctrl, IDX_ARG2);
  const a3 = Atomics.load(ch.ctrl, IDX_ARG3);
  const a4 = Atomics.load(ch.ctrl, IDX_ARG4);
  const dataLen = Atomics.load(ch.ctrl, IDX_DATA_LEN);

  try {
    switch (opcode) {

      // ============ Async operations ============

      case FsOp.OPEN: {
        let path: string;
        let flags: number | string;
        if (a1 === -1) {
          // String flags: [pathLen:u32][path][flagsStr]
          const pathLen = new DataView(ch.data.buffer, ch.data.byteOffset, 4).getUint32(0, true);
          path = decoder.decode(ch.data.slice(4, 4 + pathLen));
          flags = decoder.decode(ch.data.slice(4 + pathLen, dataLen));
        } else {
          path = decoder.decode(ch.data.slice(0, dataLen));
          flags = a1;
        }
        const mode = a2;
        return async () => {
          try {
            const fd = await backend.open(path, flags, mode);
            Atomics.store(ch.ctrl, IDX_RESULT, fd);
            respond(ch);
          } catch (e: any) { respondError(ch, e); }
        };
      }

      case FsOp.MKDIR: {
        const path = decoder.decode(ch.data.slice(0, dataLen));
        return async () => {
          try {
            await backend.mkdir(path);
            respond(ch);
          } catch (e: any) { respondError(ch, e); }
        };
      }

      case FsOp.RENAME: {
        const dv = new DataView(ch.data.buffer, ch.data.byteOffset);
        const oldLen = dv.getUint32(0, true);
        const oldPath = decoder.decode(ch.data.slice(4, 4 + oldLen));
        const newPath = decoder.decode(ch.data.slice(4 + oldLen, dataLen));
        return async () => {
          try {
            await backend.rename(oldPath, newPath);
            respond(ch);
          } catch (e: any) { respondError(ch, e); }
        };
      }

      case FsOp.RMDIR: {
        const path = decoder.decode(ch.data.slice(0, dataLen));
        return async () => {
          try {
            await backend.rmdir(path);
            respond(ch);
          } catch (e: any) { respondError(ch, e); }
        };
      }

      case FsOp.UNLINK: {
        const path = decoder.decode(ch.data.slice(0, dataLen));
        return async () => {
          try {
            await backend.unlink(path);
            respond(ch);
          } catch (e: any) { respondError(ch, e); }
        };
      }

      // ============ Sync operations ============

      case FsOp.CLOSE: {
        backend.close(a1);
        respond(ch);
        return null;
      }

      case FsOp.READ: {
        const len = Math.min(a2, DATA_BUFFER_SIZE);
        const pos = a4 === -1 ? null : (a4 * 0x100000000 + (a3 >>> 0));
        const { data, bytesRead } = backend.read(a1, len, pos);
        ch.data.set(data.subarray(0, bytesRead), 0);
        Atomics.store(ch.ctrl, IDX_RESULT, bytesRead);
        Atomics.store(ch.ctrl, IDX_DATA_LEN, bytesRead);
        respond(ch);
        return null;
      }

      case FsOp.WRITE: {
        const pos = a3 === -1 ? null : (a3 * 0x100000000 + (a2 >>> 0));
        const writeData = ch.data.slice(0, dataLen);
        const written = backend.write(a1, writeData, pos);
        Atomics.store(ch.ctrl, IDX_RESULT, written);
        respond(ch);
        return null;
      }

      case FsOp.FSTAT: {
        const stat = backend.fstat(a1);
        serializeStat(stat, ch.data, 0);
        Atomics.store(ch.ctrl, IDX_DATA_LEN, STAT_SIZE);
        respond(ch);
        return null;
      }

      case FsOp.STAT: {
        const path = decoder.decode(ch.data.slice(0, dataLen));
        const stat = backend.stat(path);
        serializeStat(stat, ch.data, 0);
        Atomics.store(ch.ctrl, IDX_DATA_LEN, STAT_SIZE);
        respond(ch);
        return null;
      }

      case FsOp.LSTAT: {
        const path = decoder.decode(ch.data.slice(0, dataLen));
        const stat = backend.lstat(path);
        serializeStat(stat, ch.data, 0);
        Atomics.store(ch.ctrl, IDX_DATA_LEN, STAT_SIZE);
        respond(ch);
        return null;
      }

      case FsOp.FTRUNCATE: {
        backend.ftruncate(a1, a2);
        respond(ch);
        return null;
      }

      case FsOp.FUTIMES: {
        const tv = new DataView(ch.data.buffer, ch.data.byteOffset, 16);
        backend.futimes(a1, tv.getFloat64(0, true), tv.getFloat64(8, true));
        respond(ch);
        return null;
      }

      case FsOp.UTIMES: {
        const path = decoder.decode(ch.data.slice(0, a1));
        const tv = new DataView(ch.data.buffer, ch.data.byteOffset + a1, 16);
        backend.utimes(path, tv.getFloat64(0, true), tv.getFloat64(8, true));
        respond(ch);
        return null;
      }

      case FsOp.READDIR: {
        const path = decoder.decode(ch.data.slice(0, dataLen));
        if (a1) {
          // withFileTypes
          const entries = backend.readdir(path, true);
          let off = 0;
          const dv = new DataView(ch.data.buffer, ch.data.byteOffset);
          dv.setUint32(off, entries.length, true); off += 4;
          for (const ent of entries) {
            let type = 0;
            if (ent.isFile()) type = 1;
            else if (ent.isDirectory()) type = 2;
            ch.data[off++] = type;
            const nameBytes = encoder.encode(ent.name);
            dv.setUint32(off, nameBytes.length, true); off += 4;
            ch.data.set(nameBytes, off); off += nameBytes.length;
          }
          Atomics.store(ch.ctrl, IDX_DATA_LEN, off);
        } else {
          const entries = backend.readdir(path, false) as string[];
          const json = JSON.stringify(entries);
          const bytes = encoder.encode(json);
          ch.data.set(bytes, 0);
          Atomics.store(ch.ctrl, IDX_DATA_LEN, bytes.length);
        }
        respond(ch);
        return null;
      }

      case FsOp.LINK:
        throw Object.assign(new Error('ENOSYS: hardlinks not supported'), { code: 'ENOSYS' });

      case FsOp.SYMLINK:
        throw Object.assign(new Error('ENOSYS: symlinks not supported'), { code: 'ENOSYS' });

      case FsOp.READLINK: {
        // OPFS has no symlinks — return EINVAL (not a symlink), which is
        // what Node.js returns for non-symlink paths.  The WASI library
        // calls readlinkSync during path resolution and expects EINVAL to
        // mean "not a symlink, continue resolving."
        const rlPath = decoder.decode(ch.data.slice(0, dataLen));
        throw Object.assign(new Error(`EINVAL: ${rlPath}`), { code: 'EINVAL' });
      }

      case FsOp.REALPATH: {
        const path = decoder.decode(ch.data.slice(0, dataLen));
        const resolved = backend.realpath(path);
        const bytes = encoder.encode(resolved);
        ch.data.set(bytes, 0);
        Atomics.store(ch.ctrl, IDX_DATA_LEN, bytes.length);
        respond(ch);
        return null;
      }

      case FsOp.FDATASYNC: {
        backend.fdatasync(a1);
        respond(ch);
        return null;
      }

      case FsOp.FSYNC: {
        backend.fsync(a1);
        respond(ch);
        return null;
      }

      default:
        throw Object.assign(new Error(`unsupported fs op: ${opcode}`), { code: 'ENOSYS' });
    }
  } catch (e: any) {
    respondError(ch, e);
    return null;
  }
}

// --- Response helpers ---

function respond(ch: Channel): void {
  Atomics.store(ch.ctrl, IDX_ERROR_CODE, 0);
  Atomics.store(ch.ctrl, IDX_REQUEST_FLAG, 0);
  Atomics.store(ch.ctrl, IDX_RESPONSE_FLAG, 1);
  Atomics.notify(ch.ctrl, IDX_RESPONSE_FLAG);
}

function respondError(ch: Channel, e: any): void {
  // Don't log EINVAL from readlink (expected, very frequent) or ENOENT (normal)
  if (e?.code !== 'EINVAL' && e?.code !== 'ENOENT') {
    trace(`ERROR ${e?.code ?? '?'}: ${e?.message ?? e}`);
  }
  const code = e?.code ? errorStringToCode(e.code) : 255;
  Atomics.store(ch.ctrl, IDX_ERROR_CODE, code);
  const msg = encoder.encode(e?.message ?? 'unknown error');
  const len = Math.min(msg.length, DATA_BUFFER_SIZE);
  ch.data.set(msg.subarray(0, len), 0);
  Atomics.store(ch.ctrl, IDX_DATA_LEN, len);
  Atomics.store(ch.ctrl, IDX_REQUEST_FLAG, 0);
  Atomics.store(ch.ctrl, IDX_RESPONSE_FLAG, 1);
  Atomics.notify(ch.ctrl, IDX_RESPONSE_FLAG);
}
