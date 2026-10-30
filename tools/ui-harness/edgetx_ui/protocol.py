"""Encoding and strict validation for the EdgeTX simulator protocol."""

from __future__ import annotations

import json
import re
from dataclasses import dataclass
from typing import Any, Dict, Mapping, Optional, Sequence, Tuple, Union


PROTOCOL_VERSION = 1
MAX_RECORD_BYTES = 16 * 1024
UINT64_MAX = (1 << 64) - 1

_COMMAND_PATTERN = re.compile(r"^[a-z][a-z0-9-]*$")
_TARGET_PATTERN = re.compile(r"^[A-Za-z0-9_.+-]+$")
_REMAINDER_ARGUMENT_COMMANDS = frozenset(("capture",))

CAPABILITY_NAMES = (
    "rotary",
    "touch",
    "switches",
    "analog",
    "telemetry",
    "lua",
    "capture",
    "warm_restart",
)

_STATUS_PHASES = frozenset(("starting", "ready", "restarting", "stopped"))
_ASYNC_OPERATIONS = frozenset(
    ("none", "wait_frame", "capture", "firmware", "reload_lua", "restart")
)
_LUA_STATES = frozenset(
    ("unavailable", "not_observed", "idle", "reloading", "running", "panic")
)


class ProtocolViolation(ValueError):
    """Raised when bytes on the protocol channel violate protocol v1."""


@dataclass(frozen=True)
class Response:
    id: int
    ok: bool
    epoch: int
    result: Optional[Dict[str, Any]]
    error_code: Optional[str]
    error_message: Optional[str]
    raw: Dict[str, Any]


@dataclass(frozen=True)
class Event:
    epoch: int
    code: str
    message: str
    raw: Dict[str, Any]


@dataclass(frozen=True)
class LcdDescription:
    width: int
    height: int
    depth: int


@dataclass(frozen=True)
class NamedRange:
    name: str
    minimum: int
    maximum: int


@dataclass(frozen=True)
class Capabilities:
    rotary: bool
    touch: bool
    switches: bool
    analog: bool
    telemetry: bool
    lua: bool
    capture: bool
    warm_restart: bool

    def supports(self, name: str) -> bool:
        if name not in CAPABILITY_NAMES:
            raise ValueError("unknown capability: " + name)
        return bool(getattr(self, name))


@dataclass(frozen=True)
class Description:
    target: str
    lcd: LcdDescription
    commands: Tuple[str, ...]
    capabilities: Capabilities
    keys: Tuple[str, ...]
    switches: Tuple[NamedRange, ...]
    analogs: Tuple[NamedRange, ...]


@dataclass(frozen=True)
class Status:
    epoch: int
    running: bool
    phase: str
    target: str
    lcd: LcdDescription
    display_sequence: int
    async_operation: str
    request_queue_depth: int
    firmware_mailbox_depth: int
    line_overflow_count: int
    queue_overflow_count: int
    stale_completion_count: int
    active_key_count: int
    touch_active: bool
    analog_override_count: int
    lua_state: str
    capabilities: Capabilities
    output_root: str


@dataclass(frozen=True)
class FrameBarrier:
    epoch: int
    display_sequence: int


@dataclass(frozen=True)
class LuaReload:
    epoch: int
    generation: int
    state: str


@dataclass(frozen=True)
class CaptureArtifact:
    epoch: int
    display_sequence: int
    path: str
    width: int
    height: int
    depth: int
    byte_count: int


Message = Union[Response, Event]


def encode_request(
    request_id: int, command: str, arguments: Sequence[str] = ()
) -> bytes:
    """Serialize one request record without relying on shell quoting."""

    if not _is_uint64(request_id) or request_id == 0:
        raise ValueError("request id must be in 1..UINT64_MAX")
    if not isinstance(command, str) or not _COMMAND_PATTERN.fullmatch(command):
        raise ValueError("command must be a canonical lowercase ASCII token")
    if command in _REMAINDER_ARGUMENT_COMMANDS and len(arguments) > 1:
        raise ValueError("capture accepts at most one remainder argument")

    tokens = ["v1", str(request_id), command]
    for argument in arguments:
        if not isinstance(argument, str) or not argument:
            raise ValueError("request arguments must be non-empty strings")
        if any(character in argument for character in ("\0", "\r", "\n")):
            raise ValueError("request arguments cannot contain NUL, CR, or LF")
        if argument != argument.strip(" "):
            raise ValueError("request arguments cannot start or end with spaces")
        if " " in argument and command not in _REMAINDER_ARGUMENT_COMMANDS:
            raise ValueError(
                "only remainder arguments may contain internal spaces"
            )
        tokens.append(argument)

    encoded = " ".join(tokens).encode("utf-8")
    if len(encoded) + 1 > MAX_RECORD_BYTES:
        raise ValueError("request record exceeds 16 KiB")
    return encoded + b"\n"


def parse_message(record: bytes) -> Message:
    """Parse one complete JSON record, excluding its newline delimiter."""

    if not isinstance(record, bytes):
        raise TypeError("protocol record must be bytes")
    if not record:
        raise ProtocolViolation("protocol stdout emitted an empty record")
    if len(record) + 1 > MAX_RECORD_BYTES:
        raise ProtocolViolation("protocol response exceeds 16 KiB")
    if b"\r" in record or b"\n" in record:
        raise ProtocolViolation("protocol response contains an embedded newline")

    try:
        text = record.decode("utf-8", errors="strict")
    except UnicodeDecodeError as error:
        raise ProtocolViolation("protocol response is not valid UTF-8") from error

    try:
        payload = json.loads(
            text,
            parse_constant=_reject_json_constant,
            object_pairs_hook=_unique_json_object,
        )
    except ProtocolViolation:
        raise
    except (ValueError, json.JSONDecodeError) as error:
        raise ProtocolViolation("protocol response is not valid JSON") from error

    if not isinstance(payload, dict):
        raise ProtocolViolation("protocol response must be a JSON object")
    if payload.get("version") != PROTOCOL_VERSION or not _is_integer(
        payload.get("version")
    ):
        raise ProtocolViolation("protocol response has an unsupported version")

    epoch = payload.get("epoch")
    if not _is_uint64(epoch):
        raise ProtocolViolation("protocol response has an invalid epoch")

    message_type = payload.get("type")
    if message_type == "response":
        return _parse_response(payload, epoch)
    if message_type == "event":
        return _parse_event(payload, epoch)
    raise ProtocolViolation("protocol response has an invalid type")


def decode_description(response: Response) -> Description:
    """Validate and decode the bounded result of a successful describe call."""

    result = _successful_result(response, "describe")
    _require_exact_keys(
        result,
        {
            "protocol_version",
            "target",
            "lcd",
            "commands",
            "capabilities",
            "keys",
            "switches",
            "analogs",
        },
        "describe result",
    )
    _require_protocol_version(result)
    return Description(
        target=_target(result.get("target"), "describe target"),
        lcd=_lcd(result.get("lcd"), "describe lcd"),
        commands=_string_tuple(
            result.get("commands"), "describe commands", _COMMAND_PATTERN
        ),
        capabilities=_capabilities(result.get("capabilities")),
        keys=_string_tuple(result.get("keys"), "describe keys", _TARGET_PATTERN),
        switches=_named_ranges(result.get("switches"), "describe switches"),
        analogs=_named_ranges(result.get("analogs"), "describe analogs"),
    )


def decode_status(response: Response) -> Status:
    """Validate and decode one internally consistent status snapshot."""

    result = _successful_result(response, "status")
    _require_exact_keys(
        result,
        {
            "protocol_version",
            "running",
            "phase",
            "target",
            "lcd",
            "display_seq",
            "async_operation",
            "request_queue_depth",
            "firmware_mailbox_depth",
            "line_overflow_count",
            "queue_overflow_count",
            "stale_completion_count",
            "active_key_count",
            "touch_active",
            "analog_override_count",
            "lua_state",
            "capabilities",
            "output_root",
        },
        "status result",
    )
    _require_protocol_version(result)

    running = result.get("running")
    touch_active = result.get("touch_active")
    if not isinstance(running, bool) or not isinstance(touch_active, bool):
        raise ProtocolViolation("status boolean fields are invalid")

    phase = result.get("phase")
    if not isinstance(phase, str) or phase not in _STATUS_PHASES:
        raise ProtocolViolation("status phase is invalid")
    async_operation = result.get("async_operation")
    if (
        not isinstance(async_operation, str)
        or async_operation not in _ASYNC_OPERATIONS
    ):
        raise ProtocolViolation("status async operation is invalid")
    lua_state = result.get("lua_state")
    if not isinstance(lua_state, str) or lua_state not in _LUA_STATES:
        raise ProtocolViolation("status Lua state is invalid")
    output_root = result.get("output_root")
    if not isinstance(output_root, str) or output_root not in (
        "ready",
        "invalid",
    ):
        raise ProtocolViolation("status output-root state is invalid")

    return Status(
        epoch=response.epoch,
        running=running,
        phase=phase,
        target=_target(result.get("target"), "status target"),
        lcd=_lcd(result.get("lcd"), "status lcd"),
        display_sequence=_uint64(result.get("display_seq"), "display sequence"),
        async_operation=async_operation,
        request_queue_depth=_uint64(
            result.get("request_queue_depth"), "request queue depth"
        ),
        firmware_mailbox_depth=_uint64(
            result.get("firmware_mailbox_depth"), "firmware mailbox depth"
        ),
        line_overflow_count=_uint64(
            result.get("line_overflow_count"), "line overflow count"
        ),
        queue_overflow_count=_uint64(
            result.get("queue_overflow_count"), "queue overflow count"
        ),
        stale_completion_count=_uint64(
            result.get("stale_completion_count"), "stale completion count"
        ),
        active_key_count=_uint64(
            result.get("active_key_count"), "active key count"
        ),
        touch_active=touch_active,
        analog_override_count=_uint64(
            result.get("analog_override_count"), "analog override count"
        ),
        lua_state=lua_state,
        capabilities=_capabilities(result.get("capabilities")),
        output_root=output_root,
    )


def decode_frame(response: Response) -> FrameBarrier:
    """Validate the terminal result of a wait-frame request."""

    result = _successful_result(response, "wait-frame")
    _require_exact_keys(result, {"display_seq"}, "wait-frame result")
    return FrameBarrier(
        epoch=response.epoch,
        display_sequence=_uint64(
            result.get("display_seq"), "wait-frame display sequence"
        ),
    )


def decode_restart(response: Response) -> FrameBarrier:
    """Validate the first-frame result of a successful warm restart."""

    result = _successful_result(response, "restart")
    _require_exact_keys(result, {"display_seq"}, "restart result")
    return FrameBarrier(
        epoch=response.epoch,
        display_sequence=_uint64(
            result.get("display_seq"), "restart display sequence"
        ),
    )


def decode_lua_reload(response: Response) -> LuaReload:
    """Validate a generation-correlated successful Lua reload."""

    result = _successful_result(response, "reload-lua")
    _require_exact_keys(result, {"generation", "state"}, "reload-lua result")
    state = result.get("state")
    if state != "running":
        raise ProtocolViolation("reload-lua terminal state is invalid")
    generation = _uint64(result.get("generation"), "Lua reload generation")
    if generation == 0:
        raise ProtocolViolation("Lua reload generation must be nonzero")
    return LuaReload(epoch=response.epoch, generation=generation, state=state)


def decode_capture(response: Response) -> CaptureArtifact:
    """Validate the terminal metadata of a native framebuffer capture."""

    result = _successful_result(response, "capture")
    _require_exact_keys(
        result,
        {"display_seq", "path", "width", "height", "depth", "bytes"},
        "capture result",
    )
    path = result.get("path")
    if not isinstance(path, str):
        raise ProtocolViolation("capture result path is invalid")
    try:
        path_bytes = path.encode("utf-8")
    except UnicodeEncodeError as error:
        raise ProtocolViolation("capture result path is invalid") from error
    path_parts = path.split("/")
    if (
        not path
        or path != path.strip(" ")
        or any(character in path for character in ("\0", "\r", "\n", "\\"))
        or len(path_bytes) > 1024
        or path.startswith("/")
        or (
            len(path) >= 2
            and path[0].isascii()
            and path[0].isalpha()
            and path[1] == ":"
        )
        or any(part in ("", ".", "..") for part in path_parts)
        or not path.endswith(".ppm")
    ):
        raise ProtocolViolation("capture result path is invalid")

    width = _uint64(result.get("width"), "capture width")
    height = _uint64(result.get("height"), "capture height")
    depth = _uint64(result.get("depth"), "capture depth")
    byte_count = _uint64(result.get("bytes"), "capture byte count")
    if width == 0 or width > 65535 or height == 0 or height > 65535:
        raise ProtocolViolation("capture dimensions are invalid")
    if depth != 16:
        raise ProtocolViolation("capture depth is not RGB565")
    expected_bytes = len(f"P6\n{width} {height}\n255\n".encode("ascii"))
    expected_bytes += width * height * 3
    if byte_count != expected_bytes:
        raise ProtocolViolation("capture byte count does not match dimensions")

    display_sequence = _uint64(
        result.get("display_seq"), "capture display sequence"
    )
    if display_sequence == 0:
        raise ProtocolViolation("capture display sequence is invalid")

    return CaptureArtifact(
        epoch=response.epoch,
        display_sequence=display_sequence,
        path=path,
        width=width,
        height=height,
        depth=depth,
        byte_count=byte_count,
    )


def _parse_response(payload: Dict[str, Any], epoch: int) -> Response:
    request_id = payload.get("id")
    if not _is_uint64(request_id) or request_id == 0:
        raise ProtocolViolation("protocol response has an invalid request id")

    ok = payload.get("ok")
    if not isinstance(ok, bool):
        raise ProtocolViolation("protocol response has an invalid ok field")

    base_keys = {"version", "type", "id", "ok", "epoch"}
    allowed = (base_keys, base_keys | {"result"}) if ok else (
        base_keys | {"error"},
    )
    if set(payload) not in allowed:
        expected = base_keys | ({"result"} if ok else {"error"})
        _require_exact_keys(payload, expected, "protocol response")

    result = payload.get("result")
    if "result" in payload and not isinstance(result, dict):
        raise ProtocolViolation("protocol response result must be an object")

    error_code: Optional[str] = None
    error_message: Optional[str] = None
    error = payload.get("error")
    if ok:
        if error is not None:
            raise ProtocolViolation("successful protocol response contains an error")
    else:
        if not isinstance(error, dict):
            raise ProtocolViolation("failed protocol response has no error object")
        _require_exact_keys(error, {"code", "message"}, "protocol error")
        error_code = error.get("code")
        error_message = error.get("message")
        if not isinstance(error_code, str) or not error_code:
            raise ProtocolViolation("protocol error has an invalid code")
        if not isinstance(error_message, str):
            raise ProtocolViolation("protocol error has an invalid message")

    return Response(
        id=request_id,
        ok=ok,
        epoch=epoch,
        result=result,
        error_code=error_code,
        error_message=error_message,
        raw=payload,
    )


def _parse_event(payload: Dict[str, Any], epoch: int) -> Event:
    _require_exact_keys(
        payload, {"version", "type", "id", "epoch", "event"}, "protocol event"
    )
    if payload.get("id", object()) is not None:
        raise ProtocolViolation("protocol event id must be null")
    event = payload.get("event")
    if not isinstance(event, dict):
        raise ProtocolViolation("protocol event has no event object")
    _require_exact_keys(event, {"code", "message"}, "protocol event payload")
    code = event.get("code")
    message = event.get("message")
    if not isinstance(code, str) or not code:
        raise ProtocolViolation("protocol event has an invalid code")
    if not isinstance(message, str):
        raise ProtocolViolation("protocol event has an invalid message")
    return Event(epoch=epoch, code=code, message=message, raw=payload)


def _is_integer(value: object) -> bool:
    return isinstance(value, int) and not isinstance(value, bool)


def _is_uint64(value: object) -> bool:
    return _is_integer(value) and 0 <= value <= UINT64_MAX


def _reject_json_constant(value: str) -> None:
    raise ValueError("non-standard JSON constant: " + value)


def _unique_json_object(pairs: Sequence[Tuple[str, Any]]) -> Dict[str, Any]:
    result: Dict[str, Any] = {}
    for key, value in pairs:
        if key in result:
            raise ProtocolViolation(
                "protocol response contains duplicate JSON key: " + key
            )
        result[key] = value
    return result


def _successful_result(response: Response, command: str) -> Dict[str, Any]:
    if not response.ok:
        raise ProtocolViolation(command + " response is not successful")
    if response.result is None:
        raise ProtocolViolation(command + " response has no result")
    return response.result


def _require_exact_keys(
    value: Mapping[str, Any], expected: set[str], label: str
) -> None:
    actual = set(value)
    if actual != expected:
        missing = sorted(expected - actual)
        extra = sorted(actual - expected)
        detail = ""
        if missing:
            detail += "; missing " + ", ".join(missing)
        if extra:
            detail += "; extra " + ", ".join(extra)
        raise ProtocolViolation(label + " has an invalid schema" + detail)


def _require_protocol_version(result: Mapping[str, Any]) -> None:
    version = result.get("protocol_version")
    if not _is_integer(version) or version != PROTOCOL_VERSION:
        raise ProtocolViolation("result protocol version is invalid")


def _target(value: object, label: str) -> str:
    if (
        not isinstance(value, str)
        or not value
        or len(value) > 64
        or _TARGET_PATTERN.fullmatch(value) is None
    ):
        raise ProtocolViolation(label + " is invalid")
    return value


def _lcd(value: object, label: str) -> LcdDescription:
    if not isinstance(value, dict):
        raise ProtocolViolation(label + " must be an object")
    _require_exact_keys(value, {"width", "height", "depth"}, label)
    width = value.get("width")
    height = value.get("height")
    depth = value.get("depth")
    if (
        not _is_integer(width)
        or not _is_integer(height)
        or width <= 0
        or height <= 0
        or width > 65535
        or height > 65535
        or not _is_integer(depth)
        or depth not in (1, 4, 16)
    ):
        raise ProtocolViolation(label + " dimensions are invalid")
    return LcdDescription(width=width, height=height, depth=depth)


def _capabilities(value: object) -> Capabilities:
    if not isinstance(value, dict):
        raise ProtocolViolation("capabilities must be an object")
    _require_exact_keys(value, set(CAPABILITY_NAMES), "capabilities")
    for name in CAPABILITY_NAMES:
        if not isinstance(value.get(name), bool):
            raise ProtocolViolation("capability " + name + " is not boolean")
    return Capabilities(**{name: value[name] for name in CAPABILITY_NAMES})


def _string_tuple(
    value: object, label: str, pattern: re.Pattern[str]
) -> Tuple[str, ...]:
    if not isinstance(value, list) or len(value) > 256:
        raise ProtocolViolation(label + " must be a bounded array")
    strings = []
    for item in value:
        if (
            not isinstance(item, str)
            or not item
            or len(item) > 64
            or pattern.fullmatch(item) is None
        ):
            raise ProtocolViolation(label + " contains an invalid name")
        strings.append(item)
    if len(set(strings)) != len(strings):
        raise ProtocolViolation(label + " contains duplicate names")
    return tuple(strings)


def _named_ranges(value: object, label: str) -> Tuple[NamedRange, ...]:
    if not isinstance(value, list) or len(value) > 256:
        raise ProtocolViolation(label + " must be a bounded array")
    ranges = []
    names = set()
    for item in value:
        if not isinstance(item, dict):
            raise ProtocolViolation(label + " entries must be objects")
        _require_exact_keys(item, {"name", "min", "max"}, label + " entry")
        name = _target(item.get("name"), label + " name")
        minimum = item.get("min")
        maximum = item.get("max")
        if (
            not _is_integer(minimum)
            or not _is_integer(maximum)
            or minimum < -(1 << 31)
            or maximum > (1 << 31) - 1
            or minimum > maximum
        ):
            raise ProtocolViolation(label + " contains an invalid range")
        if name in names:
            raise ProtocolViolation(label + " contains duplicate names")
        names.add(name)
        ranges.append(NamedRange(name=name, minimum=minimum, maximum=maximum))
    return tuple(ranges)


def _uint64(value: object, label: str) -> int:
    if not _is_uint64(value):
        raise ProtocolViolation(label + " is invalid")
    return value
