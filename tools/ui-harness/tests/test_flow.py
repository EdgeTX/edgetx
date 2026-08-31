from __future__ import annotations

import contextlib
import hashlib
import io
import json
import sys
import tempfile
import unittest
from pathlib import Path
from types import SimpleNamespace
from typing import Any


HARNESS_ROOT = Path(__file__).resolve().parents[1]
REPOSITORY_ROOT = HARNESS_ROOT.parents[1]
sys.path.insert(0, str(HARNESS_ROOT))

from edgetx_ui.cli import main  # noqa: E402
from edgetx_ui.flow import (  # noqa: E402
    FlowExecutionError,
    FlowRunner,
    FlowValidationError,
    load_flow,
)
from edgetx_ui.protocol import Response  # noqa: E402


def _response(request_id: int, command: str) -> Response:
    raw = {
        "version": 1,
        "type": "response",
        "id": request_id,
        "epoch": 1,
        "ok": True,
        "result": {"command": command},
    }
    return Response(request_id, True, 1, raw["result"], None, None, raw)


class _FlowSession:
    instances: list["_FlowSession"] = []

    def __init__(self, executable: str, output_root: Path, **options: Any) -> None:
        self.executable = executable
        self.output_root = Path(output_root)
        self.options = options
        self.protocol_records: list[dict[str, Any]] = []
        self.recent_stderr = "bounded fixture diagnostic"
        self.returncode = None
        self.termination_stage = "not-started"
        self.status_response = None
        self.description = SimpleNamespace(
            target="tx16s",
            lcd=SimpleNamespace(width=480, height=272, depth=16),
        )
        self._next_id = 1
        type(self).instances.append(self)

    def _reply(self, command: str) -> Response:
        request_id = self._next_id
        self._next_id += 1
        request = {
            "version": 1,
            "id": request_id,
            "command": command,
            "args": [],
        }
        response = _response(request_id, command)
        self.protocol_records.extend(
            (
                {"direction": "request", "message": request},
                {"direction": "response", "message": response.raw},
            )
        )
        return response

    def start(self, **options: Any) -> Response:
        self.termination_stage = "running"
        self.status_response = self._reply("wait-ready")
        return self.status_response

    def release_all(self, **options: Any) -> Response:
        return self._reply("release-all")

    def capture_png(self, path: str, **options: Any) -> dict[str, str]:
        target = self.output_root / Path(path)
        target.with_suffix(".ppm").write_bytes(b"P6\n1 1\n255\n\x00\x00\x00")
        target.write_bytes(b"test-png")
        target.with_suffix(".capture.json").write_text("{}\n", encoding="utf-8")
        self._reply("capture")
        return {"path": path}

    def stop(self, **options: Any) -> Response:
        if self.termination_stage != "graceful":
            response = self._reply("stop")
            self.returncode = 0
            self.termination_stage = "graceful"
            return response
        return self._reply("stop")

    def close(self) -> None:
        self.returncode = 0
        self.termination_stage = "closed"


class _FailingFlowSession(_FlowSession):
    def release_all(self, **options: Any) -> Response:
        raise RuntimeError("deliberate step failure")


class FlowScenarioTests(unittest.TestCase):
    def setUp(self) -> None:
        self.temporary_directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary_directory.cleanup)
        self.root = Path(self.temporary_directory.name)
        self.fixture = self.root / "fixture"
        (self.fixture / "settings").mkdir(parents=True)
        (self.fixture / "sdcard").mkdir()
        (self.fixture / "settings" / "radio.yml").write_text(
            "fixture: immutable\n", encoding="utf-8"
        )
        self.runs = self.root / "runs"
        _FlowSession.instances.clear()

    def write_flow(self, payload: dict[str, Any], name: str = "flow.json") -> Path:
        path = self.root / name
        path.write_text(json.dumps(payload), encoding="utf-8")
        return path

    def minimal_payload(self) -> dict[str, Any]:
        return {
            "schema": 1,
            "target": "tx16s",
            "requires": ["capture"],
            "steps": [
                {"action": "wait-ready", "timeout_ms": 1000},
                {"action": "release-all"},
            ],
        }

    def runner(self, flow_path: Path, session_factory: Any = _FlowSession) -> FlowRunner:
        return FlowRunner(
            load_flow(flow_path),
            self.fixture,
            self.runs,
            "fixture-simulator",
            session_factory=session_factory,
        )

    def test_q01_strict_schema_accepts_only_the_canonical_shape(self) -> None:
        flow = load_flow(self.write_flow(self.minimal_payload()))
        self.assertEqual(flow.schema, 1)
        self.assertEqual(flow.target, "tx16s")
        duplicate = self.root / "duplicate.json"
        duplicate.write_text(
            '{"schema":1,"schema":1,"target":"tx16s","requires":[],"steps":[]}',
            encoding="utf-8",
        )
        with self.assertRaisesRegex(FlowValidationError, "duplicate JSON key"):
            load_flow(duplicate)

    def test_q02_unknown_top_level_and_step_fields_are_rejected(self) -> None:
        top = self.minimal_payload() | {"output": "somewhere"}
        with self.assertRaisesRegex(FlowValidationError, "unknown output"):
            load_flow(self.write_flow(top, "unknown-top.json"))
        step = self.minimal_payload()
        step["steps"][1]["surprise"] = True
        with self.assertRaisesRegex(FlowValidationError, "unknown surprise"):
            load_flow(self.write_flow(step, "unknown-step.json"))

    def test_q03_target_ranges_are_rejected_before_session_creation(self) -> None:
        payload = self.minimal_payload()
        payload["steps"][1] = {"action": "tap", "x": 480, "y": 0, "hold_ms": 1}
        with self.assertRaisesRegex(FlowValidationError, "x must be"):
            load_flow(self.write_flow(payload))
        self.assertEqual(_FlowSession.instances, [])

    def test_q04_required_capabilities_and_target_are_startup_contracts(self) -> None:
        path = self.write_flow(self.minimal_payload())
        self.runner(path).run()
        options = _FlowSession.instances[-1].options
        self.assertEqual(options["required_capabilities"], ("capture",))
        self.assertEqual(options["expected_target"], "tx16s")
        self.assertEqual(options["expected_lcd"], (480, 272, 16))

    def test_q05_fixture_template_is_never_used_as_writable_state(self) -> None:
        before = hashlib.sha256((self.fixture / "settings" / "radio.yml").read_bytes()).hexdigest()
        result = self.runner(self.write_flow(self.minimal_payload())).run()
        after = hashlib.sha256((self.fixture / "settings" / "radio.yml").read_bytes()).hexdigest()
        self.assertEqual(before, after)
        self.assertTrue((result.run_directory / "settings" / "radio.yml").is_file())
        manifest = json.loads(result.manifest.read_text(encoding="utf-8"))
        self.assertTrue(manifest["fixture"]["unchanged"])

    def test_q06_each_execution_owns_a_unique_run_directory(self) -> None:
        path = self.write_flow(self.minimal_payload())
        first = self.runner(path).run()
        second = self.runner(path).run()
        self.assertNotEqual(first.run_directory, second.run_directory)
        self.assertTrue(first.run_directory.is_dir())
        self.assertTrue(second.run_directory.is_dir())

    def test_q07_manifest_protocol_and_artifact_hashes_are_verified(self) -> None:
        payload = self.minimal_payload()
        payload["steps"].insert(
            1,
            {"action": "capture", "name": "home", "format": "png", "timeout_ms": 1000},
        )
        result = self.runner(self.write_flow(payload)).run()
        manifest = json.loads(result.manifest.read_text(encoding="utf-8"))
        self.assertTrue(manifest["success"])
        self.assertGreater(len(manifest["protocol"]), 0)
        paths = {item["path"]: item for item in manifest["artifacts"]}
        for required in (
            "protocol.jsonl",
            "stderr.log",
            "artifacts/checkpoints/home.ppm",
            "artifacts/checkpoints/home.png",
            "artifacts/checkpoints/home.capture.json",
        ):
            artifact = result.run_directory / required
            self.assertEqual(
                hashlib.sha256(artifact.read_bytes()).hexdigest(),
                paths[required]["sha256"],
            )

    def test_q08_failed_step_and_bounded_stderr_survive_in_manifest(self) -> None:
        runner = self.runner(self.write_flow(self.minimal_payload()), _FailingFlowSession)
        with self.assertRaises(FlowExecutionError) as caught:
            runner.run()
        manifest = json.loads(caught.exception.result.manifest.read_text(encoding="utf-8"))
        self.assertFalse(manifest["success"])
        self.assertEqual(manifest["failure"]["step"], 1)
        self.assertEqual(manifest["failure"]["action"], "release-all")
        self.assertIn("bounded fixture diagnostic", (caught.exception.result.run_directory / "stderr.log").read_text())

    def test_q09_checked_in_smoke_is_the_documented_phase7_contract(self) -> None:
        flow = load_flow(HARNESS_ROOT / "flows" / "tx16s-smoke.json")
        actions = {step["action"] for step in flow.steps}
        self.assertTrue(
            {
                "wait-ready", "press", "rotate", "tap", "set-switch", "set-analog",
                "set-telemetry", "reload-lua", "wait-next-frame", "capture", "release-all",
            }.issubset(actions)
        )
        readme = (HARNESS_ROOT / "README.md").read_text(encoding="utf-8")
        self.assertIn("edgetx-ui smoke", readme)

    def test_q10_cli_returns_nonzero_and_preserves_manifest_on_failure(self) -> None:
        path = self.write_flow(self.minimal_payload())
        stderr = io.StringIO()
        with contextlib.redirect_stderr(stderr):
            exit_code = main(
                [
                    "run-flow", str(path), "--fixture", str(self.fixture),
                    "--runs", str(self.runs), str(self.root / "missing-simulator"),
                ]
            )
        self.assertEqual(exit_code, 2)
        payload = json.loads(stderr.getvalue())
        self.assertFalse(payload["ok"])
        self.assertTrue(Path(payload["manifest"]).is_file())


if __name__ == "__main__":
    unittest.main()
