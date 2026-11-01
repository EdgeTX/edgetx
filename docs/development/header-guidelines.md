# Header Guidelines

A trivial file like `maths.cpp` used to expand to more than 54,000 lines, because `edgetx.h` includes nearly everything. This has three costs:

- **Compile time:** about 84% of compile time is spent in the front end, mostly parsing headers.
- **Rebuilds:** a change to `edgetx.h`, or to anything it includes, rebuilds most of the tree.
- **Coupling:** code can reach anything without saying so, which blurs the separation between hardware and software.

These rules apply to new code. Existing code moves towards them gradually, in small pull requests.

## Rules

### 1. Depend downwards only

From lowest to highest:

1. `os`
2. `hal`, `drivers`
3. `boards`, `targets`
4. Core: data model, mixer, telemetry, storage
5. `gui`

A layer only includes headers from its own level or below. In particular, the data model must not include GUI headers.

### 2. Do not include `edgetx.h` in headers or hardware code

Headers and everything under `os`, `hal`, `drivers`, `boards` and `targets` must not include `edgetx.h`. `tools/check_includes.py` enforces this in CI.

Other new files should avoid it too. If a file only needs a few declarations, move them to a small header, as `maths.h` does.

### 3. Include what you use

Each file includes the headers for what it uses directly, and does not rely on getting them through another header.

### 4. Declare unconditionally

Use `#if` around definitions and struct layouts, not function declarations. A conditional declaration makes every file that sees it differ between radios, even when the file never calls the function, which prevents reusing compiled output.

```cpp
// Avoid
#if defined(FUNCTION_SWITCHES)
void evalFunctionSwitches();
#endif

// Prefer
void evalFunctionSwitches();
```

### 5. Keep widely included headers light

In a header included by many files:

- forward-declare types used only by pointer or reference;
- avoid `<string>`, `<vector>`, `<functional>` and similar if they can stay in the `.cpp`.

### 6. Do not add global `-D` flags

Every file sees a global define. For a new option, use a config header that only the files concerned include, or runtime data from the hardware definitions.

## The `edgetx.h` baseline

`tools/check_includes_baseline.txt` lists the headers and hardware files that still include `edgetx.h`. It can only shrink:

- a new include in a header or in hardware code fails the check;
- once a file is fixed, its entry must be removed. `python3 tools/check_includes.py --update` does this.

Do not use `--update` to add entries.

## Precompiled standard library headers

WASM builds precompile the headers in `radio/src/targets/simu/pch_std.h`, and force-include them in every C++ file. A file that forgets `#include <string>` can therefore still compile for WASM, but the firmware build will catch it.

Only add standard library headers to that file. `USE_PCH=OFF` turns it off.
