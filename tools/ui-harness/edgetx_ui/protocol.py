"""Encoding and strict validation for the EdgeTX simulator protocol."""

from __future__ import annotations

import json
import re
from dataclasses import dataclass
from typing import Any, Dict, Optional, Sequence, Union


PROTOCOL_VERSION = 1
MAX_RECORD_BYTES = 16 * 1024
UINT64_MAX = (1 << 64) - 1

_COMMAND_PATTERN = re.compile(r"^[a-z][a-z0-9-]*$")


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


Message = Union[Response, Event]


def encode_request(
    request_id: int, command: str, arguments: Sequence[str] = ()
) -> bytes:
    """Serialize one request record without relying on shell quoting."""

    if not _is_uint64(request_id) or request_id == 0:
        raise ValueError("request id must be in 1..UINT64_MAX")
    if not isinstance(command, str) or not _COMMAND_PATTERN.fullmatch(command):
        raise ValueError("command must be a canonical lowercase ASCII token")

    tokens = ["v1", str(request_id), command]
    for argument in arguments:
        if not isinstance(argument, str) or not argument:
            raise ValueError("request arguments must be non-empty strings")
        if any(character in argument for character in ("\0", "\r", "\n")):
            raise ValueError("request arguments cannot contain NUL, CR, or LF")
        if argument != argument.strip(" "):
            raise ValueError("request arguments cannot start or end with spaces")
        tokens.append(argument)

    encoded = " ".join(tokens).encode("utf-8")
    if len(encoded) > MAX_RECORD_BYTES:
        raise ValueError("request record exceeds 16 KiB")
    return encoded + b"\n"


def parse_message(record: bytes) -> Message:
    """Parse one complete JSON record, excluding its newline delimiter."""

    if not isinstance(record, bytes):
        raise TypeError("protocol record must be bytes")
    if not record:
        raise ProtocolViolation("protocol stdout emitted an empty record")
    if len(record) > MAX_RECORD_BYTES:
        raise ProtocolViolation("protocol response exceeds 16 KiB")
    if b"\r" in record or b"\n" in record:
        raise ProtocolViolation("protocol response contains an embedded newline")

    try:
        text = record.decode("utf-8", errors="strict")
    except UnicodeDecodeError as error:
        raise ProtocolViolation("protocol response is not valid UTF-8") from error

    try:
        payload = json.loads(text, parse_constant=_reject_json_constant)
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


def _parse_response(payload: Dict[str, Any], epoch: int) -> Response:
    request_id = payload.get("id")
    if not _is_uint64(request_id) or request_id == 0:
        raise ProtocolViolation("protocol response has an invalid request id")

    ok = payload.get("ok")
    if not isinstance(ok, bool):
        raise ProtocolViolation("protocol response has an invalid ok field")

    result = payload.get("result")
    if result is not None and not isinstance(result, dict):
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
    if payload.get("id", object()) is not None:
        raise ProtocolViolation("protocol event id must be null")
    event = payload.get("event")
    if not isinstance(event, dict):
        raise ProtocolViolation("protocol event has no event object")
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
