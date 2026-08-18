# EdgeTX simulator UI harness

This directory contains the dependency-free host side of the native simulator
automation protocol. The current foundation intentionally supports only the
implemented `ping` and `stop` commands. Input, discovery, flows, and capture are
added in later phases of the consolidation plan.

Run a lifecycle probe with Python 3:

```text
python tools/ui-harness/edgetx-ui probe \
  --output build/ui-harness/manual-run \
  build/native/radio/src/targets/simu/simu \
  -- --storage <sdcard-copy> --settings <settings-copy>
```

The client launches with binary pipes, starts independent stdout and stderr
readers before sending `ping`, correlates every response by request ID, and
always reaps the child process. It does not use pipe `select`, shell command
interpolation, or a fixed startup sleep, so the same lifecycle works on POSIX
and Windows.

Run the focused tests from the repository root:

```text
python -m unittest discover -s tools/ui-harness/tests -v
```
