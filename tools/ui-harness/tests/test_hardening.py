from __future__ import annotations

import hashlib
import json
import sys
import tempfile
import unittest
from unittest import mock
from pathlib import Path
from typing import Any


HARNESS_ROOT = Path(__file__).resolve().parents[1]
FAKE_SIMULATOR = Path(__file__).with_name("fake_simulator.py")
sys.path.insert(0, str(HARNESS_ROOT))

from edgetx_ui.hardening import (  # noqa: E402
    HardeningError,
    HardeningExecutionError,
    HardeningRunner,
    MAX_CAPTURE_COUNT,
    MAX_LIFECYCLE_CYCLES,
    MAX_LUA_RELOADS,
    MAX_PING_COUNT,
    MAX_WARM_RESTARTS,
)
import edgetx_ui.hardening as hardening_module  # noqa: E402
from edgetx_ui.cli import build_parser  # noqa: E402
from edgetx_ui.session import SimulatorSession  # noqa: E402


class Phase8HardeningTests(unittest.TestCase):
    def setUp(self) -> None:
        self.temporary_directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary_directory.cleanup)
        self.output_root = Path(self.temporary_directory.name)

    def session(self, mode: str) -> SimulatorSession:
        return SimulatorSession(
            sys.executable,
            self.output_root,
            simulator_args=(str(FAKE_SIMULATOR), mode),
            request_timeout=3.0,
            stop_timeout=0.5,
            terminate_timeout=0.5,
            kill_timeout=0.5,
            reader_join_timeout=0.5,
        )

    def assert_reaped(self, session: SimulatorSession) -> None:
        self.assertEqual(session.returncode, 0)
        self.assertFalse(session.reader_threads_alive)

    def test_r01_ten_thousand_requests_remain_correlated(self) -> None:
        session = self.session("normal")
        try:
            session.start()
            first = session.ping()
            previous_id = first.id
            for _ in range(9_999):
                response = session.ping()
                self.assertEqual(response.id, previous_id + 1)
                self.assertTrue(response.ok)
                previous_id = response.id
            self.assertEqual(first.id, 4)
            self.assertEqual(previous_id, 10_003)
            stop = session.stop()
            assert stop is not None
            self.assertEqual(stop.id, 10_004)
        finally:
            session.close()
        self.assert_reaped(session)

    def test_r02_twenty_lua_reloads_and_warm_restarts_are_monotonic(self) -> None:
        session = self.session("phase6")
        try:
            session.start()
            generations = [session.reload_lua().generation for _ in range(20)]
            self.assertEqual(generations, list(range(1, 21)))

            before = session.read_status()
            epochs = []
            sequences = []
            for _ in range(20):
                restarted = session.restart()
                epochs.append(restarted.epoch)
                sequences.append(restarted.display_sequence)
            self.assertEqual(
                epochs, list(range(before.epoch + 1, before.epoch + 21))
            )
            self.assertEqual(sequences, sorted(set(sequences)))
            final = session.read_status()
            self.assertEqual(final.epoch, epochs[-1])
            self.assertEqual(final.active_key_count, 0)
            self.assertFalse(final.touch_active)
            self.assertEqual(final.analog_override_count, 0)
        finally:
            session.close()
        self.assert_reaped(session)

    def test_r03_twenty_static_captures_are_identical_and_isolated(self) -> None:
        session = self.session("phase5")
        try:
            session.start()
            hashes = []
            sequences = []
            for index in range(20):
                artifact = session.capture_ppm(f"static-{index:02d}.ppm")
                capture_path = self.output_root / artifact.path
                hashes.append(hashlib.sha256(capture_path.read_bytes()).hexdigest())
                sequences.append(artifact.display_sequence)
            self.assertEqual(len(set(hashes)), 1)
            self.assertEqual(sequences, sorted(set(sequences)))

            session.key_down("ENTER")
            changed = session.capture_ppm("changed.ppm")
            changed_hash = hashlib.sha256(
                (self.output_root / changed.path).read_bytes()
            ).hexdigest()
            self.assertNotEqual(changed_hash, hashes[0])
            session.key_up("ENTER")

            self.assertEqual(len(list(self.output_root.glob("static-*.ppm"))), 20)
            self.assertFalse(list(self.output_root.rglob("*.tmp-ui-harness")))
        finally:
            session.close()
        self.assert_reaped(session)

    def test_r04_runner_preserves_a_complete_machine_readable_report(self) -> None:
        fixture = self.output_root / "fixture"
        (fixture / "settings").mkdir(parents=True)
        (fixture / "sdcard").mkdir()
        (fixture / "settings" / "radio.yml").write_text(
            "fixture\n", encoding="utf-8"
        )
        fixture_before = hashlib.sha256(
            (fixture / "settings" / "radio.yml").read_bytes()
        ).hexdigest()

        result = HardeningRunner(
            fixture,
            self.output_root / "runs",
            sys.executable,
            simulator_args=(str(FAKE_SIMULATOR), "phase6"),
            lifecycle_cycles=2,
            ping_count=25,
            lua_reloads=2,
            warm_restarts=2,
            capture_count=2,
            expected_target="test-target",
        ).run()

        report = json.loads(result.report.read_text(encoding="utf-8"))
        self.assertTrue(result.success)
        self.assertTrue(report["success"])
        self.assertEqual(report["lifecycle"]["completed"], 2)
        self.assertEqual(report["stress"]["ping"]["completed"], 25)
        self.assertEqual(report["stress"]["lua"]["generations"], [1, 2])
        self.assertEqual(report["stress"]["warm_restart"]["completed"], 2)
        self.assertEqual(report["stress"]["visual"]["completed"], 2)
        self.assertTrue(report["stress"]["visual"]["identical"])
        self.assertTrue(report["stress"]["visual"]["changed_differs"])
        self.assertTrue(report["cleanup"]["no_temporaries"])
        self.assertTrue(report["reap"]["all_reaped"])
        for cycle in report["lifecycle"]["cycles"]:
            self.assertFalse(cycle["writer_thread_alive"])
            evidence = cycle["protocol"]
            self.assertTrue(evidence["available"])
            self.assertGreater(evidence["records"], 0)
            protocol = result.run_directory / evidence["path"]
            self.assertEqual(
                hashlib.sha256(protocol.read_bytes()).hexdigest(),
                evidence["sha256"],
            )
        stress_evidence = report["stress"]["protocol"]
        self.assertTrue(stress_evidence["available"])
        self.assertGreater(stress_evidence["records"], 0)
        self.assertNotIn("direction", stress_evidence)
        self.assertFalse(report["stress"]["reap"]["writer_thread_alive"])
        self.assertTrue(report["fixture"]["unchanged"])
        self.assertEqual(
            hashlib.sha256(
                (fixture / "settings" / "radio.yml").read_bytes()
            ).hexdigest(),
            fixture_before,
        )

    def fixture(self) -> Path:
        fixture = self.output_root / "failure-fixture"
        (fixture / "settings").mkdir(parents=True)
        (fixture / "sdcard").mkdir()
        return fixture

    def test_failure_contract_preserves_report_and_cleanup_evidence(self) -> None:
        fixture = self.fixture()
        report_path = self.output_root / "failure-report.json"
        runner = HardeningRunner(
            fixture,
            self.output_root / "failure-runs",
            sys.executable,
            report_path=report_path,
            simulator_args=(str(FAKE_SIMULATOR), "command-error"),
            lifecycle_cycles=1,
            ping_count=0,
            lua_reloads=0,
            warm_restarts=0,
            capture_count=0,
            expected_target="test-target",
        )

        with self.assertRaises(HardeningExecutionError) as caught:
            runner.run()
        payload = json.loads(caught.exception.result.report.read_text(encoding="utf-8"))
        self.assertFalse(payload["success"])
        self.assertEqual(payload["failure"]["stage"], "lifecycle")
        self.assertEqual(payload["lifecycle"]["completed"], 0)
        self.assertEqual(len(payload["lifecycle"]["cycles"]), 1)
        self.assertTrue(payload["reap"]["all_reaped"])
        self.assertTrue(payload["fixture"]["unchanged"])
        self.assertTrue(payload["cleanup"]["no_temporaries"])

    def test_failure_contract_captures_base_exception_and_reap_state(self) -> None:
        class DeliberateAbort(BaseException):
            pass

        class AbortingSession:
            def __init__(self, *args: object, **kwargs: object) -> None:
                self.process = None
                self.returncode = None
                self.termination_stage = "not-started"
                self.reader_threads_alive = False
                self.cwd = Path(kwargs["cwd"])

            def start(self, **kwargs: object) -> None:
                raise DeliberateAbort("deliberate base exception at " + str(self.cwd))

            def close(self) -> None:
                self.returncode = -1
                self.termination_stage = "closed"

        fixture = self.fixture()
        runner = HardeningRunner(
            fixture,
            self.output_root / "base-exception-runs",
            "fixture-simulator",
            lifecycle_cycles=1,
            ping_count=0,
            lua_reloads=0,
            warm_restarts=0,
            capture_count=0,
            expected_target="test-target",
            session_factory=AbortingSession,
        )
        with self.assertRaises(HardeningExecutionError) as caught:
            runner.run()
        payload = json.loads(caught.exception.result.report.read_text(encoding="utf-8"))
        self.assertEqual(payload["failure"]["error_type"], "DeliberateAbort")
        self.assertEqual(payload["failure"]["stage"], "lifecycle")
        self.assertEqual(payload["lifecycle"]["cycles"][0]["returncode"], -1)
        self.assertTrue(payload["reap"]["all_reaped"])
        self.assertNotIn(
            self.output_root.as_posix(),
            json.dumps(payload).replace("\\", "/"),
        )

    def test_failure_contract_identifies_each_public_api_stage(self) -> None:
        fixture = self.fixture()

        class InjectedFailureSession:
            def __init__(
                self, failure_method: str, *args: object, **kwargs: object
            ) -> None:
                self._failure_method = failure_method
                self._session = SimulatorSession(*args, **kwargs)

            def __getattr__(self, name: str) -> Any:
                value = getattr(self._session, name)
                if name != self._failure_method:
                    return value

                def fail(*args: object, **kwargs: object) -> None:
                    raise RuntimeError("injected failure in " + name)

                return fail

        stages = (
            ("ping", "ping"),
            ("reload_lua", "lua"),
            ("restart", "restart"),
            ("capture_ppm", "capture"),
            ("stop", "cleanup"),
        )
        for failure_method, expected_stage in stages:
            with self.subTest(stage=expected_stage):
                def factory(
                    *args: object,
                    _method: str = failure_method,
                    **kwargs: object,
                ) -> InjectedFailureSession:
                    return InjectedFailureSession(_method, *args, **kwargs)

                runner = HardeningRunner(
                    fixture,
                    self.output_root / (expected_stage + "-failure-runs"),
                    sys.executable,
                    simulator_args=(str(FAKE_SIMULATOR), "phase6"),
                    lifecycle_cycles=0,
                    ping_count=1,
                    lua_reloads=1,
                    warm_restarts=1,
                    capture_count=1,
                    expected_target="test-target",
                    session_factory=factory,
                )
                with self.assertRaises(HardeningExecutionError) as caught:
                    runner.run()
                payload = json.loads(
                    caught.exception.result.report.read_text(encoding="utf-8")
                )
                self.assertEqual(payload["failure"]["stage"], expected_stage)
                self.assertTrue(payload["fixture"]["unchanged"])
                self.assertTrue(payload["cleanup"]["no_temporaries"])
                self.assertTrue(payload["reap"]["all_reaped"])
                self.assertIsNotNone(payload["stress"]["reap"]["returncode"])

    def test_cleanup_evidence_collection_failure_is_reported(self) -> None:
        fixture = self.fixture()
        runner = HardeningRunner(
            fixture,
            self.output_root / "cleanup-collection-runs",
            sys.executable,
            simulator_args=(str(FAKE_SIMULATOR), "phase6"),
            lifecycle_cycles=0,
            ping_count=0,
            lua_reloads=0,
            warm_restarts=0,
            capture_count=0,
            expected_target="test-target",
        )
        with mock.patch.object(
            hardening_module,
            "_temporary_paths",
            side_effect=OSError("cleanup evidence unavailable"),
        ):
            with self.assertRaises(HardeningExecutionError) as caught:
                runner.run()
        payload = json.loads(caught.exception.result.report.read_text(encoding="utf-8"))
        self.assertEqual(payload["failure"]["stage"], "cleanup")
        self.assertFalse(payload["cleanup"]["no_temporaries"])
        self.assertEqual(
            payload["cleanup"]["collection_error"]["error_type"], "OSError"
        )
        self.assertTrue(payload["fixture"]["unchanged"])
        self.assertTrue(payload["reap"]["all_reaped"])

    def test_report_publication_failure_preserves_fallback_evidence(self) -> None:
        fixture = self.fixture()
        requested = self.output_root / "unpublished-report.json"
        runner = HardeningRunner(
            fixture,
            self.output_root / "publication-failure-runs",
            sys.executable,
            report_path=requested,
            simulator_args=(str(FAKE_SIMULATOR), "phase6"),
            lifecycle_cycles=0,
            ping_count=0,
            lua_reloads=0,
            warm_restarts=0,
            capture_count=0,
            expected_target="test-target",
        )
        original_write = hardening_module._write_report

        def fail_requested(
            path: Path, payload: object, *, force: bool = False
        ) -> None:
            if path == requested.resolve():
                raise OSError("publication deliberately unavailable")
            original_write(path, payload, force=force)

        with mock.patch.object(
            hardening_module, "_write_report", side_effect=fail_requested
        ):
            with self.assertRaises(HardeningExecutionError) as caught:
                runner.run()
        self.assertFalse(requested.exists())
        self.assertNotEqual(caught.exception.result.report, requested)
        payload = json.loads(caught.exception.result.report.read_text(encoding="utf-8"))
        self.assertEqual(payload["failure"]["stage"], "report")
        self.assertTrue(payload["fixture"]["unchanged"])
        self.assertTrue(payload["cleanup"]["no_temporaries"])
        self.assertTrue(payload["reap"]["all_reaped"])

    def test_report_is_exclusive_by_default_and_force_is_explicit(self) -> None:
        fixture = self.fixture()
        report_path = self.output_root / "exclusive.json"
        report_path.write_text("keep\n", encoding="utf-8")

        options: dict[str, Any] = {
            "report_path": report_path,
            "simulator_args": (str(FAKE_SIMULATOR), "phase6"),
            "lifecycle_cycles": 0,
            "ping_count": 0,
            "lua_reloads": 0,
            "warm_restarts": 0,
            "capture_count": 0,
            "expected_target": "test-target",
        }
        with self.assertRaisesRegex(HardeningError, "already exists"):
            HardeningRunner(
                fixture, self.output_root / "exclusive-runs", sys.executable, **options
            ).run()
        self.assertEqual(report_path.read_text(encoding="utf-8"), "keep\n")

        result = HardeningRunner(
            fixture,
            self.output_root / "exclusive-runs",
            sys.executable,
            force_report=True,
            **options,
        ).run()
        self.assertTrue(result.success)
        self.assertTrue(json.loads(report_path.read_text(encoding="utf-8"))["success"])

        with self.assertRaisesRegex(HardeningError, "inside the fixture"):
            HardeningRunner(
                fixture,
                self.output_root / "fixture-report-runs",
                sys.executable,
                report_path=fixture / "report.json",
                force_report=True,
                lifecycle_cycles=0,
                ping_count=0,
                lua_reloads=0,
                warm_restarts=0,
                capture_count=0,
                expected_target="test-target",
            ).run()

    def test_cli_force_flag_is_explicit_and_disabled_by_default(self) -> None:
        parser = build_parser()
        normal = parser.parse_args(["harden", "fixture-simulator"])
        forced = parser.parse_args(["harden", "--force", "fixture-simulator"])
        self.assertFalse(normal.force)
        self.assertTrue(forced.force)

    def test_runner_rejects_each_count_one_above_its_documented_limit(self) -> None:
        fixture = self.fixture()
        limits = (
            ("lifecycle_cycles", MAX_LIFECYCLE_CYCLES),
            ("ping_count", MAX_PING_COUNT),
            ("lua_reloads", MAX_LUA_RELOADS),
            ("warm_restarts", MAX_WARM_RESTARTS),
            ("capture_count", MAX_CAPTURE_COUNT),
        )
        for argument, maximum in limits:
            with self.subTest(argument=argument):
                with self.assertRaisesRegex(ValueError, "must not exceed"):
                    HardeningRunner(
                        fixture,
                        self.output_root / (argument + "-runs"),
                        sys.executable,
                        **{argument: maximum + 1},
                    )

    def test_report_redacts_machine_specific_paths_and_command(self) -> None:
        fixture = self.fixture()
        result = HardeningRunner(
            fixture,
            self.output_root / "redaction-runs",
            sys.executable,
            simulator_args=(str(FAKE_SIMULATOR), "phase6"),
            lifecycle_cycles=0,
            ping_count=0,
            lua_reloads=0,
            warm_restarts=0,
            capture_count=0,
            expected_target="test-target",
        ).run()
        text = result.report.read_text(encoding="utf-8")
        payload = json.loads(text)

        self.assertNotIn(self.output_root.as_posix(), text.replace("\\", "/"))
        self.assertEqual(payload["fixture"]["path"], "<fixture>")
        self.assertEqual(payload["run_directory"], "<run>")
        self.assertEqual(payload["simulator"]["command"][0], Path(sys.executable).name)
        self.assertIn("phase6", payload["simulator"]["command"])


if __name__ == "__main__":
    unittest.main()
