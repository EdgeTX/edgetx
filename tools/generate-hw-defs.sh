#!/bin/bash

# Stops on first error, echo on
set -e
# set -x

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
. "$SCRIPT_DIR/build-common.sh"

: ${SRCDIR:="$(dirname "$SCRIPT_DIR")"}

: ${FLAVOR:="nv14;el18;pl18;pl18ev;pl18u;nb4p;st16;c14;pa01;t12;t12max;t15;t15pro;t15h7;t18h7;t22;t16;t18;t8;zorro;pocket;commando8;tlite;tpro;tprov2;tpros;bumblebee;t20;t20v2;t14;lr3pro;mt12;gx12;gx10;tx12;tx12mk2;boxer;tx16s;tx16smk3;x10;x10express;x12s;x7;x7access;x9d;x9dp;x9dp2019;x9e;x9lite;x9lites;xlite;xlites;f16;v16;v14;v14lcd;v12;v15;tx15;gx15;h17"}
: ${COMMON_OPTIONS:="-DCMAKE_BUILD_TYPE=$BUILD_TYPE -DCMAKE_RULE_MESSAGES=OFF ${CMAKE_NO_AUTHOR_WARNINGS} -DCMAKE_MESSAGE_LOG_LEVEL=WARNING"}

# Scratch space under build/ - other build trees there are left alone
GEN_ROOT=$(reset_scratch_build_dir gen-hw-defs) || exit 1

target_names=$(echo "$FLAVOR" | tr '[:upper:]' '[:lower:]' | tr ';' '\n')

for target_name in $target_names
do
    BUILD_OPTIONS=${COMMON_OPTIONS}
    BUILD_OPTIONS+=" $EXTRA_OPTIONS "

    echo "Processing ${target_name}"

    if ! get_target_build_options "$target_name"; then
        echo "Error: Failed to find a match for target '$target_name'"
        exit 1
    fi

    # Own build tree per target - a shared one leaks cached options between them
    BUILD_DIR="${GEN_ROOT}/${target_name}"

    cmake ${BUILD_OPTIONS} -S "${SRCDIR}" -B "${BUILD_DIR}"
    cmake --build "${BUILD_DIR}" --target arm-none-eabi-configure
    cmake --build "${BUILD_DIR}/arm-none-eabi" --target hardware_defs

    cmake -E rm -rf "${BUILD_DIR}"
done

cmake -E rm -rf "${GEN_ROOT}"
