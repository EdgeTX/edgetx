#!/bin/bash

# Stop on first error, echo on
set -e
set -x

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
. "$SCRIPT_DIR/build-common.sh" 

for i in "$@"
do
case $i in
    -Wno-error)
      WERROR=0
      shift
      ;;
    -b*)
      FLAVOR="${i#*b}"
      shift
      ;;
esac
done

# Add GCC_ARM to PATH
if [[ -n ${GCC_ARM} ]] ; then
  export PATH=${GCC_ARM}:$PATH
fi

: ${SRCDIR:=$(dirname "$(pwd)/$0")/..}

: ${BUILD_TYPE:=Debug}
: ${COMMON_OPTIONS:="-DCMAKE_BUILD_TYPE=$BUILD_TYPE ${CMAKE_NO_AUTHOR_WARNINGS} "}
if (( $WERROR )); then COMMON_OPTIONS+=" -DWARNINGS_AS_ERRORS=YES "; fi

: ${EXTRA_OPTIONS:="$EXTRA_OPTIONS"}

COMMON_OPTIONS+=${EXTRA_OPTIONS}" "

: ${FIRMARE_TARGET:="firmware-size"}

# Determine parallel jobs
determine_max_jobs

# Scratch space under build/ - other build trees there are left alone
BUILD_DIR=$(reset_scratch_build_dir ci-tests) || exit 1
cd "${BUILD_DIR}"

target_names=$(echo "$FLAVOR" | tr '[:upper:]' '[:lower:]' | tr ';' '\n')

for target_name in $target_names
do
    BUILD_OPTIONS=${COMMON_OPTIONS}

    echo "Testing ${target_name}"

    if ! get_target_build_options "$target_name"; then
        echo "Error: Failed to find a match for target '$target_name'"
        exit 1
    fi

    cmake ${BUILD_OPTIONS} "${SRCDIR}"

    cmake_build_parallel . --target native-configure
    cmake_build_parallel native --target tests-radio

    rm -f CMakeCache.txt native/CMakeCache.txt
done
