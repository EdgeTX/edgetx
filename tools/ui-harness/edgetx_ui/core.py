import json
import os
import subprocess
import sys
import tempfile
import time
from collections import deque
import select
from dataclasses import dataclass
from pathlib import Path
from typing import Any

from .png import convert_ppm_to_png


REPO_ROOT = Path(__file__).resolve().parents[3]


@dataclass(frozen=True)
class Target:
    name: str
    pcb: str
    pcbrev: str
    width: int
    height: int


TARGETS = {
    "tx16s": Target("tx16s", "X10", "TX16S", 480, 272),
}


KEYS = {
    "MENU",
    "EXIT",
    "ENTER",
    "PAGEUP",
    "PAGEDN",
    "UP",
    "DOWN",
    "LEFT",
    "RIGHT",
    "PLUS",
    "MINUS",
    "MODEL",
    "TELE",
    "SYS",
    "SHIFT",
    "BIND",
}


class HarnessError(RuntimeError):
    pass


def repo_root() -> Path:
    return REPO_ROOT


def target_config(target: str) -> Target:
    try:
        return TARGETS[target.lower()]
    except KeyError as exc:
        raise HarnessError(f"unsupported target: {target}") from exc


def build_dir(target: str) -> Path:
    return REPO_ROOT / "build" / "ui-harness" / target.lower()


def configure_command(target: str) -> list[str]:
    cfg = target_config(target)
    python_exec = project_python_executable()
    return [
        "cmake",
        "-S",
        str(REPO_ROOT),
        "-B",
        str(build_dir(cfg.name)),
        "-DCMAKE_TOOLCHAIN_FILE=cmake/toolchain/native.cmake",
        "-DEdgeTX_SUPERBUILD=OFF",
        "-DNATIVE_BUILD=ON",
        "-DDISABLE_COMPANION=ON",
        "-DEDGE_TX_BUILD_TESTS=OFF",
        f"-DPython3_EXECUTABLE={python_exec}",
        f"-DPCB={cfg.pcb}",
        f"-DPCBREV={cfg.pcbrev}",
    ]


def build_command(target: str) -> list[str]:
    return ["cmake", "--build", str(build_dir(target)), "--target", "simu"]


def build_simulator(target: str) -> dict[str, Any]:
    cfg = target_config(target)
    commands = [configure_command(cfg.name), build_command(cfg.name)]
    for command in commands:
        result = subprocess.run(
            command,
            cwd=REPO_ROOT,
            text=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            check=False,
        )
        if result.returncode != 0:
            raise HarnessError(
                "simulator build failed while running:\n"
                + " ".join(command)
                + "\n\n"
                + result.stdout[-4000:]
            )
    return {"target": cfg.name, "build_dir": str(build_dir(cfg.name)), "executable": str(find_simu_executable(cfg.name))}


def find_simu_executable(target: str) -> Path:
    base = build_dir(target)
    names = ["simu", "simu.exe"]
    for name in names:
        for candidate in base.rglob(name):
            if candidate.is_file() and os.access(candidate, os.X_OK):
                return candidate
    raise HarnessError(
        f"could not find built simulator executable under {base}; run `tools/ui-harness/edgetx-ui build {target}`"
    )


def git_commit() -> str:
    result = subprocess.run(
        ["git", "rev-parse", "--short", "HEAD"],
        cwd=REPO_ROOT,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.DEVNULL,
        check=False,
    )
    return result.stdout.strip() if result.returncode == 0 else "unknown"


def project_python_executable() -> str:
    venv_python = REPO_ROOT / ".venv" / "bin" / "python"
    if venv_python.exists():
        return str(venv_python)
    return sys.executable


class SdlAutomationSession:
    def __init__(
        self,
        target: str = "tx16s",
        sdcard: Path | None = None,
        settings: Path | None = None,
        width: int = 800,
        height: int = 600,
    ) -> None:
        self.target = target_config(target)
        self.sdcard = Path(sdcard) if sdcard else default_fixture_path("sdcard", self.target.name)
        self.settings = Path(settings) if settings else default_fixture_path("settings", self.target.name)
        self.width = width
        self.height = height
        self.process: subprocess.Popen[str] | None = None

    def start(self) -> dict[str, Any]:
        if self.process and self.process.poll() is None:
            return self.status()

        self.sdcard.mkdir(parents=True, exist_ok=True)
        self.settings.mkdir(parents=True, exist_ok=True)
        executable = find_simu_executable(self.target.name)
        self.process = subprocess.Popen(
            [
                str(executable),
                "--width",
                str(self.width),
                "--height",
                str(self.height),
                "--storage",
                str(self.sdcard),
                "--settings",
                str(self.settings),
                "--automation-stdio",
            ],
            cwd=REPO_ROOT,
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            bufsize=1,
        )
        time.sleep(0.5)
        return self.status()

    def stop(self) -> dict[str, Any]:
        if not self.process:
            return {"running": False}
        if self.process.poll() is None:
            try:
                self.command("stop", timeout=2.0)
            except HarnessError:
                self.process.terminate()
            try:
                self.process.wait(timeout=3.0)
            except subprocess.TimeoutExpired:
                self.process.kill()
        return {"running": False}

    def command(self, command: str, timeout: float = 5.0) -> dict[str, Any]:
        if not self.process or self.process.poll() is not None:
            raise HarnessError("simulator is not running")
        assert self.process.stdin is not None
        assert self.process.stdout is not None
        self.process.stdin.write(command + "\n")
        self.process.stdin.flush()

        deadline = time.monotonic() + timeout
        recent_lines: deque[str] = deque(maxlen=200)
        buffer = ""
        stdout_fd = self.process.stdout.fileno()
        while time.monotonic() < deadline:
            remaining = max(0.0, deadline - time.monotonic())
            ready, _, _ = select.select([stdout_fd], [], [], remaining)
            if not ready:
                break
            chunk = os.read(stdout_fd, 4096)
            if not chunk:
                break
            buffer += chunk.decode("utf-8", errors="replace")
            while "\n" in buffer:
                line, buffer = buffer.split("\n", 1)
                line = line.strip()
                if not line:
                    continue
                recent_lines.append(line)
                if line.startswith("{") and line.endswith("}"):
                    response = json.loads(line)
                    if not response.get("ok", False):
                        raise HarnessError(response.get("error", "simulator command failed"))
                    return response
        tail = "\n".join(recent_lines)
        if tail:
            raise HarnessError(
                f"timed out waiting for simulator response to `{command}`\nRecent output:\n{tail}"
            )
        raise HarnessError(f"timed out waiting for simulator response to `{command}`")

    def status(self) -> dict[str, Any]:
        response = self.command("status")
        return {
            "target": self.target.name,
            "backend": "sdl-automation",
            "running": bool(response.get("running")),
            "width": int(response.get("width", 0)),
            "height": int(response.get("height", 0)),
            "depth": int(response.get("depth", 0)),
            "sdcard": str(self.sdcard),
            "settings": str(self.settings),
        }

    def press(self, key: str, duration_ms: int = 120) -> dict[str, Any]:
        normalized = normalize_key(key)
        return self.command(f"press {normalized} {duration_ms}")

    def long_press(self, key: str, duration_ms: int = 800) -> dict[str, Any]:
        normalized = normalize_key(key)
        return self.command(f"long_press {normalized} {duration_ms}")

    def rotate(self, steps: int) -> dict[str, Any]:
        return self.command(f"rotate {int(steps)}")

    def touch(self, x: int, y: int, duration_ms: int = 120) -> dict[str, Any]:
        return self.command(f"touch {int(x)} {int(y)} {int(duration_ms)}")

    def wait(self, ms: int) -> dict[str, Any]:
        return self.command(f"wait {int(ms)}")

    def screenshot(self, name: str, out_dir: Path) -> dict[str, Any]:
        out_dir.mkdir(parents=True, exist_ok=True)
        safe_name = "".join(ch if ch.isalnum() or ch in "-_" else "_" for ch in name)
        png_path = out_dir / f"{safe_name}.png"
        metadata_path = out_dir / f"{safe_name}.json"
        with tempfile.NamedTemporaryFile(suffix=".ppm", delete=False) as temp:
            ppm_path = Path(temp.name)
        try:
            response = self.command(f"screenshot_ppm {ppm_path}", timeout=5.0)
            width, height = convert_ppm_to_png(ppm_path, png_path)
        finally:
            ppm_path.unlink(missing_ok=True)

        metadata = {
            "name": name,
            "target": self.target.name,
            "backend": "sdl-automation",
            "width": width,
            "height": height,
            "depth": int(response.get("depth", 0)),
            "path": str(png_path),
            "git_commit": git_commit(),
            "timestamp": time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime()),
        }
        metadata_path.write_text(json.dumps(metadata, indent=2) + "\n")
        return metadata | {"metadata": str(metadata_path)}


def normalize_key(key: str) -> str:
    normalized = key.upper()
    if normalized not in KEYS:
        raise HarnessError(f"unknown key `{key}`; expected one of {', '.join(sorted(KEYS))}")
    return normalized


def default_fixture_path(kind: str, target: str) -> Path:
    return REPO_ROOT / "tools" / "ui-harness" / "fixtures" / f"{kind}-{target}"


class HarnessService:
    def __init__(self) -> None:
        self.session: SdlAutomationSession | None = None

    def build(self, target: str = "tx16s") -> dict[str, Any]:
        return build_simulator(target)

    def start(
        self,
        target: str = "tx16s",
        sdcard: str | None = None,
        settings: str | None = None,
    ) -> dict[str, Any]:
        self.stop()
        self.session = SdlAutomationSession(
            target=target,
            sdcard=Path(sdcard) if sdcard else None,
            settings=Path(settings) if settings else None,
        )
        return self.session.start()

    def stop(self) -> dict[str, Any]:
        if not self.session:
            return {"running": False}
        result = self.session.stop()
        self.session = None
        return result

    def require_session(self) -> SdlAutomationSession:
        if not self.session:
            raise HarnessError("simulator has not been started")
        return self.session

    def status(self) -> dict[str, Any]:
        return self.require_session().status()

    def press(self, key: str, duration_ms: int = 120) -> dict[str, Any]:
        return self.require_session().press(key, duration_ms)

    def long_press(self, key: str, duration_ms: int = 800) -> dict[str, Any]:
        return self.require_session().long_press(key, duration_ms)

    def rotate(self, steps: int) -> dict[str, Any]:
        return self.require_session().rotate(steps)

    def touch(self, x: int, y: int, duration_ms: int = 120) -> dict[str, Any]:
        return self.require_session().touch(x, y, duration_ms)

    def wait(self, ms: int) -> dict[str, Any]:
        return self.require_session().wait(ms)

    def screenshot(self, name: str, out_dir: str | None = None) -> dict[str, Any]:
        output = Path(out_dir) if out_dir else REPO_ROOT / "build" / "ui-harness" / "screenshots"
        return self.require_session().screenshot(name, output)

    def run_flow(self, flow_path: str) -> dict[str, Any]:
        flow = json.loads(Path(flow_path).read_text())
        target = flow.get("target", "tx16s")
        output_dir = Path(flow.get("output", REPO_ROOT / "build" / "ui-harness" / "screenshots" / Path(flow_path).stem))
        self.start(target, flow.get("sdcard"), flow.get("settings"))
        screenshots = []
        failed_step = -1
        try:
            for index, step in enumerate(flow.get("steps", [])):
                failed_step = index
                self._run_step(step, output_dir, screenshots)
            return {"flow": flow_path, "screenshots": screenshots, "output": str(output_dir)}
        except Exception as exc:
            return {
                "flow": flow_path,
                "error": str(exc),
                "screenshots": screenshots,
                "failed_step": failed_step,
                "output": str(output_dir),
            }
        finally:
            self.stop()

    def _run_step(self, step: dict[str, Any], output_dir: Path, screenshots: list[dict[str, Any]]) -> None:
        if "wait" in step:
            value = step["wait"]
            self.wait(int(value.get("ms", 250) if isinstance(value, dict) else value))
        elif "press" in step:
            value = step["press"]
            if isinstance(value, str):
                self.press(value)
            else:
                self.press(value["key"], int(value.get("duration_ms", 120)))
        elif "long_press" in step:
            value = step["long_press"]
            if isinstance(value, str):
                self.long_press(value)
            else:
                self.long_press(value["key"], int(value.get("duration_ms", 800)))
        elif "rotate" in step:
            self.rotate(int(step["rotate"]))
        elif "touch" in step:
            value = step["touch"]
            self.touch(int(value["x"]), int(value["y"]), int(value.get("duration_ms", 120)))
        elif "screenshot" in step:
            screenshots.append(self.screenshot(str(step["screenshot"]), str(output_dir)))
        else:
            raise HarnessError(f"unknown flow step: {step}")
