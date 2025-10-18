#!/bin/bash

# Stops on first error, echo on
set -e
set -x

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
. "$SCRIPT_DIR/build-common.sh" 

# If no build target, exit
#: "${FLAVOR:=ALL}"

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
: ${COMMON_OPTIONS:="-DCMAKE_BUILD_TYPE=$BUILD_TYPE -Wno-dev "}
if (( $WERROR )); then COMMON_OPTIONS+=" -DWARNINGS_AS_ERRORS=YES "; fi

: ${EXTRA_OPTIONS:="$EXTRA_OPTIONS"}

COMMON_OPTIONS+=${EXTRA_OPTIONS}

: ${FIRMARE_TARGET:="firmware-size"}

# wipe build directory clean
rm -rf build && mkdir -p build && cd build

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

    cmake --build . --target arm-none-eabi-configure --parallel
    cmake --build arm-none-eabi --target ${FIRMARE_TARGET} --parallel

    cmake --build . --target native-configure --parallel
    cmake --build native --target libsimulator --parallel
    cmake --build native --target tests-radio --parallel

    rm -f CMakeCache.txt native/CMakeCache.txt arm-none-eabi/CMakeCache.txt
done
