"""Reproducible lifecycle and visual hardening for a real TX16S simulator."""

from __future__ import annotations

import hashlib
import json
import os
import platform
import shutil
import subprocess
import tempfile
from dataclasses import dataclass
from pathlib import Path
from typing import Any, Dict, List, Mapping, Optional, Sequence, Tuple

from .ppm import digest_file, read_ppm
from .session import SimulatorSession


DEFAULT_LIFECYCLE_CYCLES = 100
DEFAULT_PING_COUNT = 10_000
DEFAULT_LUA_RELOADS = 20
DEFAULT_WARM_RESTARTS = 20
DEFAULT_CAPTURE_COUNT = 20
TX16S_LCD = (480, 272, 16)
REQUIRED_CAPABILITIES = ("rotary", "touch", "lua", "capture", "warm_restart")


@dataclass(frozen=True)
class HardeningResult:
    run_directory: Path
    report: Path
    success: bool


class HardeningError(RuntimeError):
    """The hardening run could not be prepared."""


class HardeningExecutionError(HardeningError):
    """A hardening gate failed after its evidence directory was created."""

    def __init__(self, message: str, result: HardeningResult) -> None:
        super().__init__(message)
        self.result = result


class HardeningRunner:
    """Run bounded lifecycle and capture stress through the public host API."""

    def __init__(
        self,
        fixture_root: Path,
        runs_root: Path,
        executable: str,
        *,
        report_path: Optional[Path] = None,
        simulator_args: Sequence[str] = (),
        command_timeout: float = 10.0,
        lifecycle_cycles: int = DEFAULT_LIFECYCLE_CYCLES,
        ping_count: int = DEFAULT_PING_COUNT,
        lua_reloads: int = DEFAULT_LUA_RELOADS,
        warm_restarts: int = DEFAULT_WARM_RESTARTS,
        capture_count: int = DEFAULT_CAPTURE_COUNT,
        expected_target: str = "tx16s",
        expected_lcd: Tuple[int, int, int] = TX16S_LCD,
        session_factory: Any = SimulatorSession,
    ) -> None:
        self.fixture_root = fixture_root.resolve(strict=True)
        self.runs_root = runs_root
        self.executable = _executable_path(executable)
        self.report_path = report_path
        self.simulator_args = tuple(str(value) for value in simulator_args)
        self.command_timeout = _positive_timeout(command_timeout)
        self.lifecycle_cycles = _count(lifecycle_cycles, "lifecycle cycles")
        self.ping_count = _count(ping_count, "ping count")
        self.lua_reloads = _count(lua_reloads, "Lua reload count")
        self.warm_restarts = _count(warm_restarts, "warm restart count")
        self.capture_count = _count(capture_count, "capture count")
        if not expected_target:
            raise ValueError("expected target must not be empty")
        self.expected_target = expected_target
        self.expected_lcd = expected_lcd
        self.session_factory = session_factory
        _validate_fixture(self.fixture_root)

    def run(self) -> HardeningResult:
        source_hash = _tree_hash(self.fixture_root)
        self.runs_root.mkdir(parents=True, exist_ok=True)
        runs = self.runs_root.resolve(strict=True)
        if _is_below(runs, self.fixture_root):
            raise HardeningError("runs root must not be inside the fixture")
        run_directory = Path(
            tempfile.mkdtemp(prefix="tx16s-hardening-", dir=str(runs))
        ).resolve(strict=True)
        report_path = (
            self.report_path.resolve()
            if self.report_path is not None
            else run_directory / "hardening-report.json"
        )

        report: Dict[str, Any] = {
            "schema_version": 1,
            "success": False,
            "configuration": {
                "lifecycle_cycles": self.lifecycle_cycles,
                "ping_count": self.ping_count,
                "lua_reloads": self.lua_reloads,
                "warm_restarts": self.warm_restarts,
                "captures": self.capture_count,
                "target": self.expected_target,
                "lcd": {
                    "width": self.expected_lcd[0],
                    "height": self.expected_lcd[1],
                    "depth": self.expected_lcd[2],
                },
            },
            "environment": {
                "edgetx_commit": _git_commit(self.fixture_root),
                "platform": platform.platform(),
                "python": platform.python_version(),
            },
            "fixture": {
                "path": self.fixture_root.as_posix(),
                "sha256": source_hash,
                "unchanged": False,
            },
            "run_directory": run_directory.as_posix(),
            "lifecycle": {"completed": 0, "cycles": []},
            "stress": {},
            "cleanup": {"temporary_paths": [], "no_temporaries": False},
            "reap": {"all_reaped": False},
            "failure": None,
        }

        failure: Optional[BaseException] = None
        stage = "lifecycle"
        try:
            self._run_lifecycle(run_directory, report["lifecycle"])
            stage = "public-api-stress"
            self._run_public_api_stress(run_directory, report["stress"])
        except BaseException as error:
            failure = error
            report["failure"] = {
                "stage": stage,
                "error_type": type(error).__name__,
                "message": str(error),
            }

        final_hash = _tree_hash(self.fixture_root)
        report["fixture"]["unchanged"] = final_hash == source_hash
        temporary_paths = _temporary_paths(run_directory)
        report["cleanup"] = {
            "temporary_paths": [
                path.relative_to(run_directory).as_posix()
                for path in temporary_paths
            ],
            "no_temporaries": not temporary_paths,
        }
        cycles = report["lifecycle"]["cycles"]
        stress_reap = report.get("stress", {}).get("reap", {})
        all_reaped = all(
            item.get("returncode") is not None and not item.get("reader_threads_alive")
            for item in cycles
        ) and (
            not stress_reap
            or (
                stress_reap.get("returncode") is not None
                and not stress_reap.get("reader_threads_alive")
            )
        )
        report["reap"]["all_reaped"] = all_reaped

        if failure is None and final_hash != source_hash:
            failure = HardeningError("immutable fixture changed during hardening")
        if failure is None and temporary_paths:
            failure = HardeningError("temporary artifacts remain after hardening")
        if failure is None and not all_reaped:
            failure = HardeningError("a simulator process or reader thread was not reaped")
        if failure is not None and report["failure"] is None:
            report["failure"] = {
                "stage": "final-gates",
                "error_type": type(failure).__name__,
                "message": str(failure),
            }

        report["success"] = failure is None
        _write_report(report_path, report)
        result = HardeningResult(run_directory, report_path, failure is None)
        if failure is not None:
            raise HardeningExecutionError(str(failure), result) from failure
        return result

    def _run_lifecycle(
        self, run_directory: Path, lifecycle: Dict[str, Any]
    ) -> None:
        root = run_directory / "lifecycle"
        root.mkdir()
        for index in range(self.lifecycle_cycles):
            cycle_root, artifacts, arguments = self._copy_run(
                root, "cycle-" + str(index + 1).zfill(3) + "-"
            )
            session = self._session(
                cycle_root, artifacts, arguments, required_capabilities=()
            )
            try:
                session.start(timeout=self.command_timeout)
                session.stop(timeout=self.command_timeout)
            except BaseException:
                session.close()
                raise
            finally:
                process = session.process
                lifecycle["cycles"].append(
                    {
                        "index": index + 1,
                        "run": cycle_root.relative_to(run_directory).as_posix(),
                        "pid": process.pid if process is not None else None,
                        "returncode": session.returncode,
                        "termination": session.termination_stage,
                        "reader_threads_alive": session.reader_threads_alive,
                    }
                )
            lifecycle["completed"] = index + 1

    def _run_public_api_stress(
        self, run_directory: Path, payload: Dict[str, Any]
    ) -> None:
        root = run_directory / "stress"
        root.mkdir()
        session_root, artifacts, arguments = self._copy_run(root, "session-")
        checkpoints = artifacts / "checkpoints"
        checkpoints.mkdir()
        session = self._session(
            session_root,
            artifacts,
            arguments,
            required_capabilities=REQUIRED_CAPABILITIES,
        )
        payload.update({
            "run": session_root.relative_to(run_directory).as_posix(),
            "ping": {"requested": self.ping_count, "completed": 0},
            "lua": {"requested": self.lua_reloads, "completed": 0, "generations": []},
            "warm_restart": {
                "requested": self.warm_restarts,
                "completed": 0,
                "epochs": [],
                "display_sequences": [],
                "stale_completion_counts": [],
            },
            "visual": {
                "requested": self.capture_count,
                "completed": 0,
                "static": [],
                "identical": True,
                "changed": None,
                "changed_differs": False,
            },
            "reap": {},
        })
        first_ping_id: Optional[int] = None
        last_ping_id: Optional[int] = None
        try:
            session.start(timeout=self.command_timeout)
            for index in range(self.ping_count):
                response = session.ping(timeout=self.command_timeout)
                if first_ping_id is None:
                    first_ping_id = response.id
                last_ping_id = response.id
                payload["ping"]["completed"] = index + 1
            payload["ping"]["first_id"] = first_ping_id
            payload["ping"]["last_id"] = last_ping_id

            for index in range(self.lua_reloads):
                result = session.reload_lua(timeout=self.command_timeout)
                expected = index + 1
                if result.generation != expected or result.state != "running":
                    raise HardeningError("Lua reload generation/state is not deterministic")
                payload["lua"]["generations"].append(result.generation)
                payload["lua"]["completed"] = expected

            status = session.read_status(timeout=self.command_timeout)
            previous_epoch = status.epoch
            previous_sequence = status.display_sequence
            for index in range(self.warm_restarts):
                restarted = session.restart(timeout=self.command_timeout)
                status = session.read_status(timeout=self.command_timeout)
                if restarted.epoch != previous_epoch + 1:
                    raise HardeningError("warm restart did not advance exactly one epoch")
                if restarted.display_sequence <= previous_sequence:
                    raise HardeningError("warm restart display sequence did not advance")
                if status.stale_completion_count != 0:
                    raise HardeningError("stale completion observed after warm restart")
                previous_epoch = restarted.epoch
                previous_sequence = restarted.display_sequence
                payload["warm_restart"]["epochs"].append(restarted.epoch)
                payload["warm_restart"]["display_sequences"].append(
                    restarted.display_sequence
                )
                payload["warm_restart"]["stale_completion_counts"].append(
                    status.stale_completion_count
                )
                payload["warm_restart"]["completed"] = index + 1

            self._prepare_visual_state(session)
            baseline: Optional[str] = None
            for index in range(self.capture_count):
                relative = "checkpoints/static-" + str(index + 1).zfill(2) + ".ppm"
                artifact = session.capture_ppm(relative, timeout=self.command_timeout)
                path = artifacts / Path(relative)
                image = read_ppm(path)
                digest = digest_file(path)
                rgb_sha256 = hashlib.sha256(image.rgb).hexdigest()
                if baseline is None:
                    baseline = rgb_sha256
                elif rgb_sha256 != baseline:
                    payload["visual"]["identical"] = False
                payload["visual"]["static"].append(
                    {
                        "path": path.relative_to(run_directory).as_posix(),
                        "bytes": digest.byte_count,
                        "sha256": digest.sha256,
                        "rgb_sha256": rgb_sha256,
                        "display_sequence": artifact.display_sequence,
                    }
                )
                payload["visual"]["completed"] = index + 1
            if not payload["visual"]["identical"]:
                raise HardeningError("static TX16S captures are not identical")

            if self.capture_count:
                session.rotate(1, timeout=self.command_timeout)
                changed = session.capture_ppm(
                    "checkpoints/changed.ppm", timeout=self.command_timeout
                )
                changed_path = artifacts / "checkpoints" / "changed.ppm"
                changed_image = read_ppm(changed_path)
                changed_digest = digest_file(changed_path)
                changed_rgb = hashlib.sha256(changed_image.rgb).hexdigest()
                payload["visual"]["changed"] = {
                    "path": changed_path.relative_to(run_directory).as_posix(),
                    "bytes": changed_digest.byte_count,
                    "sha256": changed_digest.sha256,
                    "rgb_sha256": changed_rgb,
                    "display_sequence": changed.display_sequence,
                }
                payload["visual"]["changed_differs"] = changed_rgb != baseline
                if not payload["visual"]["changed_differs"]:
                    raise HardeningError("deliberate visible-state capture did not change")

            final_status = session.read_status(timeout=self.command_timeout)
            if final_status.stale_completion_count != 0:
                raise HardeningError("stale completion observed at end of stress run")
            payload["final_status"] = {
                "epoch": final_status.epoch,
                "display_sequence": final_status.display_sequence,
                "stale_completion_count": final_status.stale_completion_count,
            }
            session.stop(timeout=self.command_timeout)
        except BaseException:
            session.close()
            raise
        finally:
            process = session.process
            payload["reap"] = {
                "pid": process.pid if process is not None else None,
                "returncode": session.returncode,
                "termination": session.termination_stage,
                "reader_threads_alive": session.reader_threads_alive,
            }

    @staticmethod
    def _prepare_visual_state(session: SimulatorSession) -> None:
        """Enter the full-screen model manager, avoiding the home-screen RTC."""

        session.press("ENTER", duration=0.12)
        session.tap(40, 70, duration=0.08)

    def _copy_run(
        self, parent: Path, prefix: str
    ) -> Tuple[Path, Path, Tuple[str, ...]]:
        run = Path(tempfile.mkdtemp(prefix=prefix, dir=str(parent))).resolve(strict=True)
        settings = run / "settings"
        storage = run / "sdcard"
        artifacts = run / "artifacts"
        try:
            shutil.copytree(self.fixture_root / "settings", settings, symlinks=False)
            shutil.copytree(self.fixture_root / "sdcard", storage, symlinks=False)
            artifacts.mkdir()
        except BaseException:
            shutil.rmtree(run, ignore_errors=True)
            raise
        arguments = _replace_option(self.simulator_args, "--settings", str(settings))
        arguments = _replace_option(arguments, "--storage", str(storage))
        return run, artifacts, arguments

    def _session(
        self,
        run: Path,
        artifacts: Path,
        arguments: Sequence[str],
        *,
        required_capabilities: Sequence[str],
    ) -> SimulatorSession:
        return self.session_factory(
            self.executable,
            artifacts,
            simulator_args=arguments,
            cwd=run,
            request_timeout=self.command_timeout,
            stop_timeout=self.command_timeout,
            terminate_timeout=self.command_timeout,
            kill_timeout=self.command_timeout,
            reader_join_timeout=self.command_timeout,
            required_capabilities=required_capabilities,
            expected_target=self.expected_target,
            expected_lcd=self.expected_lcd,
        )


def _replace_option(arguments: Sequence[str], option: str, value: str) -> Tuple[str, ...]:
    result = list(arguments)
    positions = [index for index, item in enumerate(result) if item == option]
    if len(positions) > 1:
        raise ValueError("simulator arguments repeat " + option)
    if positions:
        index = positions[0]
        if index + 1 >= len(result) or result[index + 1].startswith("--"):
            raise ValueError("simulator argument has no value: " + option)
        result[index + 1] = value
    else:
        result.extend((option, value))
    return tuple(result)


def _executable_path(value: str) -> str:
    path = Path(value)
    if path.is_absolute():
        return str(path)
    if path.parent != Path("."):
        return str(path.resolve(strict=True))
    discovered = shutil.which(value)
    return discovered if discovered is not None else value


def _validate_fixture(root: Path) -> None:
    for child in (root / "settings", root / "sdcard"):
        if not child.is_dir():
            raise ValueError("fixture is missing directory: " + child.name)
    for path in root.rglob("*"):
        if path.is_symlink():
            raise ValueError("fixture must not contain symlinks")


def _tree_hash(root: Path) -> str:
    digest = hashlib.sha256()
    for path in sorted(
        root.rglob("*"), key=lambda item: item.relative_to(root).as_posix()
    ):
        relative = path.relative_to(root).as_posix().encode("utf-8")
        if path.is_symlink():
            raise ValueError("fixture must not contain symlinks")
        if path.is_dir():
            digest.update(b"D\0" + relative + b"\0")
        elif path.is_file():
            digest.update(b"F\0" + relative + b"\0")
            with path.open("rb") as stream:
                for chunk in iter(lambda: stream.read(64 * 1024), b""):
                    digest.update(chunk)
        else:
            raise ValueError("fixture contains an unsupported entry")
    return digest.hexdigest()


def _temporary_paths(root: Path) -> List[Path]:
    return sorted(
        (
            path
            for path in root.rglob("*")
            if path.is_file()
            and (".tmp-v1-" in path.name or path.name.endswith(".tmp-ui-harness"))
        ),
        key=lambda path: path.relative_to(root).as_posix(),
    )


def _write_report(path: Path, payload: Mapping[str, Any]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    encoded = (
        json.dumps(payload, ensure_ascii=False, indent=2, sort_keys=True) + "\n"
    ).encode("utf-8")
    temporary = path.with_name("." + path.name + ".tmp-ui-harness")
    try:
        with temporary.open("wb") as stream:
            stream.write(encoded)
            stream.flush()
            os.fsync(stream.fileno())
        os.replace(temporary, path)
    finally:
        try:
            temporary.unlink()
        except FileNotFoundError:
            pass


def _git_commit(start: Path) -> str:
    result = subprocess.run(
        ["git", "rev-parse", "HEAD"],
        cwd=start,
        stdout=subprocess.PIPE,
        stderr=subprocess.DEVNULL,
        text=True,
        check=False,
    )
    return result.stdout.strip() if result.returncode == 0 else "unknown"


def _positive_timeout(value: float) -> float:
    if not isinstance(value, (int, float)) or value <= 0 or value > 60:
        raise ValueError("command timeout must be in (0, 60]")
    return float(value)


def _count(value: int, label: str) -> int:
    if isinstance(value, bool) or not isinstance(value, int) or value < 0:
        raise ValueError(label + " must be a non-negative integer")
    return value


def _is_below(path: Path, parent: Path) -> bool:
    try:
        path.relative_to(parent)
    except ValueError:
        return False
    return True
