#!/bin/bash
#
# Build edgetx-luac.wasm — a browser-ready WASM module that compiles
# Lua source to EdgeTX-compatible bytecode entirely in memory.
#
# Requirements: wasi-sdk (default /opt/wasi-sdk)
#
# Usage:
#   cd radio/src/thirdparty/Lua
#   ./build-wasm.sh
#
# The output edgetx-luac.wasm is placed in the current directory.

set -euo pipefail

WASI_SDK="${WASI_SDK:-/opt/wasi-sdk}"
CC="${WASI_SDK}/bin/clang"

if [ ! -x "$CC" ]; then
  echo "Error: wasi-sdk clang not found at $CC" >&2
  echo "Set WASI_SDK to your wasi-sdk installation path." >&2
  exit 1
fi

SRCDIR="$(cd "$(dirname "$0")/src" && pwd)"

LUA_CORE_SRC=(
  "$SRCDIR/lapi.c"
  "$SRCDIR/lcode.c"
  "$SRCDIR/lctype.c"
  "$SRCDIR/ldebug.c"
  "$SRCDIR/ldo.c"
  "$SRCDIR/ldump.c"
  "$SRCDIR/lfunc.c"
  "$SRCDIR/lgc.c"
  "$SRCDIR/llex.c"
  "$SRCDIR/lmem.c"
  "$SRCDIR/lobject.c"
  "$SRCDIR/lopcodes.c"
  "$SRCDIR/lparser.c"
  "$SRCDIR/lstate.c"
  "$SRCDIR/lstring.c"
  "$SRCDIR/ltable.c"
  "$SRCDIR/ltm.c"
  "$SRCDIR/lundump.c"
  "$SRCDIR/lvm.c"
  "$SRCDIR/lzio.c"
  "$SRCDIR/lauxlib.c"
)

echo "Building edgetx-luac.wasm..."

"$CC" \
  --target=wasm32-wasi \
  --sysroot="${WASI_SDK}/share/wasi-sysroot" \
  -Os \
  -ffunction-sections \
  -fdata-sections \
  -mllvm -wasm-enable-sjlj \
  -DLUA_HOST_BUILD \
  -DLUA_CROSS_COMPILER \
  -DLUA_COMPAT_5_2 \
  -I"$SRCDIR" \
  -mexec-model=reactor \
  -Wl,--gc-sections \
  -Wl,--strip-debug \
  -Wl,--export-dynamic \
  -Wl,-mllvm,-wasm-enable-sjlj \
  -nodefaultlibs -lc -lsetjmp \
  -o edgetx-luac.wasm \
  "$SRCDIR/luac_wasm.c" \
  "${LUA_CORE_SRC[@]}"

SIZE=$(wc -c < edgetx-luac.wasm | tr -d ' ')
echo "Done: edgetx-luac.wasm (${SIZE} bytes)"

# Show WASI imports (ideally very few / none needed at runtime)
if command -v wasm-objdump &>/dev/null; then
  echo ""
  echo "WASI imports:"
  wasm-objdump -j Import -x edgetx-luac.wasm 2>/dev/null | grep wasi || echo "  (none)"
fi
