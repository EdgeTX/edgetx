# Vendored MSYS2 packages

The Windows Companion build (`.github/workflows/win_cpn-64.yml`) installs
these with `pacman -U`, because they are no longer in MSYS2's `mingw64`
package database.

## mingw-w64-x86_64-dfu-util-0.11-2-any.pkg.tar.zst

- Source: https://repo.msys2.org/mingw/mingw64/ (built by MSYS2 CI
  2025-05-31, signed 2025-06-02). The detached signature
  (`.pkg.tar.zst.sig`) is kept alongside, so `pacman -U` can check it
  against the runner's MSYS2 keyring.
- SHA-256: `950652382f6632ac4b7e42d39d0cda19ef593f1a490c56c814e9f65a5aabe89c`
- Why vendored: MSYS2 dropped `dfu-util` from the `mingw64` environment
  (it's still packaged for `ucrt64`, `clang64` and `clangarm64`), so
  `pacman -S mingw-w64-x86_64-dfu-util` fails with "target not found".
  Companion bundles `dfu-util.exe` and `libusb-1.0.dll` into the Windows
  installer for DFU flashing. 2.12 and later don't need it, because they
  use `rs_dfu` (#6521).
- Dependencies: `mingw-w64-x86_64-libusb` and
  `mingw-w64-x86_64-libwinpthread`, both still in the `mingw64` database.
- Licence: GPL2 (per the package metadata). Upstream source:
  https://dfu-util.sourceforge.net/
