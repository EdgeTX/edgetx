"""Minimal command-line entry point for the simulator session foundation."""

from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path
from typing import Optional, Sequence

from .session import SessionError, SimulatorSession


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
    return parser


def main(argv: Optional[Sequence[str]] = None) -> int:
    args = build_parser().parse_args(argv)
    if args.command != "probe":
        raise AssertionError("unhandled command")

    simulator_args = list(args.simulator_args)
    if simulator_args[:1] == ["--"]:
        simulator_args = simulator_args[1:]
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


if __name__ == "__main__":
    raise SystemExit(main())
