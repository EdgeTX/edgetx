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

# Determine parallel jobs
determine_max_jobs

QUIET_FLAGS=""
if [[ "$CMAKE_GENERATOR" == "Ninja" ]]; then
  QUIET_FLAGS="-- --quiet"
else
  # Assume Makefile generator for non-Ninja builds
  COMMON_OPTIONS="-DCMAKE_RULE_MESSAGES=OFF"
fi

COMMON_OPTIONS="${COMMON_OPTIONS} -DCMAKE_BUILD_TYPE=Release -DCMAKE_MESSAGE_LOG_LEVEL=WARNING -Wno-dev"
if [ "$(uname)" = "Darwin" ]; then
  COMMON_OPTIONS="${COMMON_OPTIONS} -DCMAKE_OSX_DEPLOYMENT_TARGET='11.0'"
fi

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

rm -rf build && mkdir build && cd build

get_platform_config() {
    local platform=$(uname)
    case "$platform" in
        "Darwin")
            PACKAGE_TARGET="package"
            PACKAGE_FILES="*.dmg"
            PACKAGE_NAME="macOS DMG"
            PACKAGE_EMOJI="🍎"
            ;;
        "Linux")
            PACKAGE_TARGET="package"
            PACKAGE_FILES="*.AppImage"
            PACKAGE_NAME="Linux AppImage"
            PACKAGE_EMOJI="🐧"
            ;;
        *)
            PACKAGE_TARGET="installer"
            PACKAGE_FILES="companion/*.exe"
            PACKAGE_NAME="Windows installer"
            PACKAGE_EMOJI="🪟"
            ;;
    esac
}

get_platform_config

BUILD_OPTIONS="${COMMON_OPTIONS} -DEdgeTX_SUPERBUILD:BOOL=0 -DNATIVE_BUILD:BOOL=1"
LOG_FILE="build-companion.log"

if [[ -n "$GITHUB_ACTIONS" ]]; then
    echo "::group::📦 Building $PACKAGE_NAME"
else
    echo "📦 Building $PACKAGE_NAME"
    echo "=========================================="
fi

clean_build() {
    rm -f CMakeCache.txt native/CMakeCache.txt
}

clean_build && mkdir -p native/plugins

# Copy WASM simulator modules if available
if [[ -d "${SRCDIR}/wasm-modules" ]]; then
  cp "${SRCDIR}"/wasm-modules/*.wasm native/plugins/ 2>/dev/null && \
    echo "    🔌 Copied WASM modules to plugins/" || true
fi

error_status=0

if ! cmake -B native -S "${SRCDIR}" --toolchain cmake/toolchain/native.cmake ${BUILD_OPTIONS} >> "$LOG_FILE" 2>&1; then
    echo "    ❌ CMake configuration failed"
    cat "$LOG_FILE"
    error_status=1
elif ! cmake_build_parallel native --target companion ${QUIET_FLAGS} >> "$LOG_FILE" 2>&1; then
    echo "    ❌ Companion build failed"
    cat "$LOG_FILE"
    error_status=1
elif ! cmake_build_parallel native --target ${PACKAGE_TARGET} >> "$LOG_FILE" 2>&1; then
    echo "    ❌ Packaging failed"
    cat "$LOG_FILE"
    error_status=1
elif cp native/$PACKAGE_FILES "${OUTDIR}" 2>/dev/null; then
    echo "    ✅ Build completed successfully!"
    echo "    📁 Package saved to: ${OUTDIR}"
else
    echo "    ❌ Failed to copy package files to output directory"
    ls -la native/ || echo "native/ directory not found"
    error_status=1
fi

if [[ -n "$GITHUB_ACTIONS" ]]; then
    echo "::endgroup::"
fi

exit $error_status
