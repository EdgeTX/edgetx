from __future__ import annotations

import json
import sys
import unittest
from pathlib import Path


HARNESS_ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(HARNESS_ROOT))

from edgetx_ui.protocol import (  # noqa: E402
    MAX_RECORD_BYTES,
    Event,
    ProtocolViolation,
    Response,
    decode_description,
    decode_status,
    encode_request,
    parse_message,
)


def encoded(payload: object) -> bytes:
    return json.dumps(payload, separators=(",", ":")).encode("utf-8")


def capability_payload() -> dict[str, bool]:
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


def description_payload() -> dict[str, object]:
    return {
        "protocol_version": 1,
        "target": "tx16s",
        "lcd": {"width": 480, "height": 272, "depth": 16},
        "commands": ["ping", "status", "describe", "stop"],
        "capabilities": capability_payload(),
        "keys": [],
        "switches": [],
        "analogs": [],
    }


def status_payload() -> dict[str, object]:
    return {
        "protocol_version": 1,
        "running": True,
        "phase": "ready",
        "target": "tx16s",
        "lcd": {"width": 480, "height": 272, "depth": 16},
        "display_seq": 8,
        "async_operation": "none",
        "request_queue_depth": 0,
        "firmware_mailbox_depth": 0,
        "line_overflow_count": 0,
        "queue_overflow_count": 0,
        "active_key_count": 0,
        "touch_active": False,
        "analog_override_count": 0,
        "lua_state": "unavailable",
        "capabilities": capability_payload(),
        "output_root": "ready",
    }


class EncodeRequestTests(unittest.TestCase):
    def test_encodes_exact_ping_record(self) -> None:
        self.assertEqual(encode_request(1, "ping"), b"v1 1 ping\n")

    def test_preserves_internal_spaces_in_final_argument(self) -> None:
        self.assertEqual(
            encode_request(9, "capture", ("checkpoints/home screen.ppm",)),
            b"v1 9 capture checkpoints/home screen.ppm\n",
        )

    def test_rejects_ambiguous_or_oversized_fields(self) -> None:
        invalid_calls = (
            lambda: encode_request(0, "ping"),
            lambda: encode_request(True, "ping"),
            lambda: encode_request(1, "Ping"),
            lambda: encode_request(1, "ping", ("",)),
            lambda: encode_request(1, "capture", ("bad\npath",)),
            lambda: encode_request(1, "capture", (" leading",)),
            lambda: encode_request(1, "capture", ("x" * MAX_RECORD_BYTES,)),
        )
        for invalid_call in invalid_calls:
            with self.subTest(call=invalid_call):
                with self.assertRaises(ValueError):
                    invalid_call()


class ParseMessageTests(unittest.TestCase):
    def test_parses_success_and_failure_responses(self) -> None:
        success = parse_message(
            encoded(
                {
                    "version": 1,
                    "type": "response",
                    "id": 3,
                    "ok": True,
                    "epoch": 2,
                    "result": {"display_seq": 4},
                }
            )
        )
        self.assertIsInstance(success, Response)
        self.assertEqual(success.id, 3)
        self.assertEqual(success.result, {"display_seq": 4})

        failure = parse_message(
            encoded(
                {
                    "version": 1,
                    "type": "response",
                    "id": 4,
                    "ok": False,
                    "epoch": 2,
                    "error": {"code": "out_of_range", "message": "bad value"},
                }
            )
        )
        self.assertIsInstance(failure, Response)
        self.assertFalse(failure.ok)
        self.assertEqual(failure.error_code, "out_of_range")

    def test_parses_uncorrelated_event(self) -> None:
        event = parse_message(
            encoded(
                {
                    "version": 1,
                    "type": "event",
                    "id": None,
                    "epoch": 0,
                    "event": {"code": "queue_full", "message": "bounded"},
                }
            )
        )
        self.assertIsInstance(event, Event)
        self.assertEqual(event.code, "queue_full")

    def test_rejects_malformed_or_ambiguous_messages(self) -> None:
        invalid_records = (
            b"\xff",
            b"not-json",
            b"[]",
            encoded({"version": True, "type": "response", "epoch": 0}),
            encoded(
                {
                    "version": 1,
                    "type": "response",
                    "id": True,
                    "ok": True,
                    "epoch": 0,
                }
            ),
            encoded(
                {
                    "version": 1,
                    "type": "response",
                    "id": 1,
                    "ok": False,
                    "epoch": 0,
                }
            ),
            encoded(
                {
                    "version": 1,
                    "type": "event",
                    "id": 1,
                    "epoch": 0,
                    "event": {"code": "bad", "message": "bad"},
                }
            ),
            b'{"version":1,"type":"response","id":1,"ok":true,"epoch":NaN}',
        )
        for record in invalid_records:
            with self.subTest(record=record):
                with self.assertRaises(ProtocolViolation):
                    parse_message(record)


class DiscoveryResultTests(unittest.TestCase):
    @staticmethod
    def response(result: dict[str, object], *, epoch: int = 1) -> Response:
        message = parse_message(
            encoded(
                {
                    "version": 1,
                    "type": "response",
                    "id": 1,
                    "ok": True,
                    "epoch": epoch,
                    "result": result,
                }
            )
        )
        assert isinstance(message, Response)
        return message

    def test_decodes_description_and_ready_status(self) -> None:
        description = decode_description(self.response(description_payload()))
        status = decode_status(self.response(status_payload()))

        self.assertEqual(description.target, "tx16s")
        self.assertEqual(description.lcd.width, 480)
        self.assertEqual(description.commands[-1], "stop")
        self.assertFalse(description.capabilities.capture)
        self.assertEqual(status.phase, "ready")
        self.assertEqual(status.display_sequence, 8)
        self.assertEqual(status.epoch, 1)

    def test_rejects_unbounded_or_ambiguous_discovery_shapes(self) -> None:
        missing = description_payload()
        del missing["commands"]

        duplicate = description_payload()
        duplicate["commands"] = ["ping", "ping"]

        non_boolean = description_payload()
        non_boolean_capabilities = capability_payload()
        non_boolean_capabilities["capture"] = 1  # type: ignore[assignment]
        non_boolean["capabilities"] = non_boolean_capabilities

        bad_ready = status_payload()
        bad_ready["display_seq"] = True

        for result, decoder in (
            (missing, decode_description),
            (duplicate, decode_description),
            (non_boolean, decode_description),
            (bad_ready, decode_status),
        ):
            with self.subTest(result=result):
                with self.assertRaises(ProtocolViolation):
                    decoder(self.response(result))


if __name__ == "__main__":
    unittest.main()
