from __future__ import annotations

import hashlib
import json
import sys
import tempfile
import unittest
from pathlib import Path


HARNESS_ROOT = Path(__file__).resolve().parents[1]
FAKE_SIMULATOR = Path(__file__).with_name("fake_simulator.py")
sys.path.insert(0, str(HARNESS_ROOT))

from edgetx_ui.hardening import HardeningRunner  # noqa: E402
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
        self.assertTrue(report["fixture"]["unchanged"])
        self.assertEqual(
            hashlib.sha256(
                (fixture / "settings" / "radio.yml").read_bytes()
            ).hexdigest(),
            fixture_before,
        )


if __name__ == "__main__":
    unittest.main()
