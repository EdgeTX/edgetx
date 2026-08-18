"""Small subprocess fixture for portable session lifecycle tests."""

from __future__ import annotations

import json
import sys
import time
from typing import Any, Dict, List, Optional


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


def is_phase4(mode: str) -> bool:
    return mode.startswith("phase4")


def capabilities(mode: str) -> Dict[str, bool]:
    return {
        "rotary": is_phase4(mode),
        "touch": is_phase4(mode),
        "switches": False,
        "analog": False,
        "telemetry": False,
        "lua": False,
        "capture": False,
        "warm_restart": False,
    }


def description(mode: str) -> Dict[str, Any]:
    commands = ["ping", "status", "describe", "stop"]
    if is_phase4(mode):
        commands = [
            "ping",
            "status",
            "describe",
            "key-down",
            "key-up",
            "rotate",
            "touch-down",
            "touch-move",
            "touch-up",
            "wait-frame",
            "release-all",
            "stop",
        ]
    if mode == "missing-command":
        commands.remove("status")
    result: Dict[str, Any] = {
        "protocol_version": 1,
        "target": "test-target",
        "lcd": {"width": 480, "height": 272, "depth": 16},
        "commands": commands,
        "capabilities": capabilities(mode),
        "keys": ["EXIT", "ENTER"] if is_phase4(mode) else [],
        "switches": [],
        "analogs": [],
    }
    if mode == "bad-description":
        result["capabilities"]["capture"] = "false"
    return result


def status(
    mode: str, poll: int, state: Optional[Dict[str, Any]] = None
) -> Dict[str, Any]:
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
        "display_seq": (
            int(state["display_seq"])
            if state is not None and ready
            else (1 if ready else 0)
        ),
        "async_operation": "none",
        "request_queue_depth": 0,
        "firmware_mailbox_depth": 0,
        "line_overflow_count": 0,
        "queue_overflow_count": 0,
        "active_key_count": len(state["keys"]) if state is not None else 0,
        "touch_active": bool(state["touch"]) if state is not None else False,
        "analog_override_count": 0,
        "lua_state": "unavailable",
        "capabilities": capabilities(mode),
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
    state: Optional[Dict[str, Any]] = None,
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
        payload["result"] = status(mode, status_poll, state)
        if payload["result"]["phase"] != "ready":
            payload["epoch"] = 0
    return payload


def phase4_response(
    request_id: int,
    command: str,
    arguments: List[str],
    mode: str,
    status_poll: int,
    state: Dict[str, Any],
) -> Dict[str, Any]:
    payload = response(request_id, command, mode, status_poll, state)
    payload["epoch"] = 1

    def fail(code: str, message: str) -> Dict[str, Any]:
        payload["ok"] = False
        payload["error"] = {"code": code, "message": message}
        payload.pop("result", None)
        return payload

    if command == "key-down":
        key = arguments[0]
        if key not in ("EXIT", "ENTER"):
            return fail("unsupported_target", "unsupported key")
        if key in state["keys"]:
            return fail("key_already_down", "key already down")
        state["keys"].add(key)
    elif command == "key-up":
        key = arguments[0]
        if key not in state["keys"]:
            return fail("key_not_down", "key not down")
        if mode == "phase4-release-error":
            return fail("internal_error", "injected key release failure")
        state["keys"].remove(key)
    elif command == "touch-down":
        if state["touch"]:
            return fail("touch_already_down", "touch already down")
        state["touch"] = True
    elif command == "touch-move":
        if not state["touch"]:
            return fail("touch_not_down", "touch not down")
    elif command == "touch-up":
        if not state["touch"]:
            return fail("touch_not_down", "touch not down")
        if mode == "phase4-release-error":
            return fail("internal_error", "injected touch release failure")
        state["touch"] = False
    elif command == "release-all":
        state["keys"].clear()
        state["touch"] = False
    elif command == "wait-frame":
        minimum = int(arguments[0])
        state["display_seq"] = max(int(state["display_seq"]), minimum)
        completed = state["display_seq"]
        if mode == "phase4-bad-frame" and minimum > 0:
            completed = minimum - 1
        payload["result"] = {"display_seq": completed}
    elif command not in (
        "ping",
        "status",
        "describe",
        "rotate",
        "stop",
    ):
        return fail("unsupported_command", "unsupported fixture command")
    return payload


def main() -> int:
    mode = sys.argv[1] if len(sys.argv) > 1 else "normal"
    print("fake simulator started: " + mode, file=sys.stderr, flush=True)
    status_poll = 0
    state: Dict[str, Any] = {
        "keys": set(),
        "touch": False,
        "display_seq": 1,
    }

    for raw_line in sys.stdin.buffer:
        fields = raw_line.decode("utf-8").rstrip("\n").split(" ")
        request_id = int(fields[1])
        command = fields[2]
        arguments = fields[3:]

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
        if is_phase4(mode):
            if mode == "phase4-wait-hang" and command == "wait-frame":
                time.sleep(60)
                continue
            emit(
                phase4_response(
                    request_id,
                    command,
                    arguments,
                    mode,
                    status_poll,
                    state,
                )
            )
            if command == "stop":
                return 0
            continue
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
