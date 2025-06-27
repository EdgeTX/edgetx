#!/usr/bin/env python3
"""
Translation Checker for EdgeTX Translations

This script can check:
1. Bootloader translations (bl_translations.h) - unified file with multiple languages
2. Individual language translation files (*.h) - separate files per language

It ensures that all translation languages have the same set of translation strings defined.
"""

import re
import sys
import os
from pathlib import Path
from collections import defaultdict
from typing import Dict, Set, List, Tuple, Optional
import argparse
import glob

class TranslationChecker:
    def __init__(self):
        self.bootloader_translations = defaultdict(set)  # language -> set of bootloader translation keys
        self.language_translations = defaultdict(set)    # language -> set of language translation keys
        self.bootloader_keys = set()
        self.language_keys = set()
        self.checked_files = []
        
    def find_translations_directory(self, start_path: str) -> Optional[Path]:
        """Find the translations directory by searching up from the given path."""
        current_path = Path(start_path).resolve()
        
        # If the provided path is already the translations directory
        if current_path.name == "translations" and current_path.is_dir():
            return current_path
            
        # If the provided path is a file in translations directory
        if current_path.parent.name == "translations":
            return current_path.parent
            
        # Search up the directory tree
        while current_path != current_path.parent:
            translations_path = current_path / "radio" / "src" / "translations"
            if translations_path.exists() and translations_path.is_dir():
                return translations_path
            current_path = current_path.parent
        
        return None
    
    def parse_bootloader_file(self, file_path: Path):
        """Parse the bootloader translation file (bl_translations.h)."""
        if not file_path.exists():
            print(f"Warning: File {file_path} does not exist")
            return False
            
        with open(file_path, 'r', encoding='utf-8') as f:
            content = f.read()
        
        # Split content into lines for easier processing
        lines = content.split('\n')
        current_language = None
        in_translation_block = False
        conditional_depth = 0
        
        for i, line in enumerate(lines):
            line = line.strip()
            
            # Skip empty lines and comments
            if not line or line.startswith('//'):
                continue
            
            # Check for translation language blocks
            translation_match = re.match(r'#(?:if|elif)\s+defined\(TRANSLATIONS_([A-Z]+)\)', line)
            if translation_match:
                current_language = translation_match.group(1)
                in_translation_block = True
                conditional_depth = 0
                continue
                
            # Check for else block (default/English)
            if re.match(r'#else', line) and in_translation_block and conditional_depth == 0:
                current_language = "EN"  # Default language
                continue
                
            # Track conditional compilation depth
            if re.match(r'#if', line) and in_translation_block:
                conditional_depth += 1
                continue
            elif re.match(r'#endif', line) and in_translation_block:
                if conditional_depth > 0:
                    conditional_depth -= 1
                else:
                    # This is the end of the translation block
                    in_translation_block = False
                    current_language = None
                continue
            
            # Skip non-translation lines
            if not in_translation_block or not current_language:
                continue
                
            # Parse #define statements
            define_match = re.match(r'#define\s+(TR_BL_\w+)', line)
            if define_match:
                key = define_match.group(1)
                self.bootloader_translations[current_language].add(key)
                self.bootloader_keys.add(key)
        
        self.checked_files.append(str(file_path))
        return True
    
    def parse_language_file(self, file_path: Path) -> Optional[str]:
        """Parse an individual language translation file (e.g., en.h, fr.h)."""
        if not file_path.exists():
            return None
            
        # Extract language code from filename
        language = file_path.stem.upper()
        
        with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
            content = f.read()
        
        # Find all #define TR_ statements
        define_matches = re.findall(r'#define\s+(TR_\w+)', content)
        
        for key in define_matches:
            self.language_translations[language].add(key)
            self.language_keys.add(key)
        
        self.checked_files.append(str(file_path))
        return language
    
    def check_bootloader_translations(self, translations_dir: Path) -> bool:
        """Check bootloader translations."""
        bl_file = translations_dir / "bl_translations.h"
        return self.parse_bootloader_file(bl_file)
    
    def check_language_translations(self, translations_dir: Path) -> List[str]:
        """Check individual language translation files."""
        languages_found = []
        
        # Look for .h files that are language files (excluding special files)
        exclude_files = {"bl_translations.h", "untranslated.h"}
        
        for h_file in translations_dir.glob("*.h"):
            if h_file.name in exclude_files:
                continue
                
            # Skip files that are clearly not language files
            if h_file.name.startswith("tts_"):
                continue
                
            language = self.parse_language_file(h_file)
            if language:
                languages_found.append(language)
        
        return languages_found
    
    def analyze(self) -> Dict[str, any]:
        """Analyze translations and return results."""
        bootloader_languages = list(self.bootloader_translations.keys())
        language_languages = list(self.language_translations.keys())
        
        results = {
            "bootloader": {
                "languages": bootloader_languages,
                "total_keys": len(self.bootloader_keys),
                "missing_keys": defaultdict(set),
                "extra_keys": defaultdict(set),
                "summary": {}
            },
            "language_files": {
                "languages": language_languages,
                "total_keys": len(self.language_keys),
                "missing_keys": defaultdict(set),
                "extra_keys": defaultdict(set),
                "summary": {}
            },
            "checked_files": self.checked_files
        }
        
        # Analyze bootloader translations
        for lang in bootloader_languages:
            lang_keys = self.bootloader_translations[lang]
            results["bootloader"]["missing_keys"][lang] = self.bootloader_keys - lang_keys
            results["bootloader"]["extra_keys"][lang] = lang_keys - self.bootloader_keys
            results["bootloader"]["summary"][lang] = {
                "total": len(lang_keys),
                "missing": len(results["bootloader"]["missing_keys"][lang]),
                "extra": len(results["bootloader"]["extra_keys"][lang])
            }
        
        # Analyze language file translations
        for lang in language_languages:
            lang_keys = self.language_translations[lang]
            results["language_files"]["missing_keys"][lang] = self.language_keys - lang_keys
            results["language_files"]["extra_keys"][lang] = lang_keys - self.language_keys
            results["language_files"]["summary"][lang] = {
                "total": len(lang_keys),
                "missing": len(results["language_files"]["missing_keys"][lang]),
                "extra": len(results["language_files"]["extra_keys"][lang])
            }
        
        return results
    
    def print_report(self, verbose=False):
        """Print a detailed report of the translation analysis."""
        results = self.analyze()
        
        has_bootloader = bool(results["bootloader"]["languages"])
        has_language_files = bool(results["language_files"]["languages"])
        
        if not has_bootloader and not has_language_files:
            print("No translation files found or processed.")
            return
        
        print("EdgeTX Translation Analysis")
        print("=" * 50)
        
        if verbose:
            print(f"Checked files: {len(results['checked_files'])}")
            for file_path in results['checked_files']:
                print(f"  - {file_path}")
            print()
        
        # Report bootloader translations
        if has_bootloader:
            self._print_section_report("Bootloader Translations (bl_translations.h)", 
                                     results["bootloader"], self.bootloader_keys, verbose)
        
        # Report language file translations  
        if has_language_files:
            self._print_section_report("Language File Translations (*.h)", 
                                     results["language_files"], self.language_keys, verbose)
    
    def _print_section_report(self, title: str, section_results: Dict, all_keys: Set, verbose: bool):
        """Print report for a specific section (bootloader or language files)."""
        print(f"\n{title}")
        print("-" * len(title))
        print(f"Total unique translation keys: {section_results['total_keys']}")
        print(f"Languages found: {', '.join(sorted(section_results['languages']))}")
        print()
        
        # Summary table
        print("Summary by Language:")
        print("-" * 60)
        print(f"{'Language':<12} {'Total':<8} {'Missing':<10} {'Extra':<8}")
        print("-" * 60)
        
        for lang in sorted(section_results['languages']):
            summary = section_results['summary'][lang]
            print(f"{lang:<12} {summary['total']:<8} {summary['missing']:<10} {summary['extra']:<8}")
        
        print()
        
        # Detailed missing keys report
        has_issues = False
        for lang in sorted(section_results['languages']):
            missing = section_results['missing_keys'][lang]
            extra = section_results['extra_keys'][lang]
            
            if missing or extra:
                has_issues = True
                print(f"Issues for {lang}:")
                
                if missing:
                    print(f"  Missing keys ({len(missing)}):")
                    for key in sorted(missing):
                        print(f"    - {key}")
                
                if extra:
                    print(f"  Extra keys ({len(extra)}):")
                    for key in sorted(extra):
                        print(f"    + {key}")
                print()
        
        if not has_issues:
            print("✅ All languages have consistent translation keys!")
        else:
            print("❌ Translation inconsistencies found!")
            
        # Show all translation keys for reference (only if verbose)
        if verbose:
            print(f"\nAll Translation Keys ({len(all_keys)}):")
            print("-" * 40)
            for key in sorted(all_keys):
                print(f"  {key}")
            print()

def main():
    parser = argparse.ArgumentParser(
        description="Check EdgeTX translation consistency",
        epilog="""
Examples:
  # Check bootloader translations only
  python3 check_translations.py --bootloader
  
  # Check individual language files only  
  python3 check_translations.py --languages
  
  # Check both (default)
  python3 check_translations.py
  
  # Check with verbose output
  python3 check_translations.py -v
  
  # Specify path to translations directory or EdgeTX root
  python3 check_translations.py /path/to/edgetx/radio/src/translations
        """,
        formatter_class=argparse.RawDescriptionHelpFormatter
    )
    parser.add_argument("path", nargs="?", default=".", 
                       help="Path to translations directory, EdgeTX root, or current directory")
    parser.add_argument("-v", "--verbose", action="store_true", 
                       help="Show all translation keys")
    parser.add_argument("--bootloader", action="store_true",
                       help="Check only bootloader translations (bl_translations.h)")
    parser.add_argument("--languages", action="store_true", 
                       help="Check only individual language files (*.h)")
    
    args = parser.parse_args()
    
    checker = TranslationChecker()
    
    # Find translations directory
    translations_dir = checker.find_translations_directory(args.path)
    if not translations_dir:
        print(f"Error: Could not find translations directory from path: {args.path}")
        print("Please specify a path to the EdgeTX repository root or translations directory.")
        sys.exit(1)
    
    print(f"Using translations directory: {translations_dir}")
    print()
    
    # Determine what to check
    check_bootloader = args.bootloader or not args.languages
    check_languages = args.languages or not args.bootloader
    
    if check_bootloader:
        if not checker.check_bootloader_translations(translations_dir):
            print("Warning: Could not process bootloader translations")
    
    if check_languages:
        languages_found = checker.check_language_translations(translations_dir)
        if not languages_found:
            print("Warning: No individual language translation files found")
    
    checker.print_report(verbose=args.verbose)

if __name__ == "__main__":
    main()
