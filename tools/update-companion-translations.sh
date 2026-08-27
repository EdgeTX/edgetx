#!/bin/bash

# Stops on first error, echo on
set -e
set -x

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
. "$SCRIPT_DIR/build-common.sh" 

# Determine number of CPU cores for parallel builds
if [ -f /usr/bin/nproc ]; then
    num_cpus=$(nproc)
elif [ "$(uname)" = "Darwin" ]; then
    num_cpus=$(sysctl -n hw.ncpu)
elif [ -f /usr/sbin/sysctl ]; then
    num_cpus=$(sysctl -n hw.logicalcpu)
else
    num_cpus=2
fi
: "${JOBS:=$num_cpus}"

# Parse command line arguments
while [ $# -gt 0 ]
do
  case "$1" in
    --jobs=*)
      JOBS="${1#*=}";;
    -j*)
      JOBS="${1#*j}";;
    -h|--help)
      echo "Usage: $0 [-j<jobs>|--jobs=<jobs>]"
      echo "Build companion translations using tx16s target configuration"
      echo ""
      echo "Options:"
      echo "  -j<jobs>, --jobs=<jobs>  Number of parallel jobs (default: $num_cpus)"
      echo "  -h, --help              Show this help message"
      exit 0;;
    -*)
      echo >&2 "usage: $0 [-j<jobs>|--jobs=<jobs>]"
      echo >&2 "Use -h or --help for more information"
      exit 1;;
    *)
      echo >&2 "usage: $0 [-j<jobs>|--jobs=<jobs>]"
      echo >&2 "Use -h or --help for more information"
      exit 1;;
  esac
  shift
done

# Project root directory (one level up from tools)
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"

# Set common build options
: "${BUILD_TYPE:=Release}"
: "${COMMON_OPTIONS:="-DCMAKE_BUILD_TYPE=$BUILD_TYPE -DCMAKE_RULE_MESSAGES=OFF -Wno-dev"}"

BUILD_OPTIONS="${COMMON_OPTIONS} "

echo "Building companion translations with tx16s target configuration..."

# Get tx16s target build options from build-common.sh
if ! get_target_build_options "tx16s"; then
    echo "Error: Failed to find a match for target 'tx16s'"
    exit 1
fi

# Ensure we're in the project root
cd "${PROJECT_ROOT}"

# Create and enter build directory
rm -rf build
mkdir build
cd build

echo "Configuring build with options: ${BUILD_OPTIONS}"
cmake ${BUILD_OPTIONS} "${PROJECT_ROOT}"

echo "Configuring native build..."
cmake --build . --target native-configure

echo "Building companion translations with ${JOBS} parallel jobs..."
make -j"${JOBS}" -C native companion_translations
