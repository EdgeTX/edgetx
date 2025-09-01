#!/usr/bin/env python3
"""
EdgeTX JSON validation script

This script validates JSON files with optional schema validation:
1. Generic JSON syntax validation for any JSON file
2. EdgeTX fw.json schema validation (targets array, changelog string)
3. EdgeTX fw.json alphabetical order validation

Usage:
    python3 tools/validate-json.py [options] [file.json]
    
Options:
    --fw-schema, -f      Validate against EdgeTX fw.json schema
    --syntax-only, -s    Only validate JSON syntax (and schema if --fw-schema)
    --order-only, -o     Only validate alphabetical order (requires --fw-schema)
    --help, -h           Show this help message
    
Auto-detection:
    - If filename is 'fw.json', automatically applies --fw-schema
    - If no file specified and 'fw.json' exists, uses it with --fw-schema
    
Examples:
    python3 validate-json.py data.json                    # Generic JSON validation
    python3 validate-json.py fw.json                      # Auto-detects fw.json schema
    python3 validate-json.py --fw-schema config.json      # Force fw.json schema on other file
    python3 validate-json.py --syntax-only fw.json        # Only syntax + fw schema
    python3 validate-json.py --order-only fw.json         # Only order validation

Exit codes:
    0: All validations passed
    1: Validation failed
    2: File not found or other error
"""

import json
import sys
import os
import argparse
from pathlib import Path


def validate_json_syntax(file_path, show_message=True):
    """Validate JSON syntax and load the data."""
    if show_message:
        print(f'🔍 Validating JSON syntax: {file_path}')
    
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            data = json.load(f)
        if show_message:
            print('✅ JSON syntax is valid')
        return data
    except json.JSONDecodeError as e:
        print(f'❌ JSON syntax error: {e}')
        return None
    except FileNotFoundError:
        print(f'❌ File not found: {file_path}')
        return None
    except Exception as e:
        print(f'❌ Error reading file: {e}')
        return None


def validate_fw_schema(data):
    """Validate the EdgeTX fw.json schema structure."""
    print('🔍 Validating EdgeTX fw.json schema structure...')
    
    # Check required fields
    if 'targets' not in data:
        print('❌ Missing required field: "targets"')
        return False
    if 'changelog' not in data:
        print('❌ Missing required field: "changelog"')
        return False
    
    # Check field types
    if not isinstance(data['targets'], list):
        print('❌ Field "targets" must be an array')
        return False
    if not isinstance(data['changelog'], str):
        print('❌ Field "changelog" must be a string')
        return False
    
    # Validate each target structure
    for i, target in enumerate(data['targets']):
        target_num = i + 1
        if not isinstance(target, list):
            print(f'❌ Target {target_num} must be an array')
            return False
        if len(target) != 2:
            print(f'❌ Target {target_num} must have exactly 2 elements, found {len(target)}')
            return False
        if not isinstance(target[0], str):
            print(f'❌ Target {target_num} first element (name) must be a string')
            return False
        if not isinstance(target[1], str):
            print(f'❌ Target {target_num} second element (prefix) must be a string')
            return False
        
        # Check for empty strings
        if not target[0].strip():
            print(f'❌ Target {target_num} name cannot be empty')
            return False
        if not target[1].strip():
            print(f'❌ Target {target_num} prefix cannot be empty')
            return False
    
    # Check for extra fields
    allowed_fields = {'targets', 'changelog'}
    extra_fields = set(data.keys()) - allowed_fields
    if extra_fields:
        print(f'❌ Unexpected fields found: {sorted(extra_fields)}')
        return False
    
    print(f'✅ EdgeTX fw.json schema validation passed - found {len(data["targets"])} targets')
    return True


def validate_fw_alphabetical_order(data):
    """Validate that fw.json targets are in case-insensitive alphabetical order."""
    print('🔍 Validating fw.json targets alphabetical order (case-insensitive)...')
    
    # Extract target names (first element of each target pair)
    target_names = [target[0] for target in data['targets']]
    
    # Sort case-insensitively
    sorted_names = sorted(target_names, key=str.lower)
    
    # Check if they're in order
    if target_names != sorted_names:
        print('❌ ERROR: Targets are not in alphabetical order (case-insensitive)!')
        print()
        print('Current order:')
        for i, name in enumerate(target_names):
            print(f'  {i+1:2d}. {name}')
        print()
        print('Expected alphabetical order (case-insensitive):')
        for i, name in enumerate(sorted_names):
            print(f'  {i+1:2d}. {name}')
        print()
        
        # Show misplaced targets
        misplaced = []
        for i, (current, expected) in enumerate(zip(target_names, sorted_names)):
            if current != expected:
                misplaced.append(f'Position {i+1}: found "{current}", expected "{expected}"')
        
        if misplaced:
            print('Misplaced targets:')
            for item in misplaced[:10]:  # Show first 10
                print(f'  - {item}')
            if len(misplaced) > 10:
                print(f'  - ... and {len(misplaced) - 10} more')
        
        return False
    else:
        print('✅ All targets are in alphabetical order (case-insensitive)!')
        print(f'Found {len(target_names)} targets, all properly sorted.')
        return True


def find_json_file(filename=None):
    """Find JSON file in current directory or repository root."""
    if filename:
        return filename
    
    # Default to fw.json for backwards compatibility
    # Check current directory first
    if os.path.exists('fw.json'):
        return 'fw.json'
    elif os.path.exists('../fw.json'):
        return '../fw.json'
    else:
        # Try to find repository root
        current_dir = Path.cwd()
        for parent in [current_dir] + list(current_dir.parents):
            fw_json = parent / 'fw.json'
            if fw_json.exists():
                return str(fw_json)
        return None


def is_fw_json_file(file_path):
    """Check if the file is fw.json based on filename."""
    return os.path.basename(file_path).lower() == 'fw.json'


def main():
    """Main validation function."""
    parser = argparse.ArgumentParser(
        description='EdgeTX JSON validator',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
    python3 validate-json.py data.json                    # Generic JSON validation
    python3 validate-json.py fw.json                      # Auto-detects fw.json schema
    python3 validate-json.py --fw-schema config.json      # Force fw.json schema validation
    python3 validate-json.py --syntax-only fw.json        # Only syntax + fw schema
    python3 validate-json.py --order-only fw.json         # Only order validation
        """
    )
    
    parser.add_argument('file', nargs='?', help='Path to JSON file (default: auto-detect fw.json)')
    parser.add_argument('-f', '--fw-schema', action='store_true', 
                       help='Validate against EdgeTX fw.json schema')
    parser.add_argument('-s', '--syntax-only', action='store_true', 
                       help='Only validate JSON syntax (and schema if --fw-schema)')
    parser.add_argument('-o', '--order-only', action='store_true',
                       help='Only validate alphabetical order (requires --fw-schema)')
    
    args = parser.parse_args()
    
    # Validate argument combinations
    if args.order_only and not args.fw_schema:
        # Auto-enable fw-schema if doing order validation and file looks like fw.json
        file_to_check = args.file or find_json_file()
        if file_to_check and is_fw_json_file(file_to_check):
            args.fw_schema = True
        else:
            print('❌ --order-only requires --fw-schema or fw.json file')
            sys.exit(2)
    
    # Determine file path
    file_path = find_json_file(args.file)
    if not file_path:
        if args.file:
            file_path = args.file  # Use provided file even if not found (will error later)
        else:
            print('❌ No JSON file found. Please specify the file path.')
            sys.exit(2)
    
    # Auto-detect fw.json schema
    if not args.fw_schema and is_fw_json_file(file_path):
        args.fw_schema = True
    
    # Determine what to validate
    check_syntax = not args.order_only  # Always check syntax unless order-only
    check_schema = args.fw_schema and not args.order_only  # Check schema if fw-schema and not order-only
    check_order = args.fw_schema and not args.syntax_only  # Check order if fw-schema and not syntax-only
    
    print(f'EdgeTX JSON Validator')
    print(f'====================')
    print(f'File: {file_path}')
    
    if args.fw_schema:
        if args.syntax_only:
            print('Mode: EdgeTX fw.json syntax and schema validation only')
        elif args.order_only:
            print('Mode: EdgeTX fw.json alphabetical order validation only')
        else:
            print('Mode: Full EdgeTX fw.json validation (syntax + schema + order)')
    else:
        print('Mode: Generic JSON syntax validation')
    
    print()
    
    # Load JSON data (required for any validation)
    if args.order_only:
        # For order-only mode, silently load the JSON (we still need to parse it)
        print('🔍 Loading JSON for alphabetical order validation...')
        data = validate_json_syntax(file_path, show_message=False)
        if data is None:
            sys.exit(1)
        print('✅ JSON loaded successfully')
    else:
        # For syntax validation, show the full syntax validation
        data = validate_json_syntax(file_path, show_message=True)
        if data is None:
            sys.exit(1)
    
    validation_passed = True
    
    if check_schema:
        print()
        # Validate EdgeTX fw.json schema structure
        if not validate_fw_schema(data):
            validation_passed = False
            if args.syntax_only:  # If syntax-only mode, exit immediately on failure
                sys.exit(1)
            # Don't continue to order validation if schema failed
            check_order = False
    
    if check_order:
        if not args.order_only:  # Only add spacing if we're not in order-only mode
            print()
        # Validate alphabetical order
        if not validate_fw_alphabetical_order(data):
            validation_passed = False
    
    if not validation_passed:
        sys.exit(1)
    
    print()
    if args.fw_schema:
        if args.syntax_only:
            print('🎉 EdgeTX fw.json syntax and schema validation passed!')
        elif args.order_only:
            print('🎉 EdgeTX fw.json alphabetical order validation passed!')
        else:
            print('🎉 All EdgeTX fw.json validations passed!')
    else:
        print('🎉 JSON syntax validation passed!')


if __name__ == '__main__':
    main()
