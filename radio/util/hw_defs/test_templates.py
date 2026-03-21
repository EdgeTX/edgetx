"""Test that all templates render successfully for all board hw_defs.

Runs all .jinja templates against all .json board definitions in a single
process, accumulating failures rather than stopping at the first one.

Usage:
    python3 test_templates.py <hw_defs_dir>
"""

import argparse
import contextlib
import io
import os
import sys
from pathlib import Path

from generator import generate_from_template


def main():
    parser = argparse.ArgumentParser(description="Test all templates x all boards")
    parser.add_argument("hw_defs_dir", help="Directory containing board .json files")
    args = parser.parse_args()

    hw_defs_dir = Path(args.hw_defs_dir)
    if not hw_defs_dir.is_dir():
        print(f"ERROR: {hw_defs_dir} is not a directory", file=sys.stderr)
        sys.exit(1)

    script_dir = Path(os.path.dirname(os.path.abspath(__file__)))
    templates = sorted(script_dir.glob("*.jinja"))
    json_files = sorted(hw_defs_dir.glob("*.json"))

    if not templates:
        print("ERROR: no .jinja templates found", file=sys.stderr)
        sys.exit(1)

    if not json_files:
        print(f"ERROR: no .json files found in {hw_defs_dir}", file=sys.stderr)
        sys.exit(1)

    print(f"Testing {len(json_files)} boards x {len(templates)} templates "
          f"= {len(json_files) * len(templates)} combinations")

    failures = []

    for json_file in json_files:
        board = json_file.stem
        for tmpl in templates:
            try:
                with contextlib.redirect_stdout(io.StringIO()):
                    generate_from_template(str(json_file), str(tmpl), board)
            except SystemExit as e:
                if e.code != 0:
                    failures.append((board, tmpl.name))

    if failures:
        print(f"\nFAILED ({len(failures)}):", file=sys.stderr)
        for board, tmpl in failures:
            print(f"  board={board} template={tmpl}", file=sys.stderr)
        sys.exit(1)

    print(f"All combinations passed.")


if __name__ == "__main__":
    main()
