/**
 * Main-thread side of the filesystem proxy.
 * Listens for requests from workers via SharedArrayBuffer + Atomics.waitAsync,
 * dispatches them to the memfs instance, and sends responses back.
 */

import { Buffer } from 'buffer';
import {
  FsOp,
  IDX_REQUEST_FLAG, IDX_RESPONSE_FLAG, IDX_OPCODE,
  IDX_ARG1, IDX_ARG2, IDX_ARG3, IDX_ARG4,
  IDX_RESULT, IDX_ERROR_CODE, IDX_DATA_LEN,
  CTRL_BUFFER_SIZE, DATA_BUFFER_SIZE,
  errorStringToCode, serializeStat, STAT_SIZE,
} from './fs-proxy-protocol';

interface Channel {
  ctrl: Int32Array;
  data: Uint8Array;
  active: boolean;
}

export class FsProxyHost {
  private fs: any;
  private channels = new Map<number, Channel>();
  private encoder = new TextEncoder();
  private decoder = new TextDecoder();
  onTrace: ((msg: string) => void) | null = null;

  constructor(fs: any) {
    this.fs = fs;
  }

  createChannel(workerId: number): { ctrlBuffer: SharedArrayBuffer; dataBuffer: SharedArrayBuffer } {
    const ctrlBuffer = new SharedArrayBuffer(CTRL_BUFFER_SIZE);
    const dataBuffer = new SharedArrayBuffer(DATA_BUFFER_SIZE);
    const ctrl = new Int32Array(ctrlBuffer);
    const data = new Uint8Array(dataBuffer);
    this.channels.set(workerId, { ctrl, data, active: true });
    return { ctrlBuffer, dataBuffer };
  }

  listen(workerId: number): void {
    const channel = this.channels.get(workerId);
    if (!channel) return;

    const poll = () => {
      if (!channel.active) return;

      const result = Atomics.waitAsync(channel.ctrl, IDX_REQUEST_FLAG, 0);
      if (result.async) {
        result.value.then(() => {
          if (!channel.active) return;
          this.processRequest(channel);
          poll();
        });
      } else {
        // Already non-zero — process immediately
        this.processRequest(channel);
        if (channel.active) poll();
      }
    };
    poll();
  }

  removeChannel(workerId: number): void {
    const channel = this.channels.get(workerId);
    if (channel) {
      channel.active = false;
      this.channels.delete(workerId);
    }
  }

  private processRequest(ch: Channel): void {
    const opcode = Atomics.load(ch.ctrl, IDX_OPCODE) as FsOp;
    const dataLen = Atomics.load(ch.ctrl, IDX_DATA_LEN);

    // Log the operation for debugging
    if (this.onTrace) {
      let detail = `op=${FsOp[opcode] ?? opcode}`;
      const a1 = Atomics.load(ch.ctrl, IDX_ARG1);
      if (opcode === FsOp.OPEN || opcode === FsOp.STAT || opcode === FsOp.LSTAT ||
          opcode === FsOp.MKDIR || opcode === FsOp.READDIR || opcode === FsOp.UNLINK ||
          opcode === FsOp.RMDIR || opcode === FsOp.READLINK || opcode === FsOp.REALPATH) {
        const path = this.decoder.decode(ch.data.slice(0, Math.min(dataLen, 200)));
        detail += ` path="${path}"`;
      } else if (opcode === FsOp.READ || opcode === FsOp.WRITE ||
                 opcode === FsOp.FSTAT || opcode === FsOp.CLOSE) {
        detail += ` fd=${a1}`;
      }
      this.onTrace(`[fs-proxy] ${detail}\n`);
    }

    try {
      this.dispatch(opcode, ch);
      Atomics.store(ch.ctrl, IDX_ERROR_CODE, 0);
    } catch (e: any) {
      const code = e?.code ? errorStringToCode(e.code) : 255;
      Atomics.store(ch.ctrl, IDX_ERROR_CODE, code);
      const msg = this.encoder.encode(e?.message ?? 'unknown error');
      ch.data.set(msg.subarray(0, Math.min(msg.length, DATA_BUFFER_SIZE)), 0);
      Atomics.store(ch.ctrl, IDX_DATA_LEN, Math.min(msg.length, DATA_BUFFER_SIZE));
      if (this.onTrace) {
        this.onTrace(`[fs-proxy] ERROR ${e?.code ?? ''}: ${e?.message}\n`);
      }
    }

    // Signal response ready
    Atomics.store(ch.ctrl, IDX_REQUEST_FLAG, 0);
    Atomics.store(ch.ctrl, IDX_RESPONSE_FLAG, 1);
    Atomics.notify(ch.ctrl, IDX_RESPONSE_FLAG);
  }

  private readString(ch: Channel, offset: number, len: number): string {
    return this.decoder.decode(ch.data.slice(offset, offset + len));
  }

  private dispatch(op: FsOp, ch: Channel): void {
    const a1 = Atomics.load(ch.ctrl, IDX_ARG1);
    const a2 = Atomics.load(ch.ctrl, IDX_ARG2);
    const a3 = Atomics.load(ch.ctrl, IDX_ARG3);
    const a4 = Atomics.load(ch.ctrl, IDX_ARG4);
    const dataLen = Atomics.load(ch.ctrl, IDX_DATA_LEN);

    switch (op) {
      case FsOp.OPEN: {
        // a1 = flags (number or -1 for string), a2 = mode
        // data = path string [+ flags string if a1 === -1]
        let path: string;
        let flags: number | string;
        if (a1 === -1) {
          // Flags sent as string after path: [pathLen:u32][path][flagsStr]
          const pathLen = new DataView(ch.data.buffer, ch.data.byteOffset, 4).getUint32(0, true);
          path = this.readString(ch, 4, pathLen);
          flags = this.readString(ch, 4 + pathLen, dataLen - 4 - pathLen);
        } else {
          path = this.readString(ch, 0, dataLen);
          flags = a1;
        }
        const fd = this.fs.openSync(path, flags, a2);
        Atomics.store(ch.ctrl, IDX_RESULT, fd);
        break;
      }

      case FsOp.CLOSE: {
        this.fs.closeSync(a1);
        break;
      }

      case FsOp.READ: {
        // a1=fd, a2=length, a3=position (low32), a4=position (high32, -1 for null)
        const len = Math.min(a2, DATA_BUFFER_SIZE);
        const buf = Buffer.alloc(len);
        const pos = a4 === -1 ? null : (a4 * 0x100000000 + (a3 >>> 0));
        const bytesRead = this.fs.readSync(a1, buf, 0, len, pos);
        ch.data.set(new Uint8Array(buf.buffer, buf.byteOffset, bytesRead), 0);
        Atomics.store(ch.ctrl, IDX_RESULT, bytesRead);
        Atomics.store(ch.ctrl, IDX_DATA_LEN, bytesRead);
        break;
      }

      case FsOp.WRITE: {
        // a1=fd, a2=position (low32), a3=position (high32, -1 for null)
        const pos = a3 === -1 ? null : (a3 * 0x100000000 + (a2 >>> 0));
        const writeData = Buffer.from(ch.data.slice(0, dataLen));
        const written = this.fs.writeSync(a1, writeData, 0, dataLen, pos);
        Atomics.store(ch.ctrl, IDX_RESULT, written);
        break;
      }

      case FsOp.FSTAT: {
        const stat = this.fs.fstatSync(a1, { bigint: true });
        serializeStat(stat, ch.data, 0);
        Atomics.store(ch.ctrl, IDX_DATA_LEN, STAT_SIZE);
        break;
      }

      case FsOp.STAT: {
        const path = this.readString(ch, 0, dataLen);
        const stat = this.fs.statSync(path, { bigint: true });
        serializeStat(stat, ch.data, 0);
        Atomics.store(ch.ctrl, IDX_DATA_LEN, STAT_SIZE);
        break;
      }

      case FsOp.LSTAT: {
        const path = this.readString(ch, 0, dataLen);
        const stat = this.fs.lstatSync(path, { bigint: true });
        serializeStat(stat, ch.data, 0);
        Atomics.store(ch.ctrl, IDX_DATA_LEN, STAT_SIZE);
        break;
      }

      case FsOp.FTRUNCATE: {
        // a1=fd, a2=len
        this.fs.ftruncateSync(a1, a2);
        break;
      }

      case FsOp.FUTIMES: {
        // a1=fd, atime/mtime as f64 in data buffer
        const tv = new DataView(ch.data.buffer, ch.data.byteOffset, 16);
        const atime = tv.getFloat64(0, true);
        const mtime = tv.getFloat64(8, true);
        this.fs.futimesSync(a1, atime, mtime);
        break;
      }

      case FsOp.UTIMES: {
        // path in data, a1=path length, atime/mtime after path
        const path = this.readString(ch, 0, a1);
        const tv = new DataView(ch.data.buffer, ch.data.byteOffset + a1, 16);
        const atime = tv.getFloat64(0, true);
        const mtime = tv.getFloat64(8, true);
        this.fs.utimesSync(path, atime, mtime);
        break;
      }

      case FsOp.MKDIR: {
        const path = this.readString(ch, 0, dataLen);
        this.fs.mkdirSync(path);
        break;
      }

      case FsOp.READDIR: {
        const path = this.readString(ch, 0, dataLen);
        // a1: 1=withFileTypes, 0=names only
        if (a1) {
          const entries: any[] = this.fs.readdirSync(path, { withFileTypes: true });
          // Serialize: [count:u32] then for each [type:u8][nameLen:u32][name bytes]
          let off = 0;
          const dv = new DataView(ch.data.buffer, ch.data.byteOffset);
          dv.setUint32(off, entries.length, true); off += 4;
          for (const ent of entries) {
            let type = 0;
            if (ent.isFile()) type = 1;
            else if (ent.isDirectory()) type = 2;
            else if (typeof ent.isSymbolicLink === 'function' && ent.isSymbolicLink()) type = 3;
            ch.data[off++] = type;
            const nameBytes = this.encoder.encode(ent.name);
            dv.setUint32(off, nameBytes.length, true); off += 4;
            ch.data.set(nameBytes, off); off += nameBytes.length;
          }
          Atomics.store(ch.ctrl, IDX_DATA_LEN, off);
        } else {
          const entries: string[] = this.fs.readdirSync(path);
          const json = JSON.stringify(entries);
          const bytes = this.encoder.encode(json);
          ch.data.set(bytes, 0);
          Atomics.store(ch.ctrl, IDX_DATA_LEN, bytes.length);
        }
        break;
      }

      case FsOp.RENAME: {
        // data = [oldPathLen:u32][oldPath][newPath]
        const dv = new DataView(ch.data.buffer, ch.data.byteOffset);
        const oldLen = dv.getUint32(0, true);
        const oldPath = this.readString(ch, 4, oldLen);
        const newPath = this.readString(ch, 4 + oldLen, dataLen - 4 - oldLen);
        this.fs.renameSync(oldPath, newPath);
        break;
      }

      case FsOp.RMDIR: {
        const path = this.readString(ch, 0, dataLen);
        this.fs.rmdirSync(path);
        break;
      }

      case FsOp.UNLINK: {
        const path = this.readString(ch, 0, dataLen);
        this.fs.unlinkSync(path);
        break;
      }

      case FsOp.LINK: {
        const dv = new DataView(ch.data.buffer, ch.data.byteOffset);
        const oldLen = dv.getUint32(0, true);
        const oldPath = this.readString(ch, 4, oldLen);
        const newPath = this.readString(ch, 4 + oldLen, dataLen - 4 - oldLen);
        this.fs.linkSync(oldPath, newPath);
        break;
      }

      case FsOp.SYMLINK: {
        const dv = new DataView(ch.data.buffer, ch.data.byteOffset);
        const targetLen = dv.getUint32(0, true);
        const target = this.readString(ch, 4, targetLen);
        const path = this.readString(ch, 4 + targetLen, dataLen - 4 - targetLen);
        this.fs.symlinkSync(target, path);
        break;
      }

      case FsOp.READLINK: {
        const path = this.readString(ch, 0, dataLen);
        const target: string = this.fs.readlinkSync(path);
        const bytes = this.encoder.encode(target);
        ch.data.set(bytes, 0);
        Atomics.store(ch.ctrl, IDX_DATA_LEN, bytes.length);
        break;
      }

      case FsOp.REALPATH: {
        const path = this.readString(ch, 0, dataLen);
        const resolved: string = this.fs.realpathSync(path, 'utf8');
        const bytes = this.encoder.encode(resolved);
        ch.data.set(bytes, 0);
        Atomics.store(ch.ctrl, IDX_DATA_LEN, bytes.length);
        break;
      }

      case FsOp.FDATASYNC: {
        if (typeof this.fs.fdatasyncSync === 'function') {
          this.fs.fdatasyncSync(a1);
        }
        break;
      }

      case FsOp.FSYNC: {
        if (typeof this.fs.fsyncSync === 'function') {
          this.fs.fsyncSync(a1);
        }
        break;
      }

      default:
        throw Object.assign(new Error(`unsupported fs op: ${op}`), { code: 'ENOSYS' });
    }
  }
}
