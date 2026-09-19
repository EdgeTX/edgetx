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
    decode_capture,
    decode_description,
    decode_frame,
    decode_lua_reload,
    decode_restart,
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
        "stale_completion_count": 0,
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

    def test_preserves_internal_spaces_in_capture_remainder(self) -> None:
        self.assertEqual(encode_request(8, "capture"), b"v1 8 capture\n")
        self.assertEqual(
            encode_request(9, "capture", ("checkpoints/home  screen.ppm",)),
            b"v1 9 capture checkpoints/home  screen.ppm\n",
        )

    def test_rejects_ambiguous_argument_boundaries(self) -> None:
        invalid_calls = (
            lambda: encode_request(1, "key-down", ("ENTER extra",)),
            lambda: encode_request(1, "set-switch", ("SA extra", "1")),
            lambda: encode_request(
                1, "set-telemetry", ("1", "0", "0", "1", "0", "0 RSSI")
            ),
            lambda: encode_request(
                1, "capture", ("checkpoints/home", "screen.ppm")
            ),
        )
        for invalid_call in invalid_calls:
            with self.subTest(call=invalid_call):
                with self.assertRaises(ValueError):
                    invalid_call()

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

    def test_wire_limit_includes_the_newline_delimiter(self) -> None:
        prefix = b"v1 1 capture "
        exact_argument = "x" * (MAX_RECORD_BYTES - len(prefix) - 1)
        encoded_request = encode_request(1, "capture", (exact_argument,))
        self.assertEqual(len(encoded_request), MAX_RECORD_BYTES)

        with self.assertRaisesRegex(ValueError, "16 KiB"):
            encode_request(1, "capture", (exact_argument + "x",))


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

    def test_rejects_null_result_when_result_member_is_present(self) -> None:
        with self.assertRaisesRegex(ProtocolViolation, "result must be an object"):
            parse_message(
                encoded(
                    {
                        "version": 1,
                        "type": "response",
                        "id": 3,
                        "ok": True,
                        "epoch": 2,
                        "result": None,
                    }
                )
            )

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

    def test_rejects_duplicate_keys_at_every_json_depth(self) -> None:
        records = (
            b'{"version":1,"version":1,"type":"response","id":1,"ok":true,"epoch":0,"result":{}}',
            b'{"version":1,"type":"response","id":1,"ok":true,"epoch":0,"result":{"value":1,"value":2}}',
            b'{"version":1,"type":"response","id":1,"ok":false,"epoch":0,"error":{"code":"bad","code":"worse","message":"x"}}',
            b'{"version":1,"type":"event","id":null,"epoch":0,"event":{"code":"bad","message":"x","message":"y"}}',
        )
        for record in records:
            with self.subTest(record=record), self.assertRaises(ProtocolViolation):
                parse_message(record)
        with self.assertRaisesRegex(ProtocolViolation, "duplicate JSON key: version"):
            parse_message(records[0])

    def test_requires_exact_success_failure_and_event_schemas(self) -> None:
        valid_success = {
            "version": 1,
            "type": "response",
            "id": 1,
            "ok": True,
            "epoch": 0,
            "result": {},
        }
        valid_failure = {
            "version": 1,
            "type": "response",
            "id": 1,
            "ok": False,
            "epoch": 0,
            "error": {"code": "bad", "message": "x"},
        }
        valid_event = {
            "version": 1,
            "type": "event",
            "id": None,
            "epoch": 0,
            "event": {"code": "queue_full", "message": "x"},
        }
        invalid = (
            valid_success | {"extra": 1},
            valid_success | {"error": {"code": "bad", "message": "x"}},
            valid_failure | {"result": {}},
            valid_failure | {"error": {"code": "bad", "message": "x", "extra": 1}},
            valid_event | {"ok": False},
            valid_event | {"event": {"code": "queue_full", "message": "x", "extra": 1}},
        )
        for payload in invalid:
            with self.subTest(payload=payload), self.assertRaises(ProtocolViolation):
                parse_message(encoded(payload))

    def test_response_wire_limit_reserves_the_newline_delimiter(self) -> None:
        prefix = b'{"version":1,"type":"event","id":null,"epoch":0,"event":{"code":"x","message":"'
        suffix = b'"}}'
        exact = prefix + b"x" * (MAX_RECORD_BYTES - 1 - len(prefix) - len(suffix)) + suffix
        self.assertEqual(len(exact) + 1, MAX_RECORD_BYTES)
        self.assertIsInstance(parse_message(exact), Event)

        with self.assertRaisesRegex(ProtocolViolation, "16 KiB"):
            parse_message(exact[:-len(suffix)] + b"x" + suffix)


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


class FrameResultTests(unittest.TestCase):
    def response(self, result: object) -> Response:
        message = parse_message(
            encoded(
                {
                    "version": 1,
                    "type": "response",
                    "id": 7,
                    "ok": True,
                    "epoch": 3,
                    "result": result,
                }
            )
        )
        assert isinstance(message, Response)
        return message

    def test_decodes_exact_wait_frame_result(self) -> None:
        barrier = decode_frame(self.response({"display_seq": 42}))
        self.assertEqual(barrier.epoch, 3)
        self.assertEqual(barrier.display_sequence, 42)

    def test_rejects_ambiguous_wait_frame_result(self) -> None:
        for result in (
            {},
            {"display_seq": True},
            {"display_seq": -1},
            {"display_seq": 4, "extra": 1},
        ):
            with self.subTest(result=result):
                with self.assertRaises(ProtocolViolation):
                    decode_frame(self.response(result))

    def test_decodes_restart_and_generation_observed_lua_results(self) -> None:
        restart = decode_restart(self.response({"display_seq": 43}))
        lua = decode_lua_reload(
            self.response({"generation": 7, "state": "running"})
        )

        self.assertEqual((restart.epoch, restart.display_sequence), (3, 43))
        self.assertEqual((lua.epoch, lua.generation, lua.state), (3, 7, "running"))

    def test_rejects_ambiguous_restart_and_lua_results(self) -> None:
        for decoder, result in (
            (decode_restart, {"display_seq": 4, "extra": 1}),
            (decode_lua_reload, {"generation": 0, "state": "running"}),
            (decode_lua_reload, {"generation": 1, "state": "panic"}),
            (decode_lua_reload, {"generation": 1}),
        ):
            with self.subTest(decoder=decoder, result=result):
                with self.assertRaises(ProtocolViolation):
                    decoder(self.response(result))


class CaptureResultTests(unittest.TestCase):
    @staticmethod
    def response(result: object) -> Response:
        message = parse_message(
            encoded(
                {
                    "version": 1,
                    "type": "response",
                    "id": 9,
                    "ok": True,
                    "epoch": 4,
                    "result": result,
                }
            )
        )
        assert isinstance(message, Response)
        return message

    @staticmethod
    def valid_result() -> dict[str, object]:
        return {
            "display_seq": 42,
            "path": "checkpoints/home screen.ppm",
            "width": 480,
            "height": 272,
            "depth": 16,
            "bytes": 391695,
        }

    def test_decodes_exact_capture_metadata(self) -> None:
        artifact = decode_capture(self.response(self.valid_result()))

        self.assertEqual(artifact.epoch, 4)
        self.assertEqual(artifact.display_sequence, 42)
        self.assertEqual(artifact.path, "checkpoints/home screen.ppm")
        self.assertEqual((artifact.width, artifact.height), (480, 272))
        self.assertEqual(artifact.depth, 16)
        self.assertEqual(artifact.byte_count, 391695)

    def test_rejects_noncanonical_or_inconsistent_capture_metadata(self) -> None:
        mutations = (
            ("missing", None),
            ("extra", 1),
            ("display_seq", 0),
            ("display_seq", True),
            ("path", "../escape.ppm"),
            ("path", "/absolute.ppm"),
            ("path", "C:/rooted.ppm"),
            ("path", "wrong.PNG"),
            ("depth", 1),
            ("width", 0),
            ("bytes", 391694),
        )
        for field, value in mutations:
            result = self.valid_result()
            if field == "missing":
                del result["path"]
            else:
                result[field] = value
            with self.subTest(field=field, value=value):
                with self.assertRaises(ProtocolViolation):
                    decode_capture(self.response(result))


if __name__ == "__main__":
    unittest.main()
