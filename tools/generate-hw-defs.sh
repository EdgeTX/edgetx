#!/bin/bash

# Stops on first error, echo on
set -e
# set -x

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
. "$SCRIPT_DIR/build-common.sh"

: "${SRCDIR:=$(dirname "$(pwd)/$0")/..}"

: ${FLAVOR:="nv14;el18;t12;t12max;t15;t16;t18;t8;zorro;pocket;commando8;tlite;tpro;tprov2;tpros;bumblebee;t20;t20v2;t14;lr3pro;mt12;gx12;tx12;tx12mk2;boxer;tx16s;x10;x10express;x12s;x7;x7access;x9d;x9dp;x9dp2019;x9e;x9lite;x9lites;xlite;xlites;f16;v14;v12"}
: ${COMMON_OPTIONS:="-DCMAKE_BUILD_TYPE=$BUILD_TYPE -DCMAKE_RULE_MESSAGES=OFF -Wno-dev -DCMAKE_MESSAGE_LOG_LEVEL=WARNING"}

# wipe build directory clean
rm -rf build && mkdir -p build && cd build

target_names=$(echo "$FLAVOR" | tr '[:upper:]' '[:lower:]' | tr ';' '\n')

TARGET_DIR="${SRCDIR}/radio/src/targets/hw_defs"

for target_name in $target_names
do
    BUILD_OPTIONS=${COMMON_OPTIONS}
    BUILD_OPTIONS+=" $EXTRA_OPTIONS "

    echo "Processing ${target_name}"

    if ! get_target_build_options "$target_name"; then
        echo "Error: Failed to find a match for target '$target_name'"
        exit 1
    fi

    cmake ${BUILD_OPTIONS} "${SRCDIR}"
    cmake --build . --target arm-none-eabi-configure
    cmake --build arm-none-eabi --target hardware_defs

    mkdir -p ${TARGET_DIR}
    mv arm-none-eabi/radio/src/*.json* ${TARGET_DIR}
    
    rm -f CMakeCache.txt arm-none-eabi/CMakeCache.txt
done
