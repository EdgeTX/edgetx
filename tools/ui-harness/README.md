# EdgeTX UI Harness

This harness gives agents and CI a repeatable control plane for TX16S-class
simulator UI work.

The current backend starts the SDL simulator with `--automation-stdio`. The
automation protocol runs inside the simulator process and uses `simuSetKey`,
`simuRotaryEncoderEvent`, `simuTouchDown`, `simuTouchUp`, and `simuLcdCopy`.
Screenshots are copied from the simulator framebuffer, then converted to PNG by
the Python harness.

## Usage

```sh
uv run tools/ui-harness/edgetx-ui build tx16s
uv run tools/ui-harness/edgetx-ui smoke --target tx16s
uv run tools/ui-harness/edgetx-ui run-flow tools/ui-harness/flows/tx16s-smoke.json
uv run tools/ui-harness/edgetx-mcp
```

`edgetx-mcp` is a stdio MCP server. It exposes build/start/stop, key, rotary,
touch, wait, screenshot, status, and run-flow tools. It uses the same Python
core as the CLI.

The root `pyproject.toml` mirrors the Python build dependencies used by
EdgeTX's existing `requirements.txt`. Running through `uv` gives CMake a Python
with Pillow, clang bindings, lz4, jinja2, and the other scripts dependencies.

## Target

The first target is `tx16s`, configured as `PCB=X10` and `PCBREV=TX16S`.

Default fixtures are created on demand:

```text
tools/ui-harness/fixtures/sdcard-tx16s
tools/ui-harness/fixtures/settings-tx16s
```
