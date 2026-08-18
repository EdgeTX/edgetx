from __future__ import annotations

import hashlib
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

from edgetx_ui.ppm import read_png  # noqa: E402
from edgetx_ui.session import (  # noqa: E402
    MAX_STDERR_BYTES,
    MAX_STDERR_LINES,
    CommandFailed,
    ProcessExited,
    ProtocolFailure,
    RequestTimeout,
    SessionError,
    SimulatorSession,
    StartupMismatch,
)


class SimulatorSessionTests(unittest.TestCase):
    def setUp(self) -> None:
        self.temporary_directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary_directory.cleanup)
        self.output_root = Path(self.temporary_directory.name)

    def session(self, mode: str, **overrides: object) -> SimulatorSession:
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
        ready = session.start()
        stop = session.stop()

        self.assertEqual(session.startup_ping.id, 1)
        self.assertEqual(session.description_response.id, 2)
        self.assertEqual(ready.id, 3)
        self.assertEqual(ready.result["phase"], "ready")
        self.assertEqual(stop.id, 4)
        self.assertEqual(session.returncode, 0)
        self.assertEqual(session.termination_stage, "graceful")
        self.assert_reaped(session)

    def test_event_does_not_steal_correlated_response(self) -> None:
        session = self.session("event")
        try:
            ready = session.start()
            self.assertEqual(ready.id, 3)
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
            self.assertEqual(ping.id, 5)
        finally:
            session.close()
        self.assert_reaped(session)

    def test_exit_after_ping_is_not_reported_as_a_clean_stop(self) -> None:
        session = self.session("exit-after-ping")
        with self.assertRaises(ProcessExited):
            session.start()
        self.assert_reaped(session)

    def test_stop_escalates_when_child_acknowledges_but_does_not_exit(self) -> None:
        session = self.session(
            "ignore-stop", stop_timeout=0.1, terminate_timeout=0.5
        )
        session.start()
        stop = session.stop()

        self.assertEqual(stop.id, 4)
        self.assertEqual(session.termination_stage, "terminated")
        self.assert_reaped(session)

    def test_stop_kills_and_waits_when_terminate_does_not_reap(self) -> None:
        session = self.session(
            "ignore-stop",
            stop_timeout=0.1,
            terminate_timeout=0.1,
            kill_timeout=0.5,
        )
        session.start()
        process = session.process
        assert process is not None
        process.terminate = lambda: None  # type: ignore[method-assign]

        stop = session.stop()

        self.assertEqual(stop.id, 4)
        self.assertEqual(session.termination_stage, "killed")
        self.assert_reaped(session)

    def test_start_polls_status_until_a_real_first_frame(self) -> None:
        session = self.session("starting-then-ready")
        try:
            ready = session.start()
            self.assertEqual(ready.id, 5)
            self.assertEqual(session.status.phase, "ready")
            self.assertEqual(session.status.epoch, 1)
            self.assertEqual(session.status.display_sequence, 1)
        finally:
            session.close()
        self.assert_reaped(session)

    def test_startup_deadline_bounds_a_never_ready_simulator(self) -> None:
        session = self.session("never-ready")
        with self.assertRaisesRegex(RequestTimeout, "readiness"):
            session.start(timeout=1.0)
        self.assert_reaped(session)

    def test_discovery_schema_and_status_identity_are_strict(self) -> None:
        bad_description = self.session("bad-description")
        with self.assertRaisesRegex(ProtocolFailure, "capture.*boolean"):
            bad_description.start()
        self.assert_reaped(bad_description)

        mismatched_status = self.session("status-mismatch")
        with self.assertRaisesRegex(ProtocolFailure, "target differs"):
            mismatched_status.start()
        self.assert_reaped(mismatched_status)

    def test_target_lcd_and_required_capabilities_are_validated(self) -> None:
        matching = self.session(
            "normal",
            expected_target="test-target",
            expected_lcd=(480, 272, 16),
        )
        matching.start()
        matching.stop()
        self.assert_reaped(matching)

        missing = self.session("normal", required_capabilities=("capture",))
        with self.assertRaisesRegex(StartupMismatch, "capture"):
            missing.start()
        self.assert_reaped(missing)

    def test_phase4_discovery_and_primitives_are_validated(self) -> None:
        session = self.session("phase4")
        try:
            session.start()
            assert session.description is not None
            self.assertTrue(session.description.capabilities.rotary)
            self.assertTrue(session.description.capabilities.touch)
            self.assertEqual(session.description.keys, ("EXIT", "ENTER"))

            session.key_down("ENTER")
            self.assertEqual(session.read_status().active_key_count, 1)
            with self.assertRaises(CommandFailed) as duplicate:
                session.key_down("ENTER")
            self.assertEqual(
                duplicate.exception.response.error_code, "key_already_down"
            )
            session.key_up("ENTER")

            session.rotate(-128)
            session.rotate(128)
            session.touch_down(0, 0)
            session.touch_move(479, 271)
            session.touch_up()
            session.release_all()
            status = session.read_status()
            self.assertEqual(status.active_key_count, 0)
            self.assertFalse(status.touch_active)
        finally:
            session.close()
        self.assert_reaped(session)

    def test_phase4_host_composites_release_owned_inputs(self) -> None:
        session = self.session("phase4")
        try:
            session.start()
            session.press("ENTER", duration=0)
            session.long_press("EXIT", duration=0)
            session.tap(0, 0, duration=0)
            session.drag(((0, 0), (240, 136), (479, 271)), duration=0)
            status = session.read_status()
            self.assertEqual(status.active_key_count, 0)
            self.assertFalse(status.touch_active)
        finally:
            session.close()
        self.assert_reaped(session)

    def test_phase4_composite_release_failure_falls_back_to_release_all(
        self,
    ) -> None:
        session = self.session("phase4-release-error")
        try:
            session.start()
            with self.assertRaises(CommandFailed):
                session.press("ENTER", duration=0)
            self.assertEqual(session.read_status().active_key_count, 0)

            with self.assertRaises(CommandFailed):
                session.tap(1, 1, duration=0)
            self.assertFalse(session.read_status().touch_active)
        finally:
            session.close()
        self.assert_reaped(session)

    def test_phase4_local_validation_does_not_consume_request_ids(self) -> None:
        session = self.session("phase4")
        try:
            session.start()
            for invalid_call in (
                lambda: session.key_down("UNKNOWN"),
                lambda: session.rotate(0),
                lambda: session.rotate(129),
                lambda: session.touch_down(-1, 0),
                lambda: session.touch_move(480, 0),
                lambda: session.drag(((0, 0),), duration=0),
            ):
                with self.subTest(call=invalid_call):
                    with self.assertRaises(ValueError):
                        invalid_call()
            self.assertEqual(session.ping().id, 4)
        finally:
            session.close()
        self.assert_reaped(session)

    def test_phase4_wait_frame_uses_fresh_status_and_strict_result(self) -> None:
        session = self.session("phase4")
        try:
            session.start()
            current = session.wait_frame(1)
            self.assertEqual(current.display_sequence, 1)
            next_frame = session.wait_next_frame()
            self.assertEqual(next_frame.display_sequence, 2)
            self.assertEqual(next_frame.epoch, 1)
        finally:
            session.close()
        self.assert_reaped(session)

        malformed = self.session("phase4-bad-frame")
        try:
            malformed.start()
            with self.assertRaisesRegex(ProtocolFailure, "below"):
                malformed.wait_frame(2)
        finally:
            malformed.close()
        self.assert_reaped(malformed)

    def test_phase4_wait_timeout_poisons_and_reaps_session(self) -> None:
        session = self.session(
            "phase4-wait-hang",
            request_timeout=0.1,
            stop_timeout=0.1,
            terminate_timeout=0.5,
        )
        session.start(timeout=1.0)
        with self.assertRaises(RequestTimeout):
            session.wait_frame(2)
        with self.assertRaises(SessionError):
            session.ping()
        session.close()
        self.assertIn(session.termination_stage, ("terminated", "killed"))
        self.assert_reaped(session)

    def test_phase5_capture_ppm_is_fresh_and_preserves_safe_spaces(self) -> None:
        capture_dir = self.output_root / "check points"
        capture_dir.mkdir()
        session = self.session("phase5")
        try:
            session.start()
            artifact = session.capture_ppm("check points/home screen.ppm")

            self.assertEqual(artifact.path, "check points/home screen.ppm")
            self.assertEqual(artifact.display_sequence, 2)
            self.assertEqual((artifact.width, artifact.height), (480, 272))
            self.assertEqual(artifact.depth, 16)
            capture_path = self.output_root / "check points" / "home screen.ppm"
            self.assertEqual(capture_path.stat().st_size, artifact.byte_count)
            self.assertTrue(
                capture_path.read_bytes().startswith(b"P6\n480 272\n255\n")
            )
        finally:
            session.close()
        self.assert_reaped(session)

    def test_phase5_local_path_rejection_does_not_consume_request_ids(self) -> None:
        existing = self.output_root / "existing.ppm"
        existing.write_bytes(b"keep")
        session = self.session("phase5")
        try:
            session.start()
            invalid_paths = [
                "../escape.ppm",
                "/absolute.ppm",
                "C:/rooted.ppm",
                "missing/parent.ppm",
                "existing.ppm",
                "wrong.PNG",
                "double//separator.ppm",
                "back\\slash.ppm",
                "a" * 1021 + ".ppm",
            ]
            if sys.platform == "win32":
                invalid_paths.append("CON.ppm")
            for path in invalid_paths:
                with self.subTest(path=path), self.assertRaises(ValueError):
                    session.capture_ppm(path)
            self.assertEqual(session.ping().id, 4)
            self.assertEqual(existing.read_bytes(), b"keep")
        finally:
            session.close()
        self.assert_reaped(session)

    def test_phase5_png_bundle_has_verified_hashes_and_stable_metadata(self) -> None:
        (self.output_root / "captures").mkdir()
        session = self.session("phase5")
        try:
            session.start()
            bundle = session.capture_png("captures/home screen.png")

            manifest_path = (
                self.output_root / "captures" / "home screen.capture.json"
            )
            manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
            png_path = self.output_root / "captures" / "home screen.png"
            ppm_path = self.output_root / "captures" / "home screen.ppm"
            self.assertEqual(
                bundle.png.sha256,
                hashlib.sha256(png_path.read_bytes()).hexdigest(),
            )
            self.assertEqual(
                bundle.ppm.sha256,
                hashlib.sha256(ppm_path.read_bytes()).hexdigest(),
            )
            self.assertEqual(manifest["schema_version"], 1)
            self.assertEqual(
                manifest["display_seq"], bundle.capture.display_sequence
            )
            self.assertEqual(
                manifest["artifacts"]["png"]["sha256"], bundle.png.sha256
            )
            self.assertEqual(
                manifest["artifacts"]["ppm"]["sha256"], bundle.ppm.sha256
            )
            self.assertEqual(
                (read_png(png_path).width, read_png(png_path).height),
                (480, 272),
            )
            self.assertFalse(list(self.output_root.rglob("*.tmp-ui-harness")))
        finally:
            session.close()
        self.assert_reaped(session)

    def test_phase5_static_hashes_match_and_visible_state_changes_hash(self) -> None:
        session = self.session("phase5")
        try:
            session.start()
            static_hashes = []
            for index in range(3):
                artifact = session.capture_ppm(f"static-{index}.ppm")
                path = self.output_root / artifact.path
                static_hashes.append(hashlib.sha256(path.read_bytes()).hexdigest())
            self.assertEqual(len(set(static_hashes)), 1)

            session.key_down("ENTER")
            changed = session.capture_ppm("changed.ppm")
            changed_hash = hashlib.sha256(
                (self.output_root / changed.path).read_bytes()
            ).hexdigest()
            self.assertNotEqual(changed_hash, static_hashes[0])
            session.key_up("ENTER")
        finally:
            session.close()
        self.assert_reaped(session)

    def test_phase5_rejects_a_stale_capture_result(self) -> None:
        session = self.session("phase5-bad-capture")
        try:
            session.start()
            with self.assertRaisesRegex(ProtocolFailure, "newer"):
                session.capture_ppm("stale.ppm")
        finally:
            session.close()
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
        self.assertEqual(payload["describe"]["id"], 2)
        self.assertEqual(payload["ready"]["id"], 3)
        self.assertEqual(payload["stop"]["id"], 4)
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
