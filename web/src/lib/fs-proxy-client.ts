/**
 * Worker-thread side of the filesystem proxy.
 * Implements a synchronous Node.js-style fs interface by forwarding
 * calls to the main thread via SharedArrayBuffer + Atomics.wait.
 */

import {
  FsOp,
  IDX_REQUEST_FLAG, IDX_RESPONSE_FLAG, IDX_OPCODE,
  IDX_ARG1, IDX_ARG2, IDX_ARG3, IDX_ARG4,
  IDX_RESULT, IDX_ERROR_CODE, IDX_DATA_LEN,
  errorCodeToString, deserializeStat,
} from './fs-proxy-protocol';

class BigIntStatsProxy {
  dev: bigint; ino: bigint; mode: bigint; nlink: bigint;
  uid: bigint; gid: bigint; rdev: bigint; size: bigint;
  blksize: bigint; blocks: bigint;
  atimeMs: bigint; mtimeMs: bigint; ctimeMs: bigint; birthtimeMs: bigint;
  atimeNs: bigint; mtimeNs: bigint; ctimeNs: bigint; birthtimeNs: bigint;
  atime: Date; mtime: Date; ctime: Date; birthtime: Date;
  private _flags: number;

  constructor(d: ReturnType<typeof deserializeStat>) {
    this.dev = d.dev; this.ino = d.ino; this.mode = d.mode; this.nlink = d.nlink;
    this.uid = d.uid; this.gid = d.gid; this.rdev = d.rdev; this.size = d.size;
    this.blksize = d.blksize; this.blocks = d.blocks;
    this.atimeMs = d.atimeMs; this.mtimeMs = d.mtimeMs;
    this.ctimeMs = d.ctimeMs; this.birthtimeMs = d.birthtimeMs;
    this.atimeNs = d.atimeMs * 1000000n;
    this.mtimeNs = d.mtimeMs * 1000000n;
    this.ctimeNs = d.ctimeMs * 1000000n;
    this.birthtimeNs = d.birthtimeMs * 1000000n;
    this.atime = new Date(Number(d.atimeMs));
    this.mtime = new Date(Number(d.mtimeMs));
    this.ctime = new Date(Number(d.ctimeMs));
    this.birthtime = new Date(Number(d.birthtimeMs));
    this._flags = d.flags;
  }

  isFile(): boolean { return !!(this._flags & 1); }
  isDirectory(): boolean { return !!(this._flags & 2); }
  isSymbolicLink(): boolean { return !!(this._flags & 4); }
  isCharacterDevice(): boolean { return !!(this._flags & 8); }
  isBlockDevice(): boolean { return !!(this._flags & 16); }
  isSocket(): boolean { return !!(this._flags & 32); }
  isFIFO(): boolean { return !!(this._flags & 64); }
}

class DirentProxy {
  name: string;
  private _type: number;

  constructor(name: string, type: number) {
    this.name = name;
    this._type = type;
  }

  isFile(): boolean { return this._type === 1; }
  isDirectory(): boolean { return this._type === 2; }
  isSymbolicLink(): boolean { return this._type === 3; }
  isCharacterDevice(): boolean { return this._type === 4; }
  isBlockDevice(): boolean { return this._type === 5; }
  isSocket(): boolean { return this._type === 6; }
  isFIFO(): boolean { return this._type === 7; }
}

export class FsProxyClient {
  private ctrl: Int32Array;
  private data: Uint8Array;
  private wake: Int32Array | null;
  private encoder = new TextEncoder();
  private decoder = new TextDecoder();

  constructor(ctrlBuffer: SharedArrayBuffer, dataBuffer: SharedArrayBuffer, wakeBuffer?: SharedArrayBuffer) {
    this.ctrl = new Int32Array(ctrlBuffer);
    this.data = new Uint8Array(dataBuffer);
    this.wake = wakeBuffer ? new Int32Array(wakeBuffer) : null;
  }

  private call(opcode: FsOp): void {
    Atomics.store(this.ctrl, IDX_OPCODE, opcode);
    Atomics.store(this.ctrl, IDX_RESPONSE_FLAG, 0);
    Atomics.store(this.ctrl, IDX_REQUEST_FLAG, 1);
    Atomics.notify(this.ctrl, IDX_REQUEST_FLAG);
    if (this.wake) {
      Atomics.store(this.wake, 0, 1);
      Atomics.notify(this.wake, 0);
    }
    Atomics.wait(this.ctrl, IDX_RESPONSE_FLAG, 0);
  }

  private checkError(): void {
    const code = Atomics.load(this.ctrl, IDX_ERROR_CODE);
    if (code !== 0) {
      const len = Atomics.load(this.ctrl, IDX_DATA_LEN);
      const msg = this.decoder.decode(this.data.slice(0, len));
      const err = new Error(msg);
      (err as any).code = errorCodeToString(code);
      throw err;
    }
  }

  private writeStr(s: string, offset = 0): number {
    const bytes = this.encoder.encode(s);
    this.data.set(bytes, offset);
    return bytes.length;
  }

  openSync(path: string, flags: number | string, mode: number = 0o666): number {
    if (typeof flags === 'string') {
      // Send flags as string: a1=-1 sentinel, data=[pathLen:u32][path][flagsStr]
      const pathBytes = this.encoder.encode(path);
      const flagsBytes = this.encoder.encode(flags);
      const dv = new DataView(this.data.buffer, this.data.byteOffset);
      dv.setUint32(0, pathBytes.length, true);
      this.data.set(pathBytes, 4);
      this.data.set(flagsBytes, 4 + pathBytes.length);
      Atomics.store(this.ctrl, IDX_ARG1, -1);
      Atomics.store(this.ctrl, IDX_ARG2, mode);
      Atomics.store(this.ctrl, IDX_DATA_LEN, 4 + pathBytes.length + flagsBytes.length);
    } else {
      const len = this.writeStr(path);
      Atomics.store(this.ctrl, IDX_ARG1, flags);
      Atomics.store(this.ctrl, IDX_ARG2, mode);
      Atomics.store(this.ctrl, IDX_DATA_LEN, len);
    }
    this.call(FsOp.OPEN);
    this.checkError();
    return Atomics.load(this.ctrl, IDX_RESULT);
  }

  closeSync(fd: number): void {
    Atomics.store(this.ctrl, IDX_ARG1, fd);
    Atomics.store(this.ctrl, IDX_DATA_LEN, 0);
    this.call(FsOp.CLOSE);
    this.checkError();
  }

  readSync(fd: number, buffer: Uint8Array, offset: number, length: number, position: number | null): number {
    Atomics.store(this.ctrl, IDX_ARG1, fd);
    Atomics.store(this.ctrl, IDX_ARG2, length);
    if (position === null || position === undefined) {
      Atomics.store(this.ctrl, IDX_ARG3, 0);
      Atomics.store(this.ctrl, IDX_ARG4, -1);
    } else {
      Atomics.store(this.ctrl, IDX_ARG3, position | 0);
      Atomics.store(this.ctrl, IDX_ARG4, Math.floor(position / 0x100000000) | 0);
    }
    Atomics.store(this.ctrl, IDX_DATA_LEN, 0);
    this.call(FsOp.READ);
    this.checkError();
    const bytesRead = Atomics.load(this.ctrl, IDX_RESULT);
    buffer.set(this.data.subarray(0, bytesRead), offset);
    return bytesRead;
  }

  writeSync(fd: number, buffer: Uint8Array, offset: number, length: number, position: number | null): number {
    this.data.set(buffer.subarray(offset, offset + length), 0);
    Atomics.store(this.ctrl, IDX_ARG1, fd);
    if (position === null || position === undefined) {
      Atomics.store(this.ctrl, IDX_ARG2, 0);
      Atomics.store(this.ctrl, IDX_ARG3, -1);
    } else {
      Atomics.store(this.ctrl, IDX_ARG2, position | 0);
      Atomics.store(this.ctrl, IDX_ARG3, Math.floor(position / 0x100000000) | 0);
    }
    Atomics.store(this.ctrl, IDX_DATA_LEN, length);
    this.call(FsOp.WRITE);
    this.checkError();
    return Atomics.load(this.ctrl, IDX_RESULT);
  }

  fstatSync(fd: number, _options?: { bigint: boolean }): BigIntStatsProxy {
    Atomics.store(this.ctrl, IDX_ARG1, fd);
    Atomics.store(this.ctrl, IDX_DATA_LEN, 0);
    this.call(FsOp.FSTAT);
    this.checkError();
    return new BigIntStatsProxy(deserializeStat(this.data, 0));
  }

  statSync(path: string, _options?: { bigint: boolean }): BigIntStatsProxy {
    const len = this.writeStr(path);
    Atomics.store(this.ctrl, IDX_DATA_LEN, len);
    this.call(FsOp.STAT);
    this.checkError();
    return new BigIntStatsProxy(deserializeStat(this.data, 0));
  }

  lstatSync(path: string, _options?: { bigint: boolean }): BigIntStatsProxy {
    const len = this.writeStr(path);
    Atomics.store(this.ctrl, IDX_DATA_LEN, len);
    this.call(FsOp.LSTAT);
    this.checkError();
    return new BigIntStatsProxy(deserializeStat(this.data, 0));
  }

  ftruncateSync(fd: number, len: number): void {
    Atomics.store(this.ctrl, IDX_ARG1, fd);
    Atomics.store(this.ctrl, IDX_ARG2, len);
    Atomics.store(this.ctrl, IDX_DATA_LEN, 0);
    this.call(FsOp.FTRUNCATE);
    this.checkError();
  }

  futimesSync(fd: number, atime: number, mtime: number): void {
    Atomics.store(this.ctrl, IDX_ARG1, fd);
    const tv = new DataView(this.data.buffer, this.data.byteOffset, 16);
    tv.setFloat64(0, atime, true);
    tv.setFloat64(8, mtime, true);
    Atomics.store(this.ctrl, IDX_DATA_LEN, 16);
    this.call(FsOp.FUTIMES);
    this.checkError();
  }

  utimesSync(path: string, atime: number, mtime: number): void {
    const pathLen = this.writeStr(path);
    Atomics.store(this.ctrl, IDX_ARG1, pathLen);
    const tv = new DataView(this.data.buffer, this.data.byteOffset + pathLen, 16);
    tv.setFloat64(0, atime, true);
    tv.setFloat64(8, mtime, true);
    Atomics.store(this.ctrl, IDX_DATA_LEN, pathLen + 16);
    this.call(FsOp.UTIMES);
    this.checkError();
  }

  mkdirSync(path: string): void {
    const len = this.writeStr(path);
    Atomics.store(this.ctrl, IDX_DATA_LEN, len);
    this.call(FsOp.MKDIR);
    this.checkError();
  }

  readdirSync(path: string, options?: { withFileTypes: boolean }): any[] {
    const len = this.writeStr(path);
    Atomics.store(this.ctrl, IDX_ARG1, options?.withFileTypes ? 1 : 0);
    Atomics.store(this.ctrl, IDX_DATA_LEN, len);
    this.call(FsOp.READDIR);
    this.checkError();

    const respLen = Atomics.load(this.ctrl, IDX_DATA_LEN);

    if (options?.withFileTypes) {
      const dv = new DataView(this.data.buffer, this.data.byteOffset);
      let off = 0;
      const count = dv.getUint32(off, true); off += 4;
      const result: DirentProxy[] = [];
      for (let i = 0; i < count; i++) {
        const type = this.data[off++];
        const nameLen = dv.getUint32(off, true); off += 4;
        const name = this.decoder.decode(this.data.slice(off, off + nameLen));
        off += nameLen;
        result.push(new DirentProxy(name, type));
      }
      return result;
    } else {
      const json = this.decoder.decode(this.data.slice(0, respLen));
      return JSON.parse(json);
    }
  }

  renameSync(oldPath: string, newPath: string): void {
    const oldBytes = this.encoder.encode(oldPath);
    const newBytes = this.encoder.encode(newPath);
    const dv = new DataView(this.data.buffer, this.data.byteOffset);
    dv.setUint32(0, oldBytes.length, true);
    this.data.set(oldBytes, 4);
    this.data.set(newBytes, 4 + oldBytes.length);
    Atomics.store(this.ctrl, IDX_DATA_LEN, 4 + oldBytes.length + newBytes.length);
    this.call(FsOp.RENAME);
    this.checkError();
  }

  rmdirSync(path: string): void {
    const len = this.writeStr(path);
    Atomics.store(this.ctrl, IDX_DATA_LEN, len);
    this.call(FsOp.RMDIR);
    this.checkError();
  }

  unlinkSync(path: string): void {
    const len = this.writeStr(path);
    Atomics.store(this.ctrl, IDX_DATA_LEN, len);
    this.call(FsOp.UNLINK);
    this.checkError();
  }

  linkSync(existingPath: string, newPath: string): void {
    const oldBytes = this.encoder.encode(existingPath);
    const newBytes = this.encoder.encode(newPath);
    const dv = new DataView(this.data.buffer, this.data.byteOffset);
    dv.setUint32(0, oldBytes.length, true);
    this.data.set(oldBytes, 4);
    this.data.set(newBytes, 4 + oldBytes.length);
    Atomics.store(this.ctrl, IDX_DATA_LEN, 4 + oldBytes.length + newBytes.length);
    this.call(FsOp.LINK);
    this.checkError();
  }

  symlinkSync(target: string, path: string): void {
    const targetBytes = this.encoder.encode(target);
    const pathBytes = this.encoder.encode(path);
    const dv = new DataView(this.data.buffer, this.data.byteOffset);
    dv.setUint32(0, targetBytes.length, true);
    this.data.set(targetBytes, 4);
    this.data.set(pathBytes, 4 + targetBytes.length);
    Atomics.store(this.ctrl, IDX_DATA_LEN, 4 + targetBytes.length + pathBytes.length);
    this.call(FsOp.SYMLINK);
    this.checkError();
  }

  readlinkSync(path: string): string {
    const len = this.writeStr(path);
    Atomics.store(this.ctrl, IDX_DATA_LEN, len);
    this.call(FsOp.READLINK);
    this.checkError();
    const respLen = Atomics.load(this.ctrl, IDX_DATA_LEN);
    return this.decoder.decode(this.data.slice(0, respLen));
  }

  realpathSync(path: string, _encoding?: string): string {
    const len = this.writeStr(path);
    Atomics.store(this.ctrl, IDX_DATA_LEN, len);
    this.call(FsOp.REALPATH);
    this.checkError();
    const respLen = Atomics.load(this.ctrl, IDX_DATA_LEN);
    return this.decoder.decode(this.data.slice(0, respLen));
  }

  fdatasyncSync(fd: number): void {
    Atomics.store(this.ctrl, IDX_ARG1, fd);
    Atomics.store(this.ctrl, IDX_DATA_LEN, 0);
    this.call(FsOp.FDATASYNC);
    this.checkError();
  }

  fsyncSync(fd: number): void {
    Atomics.store(this.ctrl, IDX_ARG1, fd);
    Atomics.store(this.ctrl, IDX_DATA_LEN, 0);
    this.call(FsOp.FSYNC);
    this.checkError();
  }
}
