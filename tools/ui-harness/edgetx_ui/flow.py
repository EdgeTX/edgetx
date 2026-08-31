"""Strict declarative scenarios for the EdgeTX simulator UI harness."""

from __future__ import annotations

import hashlib
import json
import os
import platform
import re
import shutil
import subprocess
import sys
import tempfile
import time
from dataclasses import asdict, dataclass, is_dataclass
from datetime import datetime, timezone
from pathlib import Path
from typing import Any, Callable, Dict, Iterable, List, Mapping, Optional, Sequence, Tuple

from .ppm import ArtifactDigest, digest_file
from .protocol import CAPABILITY_NAMES, Response
from .session import CaptureBundle, SimulatorSession


MAX_FLOW_STEPS = 1000
MAX_TIMEOUT_MS = 60_000
ARTIFACT_NAME = re.compile(r"^[A-Za-z0-9][A-Za-z0-9_-]{0,63}$")


@dataclass(frozen=True)
class TargetFlowSpec:
    keys: frozenset[str]
    switches: Mapping[str, Tuple[int, int]]
    analogs: Mapping[str, Tuple[int, int]]
    lcd: Tuple[int, int, int]


TX16S_SPEC = TargetFlowSpec(
    keys=frozenset(("EXIT", "ENTER", "PAGEUP", "PAGEDN", "MODEL", "TELE", "SYS")),
    switches={name: (-1, 1) for name in ("SA", "SB", "SC", "SD", "SE", "SF", "SG", "SH")},
    analogs={
        name: (0, 4096)
        for name in (
            "Rud", "Ele", "Thr", "Ail", "P1", "P2", "P3", "SL1", "SL2",
            "EXT1", "EXT2", "EXT3", "EXT4",
        )
    },
    lcd=(480, 272, 16),
)
TARGET_SPECS = {"tx16s": TX16S_SPEC}


@dataclass(frozen=True)
class FlowDefinition:
    source: Path
    schema: int
    target: str
    requires: Tuple[str, ...]
    steps: Tuple[Mapping[str, Any], ...]
    sha256: str


@dataclass(frozen=True)
class FlowRunResult:
    run_directory: Path
    manifest: Path
    success: bool


class FlowError(RuntimeError):
    """Base class for flow validation and execution failures."""


class FlowValidationError(FlowError):
    pass


class FlowExecutionError(FlowError):
    def __init__(self, message: str, result: FlowRunResult) -> None:
        super().__init__(message)
        self.result = result


def load_flow(path: Path) -> FlowDefinition:
    """Load one strict JSON flow and validate every step before launch."""

    source = path.resolve(strict=True)
    try:
        encoded = source.read_bytes()
        payload = json.loads(
            encoded.decode("utf-8"),
            object_pairs_hook=_unique_object,
            parse_constant=_reject_constant,
        )
    except (OSError, UnicodeError, json.JSONDecodeError, ValueError) as error:
        raise FlowValidationError("cannot load strict flow JSON: " + str(error)) from error
    if not isinstance(payload, dict):
        raise FlowValidationError("flow root must be an object")
    _exact_keys(payload, {"schema", "target", "requires", "steps"}, "flow")
    if payload["schema"] != 1 or isinstance(payload["schema"], bool):
        raise FlowValidationError("flow schema must be integer 1")
    target = payload["target"]
    if not isinstance(target, str) or target not in TARGET_SPECS:
        raise FlowValidationError("unsupported flow target: " + str(target))
    requires = _required_capabilities(payload["requires"])
    raw_steps = payload["steps"]
    if not isinstance(raw_steps, list) or not 1 <= len(raw_steps) <= MAX_FLOW_STEPS:
        raise FlowValidationError("flow steps must contain 1..1000 entries")
    steps = tuple(
        _validate_step(step, index, TARGET_SPECS[target])
        for index, step in enumerate(raw_steps)
    )
    if steps[0]["action"] != "wait-ready":
        raise FlowValidationError("step 0 must be the single wait-ready action")
    if sum(step["action"] == "wait-ready" for step in steps) != 1:
        raise FlowValidationError("wait-ready must appear exactly once")
    action_capabilities = {
        "rotate": "rotary",
        "tap": "touch",
        "drag": "touch",
        "set-switch": "switches",
        "set-analog": "analog",
        "clear-analog": "analog",
        "set-telemetry": "telemetry",
        "reload-lua": "lua",
        "restart": "warm_restart",
        "capture": "capture",
    }
    undeclared = sorted(
        {
            capability
            for step in steps
            for capability in (action_capabilities.get(step["action"]),)
            if capability is not None and capability not in requires
        }
    )
    if undeclared:
        raise FlowValidationError(
            "flow actions use undeclared capabilities: " + ", ".join(undeclared)
        )
    return FlowDefinition(
        source=source,
        schema=1,
        target=target,
        requires=requires,
        steps=steps,
        sha256=hashlib.sha256(encoded).hexdigest(),
    )


class FlowRunner:
    """Copy an immutable fixture and execute one validated scenario."""

    def __init__(
        self,
        flow: FlowDefinition,
        fixture_root: Path,
        runs_root: Path,
        executable: str,
        *,
        simulator_args: Sequence[str] = (),
        command_timeout: float = 10.0,
        session_factory: Callable[..., SimulatorSession] = SimulatorSession,
    ) -> None:
        self.flow = flow
        self.fixture_root = fixture_root.resolve(strict=True)
        self.runs_root = runs_root
        self.executable = executable
        self.simulator_args = tuple(simulator_args)
        self.command_timeout = command_timeout
        self.session_factory = session_factory
        self._validate_fixture()

    def run(self) -> FlowRunResult:
        source_hash = _tree_hash(self.fixture_root)
        self.runs_root.mkdir(parents=True, exist_ok=True)
        runs = self.runs_root.resolve(strict=True)
        if _is_below(runs, self.fixture_root):
            raise FlowValidationError("runs root must not be inside the fixture")
        run_directory = Path(
            tempfile.mkdtemp(prefix=self.flow.source.stem + "-", dir=str(runs))
        ).resolve(strict=True)
        settings = run_directory / "settings"
        sdcard = run_directory / "sdcard"
        artifacts = run_directory / "artifacts"
        checkpoints = artifacts / "checkpoints"
        shutil.copytree(self.fixture_root / "settings", settings, symlinks=False)
        shutil.copytree(self.fixture_root / "sdcard", sdcard, symlinks=False)
        checkpoints.mkdir(parents=True)

        args = _replace_simulator_path(self.simulator_args, "--settings", settings)
        args = _replace_simulator_path(args, "--storage", sdcard)
        session = self.session_factory(
            self.executable,
            artifacts,
            simulator_args=args,
            cwd=run_directory,
            request_timeout=self.command_timeout,
            stop_timeout=self.command_timeout,
            terminate_timeout=self.command_timeout,
            kill_timeout=self.command_timeout,
            required_capabilities=self.flow.requires,
            expected_target=self.flow.target,
            expected_lcd=TARGET_SPECS[self.flow.target].lcd,
        )
        started_at = _utc_now()
        started_monotonic = time.monotonic()
        step_records: List[Dict[str, Any]] = []
        retired_protocol: List[Dict[str, Any]] = []
        retired_stderr: List[str] = []
        failure: Optional[Dict[str, Any]] = None
        caught: Optional[BaseException] = None
        try:
            startup_timeout = self.flow.steps[0]["timeout_ms"] / 1000.0
            session.start(timeout=startup_timeout)
            for index, step in enumerate(self.flow.steps):
                step_started = time.monotonic()
                record: Dict[str, Any] = {
                    "index": index,
                    "action": step["action"],
                    "status": "running",
                }
                step_records.append(record)
                try:
                    if step["action"] == "restart-process":
                        previous = session
                        session = previous.restart_process(
                            self.fixture_root,
                            run_directory / "restarts",
                            timeout=step["timeout_ms"] / 1000.0,
                        )
                        retired_protocol.extend(previous.protocol_records)
                        if previous.recent_stderr:
                            retired_stderr.append(previous.recent_stderr)
                        result = {
                            "run_directory": str(session.fixture_run_directory)
                        }
                    else:
                        result = self._execute_step(session, step)
                    record["result"] = _json_value(result)
                    record["status"] = "passed"
                except BaseException as error:
                    record["status"] = "failed"
                    record["error"] = str(error)
                    failure = {
                        "step": index,
                        "action": step["action"],
                        "error_type": type(error).__name__,
                        "message": str(error),
                    }
                    raise
                finally:
                    record["elapsed_ms"] = round(
                        (time.monotonic() - step_started) * 1000.0, 3
                    )
        except BaseException as error:
            caught = error
        finally:
            try:
                session.stop(timeout=self.command_timeout)
            except BaseException as stop_error:
                if caught is None:
                    caught = stop_error
                    failure = {
                        "step": None,
                        "action": "clean-stop",
                        "error_type": type(stop_error).__name__,
                        "message": str(stop_error),
                    }
                else:
                    session.close()

        protocol_records = retired_protocol + list(
            getattr(session, "protocol_records", ())
        )
        protocol_path = run_directory / "protocol.jsonl"
        protocol_path.write_text(
            "".join(_canonical_json(record) + "\n" for record in protocol_records),
            encoding="utf-8",
            newline="\n",
        )
        stderr_path = run_directory / "stderr.log"
        current_stderr = getattr(session, "recent_stderr", "")
        stderr_text = "\n".join(
            item for item in (*retired_stderr, current_stderr) if item
        )
        stderr_path.write_text(
            stderr_text + ("\n" if stderr_text else ""), encoding="utf-8", newline="\n"
        )

        final_source_hash = _tree_hash(self.fixture_root)
        if final_source_hash != source_hash and caught is None:
            caught = FlowError("immutable fixture changed during the run")
            failure = {
                "step": None,
                "action": "fixture-integrity",
                "error_type": "FlowError",
                "message": str(caught),
            }

        artifact_digests = _artifact_digests(run_directory, artifacts)
        description = getattr(session, "description", None)
        manifest_payload: Dict[str, Any] = {
            "schema_version": 1,
            "success": caught is None,
            "flow": {
                "path": self.flow.source.as_posix(),
                "sha256": self.flow.sha256,
                "target": self.flow.target,
                "requires": list(self.flow.requires),
            },
            "fixture": {
                "path": self.fixture_root.as_posix(),
                "sha256": source_hash,
                "unchanged": final_source_hash == source_hash,
            },
            "environment": {
                "edgetx_commit": _git_commit(self.flow.source.parent),
                "platform": platform.platform(),
                "python": platform.python_version(),
            },
            "simulator": {
                "command": list(
                    getattr(session, "command", (self.executable, *args))
                ),
                "returncode": getattr(session, "returncode", None),
                "termination": getattr(session, "termination_stage", "unknown"),
                "target": getattr(description, "target", self.flow.target),
                "lcd": _json_value(getattr(description, "lcd", None)),
            },
            "started_at": started_at,
            "ended_at": _utc_now(),
            "elapsed_ms": round((time.monotonic() - started_monotonic) * 1000.0, 3),
            "steps": step_records,
            "protocol": protocol_records,
            "artifacts": artifact_digests,
            "failure": failure,
        }
        manifest_path = run_directory / "manifest.json"
        manifest_path.write_text(
            json.dumps(manifest_payload, ensure_ascii=False, indent=2, sort_keys=True) + "\n",
            encoding="utf-8",
            newline="\n",
        )
        result = FlowRunResult(run_directory, manifest_path, caught is None)
        if caught is not None:
            raise FlowExecutionError(str(caught), result) from caught
        return result

    def _execute_step(
        self, session: SimulatorSession, step: Mapping[str, Any]
    ) -> Any:
        action = step["action"]
        if action == "wait-ready":
            response = session.status_response
            return response.raw if response is not None else {"ready": True}
        if action == "press":
            return session.press(
                step["key"], duration=step["hold_ms"] / 1000.0,
                timeout=self.command_timeout,
            )
        if action == "long-press":
            return session.long_press(
                step["key"], duration=step["hold_ms"] / 1000.0,
                timeout=self.command_timeout,
            )
        if action == "rotate":
            return session.rotate(step["steps"], timeout=self.command_timeout)
        if action == "tap":
            return session.tap(
                step["x"], step["y"], duration=step["hold_ms"] / 1000.0,
                timeout=self.command_timeout,
            )
        if action == "drag":
            points = tuple((point["x"], point["y"]) for point in step["points"])
            duration = step["step_ms"] * (len(points) - 1) / 1000.0
            return session.drag(points, duration=duration, timeout=self.command_timeout)
        if action == "wait-ms":
            time.sleep(step["ms"] / 1000.0)
            return {"waited_ms": step["ms"]}
        if action == "wait-next-frame":
            return session.wait_next_frame(timeout=step["timeout_ms"] / 1000.0)
        if action == "set-switch":
            return session.set_switch(step["name"], step["position"], timeout=self.command_timeout)
        if action == "set-analog":
            return session.set_analog(step["name"], step["value"], timeout=self.command_timeout)
        if action == "clear-analog":
            return session.clear_analog(step["name"], timeout=self.command_timeout)
        if action == "set-telemetry":
            return session.set_telemetry(
                step["id"], step["sub_id"], step["instance"], step["value"],
                step["unit"], step["precision"], step.get("name"),
                timeout=self.command_timeout,
            )
        if action == "reload-lua":
            return session.reload_lua(timeout=step["timeout_ms"] / 1000.0)
        if action == "restart":
            return session.restart(timeout=step["timeout_ms"] / 1000.0)
        if action == "capture":
            return session.capture_png(
                "checkpoints/" + step["name"] + ".png",
                timeout=step["timeout_ms"] / 1000.0,
            )
        if action == "release-all":
            return session.release_all(timeout=self.command_timeout)
        raise AssertionError("validated flow action has no executor: " + str(action))

    def _validate_fixture(self) -> None:
        if not self.fixture_root.is_dir():
            raise FlowValidationError("fixture root must be a directory")
        for child in (self.fixture_root / "settings", self.fixture_root / "sdcard"):
            if not child.is_dir():
                raise FlowValidationError("fixture is missing directory: " + child.name)
        for current, directories, files in os.walk(self.fixture_root, followlinks=False):
            for name in (*directories, *files):
                if (Path(current) / name).is_symlink():
                    raise FlowValidationError("fixture must not contain symlinks")


def _validate_step(
    value: object, index: int, target: TargetFlowSpec
) -> Mapping[str, Any]:
    if not isinstance(value, dict):
        raise FlowValidationError("step " + str(index) + " must be an object")
    action = value.get("action")
    if not isinstance(action, str):
        raise FlowValidationError("step " + str(index) + " action must be a string")
    label = "step " + str(index) + " (" + action + ")"
    schemas = {
        "wait-ready": {"action", "timeout_ms"},
        "press": {"action", "key", "hold_ms"},
        "long-press": {"action", "key", "hold_ms"},
        "rotate": {"action", "steps"},
        "tap": {"action", "x", "y", "hold_ms"},
        "drag": {"action", "points", "step_ms"},
        "wait-ms": {"action", "ms"},
        "wait-next-frame": {"action", "timeout_ms"},
        "set-switch": {"action", "name", "position"},
        "set-analog": {"action", "name", "value"},
        "clear-analog": {"action", "name"},
        "set-telemetry": {"action", "id", "sub_id", "instance", "value", "unit", "precision"},
        "reload-lua": {"action", "timeout_ms"},
        "restart": {"action", "timeout_ms"},
        "restart-process": {"action", "timeout_ms"},
        "capture": {"action", "name", "format", "timeout_ms"},
        "release-all": {"action"},
    }
    optional = {"set-telemetry": {"name"}}
    if action not in schemas:
        raise FlowValidationError(label + " is unknown")
    _exact_keys(value, schemas[action], label, optional.get(action, set()))
    result = dict(value)
    if action in (
        "wait-ready", "wait-next-frame", "reload-lua", "restart",
        "restart-process", "capture",
    ):
        _integer(value["timeout_ms"], 1, MAX_TIMEOUT_MS, label + " timeout_ms")
    if action in ("press", "long-press"):
        if value["key"] not in target.keys:
            raise FlowValidationError(label + " key is unsupported")
        _integer(value["hold_ms"], 0, MAX_TIMEOUT_MS, label + " hold_ms")
    elif action == "rotate":
        steps = _integer(value["steps"], -128, 128, label + " steps")
        if steps == 0:
            raise FlowValidationError(label + " steps cannot be zero")
    elif action == "tap":
        _point(value["x"], value["y"], target, label)
        _integer(value["hold_ms"], 0, MAX_TIMEOUT_MS, label + " hold_ms")
    elif action == "drag":
        points = value["points"]
        if not isinstance(points, list) or not 2 <= len(points) <= 256:
            raise FlowValidationError(label + " points must contain 2..256 entries")
        for point_index, point in enumerate(points):
            if not isinstance(point, dict):
                raise FlowValidationError(label + " point must be an object")
            _exact_keys(point, {"x", "y"}, label + " point " + str(point_index))
            _point(point["x"], point["y"], target, label)
        _integer(value["step_ms"], 0, MAX_TIMEOUT_MS, label + " step_ms")
    elif action == "wait-ms":
        _integer(value["ms"], 0, MAX_TIMEOUT_MS, label + " ms")
    elif action == "set-switch":
        switch_range = target.switches.get(value["name"])
        if switch_range is None:
            raise FlowValidationError(label + " switch is unsupported")
        _integer(value["position"], *switch_range, label + " position")
    elif action == "set-analog":
        analog_range = target.analogs.get(value["name"])
        if analog_range is None:
            raise FlowValidationError(label + " analog is unsupported")
        _integer(value["value"], *analog_range, label + " value")
    elif action == "clear-analog":
        if value["name"] != "all" and value["name"] not in target.analogs:
            raise FlowValidationError(label + " analog is unsupported")
    elif action == "set-telemetry":
        _integer(value["id"], 1, 65535, label + " id")
        _integer(value["sub_id"], 0, 7, label + " sub_id")
        _integer(value["instance"], 0, 255, label + " instance")
        _integer(value["value"], -(1 << 31), (1 << 31) - 1, label + " value")
        _integer(value["unit"], 0, 29, label + " unit")
        _integer(value["precision"], 0, 2, label + " precision")
        name = value.get("name")
        if name is not None and (
            not isinstance(name, str) or re.fullmatch(r"[A-Za-z0-9_-]{1,4}", name) is None
        ):
            raise FlowValidationError(label + " telemetry name is invalid")
    elif action == "capture":
        if value["format"] != "png":
            raise FlowValidationError(label + " format must be png")
        if not isinstance(value["name"], str) or ARTIFACT_NAME.fullmatch(value["name"]) is None:
            raise FlowValidationError(label + " artifact name is invalid")
    return result


def _unique_object(pairs: Iterable[Tuple[str, Any]]) -> Dict[str, Any]:
    result: Dict[str, Any] = {}
    for key, value in pairs:
        if key in result:
            raise ValueError("duplicate JSON key: " + key)
        result[key] = value
    return result


def _reject_constant(value: str) -> None:
    raise ValueError("non-standard JSON constant: " + value)


def _exact_keys(
    value: Mapping[str, Any], required: set[str], label: str, optional: set[str] = set()
) -> None:
    actual = set(value)
    missing = required - actual
    extra = actual - required - optional
    if missing or extra:
        details = []
        if missing:
            details.append("missing " + ", ".join(sorted(missing)))
        if extra:
            details.append("unknown " + ", ".join(sorted(extra)))
        raise FlowValidationError(label + " has invalid fields: " + "; ".join(details))


def _required_capabilities(value: object) -> Tuple[str, ...]:
    if not isinstance(value, list):
        raise FlowValidationError("flow requires must be an array")
    result = tuple(value)
    if any(not isinstance(item, str) or item not in CAPABILITY_NAMES for item in result):
        raise FlowValidationError("flow contains an unknown required capability")
    if len(set(result)) != len(result):
        raise FlowValidationError("flow required capabilities cannot repeat")
    return result


def _integer(value: object, minimum: int, maximum: int, label: str) -> int:
    if isinstance(value, bool) or not isinstance(value, int) or not minimum <= value <= maximum:
        raise FlowValidationError(
            label + " must be an integer in " + str(minimum) + ".." + str(maximum)
        )
    return value


def _point(x: object, y: object, target: TargetFlowSpec, label: str) -> None:
    _integer(x, 0, target.lcd[0] - 1, label + " x")
    _integer(y, 0, target.lcd[1] - 1, label + " y")


def _replace_simulator_path(
    arguments: Sequence[str], option: str, path: Path
) -> Tuple[str, ...]:
    result = list(arguments)
    positions = [index for index, value in enumerate(result) if value == option]
    if len(positions) > 1:
        raise FlowValidationError("simulator arguments repeat " + option)
    if positions:
        position = positions[0]
        if position + 1 >= len(result) or result[position + 1].startswith("--"):
            raise FlowValidationError("simulator argument has no value: " + option)
        result[position + 1] = str(path)
    else:
        result.extend((option, str(path)))
    return tuple(result)


def _tree_hash(root: Path) -> str:
    digest = hashlib.sha256()
    for path in sorted(root.rglob("*"), key=lambda item: item.relative_to(root).as_posix()):
        relative = path.relative_to(root).as_posix().encode("utf-8")
        if path.is_symlink():
            raise FlowValidationError("fixture must not contain symlinks")
        if path.is_dir():
            digest.update(b"D\0" + relative + b"\0")
        elif path.is_file():
            digest.update(b"F\0" + relative + b"\0")
            with path.open("rb") as stream:
                for chunk in iter(lambda: stream.read(64 * 1024), b""):
                    digest.update(chunk)
        else:
            raise FlowValidationError("fixture contains an unsupported entry")
    return digest.hexdigest()


def _artifact_digests(run_directory: Path, artifacts: Path) -> List[Dict[str, Any]]:
    paths = [run_directory / "protocol.jsonl", run_directory / "stderr.log"]
    paths.extend(
        path
        for path in run_directory.iterdir()
        if path.is_file()
        and path.name not in ("manifest.json", "protocol.jsonl", "stderr.log")
    )
    paths.extend(path for path in artifacts.rglob("*") if path.is_file())
    restart_root = run_directory / "restarts"
    if restart_root.is_dir():
        paths.extend(
            path
            for path in restart_root.rglob("*")
            if path.is_file() and "artifacts" in path.relative_to(restart_root).parts
        )
    result = []
    for path in sorted(paths, key=lambda item: item.relative_to(run_directory).as_posix()):
        item = digest_file(path)
        result.append(
            {
                "path": path.relative_to(run_directory).as_posix(),
                "bytes": item.byte_count,
                "sha256": item.sha256,
            }
        )
    return result


def _json_value(value: Any) -> Any:
    if value is None or isinstance(value, (str, int, float, bool)):
        return value
    if isinstance(value, Response):
        return value.raw
    if isinstance(value, ArtifactDigest):
        return {"path": value.path.as_posix(), "bytes": value.byte_count, "sha256": value.sha256}
    if isinstance(value, CaptureBundle):
        return {
            "capture": _json_value(value.capture),
            "ppm": _json_value(value.ppm),
            "png": _json_value(value.png),
            "manifest": _json_value(value.manifest),
        }
    if is_dataclass(value):
        return _json_value(asdict(value))
    if isinstance(value, Mapping):
        return {str(key): _json_value(item) for key, item in value.items()}
    if isinstance(value, (list, tuple)):
        return [_json_value(item) for item in value]
    return str(value)


def _canonical_json(value: Any) -> str:
    return json.dumps(value, ensure_ascii=False, sort_keys=True, separators=(",", ":"))


def _git_commit(start: Path) -> str:
    result = subprocess.run(
        ["git", "rev-parse", "HEAD"], cwd=start, text=True,
        stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, check=False,
    )
    return result.stdout.strip() if result.returncode == 0 else "unknown"


def _utc_now() -> str:
    return datetime.now(timezone.utc).isoformat(timespec="milliseconds").replace("+00:00", "Z")


def _is_below(path: Path, parent: Path) -> bool:
    try:
        path.relative_to(parent)
    except ValueError:
        return False
    return True
