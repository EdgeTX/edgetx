# User Build Guide

This guide describes how to use the local user build workflow in this repository.

## Scope

This guide covers:
- Interactive build flow (`tools/user-builds.sh`)
- Firmware, simulator, and companion outputs
- Radio and firmware option TSV files
- Safe regeneration rules for curated TSV files
- Common troubleshooting

## Files Used by the Workflow

- `tools/user-builds.sh`
- `tools/radio-targets.tsv` (curated radio list)
- `tools/radio-firmware-options.tsv` (curated firmware option list)
- `tools/radio-targets.py` (generator/check tool)
- `tools/radio-firmware-options.py` (generator/check tool)

## Prerequisites

### Required tools

- `cmake`
- `python3`
- Build toolchain used by your platform (for simulator/companion)

### Firmware-only extra requirement

Firmware builds require Arm GNU toolchain 14.2.rel1:
- `arm-none-eabi-gcc`
- `arm-none-eabi-g++`
- Compiler version must resolve to `14.2.1`

The script enforces this for firmware builds.

### Python packages

Build helpers require these Python packages in the Python interpreter used by the build:
- `Pillow` (`PIL`)
- `jinja2`
- `lz4`

A virtual environment is recommended but not strictly mandatory.

## Recommended Virtual Environment

Example (using a shared local venv path):

```bash
python3 -m venv /Users/jimb40/GitHub/.venvs/edgetx
source /Users/jimb40/GitHub/.venvs/edgetx/bin/activate
python -m pip install -U pip Pillow jinja2 lz4
```

If CMake was configured with a stale Python path, reconfigure with:

```bash
cmake -S . -B build -DPython3_EXECUTABLE=/Users/jimb40/GitHub/.venvs/edgetx/bin/python3
```

## Running User Builds

From repository root:

```bash
tools/user-builds.sh
```

Optional flags:
- `--txt` disables `fzf` UI and uses plain terminal prompts
- `--select <index>` preselects a radio index from `tools/radio-targets.tsv`

## Interactive flow summary

The script asks for:
1. Clean build or reuse existing `build/`
2. Build type: `Firmware`, `Simulator`, or `Companion`
3. Radio target from `tools/radio-targets.tsv`
4. For firmware: whether to use custom firmware options
5. Build confirmation

Selections are remembered in:
- `user-builds/.user-builds-config`

## Build Outputs

### Firmware output

- Default firmware output directory:
  - `user-builds/firmwares/`
- Firmware with custom options:
  - `user-builds/firmwares-custom/`

Typical artifacts:
- `*.bin`
- `*.uf2` (when produced by target)
- `*.buildinfo.txt` (for custom-option builds)

Custom-option firmware names include a deterministic fingerprint:
- `...-cust-<hash8>`

### Simulator and companion output (macOS app bundles)

The script creates app bundles in:
- `user-builds/edgetx-simulator-<version>.app`
- `user-builds/edgetx-companion-<version>.app`

It also updates matching simulator libraries inside the app bundle.

## TSV Curation and Protection

`tools/radio-targets.tsv` and `tools/radio-firmware-options.tsv` are treated as curated files.

Generator scripts are now safe by default:
- They refuse to overwrite existing TSV files unless `--force` is provided.
- They support `--check` to compare generated output against curated TSV files.
- Their default write target is a separate `*.generated.tsv` file.

## `tools/radio-targets.py`

Generate to safe default output:

```bash
tools/radio-targets.py --export-tsv
# writes tools/radio-targets.generated.tsv
```

Check curated file consistency:

```bash
tools/radio-targets.py --check
# checks tools/radio-targets.tsv
```

Intentionally overwrite curated TSV:

```bash
tools/radio-targets.py --export-tsv tools/radio-targets.tsv --force
```

## `tools/radio-firmware-options.py`

Generate to safe default output:

```bash
tools/radio-firmware-options.py
# writes tools/radio-firmware-options.generated.tsv
```

Check curated file consistency:

```bash
tools/radio-firmware-options.py --check
# checks tools/radio-firmware-options.tsv
```

Intentionally overwrite curated TSV:

```bash
tools/radio-firmware-options.py --out tools/radio-firmware-options.tsv --force
```

## Suggested TSV update workflow

1. Generate to `*.generated.tsv`
2. Review differences manually
3. If approved, overwrite curated file with `--force`
4. Re-run `--check` to confirm consistency

## Troubleshooting

### Missing `PIL` / `jinja2` / `lz4`

Symptoms:
- `ModuleNotFoundError: No module named 'PIL'`
- `ModuleNotFoundError: No module named 'jinja2'`

Fix:
1. Activate your intended environment
2. Install required packages
3. Reconfigure CMake to that Python executable

### Wrong Python picked during build

Check active Python:

```bash
which python3
python3 -c 'import sys; print(sys.executable)'
```

Force CMake to the correct interpreter:

```bash
cmake -S . -B build -DPython3_EXECUTABLE=/path/to/your/python3
```

### Firmware toolchain version rejection

If firmware build reports unsupported toolchain version, ensure `arm-none-eabi-g++ -dumpversion` resolves to `14.2.1`.

## Notes

- The workflow does not require committing generated outputs.
- `user-builds/` is for local artifacts and per-user build state.
