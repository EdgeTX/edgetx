#!/bin/bash

# Stops on first error
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
. "$SCRIPT_DIR/build-common.sh"

# Add GCC_ARM to PATH
if [[ -n ${GCC_ARM} ]] ; then
  export PATH=${GCC_ARM}:$PATH
fi

: ${FLAVOR:="x10;t15;pl18;nv14;pl18u;nb4p;x9d;x9dp2019;x9e;xlite;xlites;x7;tpro;t20;f16;gx12;st16;c14;pa01;tx15;gx15;t15pro;tx16smk3;t22;v12"}
: ${SRCDIR:="$(dirname "$SCRIPT_DIR")"}

: ${COMMON_OPTIONS:="-DCMAKE_BUILD_TYPE=$BUILD_TYPE -DCMAKE_RULE_MESSAGES=OFF -Wno-dev -DDISABLE_COMPANION=YES -DCMAKE_MESSAGE_LOG_LEVEL=WARNING"}

# wipe build directory clean
cmake -E rm -rf build

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

    # Own build tree per target - a shared one leaks cached options between them
    BUILD_DIR="build/${target_name}"

    cmake ${BUILD_OPTIONS} -S "${SRCDIR}" -B "${BUILD_DIR}"
    cmake --build "${BUILD_DIR}" --target native-configure
    cmake --build "${BUILD_DIR}/native" --target yaml_data

    cmake -E rm -rf "${BUILD_DIR}"
done
