# EdgeTX dev containers

This folder contains several dev container configurations. When more than one
exists, VS Code shows a picker when you run **Dev Containers: Reopen in
Container** (or **Rebuild and Reopen in Container**). To switch later, use
**Dev Containers: Switch Container** or reopen the folder locally and pick
again.

| Configuration | Use when |
|---|---|
| `edgetx-dev` (default) | No host integration needed. Used by CI/Codespaces; works everywhere. |
| `edgetx-dev (WSL2)` | Windows host with WSL2 — sound via WSLg's PulseAudio. |
| `edgetx-dev (Linux)` | Linux host — sound via the host PulseAudio/PipeWire socket. |
| `edgetx-dev (macOS)` | macOS host — sound via PulseAudio over TCP and GUI apps via XQuartz X11 forwarding (host setup required, see below). |

The variants are copies of the default config plus host-specific settings —
`devcontainer.json` has no inheritance mechanism, so **changes to the default
config must be mirrored into each variant**.

The audio variants connect the container to a host PulseAudio server through
the `PULSE_SERVER` environment variable. The simulator's SDL audio backend
picks this up automatically; no code changes or in-container audio daemon are
needed.

## Windows (WSL2)

Works out of the box on Windows 10 21H2+ / Windows 11, where WSLg provides a
PulseAudio server. Both Docker Desktop (WSL2 backend) and Docker Engine
installed inside a WSL distro are supported — the config mounts both candidate
socket paths and uses whichever exists.

## Linux

Works out of the box with PulseAudio or PipeWire (which provides a
PulseAudio-compatible socket at `$XDG_RUNTIME_DIR/pulse/native`).

## macOS

Docker runs in a VM on macOS, so audio and display can't be passed through
directly — both go via the network instead. Two separate one-time setups are
needed: PulseAudio for audio, and XQuartz for X11 display forwarding.

### Audio

The container streams audio over TCP to a PulseAudio server on the Mac.
One-time setup:

```sh
brew install pulseaudio
pulseaudio --load="module-native-protocol-tcp auth-anonymous=1" --exit-idle-time=-1 --daemon
```

Re-run the `pulseaudio` command after a reboot, or set it up as a service with
`brew services start pulseaudio` (then add the TCP module with
`auth-anonymous=1` to `$(brew --prefix)/etc/pulse/default.pa`).

### Display (X11)

GUI apps (e.g. the simulator) are shown on the Mac via X11 forwarding to
XQuartz. One-time setup:

```sh
brew install --cask xquartz
```

Then launch XQuartz and, in its preferences (**Settings → Security**), enable
**"Allow connections from network clients"**. Restart XQuartz (or log out and
back in) for the change to take effect.

Each time before starting the container, allow connections from the
container on the Mac host:

```sh
xhost +localhost
```

## Testing / troubleshooting

Quick audible check from inside the container:

```sh
sudo apt-get update && sudo apt-get install -y pulseaudio-utils
timeout 2 pacat -p /dev/urandom   # ~2 seconds of static
```

If there's no sound:

- **WSL2:** check the socket exists in the container
  (`test -S /mnt/wslg/PulseServer && echo OK`) and that WSLg is available on
  the host (`/mnt/wslg/PulseServer` exists inside your WSL distro;
  `wsl.exe --version` should list a WSLg version).
- **Linux:** check `$XDG_RUNTIME_DIR/pulse/native` exists on the host and that
  `/run/pulse/native` is a socket inside the container.
- **macOS:** check the server is listening on the host
  (`lsof -iTCP:4713 -sTCP:LISTEN`) and reachable from the container
  (`pactl info` with pulseaudio-utils installed).

If GUI apps fail to open on macOS with a "cannot connect to display" error:

- Make sure XQuartz is running and "Allow connections from network clients"
  is enabled in its preferences (requires restarting XQuartz after changing).
- Re-run `xhost +localhost` on the Mac host — this needs to be done again
  after every XQuartz restart.
