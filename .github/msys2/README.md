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

## mingw-w64-x86_64-python-lz4-4.4.5-3-any.pkg.tar.zst

- Source: https://repo.msys2.org/mingw/mingw64/ (built by MSYS2 CI
  2026-01-26, signed 2026-01-28). The detached signature
  (`.pkg.tar.zst.sig`) is kept alongside, so `pacman -U` can check it
  against the runner's MSYS2 keyring. It is the newest mingw64 build
  still on the server, built after MSYS2 moved to Python 3.14.
- SHA-256: `ad1408fa5076be10d11fda783541d3d45571cac487b7f1b865362f77c3a3e59b`
- Why vendored: MSYS2 dropped `python-lz4` from the `mingw64`
  environment (it's still packaged for `ucrt64`, `clang64` and
  `clangarm64`), so `pacman -S mingw-w64-x86_64-python-lz4` fails with
  "target not found". `radio/util/encode-bitmap.py` imports `lz4.block`
  to compress the colour LCD bitmaps, so the build can't run without it.
  The workflow checks `import lz4.block` right after installing it.
- Dependencies: `mingw-w64-x86_64-python` (still in the `mingw64`
  database). Its PKGBUILD builds against `mingw-w64-x86_64-lz4` through
  pkg-config, so that library (still in the `mingw64` database) is
  installed with `pacman -S` in case the extension links to `liblz4.dll`.
- Licence: BSD-3-Clause (per the PKGBUILD). Upstream source:
  https://github.com/python-lz4/python-lz4
- If it stops working (for example after a Python minor-version change
  in MSYS2), the fallback is `python -m pip install lz4`, built from
  source with the MSYS2 toolchain.
