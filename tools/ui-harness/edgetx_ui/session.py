"""Cross-platform subprocess lifecycle for EdgeTX simulator automation."""

from __future__ import annotations

import math
import os
import queue
import subprocess
import threading
import time
from collections import deque
from pathlib import Path
from typing import BinaryIO, Deque, Mapping, Optional, Sequence, Tuple, Union

from .protocol import (
    CAPABILITY_NAMES,
    Description,
    Event,
    MAX_RECORD_BYTES,
    ProtocolViolation,
    Response,
    Status,
    decode_description,
    decode_status,
    encode_request,
    parse_message,
)


READ_CHUNK_BYTES = 4096
MAX_STDERR_LINES = 200
MAX_STDERR_BYTES = 256 * 1024
MAX_EVENTS = 64
MAX_COMMAND_TIMEOUT = 60.0
REQUIRED_STARTUP_COMMANDS = frozenset(("ping", "status", "describe", "stop"))


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
        required_capabilities: Sequence[str] = (),
        expected_target: Optional[str] = None,
        expected_lcd: Optional[Tuple[int, int, int]] = None,
    ) -> None:
        self._executable = os.fspath(executable)
        self._output_root = Path(output_root)
        self._simulator_args = tuple(os.fspath(value) for value in simulator_args)
        self._cwd = os.fspath(cwd) if cwd is not None else None
        self._env = dict(env) if env is not None else None
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
        self._stdout_closed = threading.Event()
        self._stderr_closed = threading.Event()
        self._diagnostics = _BoundedDiagnostics()
        self._events: Deque[Event] = deque(maxlen=MAX_EVENTS)

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

        try:
            process = subprocess.Popen(
                list(self.command),
                cwd=self._cwd,
                env=self._env,
                stdin=subprocess.PIPE,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=False,
                bufsize=-1,
                shell=False,
            )
        except OSError as error:
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
        self._stdout_thread.start()
        self._stderr_thread.start()

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
                self._status_response = self.request(
                    "status", timeout=self._startup_remaining(deadline)
                )
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
                assert process.stdin is not None
                process.stdin.write(record)
                process.stdin.flush()
            except (BrokenPipeError, OSError, ValueError) as error:
                self._clear_pending(pending)
                raise self._process_error(
                    "cannot write request " + str(request_id), request_id
                ) from error

            deadline = time.monotonic() + request_timeout
            try:
                response = self._wait_for_response(pending, request_id, deadline)
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

            self._close_reader_pipes(process)
            threads_alive = self._join_reader_threads()
            if threads_alive:
                primary_error = primary_error or SessionError(
                    "simulator reader threads did not stop"
                )

            with self._state_lock:
                self._closed = True
                self._pending_id = None
                self._pending_queue = None

            if primary_error is not None and raise_errors:
                raise self._with_context(primary_error, None)
            return self._stop_response

    @staticmethod
    def _close_reader_pipes(process: subprocess.Popen[bytes]) -> None:
        for stream in (process.stdout, process.stderr):
            if stream is not None:
                try:
                    stream.close()
                except OSError:
                    pass

    def _join_reader_threads(self) -> bool:
        for thread in (self._stdout_thread, self._stderr_thread):
            if thread is not None:
                thread.join(timeout=self._reader_join_timeout)
        return self.reader_threads_alive

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


def _validated_timeout(value: float, name: str) -> float:
    try:
        timeout = float(value)
    except (TypeError, ValueError) as error:
        raise ValueError(name + " timeout must be a number") from error
    if not math.isfinite(timeout) or timeout <= 0 or timeout > MAX_COMMAND_TIMEOUT:
        raise ValueError(name + " timeout must be in (0, 60] seconds")
    return timeout


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
