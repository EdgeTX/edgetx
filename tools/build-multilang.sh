#!/bin/bash

# EdgeTX Multi-Language Multi-Variant Build Script
# Written with assistance from GitHub Copilot AI
#
# This script automatically builds all languages for all firmware variants
# with intelligent filtering for character-based languages on B&W screens

# Manual override for color screen targets (set this to override auto-detection)
COLOR_SCREEN_TARGETS_OVERRIDE=""

# Function to build targets by screen type and optional processor filter
build_targets_by_screen_type() {
    local screen_type="$1"  # "color" or "bw"
    local processor_filter="$2"  # "F2", "F4", "H7", "exclude-F2", etc. (optional)
    local all_targets=$(get_all_targets)
    local filtered_targets=""
    local failed_targets=0
    local overall_start_time=$(date +%s)
    
    # Filter by screen type first
    for target in $all_targets; do
        local is_color_target=false
        if is_color_screen_target "$target"; then
            is_color_target=true
        fi
        
        if [[ "$screen_type" == "color" && "$is_color_target" == "true" ]]; then
            filtered_targets="$filtered_targets $target"
        elif [[ "$screen_type" == "bw" && "$is_color_target" == "false" ]]; then
            filtered_targets="$filtered_targets $target"
        fi
    done
    
    # Apply processor filter if specified
    if [[ -n "$processor_filter" ]]; then
        local processor_filtered_targets=""
        
        if [[ "$processor_filter" == exclude-* ]]; then
            # Exclude specific processor
            local exclude_proc="${processor_filter#exclude-}"
            for target in $filtered_targets; do
                local target_processor=$(get_stm32_processor_type "$target")
                if [[ "$target_processor" != "$exclude_proc" && "$target_processor" != "UNKNOWN" ]]; then
                    processor_filtered_targets="$processor_filtered_targets $target"
                fi
            done
        else
            # Include only specific processor
            for target in $filtered_targets; do
                local target_processor=$(get_stm32_processor_type "$target")
                if [[ "$target_processor" == "$processor_filter" ]]; then
                    processor_filtered_targets="$processor_filtered_targets $target"
                fi
            done
        fi
        
        filtered_targets="$processor_filtered_targets"
    fi
    
    # Build the filtered targets
    if [[ -z "$filtered_targets" ]]; then
        echo "No targets match the specified criteria."
        return 0
    fi
    
    echo "Building $screen_type screen targets$(if [[ -n "$processor_filter" ]]; then echo " with processor filter: $processor_filter"; fi)..."
    echo "Selected targets: $filtered_targets"
    echo
    
    for target in $filtered_targets; do
        echo "========================================"
        echo "Processing target: $target ($(get_stm32_processor_type "$target") processor)"
        echo "========================================"
        
        if ! build_target_all_languages "$target"; then
            ((failed_targets++))
            echo "Failed to build all languages for target $target"
        fi
        
        echo
    done
    
    # Calculate overall elapsed time
    local overall_end_time=$(date +%s)
    local overall_elapsed=$((overall_end_time - overall_start_time))
    
    # Format overall elapsed time
    local overall_elapsed_formatted
    if [[ $overall_elapsed -ge 3600 ]]; then
        overall_elapsed_formatted=$(printf "%dh %dm %ds" $((overall_elapsed / 3600)) $(((overall_elapsed % 3600) / 60)) $((overall_elapsed % 60)))
    elif [[ $overall_elapsed -ge 60 ]]; then
        overall_elapsed_formatted=$(printf "%dm %ds" $((overall_elapsed / 60)) $((overall_elapsed % 60)))
    else
        overall_elapsed_formatted="${overall_elapsed}s"
    fi
    
    if [[ $failed_targets -gt 0 && "${CONTINUE_ON_FAILURE:-}" != "1" ]]; then
        echo "========================================="
        echo "BUILD SUMMARY"
        echo "========================================="
        printf "\033[31m✗\033[0m Warning: %d targets had build failures\n" "$failed_targets"
        echo "Total build time: ${overall_elapsed_formatted}"
        return 1
    else
        echo "========================================="
        echo "BUILD SUMMARY"
        echo "========================================="
        if [[ $failed_targets -gt 0 && "${CONTINUE_ON_FAILURE:-}" == "1" ]]; then
            printf "\033[32m✓\033[0m All targets attempted successfully!\n"
            echo "  Note: Some language builds failed but were ignored due to --continue-on-failure"
        else
            printf "\033[32m✓\033[0m All targets built successfully!\n"
        fi
        echo "Total build time: ${overall_elapsed_formatted}"
        return 0
    fi
}
# with intelligent filtering for character-based languages on B&W screens

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

# Source the build-common.sh for target definitions
if [[ -f "$SCRIPT_DIR/tools/build-common.sh" ]]; then
    . "$SCRIPT_DIR/tools/build-common.sh"
elif [[ -f "$SCRIPT_DIR/build-common.sh" ]]; then
    . "$SCRIPT_DIR/build-common.sh"
else
    echo "Error: Could not find build-common.sh"
    exit 1
fi

# Auto-discover available languages from radio/src/CMakeLists.txt
get_radio_languages() {
    local cmake_file=""
    if [[ -f "$SCRIPT_DIR/../radio/src/CMakeLists.txt" ]]; then
        cmake_file="$SCRIPT_DIR/../radio/src/CMakeLists.txt"
    else
        echo "Error: Could not find radio/src/CMakeLists.txt" >&2
        return 1
    fi
    
    # Extract languages from: set(RADIO_LANGUAGES CN CZ DA DE EN ES FI FR HE HU IT JP KO NL PL PT RU SE SK TW UA)
    # Strip carriage returns to handle Windows line endings
    grep "^set(RADIO_LANGUAGES" "$cmake_file" | sed 's/set(RADIO_LANGUAGES \(.*\))/\1/' | tr -d '\r'
}

# Define character-based/logographic languages (configurable with auto-detection fallback)
# These should not be built for B&W screens due to font/display limitations
LOGOGRAPHIC_LANGUAGES_OVERRIDE=""  # Set this to override auto-detection

get_logographic_languages() {
    # Use override if set
    if [[ -n "$LOGOGRAPHIC_LANGUAGES_OVERRIDE" ]]; then
        echo "$LOGOGRAPHIC_LANGUAGES_OVERRIDE"
        return 0
    fi
    
    # Auto-detect logographic languages based on known character sets
    # These are languages that typically use complex character sets that may not display well on B&W screens
    local known_logographic="CN JP KO TW"
    
    # Could be enhanced to check translation files for unicode/multibyte characters
    # For now, use the known list as fallback
    echo "$known_logographic"
}

# Extract all firmware variants from build-common.sh
get_all_targets() {
    # Extract target names from the get_target_build_options function
    local build_common_path=""
    if [[ -f "$SCRIPT_DIR/tools/build-common.sh" ]]; then
        build_common_path="$SCRIPT_DIR/tools/build-common.sh"
    elif [[ -f "$SCRIPT_DIR/build-common.sh" ]]; then
        build_common_path="$SCRIPT_DIR/build-common.sh"
    else
        echo "Error: Could not find build-common.sh" >&2
        return 1
    fi
    
    # Extract target names from case statement in get_target_build_options function
    awk '
    /^get_target_build_options\(\)/ { in_function=1; next }
    in_function && /^}$/ { exit }
    in_function && /^[[:space:]]*[a-z0-9_-]+\)/ { 
        gsub(/^[[:space:]]*/, "")
        gsub(/\).*$/, "")
        if ($0 != "*" && $0 != "esac") print $0
    }
    ' "$build_common_path" | sort -u
}

# Auto-discover color screen targets by checking target CMakeLists.txt files - fully dynamic implementation
get_color_screen_targets() {
    local color_targets=""
    local all_targets
    
    all_targets=$(get_all_targets)
    
    for target in $all_targets; do
        # Get the PCB type for this target from build-common.sh
        local pcb_line
        pcb_line=$(awk -v target="$target" '
        /^get_target_build_options\(\)/ { in_function=1; next }
        in_function && /^}$/ { exit }
        in_function && $0 ~ "^[[:space:]]*" target "\\)" {
            found_target=1; next
        }
        found_target && /BUILD_OPTIONS.*-DPCB=/ {
            print $0; exit
        }
        found_target && /;;/ { exit }
        ' "$SCRIPT_DIR/build-common.sh" 2>/dev/null || true)
        
        if [[ -n "$pcb_line" ]]; then
            # Extract PCB type (e.g., -DPCB=X10 -> X10)
            local pcb_type
            pcb_type=$(echo "$pcb_line" | grep -o '\-DPCB=[A-Z0-9+]*' | cut -d= -f2)
            
            if [[ -n "$pcb_type" ]]; then
                # Dynamically determine target directory by parsing radio/src/CMakeLists.txt
                local radio_cmake_file="../radio/src/CMakeLists.txt"
                local target_dir=""
                
                if [[ -f "$SCRIPT_DIR/$radio_cmake_file" ]]; then
                    # Parse the if/elseif chain to find which target directory corresponds to our PCB type
                    target_dir=$(awk -v pcb="$pcb_type" '
                    /^if\(PCB STREQUAL|^elseif\(PCB STREQUAL/ {
                        condition_line = $0
                        # Extract all PCB types from the condition
                        gsub(/^[^(]*\(/, "", condition_line)  # Remove everything before first (
                        gsub(/\).*$/, "", condition_line)     # Remove everything after last )
                        gsub(/PCB STREQUAL /, "", condition_line)  # Remove "PCB STREQUAL "
                        gsub(/ OR /, " ", condition_line)     # Replace " OR " with space
                        
                        # Check if our PCB type is in this condition
                        if ((" " condition_line " ") ~ (" " pcb " ")) {
                            found_pcb = 1
                        }
                    }
                    found_pcb && /include\(targets\/.*\/CMakeLists\.txt\)/ {
                        # Extract target directory from include(targets/horus/CMakeLists.txt)
                        match($0, /targets\/([^\/]+)\//, arr)
                        if (arr[1]) {
                            print arr[1]
                            exit
                        }
                    }
                    /^else\(\)|^endif\(\)/ { found_pcb = 0 }
                    ' "$SCRIPT_DIR/$radio_cmake_file")
                fi
                
                if [[ -n "$target_dir" ]]; then
                    # Check if this target directory has color screen support
                    local target_cmake_file="../radio/src/targets/$target_dir/CMakeLists.txt"
                    
                    if [[ -f "$SCRIPT_DIR/$target_cmake_file" ]]; then
                        # Look for color screen indicators: GUI_DIR with colorlcd, COLORLCD definitions, etc.
                        if grep -q "GUI_DIR.*colorlcd\|COLORLCD\|set(GUI_DIR.*colorlcd" "$SCRIPT_DIR/$target_cmake_file"; then
                            color_targets="$color_targets $target"
                        fi
                    fi
                fi
            fi
        fi
    done
    
    # Return unique sorted targets
    echo "$color_targets" | tr ' ' '\n' | sort -u | tr '\n' ' '
}

# Auto-discover STM32 processor types for targets - fully dynamic implementation
get_stm32_processor_type() {
    local target=$1
    
    # Hard-coded case for x9dp (will be phased out)
    if [[ "$target" == "x9dp" ]]; then
        echo "F2"
        return 0
    fi
    
    # Get the PCB configuration for this target from build-common.sh
    local pcb_line
    pcb_line=$(awk -v target="$target" '
    /^get_target_build_options\(\)/ { in_function=1; next }
    in_function && /^}$/ { exit }
    in_function && $0 ~ "^[[:space:]]*" target "\\)" {
        found_target=1; next
    }
    found_target && /BUILD_OPTIONS.*-DPCB=/ {
        print $0
        exit
    }
    found_target && /;;/ { exit }
    ' "$SCRIPT_DIR/build-common.sh" 2>/dev/null || true)
    
    if [[ -z "$pcb_line" ]]; then
        echo "UNKNOWN"
        return 1
    fi
    
    # Extract PCB type and PCBREV from BUILD_OPTIONS+="-DPCB=X7 -DPCBREV=T8"
    local pcb_type
    local pcb_rev
    pcb_type=$(echo "$pcb_line" | grep -o '\-DPCB=[A-Z0-9+]*' | cut -d= -f2)
    pcb_rev=$(echo "$pcb_line" | grep -o '\-DPCBREV=[A-Z0-9+]*' | cut -d= -f2 || true)
    
    if [[ -z "$pcb_type" ]]; then
        echo "UNKNOWN"
        return 1
    fi
    
    # Dynamically determine target directory by parsing radio/src/CMakeLists.txt
    local radio_cmake_file="../radio/src/CMakeLists.txt"
    local target_dir=""
    
    if [[ -f "$SCRIPT_DIR/$radio_cmake_file" ]]; then
        # Parse the if/elseif chain to find which target directory corresponds to our PCB type
        target_dir=$(awk -v pcb="$pcb_type" '
        /^if\(PCB STREQUAL|^elseif\(PCB STREQUAL/ {
            condition_line = $0
            # Extract all PCB types from the condition
            gsub(/^[^(]*\(/, "", condition_line)  # Remove everything before first (
            gsub(/\).*$/, "", condition_line)     # Remove everything after last )
            gsub(/PCB STREQUAL /, "", condition_line)  # Remove "PCB STREQUAL "
            gsub(/ OR /, " ", condition_line)     # Replace " OR " with space
            
            # Check if our PCB type is in this condition
            if ((" " condition_line " ") ~ (" " pcb " ")) {
                found_pcb = 1
            }
        }
        found_pcb && /include\(targets\/.*\/CMakeLists\.txt\)/ {
            # Extract target directory from include(targets/horus/CMakeLists.txt)
            match($0, /targets\/([^\/]+)\//, arr)
            if (arr[1]) {
                print arr[1]
                exit
            }
        }
        /^else\(\)|^endif\(\)/ { found_pcb = 0 }
        ' "$SCRIPT_DIR/$radio_cmake_file")
    fi
    
    if [[ -z "$target_dir" ]]; then
        echo "UNKNOWN"
        return 1
    fi
    
    # Simulate the CMake logic to determine CPU_TYPE_FULL based on PCB and PCBREV
    local target_cmake_file="../radio/src/targets/$target_dir/CMakeLists.txt"
    
    if [[ -f "$SCRIPT_DIR/$target_cmake_file" ]]; then
        # Parse the target CMakeLists.txt to determine CPU_TYPE_FULL
        # We need to simulate the if/elseif logic for PCB and PCBREV
        local cpu_type_full
        
        # First, try to find CPU_TYPE_FULL within the specific PCBREV block if PCBREV is specified
        if [[ -n "$pcb_rev" ]]; then
            cpu_type_full=$(awk -v pcb="$pcb_type" -v pcbrev="$pcb_rev" '
            BEGIN { in_pcb_block = 0; in_pcbrev_block = 0; cpu_found = 0 }
            
            # Look for PCB conditions
            /^if\(PCB STREQUAL|^elseif\(PCB STREQUAL/ {
                condition_line = $0
                gsub(/^[^(]*\(/, "", condition_line)
                gsub(/\).*$/, "", condition_line)
                gsub(/PCB STREQUAL /, "", condition_line)
                gsub(/ OR /, " ", condition_line)
                
                # Check if our PCB type is in this condition (use index for exact matching)
                if (index(" " condition_line " ", " " pcb " ") > 0) {
                    in_pcb_block = 1
                } else {
                    in_pcb_block = 0
                }
                in_pcbrev_block = 0
            }
            
            # Look for PCBREV conditions within PCB block
            in_pcb_block && /\$\{PCBREV\} STREQUAL/ {
                condition_line = $0
                gsub(/.*\$\{PCBREV\} STREQUAL /, "", condition_line)
                gsub(/\).*$/, "", condition_line)
                gsub(/ .*$/, "", condition_line)
                
                if (condition_line == pcbrev) {
                    in_pcbrev_block = 1
                } else {
                    in_pcbrev_block = 0
                }
            }
            
            # Accept CPU_TYPE_FULL within PCBREV block
            in_pcbrev_block && /set\(CPU_TYPE_FULL STM32/ {
                match($0, /STM32[FH][0-9RSx]+[A-Z]*/, arr)
                if (arr[0] && !cpu_found) {
                    print arr[0]
                    cpu_found = 1
                    exit
                }
            }
            
            # Reset PCBREV block when we hit another condition or endif
            /^[[:space:]]*if\(|^[[:space:]]*elseif\(|^[[:space:]]*else\(|^[[:space:]]*endif\(/ && !/PCBREV/ {
                if (in_pcb_block && in_pcbrev_block) {
                    in_pcbrev_block = 0
                }
            }
            
            # Reset everything when we exit the PCB block
            /^else\(\)|^endif\(\)/ && !(/PCBREV/) {
                in_pcb_block = 0
                in_pcbrev_block = 0
            }
            ' "$SCRIPT_DIR/$target_cmake_file")
        fi
        
        # If no CPU_TYPE_FULL found in PCBREV block, or no PCBREV specified,
        # look for CPU_TYPE_FULL within the specific PCB block
        if [[ -z "$cpu_type_full" ]]; then
            # Use sed to extract the specific PCB block, then look for CPU_TYPE_FULL
            # For targets without PCBREV, we want the "else()" case, not the PCBREV-specific ones
            local pcb_block_start_pattern="^elseif(PCB STREQUAL $pcb_type)\|^if(PCB STREQUAL.*$pcb_type"
            local pcb_block_end_pattern="^elseif(PCB STREQUAL\|^else()\|^endif()"
            
            if [[ -z "$pcb_rev" ]]; then
                # No PCBREV specified, first try to find CPU_TYPE_FULL in else() block for complex targets
                cpu_type_full=$(sed -n "/$pcb_block_start_pattern/,/$pcb_block_end_pattern/p" "$SCRIPT_DIR/$target_cmake_file" | \
                               awk '
                               BEGIN { in_else_block = 0 }
                               /else\(\)/ { in_else_block = 1; next }
                               /endif\(\)/ { in_else_block = 0; next }
                               /\$\{PCBREV\} STREQUAL/ { in_else_block = 0; next }
                               in_else_block && /set\(CPU_TYPE_FULL STM32/ {
                                   match($0, /STM32[FH][0-9RSx]+[A-Z]*/, arr)
                                   if (arr[0]) { print arr[0]; exit }
                               }
                               # If no else() block, take any CPU_TYPE_FULL not in a PCBREV block
                               !in_else_block && /set\(CPU_TYPE_FULL STM32/ && (prev_line !~ /\$\{PCBREV\} STREQUAL/) {
                                   match($0, /STM32[FH][0-9RSx]+[A-Z]*/, arr)
                                   if (arr[0] && !found_any) { found_any=1; print arr[0]; exit }
                               }
                               { prev_line = $0 }
                               ')
                
                # If still not found (simple targets), use global CPU_TYPE_FULL
                if [[ -z "$cpu_type_full" ]]; then
                    cpu_type_full=$(grep "set(CPU_TYPE_FULL STM32" "$SCRIPT_DIR/$target_cmake_file" | head -1 | grep -o "STM32[FH][0-9RSx]*[A-Z]*")
                fi
            else
                # PCBREV specified but no CPU_TYPE_FULL found in PCBREV block, use global CPU_TYPE_FULL
                cpu_type_full=$(grep "set(CPU_TYPE_FULL STM32" "$SCRIPT_DIR/$target_cmake_file" | head -1 | grep -o "STM32[FH][0-9RSx]*[A-Z]*")
            fi
        fi
        
        if [[ -n "$cpu_type_full" ]]; then
            # Convert CPU_TYPE_FULL to CPU_TYPE (F2, F4, H7, etc.)
            local cpu_type
            case "$cpu_type_full" in
                STM32F2*) cpu_type="F2" ;;
                STM32F4*) cpu_type="F4" ;;
                STM32H7*) cpu_type="H7" ;;
                *) cpu_type="UNKNOWN" ;;
            esac
            echo "$cpu_type"
            return 0
        fi
    fi
    
    echo "UNKNOWN"
    return 1
}

# Function to get targets by STM32 processor type
get_targets_by_processor() {
    local processor_filter="$1"
    local all_targets=$(get_all_targets)
    local filtered_targets=""
    
    for target in $all_targets; do
        local target_processor=$(get_stm32_processor_type "$target")
        if [[ "$target_processor" == "$processor_filter" ]]; then
            filtered_targets="$filtered_targets $target"
        fi
    done
    
    echo "$filtered_targets"
}

# Function to get targets by processor exclusion
get_targets_excluding_processor() {
    local exclude_processor="$1"
    local all_targets=$(get_all_targets)
    local filtered_targets=""
    
    for target in $all_targets; do
        local target_processor=$(get_stm32_processor_type "$target")
        if [[ "$target_processor" != "$exclude_processor" && "$target_processor" != "UNKNOWN" ]]; then
            filtered_targets="$filtered_targets $target"
        fi
    done
    
    echo "$filtered_targets"
}

# Function to check if a target uses color screen
is_color_screen_target() {
    local target=$1
    local color_targets
    
    # Use override if set, otherwise auto-detect
    if [[ -n "$COLOR_SCREEN_TARGETS_OVERRIDE" ]]; then
        color_targets="$COLOR_SCREEN_TARGETS_OVERRIDE"
    else
        color_targets=$(get_color_screen_targets)
    fi
    
    # Check if target is in the color targets list
    for color_target in $color_targets; do
        if [[ "$target" == "$color_target" ]]; then
            return 0
        fi
    done
    return 1
}

# Function to check if a language is logographic
is_logographic_language() {
    local lang=$1
    local logographic_langs
    
    logographic_langs=$(get_logographic_languages)
    
    for logo_lang in $logographic_langs; do
        if [[ "$lang" == "$logo_lang" ]]; then
            return 0
        fi
    done
    return 1
}

# Function to get valid languages for a target
get_valid_languages_for_target() {
    local target=$1
    local valid_languages=""
    local radio_languages
    
    radio_languages=$(get_radio_languages)
    
    # Check if force all languages flag is set
    if [[ "${FORCE_ALL_LANGUAGES:-}" == "1" ]]; then
        # Force all languages mode: build all languages regardless of screen type
        valid_languages="$radio_languages"
    else
        # Normal mode: filter based on screen type
        for lang in $radio_languages; do
            if is_color_screen_target "$target"; then
                # Color screen: all languages allowed
                valid_languages="$valid_languages $lang"
            else
                # B&W screen: exclude logographic languages
                if ! is_logographic_language "$lang"; then
                    valid_languages="$valid_languages $lang"
                fi
            fi
        done
    fi
    
    echo "$valid_languages"
}

# Function to build a specific target with a specific language
build_target_language() {
    local target=$1
    local language=$2
    
    # Show building message without newline
    printf "Building %s with language %s... " "$target" "$language"
    
    # Dry run mode - just simulate the build
    if [[ "${DRY_RUN:-}" == "1" ]]; then
        printf "\033[32m✓\033[0m [DRY-RUN]\n"
        printf "  → Would build: TRANSLATIONS=%s FLAVOR=%s\n" "$language" "$target"
        return 0
    fi
    
    # Use build-gh.sh with the target and set the TRANSLATIONS environment variable
    # build-gh.sh expects FLAVOR environment variable with the target name
    local build_script=""
    if [[ -f "$SCRIPT_DIR/tools/build-gh.sh" ]]; then
        build_script="$SCRIPT_DIR/tools/build-gh.sh"
    elif [[ -f "$SCRIPT_DIR/build-gh.sh" ]]; then
        build_script="$SCRIPT_DIR/build-gh.sh"
    else
        echo "Error: Could not find build-gh.sh"
        return 1
    fi
    
    export FLAVOR="$target"
    export SRCDIR="/workspaces/edgetx"
    export EXTRA_OPTIONS="-DTRANSLATIONS=$language "
    
    # Create build directory if it doesn't exist and change to it
    local build_dir="$SCRIPT_DIR/../build"
    mkdir -p "$build_dir"
    local original_pwd="$PWD"
    cd "$build_dir"
    
    # Create a unique log file for this build
    local log_file="/tmp/edgetx-build-${target}-${language}-$$.log"
    
    # Run build script and capture all output to log file
    "$build_script" > "$log_file" 2>&1
    
    local exit_code=$?
    
    # Return to original directory
    cd "$original_pwd"
    
    if [[ $exit_code -eq 0 ]]; then
        printf "\033[32m✓\033[0m Success!\n"
        
        # Rename firmware files to include language code
        local git_sha=$(git rev-parse --short HEAD 2>/dev/null || echo "unknown")
        local base_name="${target}-${git_sha}"
        local lang_suffix="-${language,,}"  # Convert language to lowercase
        
        # Check for and rename .bin file
        if [[ -f "${base_name}.bin" ]]; then
            mv "${base_name}.bin" "${base_name}${lang_suffix}.bin"
        fi
        
        # Check for and rename .uf2 file  
        if [[ -f "${base_name}.uf2" ]]; then
            mv "${base_name}.uf2" "${base_name}${lang_suffix}.uf2"
        fi
        
        # Clean up log file on success
        rm -f "$log_file"
    else
        printf "\033[31m✗\033[0m Failed!\n"
        printf "  Build log saved to: %s\n" "$log_file"
        if [[ "${CONTINUE_ON_FAILURE:-0}" != "1" ]]; then
            printf "  Terminating current batch due to build failure\n"
        fi
        return 1
    fi
    
    return 0
}

# Function to build all languages for a target
build_target_all_languages() {
    local target=$1
    local languages
    local failed_builds=0
    local start_time=$(date +%s)
    
    languages=$(get_valid_languages_for_target "$target")
    
    if [[ "${FORCE_ALL_LANGUAGES:-}" == "1" ]]; then
        echo "Building target '$target' with ALL languages (forced): $languages"
        echo "  → Force mode - building all languages including logographic for B&W screens"
    else
        echo "Building target '$target' with languages: $languages"
        
        if is_color_screen_target "$target"; then
            echo "  → Color screen target - building all languages"
        else
            echo "  → B&W screen target - excluding logographic languages ($(get_logographic_languages))"
        fi
    fi
    
    for language in $languages; do
        if ! build_target_language "$target" "$language"; then
            ((failed_builds++))
            # Stop building more languages for this target after first failure unless --continue-on-failure is set
            if [[ "${CONTINUE_ON_FAILURE:-}" != "1" ]]; then
                break
            fi
        fi
    done
    
    local end_time=$(date +%s)
    local elapsed=$((end_time - start_time))
    local elapsed_formatted
    
    # Format elapsed time nicely
    if [[ $elapsed -ge 3600 ]]; then
        # Hours, minutes, seconds
        elapsed_formatted=$(printf "%dh %dm %ds" $((elapsed / 3600)) $(((elapsed % 3600) / 60)) $((elapsed % 60)))
    elif [[ $elapsed -ge 60 ]]; then
        # Minutes and seconds
        elapsed_formatted=$(printf "%dm %ds" $((elapsed / 60)) $((elapsed % 60)))
    else
        # Just seconds
        elapsed_formatted="${elapsed}s"
    fi
    
    if [[ $failed_builds -gt 0 ]]; then
        echo "Warning: $failed_builds language builds failed for target $target (${elapsed_formatted})"
        return 1
    else
        echo "Target $target completed successfully (${elapsed_formatted})"
    fi
    
    return 0
}

# Function to build all targets with all valid languages
build_all_targets() {
    local all_targets
    local failed_targets=0
    local overall_start_time=$(date +%s)
    
    all_targets=$(get_all_targets)
    
    echo "Building all firmware variants with appropriate languages..."
    echo
    echo "Detected targets: $(echo $all_targets | tr ' ' ',')"
    echo
    
    for target in $all_targets; do
        echo "========================================"
        echo "Processing target: $target"
        echo "========================================"
        
        if ! build_target_all_languages "$target"; then
            ((failed_targets++))
            echo "Failed to build all languages for target $target"
        fi
        
        echo
    done
    
    # Calculate overall elapsed time
    local overall_end_time=$(date +%s)
    local overall_elapsed=$((overall_end_time - overall_start_time))
    
    # Format overall elapsed time
    local overall_elapsed_formatted
    if [[ $overall_elapsed -ge 3600 ]]; then
        overall_elapsed_formatted=$(printf "%dh %dm %ds" $((overall_elapsed / 3600)) $(((overall_elapsed % 3600) / 60)) $((overall_elapsed % 60)))
    elif [[ $overall_elapsed -ge 60 ]]; then
        overall_elapsed_formatted=$(printf "%dm %ds" $((overall_elapsed / 60)) $((overall_elapsed % 60)))
    else
        overall_elapsed_formatted="${overall_elapsed}s"
    fi
    
    if [[ $failed_targets -gt 0 && "${CONTINUE_ON_FAILURE:-}" != "1" ]]; then
        echo "========================================="
        echo "BUILD SUMMARY"
        echo "========================================="
        printf "\033[31m✗\033[0m Warning: %d targets had build failures\n" "$failed_targets"
        echo "Total build time: ${overall_elapsed_formatted}"
        return 1
    else
        echo "========================================="
        echo "BUILD SUMMARY"
        echo "========================================="
        if [[ $failed_targets -gt 0 && "${CONTINUE_ON_FAILURE:-}" == "1" ]]; then
            printf "\033[32m✓\033[0m All targets attempted successfully!\n"
            echo "  Note: Some language builds failed but were ignored due to --continue-on-failure"
        else
            printf "\033[32m✓\033[0m All targets built successfully!\n"
        fi
        echo "Total build time: ${overall_elapsed_formatted}"
        return 0
    fi
}

show_usage() {
    echo "EdgeTX Multi-Language Multi-Variant Build Script"
    echo
    echo "Usage: $0 [OPTIONS] [TARGET]"
    echo
    echo "Options:"
    echo "  -h, --help           Show this help message"
    echo "  -l, --list-targets   List all available firmware targets"
    echo "  -c, --list-color     List color screen targets"
    echo "  -b, --list-bw        List black & white screen targets"
    echo "  -L, --list-languages List all available languages (auto-detected)"
    echo "  --list-logographic   List logographic/character-based languages"
    echo "  --list-color-targets-detected  Show auto-detected color screen targets"
    echo "  --list-processors    List targets grouped by STM32 processor type"
    echo "  -n, --dry-run        Show what would be built without building"
    echo "  -f, --force-all      Force build all languages for target (ignores screen type)"
    echo "  --continue-on-failure Continue building other languages/targets even after failures"
    echo "  --color-only         Build all color screen targets only"
    echo "  --bw-only            Build all black & white screen targets only"
    echo "  --processor PROC     Build targets with specific STM32 processor (F2|F4|H7)"
    echo "  --exclude-processor PROC  Build targets excluding specific processor"
    echo "  --filter-processor PROC    Filter list commands by processor (F2|F4|H7|exclude-F2|etc.)"
    echo
    echo "Arguments:"
    echo "  TARGET [TARGET...]  Build all valid languages for specific target(s)"
    echo "                      (if not specified, builds all targets with screen type rules)"
    echo
    echo "Examples:"
    echo "  $0                  # Build all languages for all targets"
    echo "  $0 tx16s            # Build all languages for TX16S"
    echo "  $0 tx16s gx12 x9d   # Build all languages for multiple targets"
    echo "  $0 x7               # Build non-logographic languages for X7 (B&W)"
    echo "  $0 -f x7            # Force build ALL languages for X7 (including CN,JP,KO,TW)"
    echo "  $0 -f tx16s st16 x9d # Force build ALL languages for multiple targets"
    echo "  $0 --color-only     # Build all color screen targets with appropriate languages"
    echo "  $0 --bw-only        # Build all B&W screen targets (excluding logographic)"
    echo "  $0 --bw-only -f     # Build all B&W screen targets with ALL languages"
    echo "  $0 --processor F2   # Build all F2 processor targets"
    echo "  $0 --bw-only --processor F2  # Build F2 B&W targets only"
    echo "  $0 --exclude-processor F2    # Build all targets except F2"
    echo "  $0 -l               # List all available targets"
    echo "  $0 -l --filter-processor F4  # List only F4 targets"
    echo "  $0 -c --filter-processor H7  # List only H7 color screen targets"
    echo "  $0 -b --filter-processor exclude-F2  # List B&W targets excluding F2"
    echo "  $0 --list-processors     # Show all targets grouped by processor type"
    echo "  $0 -n x7            # Show what languages would be built for X7"
    echo "  $0 -n tx16s gx12    # Show what would be built for multiple targets"
    echo "  $0 --continue-on-failure x7  # Build all languages for X7 even if some fail"
    echo "  $0 --continue-on-failure --color-only  # Build all color targets, continue on failures"
    echo
    echo "Language Rules:"
    echo "  - Color screen radios: All languages ($(get_radio_languages))"
    echo "  - B&W screen radios: All languages except logographic ($(get_logographic_languages))"
    echo "  - Force mode (-f): All languages regardless of screen type"
    echo
    echo "Auto-Detection:"
    echo "  - Languages: Parsed from radio/src/CMakeLists.txt"
    echo "  - Color targets: Detected from radio/src/targets/*/CMakeLists.txt"
    echo "  - Logographic languages: Configurable (override with LOGOGRAPHIC_LANGUAGES_OVERRIDE)"
}

# Function to list targets by screen type with optional processor filtering
list_targets_by_type() {
    local screen_filter="$1"     # "color", "bw", or "all"
    local processor_filter="$2"  # Optional: "F2", "F4", "H7", "exclude-F2", etc.
    
    echo "Scanning for available targets..." >&2
    local all_targets
    local color_targets=""
    local bw_targets=""
    
    all_targets=$(get_all_targets)
    
    # First, categorize by screen type
    for target in $all_targets; do
        if is_color_screen_target "$target"; then
            color_targets="$color_targets $target"
        else
            bw_targets="$bw_targets $target"
        fi
    done
    
    # Apply processor filter if specified
    if [[ -n "$processor_filter" ]]; then
        local filtered_color_targets=""
        local filtered_bw_targets=""
        
        if [[ "$processor_filter" == exclude-* ]]; then
            # Exclude specific processor
            local exclude_proc="${processor_filter#exclude-}"
            for target in $color_targets; do
                local target_processor=$(get_stm32_processor_type "$target")
                if [[ "$target_processor" != "$exclude_proc" && "$target_processor" != "UNKNOWN" ]]; then
                    filtered_color_targets="$filtered_color_targets $target"
                fi
            done
            for target in $bw_targets; do
                local target_processor=$(get_stm32_processor_type "$target")
                if [[ "$target_processor" != "$exclude_proc" && "$target_processor" != "UNKNOWN" ]]; then
                    filtered_bw_targets="$filtered_bw_targets $target"
                fi
            done
        else
            # Include only specific processor
            for target in $color_targets; do
                local target_processor=$(get_stm32_processor_type "$target")
                if [[ "$target_processor" == "$processor_filter" ]]; then
                    filtered_color_targets="$filtered_color_targets $target"
                fi
            done
            for target in $bw_targets; do
                local target_processor=$(get_stm32_processor_type "$target")
                if [[ "$target_processor" == "$processor_filter" ]]; then
                    filtered_bw_targets="$filtered_bw_targets $target"
                fi
            done
        fi
        
        color_targets="$filtered_color_targets"
        bw_targets="$filtered_bw_targets"
    fi
    
    # Display results based on screen filter
    case "$screen_filter" in
        "color")
            if [[ -n "$processor_filter" ]]; then
                echo "Color screen targets with processor filter ($processor_filter):"
            else
                echo "Color screen targets:"
            fi
            for target in $color_targets; do
                echo "  $target ($(get_stm32_processor_type "$target"))"
            done
            ;;
        "bw")
            if [[ -n "$processor_filter" ]]; then
                echo "Black & white screen targets with processor filter ($processor_filter):"
            else
                echo "Black & white screen targets:"
            fi
            for target in $bw_targets; do
                echo "  $target ($(get_stm32_processor_type "$target"))"
            done
            ;;
        "all")
            if [[ -n "$processor_filter" ]]; then
                echo "Color screen targets with processor filter ($processor_filter):"
            else
                echo "Color screen targets:"
            fi
            for target in $color_targets; do
                echo "  $target ($(get_stm32_processor_type "$target"))"
            done
            echo
            if [[ -n "$processor_filter" ]]; then
                echo "Black & white screen targets with processor filter ($processor_filter):"
            else
                echo "Black & white screen targets:"
            fi
            for target in $bw_targets; do
                echo "  $target ($(get_stm32_processor_type "$target"))"
            done
            ;;
    esac
}

# Function to list targets by processor type
list_targets_by_processor() {
    local all_targets=$(get_all_targets)
    local f2_targets=""
    local f4_targets=""
    local h7_targets=""
    local unknown_targets=""
    
    for target in $all_targets; do
        local processor=$(get_stm32_processor_type "$target")
        case "$processor" in
            F2) f2_targets="$f2_targets $target" ;;
            F4) f4_targets="$f4_targets $target" ;;
            H7) h7_targets="$h7_targets $target" ;;
            *) unknown_targets="$unknown_targets $target" ;;
        esac
    done
    
    echo "Targets grouped by STM32 processor type:"
    echo
    echo "STM32F2 targets:"
    for target in $f2_targets; do
        echo "  $target"
    done
    echo
    echo "STM32F4 targets:"
    for target in $f4_targets; do
        echo "  $target"
    done
    echo
    echo "STM32H7 targets:"
    for target in $h7_targets; do
        echo "  $target"
    done
    
    if [[ -n "$unknown_targets" ]]; then
        echo
        echo "Unknown processor targets:"
        for target in $unknown_targets; do
            echo "  $target"
        done
    fi
}

# Main script logic
# Parse force flag first
FORCE_ALL_LANGUAGES=0
DRY_RUN=0
CONTINUE_ON_FAILURE=0
TARGET=""
SCREEN_TYPE_FILTER=""
PROCESSOR_FILTER=""
LIST_PROCESSOR_FILTER=""
ACTION=""

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            show_usage
            exit 0
            ;;
        -l|--list-targets)
            ACTION="list-all"
            shift
            ;;
        -c|--list-color)
            ACTION="list-color"
            shift
            ;;
        -b|--list-bw)
            ACTION="list-bw"
            shift
            ;;
        -L|--list-languages)
            ACTION="list-languages"
            shift
            ;;
        --list-logographic)
            ACTION="list-logographic"
            shift
            ;;
        --list-color-targets-detected)
            ACTION="list-color-detected"
            shift
            ;;
        --list-processors)
            ACTION="list-processors"
            shift
            ;;
        -n|--dry-run)
            DRY_RUN=1
            shift
            ;;
        -f|--force-all)
            FORCE_ALL_LANGUAGES=1
            shift
            ;;
        --continue-on-failure)
            CONTINUE_ON_FAILURE=1
            shift
            ;;
        --color-only)
            SCREEN_TYPE_FILTER="color"
            shift
            ;;
        --bw-only)
            SCREEN_TYPE_FILTER="bw"
            shift
            ;;
        --processor)
            if [[ -n "${2:-}" && ! "$2" =~ ^- ]]; then
                PROCESSOR_FILTER="$2"
                shift 2
            else
                echo "Error: --processor requires a processor type (F2|F4|H7)"
                exit 1
            fi
            ;;
        --exclude-processor)
            if [[ -n "${2:-}" && ! "$2" =~ ^- ]]; then
                PROCESSOR_FILTER="exclude-$2"
                shift 2
            else
                echo "Error: --exclude-processor requires a processor type (F2|F4|H7)"
                exit 1
            fi
            ;;
        --filter-processor)
            if [[ -n "${2:-}" && ! "$2" =~ ^- ]]; then
                LIST_PROCESSOR_FILTER="$2"
                shift 2
            else
                echo "Error: --filter-processor requires a processor type (F2|F4|H7|exclude-F2|etc.)"
                exit 1
            fi
            ;;
        -*)
            echo "Unknown option: $1"
            show_usage
            exit 1
            ;;
        *)
            if [[ -z "$TARGET" ]]; then
                TARGET="$1"
            else
                # Support multiple targets - append to TARGET with space separator
                TARGET="$TARGET $1"
            fi
            shift
            ;;
    esac
done

# Execute actions after all arguments are parsed
case "$ACTION" in
    "list-all")
        list_targets_by_type "all" "$LIST_PROCESSOR_FILTER"
        exit 0
        ;;
    "list-color")
        list_targets_by_type "color" "$LIST_PROCESSOR_FILTER"
        exit 0
        ;;
    "list-bw")
        list_targets_by_type "bw" "$LIST_PROCESSOR_FILTER"
        exit 0
        ;;
    "list-languages")
        echo "All available languages:"
        for lang in $(get_radio_languages); do
            echo "  $lang"
        done
        exit 0
        ;;
    "list-logographic")
        echo "Logographic/character-based languages:"
        for lang in $(get_logographic_languages); do
            echo "  $lang"
        done
        echo
        echo "These languages are not built for B&W screen radios (unless --force-all is used)."
        exit 0
        ;;
    "list-color-detected")
        echo "Auto-detected color screen targets:"
        for target in $(get_color_screen_targets); do
            echo "  $target"
        done
        exit 0
        ;;
    "list-processors")
        list_targets_by_processor
        exit 0
        ;;
esac

# Export flags for use in functions
export FORCE_ALL_LANGUAGES
export DRY_RUN
export CONTINUE_ON_FAILURE

# Validate processor filter if specified
if [[ -n "$PROCESSOR_FILTER" ]]; then
    proc="${PROCESSOR_FILTER#exclude-}"
    if [[ "$proc" != "F2" && "$proc" != "F4" && "$proc" != "H7" ]]; then
        echo "Error: Invalid processor type '$proc'. Valid options: F2, F4, H7"
        exit 1
    fi
fi

# Validate list processor filter if specified
if [[ -n "$LIST_PROCESSOR_FILTER" ]]; then
    proc="${LIST_PROCESSOR_FILTER#exclude-}"
    if [[ "$proc" != "F2" && "$proc" != "F4" && "$proc" != "H7" ]]; then
        echo "Error: Invalid filter processor type '$proc'. Valid options: F2, F4, H7, exclude-F2, exclude-F4, exclude-H7"
        exit 1
    fi
fi

# Handle screen type filtering (--color-only or --bw-only)
if [[ -n "$SCREEN_TYPE_FILTER" ]]; then
    if [[ -n "$TARGET" ]]; then
        echo "Error: Cannot specify both screen type filter and individual target"
        show_usage
        exit 1
    fi
    
    build_targets_by_screen_type "$SCREEN_TYPE_FILTER" "$PROCESSOR_FILTER"
    exit $?
fi

# Handle processor-only filtering (--processor or --exclude-processor without screen filter)
if [[ -n "$PROCESSOR_FILTER" && -z "$SCREEN_TYPE_FILTER" ]]; then
    if [[ -n "$TARGET" ]]; then
        echo "Error: Cannot specify both processor filter and individual target"
        show_usage
        exit 1
    fi
    
    # Build all targets (both color and B&W) with processor filter
    filtered_targets=""
    if [[ "$PROCESSOR_FILTER" == exclude-* ]]; then
        filtered_targets=$(get_targets_excluding_processor "${PROCESSOR_FILTER#exclude-}")
    else
        filtered_targets=$(get_targets_by_processor "$PROCESSOR_FILTER")
    fi
    
    if [[ -z "$filtered_targets" ]]; then
        echo "No targets match the processor filter: $PROCESSOR_FILTER"
        exit 0
    fi
    
    echo "Building targets with processor filter: $PROCESSOR_FILTER"
    echo "Selected targets: $filtered_targets"
    echo
    
    failed_targets=0
    for target in $filtered_targets; do
        echo "========================================"
        echo "Processing target: $target ($(get_stm32_processor_type "$target") processor)"
        echo "========================================"
        
        if ! build_target_all_languages "$target"; then
            ((failed_targets++))
            echo "Failed to build all languages for target $target"
        fi
        
        echo
    done
    
    if [[ $failed_targets -gt 0 ]]; then
        echo "Warning: $failed_targets targets had build failures"
        exit 1
    else
        echo "All targets built successfully!"
        exit 0
    fi
fi

# Handle dry-run mode
if [[ $DRY_RUN -eq 1 ]]; then
    if [[ -n "$TARGET" ]]; then
        all_targets=$(get_all_targets)
        
        # Convert TARGET string to array and validate all targets exist
        read -ra target_array <<< "$TARGET"
        for target in "${target_array[@]}"; do
            if ! echo "$all_targets" | grep -q "^$target$"; then
                echo "Error: Unknown target '$target'"
                echo
                echo "Available targets:"
                list_targets_by_type "all"
                exit 1
            fi
        done
        
        # Build all specified targets in dry-run mode
        echo "DRY-RUN: Building specified targets: ${target_array[*]}"
        echo
        
        for target in "${target_array[@]}"; do
            echo "========================================"
            echo "Processing target: $target"
            echo "========================================"
            
            build_target_all_languages "$target"
            echo
        done
        exit 0
    fi
    # If no specific target, let it fall through to the default behavior below
    # The dry-run logic is handled in build_single_target function
fi

# Handle target building
if [[ -n "$TARGET" ]]; then
    all_targets=$(get_all_targets)
    
    # Convert TARGET string to array and validate all targets exist
    read -ra target_array <<< "$TARGET"
    for target in "${target_array[@]}"; do
        if ! echo "$all_targets" | grep -q "^$target$"; then
            echo "Error: Unknown target '$target'"
            echo
            echo "Available targets:"
            list_targets_by_type "all"
            exit 1
        fi
    done
    
    # Build all specified targets
    failed_targets=0
    overall_start_time=$(date +%s)
    echo "Building specified targets: ${target_array[*]}"
    echo
    
    # Temporarily disable exit on error for target building
    set +e
    
    for target in "${target_array[@]}"; do
        echo "========================================"
        echo "Processing target: $target"
        echo "========================================"
        
        if ! build_target_all_languages "$target"; then
            ((failed_targets++))
            echo "Failed to build all languages for target $target"
        fi
        
        echo
    done
    
    # Re-enable exit on error
    set -e
    
    # Calculate overall elapsed time
    overall_end_time=$(date +%s)
    overall_elapsed=$((overall_end_time - overall_start_time))
    
    # Format overall elapsed time
    if [[ $overall_elapsed -ge 3600 ]]; then
        overall_elapsed_formatted=$(printf "%dh %dm %ds" $((overall_elapsed / 3600)) $(((overall_elapsed % 3600) / 60)) $((overall_elapsed % 60)))
    elif [[ $overall_elapsed -ge 60 ]]; then
        overall_elapsed_formatted=$(printf "%dm %ds" $((overall_elapsed / 60)) $((overall_elapsed % 60)))
    else
        overall_elapsed_formatted="${overall_elapsed}s"
    fi
    
    if [[ $failed_targets -gt 0 && "${CONTINUE_ON_FAILURE:-}" != "1" ]]; then
        echo "========================================="
        echo "BUILD SUMMARY"
        echo "========================================="
        printf "\033[31m✗\033[0m Warning: %d targets had build failures\n" "$failed_targets"
        echo "Total build time: ${overall_elapsed_formatted}"
        exit 1
    else
        echo "========================================="
        echo "BUILD SUMMARY"
        echo "========================================="
        if [[ $failed_targets -gt 0 && "${CONTINUE_ON_FAILURE:-}" == "1" ]]; then
            printf "\033[32m✓\033[0m All targets attempted successfully!\n"
            echo "  Note: Some language builds failed but were ignored due to --continue-on-failure"
        else
            printf "\033[32m✓\033[0m All targets built successfully!\n"
        fi
        echo "Total build time: ${overall_elapsed_formatted}"
        exit 0
    fi
else
    # No target specified - build all targets
    build_all_targets
    exit $?
fi
