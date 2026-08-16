# Code Generation

Several source files in the EdgeTX tree are not written by hand — they are produced
by generator scripts and then **committed to git**. The build does not run these
generators, so if you change one of their inputs you must re-run the generator and
commit the regenerated output alongside your change.

| Generator | Produces | Re-run when you change |
|---|---|---|
| `radio/src/fonts/lvgl/make_fonts.sh` | `radio/src/fonts/lvgl/{std,sml,lrg}/lv_font_*.c` | a font source, or add a character to the `cn`/`tw`/`jp`/`he`/`ko`/`ru`/`ua` translations |
| `tools/cfn_sorter.sh` | `radio/src/cfn_sort.cpp` | a special function name (`TR_SF_*`) in any translation, or `radio/src/dataconstants.h` |
| `tools/generate-yaml.sh` | `radio/src/storage/yaml/yaml_datastructs_*.cpp` | `ModelData`/`RadioData` structures |

!!! note
    Both translation-driven generators are narrower than they look. `cfn_sorter.cpp`
    reads only the 25 `TR_SF_*` special function names, so editing any other string
    leaves `cfn_sort.cpp` untouched. The fonts only change if a translation
    introduces a character the font does not already contain, and only for the seven
    languages listed above — the Latin-script translations never affect them.

    If you are unsure, just run the generator: it rewrites its output in place, so
    `git status` afterwards tells you whether anything actually moved.

## Using the development container

All three generators need tooling that is awkward to install by hand — a set of
system locales, the `lv_font_conv` npm package, and a specific `libclang` version.
The [EdgeTX development container](https://github.com/EdgeTX/build-edgetx) has all
of it preinstalled, and is the reference environment:

```shell
docker run -it --rm -v $(pwd):/src -w /src \
  ghcr.io/edgetx/edgetx-dev \
  bash
```

Everything below can then be run from `/src` inside that container. If you use the
[dev container](https://containers.dev/) integration in VS Code, the same image is
picked up automatically from `.devcontainer/devcontainer.json`.

## LVGL fonts

`radio/src/fonts/lvgl/make_fonts.sh` regenerates the LVGL font tables for the colour
LCD radios, in three sizes (`std`, `sml` and `lrg` — selected at build time by the
radio's screen resolution).

```shell
radio/src/fonts/lvgl/make_fonts.sh
```

It reads the TrueType/OpenType sources under `radio/src/fonts/` (Roboto, Noto, Arimo
and Nanum), the EdgeTX symbol fonts in `radio/src/fonts/lvgl/EdgeTX/`, and the
FontAwesome set that ships inside the LVGL submodule. For the non-Latin languages it
also scans the translation headers to work out which glyphs are actually needed, via
the `get_char_*.py` helpers in the same directory.

Requirements:

- `lv_font_conv` — `npm install -g lv_font_conv`
- `python3` and `gcc` (the script compiles a small LZ4 compression helper on the fly)
- the LVGL submodule — `git submodule update --init --recursive`

A full run takes a few minutes and rewrites around 150 files.

!!! warning
    If a translation header cannot be read, the script prints
    `WARNING: No characters found ... Skipping` and carries on rather than failing.
    Check the output for that message — an otherwise clean run may simply have
    skipped a language.

## Custom function sort order

`tools/cfn_sorter.sh` generates `radio/src/cfn_sort.cpp`, the table that puts the
custom function list into alphabetical order *for each translated language*.

```shell
tools/cfn_sorter.sh
```

It compiles `tools/cfn_sorter.cpp` once per language (selecting the language with a
`-DLNG_xx` define) and concatenates the results. Sorting uses `std::locale`, so the
matching system locale must exist, or the program aborts at runtime with
`locale::facet::_S_create_c_locale name not valid`.

The dev container and the `tools/setup_buildenv_ubuntu*.sh` scripts install these
already. On any other system, generate them by hand:

```shell
apt update && apt install locales
locale-gen zh_CN.UTF-8 cs_CZ.UTF-8 da_DK.UTF-8 de_DE.UTF-8 es_ES.UTF-8 en_US.UTF-8 \
  fi_FI.UTF-8 fr_FR.UTF-8 he_IL.UTF-8 it_IT.UTF-8 ja_JP.UTF-8 ko_KR.UTF-8 \
  nl_NL.UTF-8 pl_PL.UTF-8 pt_PT.UTF-8 ru_RU.UTF-8 sv_SE.UTF-8 zh_TW.UTF-8 uk_UA.UTF-8
```

The full list is repeated in the comments at the top of the script. Only a C++
compiler and those locales are needed; a run takes a few seconds.

## YAML parsers

`tools/generate-yaml.sh` regenerates the YAML storage parsers. Because it depends on
a particular `libclang` version, it should always be run in the development
container:

```shell
docker run -it --rm -v $(pwd):/src \
  ghcr.io/edgetx/edgetx-dev \
  /src/tools/generate-yaml.sh
```

Set `FLAVOR` to a semicolon-separated list to limit which radios are generated:

```shell
docker run -it --rm -v $(pwd):/src \
  -e "FLAVOR=tx16s;nv14;x7;x9d" \
  ghcr.io/edgetx/edgetx-dev \
  /src/tools/generate-yaml.sh
```

!!! warning
    The script deletes and recreates a `build` directory in the current working
    directory, so run it from the repository root and expect any existing `build/`
    to be removed.

See [YAML Parser/Generator](yaml-parser-generator.md) for how the parsers work, and
for what to do when adding a new radio.

## Checking your regenerated output

After running a generator, review what changed before committing:

```shell
git status --porcelain -- radio/src/fonts/lvgl radio/src/cfn_sort.cpp radio/src/storage/yaml
git diff --stat
```

An empty result means the committed files were already up to date and there is
nothing to commit. Otherwise, commit the regenerated files together with the change
that caused them to move.

!!! tip
    Both `make_fonts.sh` and `cfn_sorter.sh` leave intermediate files behind
    (`lv_font.inc`, `lz4_font` and `tools/a.out`) if they are interrupted. They are
    removed automatically on a successful run, but are safe to delete by hand.
