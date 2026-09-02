/**
 * OPFS-backed filesystem for the FS Worker.
 *
 * Maintains a lightweight in-memory directory index (metadata only) for
 * synchronous path resolution and readdir.  File I/O uses OPFS
 * SyncAccessHandles for synchronous read/write without any memfs layer.
 *
 * Async operations (open, mkdir, unlink, rmdir, rename) need OPFS directory
 * mutations which are inherently async.  The FS Worker handles these by
 * briefly breaking out of its synchronous Atomics loop.
 */

// --- Directory index node types ---

interface FileNode {
  name: string;
  kind: 'file';
  handle: FileSystemFileHandle;
  size: number;
  mtime: number;
}

interface DirNode {
  name: string;
  kind: 'directory';
  handle: FileSystemDirectoryHandle;
  children: Map<string, FileNode | DirNode>;
  mtime: number;
}

type FsNode = FileNode | DirNode;

// --- Open file tracking ---

interface OpenFlags {
  read: boolean;
  write: boolean;
  append: boolean;
  create: boolean;
  truncate: boolean;
  exclusive: boolean;
}

/** Shared SyncAccessHandle with refcounting for multiple fds on the same file. */
interface SharedHandle {
  syncHandle: FileSystemSyncAccessHandle;
  node: FileNode;
  refCount: number;
}

interface OpenFd {
  /** Shared handle (null for directory fds). */
  shared: SharedHandle | null;
  /** The filesystem node (file or directory). */
  node: FsNode;
  /** Current read/write position. */
  position: number;
  flags: OpenFlags;
}

// --- Flag parsing ---

function parseOpenFlags(flags: number | string): OpenFlags {
  if (typeof flags === 'string') {
    const f = flags;
    const plus = f.includes('+');
    return {
      read: f[0] === 'r' || plus,
      write: f[0] === 'w' || f[0] === 'a' || plus,
      append: f[0] === 'a',
      create: f[0] === 'w' || f[0] === 'a',
      truncate: f[0] === 'w' && !plus,
      exclusive: f.includes('x'),
    };
  }
  // Numeric flags (Node.js / Linux O_* values)
  const access = flags & 3;
  return {
    read: access === 0 || access === 2,   // O_RDONLY=0, O_RDWR=2
    write: access === 1 || access === 2,  // O_WRONLY=1, O_RDWR=2
    append: !!(flags & 0o2000),           // O_APPEND
    create: !!(flags & 0o100),            // O_CREAT
    truncate: !!(flags & 0o1000),         // O_TRUNC
    exclusive: !!(flags & 0o200),         // O_EXCL
  };
}

// --- Stat result (compatible with serializeStat in fs-proxy-protocol) ---

class StatResult {
  dev = 0;
  ino: number;
  mode: number;
  nlink = 1;
  uid = 0;
  gid = 0;
  rdev = 0;
  size: number;
  blksize = 4096;
  blocks: number;
  atimeMs: number;
  mtimeMs: number;
  ctimeMs: number;
  birthtimeMs: number;
  private _isFile: boolean;

  constructor(isFile: boolean, size: number, mtime: number, ino: number) {
    this._isFile = isFile;
    this.ino = ino;
    this.mode = isFile ? 0o100644 : 0o040755;
    this.size = size;
    this.blocks = Math.ceil(size / 512);
    this.atimeMs = mtime;
    this.mtimeMs = mtime;
    this.ctimeMs = mtime;
    this.birthtimeMs = mtime;
  }

  isFile() { return this._isFile; }
  isDirectory() { return !this._isFile; }
  isSymbolicLink() { return false; }
  isCharacterDevice() { return false; }
  isBlockDevice() { return false; }
  isSocket() { return false; }
  isFIFO() { return false; }
}

// --- Backend ---

export class OpfsBackend {
  private root: DirNode | null = null;
  private fdTable = new Map<number, OpenFd>();
  /** Path → shared SyncAccessHandle for refcounting across multiple fds. */
  private openHandles = new Map<string, SharedHandle>();
  private nextFd = 3; // 0-2 reserved for stdin/stdout/stderr
  private nextIno = 1;

  /**
   * Initialize: get the OPFS directory for this radio and build the
   * in-memory directory index by scanning the OPFS tree.
   */
  async init(radioKey: string): Promise<void> {
    const storageRoot = await navigator.storage.getDirectory();
    const edgetx = await storageRoot.getDirectoryHandle('edgetx-web', { create: true });
    const radioDir = await edgetx.getDirectoryHandle(radioKey, { create: true });

    this.root = {
      name: '',
      kind: 'directory',
      handle: radioDir,
      children: new Map(),
      mtime: Date.now(),
    };

    await this.scanDir(this.root);
  }

  /** Whether the OPFS tree has any content for this radio. */
  get hasContent(): boolean {
    return this.root !== null && this.root.children.size > 0;
  }

  /** Close all open handles (for shutdown). */
  closeAll(): void {
    for (const [fd] of this.fdTable) {
      try { this.close(fd); } catch { /* best effort */ }
    }
  }

  // --- Async scan ---

  private async scanDir(dir: DirNode): Promise<void> {
    for await (const [name, handle] of (dir.handle as any).entries()) {
      if (handle.kind === 'directory') {
        const child: DirNode = {
          name,
          kind: 'directory',
          handle: handle as FileSystemDirectoryHandle,
          children: new Map(),
          mtime: Date.now(),
        };
        dir.children.set(name, child);
        await this.scanDir(child);
      } else {
        const file = await (handle as FileSystemFileHandle).getFile();
        const child: FileNode = {
          name,
          kind: 'file',
          handle: handle as FileSystemFileHandle,
          size: file.size,
          mtime: file.lastModified,
        };
        dir.children.set(name, child);
      }
    }
  }

  // --- Synchronous path resolution ---

  private normalizePath(path: string): string {
    return '/' + path.split('/').filter(Boolean).join('/');
  }

  private resolve(path: string): FsNode {
    if (!this.root) throw this.err('ENOENT', path);

    const segments = path.split('/').filter(Boolean);
    let node: FsNode = this.root;

    for (const seg of segments) {
      if (seg === '.') continue;
      if (seg === '..') throw this.err('EINVAL', path);
      if (node.kind !== 'directory') throw this.err('ENOTDIR', path);
      const child = node.children.get(seg);
      if (!child) throw this.err('ENOENT', path);
      node = child;
    }

    return node;
  }

  private resolveParent(path: string): { parent: DirNode; name: string } {
    if (!this.root) throw this.err('ENOENT', path);

    const segments = path.split('/').filter(Boolean);
    if (segments.length === 0) throw this.err('EINVAL', path);

    const name = segments.pop()!;
    let node: FsNode = this.root;

    for (const seg of segments) {
      if (seg === '.') continue;
      if (seg === '..') throw this.err('EINVAL', path);
      if (node.kind !== 'directory') throw this.err('ENOTDIR', path);
      const child = node.children.get(seg);
      if (!child) throw this.err('ENOENT', path);
      node = child;
    }

    if (node.kind !== 'directory') throw this.err('ENOTDIR', path);
    return { parent: node, name };
  }

  // --- File operations ---

  /** Open a file or directory.  ASYNC (needs createSyncAccessHandle). */
  async open(path: string, flags: number | string, _mode: number): Promise<number> {
    const parsed = parseOpenFlags(flags);
    const normPath = this.normalizePath(path);

    let node: FsNode;
    let created = false;

    try {
      node = this.resolve(path);
    } catch (e: any) {
      if (e.code === 'ENOENT' && parsed.create) {
        node = await this.createFile(path);
        created = true;
      } else {
        throw e;
      }
    }

    if (!created && parsed.create && parsed.exclusive) {
      throw this.err('EEXIST', path);
    }

    // Directory fd: no SyncAccessHandle, just a trackable fd for WASI
    if (node.kind === 'directory') {
      const fd = this.nextFd++;
      this.fdTable.set(fd, { shared: null, node, position: 0, flags: parsed });
      return fd;
    }

    // File fd: reuse existing SyncAccessHandle if already open (refcounting)
    let shared = this.openHandles.get(normPath);
    if (shared) {
      shared.refCount++;
    } else {
      const syncHandle = await (node as FileNode).handle.createSyncAccessHandle();
      shared = { syncHandle, node: node as FileNode, refCount: 1 };
      this.openHandles.set(normPath, shared);
    }

    if (parsed.truncate) {
      shared.syncHandle.truncate(0);
      shared.node.size = 0;
      shared.node.mtime = Date.now();
    }

    const fd = this.nextFd++;
    this.fdTable.set(fd, {
      shared,
      node,
      position: parsed.append ? shared.syncHandle.getSize() : 0,
      flags: parsed,
    });

    return fd;
  }

  /** Close a file descriptor.  Sync. */
  close(fd: number): void {
    const openFd = this.fdTable.get(fd);
    if (!openFd) throw this.err('EBADF', `fd ${fd}`);

    if (openFd.shared) {
      openFd.shared.refCount--;
      if (openFd.shared.refCount <= 0) {
        openFd.shared.syncHandle.flush();
        openFd.shared.node.size = openFd.shared.syncHandle.getSize();
        openFd.shared.syncHandle.close();
        // Remove from openHandles
        for (const [path, h] of this.openHandles) {
          if (h === openFd.shared) {
            this.openHandles.delete(path);
            break;
          }
        }
      }
    }

    this.fdTable.delete(fd);
  }

  /** Read from an open file.  Sync. */
  read(fd: number, length: number, position: number | null): { data: Uint8Array; bytesRead: number } {
    const openFd = this.getFd(fd, 'file');
    const pos = position ?? openFd.position;
    const buf = new Uint8Array(length);
    const bytesRead = openFd.shared!.syncHandle.read(buf, { at: pos });

    if (position === null) {
      openFd.position += bytesRead;
    }

    return { data: buf, bytesRead };
  }

  /** Write to an open file.  Sync. */
  write(fd: number, data: Uint8Array, position: number | null): number {
    const openFd = this.getFd(fd, 'file');
    const sh = openFd.shared!;

    let pos: number;
    if (openFd.flags.append) {
      pos = sh.syncHandle.getSize();
    } else {
      pos = position ?? openFd.position;
    }

    const written = sh.syncHandle.write(data, { at: pos });

    if (position === null || openFd.flags.append) {
      openFd.position = pos + written;
    }

    sh.node.size = sh.syncHandle.getSize();
    sh.node.mtime = Date.now();
    return written;
  }

  /** Get file status by fd.  Sync. */
  fstat(fd: number): StatResult {
    const openFd = this.fdTable.get(fd);
    if (!openFd) throw this.err('EBADF', `fd ${fd}`);

    if (openFd.node.kind === 'directory') {
      return new StatResult(false, 0, openFd.node.mtime, this.nextIno++);
    }

    const size = openFd.shared ? openFd.shared.syncHandle.getSize() : (openFd.node as FileNode).size;
    return new StatResult(true, size, openFd.node.mtime, this.nextIno++);
  }

  /** Get file status by path.  Sync. */
  stat(path: string): StatResult {
    const node = this.resolve(path);
    const size = node.kind === 'file' ? node.size : 0;
    return new StatResult(node.kind === 'file', size, node.mtime, this.nextIno++);
  }

  /** Same as stat (no symlink support).  Sync. */
  lstat(path: string): StatResult {
    return this.stat(path);
  }

  /** Truncate an open file.  Sync. */
  ftruncate(fd: number, len: number): void {
    const openFd = this.getFd(fd, 'file');
    openFd.shared!.syncHandle.truncate(len);
    openFd.shared!.node.size = len;
    openFd.shared!.node.mtime = Date.now();
  }

  /** Update timestamps by fd.  Sync (in-memory only). */
  futimes(fd: number, _atime: number, mtime: number): void {
    const openFd = this.fdTable.get(fd);
    if (!openFd) throw this.err('EBADF', `fd ${fd}`);
    openFd.node.mtime = mtime;
  }

  /** Update timestamps by path.  Sync (in-memory only). */
  utimes(path: string, _atime: number, mtime: number): void {
    const node = this.resolve(path);
    node.mtime = mtime;
  }

  /** Create a directory.  ASYNC. */
  async mkdir(path: string): Promise<void> {
    const { parent, name } = this.resolveParent(path);

    if (parent.children.has(name)) {
      throw this.err('EEXIST', path);
    }

    const handle = await parent.handle.getDirectoryHandle(name, { create: true });
    parent.children.set(name, {
      name,
      kind: 'directory',
      handle,
      children: new Map(),
      mtime: Date.now(),
    });
  }

  /** List directory contents.  Sync (reads from in-memory index). */
  readdir(path: string, withFileTypes: boolean): any[] {
    const node = this.resolve(path);
    if (node.kind !== 'directory') throw this.err('ENOTDIR', path);

    if (withFileTypes) {
      return Array.from(node.children.values()).map(child => ({
        name: child.name,
        isFile: () => child.kind === 'file',
        isDirectory: () => child.kind === 'directory',
        isSymbolicLink: () => false,
        isCharacterDevice: () => false,
        isBlockDevice: () => false,
        isSocket: () => false,
        isFIFO: () => false,
      }));
    }
    return Array.from(node.children.keys());
  }

  /** Rename / move a file or directory.  ASYNC. */
  async rename(oldPath: string, newPath: string): Promise<void> {
    const oldNode = this.resolve(oldPath);
    const { parent: oldParent, name: oldName } = this.resolveParent(oldPath);
    const { parent: newParent, name: newName } = this.resolveParent(newPath);

    // Reject rename while file is open (SyncAccessHandle holds a lock)
    const normOld = this.normalizePath(oldPath);
    if (this.openHandles.has(normOld)) {
      throw this.err('EBUSY', oldPath);
    }

    // Remove existing target if present
    if (newParent.children.has(newName)) {
      const existing = newParent.children.get(newName)!;
      if (existing.kind === 'directory' && existing.children.size > 0) {
        throw this.err('ENOTEMPTY', newPath);
      }
      await newParent.handle.removeEntry(newName, { recursive: false });
      newParent.children.delete(newName);
    }

    // Use move() API if available (Chrome 110+, Firefox 129+)
    if (typeof (oldNode.handle as any).move === 'function') {
      await (oldNode.handle as any).move(newParent.handle, newName);
      oldParent.children.delete(oldName);
      oldNode.name = newName;
      newParent.children.set(newName, oldNode);
    } else if (oldNode.kind === 'file') {
      // Fallback: read content, create new file, delete old
      const file = await oldNode.handle.getFile();
      const content = new Uint8Array(await file.arrayBuffer());

      const newHandle = await newParent.handle.getFileHandle(newName, { create: true });
      const syncHandle = await newHandle.createSyncAccessHandle();
      syncHandle.write(content);
      syncHandle.flush();
      const newSize = syncHandle.getSize();
      syncHandle.close();

      await oldParent.handle.removeEntry(oldName);
      oldParent.children.delete(oldName);

      newParent.children.set(newName, {
        name: newName,
        kind: 'file',
        handle: newHandle,
        size: newSize,
        mtime: Date.now(),
      });
    } else {
      throw this.err('ENOSYS', 'directory rename requires move() API');
    }
  }

  /** Remove an empty directory.  ASYNC. */
  async rmdir(path: string): Promise<void> {
    const { parent, name } = this.resolveParent(path);
    const child = parent.children.get(name);
    if (!child) throw this.err('ENOENT', path);
    if (child.kind !== 'directory') throw this.err('ENOTDIR', path);
    if (child.children.size > 0) throw this.err('ENOTEMPTY', path);

    await parent.handle.removeEntry(name);
    parent.children.delete(name);
  }

  /** Delete a file.  ASYNC. */
  async unlink(path: string): Promise<void> {
    const { parent, name } = this.resolveParent(path);
    const child = parent.children.get(name);
    if (!child) throw this.err('ENOENT', path);
    if (child.kind === 'directory') throw this.err('EISDIR', path);

    // Force-close any open SyncAccessHandle for this file
    const normPath = this.normalizePath(path);
    const existing = this.openHandles.get(normPath);
    if (existing) {
      existing.syncHandle.close();
      this.openHandles.delete(normPath);
    }

    await parent.handle.removeEntry(name);
    parent.children.delete(name);
  }

  // Unsupported (OPFS has no symlinks / hardlinks)
  link(): never { throw this.err('ENOSYS', 'hardlinks not supported'); }
  symlink(): never { throw this.err('ENOSYS', 'symlinks not supported'); }
  readlink(): never { throw this.err('ENOSYS', 'symlinks not supported'); }

  /** Normalize path and verify it exists.  Sync. */
  realpath(path: string): string {
    this.resolve(path);
    return this.normalizePath(path);
  }

  /** Flush file data to OPFS.  Sync. */
  fdatasync(fd: number): void {
    const openFd = this.getFd(fd, 'file');
    openFd.shared!.syncHandle.flush();
  }

  /** Flush file data + metadata.  Sync. */
  fsync(fd: number): void {
    this.fdatasync(fd);
  }

  // --- Bulk operations (for main-thread UI) ---

  /** Recursively delete all content under the OPFS root.  ASYNC. */
  async wipe(): Promise<void> {
    if (!this.root) return;
    this.closeAll();
    await this.clearDir(this.root);
  }

  /** Write a file into the OPFS tree, creating parent directories as needed.  ASYNC. */
  async writeFile(path: string, data: Uint8Array): Promise<void> {
    // Ensure parent directories exist
    const segments = path.split('/').filter(Boolean);
    let dir = this.root!;
    for (let i = 0; i < segments.length - 1; i++) {
      const seg = segments[i];
      let child = dir.children.get(seg);
      if (!child) {
        const handle = await dir.handle.getDirectoryHandle(seg, { create: true });
        child = { name: seg, kind: 'directory', handle, children: new Map(), mtime: Date.now() };
        dir.children.set(seg, child);
      }
      if (child.kind !== 'directory') throw this.err('ENOTDIR', path);
      dir = child;
    }

    const fileName = segments[segments.length - 1];
    let fileNode = dir.children.get(fileName);
    let fileHandle: FileSystemFileHandle;

    if (fileNode && fileNode.kind === 'file') {
      fileHandle = fileNode.handle;
    } else {
      fileHandle = await dir.handle.getFileHandle(fileName, { create: true });
    }

    const syncHandle = await fileHandle.createSyncAccessHandle();
    syncHandle.truncate(0);
    syncHandle.write(data);
    syncHandle.flush();
    const size = syncHandle.getSize();
    syncHandle.close();

    dir.children.set(fileName, {
      name: fileName,
      kind: 'file',
      handle: fileHandle,
      size,
      mtime: Date.now(),
    });
  }

  /** Read a file's full contents.  ASYNC. */
  async readFile(path: string): Promise<Uint8Array> {
    const node = this.resolve(path);
    if (node.kind !== 'file') throw this.err('EISDIR', path);

    const file = await node.handle.getFile();
    return new Uint8Array(await file.arrayBuffer());
  }

  /** Recursively list all file paths under a directory.  Sync. */
  listFiles(basePath = '/'): string[] {
    const result: string[] = [];
    const node = this.resolve(basePath);
    if (node.kind === 'directory') {
      this.listFilesRec(basePath, node, result);
    }
    return result;
  }

  // --- Private helpers ---

  private async createFile(path: string): Promise<FileNode> {
    const { parent, name } = this.resolveParent(path);
    const handle = await parent.handle.getFileHandle(name, { create: true });
    const node: FileNode = {
      name,
      kind: 'file',
      handle,
      size: 0,
      mtime: Date.now(),
    };
    parent.children.set(name, node);
    return node;
  }

  private async clearDir(dir: DirNode): Promise<void> {
    const names = Array.from(dir.children.keys());
    for (const name of names) {
      await dir.handle.removeEntry(name, { recursive: true });
    }
    dir.children.clear();
  }

  private listFilesRec(basePath: string, dir: DirNode, result: string[]): void {
    for (const child of dir.children.values()) {
      const childPath = basePath === '/' ? `/${child.name}` : `${basePath}/${child.name}`;
      if (child.kind === 'directory') {
        this.listFilesRec(childPath, child, result);
      } else {
        result.push(childPath);
      }
    }
  }

  /** Get an open fd, throwing EBADF or EISDIR as appropriate. */
  private getFd(fd: number, expect: 'file'): OpenFd & { shared: SharedHandle };
  private getFd(fd: number, expect?: string): OpenFd;
  private getFd(fd: number, expect?: string): OpenFd {
    const openFd = this.fdTable.get(fd);
    if (!openFd) throw this.err('EBADF', `fd ${fd}`);
    if (expect === 'file' && !openFd.shared) throw this.err('EISDIR', `fd ${fd}`);
    return openFd;
  }

  private err(code: string, detail: string): Error {
    const e = new Error(`${code}: ${detail}`);
    (e as any).code = code;
    return e;
  }
}
