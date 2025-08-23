#!/usr/bin/env python3
"""
Program to check if #define names are used anywhere in C++ code files.
Use this to verify that defines are safe to remove.
"""

import re
import argparse
import sys
from pathlib import Path
from collections import defaultdict

def load_names_to_check(names_file):
    """Load the list of names to check from the specified file."""
    try:
        with open(names_file, 'r', encoding='utf-8') as f:
            names = [line.strip() for line in f if line.strip()]
        return names
    except FileNotFoundError:
        print(f"Error: Names file '{names_file}' not found.")
        sys.exit(1)
    except Exception as e:
        print(f"Error reading names file: {e}")
        sys.exit(1)

def check_usage_in_file(file_path, names_to_check, exclude_defines=True):
    """Check if any of the names are used in the given file."""
    try:
        with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
            content = f.read()
    except Exception as e:
        print(f"Error reading file '{file_path}': {e}")
        return {}
    
    lines = content.splitlines()
    usage_found = defaultdict(list)
    
    # Pattern to match #define statements (to potentially exclude them)
    define_pattern = re.compile(r'^\s*#define\s+(\w+)')
    
    # Pre-compile regex patterns for better performance
    name_patterns = {name: re.compile(r'\b' + re.escape(name) + r'\b') for name in names_to_check}
    
    for line_num, line in enumerate(lines, 1):
        # Skip #define lines if exclude_defines is True
        if exclude_defines and define_pattern.match(line):
            continue
            
        # Skip comments (basic comment detection)
        # Remove single-line comments
        if '//' in line:
            line = line.split('//')[0]
        
        # Skip lines that are mostly comments (basic /* */ detection)
        if line.strip().startswith('/*') or line.strip().startswith('*'):
            continue
            
        # Check each name using pre-compiled patterns
        for name, pattern in name_patterns.items():
            if pattern.search(line):
                usage_found[name].append({
                    'line_num': line_num,
                    'line': line.strip(),
                    'file': str(file_path)
                })
    
    return usage_found

def find_cpp_files(directories, extensions=None):
    """Find all C++ files in the given directories."""
    if extensions is None:
        extensions = {'.cpp', '.cxx', '.cc', '.c', '.hpp', '.hxx', '.h', '.hh'}
    
    cpp_files = []
    for directory in directories:
        dir_path = Path(directory)
        if dir_path.is_file():
            if dir_path.suffix.lower() in extensions:
                cpp_files.append(dir_path)
        elif dir_path.is_dir():
            for ext in extensions:
                cpp_files.extend(dir_path.rglob(f'*{ext}'))
        else:
            print(f"Warning: '{directory}' is not a valid file or directory")
    
    return cpp_files

def main():
    parser = argparse.ArgumentParser(
        description="Check if #define names are used anywhere in C++ code"
    )
    parser.add_argument(
        "names_file",
        help="File containing names to check (one name per line)"
    )
    parser.add_argument(
        "paths",
        nargs="+",
        help="Files or directories to search (directories are searched recursively)"
    )
    parser.add_argument(
        "--include-defines",
        action="store_true",
        help="Include #define statements in the search (default: exclude them)"
    )
    parser.add_argument(
        "--extensions",
        default=".cpp,.cxx,.cc,.c,.hpp,.hxx,.h,.hh",
        help="Comma-separated list of file extensions to search (default: .cpp,.cxx,.cc,.c,.hpp,.hxx,.h,.hh)"
    )
    parser.add_argument(
        "--summary-only",
        action="store_true",
        help="Show only summary of used/unused names"
    )
    
    args = parser.parse_args()
    
    # Parse extensions
    extensions = {ext.strip() for ext in args.extensions.split(',') if ext.strip()}
    
    # Load names to check
    names_to_check = load_names_to_check(args.names_file)
    print(f"Checking usage of {len(names_to_check)} names")
    
    # Find C++ files
    cpp_files = find_cpp_files(args.paths, extensions)
    print(f"Found {len(cpp_files)} C++ files to search")
    
    if not cpp_files:
        print("No C++ files found to search!")
        sys.exit(1)
    
    # Check usage in all files
    all_usage = defaultdict(list)
    files_processed = 0
    
    for cpp_file in cpp_files:
        usage = check_usage_in_file(cpp_file, names_to_check, exclude_defines=not args.include_defines)
        files_processed += 1
        
        for name, occurrences in usage.items():
            all_usage[name].extend(occurrences)
        
        # Progress indicator for large codebases
        if files_processed % 100 == 0:
            print(f"Processed {files_processed} files...")
    
    # Analyze results
    used_names = set()
    unused_names = set()
    
    for name in names_to_check:
        if name in all_usage:
            used_names.add(name)
        else:
            unused_names.add(name)
    
    # Display results
    print(f"\n{'='*60}")
    print("USAGE ANALYSIS RESULTS")
    print(f"{'='*60}")
    
    if used_names:
        print(f"\n❌ USED NAMES ({len(used_names)}) - DO NOT REMOVE:")
        for name in sorted(used_names):
            print(f"  • {name}")
            if not args.summary_only:
                print(f"    Used in {len(set(occ['file'] for occ in all_usage[name]))} file(s):")
                for occurrence in all_usage[name][:5]:  # Show first 5 occurrences
                    print(f"      {occurrence['file']}:{occurrence['line_num']}: {occurrence['line']}")
                if len(all_usage[name]) > 5:
                    print(f"      ... and {len(all_usage[name]) - 5} more occurrences")
                print()
    
    if unused_names:
        print(f"\n✅ UNUSED NAMES ({len(unused_names)}) - SAFE TO REMOVE:")
        for name in sorted(unused_names):
            print(f"  • {name}")
    
    # Summary
    print(f"\n{'='*60}")
    print("SUMMARY:")
    print(f"  Total names checked: {len(names_to_check)}")
    print(f"  Names in use: {len(used_names)}")
    print(f"  Names unused: {len(unused_names)}")
    print(f"  Files searched: {files_processed}")
    
    if used_names:
        print(f"\n⚠️  WARNING: {len(used_names)} names are still in use!")
        print("   Review the usage before removing these #define statements.")
    else:
        print(f"\n✅ All {len(names_to_check)} names appear to be unused and safe to remove.")

if __name__ == "__main__":
    main()
