from __future__ import annotations

import json
import subprocess
import sys
import tempfile
import time
import unittest
from pathlib import Path


HARNESS_ROOT = Path(__file__).resolve().parents[1]
FAKE_SIMULATOR = Path(__file__).with_name("fake_simulator.py")
LAUNCHER = HARNESS_ROOT / "edgetx-ui"
sys.path.insert(0, str(HARNESS_ROOT))

from edgetx_ui.session import (  # noqa: E402
    MAX_STDERR_BYTES,
    MAX_STDERR_LINES,
    CommandFailed,
    ProcessExited,
    ProtocolFailure,
    RequestTimeout,
    SimulatorSession,
)


class SimulatorSessionTests(unittest.TestCase):
    def setUp(self) -> None:
        self.temporary_directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary_directory.cleanup)
        self.output_root = Path(self.temporary_directory.name)

    def session(self, mode: str, **overrides: float) -> SimulatorSession:
        options = {
            "request_timeout": 1.0,
            "stop_timeout": 0.5,
            "terminate_timeout": 0.5,
            "kill_timeout": 0.5,
            "reader_join_timeout": 0.5,
        }
        options.update(overrides)
        return SimulatorSession(
            sys.executable,
            self.output_root,
            simulator_args=(str(FAKE_SIMULATOR), mode),
            **options,
        )

    def assert_reaped(self, session: SimulatorSession) -> None:
        self.assertIsNotNone(session.returncode)
        self.assertFalse(session.reader_threads_alive)

    def test_fragmented_start_ping_stop_is_correlated_and_reaped(self) -> None:
        session = self.session("fragmented")
        ping = session.start()
        stop = session.stop()

        self.assertEqual(ping.id, 1)
        self.assertEqual(stop.id, 2)
        self.assertEqual(session.returncode, 0)
        self.assertEqual(session.termination_stage, "graceful")
        self.assert_reaped(session)

    def test_event_does_not_steal_correlated_response(self) -> None:
        session = self.session("event")
        try:
            ping = session.start()
            self.assertEqual(ping.id, 1)
            self.assertEqual(session.events[-1].code, "queue_full")
        finally:
            session.close()
        self.assert_reaped(session)

    def test_stderr_is_separate_and_bounded(self) -> None:
        session = self.session("stderr-flood", request_timeout=3.0)
        session.start()
        session.stop()

        line_count, byte_count = session.stderr_counts
        self.assertLessEqual(line_count, MAX_STDERR_LINES)
        self.assertLessEqual(byte_count, MAX_STDERR_BYTES)
        self.assertIn("diagnostic-299", session.recent_stderr)
        self.assert_reaped(session)

    def test_malformed_stdout_fails_and_cleans_up(self) -> None:
        session = self.session("malformed")
        with self.assertRaisesRegex(ProtocolFailure, "valid JSON"):
            session.start()
        self.assert_reaped(session)

    def test_mismatched_response_id_fails_and_cleans_up(self) -> None:
        session = self.session("wrong-id")
        with self.assertRaisesRegex(ProtocolFailure, "does not match"):
            session.start()
        self.assert_reaped(session)

    def test_repeated_response_id_poisoning_is_detected(self) -> None:
        session = self.session("duplicate")
        try:
            try:
                session.start()
            except ProtocolFailure:
                pass
            else:
                with self.assertRaisesRegex(
                    ProtocolFailure, "repeated|does not match"
                ):
                    session.ping()
        finally:
            session.close()
        self.assert_reaped(session)

    def test_command_error_keeps_code_and_diagnostics(self) -> None:
        session = self.session("command-error")
        with self.assertRaises(CommandFailed) as raised:
            session.start()
        self.assertEqual(raised.exception.response.error_code, "unsupported_command")
        self.assertIn("fake simulator started", str(raised.exception))
        self.assert_reaped(session)

    def test_timeout_terminates_and_reaps_child(self) -> None:
        session = self.session(
            "hang",
            request_timeout=0.1,
            stop_timeout=0.1,
            terminate_timeout=0.5,
        )
        started = time.monotonic()
        with self.assertRaises(RequestTimeout):
            session.start()
        self.assertLess(time.monotonic() - started, 2.0)
        self.assertIn(session.termination_stage, ("terminated", "killed"))
        self.assert_reaped(session)

    def test_child_crash_reports_exit_and_recent_stderr(self) -> None:
        session = self.session("crash")
        with self.assertRaises(ProcessExited) as raised:
            session.start()
        self.assertIn("code 17", str(raised.exception))
        self.assertIn("fixture crash", str(raised.exception))
        self.assert_reaped(session)

    def test_partial_record_at_eof_is_rejected(self) -> None:
        session = self.session("partial-eof")
        with self.assertRaisesRegex(ProtocolFailure, "partial record"):
            session.start()
        self.assert_reaped(session)

    def test_local_validation_error_does_not_poison_next_request(self) -> None:
        session = self.session("normal")
        try:
            session.start()
            with self.assertRaises(ValueError):
                session.request("capture", "bad\npath")
            ping = session.ping()
            self.assertEqual(ping.id, 3)
        finally:
            session.close()
        self.assert_reaped(session)

    def test_exit_after_ping_is_not_reported_as_a_clean_stop(self) -> None:
        session = self.session("exit-after-ping")
        session.start()
        with self.assertRaises(ProcessExited):
            session.stop()
        self.assert_reaped(session)

    def test_stop_escalates_when_child_acknowledges_but_does_not_exit(self) -> None:
        session = self.session(
            "ignore-stop", stop_timeout=0.1, terminate_timeout=0.5
        )
        session.start()
        stop = session.stop()

        self.assertEqual(stop.id, 2)
        self.assertEqual(session.termination_stage, "terminated")
        self.assert_reaped(session)

    def test_one_hundred_lifecycle_cycles_leave_no_reader_or_child(self) -> None:
        for cycle in range(100):
            with self.subTest(cycle=cycle):
                session = self.session("normal")
                session.start()
                session.stop()
                self.assert_reaped(session)

    def test_cli_probe_uses_the_same_session_lifecycle(self) -> None:
        result = subprocess.run(
            [
                sys.executable,
                str(LAUNCHER),
                "probe",
                "--output",
                str(self.output_root),
                "--timeout",
                "1",
                sys.executable,
                "--",
                str(FAKE_SIMULATOR),
                "normal",
            ],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            timeout=5,
            check=False,
        )
        self.assertEqual(result.returncode, 0, result.stderr)
        payload = json.loads(result.stdout)
        self.assertTrue(payload["ok"])
        self.assertEqual(payload["ping"]["id"], 1)
        self.assertEqual(payload["stop"]["id"], 2)
        self.assertEqual(payload["returncode"], 0)

    def test_cli_returns_nonzero_for_a_protocol_command_failure(self) -> None:
        result = subprocess.run(
            [
                sys.executable,
                str(LAUNCHER),
                "probe",
                "--output",
                str(self.output_root),
                "--timeout",
                "1",
                sys.executable,
                "--",
                str(FAKE_SIMULATOR),
                "command-error",
            ],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            timeout=5,
            check=False,
        )
        self.assertEqual(result.returncode, 2)
        self.assertEqual(result.stdout, "")
        self.assertIn("unsupported_command", result.stderr)


if __name__ == "__main__":
    unittest.main()
