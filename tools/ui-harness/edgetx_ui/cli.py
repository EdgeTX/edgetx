"""Minimal command-line entry point for the simulator session foundation."""

from __future__ import annotations

import argparse
import json
import os
import subprocess
import sys
from pathlib import Path
from typing import Optional, Sequence

from .flow import FlowExecutionError, FlowRunner, FlowValidationError, load_flow
from .hardening import HardeningExecutionError, HardeningRunner
from .session import SessionError, SimulatorSession


REPOSITORY_ROOT = Path(__file__).resolve().parents[3]


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="edgetx-ui")
    subcommands = parser.add_subparsers(dest="command", required=True)

    probe = subcommands.add_parser(
        "probe", help="launch a simulator and verify first-frame readiness"
    )
    probe.add_argument("--output", required=True, type=Path)
    probe.add_argument("--timeout", type=float, default=5.0)
    probe.add_argument("simulator")
    probe.add_argument(
        "simulator_args",
        nargs=argparse.REMAINDER,
        help="arguments passed to the simulator after an optional -- separator",
    )

    harden = subcommands.add_parser(
        "harden", help="run reproducible TX16S lifecycle and visual hardening"
    )
    harden.add_argument(
        "--fixture",
        type=Path,
        default=REPOSITORY_ROOT / "tools" / "ui-harness" / "fixtures" / "tx16s",
    )
    harden.add_argument(
        "--runs",
        type=Path,
        default=REPOSITORY_ROOT / "build" / "ui-harness" / "hardening",
    )
    harden.add_argument("--report", type=Path)
    harden.add_argument("--timeout", type=float, default=10.0)
    harden.add_argument("--lifecycle-cycles", type=int, default=100)
    harden.add_argument("--ping-count", type=int, default=10_000)
    harden.add_argument("--lua-reloads", type=int, default=20)
    harden.add_argument("--warm-restarts", type=int, default=20)
    harden.add_argument("--captures", type=int, default=20)
    harden.add_argument("simulator")
    harden.add_argument(
        "simulator_args",
        nargs=argparse.REMAINDER,
        help="arguments passed to the simulator after an optional -- separator",
    )

    for name, help_text in (
        ("run-flow", "validate and run a strict JSON UI automation flow"),
        ("smoke", "run the checked-in TX16S smoke flow"),
    ):
        command = subcommands.add_parser(name, help=help_text)
        if name == "run-flow":
            command.add_argument("flow", type=Path)
        command.add_argument("--fixture", type=Path)
        command.add_argument(
            "--runs",
            type=Path,
            default=REPOSITORY_ROOT / "build" / "ui-harness" / "runs",
        )
        command.add_argument("--timeout", type=float, default=10.0)
        if name == "smoke":
            command.add_argument(
                "--build-dir",
                type=Path,
                help="configure and incrementally build the TX16S simulator before running",
            )
            command.add_argument("simulator", nargs="?")
        else:
            command.add_argument("simulator")
        command.add_argument(
            "simulator_args",
            nargs=argparse.REMAINDER,
            help="arguments passed to the simulator after an optional -- separator",
        )
    return parser


def main(argv: Optional[Sequence[str]] = None) -> int:
    args = build_parser().parse_args(argv)
    simulator_args = list(args.simulator_args)
    if simulator_args[:1] == ["--"]:
        simulator_args = simulator_args[1:]

    if args.command in ("run-flow", "smoke"):
        flow_path = (
            args.flow
            if args.command == "run-flow"
            else REPOSITORY_ROOT / "tools" / "ui-harness" / "flows" / "tx16s-smoke.json"
        )
        try:
            flow = load_flow(flow_path)
            simulator = args.simulator
            if args.command == "smoke" and args.build_dir is not None:
                if simulator is not None:
                    raise FlowValidationError(
                        "smoke accepts either --build-dir or a simulator path, not both"
                    )
                simulator = _build_tx16s_simulator(args.build_dir)
            if simulator is None:
                raise FlowValidationError(
                    "smoke requires --build-dir or an existing simulator path"
                )
            fixture = args.fixture or (
                REPOSITORY_ROOT / "tools" / "ui-harness" / "fixtures" / flow.target
            )
            result = FlowRunner(
                flow,
                fixture,
                args.runs,
                simulator,
                simulator_args=simulator_args,
                command_timeout=args.timeout,
            ).run()
            print(
                json.dumps(
                    {
                        "ok": True,
                        "run": str(result.run_directory),
                        "manifest": str(result.manifest),
                    },
                    indent=2,
                    sort_keys=True,
                )
            )
            return 0
        except FlowExecutionError as error:
            print(
                json.dumps(
                    {
                        "ok": False,
                        "error": str(error),
                        "run": str(error.result.run_directory),
                        "manifest": str(error.result.manifest),
                    },
                    indent=2,
                    sort_keys=True,
                ),
                file=sys.stderr,
            )
            return 2
        except (FlowValidationError, OSError, ValueError) as error:
            print(str(error), file=sys.stderr)
            return 2

    if args.command == "harden":
        try:
            result = HardeningRunner(
                args.fixture,
                args.runs,
                args.simulator,
                report_path=args.report,
                simulator_args=simulator_args,
                command_timeout=args.timeout,
                lifecycle_cycles=args.lifecycle_cycles,
                ping_count=args.ping_count,
                lua_reloads=args.lua_reloads,
                warm_restarts=args.warm_restarts,
                capture_count=args.captures,
            ).run()
            print(
                json.dumps(
                    {
                        "ok": True,
                        "run": str(result.run_directory),
                        "report": str(result.report),
                    },
                    indent=2,
                    sort_keys=True,
                )
            )
            return 0
        except HardeningExecutionError as error:
            print(
                json.dumps(
                    {
                        "ok": False,
                        "error": str(error),
                        "run": str(error.result.run_directory),
                        "report": str(error.result.report),
                    },
                    indent=2,
                    sort_keys=True,
                ),
                file=sys.stderr,
            )
            return 2
        except (OSError, RuntimeError, ValueError) as error:
            print(str(error), file=sys.stderr)
            return 2

    if args.command != "probe":
        raise AssertionError("unhandled command")

    args.output.mkdir(parents=True, exist_ok=True)

    session = SimulatorSession(
        args.simulator,
        args.output,
        simulator_args=simulator_args,
        request_timeout=args.timeout,
        stop_timeout=args.timeout,
        terminate_timeout=args.timeout,
        kill_timeout=args.timeout,
    )
    try:
        ready = session.start(timeout=args.timeout)
        stop = session.stop(timeout=args.timeout)
        assert session.startup_ping is not None
        assert session.description_response is not None
        payload = {
            "ok": True,
            "ping": session.startup_ping.raw,
            "describe": session.description_response.raw,
            "ready": ready.raw,
            "stop": stop.raw if stop is not None else None,
            "returncode": session.returncode,
            "termination": session.termination_stage,
        }
        print(json.dumps(payload, indent=2, sort_keys=True))
        return 0
    except (SessionError, ValueError) as error:
        session.close()
        print(str(error), file=sys.stderr)
        return 2


def _build_tx16s_simulator(build_dir: Path) -> str:
    build = build_dir.resolve()
    configure = [
        "cmake",
        "-S",
        str(REPOSITORY_ROOT),
        "-B",
        str(build),
        "-G",
        os.environ.get("CMAKE_GENERATOR", "Ninja"),
        "-DCMAKE_TOOLCHAIN_FILE=cmake/toolchain/native.cmake",
        "-DEdgeTX_SUPERBUILD=OFF",
        "-DNATIVE_BUILD=ON",
        "-DDISABLE_COMPANION=ON",
        "-DPCB=X10",
        "-DPCBREV=TX16S",
        "-DDEFAULT_MODE=2",
    ]
    _run_build_command(configure, "configure TX16S simulator")
    _run_build_command(
        ["cmake", "--build", str(build), "--target", "simu", "--parallel"],
        "build TX16S simulator",
    )
    candidates = sorted(
        path
        for name in ("simu", "simu.exe")
        for path in build.rglob(name)
        if path.is_file()
    )
    if not candidates:
        raise FlowValidationError("built simulator executable was not found under " + str(build))
    return str(candidates[0])


def _run_build_command(command: Sequence[str], label: str) -> None:
    result = subprocess.run(
        list(command),
        cwd=REPOSITORY_ROOT,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        check=False,
    )
    if result.returncode != 0:
        raise FlowValidationError(
            label + " failed:\n" + result.stdout[-8000:]
        )


if __name__ == "__main__":
    raise SystemExit(main())
