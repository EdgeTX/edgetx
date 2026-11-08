# AGENTS.md — EdgeTX (FlySky G11P fork)

Fork of EdgeTX 2.11.8 (`richardclli/fs-g11p-2.11`) adding the **FlySky G11P** target.
Embedded C/C++ firmware (STM32H750) + Qt5 **Companion** desktop app. Not a Java project.

## Layout

- `radio/src/` — firmware; `radio/src/targets/<board>/` — board support (`st16`, `pl18`, ...).
- `radio/src/gui/` — UI (`128x64`, `212x64`, `colorlcd`); `radio/src/storage/yaml/` — model storage.
- `radio/util/hw_defs/` — Python generators for hardware definitions (see its README).
- `companion/src/` — Qt5 Companion; `tools/` — build scripts; `cmake/` — CMake helpers.
- `g11p-dev-plan.md` — live G11P tracker; `st16-vs-g11p-pinout.md` — board pinout deltas.

## Build environment

- Builds run in `ghcr.io/edgetx/edgetx-dev:2.11` (Arm GNU Toolchain 14.2, Qt5, SDL2).
  See `.devcontainer/` and `tools/setup_buildenv_*.sh`; do not assume the host toolchain matches.
- Scripts `rm -rf build`; `build/` and repo-root `*.uf2|*.bin` are gitignored. Container builds
  run as root, so `build/` may need `sudo rm -rf build` before a host-side script will run.
- Submodules must be checked out recursively (`git submodule update --init --recursive`).

## Commands (target = lowercase `FLAVOR`)

Repo scripts (CI-equivalent):

```sh
FLAVOR=g11p ./tools/build-gh.sh            # -> g11p-<sha>.uf2|.bin at repo root
FLAVOR=g11p ./tools/commit-tests.sh        # firmware-size + libsimulator + radio gtests
./tools/build-companion.sh "$PWD" "$PWD/output/"   # Linux AppImage; builds all simulator plugins
```

Language is a CMake option: `EXTRA_OPTIONS="-DTRANSLATIONS=CN"` (not `LANG`).
Docker form:
`docker run --rm -e FLAVOR=g11p -v "$PWD:/src" -w /src ghcr.io/edgetx/edgetx-dev:2.11 -lc './tools/build-gh.sh'`

Host release wrapper (different env contract — language is `LANG`, file is named by `git describe`):

```sh
docker run --rm -e GITHUB_REF='refs/heads/richardclli/fs-g11p-2.11' \
  -e FLAVOR=g11p -e LANG=EN \
  -v "/home/clli/edgetx:/edgetx" -v "/edgetx/build:/build" -w /edgetx \
  --entrypoint /bin/bash ghcr.io/edgetx/edgetx-dev:2.11 \
  -lc '/build/2.11-build-gh-release.sh'
```

Writes `/edgetx/build/output/etx-<git describe>-g11p-en.uf2` (+ `-bl.bin` when built). The default
`2.11-build-all-release.sh` FLAVOR list omits `g11p`, so pass `FLAVOR` explicitly.

Single tests after native configure: `build/native/gtests-radio --gtest_filter='...'`,
`build/native/gtests-companion --gtest_filter='...'`.

## Target model (high-signal)

- A flavor maps to `-DPCB=<platform>` + optional `-DPCBREV=<revision>`; register new flavors only in
  `tools/build-common.sh` (and `tools/build-flysky.py` for FlySky release builds).
- G11P is a **revision of the ST16 board** (like NB4P is of PL18): `g11p` → `-DPCB=ST16 -DPCBREV=G11P`,
  sources in `radio/src/targets/st16/`, guarded by `RADIO_G11P`.
- G11P differs from ST16: no I/O expander, IMU, Bluetooth, S.Port, external module bay or function
  switches; 320x480 portrait LCD; PL18 touch driver; `g11p_key_driver.cpp`; trainer port on UART7.
- Adding a board requires Companion edits too: `companion/src/firmwares/boards.{h,cpp}`,
  `opentxinterface.cpp`, `generalsettings.cpp`, `moduledata.cpp`, and `companion/src/CMakeLists.txt`.
- Hardware definitions come from `hal.h` + `radio/util/hw_defs/*.py`. Validate with `make hardware_defs`
  (→ `g11p.json`) and `make yaml_inputs` (→ `yaml_inputs.inc`); `tools/generate-yaml.sh` runs
  `make -C native yaml_data` and regenerates the committed `yaml_datastructs_g11p.cpp`. Edit inputs,
  not generated output.
- Keep `fw.json`, `tools/build-common.sh`, and the CI matrix in sync when touching targets/versions.

## Gotchas

- CI (`.github/workflows/*`) triggers only on `main` and `N.N` branches/tags; this fork branch gets no CI.
- Never run `tools/clang-format.sh` repo-wide (it also reformats `thirdparty/`). Format touched files only:
  `clang-format -i <file>`; style is `.clang-format` (Google + Linux braces). `tools/codeformat.sh`
  (uncrustify) excludes `thirdparty/` but is legacy.
- Commit style is Conventional Commits with scope (`feat(g11p):`, `fix(g11p):`, `docs(g11p):`). The plan
  pre-authorizes committing/pushing completed checklist items individually to
  `richardclli/fs-g11p-2.11`; otherwise do not commit unless asked.
- Keep `g11p-dev-plan.md` status/checklist current. Outstanding: hardware bring-up and adding `g11p`
  to the CI matrix.
- A `.codegraph/` index exists (gitignored); `codegraph_explore` is useful before grep/read loops.
