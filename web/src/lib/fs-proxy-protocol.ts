/**
 * Shared protocol constants for the filesystem proxy.
 * Imported by both main thread (host) and worker (client).
 */

export enum FsOp {
  OPEN = 1,
  CLOSE,
  READ,
  WRITE,
  FSTAT,
  STAT,
  LSTAT,
  FTRUNCATE,
  FUTIMES,
  UTIMES,
  MKDIR,
  READDIR,
  RENAME,
  RMDIR,
  UNLINK,
  LINK,
  SYMLINK,
  READLINK,
  REALPATH,
  FDATASYNC,
  FSYNC,
}

// Control buffer Int32 indices
export const IDX_REQUEST_FLAG = 0;
export const IDX_RESPONSE_FLAG = 1;
export const IDX_OPCODE = 2;
export const IDX_ARG1 = 3;
export const IDX_ARG2 = 4;
export const IDX_ARG3 = 5;
export const IDX_ARG4 = 6;
export const IDX_RESULT = 7;
export const IDX_ERROR_CODE = 8;
export const IDX_DATA_LEN = 9;

export const CTRL_BUFFER_SIZE = 64 * 4; // 64 Int32s
export const DATA_BUFFER_SIZE = 4 * 1024 * 1024; // 4 MB
export const WAKE_BUFFER_SIZE = 4; // single Int32 shared across all channels

// Error code mapping (Node.js errno strings → numeric)
const ERROR_CODE_MAP: Record<string, number> = {
  EPERM: 1, ENOENT: 2, ESRCH: 3, EINTR: 4, EIO: 5,
  ENXIO: 6, EBADF: 9, EAGAIN: 11, ENOMEM: 12, EACCES: 13,
  EEXIST: 17, ENODEV: 19, ENOTDIR: 20, EISDIR: 21,
  EINVAL: 22, EMFILE: 24, ENOSPC: 28, EROFS: 30,
  ENOTEMPTY: 39, ENOSYS: 38, ELOOP: 40,
};

const CODE_TO_STRING: Record<number, string> = {};
for (const [k, v] of Object.entries(ERROR_CODE_MAP)) {
  CODE_TO_STRING[v] = k;
}

export function errorStringToCode(code: string): number {
  return ERROR_CODE_MAP[code] ?? 255;
}

export function errorCodeToString(code: number): string {
  return CODE_TO_STRING[code] ?? 'EUNKNOWN';
}

// Stat serialization: 128 bytes as Float64Array (16 doubles) + flags byte
export const STAT_SIZE = 15 * 8 + 8; // 128 bytes

export interface StatData {
  dev: bigint; ino: bigint; mode: bigint; nlink: bigint;
  uid: bigint; gid: bigint; rdev: bigint; size: bigint;
  blksize: bigint; blocks: bigint;
  atimeMs: bigint; mtimeMs: bigint; ctimeMs: bigint; birthtimeMs: bigint;
  flags: number; // packed booleans
}

export function serializeStat(stat: any, buf: Uint8Array, offset: number): void {
  const view = new DataView(buf.buffer, buf.byteOffset + offset, STAT_SIZE);
  view.setFloat64(0, Number(stat.dev), true);
  view.setFloat64(8, Number(stat.ino), true);
  view.setFloat64(16, Number(stat.mode), true);
  view.setFloat64(24, Number(stat.nlink), true);
  view.setFloat64(32, Number(stat.uid), true);
  view.setFloat64(40, Number(stat.gid), true);
  view.setFloat64(48, Number(stat.rdev), true);
  view.setFloat64(56, Number(stat.size), true);
  view.setFloat64(64, Number(stat.blksize), true);
  view.setFloat64(72, Number(stat.blocks), true);
  view.setFloat64(80, Number(stat.atimeMs), true);
  view.setFloat64(88, Number(stat.mtimeMs), true);
  view.setFloat64(96, Number(stat.ctimeMs), true);
  view.setFloat64(104, Number(stat.birthtimeMs), true);

  let flags = 0;
  if (stat.isFile()) flags |= 1;
  if (stat.isDirectory()) flags |= 2;
  if (typeof stat.isSymbolicLink === 'function' && stat.isSymbolicLink()) flags |= 4;
  if (typeof stat.isCharacterDevice === 'function' && stat.isCharacterDevice()) flags |= 8;
  if (typeof stat.isBlockDevice === 'function' && stat.isBlockDevice()) flags |= 16;
  if (typeof stat.isSocket === 'function' && stat.isSocket()) flags |= 32;
  if (typeof stat.isFIFO === 'function' && stat.isFIFO()) flags |= 64;
  view.setFloat64(112, flags, true);
}

export function deserializeStat(buf: Uint8Array, offset: number): StatData {
  const view = new DataView(buf.buffer, buf.byteOffset + offset, STAT_SIZE);
  return {
    dev: BigInt(view.getFloat64(0, true)),
    ino: BigInt(view.getFloat64(8, true)),
    mode: BigInt(view.getFloat64(16, true)),
    nlink: BigInt(view.getFloat64(24, true)),
    uid: BigInt(view.getFloat64(32, true)),
    gid: BigInt(view.getFloat64(40, true)),
    rdev: BigInt(view.getFloat64(48, true)),
    size: BigInt(view.getFloat64(56, true)),
    blksize: BigInt(view.getFloat64(64, true)),
    blocks: BigInt(view.getFloat64(72, true)),
    atimeMs: BigInt(view.getFloat64(80, true)),
    mtimeMs: BigInt(view.getFloat64(88, true)),
    ctimeMs: BigInt(view.getFloat64(96, true)),
    birthtimeMs: BigInt(view.getFloat64(104, true)),
    flags: view.getFloat64(112, true),
  };
}
