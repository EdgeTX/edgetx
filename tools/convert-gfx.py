#!/usr/bin/env python3
"""
Convert SVG files to PNG icons for different screen resolutions.
Manages conversion, validation, and incremental builds with CSV tracking.
"""

import sys
import csv
import logging
import shutil
import subprocess
import time
from pathlib import Path
from datetime import datetime
from typing import Tuple, Optional, List, Dict, Set
from PIL import Image
from rich.progress import Progress, SpinnerColumn, BarColumn, TextColumn, TaskProgressColumn
from rich.logging import RichHandler
from rich.console import Console


# Set up logging and console
logger = logging.getLogger("convert-gfx")
console = Console()

# Configuration
SCRIPT_DIR = Path(__file__).parent.resolve()
BASE_DIR = SCRIPT_DIR.parent / "radio" / "src" / "bitmaps"
SRC_DIR = BASE_DIR / "img-src"
SRC_LIST = SCRIPT_DIR / "convert-gfx-list.csv"

RESOLUTIONS = {
    "320x240": 0.8,
    "480x272": 1.0,
    "800x480": 1.375,
}

BG_COLOR = "white"


def get_png_dimensions(png_path: Path) -> Optional[Tuple[int, int]]:
    """Extract PNG dimensions using PIL."""
    try:
        if not png_path.exists():
            return None
        img = Image.open(png_path)
        return img.size
    except Exception:
        return None


def get_svg_mtime(svg_path: Path) -> str:
    """Get SVG modification time as formatted string."""
    if not svg_path.exists():
        return ""
    mtime = svg_path.stat().st_mtime
    dt = datetime.fromtimestamp(mtime)
    return dt.strftime("%Y-%m-%d %H:%M:%S")


def round_divide(value: int, scale: float) -> int:
    """Round division: value / scale rounded to nearest integer."""
    if scale == 0:
        return 0
    return round(value / scale)


def check_command(cmd: str, instructions: Dict[str, str]) -> bool:
    """Check if a command is available in PATH."""
    if shutil.which(cmd) is None:
        print(f"Error: {cmd} is not installed or not in PATH")
        print(f"Please install {cmd} to use this script")
        print()
        for key, val in instructions.items():
            print(f"{key}: {val}")
        return False
    return True


def scale_dimension(value: int, scale: float) -> int:
    """Scale a dimension and round to nearest integer."""
    return round(value * scale)


def format_duration(seconds: float) -> str:
    """Format duration in seconds to human-readable string.
    
    Returns:
        String like "450ms", "2.3s", or "1m 23s"
    """
    if seconds < 1.0:
        return f"{int(seconds * 1000)}ms"
    elif seconds < 60:
        return f"{seconds:.1f}s"
    else:
        minutes = int(seconds // 60)
        secs = int(seconds % 60)
        return f"{minutes}m {secs}s"


def run_conversion(
    engine: str,
    width: int,
    height: int,
    input_file: Path,
    output_file: Path,
    dry_run: bool = False,
) -> bool:
    """Run SVG to PNG conversion using rsvg-convert or resvg."""
    if dry_run:
        return True
    
    output_file.parent.mkdir(parents=True, exist_ok=True)

    try:
        if engine == "resvg":
            subprocess.run(
                ["resvg", "--width", str(width), "--height", str(height),
                 str(input_file), str(output_file)],
                check=True,
                capture_output=True,
            )
        else:  # rsvg-convert
            subprocess.run(
                ["rsvg-convert", "--width", str(width), "--height", str(height),
                 "--output", str(output_file), str(input_file)],
                check=True,
                capture_output=True,
            )
        return True
    except subprocess.CalledProcessError as e:
        console.print(f"  [red]Error converting {input_file.name}: {e}[/red]")
        return False


def update_src_list_csv(verbose: bool = False) -> int:
    """Regenerate convert-gfx-list.csv from existing PNGs and SVGs.
    
    Preserves existing modified timestamps for unchanged entries.
    Derives dimensions from existing PNGs (prefers 480x272 base resolution).
    Maintains alphabetical sort order.
    """
    start_time = time.time()
    
    if not SRC_DIR.exists():
        print(f"Error: SVG source directory not found: {SRC_DIR}")
        return 1

    print(f"Scanning SVG files in {SRC_DIR}...")
    print(f"CSV output: {SRC_LIST}")
    print()

    # Load existing CSV to preserve modified dates
    existing_dates: Dict[str, str] = {}
    if SRC_LIST.exists():
        try:
            with open(SRC_LIST, "r") as f:
                reader = csv.DictReader(f, delimiter=";")
                for row in reader:
                    file_name = row.get("file", "").strip()
                    modified = row.get("modified", "").strip()
                    if file_name and modified:
                        existing_dates[file_name] = modified
        except Exception as e:
            print(f"Warning: Could not read existing CSV: {e}")

    # Collect SVG files first
    svg_paths = sorted(SRC_DIR.glob("**/*.svg"))

    # Scan all SVGs with progress bar
    rows = []
    with Progress(
        SpinnerColumn(),
        TextColumn("[progress.description]{task.description}"),
        BarColumn(),
        TaskProgressColumn(),
        transient=False,
    ) as progress:
        task = progress.add_task(
            "Scanning SVGs...",
            total=len(svg_paths),
        )

        for svg_path in svg_paths:
            rel_path = svg_path.relative_to(SRC_DIR)
            file_name = str(rel_path.with_suffix(""))
            png_name = f"{file_name}.png"

            # Try to get dimensions from existing PNGs (prefer 480x272 base resolution)
            width = ""
            height = ""

            png_480 = BASE_DIR / "480x272" / png_name
            png_320 = BASE_DIR / "320x240" / png_name
            png_800 = BASE_DIR / "800x480" / png_name

            dim_480 = get_png_dimensions(png_480)
            if dim_480:
                width, height = dim_480
            else:
                dim_320 = get_png_dimensions(png_320)
                if dim_320:
                    w, h = dim_320
                    width = round_divide(w, 0.8)
                    height = round_divide(h, 0.8)
                else:
                    dim_800 = get_png_dimensions(png_800)
                    if dim_800:
                        w, h = dim_800
                        width = round_divide(w, 1.375)
                        height = round_divide(h, 1.375)

            # Preserve existing modified date, or use SVG mtime for new entries
            if file_name in existing_dates:
                modified = existing_dates[file_name]
            else:
                modified = get_svg_mtime(svg_path)

            rows.append({
                "file": file_name,
                "width": str(width) if width else "",
                "height": str(height) if height else "",
                "modified": modified,
            })

            progress.update(task, advance=1)

    # Write sorted CSV
    try:
        with open(SRC_LIST, "w", newline="") as f:
            writer = csv.DictWriter(f, fieldnames=["file", "width", "height", "modified"],
                                     delimiter=";")
            writer.writeheader()
            writer.writerows(rows)
    except Exception as e:
        print(f"Error writing CSV: {e}")
        return 1

    total_svg = len(rows)
    elapsed = time.time() - start_time
    print()
    print(f"Total SVG files processed: {total_svg}")
    console.print(f"[green]CSV updated: {SRC_LIST}[/green] [cyan]({format_duration(elapsed)})[/cyan]")
    return 0


def validate_svg(verbose: bool = False) -> int:
    """Validate that SVG files match CSV entries."""
    if not SRC_LIST.exists():
        print(f"Error: CSV files list not found: {SRC_LIST}")
        return 1
    if not SRC_DIR.exists():
        print(f"Error: SVG source directory not found: {SRC_DIR}")
        return 1

    print("Validating SVG source files")
    print("---------------------------")
    print()

    has_errors = False

    # Part 0: Check CSV for missing dimensions
    print("Checking CSV for missing dimensions...")
    missing_dimensions = 0
    csv_entries = {}

    try:
        with open(SRC_LIST, "r") as f:
            reader = csv.DictReader(f, delimiter=";")
            for row in reader:
                file_name = row.get("file", "").strip()
                width = row.get("width", "").strip()
                height = row.get("height", "").strip()

                if not file_name:
                    continue

                csv_entries[file_name] = (width, height)

                if not width or not height:
                    console.print(
                        f"  [red]CSV entry missing dimensions: {file_name} "
                        f"(width: '{width}', height: '{height}')[/red]"
                    )
                    has_errors = True
                    missing_dimensions += 1
    except Exception as e:
        console.print(f"  [red]Error reading CSV: {e}[/red]")
        return 1

    if missing_dimensions > 0:
        print(f"  Total entries with missing dimensions: {missing_dimensions}")
    else:
        console.print("  [green]All CSV entries have dimensions[/green]")

    # Part 1: Check CSV entries against SVG directory
    print()
    print("Comparing CSV files list with SVG directory...")
    missing_in_dir = 0
    found_in_dir = 0

    for file_name in sorted(csv_entries.keys()):
        svg_path = SRC_DIR / f"{file_name}.svg"
        if not svg_path.exists():
            console.print(f"  [red]CSV entry missing SVG file: {file_name}.svg[/red]")
            has_errors = True
            missing_in_dir += 1
        else:
            found_in_dir += 1

    print(f"  CSV entries - Found: {found_in_dir}, Missing in SVG dir: {missing_in_dir}")

    # Part 2: Check SVG directory against CSV
    print()
    print("Comparing SVG directory with CSV files list...")

    svg_files = {p.stem for p in SRC_DIR.glob("**/*.svg")}
    csv_files = set(csv_entries.keys())
    missing_in_csv = svg_files - csv_files

    for file_name in sorted(missing_in_csv):
        console.print(f"  [red]SVG file missing in CSV list: {file_name}.svg[/red]")
        has_errors = True

    total_svg = len(svg_files)
    print(f"  SVG files - Total: {total_svg}, Missing in CSV: {len(missing_in_csv)}")

    print()
    if not has_errors and len(missing_in_csv) == 0:
        console.print("[green]SVG source files validation passed: All files match[/green]")
        return 0
    else:
        console.print("[red]SVG source files validation failed[/red]")
        return 1


def validate_png(verbose: bool = False) -> int:
    """Validate that PNG files exist for all CSV entries."""
    if not SRC_LIST.exists():
        print(f"Error: CSV files list not found: {SRC_LIST}")
        return 1
    if not SRC_DIR.exists():
        print(f"Error: SVG source directory not found: {SRC_DIR}")
        return 1

    print("Validating PNG files")
    print("--------------------")

    has_errors = False

    # Read CSV
    csv_entries = {}
    try:
        with open(SRC_LIST, "r") as f:
            reader = csv.DictReader(f, delimiter=";")
            for row in reader:
                file_name = row.get("file", "").strip()
                width = row.get("width", "").strip()
                height = row.get("height", "").strip()
                if file_name and width and height:
                    csv_entries[file_name] = None
    except Exception as e:
        print(f"Error reading CSV: {e}")
        return 1

    for resolution in RESOLUTIONS:
        print()
        print(f"Checking resolution: {resolution}")
        out_dir = BASE_DIR / resolution

        if not out_dir.exists():
            print(f"  Warning: Output directory not found: {out_dir}")
            has_errors = True
            continue

        missing_count = 0
        found_count = 0

        for file_name in csv_entries:
            png_path = out_dir / f"{file_name}.png"
            if not png_path.exists():
                console.print(f"  [red]Missing PNG: {file_name}.png[/red]")
                has_errors = True
                missing_count += 1
            else:
                found_count += 1

        print(f"  Found: {found_count}, Missing: {missing_count}")

    print()
    if not has_errors:
        console.print("[green]PNG validation passed: All PNG files exist[/green]")
        return 0
    else:
        console.print("[red]PNG validation failed: Some PNG files are missing[/red]")
        return 1


def validate_all(verbose: bool = False) -> int:
    """Validate SVG and PNG files with interleaved per-file results."""
    start_time = time.time()
    
    if not SRC_LIST.exists():
        print(f"Error: CSV files list not found: {SRC_LIST}")
        return 1
    if not SRC_DIR.exists():
        print(f"Error: SVG source directory not found: {SRC_DIR}")
        return 1

    print("Validating SVG and PNG files")
    print("----------------------------")
    print()

    has_errors = False

    # Part 0: Check CSV for missing dimensions
    print("Checking CSV for missing dimensions...")
    missing_dimensions = 0
    csv_entries = {}

    try:
        with open(SRC_LIST, "r") as f:
            reader = csv.DictReader(f, delimiter=";")
            for row in reader:
                file_name = row.get("file", "").strip()
                width = row.get("width", "").strip()
                height = row.get("height", "").strip()

                if not file_name:
                    continue

                csv_entries[file_name] = (width, height)

                if not width or not height:
                    console.print(
                        f"  [red]CSV entry missing dimensions: {file_name} "
                        f"(width: '{width}', height: '{height}')[/red]"
                    )
                    has_errors = True
                    missing_dimensions += 1
    except Exception as e:
        console.print(f"  [red]Error reading CSV: {e}[/red]")
        return 1

    if missing_dimensions > 0:
        print(f"  Total entries with missing dimensions: {missing_dimensions}")
    else:
        console.print("  [green]All CSV entries have dimensions[/green]")

    # Part 1: Check SVG/PNG for each file
    print()
    print("Validating each file across SVG and PNG...")
    svg_files = {p.stem for p in SRC_DIR.glob("**/*.svg")}
    csv_files = set(csv_entries.keys())
    all_files = sorted(svg_files | csv_files)

    svg_found = 0
    svg_missing = 0
    svg_unreferenced = 0
    png_missing_by_res = {res: 0 for res in RESOLUTIONS}
    png_found_by_res = {res: 0 for res in RESOLUTIONS}

    with Progress(
        SpinnerColumn(),
        TextColumn("[progress.description]{task.description}"),
        BarColumn(),
        TaskProgressColumn(),
        transient=False,
    ) as progress:
        task = progress.add_task(
            "Validating files...",
            total=len(all_files),
        )

        for file_name in all_files:
            # Check SVG
            svg_path = SRC_DIR / f"{file_name}.svg"
            in_csv = file_name in csv_entries
            svg_exists = svg_path.exists()

            if not svg_exists and in_csv:
                progress.print(f"  [red]SVG missing: {file_name}.svg (in CSV but not on disk)[/red]")
                has_errors = True
                svg_missing += 1
            elif svg_exists and not in_csv:
                progress.print(f"  [red]SVG unreferenced: {file_name}.svg (on disk but not in CSV)[/red]")
                svg_unreferenced += 1
            elif svg_exists and in_csv:
                svg_found += 1

            # Check PNG for each resolution (only if in CSV with dimensions)
            if in_csv:
                width, height = csv_entries[file_name]
                if width and height:
                    all_missing = True
                    for resolution in RESOLUTIONS:
                        out_dir = BASE_DIR / resolution
                        png_path = out_dir / f"{file_name}.png"
                        if png_path.exists():
                            all_missing = False
                            png_found_by_res[resolution] += 1
                        else:
                            png_missing_by_res[resolution] += 1

                    if all_missing:
                        progress.print(f"  [red]PNG missing in all resolutions: {file_name}.png[/red]")
                        has_errors = True

            progress.update(task, advance=1)

    elapsed = time.time() - start_time
    print()
    print(f"SVG validation - Found: {svg_found}, Missing: {svg_missing}, Unreferenced: {svg_unreferenced}")
    for resolution in RESOLUTIONS:
        found = png_found_by_res[resolution]
        missing = png_missing_by_res[resolution]
        total = found + missing
        if total > 0:
            print(f"PNG validation - {resolution}: Found: {found}/{total}, Missing: {missing}")

    print()
    if not has_errors and svg_unreferenced == 0:
        console.print(f"[green]All validations passed[/green] [cyan]({format_duration(elapsed)})[/cyan]")
        return 0
    elif svg_unreferenced > 0 and not has_errors:
        console.print(f"[red]Validation complete: {svg_unreferenced} unreferenced SVG file(s) found[/red] [cyan]({format_duration(elapsed)})[/cyan]")
        return 1
    else:
        console.print(f"[red]Validation failed: See errors above[/red] [cyan]({format_duration(elapsed)})[/cyan]")
        return 1


def find_orphaned_pngs() -> Dict[str, List[str]]:
    """Find PNG files that don't have corresponding SVG sources.
    
    Returns:
        Dictionary mapping resolution -> list of orphaned PNG filenames
    """
    orphaned: Dict[str, List[str]] = {res: [] for res in RESOLUTIONS}
    
    # Get all SVG basenames from source directory
    svg_files = {f.stem for f in SRC_DIR.glob("*.svg")}
    
    # Check each resolution for PNGs without SVG sources
    for resolution in RESOLUTIONS:
        dest_dir = BASE_DIR / resolution
        if not dest_dir.exists():
            continue
            
        for png_file in dest_dir.glob("*.png"):
            basename = png_file.stem
            if basename not in svg_files:
                orphaned[resolution].append(basename)
    
    return orphaned


def cleanup_orphaned_pngs(dry_run: bool = False, verbose: bool = False) -> int:
    """Remove PNG files that don't have corresponding SVG sources.
    
    Args:
        dry_run: If True, only show what would be deleted without actually deleting
        verbose: Show detailed output
    """
    start_time = time.time()
    orphaned = find_orphaned_pngs()
    
    total_orphaned = sum(len(files) for files in orphaned.values())
    
    if total_orphaned == 0:
        console.print("[green]No orphaned PNG files found[/green]")
        return 0
    
    mode_str = "[DRY RUN] " if dry_run else ""
    console.print(f"\n{mode_str}Found {total_orphaned} orphaned PNG file(s):")
    
    deleted_count = 0
    for resolution, files in orphaned.items():
        if not files:
            continue
            
        console.print(f"\n{resolution}: {len(files)} file(s)")
        dest_dir = BASE_DIR / resolution
        
        for filename in sorted(files):
            png_path = dest_dir / f"{filename}.png"
            if verbose or dry_run:
                action = "Would delete" if dry_run else "Deleting"
                console.print(f"  {action}: {filename}.png")
            
            if not dry_run:
                try:
                    png_path.unlink()
                    deleted_count += 1
                except Exception as e:
                    console.print(f"  [red]Error deleting {filename}.png: {e}[/red]")
            else:
                deleted_count += 1
    
    elapsed = time.time() - start_time
    print()
    if dry_run:
        console.print(f"[yellow]Dry run complete: {deleted_count} file(s) would be deleted[/yellow] [cyan]({format_duration(elapsed)})[/cyan]")
    else:
        console.print(f"[green]Cleanup complete: {deleted_count} file(s) deleted[/green] [cyan]({format_duration(elapsed)})[/cyan]")
    
    return 0


def process_resolution(
    resolution: str,
    engine: str,
    update_mode: bool,
    verbose: bool = False,
    dry_run: bool = False,
) -> int:
    """Generate PNGs for a specific resolution."""
    start_time = time.time()
    
    if not SRC_LIST.exists():
        print(f"Error: CSV files list not found: {SRC_LIST}")
        return 1
    if not SRC_DIR.exists():
        print(f"Error: SVG source directory not found: {SRC_DIR}")
        return 1

    scale = RESOLUTIONS.get(resolution)
    if scale is None:
        print(f"Error: Unsupported resolution '{resolution}'")
        print(f"Supported resolutions: {', '.join(RESOLUTIONS.keys())}")
        return 1

    print(f"Generating icons for resolution: {resolution} (scale: {scale}x)")

    out_dir = BASE_DIR / resolution
    if not out_dir.exists():
        print(f"Error: Output directory not found: {out_dir}")
        return 1

    mode_msg = (
        "Converting SVG files to PNG using {} (update mode - only changed files)"
        if update_mode
        else "Converting SVG files to PNG using {}"
    )
    print(mode_msg.format(engine))

    has_errors = False
    processed_count = 0
    skipped_count = 0

    # First pass: read CSV entries and validate
    csv_rows = []
    try:
        with open(SRC_LIST, "r") as f:
            reader = csv.DictReader(f, delimiter=";")
            for row in reader:
                file_name = row.get("file", "").strip()
                width_str = row.get("width", "").strip()
                height_str = row.get("height", "").strip()
                csv_date = row.get("modified", "").strip()

                if not file_name:
                    continue

                if not width_str or not height_str:
                    console.print(
                        f"  [red]Error: Skipping {file_name}.svg - "
                        f"missing dimension(s) (width: '{width_str}', height: '{height_str}')[/red]"
                    )
                    has_errors = True
                    continue

                try:
                    width = int(width_str)
                    height = int(height_str)
                except ValueError:
                    console.print(
                        f"  [red]Error: Invalid dimensions for {file_name}: "
                        f"'{width_str}x{height_str}'[/red]"
                    )
                    has_errors = True
                    continue

                csv_rows.append({
                    "file_name": file_name,
                    "width": width,
                    "height": height,
                    "csv_date": csv_date,
                })
    except Exception as e:
        print(f"Error reading CSV: {e}")
        return 1

    # Second pass: process files with progress bar
    with Progress(
        SpinnerColumn(),
        TextColumn("[progress.description]{task.description}"),
        BarColumn(),
        TaskProgressColumn(),
        TextColumn("[cyan]{task.fields[status]}"),
        transient=False,
    ) as progress:
        task = progress.add_task(
            "Converting...",
            total=len(csv_rows),
            status="",
        )

        for row in csv_rows:
            file_name = row["file_name"]
            width = row["width"]
            height = row["height"]
            csv_date = row["csv_date"]

            svg_path = SRC_DIR / f"{file_name}.svg"
            png_path = out_dir / f"{file_name}.png"
            scaled_width = scale_dimension(width, scale)
            scaled_height = scale_dimension(height, scale)

            should_process = True
            status_msg = ""

            if update_mode:
                if not svg_path.exists():
                    progress.print(
                        f"  [red]Error: SVG file not found: {file_name}.svg[/red]"
                    )
                    has_errors = True
                    progress.update(task, advance=1)
                    continue

                svg_mtime = get_svg_mtime(svg_path)

                if not png_path.exists():
                    status_msg = f"Adding {file_name}"
                    if verbose:
                        logger.debug(f"Adding new: {file_name}.svg (PNG missing for {resolution})")
                    should_process = True
                elif not csv_date:
                    status_msg = f"Adding {file_name} (no date)"
                    if verbose:
                        logger.debug(f"Adding new: {file_name}.svg (no previous date in CSV)")
                    should_process = True
                elif svg_mtime > csv_date:
                    status_msg = f"Updating {file_name}"
                    if verbose:
                        logger.debug(f"Updating: {file_name}.svg (SVG: {svg_mtime}, CSV: {csv_date})")
                    should_process = True
                else:
                    png_mtime = datetime.fromtimestamp(
                        png_path.stat().st_mtime
                    ).strftime("%Y-%m-%d %H:%M:%S")
                    if png_mtime < svg_mtime:
                        status_msg = f"Updating {file_name}"
                        if verbose:
                            logger.debug(f"Updating: {file_name}.svg (PNG older than SVG for {resolution})")
                        should_process = True
                    else:
                        skipped_count += 1
                        should_process = False

            if should_process:
                if run_conversion(engine, scaled_width, scaled_height, svg_path, png_path, dry_run):
                    processed_count += 1
                    status_msg = f"✓ {file_name}"
                    if verbose:
                        action = "Would convert" if dry_run else "Converted"
                        logger.debug(f"{action}: {file_name}.svg")
                else:
                    has_errors = True
                    status_msg = f"✗ {file_name}"
                    if verbose:
                        logger.error(f"Failed to convert: {file_name}.svg")

            progress.update(task, advance=1, status=status_msg)

    elapsed = time.time() - start_time
    print()
    
    mode_str = "[DRY RUN] " if dry_run else ""
    if update_mode:
        console.print(f"[green]{mode_str}Processed: {processed_count}, Skipped: {skipped_count}[/green]")

    if has_errors:
        print()
        console.print("[red]Warning: Some files were skipped due to missing dimensions[/red]")
        console.print("Please run --validate png to see all entries with missing dimensions")

    action = "Icons would be generated" if dry_run else "Icons generated"
    console.print(f"[green]{mode_str}Done! {action} for {resolution}[/green] [cyan]({format_duration(elapsed)})[/cyan]")
    return 0


def update_csv_dates(resolutions: List[str], verbose: bool = False) -> None:
    """Update CSV modification dates for processed files."""
    if not SRC_LIST.exists():
        return

    print()
    print("Updating CSV modification dates...")

    rows = []
    try:
        with open(SRC_LIST, "r") as f:
            reader = csv.DictReader(f, delimiter=";")
            for row in reader:
                file_name = row.get("file", "").strip()
                if file_name:
                    svg_path = SRC_DIR / f"{file_name}.svg"
                    mtime = get_svg_mtime(svg_path)
                    row["modified"] = mtime
                rows.append(row)
    except Exception as e:
        print(f"Error reading CSV: {e}")
        return

    with Progress(
        SpinnerColumn(),
        TextColumn("[progress.description]{task.description}"),
        BarColumn(),
        TaskProgressColumn(),
        transient=False,
    ) as progress:
        task = progress.add_task(
            "Updating CSV...",
            total=len(rows),
        )

        try:
            with open(SRC_LIST, "w", newline="") as f:
                writer = csv.DictWriter(f, fieldnames=["file", "width", "height", "modified"],
                                         delimiter=";")
                writer.writeheader()
                for row in rows:
                    writer.writerow(row)
                    progress.update(task, advance=1)
            print("CSV modification dates updated.")
        except Exception as e:
            print(f"Error writing CSV: {e}")


def show_help() -> None:
    """Display usage help."""
    help_text = """Converts source SVG files to PNG icons for different screen resolutions

Usage: convert-gfx.py --validate [svg|png|all]
       convert-gfx.py --make [320x240|480x272|800x480|all] [additional resolutions...] [--update] [--resvg] [--dry-run]
       convert-gfx.py --update-list
       convert-gfx.py --cleanup [--dry-run]
       convert-gfx.py --help

Options:
  --update          Only regenerate PNGs for SVGs newer than CSV date (only with --make)
  --resvg           Use resvg engine instead of rsvg-convert (default, only with --make)
  --dry-run         Show what would be done without making changes (with --make or --cleanup)
  --update-list     Regenerate convert-gfx-list.csv (file;width;height;modified) from existing PNGs/SVG dates
  --cleanup         Remove PNG files that don't have corresponding SVG sources
  --verbose, -v     Show detailed per-file logs while processing

Examples:
  convert-gfx.py                                  Displays this help message
  convert-gfx.py --validate svg                   Validates SVG source files
  convert-gfx.py --validate svg --verbose         Validates SVG source files with detailed output
  convert-gfx.py --validate png                   Validates PNG files
  convert-gfx.py --validate all                   Validates both SVG and PNG files
  convert-gfx.py --make 320x240                   Generates 320x240 PNGs
  convert-gfx.py --make 320x240 480x272           Generates 320x240 and 480x272 PNGs
  convert-gfx.py --make all                       Generates all resolutions PNGs
  convert-gfx.py --make 320x240 --update          Updates only changed SVGs for 320x240
  convert-gfx.py --make 320x240 --update -v       Updates only changed SVGs with detailed output
  convert-gfx.py --make 320x240 --dry-run         Shows what would be generated without creating files
  convert-gfx.py --make 320x240 480x272 --update  Updates only changed SVGs for 320x240 and 480x272
  convert-gfx.py --make all --update              Updates only changed SVGs for all resolutions
  convert-gfx.py --make 320x240 --resvg           Generates 320x240 PNGs (using resvg)
  convert-gfx.py --make 320x240 --update --resvg  Updates only changed SVGs for 320x240 (using resvg)
  convert-gfx.py --update-list                    Regenerates convert-gfx-list.csv with base dimensions and dates
  convert-gfx.py --update-list --verbose          Regenerates CSV with detailed output
  convert-gfx.py --cleanup                        Removes orphaned PNG files
  convert-gfx.py --cleanup --dry-run              Shows what would be deleted without deleting
  convert-gfx.py --cleanup --verbose              Removes orphaned PNGs with detailed output

Requires: rsvg-convert (default) or resvg command line tool
"""
    print(help_text)


def main() -> int:
    """Main entry point."""
    if len(sys.argv) == 1:
        show_help()
        return 0

    # Parse arguments
    args = sys.argv[1:]
    cmd = args[0]

    # Check for verbose flag globally
    verbose = "--verbose" in args or "-v" in args
    dry_run = "--dry-run" in args

    # Set up logging based on verbose flag
    if verbose:
        logging.basicConfig(
            level=logging.DEBUG,
            format="%(message)s",
            handlers=[RichHandler(rich_tracebacks=True)]
        )
    else:
        # Suppress logging when not verbose
        logger.setLevel(logging.CRITICAL)

    if cmd == "--help":
        show_help()
        return 0

    if cmd == "--validate":
        if len(args) < 2:
            print("Error: --validate requires an argument (svg, png, or all)")
            return 1
        mode = args[1]
        if mode == "svg":
            return validate_svg(verbose)
        elif mode == "png":
            return validate_png(verbose)
        elif mode == "all":
            return validate_all(verbose)
        else:
            print("Error: --validate argument must be 'svg', 'png', or 'all'")
            return 1

    if cmd == "--update-list":
        return update_src_list_csv(verbose)

    if cmd == "--cleanup":
        return cleanup_orphaned_pngs(dry_run, verbose)

    if cmd == "--make":
        if len(args) < 2:
            print("Error: --make requires at least one resolution argument")
            return 1

        resolutions = []
        engine = "rsvg-convert"
        update_mode = False
        idx = 1

        # Parse resolutions
        while idx < len(args) and args[idx] not in ("--update", "--resvg", "--verbose", "-v", "--dry-run"):
            if args[idx] == "all":
                resolutions = list(RESOLUTIONS.keys())
            else:
                resolutions.append(args[idx])
            idx += 1

        # Parse flags
        while idx < len(args):
            if args[idx] == "--update":
                update_mode = True
            elif args[idx] == "--resvg":
                engine = "resvg"
            elif args[idx] in ("--verbose", "-v", "--dry-run"):
                pass  # Already handled above
            else:
                print(f"Error: Invalid parameter '{args[idx]}'")
                return 1
            idx += 1

        if not resolutions:
            print("Error: No resolutions specified")
            return 1

        # Check for conversion engine
        engine_cmds = {
            "resvg": {"Ubuntu/Debian": "sudo apt-get install resvg",
                      "macOS": "brew install resvg",
                      "Windows": "Download from https://github.com/RazrFalcon/resvg/releases"},
            "rsvg-convert": {"Ubuntu/Debian": "sudo apt-get install librsvg2-bin",
                            "macOS": "brew install librsvg",
                            "Windows (MSYS2)": "pacman -S mingw-w64-x86_64-librsvg"},
        }

        if not check_command(engine, engine_cmds[engine]):
            return 1

        # Process each resolution
        for resolution in resolutions:
            if process_resolution(resolution, engine, update_mode, verbose, dry_run) != 0:
                return 1

        # Update CSV dates if in update mode
        if update_mode:
            update_csv_dates(resolutions, verbose)

        print("All done!")
        return 0

    print(f"Error: Unknown command '{cmd}'")
    print("Use convert-gfx.py --help for usage information")
    return 1


if __name__ == "__main__":
    sys.exit(main())
