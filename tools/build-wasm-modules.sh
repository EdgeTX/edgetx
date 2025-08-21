#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
. "$SCRIPT_DIR/build-common.sh" 

SRCDIR=$1
OUTDIR=$2

if [[ -z ${SRCDIR} ]]; then
  SRCDIR="$(pwd)"
fi

if [[ -z ${OUTDIR} ]]; then
  OUTDIR="$(pwd)/output"
fi

if [[ ! -d "${OUTDIR}" ]]; then
  mkdir -p "${OUTDIR}"
fi

QUIET_FLAGS=""
if [[ "$CMAKE_GENERATOR" == "Ninja" ]]; then
  QUIET_FLAGS="-- --quiet"
else
  # Assume Makefile generator for non-Ninja builds
  COMMON_OPTIONS="-DCMAKE_RULE_MESSAGES=OFF"
fi

COMMON_OPTIONS+=" -DCMAKE_BUILD_TYPE=Release -DCMAKE_MESSAGE_LOG_LEVEL=WARNING -Wno-dev"
COMMON_OPTIONS+=" -DCMAKE_MODULE_PATH=/opt/wasi-sdk/share/cmake/"
COMMON_OPTIONS+=" -DEdgeTX_SUPERBUILD:BOOL=0 -DNATIVE_BUILD:BOOL=1 -DDISABLE_COMPANION:BOOL=1"

# Generate EDGETX_VERSION_SUFFIX if not already set
if [[ -z ${EDGETX_VERSION_SUFFIX} ]]; then
  gh_type=$(echo "$GITHUB_REF" | awk -F / '{print $2}') #heads|tags|pull
  if [[ $gh_type = "tags" ]]; then
    # tags: refs/tags/<tag_name>
    gh_tag=${GITHUB_REF##*/}
    export EDGETX_VERSION_TAG=$gh_tag
  elif [[ $gh_type = "pull" ]]; then
    # pull: refs/pull/<pr_number>/merge
    gh_pull_number=PR$(echo "$GITHUB_REF" | awk -F / '{print $3}')
    export EDGETX_VERSION_SUFFIX=$gh_pull_number
  elif [[ $gh_type = "heads" ]]; then
    # heads: refs/heads/<branch_name>
    gh_branch=${GITHUB_REF##*/}
    export EDGETX_VERSION_SUFFIX=$gh_branch
  fi
fi

if [ "$(uname)" = "Linux" ] && [ -n "$GITHUB_ACTIONS" ]; then
  MAX_JOBS=3
fi

rm -rf build && mkdir build && cd build

# Function to output error logs (works in both GitHub Actions and terminal)
output_error_log() {
    local log_file="$1"
    local context="$2"
    
    if [[ -f "$log_file" ]]; then
        echo "------------------------------------------"
        echo " Full error output from $log_file:"
        echo "------------------------------------------"
        cat "$log_file"
    else
        echo "⚠️ Warning: Log file $log_file not found for $context"
    fi
}

# Function to show last N lines of a log file for warnings  
show_log_summary() {
    local log_file="$1"
    local lines="${2:-50}"
    local context="$3"
    
    if [[ -f "$log_file" ]]; then
        echo "------------------------------------------"
        echo "Last $lines lines from $log_file:"
        echo "------------------------------------------"
        tail -n "$lines" "$log_file"
    fi
}

run_pipeline() {
    local log_file="${1:-/dev/null}"
    local context="$2"
    local show_details="${3:-false}"
    local cmake_opts="--parallel ${MAX_JOBS} ${QUIET_FLAGS}"
    local wasi_toolchain="/opt/wasi-sdk/share/cmake/wasi-sdk-pthread.cmake"
    
    clean_build
    if ! execute_with_output "🔧 Final config" "cmake -S ${SRCDIR} -B wasm --toolchain ${wasi_toolchain} ${BUILD_OPTIONS}" "$log_file" "$show_details"; then
        output_error_log "$log_file" "$context (Configuration)"
        return 1
    fi
    if ! execute_with_output "📦 Building WASM module" "cmake --build wasm --target wasi-module ${cmake_opts}" "$log_file" "$show_details"; then
        output_error_log "$log_file" "$context (Library Build)"
        return 1
    fi

    return 0
}

execute_with_output() {
    local description="$1"
    local command="$2" 
    local log_file="$3"
    local show_output="${4:-false}"
    
    if [[ "$show_output" == "true" && "$log_file" != "/dev/null" ]]; then
        echo "    $description..." 
    fi
    
    eval "$command" >> "$log_file" 2>&1
}

clean_build() {
    rm -f CMakeCache.txt native/CMakeCache.txt
}

# Enhanced plugin builder with better error handling
build_plugin() {
    local plugin="$1"
    local log_file="build_${plugin}.log"
    local verbose="${2:-false}"
    
    BUILD_OPTIONS="${COMMON_OPTIONS} "
    
    if ! get_target_build_options "$plugin" >> "$log_file" 2>&1; then
        if [[ -n "$GITHUB_ACTIONS" ]]; then
            echo "::error::Failed to get build options for $plugin"
        fi
        output_error_log "$log_file" "$plugin (Build Options)"
        return 1
    fi
    
    # Only show detailed output in GitHub Actions or if verbose is requested
    local show_details="false"
    if [[ -n "$GITHUB_ACTIONS" || "$verbose" == "true" ]]; then
        show_details="true"
    fi
    
    if ! run_pipeline "$log_file" "$plugin" "$show_details"; then
        return 1
    fi

    if [[ -f "wasm/wasi-module.wasm" ]]; then
      cp "wasm/wasi-module.wasm" "${OUTDIR}/${plugin}.wasm" 2>/dev/null
    fi
    
    # Check for warnings and show summary if found
    if grep -q -i "warning" "$log_file"; then
        echo "    ⚠️ $plugin completed with warnings"
        if [[ "$show_details" == "true" ]]; then
            show_log_summary "$log_file" 50 "$plugin (Warnings)"
        fi
        return 0
    fi

    echo "    ✅ $plugin completed successfully"
    return 0
}

declare -a simulator_plugins=(
    # x9lite x9lites x9d x9dp x9dp2019 x9e
    # x7 x7access
    # t8 t12 t12max tx12 tx12mk2 t15 t16 t18 t20 t20v2
    xlite xlites
    x10 x10express x12s
    zorro tx16s tx15
    commando8 boxer pocket mt12 gx12
    tlite tpro tprov2 tpros bumblebee lr3pro t14
    nv14 el18 pl18 pl18ev pl18u st16 pa01
    f16 v14 v16
)

TOTAL=${#simulator_plugins[@]}
FAILED_PLUGINS=()

echo "🔨 Building $TOTAL Plugins for $PACKAGE_NAME"

for i in "${!simulator_plugins[@]}"; do
    plugin="${simulator_plugins[$i]}"
    current=$((i + 1))
    percent=$((current * 100 / TOTAL))
    
    # For terminal output, put each plugin on its own line for cleaner display
    if [[ -n "$GITHUB_ACTIONS" ]]; then
        printf "::group::📦 %-12s [%2d/%2d] %3d%%\n" "$plugin" "$current" "$TOTAL" "$percent"
    else
        echo "🔨 [$current/$TOTAL] ($percent%) Building $plugin..."
    fi

    error_status=0    
    if ! build_plugin "$plugin"; then
        FAILED_PLUGINS+=("$plugin")
        error_status=1
    fi

    if [[ -n "$GITHUB_ACTIONS" ]]; then
        echo "::endgroup::"
    fi

    if [ $error_status -ne 0 ]; then
        # TODO: if not '--build-all' used
        exit 1
    fi
done

# Show summary of any failures (if using continue-on-error approach)
if [ ${#FAILED_PLUGINS[@]} -gt 0 ]; then
    if [[ -n "$GITHUB_ACTIONS" ]]; then
        echo "::group::❌ Build Failures Summary"
    else
        echo "❌ Build Failures Summary"
        echo "=========================================="
    fi
    
    echo "The following plugins failed to build:"
    for failed_plugin in "${FAILED_PLUGINS[@]}"; do
        echo "    ❌ $failed_plugin"
    done
    
    if [[ -n "$GITHUB_ACTIONS" ]]; then
        echo "::endgroup::"
    else
        echo "=========================================="
    fi
    exit 1
fi
