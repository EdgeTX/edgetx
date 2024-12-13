#!/bin/bash

# Stops on first error
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
. "$SCRIPT_DIR/build-common.sh"

# Add GCC_ARM to PATH
if [[ -n ${GCC_ARM} ]] ; then
  export PATH=${GCC_ARM}:$PATH
fi

: ${FLAVOR:="t15;tx16s;nv14;pl18;x9d;x9dp2019;x9e;xlite;xlites;x7;tpro;t20;f16;gx12"}
: ${SRCDIR:=$(dirname "$(pwd)/$0")/..}

: ${COMMON_OPTIONS:="-DCMAKE_BUILD_TYPE=$BUILD_TYPE -DCMAKE_RULE_MESSAGES=OFF -Wno-dev -DDISABLE_COMPANION=YES -DCMAKE_MESSAGE_LOG_LEVEL=WARNING"}

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
    make native-configure
    make -C native yaml_data

    rm -f CMakeCache.txt arm-none-eabi/CMakeCache.txt
done
