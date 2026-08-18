# EdgeTX simulator UI harness

This directory contains the dependency-free host side of the native simulator
automation protocol. The current foundation supports `ping`, `status`,
`describe`, and `stop`. Input, flows, and capture are added in later phases of
the consolidation plan; `describe` advertises only commands and capabilities
that are usable in the current build.

Run a lifecycle probe with Python 3:

```text
python tools/ui-harness/edgetx-ui probe \
  --output build/ui-harness/manual-run \
  build/native/radio/src/targets/simu/simu \
  -- --storage <sdcard-copy> --settings <settings-copy>
```

The client launches with binary pipes, starts independent stdout and stderr
readers, sends `ping`, validates the bounded `describe` result, and polls
`status` until the simulator reports a real first LCD frame. Target identity,
LCD dimensions, command availability, and optional required capabilities are
checked before startup succeeds. Every response is correlated by request ID,
and every shutdown path waits for the child after graceful stop, terminate, or
kill. There is no pipe `select`, shell interpolation, or fixed startup sleep, so
the same lifecycle works on POSIX and Windows.

Run the focused tests from the repository root:

```text
python -m unittest discover -s tools/ui-harness/tests -v
```
