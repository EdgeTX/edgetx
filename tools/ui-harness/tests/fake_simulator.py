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


def response(request_id: int, *, ok: bool = True) -> Dict[str, Any]:
    payload: Dict[str, Any] = {
        "version": 1,
        "type": "response",
        "id": request_id,
        "ok": ok,
        "epoch": 0,
    }
    if not ok:
        payload["error"] = {
            "code": "unsupported_command",
            "message": "fixture rejection",
        }
    return payload


def main() -> int:
    mode = sys.argv[1] if len(sys.argv) > 1 else "normal"
    print("fake simulator started: " + mode, file=sys.stderr, flush=True)

    for raw_line in sys.stdin.buffer:
        fields = raw_line.decode("utf-8").rstrip("\n").split(" ", 3)
        request_id = int(fields[1])
        command = fields[2]

        if mode == "malformed":
            sys.stdout.buffer.write(b"not-json\n")
            sys.stdout.buffer.flush()
            continue
        if mode == "wrong-id":
            emit(response(request_id + 1))
            continue
        if mode == "duplicate":
            emit(response(request_id))
            emit(response(request_id))
            continue
        if mode == "command-error":
            emit(response(request_id, ok=False))
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

        emit(response(request_id), fragmented=mode == "fragmented")
        if mode == "exit-after-ping" and command == "ping":
            return 0
        if command == "stop":
            if mode == "ignore-stop":
                time.sleep(60)
            return 0
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
