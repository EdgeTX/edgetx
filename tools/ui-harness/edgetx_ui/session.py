"""Cross-platform subprocess lifecycle for EdgeTX simulator automation."""

from __future__ import annotations

import hashlib
import json
import math
import os
import queue
import re
import shutil
import subprocess
import tempfile
import threading
import time
from collections import deque
from dataclasses import dataclass
from pathlib import Path, PurePosixPath
from typing import Any, BinaryIO, Deque, Dict, Mapping, Optional, Sequence, Tuple, Union

from .protocol import (
    CAPABILITY_NAMES,
    CaptureArtifact,
    Description,
    Event,
    FrameBarrier,
    LuaReload,
    NamedRange,
    MAX_RECORD_BYTES,
    ProtocolViolation,
    PROTOCOL_VERSION,
    Response,
    Status,
    UINT64_MAX,
    decode_capture,
    decode_description,
    decode_frame,
    decode_lua_reload,
    decode_restart,
    decode_status,
    encode_request,
    parse_message,
)
from .ppm import (
    ArtifactDigest,
    convert_ppm_to_png,
    digest_file,
    read_ppm,
    write_json_sidecar,
)


READ_CHUNK_BYTES = 4096
MAX_STDERR_LINES = 200
MAX_STDERR_BYTES = 256 * 1024
MAX_EVENTS = 64
MAX_PROTOCOL_RECORDS = 4096
MAX_COMMAND_TIMEOUT = 60.0
TELEMETRY_UNIT_MAX = 29
TELEMETRY_LABEL_PATTERN = re.compile(r"^[A-Za-z0-9_-]{1,4}$")
REQUIRED_STARTUP_COMMANDS = frozenset(("ping", "status", "describe", "stop"))


@dataclass(frozen=True)
class CaptureBundle:
    capture: CaptureArtifact
    ppm: ArtifactDigest
    png: ArtifactDigest
    manifest: ArtifactDigest


@dataclass(frozen=True)
class _WriteWork:
    record: bytes
    completion: queue.Queue[Optional[SessionError]]


class SessionError(RuntimeError):
    """Base exception for simulator session failures."""


class ProtocolFailure(SessionError):
    """The simulator emitted malformed or miscorrelated protocol data."""


class _ProtocolClosed(ProtocolFailure):
    pass


class ProcessExited(SessionError):
    """The simulator exited before completing a request."""


class RequestTimeout(SessionError):
    """A request did not receive its terminal response before its deadline."""


class StartupMismatch(ProtocolFailure):
    """The simulator target or advertised capabilities do not match the run."""


class CommandFailed(SessionError):
    """The simulator returned a correlated error response."""

    def __init__(self, response: Response, diagnostics: str) -> None:
        message = (
            "simulator command failed for request "
            + str(response.id)
            + ": "
            + str(response.error_code)
            + ": "
            + str(response.error_message)
        )
        if diagnostics:
            message += "\nRecent simulator stderr:\n" + diagnostics
        super().__init__(message)
        self.response = response


class _BoundedDiagnostics:
    def __init__(self) -> None:
        self._lines: Deque[Tuple[str, int]] = deque()
        self._bytes = 0
        self._lock = threading.Lock()

    def append(self, raw_line: bytes) -> None:
        if len(raw_line) > MAX_STDERR_BYTES:
            raw_line = raw_line[-MAX_STDERR_BYTES:]
        text = raw_line.decode("utf-8", errors="replace")
        size = len(raw_line)
        with self._lock:
            self._lines.append((text, size))
            self._bytes += size
            while (
                len(self._lines) > MAX_STDERR_LINES
                or self._bytes > MAX_STDERR_BYTES
            ):
                _, removed_size = self._lines.popleft()
                self._bytes -= removed_size

    def text(self) -> str:
        with self._lock:
            return "\n".join(line for line, _ in self._lines)

    def counts(self) -> Tuple[int, int]:
        with self._lock:
            return len(self._lines), self._bytes


_ResponseItem = Union[Response, SessionError]


class SimulatorSession:
    """Own one simulator process and serialize correlated protocol requests."""

    def __init__(
        self,
        executable: Union[str, os.PathLike[str]],
        output_root: Union[str, os.PathLike[str]],
        *,
        simulator_args: Sequence[Union[str, os.PathLike[str]]] = (),
        cwd: Optional[Union[str, os.PathLike[str]]] = None,
        env: Optional[Mapping[str, str]] = None,
        request_timeout: float = 5.0,
        stop_timeout: float = 3.0,
        terminate_timeout: float = 2.0,
        kill_timeout: float = 2.0,
        reader_join_timeout: float = 2.0,
        protocol_sink: Optional[Union[str, os.PathLike[str]]] = None,
        required_capabilities: Sequence[str] = (),
        expected_target: Optional[str] = None,
        expected_lcd: Optional[Tuple[int, int, int]] = None,
    ) -> None:
        self._executable = os.fspath(executable)
        self._output_root = Path(output_root)
        self._simulator_args = tuple(os.fspath(value) for value in simulator_args)
        self._cwd = os.fspath(cwd) if cwd is not None else None
        self._env = dict(env) if env is not None else None
        self._protocol_sink_path = (
            Path(protocol_sink) if protocol_sink is not None else None
        )
        self._request_timeout = _validated_timeout(request_timeout, "request")
        self._stop_timeout = _validated_timeout(stop_timeout, "stop")
        self._terminate_timeout = _validated_timeout(
            terminate_timeout, "terminate"
        )
        self._kill_timeout = _validated_timeout(kill_timeout, "kill")
        self._reader_join_timeout = _validated_timeout(
            reader_join_timeout, "reader join"
        )
        self._required_capabilities = _validated_capabilities(
            required_capabilities
        )
        if expected_target is not None and (
            not isinstance(expected_target, str) or not expected_target
        ):
            raise ValueError("expected target must be a non-empty string")
        self._expected_target = expected_target
        self._expected_lcd = _validated_lcd(expected_lcd)

        self._process: Optional[subprocess.Popen[bytes]] = None
        self._stdout_thread: Optional[threading.Thread] = None
        self._stderr_thread: Optional[threading.Thread] = None
        self._writer_thread: Optional[threading.Thread] = None
        self._writer_queue: "queue.Queue[object]" = queue.Queue(maxsize=1)
        self._writer_stop = threading.Event()
        self._writer_timed_out = False
        self._stdout_closed = threading.Event()
        self._stderr_closed = threading.Event()
        self._diagnostics = _BoundedDiagnostics()
        self._events: Deque[Event] = deque(maxlen=MAX_EVENTS)
        self._protocol_records: Deque[Dict[str, Any]] = deque(
            maxlen=MAX_PROTOCOL_RECORDS
        )
        self._protocol_record_count = 0
        self._protocol_sha256 = hashlib.sha256()
        self._protocol_sink: Optional[BinaryIO] = None

        self._state_lock = threading.RLock()
        self._request_lock = threading.Lock()
        self._shutdown_lock = threading.Lock()
        self._pending_id: Optional[int] = None
        self._pending_queue: Optional[queue.Queue[_ResponseItem]] = None
        self._next_request_id = 1
        self._failure: Optional[SessionError] = None
        self._closing = False
        self._closed = False
        self._stop_response: Optional[Response] = None
        self._startup_ping: Optional[Response] = None
        self._description_response: Optional[Response] = None
        self._description: Optional[Description] = None
        self._status_response: Optional[Response] = None
        self._status: Optional[Status] = None
        self._termination_stage = "not-started"
        self._fixture_run_directory: Optional[Path] = None

    @property
    def process(self) -> Optional[subprocess.Popen[bytes]]:
        return self._process

    @property
    def returncode(self) -> Optional[int]:
        return self._process.returncode if self._process is not None else None

    @property
    def termination_stage(self) -> str:
        return self._termination_stage

    @property
    def recent_stderr(self) -> str:
        return self._diagnostics.text()

    @property
    def stderr_counts(self) -> Tuple[int, int]:
        return self._diagnostics.counts()

    @property
    def events(self) -> Tuple[Event, ...]:
        with self._state_lock:
            return tuple(self._events)

    @property
    def protocol_records(self) -> Tuple[Dict[str, Any], ...]:
        """Return the bounded diagnostic tail of protocol traffic."""

        with self._state_lock:
            return tuple(dict(record) for record in self._protocol_records)

    @property
    def protocol_record_count(self) -> int:
        with self._state_lock:
            return self._protocol_record_count

    @property
    def protocol_sha256(self) -> str:
        with self._state_lock:
            return self._protocol_sha256.hexdigest()

    @property
    def protocol_records_dropped(self) -> int:
        with self._state_lock:
            return self._protocol_record_count - len(self._protocol_records)

    @property
    def protocol_sink_path(self) -> Optional[Path]:
        return self._protocol_sink_path

    @property
    def startup_ping(self) -> Optional[Response]:
        return self._startup_ping

    @property
    def description_response(self) -> Optional[Response]:
        return self._description_response

    @property
    def description(self) -> Optional[Description]:
        return self._description

    @property
    def status_response(self) -> Optional[Response]:
        return self._status_response

    @property
    def status(self) -> Optional[Status]:
        return self._status

    @property
    def reader_threads_alive(self) -> bool:
        return any(
            thread is not None and thread.is_alive()
            for thread in (self._stdout_thread, self._stderr_thread)
        )

    @property
    def writer_thread_alive(self) -> bool:
        return self._writer_thread is not None and self._writer_thread.is_alive()

    @property
    def fixture_run_directory(self) -> Optional[Path]:
        return self._fixture_run_directory

    @property
    def command(self) -> Tuple[str, ...]:
        return (
            self._executable,
            *self._simulator_args,
            "--automation-stdio",
            "--automation-output",
            str(self._output_root),
        )

    def start(self, *, timeout: Optional[float] = None) -> Response:
        """Launch, validate discovery, and wait for the first display frame."""

        with self._state_lock:
            if self._process is not None:
                raise SessionError("simulator session can only be started once")

        startup_timeout = (
            self._request_timeout
            if timeout is None
            else _validated_timeout(timeout, "startup")
        )

        try:
            resolved_output = self._output_root.resolve(strict=True)
        except OSError as error:
            raise SessionError("automation output directory does not exist") from error
        if not resolved_output.is_dir():
            raise SessionError("automation output path is not a directory")
        self._output_root = resolved_output

        if self._protocol_sink_path is not None:
            sink_path = self._protocol_sink_path.resolve()
            if not sink_path.parent.is_dir():
                raise SessionError("protocol evidence parent directory does not exist")
            try:
                self._protocol_sink = sink_path.open("xb", buffering=64 * 1024)
            except OSError as error:
                raise SessionError(
                    "cannot create protocol evidence sink: " + str(error)
                ) from error
            self._protocol_sink_path = sink_path

        try:
            process = subprocess.Popen(
                list(self.command),
                cwd=self._cwd,
                env=self._env,
                stdin=subprocess.PIPE,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=False,
                bufsize=0,
                shell=False,
            )
        except OSError as error:
            self._close_protocol_sink()
            raise SessionError("cannot launch simulator: " + str(error)) from error

        with self._state_lock:
            self._process = process
            self._termination_stage = "running"
        self._stdout_thread = threading.Thread(
            target=self._read_stdout,
            name="edgetx-automation-stdout",
            daemon=True,
        )
        self._stderr_thread = threading.Thread(
            target=self._read_stderr,
            name="edgetx-automation-stderr",
            daemon=True,
        )
        self._writer_thread = threading.Thread(
            target=self._write_stdin,
            name="edgetx-automation-stdin",
            daemon=True,
        )
        try:
            self._stdout_thread.start()
            self._stderr_thread.start()
            self._writer_thread.start()
        except BaseException:
            self._shutdown(send_stop=False, raise_errors=False)
            raise

        deadline = time.monotonic() + startup_timeout
        try:
            self._startup_ping = self.ping(
                timeout=self._startup_remaining(deadline)
            )
            self._description_response = self.request(
                "describe", timeout=self._startup_remaining(deadline)
            )
            try:
                self._description = decode_description(
                    self._description_response
                )
            except ProtocolViolation as error:
                raise ProtocolFailure(str(error)) from error
            self._validate_description(self._description)

            while True:
                try:
                    self._status_response = self.request(
                        "status", timeout=self._startup_remaining(deadline)
                    )
                except RequestTimeout as error:
                    raise RequestTimeout(
                        self._message_with_context(
                            "timed out waiting for first-frame readiness", None
                        )
                    ) from error
                try:
                    self._status = decode_status(self._status_response)
                except ProtocolViolation as error:
                    raise ProtocolFailure(str(error)) from error
                self._validate_status(self._status, self._description)

                if self._status.phase == "ready":
                    if (
                        not self._status.running
                        or self._status.epoch == 0
                        or self._status.display_sequence == 0
                    ):
                        raise ProtocolFailure(
                            "ready status does not own a running first frame"
                        )
                    return self._status_response
                if self._status.phase == "stopped":
                    raise ProtocolFailure(
                        "simulator stopped before first-frame readiness"
                    )
        except BaseException:
            self._shutdown(send_stop=False, raise_errors=False)
            raise

    def ping(self, *, timeout: Optional[float] = None) -> Response:
        return self.request("ping", timeout=timeout)

    def read_status(self, *, timeout: Optional[float] = None) -> Status:
        """Read and validate a fresh status snapshot."""

        description = self._require_command("status")
        response = self.request("status", timeout=timeout)
        try:
            status = decode_status(response)
        except ProtocolViolation as error:
            raise ProtocolFailure(str(error)) from error
        self._validate_status(status, description)
        with self._state_lock:
            self._status_response = response
            self._status = status
        return status

    def key_down(
        self, key: str, *, timeout: Optional[float] = None
    ) -> Response:
        self._validate_key(key, "key-down")
        return self.request("key-down", key, timeout=timeout)

    def key_up(
        self, key: str, *, timeout: Optional[float] = None
    ) -> Response:
        self._validate_key(key, "key-up")
        return self.request("key-up", key, timeout=timeout)

    def rotate(
        self, steps: int, *, timeout: Optional[float] = None
    ) -> Response:
        self._require_command("rotate", capability="rotary")
        _validated_integer(steps, -128, 128, "rotary steps", exclude_zero=True)
        return self.request("rotate", str(steps), timeout=timeout)

    def touch_down(
        self, x: int, y: int, *, timeout: Optional[float] = None
    ) -> Response:
        x, y = self._validate_touch_point(x, y, "touch-down")
        return self.request("touch-down", str(x), str(y), timeout=timeout)

    def touch_move(
        self, x: int, y: int, *, timeout: Optional[float] = None
    ) -> Response:
        x, y = self._validate_touch_point(x, y, "touch-move")
        return self.request("touch-move", str(x), str(y), timeout=timeout)

    def touch_up(self, *, timeout: Optional[float] = None) -> Response:
        self._require_command("touch-up", capability="touch")
        return self.request("touch-up", timeout=timeout)

    def release_all(self, *, timeout: Optional[float] = None) -> Response:
        self._require_command("release-all")
        return self.request("release-all", timeout=timeout)

    def set_switch(
        self, name: str, position: int, *, timeout: Optional[float] = None
    ) -> Response:
        description = self._require_command("set-switch", capability="switches")
        _require_named_range(description.switches, name, "switch")
        position = _validated_integer(position, -1, 1, "switch position")
        return self.request("set-switch", name, str(position), timeout=timeout)

    def set_analog(
        self, name: str, value: int, *, timeout: Optional[float] = None
    ) -> Response:
        description = self._require_command("set-analog", capability="analog")
        analog = _require_named_range(description.analogs, name, "analog")
        value = _validated_integer(
            value, analog.minimum, analog.maximum, "analog value"
        )
        return self.request("set-analog", name, str(value), timeout=timeout)

    def clear_analog(
        self, name: str = "all", *, timeout: Optional[float] = None
    ) -> Response:
        description = self._require_command("clear-analog", capability="analog")
        if name != "all":
            _require_named_range(description.analogs, name, "analog")
        return self.request("clear-analog", name, timeout=timeout)

    def set_telemetry(
        self,
        sensor_id: int,
        sub_id: int,
        instance: int,
        value: int,
        unit: int,
        precision: int,
        name: Optional[str] = None,
        *,
        timeout: Optional[float] = None,
    ) -> Response:
        self._require_command("set-telemetry", capability="telemetry")
        sensor_id = _validated_integer(sensor_id, 1, 65535, "telemetry id")
        sub_id = _validated_integer(sub_id, 0, 7, "telemetry sub-id")
        instance = _validated_integer(instance, 0, 255, "telemetry instance")
        value = _validated_integer(
            value, -(1 << 31), (1 << 31) - 1, "telemetry value"
        )
        unit = _validated_integer(unit, 0, TELEMETRY_UNIT_MAX, "telemetry unit")
        precision = _validated_integer(precision, 0, 2, "telemetry precision")
        arguments = [
            str(sensor_id),
            str(sub_id),
            str(instance),
            str(value),
            str(unit),
            str(precision),
        ]
        if name is not None:
            if not isinstance(name, str) or not TELEMETRY_LABEL_PATTERN.fullmatch(
                name
            ):
                raise ValueError(
                    "telemetry name must match [A-Za-z0-9_-]{1,4}"
                )
            arguments.append(name)
        return self.request("set-telemetry", *arguments, timeout=timeout)

    def reload_lua(self, *, timeout: Optional[float] = None) -> LuaReload:
        self._require_command("reload-lua", capability="lua")
        response = self.request("reload-lua", timeout=timeout)
        try:
            return decode_lua_reload(response)
        except ProtocolViolation as error:
            raise ProtocolFailure(str(error)) from error

    def restart(self, *, timeout: Optional[float] = None) -> FrameBarrier:
        self._require_command("restart", capability="warm_restart")
        before = self.read_status(timeout=timeout)
        response = self.request("restart", timeout=timeout)
        try:
            restarted = decode_restart(response)
        except ProtocolViolation as error:
            raise ProtocolFailure(str(error)) from error
        if restarted.epoch <= before.epoch:
            raise ProtocolFailure("warm restart did not advance the session epoch")
        if restarted.display_sequence <= before.display_sequence:
            raise ProtocolFailure(
                "warm restart did not preserve the process display sequence"
            )
        after = self.read_status(timeout=timeout)
        if (
            after.phase != "ready"
            or not after.running
            or after.epoch != restarted.epoch
            or after.display_sequence < restarted.display_sequence
        ):
            raise ProtocolFailure("warm restart result and ready status disagree")
        return restarted

    def restart_process(
        self,
        fixture_root: Union[str, os.PathLike[str]],
        runs_root: Union[str, os.PathLike[str]],
        *,
        timeout: Optional[float] = None,
    ) -> "SimulatorSession":
        """Cold-restart into a new process and fresh writable fixture copy."""

        fixture = Path(fixture_root).resolve(strict=True)
        settings_source = _validated_fixture_directory(fixture / "settings")
        storage_source = _validated_fixture_directory(fixture / "sdcard")
        runs = Path(runs_root)
        runs.mkdir(parents=True, exist_ok=True)
        runs = runs.resolve(strict=True)
        if _path_is_below(runs, settings_source) or _path_is_below(
            runs, storage_source
        ):
            raise ValueError("runs root must not be inside the fixture template")

        simulator_args = _replace_option(
            self._simulator_args, "--settings", "{settings}"
        )
        simulator_args = _replace_option(
            simulator_args, "--storage", "{storage}"
        )

        self.stop(timeout=timeout)

        run_directory: Optional[Path] = None
        replacement: Optional[SimulatorSession] = None
        try:
            run_directory = Path(
                tempfile.mkdtemp(prefix="edgetx-ui-", dir=str(runs))
            ).resolve(strict=True)
            settings_copy = run_directory / "settings"
            storage_copy = run_directory / "sdcard"
            artifacts = run_directory / "artifacts"
            shutil.copytree(settings_source, settings_copy, symlinks=False)
            shutil.copytree(storage_source, storage_copy, symlinks=False)
            artifacts.mkdir()

            copied_args = tuple(
                str(settings_copy)
                if value == "{settings}"
                else str(storage_copy)
                if value == "{storage}"
                else value
                for value in simulator_args
            )
            replacement = SimulatorSession(
                self._executable,
                artifacts,
                simulator_args=copied_args,
                cwd=self._cwd,
                env=self._env,
                request_timeout=self._request_timeout,
                stop_timeout=self._stop_timeout,
                terminate_timeout=self._terminate_timeout,
                kill_timeout=self._kill_timeout,
                reader_join_timeout=self._reader_join_timeout,
                protocol_sink=(
                    run_directory / "protocol.jsonl"
                    if self._protocol_sink_path is not None
                    else None
                ),
                required_capabilities=self._required_capabilities,
                expected_target=self._expected_target,
                expected_lcd=self._expected_lcd,
            )
            replacement._fixture_run_directory = run_directory
            replacement.start(timeout=timeout)
            return replacement
        except BaseException:
            if replacement is not None:
                replacement.close()
            if run_directory is not None:
                shutil.rmtree(run_directory, ignore_errors=True)
            raise

    def wait_frame(
        self, minimum: int, *, timeout: Optional[float] = None
    ) -> FrameBarrier:
        self._require_command("wait-frame")
        minimum = _validated_integer(
            minimum, 0, UINT64_MAX, "minimum display sequence"
        )
        response = self.request("wait-frame", str(minimum), timeout=timeout)
        try:
            barrier = decode_frame(response)
        except ProtocolViolation as error:
            raise ProtocolFailure(str(error)) from error
        if barrier.display_sequence < minimum:
            raise ProtocolFailure(
                "wait-frame completed below its requested display sequence"
            )
        return barrier

    def wait_next_frame(
        self, *, timeout: Optional[float] = None
    ) -> FrameBarrier:
        total_timeout = (
            self._request_timeout
            if timeout is None
            else _validated_timeout(timeout, "wait-next-frame")
        )
        deadline = time.monotonic() + total_timeout

        def remaining() -> float:
            value = deadline - time.monotonic()
            if value <= 0:
                raise RequestTimeout(
                    self._message_with_context(
                        "timed out waiting for the next display frame", None
                    )
                )
            return value

        status = self.read_status(timeout=remaining())
        if status.display_sequence == UINT64_MAX:
            raise SessionError("display sequence is saturated")
        return self.wait_frame(
            status.display_sequence + 1, timeout=remaining()
        )

    def capture_ppm(
        self, relative_path: str, *, timeout: Optional[float] = None
    ) -> CaptureArtifact:
        """Capture a fresh RGB565 framebuffer as a validated native PPM."""

        description = self._require_command("capture", capability="capture")
        canonical, output_path = self._validate_artifact_path(
            relative_path, ".ppm"
        )
        total_timeout = (
            self._request_timeout
            if timeout is None
            else _validated_timeout(timeout, "capture")
        )
        deadline = time.monotonic() + total_timeout

        def remaining() -> float:
            value = deadline - time.monotonic()
            if value <= 0:
                raise RequestTimeout(
                    self._message_with_context(
                        "timed out capturing a fresh display frame", None
                    )
                )
            return value

        baseline = self.read_status(timeout=remaining())
        response = self.request("capture", canonical, timeout=remaining())
        try:
            artifact = decode_capture(response)
        except ProtocolViolation as error:
            raise ProtocolFailure(str(error)) from error
        if artifact.path != canonical:
            raise ProtocolFailure("capture result path differs from the request")
        if artifact.display_sequence <= baseline.display_sequence:
            raise ProtocolFailure("capture did not use a newer display frame")
        if (artifact.width, artifact.height, artifact.depth) != (
            description.lcd.width,
            description.lcd.height,
            description.lcd.depth,
        ):
            raise ProtocolFailure(
                "capture metadata differs from target discovery"
            )
        try:
            image = read_ppm(output_path)
        except (OSError, ValueError) as error:
            raise ProtocolFailure(
                "native capture artifact is invalid: " + str(error)
            ) from error
        if (image.width, image.height) != (artifact.width, artifact.height):
            raise ProtocolFailure(
                "PPM dimensions differ from capture metadata"
            )
        try:
            actual_bytes = output_path.stat().st_size
        except OSError as error:
            raise ProtocolFailure(
                "cannot inspect native capture artifact"
            ) from error
        if actual_bytes != artifact.byte_count:
            raise ProtocolFailure("PPM size differs from capture metadata")
        return artifact

    def capture_png(
        self, relative_path: str, *, timeout: Optional[float] = None
    ) -> CaptureBundle:
        """Capture PPM, convert and verify PNG, then write stable metadata."""

        png_relative, png_path = self._validate_artifact_path(
            relative_path, ".png"
        )
        png_pure = PurePosixPath(png_relative)
        ppm_relative = png_pure.with_suffix(".ppm").as_posix()
        manifest_relative = png_pure.with_suffix(".capture.json").as_posix()
        _, manifest_path = self._validate_artifact_path(
            manifest_relative, ".json"
        )

        capture = self.capture_ppm(ppm_relative, timeout=timeout)
        root = self._output_root.resolve(strict=True)
        ppm_path = root.joinpath(*PurePosixPath(capture.path).parts)
        try:
            ppm_digest = digest_file(ppm_path)
            image, png_digest = convert_ppm_to_png(ppm_path, png_path)
        except (OSError, ValueError) as error:
            raise ProtocolFailure(
                "cannot convert native capture to PNG: " + str(error)
            ) from error
        if (image.width, image.height) != (capture.width, capture.height):
            raise ProtocolFailure(
                "converted PNG dimensions differ from capture"
            )

        description = self._description
        if description is None:
            raise SessionError("simulator discovery is unavailable")
        manifest_payload = {
            "artifacts": {
                "png": {
                    "bytes": png_digest.byte_count,
                    "path": png_relative,
                    "sha256": png_digest.sha256,
                },
                "ppm": {
                    "bytes": ppm_digest.byte_count,
                    "path": capture.path,
                    "sha256": ppm_digest.sha256,
                },
            },
            "depth": capture.depth,
            "display_seq": capture.display_sequence,
            "epoch": capture.epoch,
            "height": capture.height,
            "schema_version": 1,
            "target": description.target,
            "width": capture.width,
        }
        try:
            manifest_digest = write_json_sidecar(
                manifest_path, manifest_payload
            )
        except (OSError, ValueError) as error:
            raise ProtocolFailure(
                "cannot write capture metadata: " + str(error)
            ) from error
        return CaptureBundle(
            capture=capture,
            ppm=ppm_digest,
            png=png_digest,
            manifest=manifest_digest,
        )

    def press(
        self,
        key: str,
        *,
        duration: float = 0.05,
        timeout: Optional[float] = None,
    ) -> Response:
        self._validate_key(key, "key-down")
        duration = _validated_duration(duration, "press duration")
        self.key_down(key, timeout=timeout)
        try:
            _sleep_for_duration(duration)
        except BaseException:
            self._best_effort_key_release(key, timeout)
            raise
        return self._key_release_with_fallback(key, timeout)

    def long_press(
        self,
        key: str,
        *,
        duration: float = 1.0,
        timeout: Optional[float] = None,
    ) -> Response:
        return self.press(key, duration=duration, timeout=timeout)

    def tap(
        self,
        x: int,
        y: int,
        *,
        duration: float = 0.05,
        timeout: Optional[float] = None,
    ) -> Response:
        x, y = self._validate_touch_point(x, y, "touch-down")
        duration = _validated_duration(duration, "tap duration")
        self.touch_down(x, y, timeout=timeout)
        try:
            _sleep_for_duration(duration)
        except BaseException:
            self._best_effort_touch_release(timeout)
            raise
        return self._touch_release_with_fallback(timeout)

    def drag(
        self,
        points: Sequence[Tuple[int, int]],
        *,
        duration: float = 0.2,
        timeout: Optional[float] = None,
    ) -> Response:
        if isinstance(points, (str, bytes)) or len(points) < 2:
            raise ValueError("drag requires at least two touch points")
        try:
            validated = tuple(
                self._validate_touch_point(x, y, "touch-move")
                for x, y in points
            )
        except (TypeError, ValueError) as error:
            raise ValueError("drag points must be valid x/y pairs") from error
        duration = _validated_duration(duration, "drag duration")
        self.touch_down(*validated[0], timeout=timeout)
        started = time.monotonic()
        segments = len(validated) - 1
        try:
            for index, point in enumerate(validated[1:], start=1):
                deadline = started + duration * index / segments
                remaining = deadline - time.monotonic()
                if remaining > 0:
                    time.sleep(remaining)
                self.touch_move(*point, timeout=timeout)
        except BaseException:
            self._best_effort_touch_release(timeout)
            raise
        return self._touch_release_with_fallback(timeout)

    def _startup_remaining(self, deadline: float) -> float:
        remaining = deadline - time.monotonic()
        if remaining <= 0:
            raise RequestTimeout(
                self._message_with_context(
                    "timed out waiting for simulator readiness", None
                )
            )
        return remaining

    def _validate_description(self, description: Description) -> None:
        commands = set(description.commands)
        missing_commands = sorted(REQUIRED_STARTUP_COMMANDS - commands)
        if missing_commands:
            raise StartupMismatch(
                "simulator discovery is missing required commands: "
                + ", ".join(missing_commands)
            )
        if self._expected_target is not None and (
            description.target != self._expected_target
        ):
            raise StartupMismatch(
                "simulator target mismatch: expected "
                + self._expected_target
                + ", received "
                + description.target
            )
        if self._expected_lcd is not None:
            actual_lcd = (
                description.lcd.width,
                description.lcd.height,
                description.lcd.depth,
            )
            if actual_lcd != self._expected_lcd:
                raise StartupMismatch(
                    "simulator LCD mismatch: expected "
                    + "x".join(str(value) for value in self._expected_lcd)
                    + ", received "
                    + "x".join(str(value) for value in actual_lcd)
                )
        missing_capabilities = [
            name
            for name in self._required_capabilities
            if not description.capabilities.supports(name)
        ]
        if missing_capabilities:
            raise StartupMismatch(
                "simulator is missing required capabilities: "
                + ", ".join(missing_capabilities)
            )

    def _require_command(
        self, command: str, *, capability: Optional[str] = None
    ) -> Description:
        with self._state_lock:
            description = self._description
        if description is None:
            raise SessionError("simulator discovery is not available")
        if command not in description.commands:
            raise SessionError("target does not advertise command: " + command)
        if capability is not None and not description.capabilities.supports(
            capability
        ):
            raise SessionError(
                "target does not advertise capability: " + capability
            )
        return description

    def _validate_key(self, key: str, command: str) -> None:
        description = self._require_command(command)
        if not isinstance(key, str) or key not in description.keys:
            raise ValueError("key is not supported by target: " + str(key))

    def _validate_touch_point(
        self, x: int, y: int, command: str
    ) -> Tuple[int, int]:
        description = self._require_command(command, capability="touch")
        return (
            _validated_integer(x, 0, description.lcd.width - 1, "touch x"),
            _validated_integer(y, 0, description.lcd.height - 1, "touch y"),
        )

    def _validate_artifact_path(
        self, value: str, extension: str
    ) -> Tuple[str, Path]:
        if not isinstance(value, str) or not value:
            raise ValueError("artifact path must be a non-empty string")
        if value != value.strip(" ") or any(
            character in value for character in ("\0", "\r", "\n", "\\")
        ):
            raise ValueError("artifact path is not a canonical relative path")
        if len(value.encode("utf-8")) > 1024:
            raise ValueError("artifact path exceeds 1024 UTF-8 bytes")
        relative = PurePosixPath(value)
        canonical = relative.as_posix()
        if (
            relative.is_absolute()
            or (
                len(value) >= 2
                and value[0].isascii()
                and value[0].isalpha()
                and value[1] == ":"
            )
            or canonical != value
            or not relative.name
            or any(part in (".", "..") for part in relative.parts)
            or relative.suffix != extension
        ):
            raise ValueError(
                "artifact path must be canonical, relative, and end in "
                + extension
            )
        if os.name == "nt" and _is_unsafe_win32_filename(relative.name):
            raise ValueError("artifact filename is reserved by Windows")

        try:
            root = self._output_root.resolve(strict=True)
            parent = root.joinpath(*relative.parts[:-1]).resolve(strict=True)
            parent.relative_to(root)
        except (OSError, ValueError) as error:
            raise ValueError(
                "artifact parent must exist below the output root"
            ) from error
        output_path = parent / relative.name
        if os.path.lexists(output_path):
            raise ValueError("artifact path already exists: " + canonical)
        return canonical, output_path

    def _key_release_with_fallback(
        self, key: str, timeout: Optional[float]
    ) -> Response:
        try:
            return self.key_up(key, timeout=timeout)
        except BaseException:
            self._best_effort_release_all(timeout)
            raise

    def _touch_release_with_fallback(
        self, timeout: Optional[float]
    ) -> Response:
        try:
            return self.touch_up(timeout=timeout)
        except BaseException:
            self._best_effort_release_all(timeout)
            raise

    def _best_effort_key_release(
        self, key: str, timeout: Optional[float]
    ) -> None:
        try:
            self._key_release_with_fallback(key, timeout)
        except BaseException:
            pass

    def _best_effort_touch_release(self, timeout: Optional[float]) -> None:
        try:
            self._touch_release_with_fallback(timeout)
        except BaseException:
            pass

    def _best_effort_release_all(self, timeout: Optional[float]) -> None:
        try:
            self.release_all(timeout=timeout)
        except BaseException:
            pass

    @staticmethod
    def _validate_status(status: Status, description: Description) -> None:
        if status.target != description.target:
            raise ProtocolFailure("status target differs from describe")
        if status.lcd != description.lcd:
            raise ProtocolFailure("status LCD differs from describe")
        if status.capabilities != description.capabilities:
            raise ProtocolFailure("status capabilities differ from describe")
        if status.output_root != "ready":
            raise StartupMismatch("simulator output root is not ready")

    def request(
        self,
        command: str,
        *arguments: str,
        timeout: Optional[float] = None,
    ) -> Response:
        request_timeout = (
            self._request_timeout
            if timeout is None
            else _validated_timeout(timeout, "command")
        )
        # One monotonic deadline covers serialization behind request_lock,
        # queue admission, the complete pipe write/flush, and correlation of
        # the terminal response. No stage receives a fresh timeout budget.
        deadline = time.monotonic() + request_timeout

        with self._request_lock:
            pending: queue.Queue[_ResponseItem] = queue.Queue(maxsize=1)
            with self._state_lock:
                process = self._require_requestable_process()
                request_id = self._next_request_id
                if request_id > (1 << 64) - 1:
                    raise SessionError("request id space is exhausted")
                self._next_request_id += 1
                self._pending_id = request_id
                self._pending_queue = pending

            try:
                record = encode_request(request_id, command, arguments)
            except (TypeError, ValueError):
                self._clear_pending(pending)
                raise
            try:
                self._record_protocol(
                    "request",
                    {
                        "version": PROTOCOL_VERSION,
                        "id": request_id,
                        "command": command,
                        "args": list(arguments),
                    },
                )
                self._submit_write(record, request_id, deadline)
            except SessionError as error:
                self._clear_pending(pending)
                self._record_failure(error)
                if self._writer_timed_out:
                    # This path runs while request_lock is owned. An abortive
                    # shutdown is safe because it never attempts a stop
                    # request, and it guarantees the blocked writer and child
                    # are gone before the timeout is exposed to the caller.
                    self._shutdown(send_stop=False, raise_errors=False)
                raise

            try:
                response = self._wait_for_response(pending, request_id, deadline)
            except RequestTimeout as error:
                # A late response cannot be safely correlated with a later
                # request on this serialized v1 session.
                self._record_failure(error)
                raise
            finally:
                self._clear_pending(pending)

            if not response.ok:
                raise CommandFailed(response, self.recent_stderr)
            return response

    def stop(self, *, timeout: Optional[float] = None) -> Optional[Response]:
        return self._shutdown(
            send_stop=True,
            raise_errors=True,
            stop_request_timeout=timeout,
        )

    def close(self) -> None:
        self._shutdown(send_stop=True, raise_errors=False)

    def __enter__(self) -> "SimulatorSession":
        self.start()
        return self

    def __exit__(self, exc_type: object, exc: object, traceback: object) -> bool:
        if exc_type is None:
            self.stop()
        else:
            self.close()
        return False

    def _require_requestable_process(self) -> subprocess.Popen[bytes]:
        process = self._process
        if process is None:
            raise SessionError("simulator session has not been started")
        if self._closed or self._closing:
            raise SessionError("simulator session is stopping")
        if self._failure is not None:
            raise self._with_context(self._failure, None)
        returncode = process.poll()
        if returncode is not None:
            raise self._process_error("simulator is not running", None)
        if self._stdout_closed.is_set():
            raise ProtocolFailure("protocol stdout is closed")
        if process.stdin is None or process.stdout is None or process.stderr is None:
            raise SessionError("simulator pipes are unavailable")
        if self._pending_queue is not None:
            raise SessionError("another request is already pending")
        return process

    def _submit_write(
        self, record: bytes, request_id: int, deadline: float
    ) -> None:
        completion: "queue.Queue[Optional[SessionError]]" = queue.Queue(maxsize=1)
        work = _WriteWork(record, completion)
        remaining = deadline - time.monotonic()
        if remaining <= 0:
            self._writer_timed_out = True
            raise RequestTimeout(
                self._message_with_context(
                    "timed out before request reached protocol stdin", request_id
                )
            )
        try:
            self._writer_queue.put(work, timeout=remaining)
        except queue.Full as error:
            self._writer_timed_out = True
            raise RequestTimeout(
                self._message_with_context(
                    "timed out enqueueing request to protocol stdin", request_id
                )
            ) from error

        while True:
            remaining = deadline - time.monotonic()
            if remaining <= 0:
                self._writer_timed_out = True
                raise RequestTimeout(
                    self._message_with_context(
                        "timed out writing request to protocol stdin", request_id
                    )
                )
            try:
                result = completion.get(timeout=min(remaining, 0.05))
            except queue.Empty:
                process = self._process
                if process is not None and process.poll() is not None:
                    raise self._process_error(
                        "simulator exited while writing", request_id
                    )
                continue
            if result is not None:
                raise self._with_context(result, request_id)
            return

    def _write_stdin(self) -> None:
        while not self._writer_stop.is_set():
            try:
                item = self._writer_queue.get(timeout=0.05)
            except queue.Empty:
                continue
            if not isinstance(item, _WriteWork):
                return

            error: Optional[SessionError] = None
            process = self._process
            try:
                if process is None or process.stdin is None:
                    raise OSError("protocol stdin is unavailable")
                view = memoryview(item.record)
                written = 0
                while written < len(view):
                    count = process.stdin.write(view[written:])
                    if count is None or count <= 0:
                        raise OSError("protocol stdin accepted no bytes")
                    written += count
                process.stdin.flush()
            except (BrokenPipeError, OSError, ValueError) as caught:
                error = ProcessExited("cannot write protocol stdin: " + str(caught))
            try:
                item.completion.put_nowait(error)
            except queue.Full:
                pass

    def _record_protocol(self, direction: str, message: Mapping[str, Any]) -> None:
        record = {"direction": direction, "message": dict(message)}
        encoded = (
            json.dumps(
                record,
                ensure_ascii=False,
                sort_keys=True,
                separators=(",", ":"),
            )
            + "\n"
        ).encode("utf-8")
        with self._state_lock:
            self._protocol_records.append(record)
            self._protocol_record_count += 1
            self._protocol_sha256.update(encoded)
            if self._protocol_sink is not None:
                try:
                    self._protocol_sink.write(encoded)
                except OSError as error:
                    raise ProtocolFailure(
                        "cannot write protocol evidence sink: " + str(error)
                    ) from error

    def _wait_for_response(
        self,
        pending: queue.Queue[_ResponseItem],
        request_id: int,
        deadline: float,
    ) -> Response:
        while True:
            remaining = deadline - time.monotonic()
            if remaining <= 0:
                raise RequestTimeout(
                    self._message_with_context(
                        "timed out waiting for request " + str(request_id),
                        request_id,
                    )
                )
            try:
                item = pending.get(timeout=min(remaining, 0.05))
            except queue.Empty:
                process = self._process
                if process is not None and process.poll() is not None:
                    raise self._process_error(
                        "simulator exited while waiting", request_id
                    )
                continue

            if isinstance(item, Response):
                with self._state_lock:
                    failure = self._failure
                if failure is not None:
                    raise self._with_context(failure, request_id)
                return item

            process = self._process
            if (
                isinstance(item, _ProtocolClosed)
                and self._stdout_closed.is_set()
                and process is not None
            ):
                returncode = process.poll()
                if returncode is None:
                    try:
                        returncode = process.wait(timeout=0.05)
                    except subprocess.TimeoutExpired:
                        pass
                if returncode is not None:
                    raise self._process_error(
                        "simulator exited while waiting", request_id
                    )
            raise self._with_context(item, request_id)

    def _read_stdout(self) -> None:
        process = self._process
        if process is None or process.stdout is None:
            self._record_failure(ProtocolFailure("protocol stdout is unavailable"))
            self._stdout_closed.set()
            return

        buffered = bytearray()
        try:
            while True:
                chunk = _read_chunk(process.stdout)
                if not chunk:
                    break
                buffered.extend(chunk)
                while True:
                    delimiter = buffered.find(b"\n")
                    if delimiter < 0:
                        break
                    record = bytes(buffered[:delimiter])
                    del buffered[: delimiter + 1]
                    self._route_message(parse_message(record))
                if len(buffered) > MAX_RECORD_BYTES:
                    raise ProtocolFailure(
                        "protocol stdout record exceeds 16 KiB without newline"
                    )
            if buffered:
                raise ProtocolFailure("protocol stdout closed with a partial record")
        except SessionError as error:
            self._record_failure(error)
        except BaseException as error:
            if not self._closing:
                self._record_failure(
                    ProtocolFailure("cannot read protocol stdout: " + str(error))
                )
        finally:
            self._stdout_closed.set()
            with self._state_lock:
                if self._pending_queue is not None:
                    self._record_failure(
                        _ProtocolClosed(
                            "protocol stdout closed while a request was pending"
                        )
                    )

    def _route_message(self, message: Union[Response, Event]) -> None:
        self._record_protocol(
            "event" if isinstance(message, Event) else "response", message.raw
        )
        if isinstance(message, Event):
            with self._state_lock:
                self._events.append(message)
            return

        with self._state_lock:
            pending = self._pending_queue
            expected_id = self._pending_id
            if pending is None or expected_id is None:
                self._record_failure(
                    ProtocolFailure(
                        "received unknown or repeated response id " + str(message.id)
                    )
                )
                return
            if message.id != expected_id:
                self._record_failure(
                    ProtocolFailure(
                        "response id "
                        + str(message.id)
                        + " does not match pending request "
                        + str(expected_id)
                    )
                )
                return
            self._pending_id = None
            self._pending_queue = None
            pending.put_nowait(message)

    def _read_stderr(self) -> None:
        process = self._process
        if process is None or process.stderr is None:
            self._stderr_closed.set()
            return

        buffered = bytearray()
        try:
            while True:
                chunk = _read_chunk(process.stderr)
                if not chunk:
                    break
                buffered.extend(chunk)
                while True:
                    delimiter = buffered.find(b"\n")
                    if delimiter < 0:
                        break
                    line = bytes(buffered[:delimiter])
                    del buffered[: delimiter + 1]
                    if line.endswith(b"\r"):
                        line = line[:-1]
                    self._diagnostics.append(line)
                if len(buffered) > MAX_STDERR_BYTES:
                    self._diagnostics.append(bytes(buffered))
                    buffered.clear()
            if buffered:
                self._diagnostics.append(bytes(buffered))
        except (OSError, ValueError) as error:
            if not self._closing:
                self._diagnostics.append(
                    ("stderr reader failed: " + str(error)).encode(
                        "utf-8", errors="replace"
                    )
                )
        finally:
            self._stderr_closed.set()

    def _record_failure(self, error: SessionError) -> None:
        with self._state_lock:
            if self._failure is None:
                self._failure = error
            pending = self._pending_queue
            self._pending_id = None
            self._pending_queue = None
            if pending is not None:
                try:
                    pending.put_nowait(self._failure)
                except queue.Full:
                    pass

    def _clear_pending(self, pending: queue.Queue[_ResponseItem]) -> None:
        with self._state_lock:
            if self._pending_queue is pending:
                self._pending_id = None
                self._pending_queue = None

    def _shutdown(
        self,
        *,
        send_stop: bool,
        raise_errors: bool,
        stop_request_timeout: Optional[float] = None,
    ) -> Optional[Response]:
        with self._shutdown_lock:
            if self._closed:
                return self._stop_response

            process = self._process
            if process is None:
                self._closed = True
                self._termination_stage = "not-started"
                return None

            primary_error: Optional[SessionError] = None
            with self._state_lock:
                existing_failure = self._failure
            if send_stop and existing_failure is not None:
                primary_error = existing_failure
            elif send_stop and process.poll() is not None:
                primary_error = self._process_error(
                    "simulator exited before stop", None
                )
            elif send_stop and self._stdout_closed.is_set():
                primary_error = ProtocolFailure(
                    "protocol stdout closed before stop"
                )
            elif send_stop:
                try:
                    self._stop_response = self.request(
                        "stop", timeout=stop_request_timeout
                    )
                except SessionError as error:
                    primary_error = error

            with self._state_lock:
                self._closing = True

            # A timed-out pipe write can hold the stdin object's I/O lock.
            # Terminate the child first so its read handle closes and releases
            # the sole writer worker before this thread closes the Python pipe.
            if self._writer_timed_out and process.poll() is None:
                try:
                    process.terminate()
                except OSError as error:
                    if process.poll() is None:
                        primary_error = primary_error or SessionError(
                            "cannot terminate simulator: " + str(error)
                        )
                try:
                    process.wait(timeout=self._terminate_timeout)
                    self._termination_stage = "terminated"
                except subprocess.TimeoutExpired:
                    try:
                        process.kill()
                    except OSError as error:
                        if process.poll() is None:
                            primary_error = primary_error or SessionError(
                                "cannot kill simulator: " + str(error)
                            )
                    try:
                        process.wait(timeout=self._kill_timeout)
                        self._termination_stage = "killed"
                    except subprocess.TimeoutExpired:
                        primary_error = primary_error or SessionError(
                            "simulator did not exit after kill"
                        )
            else:
                if process.stdin is not None:
                    try:
                        process.stdin.close()
                    except OSError:
                        pass

                try:
                    process.wait(timeout=self._stop_timeout)
                    self._termination_stage = "graceful"
                except subprocess.TimeoutExpired:
                    try:
                        process.terminate()
                    except OSError as error:
                        if process.poll() is None:
                            primary_error = primary_error or SessionError(
                                "cannot terminate simulator: " + str(error)
                            )
                    try:
                        process.wait(timeout=self._terminate_timeout)
                        self._termination_stage = "terminated"
                    except subprocess.TimeoutExpired:
                        try:
                            process.kill()
                        except OSError as error:
                            if process.poll() is None:
                                primary_error = primary_error or SessionError(
                                    "cannot kill simulator: " + str(error)
                                )
                        try:
                            process.wait(timeout=self._kill_timeout)
                            self._termination_stage = "killed"
                        except subprocess.TimeoutExpired:
                            primary_error = primary_error or SessionError(
                                "simulator did not exit after kill"
                            )

            self._writer_stop.set()
            try:
                self._writer_queue.put_nowait(None)
            except queue.Full:
                pass
            self._close_process_pipes(process)
            threads_alive = self._join_io_threads()
            if threads_alive:
                primary_error = primary_error or SessionError(
                    "simulator I/O threads did not stop"
                )
            sink_error = self._close_protocol_sink()
            if sink_error is not None:
                primary_error = primary_error or sink_error

            with self._state_lock:
                self._closed = True
                self._pending_id = None
                self._pending_queue = None

            if primary_error is not None and raise_errors:
                raise self._with_context(primary_error, None)
            return self._stop_response

    @staticmethod
    def _close_process_pipes(process: subprocess.Popen[bytes]) -> None:
        for stream in (process.stdin, process.stdout, process.stderr):
            if stream is not None:
                try:
                    stream.close()
                except OSError:
                    pass

    def _join_io_threads(self) -> bool:
        for thread in (
            self._writer_thread,
            self._stdout_thread,
            self._stderr_thread,
        ):
            if thread is not None and thread.ident is not None:
                thread.join(timeout=self._reader_join_timeout)
        return self.writer_thread_alive or self.reader_threads_alive

    def _close_protocol_sink(self) -> Optional[SessionError]:
        sink = self._protocol_sink
        self._protocol_sink = None
        if sink is None:
            return None
        try:
            sink.flush()
            os.fsync(sink.fileno())
            sink.close()
        except OSError as error:
            try:
                sink.close()
            except OSError:
                pass
            return SessionError("cannot finalize protocol evidence sink: " + str(error))
        return None

    def _process_error(
        self, prefix: str, request_id: Optional[int]
    ) -> ProcessExited:
        returncode = self.returncode
        if returncode is not None:
            self._stderr_closed.wait(timeout=0.1)
        suffix = "" if returncode is None else " with code " + str(returncode)
        return ProcessExited(
            self._message_with_context(prefix + suffix, request_id)
        )

    def _with_context(
        self, error: SessionError, request_id: Optional[int]
    ) -> SessionError:
        if isinstance(error, CommandFailed):
            return error
        return type(error)(self._message_with_context(str(error), request_id))

    def _message_with_context(
        self, message: str, request_id: Optional[int]
    ) -> str:
        if request_id is not None and "request " + str(request_id) not in message:
            message += " (request " + str(request_id) + ")"
        diagnostics = self.recent_stderr
        if diagnostics and "Recent simulator stderr:" not in message:
            message += "\nRecent simulator stderr:\n" + diagnostics
        return message


def _read_chunk(stream: BinaryIO) -> bytes:
    read1 = getattr(stream, "read1", None)
    if read1 is not None:
        return read1(READ_CHUNK_BYTES)
    return stream.read(READ_CHUNK_BYTES)


def _require_named_range(
    values: Sequence[NamedRange], name: str, label: str
) -> NamedRange:
    if not isinstance(name, str) or not name:
        raise ValueError(label + " name must be a non-empty string")
    for value in values:
        if value.name == name:
            return value
    raise ValueError(label + " is not supported by target: " + name)


def _replace_option(
    arguments: Sequence[str], option: str, replacement: str
) -> Tuple[str, ...]:
    result = list(arguments)
    positions = [index for index, value in enumerate(result) if value == option]
    if len(positions) > 1:
        raise ValueError("simulator arguments repeat " + option)
    if positions:
        index = positions[0]
        if index + 1 >= len(result) or result[index + 1].startswith("--"):
            raise ValueError("simulator argument has no value: " + option)
        result[index + 1] = replacement
    else:
        result.extend((option, replacement))
    return tuple(result)


def _validated_fixture_directory(path: Path) -> Path:
    try:
        resolved = path.resolve(strict=True)
    except OSError as error:
        raise ValueError("fixture directory does not exist: " + str(path)) from error
    if not resolved.is_dir():
        raise ValueError("fixture path is not a directory: " + str(path))
    for current, directories, files in os.walk(resolved, followlinks=False):
        for name in (*directories, *files):
            if (Path(current) / name).is_symlink():
                raise ValueError("fixture templates must not contain symlinks")
    return resolved


def _path_is_below(path: Path, parent: Path) -> bool:
    try:
        path.relative_to(parent)
    except ValueError:
        return False
    return True


def _is_unsafe_win32_filename(value: str) -> bool:
    if not value or value[-1] in (" ", "."):
        return True
    if any(
        ord(character) < 32 or character in '<>:"/\\|?*'
        for character in value
    ):
        return True
    base = value.split(".", 1)[0].upper()
    if base in ("CON", "PRN", "AUX", "NUL"):
        return True
    return (
        len(base) == 4
        and base[:3] in ("COM", "LPT")
        and base[3] in "123456789¹²³"
    )


def _validated_timeout(value: float, name: str) -> float:
    try:
        timeout = float(value)
    except (TypeError, ValueError) as error:
        raise ValueError(name + " timeout must be a number") from error
    if not math.isfinite(timeout) or timeout <= 0 or timeout > MAX_COMMAND_TIMEOUT:
        raise ValueError(name + " timeout must be in (0, 60] seconds")
    return timeout


def _validated_integer(
    value: int,
    minimum: int,
    maximum: int,
    name: str,
    *,
    exclude_zero: bool = False,
) -> int:
    if (
        isinstance(value, bool)
        or not isinstance(value, int)
        or value < minimum
        or value > maximum
        or (exclude_zero and value == 0)
    ):
        zero_note = " and cannot be zero" if exclude_zero else ""
        raise ValueError(
            name
            + " must be an integer in "
            + str(minimum)
            + ".."
            + str(maximum)
            + zero_note
        )
    return value


def _validated_duration(value: float, name: str) -> float:
    if (
        isinstance(value, bool)
        or not isinstance(value, (int, float))
        or not math.isfinite(value)
        or value < 0
        or value > MAX_COMMAND_TIMEOUT
    ):
        raise ValueError(
            name
            + " must be a finite number in 0.."
            + str(MAX_COMMAND_TIMEOUT)
            + " seconds"
        )
    return float(value)


def _sleep_for_duration(duration: float) -> None:
    deadline = time.monotonic() + duration
    remaining = deadline - time.monotonic()
    if remaining > 0:
        time.sleep(remaining)


def _validated_capabilities(values: Sequence[str]) -> Tuple[str, ...]:
    if isinstance(values, (str, bytes)):
        raise ValueError("required capabilities must be a sequence of names")
    capabilities = tuple(values)
    for name in capabilities:
        if not isinstance(name, str) or name not in CAPABILITY_NAMES:
            raise ValueError("unknown required capability: " + str(name))
    if len(set(capabilities)) != len(capabilities):
        raise ValueError("required capabilities cannot contain duplicates")
    return capabilities


def _validated_lcd(
    value: Optional[Tuple[int, int, int]],
) -> Optional[Tuple[int, int, int]]:
    if value is None:
        return None
    if not isinstance(value, tuple) or len(value) != 3:
        raise ValueError("expected LCD must be a width/height/depth tuple")
    width, height, depth = value
    if (
        not isinstance(width, int)
        or isinstance(width, bool)
        or not isinstance(height, int)
        or isinstance(height, bool)
        or not isinstance(depth, int)
        or isinstance(depth, bool)
        or width <= 0
        or height <= 0
        or width > 65535
        or height > 65535
        or depth not in (1, 4, 16)
    ):
        raise ValueError("expected LCD dimensions are invalid")
    return value
