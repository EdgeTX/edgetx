from json import JSONDecodeError
from pathlib import Path
from typing import List, Tuple
from typing_extensions import Type

from pydantic import BaseModel, ValidationError


def validate_json_files(
    directory: str, model_class: Type[BaseModel]
) -> Tuple[List[str], List[Tuple[str, str]]]:
    """
    Validate all JSON files in a directory against a Pydantic model.

    Args:
        directory: Path to the directory containing JSON files
        model_class: Pydantic model class to validate against

    Returns:
        Tuple of (valid_files, invalid_files_with_errors)
    """
    directory_path = Path(directory)

    if not directory_path.exists():
        raise FileNotFoundError(f"Directory {directory} does not exist")

    valid_files = []
    invalid_files = []

    # Find all JSON files in the directory
    json_files = list(directory_path.glob("*.json"))

    if not json_files:
        print(f"No JSON files found in {directory}")
        return valid_files, invalid_files

    print(f"Found {len(json_files)} JSON files to validate")

    for json_file in json_files:
        try:
            # Read the JSON file
            with open(json_file, "r", encoding="utf-8") as f:
                json_data = f.read()

            # Validate against given model
            model_class.model_validate_json(json_data)

            valid_files.append(str(json_file))
            print(f"✓ {json_file.name} - Valid")

        except ValidationError as e:
            error_msg = str(e)
            invalid_files.append((str(json_file), error_msg))
            print(f"✗ {json_file.name} - Validation Error:")
            print(f"  {error_msg}")

        except JSONDecodeError as e:
            error_msg = f"Invalid JSON syntax: {e}"
            invalid_files.append((str(json_file), error_msg))
            print(f"✗ {json_file.name} - JSON Decode Error:")
            print(f"  {error_msg}")

        except Exception as e:
            error_msg = f"Unexpected error: {e}"
            invalid_files.append((str(json_file), error_msg))
            print(f"✗ {json_file.name} - Unexpected Error:")
            print(f"  {error_msg}")

    return valid_files, invalid_files


def main():
    """Main function to run the validation"""
    import argparse

    parser = argparse.ArgumentParser(
        description="Validate JSON files against HardwareDefinition model"
    )
    parser.add_argument(
        "-v", "--verbose", action="store_true", help="Show detailed error messages"
    )
    parser.add_argument(
        "directory", help="Path to directory containing JSON files to validate"
    )

    args = parser.parse_args()

    try:
        from models import HardwareDefinition

        valid_files, invalid_files = validate_json_files(
            args.directory, HardwareDefinition
        )

        print("\n=== Validation Summary ===")
        print(f"Total files processed: {len(valid_files) + len(invalid_files)}")
        print(f"Valid files: {len(valid_files)}")
        print(f"Invalid files: {len(invalid_files)}")

        if invalid_files:
            print("\nInvalid files:")
            for file_path, error in invalid_files:
                if args.verbose:
                    print(f"  - {Path(file_path).name}: {error}")
                else:
                    print(f"  - {Path(file_path).name}")

        return 0 if not invalid_files else 1

    except Exception as e:
        print(f"Error during validation: {e}")
        return 1


if __name__ == "__main__":
    exit(main())
