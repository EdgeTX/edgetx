import argparse
import json
import sys

from .core import HarnessError, HarnessService, build_simulator


def print_json(payload: object) -> None:
    print(json.dumps(payload, indent=2))


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(prog="edgetx-ui")
    sub = parser.add_subparsers(dest="command", required=True)

    build = sub.add_parser("build", help="configure and build the simulator")
    build.add_argument("target", nargs="?", default="tx16s")

    run_flow = sub.add_parser("run-flow", help="run a JSON UI automation flow")
    run_flow.add_argument("flow")

    smoke = sub.add_parser("smoke", help="capture a small TX16S screenshot smoke flow")
    smoke.add_argument("--target", default="tx16s")
    smoke.add_argument("--out", default=None)

    start = sub.add_parser("start", help="start simulator, print status, then stop")
    start.add_argument("target", nargs="?", default="tx16s")
    start.add_argument("--sdcard")
    start.add_argument("--settings")

    args = parser.parse_args(argv)
    service = HarnessService()

    try:
        if args.command == "build":
            print_json(build_simulator(args.target))
        elif args.command == "run-flow":
            print_json(service.run_flow(args.flow))
        elif args.command == "smoke":
            service.start(args.target)
            try:
                service.wait(1000)
                service.press("ENTER")
                service.wait(2000)
                home = service.screenshot("home", args.out)
                service.press("MODEL")
                service.wait(500)
                menu = service.screenshot("menu", args.out)
                service.press("EXIT")
                service.wait(500)
                returned = service.screenshot("return-home", args.out)
                print_json({"screenshots": [home, menu, returned]})
            finally:
                service.stop()
        elif args.command == "start":
            print_json(service.start(args.target, args.sdcard, args.settings))
            service.stop()
        return 0
    except HarnessError as exc:
        print(str(exc), file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
