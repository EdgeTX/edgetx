# Simulator UI Automation Phase 0 Baseline

**Recorded:** 2026-08-17

**Base commit:** `0c07a3d648a929732190ab9b7378a7f6c272ab55`

**Target:** RadioMaster TX16S (`PCB=X10`, `PCBREV=TX16S`,
`DEFAULT_MODE=2`, Release unless a maintained script selects Debug)

This record is the reproducible evidence for Phase 0 of the
[unified simulator UI automation plan](simulator-ui-automation-plan.md). It
separates technical baseline completion from design acceptance.

## 1. Gate result

| Gate | State | Evidence and consequence |
|---|---|---|
| Technical exit 0T | **Passed** | Source, submodules, tools, commands, tests, artifacts, timings, limitations, and provenance are recorded below. |
| Contract gate G0 | **Pending** | D01–D19 remain `Proposed`; there is no contributor or maintainer acceptance to record. |
| Phase 1 implementation | **Blocked by G0** | Do not interpret passing builds or silence as protocol approval. |

The consolidation invitation was posted to
[#7337](https://github.com/EdgeTX/edgetx/pull/7337#issuecomment-5318333983)
and the overlap response was posted to
[#7646](https://github.com/EdgeTX/edgetx/pull/7646#issuecomment-5318333877).
As of 2026-08-17T19:37:51Z, #7337 had no human reply after that invitation and
[#7668](https://github.com/EdgeTX/edgetx/pull/7668) had no comments or reviews.
The earlier maintainer comment on #7646 identified substantial overlap with
#7337; it did not approve this replacement contract. Approval for any
`Co-authored-by` trailer for `onliner10` also remains pending.

## 2. Source manifest

| Item | Pinned value |
|---|---|
| Repository | `https://github.com/EdgeTX/edgetx.git` |
| Fetch window, UTC | `2026-08-17T19:28:11.7256807Z` to `2026-08-17T19:28:12.6045567Z` |
| `upstream/main` at fetch | `0c07a3d648a929732190ab9b7378a7f6c272ab55` |
| Commit tree | `6674c9b0c57f0e0505d584be48c5b5638275e5d9` |
| Commit subject | `fix(color): Outputs widget does not update after changing 'Extended Limits' (#7662)` |
| Commit date | `2026-08-16T19:10:50+10:00` |
| Checkout mode | Detached inside the Linux container; temporary local worktree on Windows |
| Tracked status | Clean before and after baseline instrumentation |

Pinned submodules:

| Path | Commit |
|---|---|
| `radio/src/thirdparty/AccessDenied` | `6b04dfe4d90870f09b8aa41b8ccf952ba47a55e1` |
| `radio/src/thirdparty/FreeRTOS` | `dbf70559b27d39c1fdb68dfb9a32140b6a6777a0` |
| `radio/src/thirdparty/FreeRTOS/portable/ThirdParty/Community-Supported-Ports` | `9c5bad7b2f42b4e8420f12290b9bc085d3309e8b` |
| `radio/src/thirdparty/FreeRTOS/portable/ThirdParty/Partner-Supported-Ports` | `dc3afc6e837426b4bda81bbb6cf45bfb6f34c7e9` |
| `radio/src/thirdparty/lvgl` | `5f129c540ec43a4e5aebff9f77b3688b57a78063` |
| `radio/src/thirdparty/stb` | `31c1ad37456438565541f4919958214b6e762fb4` |
| `radio/src/thirdparty/uf2` | `d03b585ed780ed51bb0d1e6e8cf233aacb408305` |
| `radio/src/thirdparty/uf2/hidapi` | `a6a622ffb680c55da0de787ff93b80280498330f` |

The Linux checkout was cloned into the container filesystem. A Windows linked
worktree was not bind-mounted because its `.git` file contains a host path that
is not meaningful inside Linux.

## 3. Environment manifest

### 3.1 Windows simulator environment

| Item | Value |
|---|---|
| OS | Windows 11 Pro `10.0.26100`, build `26100` |
| CPU | AMD Ryzen 9 5900X, 12 physical / 24 logical cores |
| Memory | 63.93 GiB |
| Power plan | High performance |
| Display | 3840x2160 desktop; 800x600 hidden simulator window for the idle probe |
| Renderer | SDL `direct3d` |
| CMake / Ninja | `4.4.2` / `1.13.2` |
| Python / Git | `3.11.9` / `2.52.0.windows.1` |
| Compiler | Clang/LLVM `19.1.5`, `x86_64-pc-windows-msvc` |
| `libclang` | The same portable LLVM 19.1.5 distribution and builtin headers under `lib/clang/19/include` |
| SDL | Official VC development archive, SDL `2.32.8` |
| MSVC host tools | Visual Studio Community `18.1.1`, installation `18.1.11312.151`, MSVC `14.50.35717` |
| Windows SDK | `10.0.26100.0` |

The compiler and SDL versions follow the
[EdgeTX Windows build instructions](https://github.com/EdgeTX/edgetx/blob/main/docs/building/windows.md).
The host has Visual Studio 18 rather than Visual Studio 17/2022; this deviation
is retained as an explicit limitation rather than hidden.

Downloaded and runtime tool hashes:

| File | Bytes | SHA-256 |
|---|---:|---|
| `LLVM-19.1.5-win64.exe` | 352,110,889 | `9b29856202d82047b504a9e501d66080bed0eaff3f1127191f4f361e0e00c770` |
| `LLVM-19.1.5-win64.exe.sig` | 543 | `d997daa0e912b192d5c02135d49049991ea76c04f356dc16b0f4f5277107c95f` |
| `clang.exe` / `clang++.exe` | 87,157,760 each | `817b71c3e9d68dea68ba1a00a1add9e0a87cbb1d781abef1280eb495d0904ae8` |
| `libclang.dll` | 64,957,440 | `83f29c98f6fa8943a522a3a29bc8c8579170eabeea979f6a89a3ab3a9d265335` |
| `SDL2-devel-2.32.8-VC.zip` | 6,184,152 | `a16e5f0e5de87a804e3c240a7d92aca0318fda176b22d95550c6512fc4f18172` |
| `SDL2.dll` | 1,576,448 | `490fd069834f308c7ec168f0c29de8d20e1b4914e14de690414874a8697296fb` |

The SDL archive digest matches the digest published on its GitHub release.
LLVM publishes the detached signature, but it was not cryptographically
validated here because the host had no trusted LLVM release key and Git's
bundled GPG could not start its keybox daemon. The download origin and observed
hash are recorded; they are not presented as a successful signature check.

### 3.2 Linux correctness and isolation environment

| Item | Value |
|---|---|
| Image | `ghcr.io/edgetx/edgetx-dev@sha256:75a076b11bcf82128f25261d4e3149b0b9aac9bd24f49fb74076b3d0e723686c` |
| Image creation | `2026-02-22T07:16:43.618396034Z` |
| Container OS | Ubuntu 22.04, amd64 |
| Kernel | WSL2 Linux `6.18.33.2-microsoft-standard-WSL2` |
| Source filesystem | `overlayfs` |
| Logical cores | 24 |
| CMake | `4.2.3` |
| Native compiler | GCC/G++ `11.4.0` |
| ARM compiler | Arm GNU Toolchain `14.2.1` (`14.2.Rel1`) |
| Python / Git | `3.10.12` / `2.53.0` |
| SDL | `2.32.10` |
| WASI SDK | `25.0`, fetched on demand by the existing superbuild |

The immutable image is the one referenced by the
[EdgeTX development container](https://github.com/EdgeTX/edgetx/blob/main/.devcontainer/devcontainer.json).

Existing `FetchContent` declarations do not pin `URL_HASH`. The exact archives
observed in this baseline were:

| Dependency archive | Bytes | SHA-256 |
|---|---:|---|
| Dear ImGui `v1.92.6.tar.gz` | 2,050,368 | `5b17c01f69545bde732b14936d89ce0f508adb83e8b56fa82448371845172bc3` |
| `wasi-sdk-25.0-x86_64-linux.tar.gz` | 114,450,290 | `52640dde13599bf127a95499e61d6d640256119456d1af8897ab6725bcf3d89c` |

## 4. Command manifest

### 4.1 Windows

The x64 Visual Studio developer environment was loaded first. The portable LLVM
directory was then prepended to `PATH`; this ordering is required because
`radio/util/find_clang.py` selects the first `libclang.dll` in `PATH`.

```powershell
$env:Path = "$llvmBin;$env:Path"

cmake -S $source -B $build -G Ninja -Wno-author `
  -DPCB=X10 -DPCBREV=TX16S -DDEFAULT_MODE=2 `
  -DCMAKE_BUILD_TYPE=Release -DDISABLE_COMPANION=ON `
  -DNATIVE_BUILD=ON -DEdgeTX_SUPERBUILD=OFF `
  -DCMAKE_TOOLCHAIN_FILE="$source/cmake/toolchain/native.cmake" `
  -DFETCHCONTENT_BASE_DIR=$fetch `
  -DSDL2_DIR="$sdlRoot/cmake" `
  -DCMAKE_C_COMPILER="$llvmBin/clang.exe" `
  -DCMAKE_CXX_COMPILER="$llvmBin/clang++.exe"

cmake --build $build --target simu --parallel 24 -- --quiet
cmake --build $build --target simu --parallel 24
& "$build/simu.exe" --help
```

Each clean repetition used a distinct build directory and a distinct
`FETCHCONTENT_BASE_DIR`. The ImGui download duration was taken from the
generated `.ninja_log`, not estimated by subtracting unrelated timings.

The launch smoke used isolated storage/settings directories. It found the SDL
top-level windows by process ID, posted `WM_CLOSE`, observed `edgeTxClose`, and
waited for process exit 0.

### 4.2 Linux

The maintained test entry point was used directly:

```bash
tools/commit-tests.sh -btx16s -Wno-error
```

The remaining representative commands were:

```bash
cmake -S . -B build/phase0-linux-simu-run1 -G "Unix Makefiles" \
  -Wno-author -DPCB=X10 -DPCBREV=TX16S -DDEFAULT_MODE=2 \
  -DCMAKE_BUILD_TYPE=Release -DDISABLE_COMPANION=ON \
  -DNATIVE_BUILD=ON -DEdgeTX_SUPERBUILD=OFF \
  -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain/native.cmake
cmake --build build/phase0-linux-simu-run1 --target simu --parallel 24

cmake -S . -B build/phase0-linux-artifacts -G "Unix Makefiles" \
  -Wno-author -DPCB=X10 -DPCBREV=TX16S -DDEFAULT_MODE=2 \
  -DCMAKE_BUILD_TYPE=Release
cmake --build build/phase0-linux-artifacts --target firmware --parallel 24
CMAKE_BUILD_PARALLEL_LEVEL=24 \
  cmake --build build/phase0-linux-artifacts --target wasi-module
```

## 5. Baseline results

### 5.1 Windows clean builds

| Valid run | ImGui download | ImGui populate pipeline | CMake configure total | Clean `simu` build | Bytes | SHA-256 |
|---:|---:|---:|---:|---:|---:|---|
| 2 | 1.876 s | 2.469 s | 18.414 s | 38.277 s | 7,346,176 | `e5d92123bd0372c08fdb9e912d71b9b2c0de10110324b5f052abe02549d2d9bc` |
| 3 | 1.779 s | 2.344 s | 16.776 s | 35.562 s | 7,346,176 | `fe410becad6d3b56c058cfc258ef764405089e22d4df0ec4dec5287ad11b2618` |
| 4 | 1.786 s | 2.444 s | 16.279 s | 39.651 s | 7,346,176 | `fc3cfe3eb97566980dce05cc31aeaebad8e9f7bd7e0b8fbe23dc0f03b5bd2510` |
| **Median** | **1.786 s** | **2.444 s** | **16.776 s** | **38.277 s** | **7,346,176** | n/a |

`CMake configure total` includes the populate pipeline; the component columns
are reported separately but are not additive. One no-change incremental build
took `0.189 s` and reported `ninja: no work to do`.

The equal-size executables have different hashes. `llvm-readobj` confirmed a
different PE/COFF `TimeDateStamp` per link, so byte-for-byte equality is not an
acceptance criterion. Each observed hash remains recorded for traceability.

### 5.2 Windows runtime smoke

| Check | Result |
|---|---|
| `simu.exe --help` | Exit 0; documented width, height, storage, settings, and help options |
| Ordinary launch | SDL Direct3D renderer created; EdgeTX initialization reached |
| Isolation | Only build-local storage and settings directories were used |
| Clean close | `WM_CLOSE` reached `edgeTxClose`; process exited 0 |
| Ready-to-close time | 0.236 s to discover top-level windows |
| Total smoke duration | 1.589 s |

### 5.3 Linux correctness and artifacts

| Check | Result | Time | Artifact / hash |
|---|---|---:|---|
| Maintained TX16S native tests | 103/103 passed, 23 suites | 61.260 s | Test execution itself: 227 ms |
| Release simulator configure | Passed | 1.038 s | GCC 11.4, SDL found |
| Release simulator clean build | Passed | 35.373 s | 66,464,944 bytes; `5b919adc393258c772bf035258433b0048b0237ba4b7594375122661107f1681` |
| Release simulator incremental | Passed | 0.889 s | No source change |
| `simu --help` | Exit 0 | n/a | Same option surface as Windows |
| Superbuild configure | Passed | 0.373 s | Release TX16S |
| Physical firmware | Passed | 507.618 s | See artifact table |
| WASM module | Passed | 69.217 s | Download + configure + build |

Physical and WASM artifact details:

| Artifact | Bytes | SHA-256 |
|---|---:|---|
| `firmware.bin` | 1,612,852 | `9064711ef228ad5d64a1bffee175a71d11c72fc3b4c2229ba4673cfcf03b3c72` |
| `firmware.elf` | 37,755,248 | `e25b68fc8f37a6da35a0900f063081cab9b27922688615772f3b1d1b77728618` |
| `edgetx-tx16s-simulator.wasm` | 5,233,497 | `37264a59768a39e267416dfae5287a1f60a68a587e80f8854f8752c997d51eff` |

`arm-none-eabi-size firmware.elf` reported `text=1,610,144`, `data=2,688`,
`bss=3,262,268`, and `dec=4,875,100` bytes. The firmware target's nested
sub-build did not inherit the requested parallelism, so its 507.618-second time
is a correctness record, not a build-performance target.

## 6. Automation-disabled idle timing

`upstream/main` already uses `SDL_GetPerformanceCounter()` around
`handleEvents()` to enforce frame pacing, but it does not retain or emit the
samples. A measurement-only probe was therefore applied locally, compiled, run,
and removed. It was never committed to an implementation branch.

Probe controls:

- enabled only by `EDGETX_PHASE0_PERF`;
- five-second warm-up;
- 30-second sampling window and at least 1,000 iterations;
- preallocated 4,096-element buffers;
- nearest-rank percentiles;
- `loop_work` ends before deliberate `SDL_Delay`;
- `loop_period` is start-to-start and includes frame pacing;
- three independent processes with copied fixtures;
- hidden 800x600 window, Direct3D renderer, Release build, high-performance
  power plan; and
- normal cleanup and exit 0 after reporting.

The complete code delta is in the appendix. The patched source SHA-256 was
`cdeb28b87be898e7f0f0a3654517cac1facec2135efd70ee118d02cb6328c87b`
(Git blob `fbbc4710629d51170245a9e99a9941f3f9b8ea7d`), and the normalized
appendix diff SHA-256 is
`1b533b8043426b9c22c88b3da10ccfbc7bcdf3794675c71fe548352f0a65875a`.
The probe executable SHA-256 was
`b7f08802152b9e0d222269ad2b4828027797b9a8750ae710b6dfbae149422292`;
it is not a product artifact.

### 6.1 Corrected runs

`loop_work` in milliseconds:

| Run | Samples | p50 | p95 | p99 | Max |
|---:|---:|---:|---:|---:|---:|
| 1 | 1,791 | 0.184700 | 0.355900 | 0.521200 | 0.756500 |
| 2 | 1,793 | 0.197200 | 0.382200 | 0.564600 | 0.932200 |
| 3 | 1,795 | 0.209900 | 0.446000 | 0.706500 | 1.505300 |
| **Median** | **1,793** | **0.197200** | **0.382200** | **0.564600** | **0.932200** |

`loop_period` in milliseconds:

| Run | Samples | p50 | p95 | p99 | Max |
|---:|---:|---:|---:|---:|---:|
| 1 | 1,791 | 16.681200 | 17.244400 | 17.622500 | 17.750500 |
| 2 | 1,793 | 16.665500 | 17.262100 | 17.653700 | 17.907800 |
| 3 | 1,795 | 16.656000 | 17.254400 | 17.658900 | 18.126700 |
| **Median** | **1,793** | **16.665500** | **17.254400** | **17.653700** | **17.907800** |

Median process wall time was `35.611 s`, including warm-up and cleanup.

The probe does add two reserved-vector appends per sampled iteration. Its
`loop_work` boundary excludes those appends; `loop_period` necessarily includes
their small cost. Phase 2 must use the same instrumentation for disabled and
enabled-but-idle paired runs, so comparison remains like-for-like.

## 7. Excluded evidence and corrections

These attempts are intentionally not counted as passing evidence:

1. A Windows build used Clang 19.1.5 as compiler but discovered the globally
   installed `libclang` 22 first. It was discarded. Prepending the portable
   LLVM 19.1.5 directory to `PATH` fixed compiler, library, and builtin-header
   alignment in all three valid runs.
2. Scoop's SDL package did not expose the `sdl2-config.cmake` required by the
   project's config-mode package lookup. A handcrafted compatibility shim and
   a Clang 22 workaround were discarded. The official SDL VC archive was used.
3. Existing Windows radio tests were not treated as a failure because their
   current target includes POSIX-only headers. The maintained Linux test script
   is authoritative for the existing suite; new pure protocol tests must still
   become cross-platform.
4. A Linux bind mount of a Windows linked worktree was discarded because its
   `.git` indirection contained a Windows path. The valid run used a native
   detached clone in the container filesystem.
5. The first performance pilot allowed sample vectors to grow dynamically.
   Because allocation could contaminate `loop_period`, all pilot values were
   discarded. The published runs reserve capacity before warm-up.
6. A first hidden-window close attempt relied on `MainWindowHandle`, which was
   zero for the hidden SDL process. The valid smoke enumerated top-level windows
   by PID and posted `WM_CLOSE`.

## 8. Known limitations and next action

- G0 is pending; this is the only blocker to beginning Phase 1.
- Windows host integration used VS 18/MSVC 14.50 rather than VS 17/2022.
- The LLVM detached signature was not validated against a trusted key.
- Existing ImGui and WASI downloads lack `URL_HASH`; observed hashes are
  recorded, but a separate build-system hardening change would be needed to
  enforce them.
- Windows PE timestamps prevent deterministic executable hashes in this setup.
- Idle timings use a hidden Direct3D window and are not a visible-window UX
  benchmark.
- Linux simulator launch was not attempted without a display server; Windows is
  the authoritative runtime-smoke environment in this plan.
- Firmware and WASM hashes are pre-feature reference points. Absence of
  automation symbols is checked only on post-implementation artifacts in
  Phase 8.

The next action is review, not code: obtain explicit responses for D01–D19,
transport naming, stdout/stderr separation, warm/cold restart terminology,
deferred surfaces, and attribution. Record accepted or objected rows and begin
Phase 1 only after G0 passes.

## Appendix A — Measurement-only probe diff

```diff
diff --git a/radio/src/targets/simu/sdl_simu.cpp b/radio/src/targets/simu/sdl_simu.cpp
index a6ab43e330..fbbc471062 100644
--- a/radio/src/targets/simu/sdl_simu.cpp
+++ b/radio/src/targets/simu/sdl_simu.cpp
@@ -28,11 +28,13 @@
 #include <imgui_impl_sdlrenderer2.h>

 #include <algorithm>
+#include <cstdlib>
 #include <iostream>
 #include <fstream>
 #include <filesystem>
 #include <regex>
 #include <string>
+#include <vector>

 #include "hal/adc_driver.h"
 #include "hal/rotary_encoder.h"
@@ -803,17 +805,64 @@ int main(int argc, char* argv[])
 #if defined(__EMSCRIPTEN__)
   emscripten_set_main_loop([]() { handleEvents(); }, 0, true);
 #else
+  const bool phase0_perf_probe = std::getenv("EDGETX_PHASE0_PERF") != nullptr;
+  const Uint64 perf_frequency = SDL_GetPerformanceFrequency();
+  const Uint64 perf_probe_start = SDL_GetPerformanceCounter();
+  Uint64 perf_sample_start = 0;
+  Uint64 previous_loop_start = 0;
+  std::vector<double> loop_work_samples;
+  std::vector<double> loop_period_samples;
+  loop_work_samples.reserve(4096);
+  loop_period_samples.reserve(4096);
+
   do {
     Uint64 start_ts = SDL_GetPerformanceCounter();
+
+    if (phase0_perf_probe && perf_sample_start == 0 &&
+        start_ts - perf_probe_start >= 5 * perf_frequency) {
+      perf_sample_start = start_ts;
+    }
+
+    if (perf_sample_start != 0 && previous_loop_start != 0) {
+      loop_period_samples.push_back(
+          (start_ts - previous_loop_start) * 1000.0 / perf_frequency);
+    }
+    previous_loop_start = start_ts;
+
     if (!handleEvents()) break;

     Uint64 end_ts = SDL_GetPerformanceCounter();
-    float elapsedMS =
-      (end_ts - start_ts) / (float)SDL_GetPerformanceFrequency() * 1000.0f;
+    float elapsedMS = (end_ts - start_ts) / (float)perf_frequency * 1000.0f;
+
+    if (perf_sample_start != 0) {
+      loop_work_samples.push_back(elapsedMS);
+    }

     // Cap to 60 FPS
     SDL_Delay(std::max(0,(int32_t)floor(16.666f - elapsedMS)));

+    if (perf_sample_start != 0 &&
+        SDL_GetPerformanceCounter() - perf_sample_start >= 30 * perf_frequency &&
+        loop_work_samples.size() >= 1000) {
+      auto report_samples = [](const char* name, std::vector<double> samples) {
+        std::sort(samples.begin(), samples.end());
+        auto percentile = [&samples](double quantile) {
+          const size_t index = std::min(
+              samples.size() - 1,
+              static_cast<size_t>(ceil(quantile * samples.size())) - 1);
+          return samples[index];
+        };
+        printf("PHASE0_PERF name=%s samples=%zu p50_ms=%.6f p95_ms=%.6f "
+               "p99_ms=%.6f max_ms=%.6f\n",
+               name, samples.size(), percentile(0.50), percentile(0.95),
+               percentile(0.99), samples.back());
+      };
+
+      report_samples("loop_work", loop_work_samples);
+      report_samples("loop_period", loop_period_samples);
+      break;
+    }
+
   } while(true);
 #endif
```

## References

- [EdgeTX Windows build instructions](https://github.com/EdgeTX/edgetx/blob/main/docs/building/windows.md)
- [EdgeTX maintained commit test script](https://github.com/EdgeTX/edgetx/blob/main/tools/commit-tests.sh)
- [EdgeTX development container](https://github.com/EdgeTX/edgetx/blob/main/.devcontainer/devcontainer.json)
- [CMake config-mode package search](https://cmake.org/cmake/help/latest/command/find_package.html)
- [Git linked-worktree details](https://git-scm.com/docs/git-worktree.html)
- [SDL performance counter](https://wiki.libsdl.org/SDL2/SDL_GetPerformanceCounter)
- [LLVM 19.1.5 release](https://github.com/llvm/llvm-project/releases/tag/llvmorg-19.1.5)
- [SDL 2.32.8 release](https://github.com/libsdl-org/SDL/releases/tag/release-2.32.8)
