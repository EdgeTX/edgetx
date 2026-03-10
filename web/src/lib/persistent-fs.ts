/**
 * OPFS-backed persistent filesystem for the web simulator.
 *
 * Runtime I/O goes through memfs (in-memory Volume).  At startup the OPFS
 * tree is loaded into memfs; on shutdown (or periodic save) the memfs tree
 * is flushed back to OPFS.
 */

import { Buffer } from 'buffer';
import { Volume, createFsFromVolume } from 'memfs-browser';

export class PersistentFS {
  /** The memfs volume used at runtime by WASI. */
  readonly volume: ReturnType<typeof Volume.fromJSON>;
  /** Node-style fs backed by the volume. */
  readonly fs: ReturnType<typeof createFsFromVolume>;
  /** OPFS root directory for this radio. */
  private opfsRoot: FileSystemDirectoryHandle | null = null;

  private radioKey: string;

  constructor(radioKey: string) {
    this.radioKey = radioKey;
    this.volume = Volume.fromJSON({ '/': null });
    this.fs = createFsFromVolume(this.volume) as any;
  }

  /** Derive a short radio key from a wasm filename. */
  static radioKeyFromWasm(wasmFile: string): string {
    // "edgetx-tx16s-simulator.wasm" → "tx16s"
    const base = wasmFile.replace(/.*\//, '').replace(/\.wasm$/, '');
    const m = base.match(/^edgetx-(.+)-simulator$/);
    return m ? m[1] : base;
  }

  /** Load OPFS contents into memfs. Returns true if data existed. */
  async load(): Promise<boolean> {
    try {
      const root = await navigator.storage.getDirectory();
      const edgetx = await root.getDirectoryHandle('edgetx-web', { create: true });
      this.opfsRoot = await edgetx.getDirectoryHandle(this.radioKey, { create: true });
    } catch {
      // OPFS not available — run without persistence
      return false;
    }

    let found = false;
    await this.loadDir(this.opfsRoot!, '/');

    // Check if we loaded anything beyond the root
    try {
      const entries = this.fs.readdirSync('/');
      found = entries.length > 0;
    } catch {
      // empty
    }

    return found;
  }

  private async loadDir(dirHandle: FileSystemDirectoryHandle, memfsPath: string): Promise<void> {
    for await (const [name, handle] of (dirHandle as any).entries()) {
      const childPath = memfsPath === '/' ? `/${name}` : `${memfsPath}/${name}`;
      if (handle.kind === 'directory') {
        try { this.fs.mkdirSync(childPath); } catch { /* exists */ }
        await this.loadDir(handle as FileSystemDirectoryHandle, childPath);
      } else {
        const file = await (handle as FileSystemFileHandle).getFile();
        const buf = await file.arrayBuffer();
        this.fs.writeFileSync(childPath, Buffer.from(buf));
      }
    }
  }

  /** Flush memfs contents to OPFS. */
  async save(): Promise<void> {
    if (!this.opfsRoot) return;

    // Clear OPFS tree first, then write current memfs state
    await this.clearDir(this.opfsRoot);
    await this.saveDir('/', this.opfsRoot);
  }

  private async clearDir(dirHandle: FileSystemDirectoryHandle): Promise<void> {
    const names: string[] = [];
    for await (const [name] of (dirHandle as any).entries()) {
      names.push(name);
    }
    for (const name of names) {
      await dirHandle.removeEntry(name, { recursive: true });
    }
  }

  private async saveDir(memfsPath: string, dirHandle: FileSystemDirectoryHandle): Promise<void> {
    let entries: string[];
    try {
      entries = this.fs.readdirSync(memfsPath) as string[];
    } catch {
      return;
    }

    for (const name of entries) {
      const childPath = memfsPath === '/' ? `/${name}` : `${memfsPath}/${name}`;
      let stat;
      try {
        stat = this.fs.statSync(childPath);
      } catch {
        continue;
      }

      if (stat.isDirectory()) {
        const subDir = await dirHandle.getDirectoryHandle(name, { create: true });
        await this.saveDir(childPath, subDir);
      } else {
        const fileHandle = await dirHandle.getFileHandle(name, { create: true });
        const writable = await fileHandle.createWritable();
        const data = this.fs.readFileSync(childPath);
        await writable.write(new Uint8Array(data as Buffer));
        await writable.close();
      }
    }
  }

  /** Delete all OPFS data for this radio. */
  async wipe(): Promise<void> {
    if (!this.opfsRoot) return;
    await this.clearDir(this.opfsRoot);

    // Also clear the memfs volume
    this.clearMemfs('/');
  }

  private clearMemfs(path: string): void {
    let entries: string[];
    try {
      entries = this.fs.readdirSync(path) as string[];
    } catch {
      return;
    }
    for (const name of entries) {
      const childPath = path === '/' ? `/${name}` : `${path}/${name}`;
      let stat;
      try {
        stat = this.fs.statSync(childPath);
      } catch {
        continue;
      }
      if (stat.isDirectory()) {
        this.clearMemfs(childPath);
        try { this.fs.rmdirSync(childPath); } catch { /* ignore */ }
      } else {
        try { this.fs.unlinkSync(childPath); } catch { /* ignore */ }
      }
    }
  }

  /** Download the entire filesystem as a zip-like tar (or individual file). */
  async downloadFile(memfsPath: string): Promise<Blob | null> {
    try {
      const data = this.fs.readFileSync(memfsPath);
      return new Blob([new Uint8Array(data as Buffer)]);
    } catch {
      return null;
    }
  }

  /** Upload a file into memfs at the given path. */
  uploadFile(memfsPath: string, data: ArrayBuffer): void {
    // Ensure parent directories exist
    const parts = memfsPath.split('/').filter(Boolean);
    let dir = '/';
    for (let i = 0; i < parts.length - 1; i++) {
      dir = dir === '/' ? `/${parts[i]}` : `${dir}/${parts[i]}`;
      try { this.fs.mkdirSync(dir); } catch { /* exists */ }
    }
    this.fs.writeFileSync(memfsPath, Buffer.from(data));
  }

  /** List files recursively for display. */
  listFiles(basePath = '/'): string[] {
    const result: string[] = [];
    this.listFilesRec(basePath, result);
    return result;
  }

  private listFilesRec(path: string, result: string[]): void {
    let entries: string[];
    try {
      entries = this.fs.readdirSync(path) as string[];
    } catch {
      return;
    }
    for (const name of entries) {
      const childPath = path === '/' ? `/${name}` : `${path}/${name}`;
      let stat;
      try {
        stat = this.fs.statSync(childPath);
      } catch {
        continue;
      }
      if (stat.isDirectory()) {
        this.listFilesRec(childPath, result);
      } else {
        result.push(childPath);
      }
    }
  }
}
