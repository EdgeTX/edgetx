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
- Dependencies: `mingw-w64-x86_64-libusb` (vendored below, since it's no
  longer in the `mingw64` database either) and
  `mingw-w64-x86_64-libwinpthread` (still in the `mingw64` database).
- Licence: GPL2 (per the package metadata). Upstream source:
  https://dfu-util.sourceforge.net/

## mingw-w64-x86_64-libusb-1.0.30-1-any.pkg.tar.zst

- Source: https://repo.msys2.org/mingw/mingw64/ (built by MSYS2 CI
  2026-07-08, signed 2026-07-09). The detached signature
  (`.pkg.tar.zst.sig`) is kept alongside, so `pacman -U` can check it
  against the runner's MSYS2 keyring. It is the newest mingw64 libusb
  build still on the server.
- SHA-256: `124edc26b009678a4c5e759481a364abf63ed7f2f49a53092e8bd0a47c0c0740`
- Why vendored: MSYS2 dropped `libusb` from the `mingw64` environment
  too, so `pacman -S mingw-w64-x86_64-libusb` fails with "target not
  found". The vendored dfu-util depends on it, and Companion only bundles
  `dfu-util.exe` and `libusb-1.0.dll` into the Windows installer when
  CMake finds libusb.
- Dependencies: none.
- Licence: LGPL-2.1 (per the package metadata). Upstream source:
  https://libusb.info/
