#!/usr/bin/env python3
"""
Program to remove #define statements from C/C++ files based on a list of names.
"""

import re
import argparse
import sys
from pathlib import Path

def load_names_to_remove(names_file):
    """Load the list of names to remove from the specified file."""
    try:
        with open(names_file, 'r', encoding='utf-8') as f:
            names = [line.strip() for line in f if line.strip()]
        return set(names)  # Use set for faster lookup
    except FileNotFoundError:
        print(f"Error: Names file '{names_file}' not found.")
        sys.exit(1)
    except Exception as e:
        print(f"Error reading names file: {e}")
        sys.exit(1)

def remove_defines_from_file(file_path, names_to_remove, dry_run=False):
    """Remove #define statements from a file based on the names list."""
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            lines = f.readlines()
    except Exception as e:
        print(f"Error reading file '{file_path}': {e}")
        return False
    
    modified_lines = []
    removed_count = 0
    i = 0
    
    # Pattern to match #define statements
    # Matches: #define NAME or #define NAME value
    define_pattern = re.compile(r'^\s*#define\s+(\w+)')
    
    while i < len(lines):
        line = lines[i]
        line_num = i + 1
        
        match = define_pattern.match(line)
        if match:
            define_name = match.group(1)
            if define_name in names_to_remove:
                # Found a #define to remove, collect all continuation lines
                lines_to_remove = []
                current_line = line
                current_line_num = line_num
                
                # Keep collecting lines while they end with backslash
                while current_line.rstrip().endswith('\\'):
                    lines_to_remove.append((current_line_num, current_line))
                    i += 1
                    if i < len(lines):
                        current_line = lines[i]
                        current_line_num = i + 1
                    else:
                        break
                
                # Add the final line (without backslash or last line of file)
                lines_to_remove.append((current_line_num, current_line))
                
                # Print what we're removing
                if not dry_run:
                    print(f"Removing multi-line #define from {file_path}:")
                    for ln, l in lines_to_remove:
                        print(f"  Line {ln}: {l.rstrip()}")
                else:
                    print(f"Would remove multi-line #define from {file_path}:")
                    for ln, l in lines_to_remove:
                        print(f"  Line {ln}: {l.rstrip()}")
                
                removed_count += 1
                i += 1  # Move to next line after the #define block
                continue
        
        # Line is not a #define to remove, keep it
        modified_lines.append(line)
        i += 1
    
    # Write back to file if not dry run and changes were made
    if not dry_run and removed_count > 0:
        try:
            with open(file_path, 'w', encoding='utf-8') as f:
                f.writelines(modified_lines)
            print(f"Successfully processed '{file_path}' - removed {removed_count} #define(s)")
        except Exception as e:
            print(f"Error writing file '{file_path}': {e}")
            return False
    elif dry_run and removed_count > 0:
        print(f"Would process '{file_path}' - {removed_count} #define(s) to remove")
    elif removed_count == 0:
        print(f"No matching #define statements found in '{file_path}'")
    
    return True

def main():
    parser = argparse.ArgumentParser(
        description="Remove #define statements from C/C++ files based on a list of names"
    )
    parser.add_argument(
        "names_file",
        help="File containing names to remove (one name per line)"
    )
    parser.add_argument(
        "source_files",
        nargs="+",
        help="Source files to process"
    )
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Show what would be removed without actually modifying files"
    )
    
    args = parser.parse_args()
    
    # Load names to remove
    names_to_remove = load_names_to_remove(args.names_file)
    print(f"Loaded {len(names_to_remove)} names to remove")
    
    if args.dry_run:
        print("DRY RUN MODE - No files will be modified")
    
    # Process each source file
    success_count = 0
    for source_file in args.source_files:
        file_path = Path(source_file)
        if not file_path.exists():
            print(f"Warning: File '{source_file}' not found, skipping")
            continue
        
        if remove_defines_from_file(file_path, names_to_remove, args.dry_run):
            success_count += 1
    
    print(f"\nProcessed {success_count} out of {len(args.source_files)} files successfully")

if __name__ == "__main__":
    main()
