"""Small subprocess fixture for portable session lifecycle tests."""

from __future__ import annotations

import json
import sys
import time
from typing import Any, Dict


def emit(payload: Dict[str, Any], *, fragmented: bool = False) -> None:
    record = (
        json.dumps(payload, ensure_ascii=False, separators=(",", ":")) + "\n"
    ).encode("utf-8")
    if fragmented:
        boundaries = (1, 4, 9, len(record))
        start = 0
        for boundary in boundaries:
            sys.stdout.buffer.write(record[start:boundary])
            sys.stdout.buffer.flush()
            start = boundary
        return
    sys.stdout.buffer.write(record)
    sys.stdout.buffer.flush()


def capabilities() -> Dict[str, bool]:
    return {
        "rotary": False,
        "touch": False,
        "switches": False,
        "analog": False,
        "telemetry": False,
        "lua": False,
        "capture": False,
        "warm_restart": False,
    }


def description(mode: str) -> Dict[str, Any]:
    commands = ["ping", "status", "describe", "stop"]
    if mode == "missing-command":
        commands.remove("status")
    result: Dict[str, Any] = {
        "protocol_version": 1,
        "target": "test-target",
        "lcd": {"width": 480, "height": 272, "depth": 16},
        "commands": commands,
        "capabilities": capabilities(),
        "keys": [],
        "switches": [],
        "analogs": [],
    }
    if mode == "bad-description":
        result["capabilities"]["capture"] = "false"
    return result


def status(mode: str, poll: int) -> Dict[str, Any]:
    ready = mode != "never-ready" and not (
        mode == "starting-then-ready" and poll < 3
    )
    target = "other-target" if mode == "status-mismatch" else "test-target"
    result: Dict[str, Any] = {
        "protocol_version": 1,
        "running": True,
        "phase": "ready" if ready else "starting",
        "target": target,
        "lcd": {"width": 480, "height": 272, "depth": 16},
        "display_seq": 1 if ready else 0,
        "async_operation": "none",
        "request_queue_depth": 0,
        "firmware_mailbox_depth": 0,
        "line_overflow_count": 0,
        "queue_overflow_count": 0,
        "active_key_count": 0,
        "touch_active": False,
        "analog_override_count": 0,
        "lua_state": "unavailable",
        "capabilities": capabilities(),
        "output_root": "invalid" if mode == "output-invalid" else "ready",
    }
    if mode == "bad-ready":
        result["phase"] = "ready"
        result["display_seq"] = 0
    return result


def response(
    request_id: int,
    command: str,
    mode: str,
    status_poll: int,
    *,
    ok: bool = True,
) -> Dict[str, Any]:
    payload: Dict[str, Any] = {
        "version": 1,
        "type": "response",
        "id": request_id,
        "ok": ok,
        "epoch": 1 if command in ("status", "stop") else 0,
    }
    if not ok:
        payload["error"] = {
            "code": "unsupported_command",
            "message": "fixture rejection",
        }
    elif command == "describe":
        payload["result"] = description(mode)
    elif command == "status":
        payload["result"] = status(mode, status_poll)
        if payload["result"]["phase"] != "ready":
            payload["epoch"] = 0
    return payload


def main() -> int:
    mode = sys.argv[1] if len(sys.argv) > 1 else "normal"
    print("fake simulator started: " + mode, file=sys.stderr, flush=True)
    status_poll = 0

    for raw_line in sys.stdin.buffer:
        fields = raw_line.decode("utf-8").rstrip("\n").split(" ", 3)
        request_id = int(fields[1])
        command = fields[2]

        if mode == "malformed":
            sys.stdout.buffer.write(b"not-json\n")
            sys.stdout.buffer.flush()
            continue
        if mode == "wrong-id":
            emit(response(request_id + 1, command, mode, status_poll))
            continue
        if mode == "duplicate":
            emit(response(request_id, command, mode, status_poll))
            emit(response(request_id, command, mode, status_poll))
            continue
        if mode == "command-error":
            emit(response(request_id, command, mode, status_poll, ok=False))
            continue
        if mode == "crash":
            print("fixture crash", file=sys.stderr, flush=True)
            return 17
        if mode == "partial-eof":
            sys.stdout.buffer.write(b'{"version":1')
            sys.stdout.buffer.flush()
            return 18
        if mode == "hang":
            time.sleep(60)
            continue
        if mode == "stderr-flood":
            for index in range(300):
                print(
                    "diagnostic-%03d-" % index + ("x" * 1024),
                    file=sys.stderr,
                )
            sys.stderr.flush()
        if mode == "event":
            emit(
                {
                    "version": 1,
                    "type": "event",
                    "id": None,
                    "epoch": 0,
                    "event": {"code": "queue_full", "message": "fixture event"},
                }
            )

        if command == "status":
            status_poll += 1
        emit(
            response(request_id, command, mode, status_poll),
            fragmented=mode == "fragmented",
        )
        if mode == "exit-after-ping" and command == "ping":
            return 0
        if command == "stop":
            if mode == "ignore-stop":
                time.sleep(60)
            return 0
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
