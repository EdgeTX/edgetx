#!/bin/bash

# Stops on first error
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
. "$SCRIPT_DIR/build-common.sh"

# Add GCC_ARM to PATH
if [[ -n ${GCC_ARM} ]] ; then
  export PATH=${GCC_ARM}:$PATH
fi

: ${FLAVOR:="t15;tx16s;pl18;nv14;pl18u;nb4p;x9d;x9dp2019;x9e;xlite;xlites;x7;tpro;t20;f16;gx12;st16"}
: ${SRCDIR:=$(dirname "$(pwd)/$0")/..}

: ${COMMON_OPTIONS:="-DDISABLE_COMPANION=y -DDISABLE_SIMULATOR=y -DDISABLE_RADIO=y -DCMAKE_MESSAGE_LOG_LEVEL=WARNING"}

# wipe build directory clean
rm -rf build && mkdir -p build && cd build

target_names=$(echo "$FLAVOR" | tr '[:upper:]' '[:lower:]' | tr ';' '\n')

for target_name in $target_names
do
    BUILD_OPTIONS=${COMMON_OPTIONS}
    BUILD_OPTIONS+=" $EXTRA_OPTIONS "

    echo "Generating YAML structures for ${target_name}"

    if ! get_target_build_options "$target_name"; then
        echo "Error: Failed to find a match for target '$target_name'"
        exit 1
    fi

    cmake ${BUILD_OPTIONS} "${SRCDIR}"
    cmake --build . --target native-configure
    cmake --build native --target yaml_data

    rm -f CMakeCache.txt native/CMakeCache.txt
done
