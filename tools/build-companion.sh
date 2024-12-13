#!/bin/bash

# Stops on first error, echo on
set -e
set -x

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
. "$SCRIPT_DIR/build-common.sh" 

if [ "$(uname)" = "Darwin" ]; then
  num_cpus=$(sysctl -n hw.ncpu)
  : "${JOBS:=$num_cpus}"
else
  JOBS=3
fi

while [ $# -gt 0 ]
do
  case "$1" in
    --jobs=*)
      JOBS="${1#*=}";;
    -j*)
      JOBS="${1#*j}";;
    -*)
      echo >&2 "usage: $0 [-j<jobs>|--jobs=<jobs>] SRCDIR OUTDIR"
      exit 1;;
    *)
      break;;   # terminate while loop
  esac
  shift
done

SRCDIR=$1
OUTDIR=$2

COMMON_OPTIONS="-DGVARS=YES -DHELI=YES -DLUA=YES -Wno-dev -DCMAKE_BUILD_TYPE=Release"
if [ "$(uname)" = "Darwin" ]; then
    COMMON_OPTIONS="${COMMON_OPTIONS} -DCMAKE_OSX_DEPLOYMENT_TARGET='10.15'"
elif [ "$(uname)" != "Linux" ]; then # Assume Windows and MSYS2
    if [ "${MSYSTEM,,}" == "mingw32" ]; then # MSYS 32bit detected
        COMMON_OPTIONS="${COMMON_OPTIONS} -DSDL2_LIBRARY_PATH=/mingw32/bin/"
    else # fallback to 64bit
        COMMON_OPTIONS="${COMMON_OPTIONS} -DSDL2_LIBRARY_PATH=/mingw64/bin/"
    fi
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

rm -rf build
mkdir build
cd build

declare -a simulator_plugins=(x9lite x9lites
                              x7 x7access
                              t8 t12 t12max tx12 tx12mk2
                              zorro commando8 boxer pocket mt12 gx12
                              tlite tpro tprov2 tpros bumblebee lr3pro t14
                              x9d x9dp x9dp2019 x9e
                              xlite xlites
                              nv14 el18 pl18 pl18ev
                              x10 x10express x12s
                              t15 t16 t18 t20 t20v2 tx16s f16 v16)

for plugin in "${simulator_plugins[@]}"
do
    BUILD_OPTIONS="${COMMON_OPTIONS} "

    echo "Building ${plugin}"
    
    if ! get_target_build_options "$plugin"; then
        echo "Error: Failed to find a match for target '$plugin'"
        exit 1
    fi

    rm -f CMakeCache.txt native/CMakeCache.txt
    cmake ${BUILD_OPTIONS} "${SRCDIR}"
    cmake --build . --target native-configure
    cmake --build native -j"${JOBS}" --target libsimulator
done                              

cmake --build . --target native-configure
if [ "$(uname)" = "Darwin" ]; then
    cmake --build native -j"${JOBS}" --target package
    cp native/*.dmg "${OUTDIR}"
elif [ "$(uname)" = "Linux" ]; then
    cmake --build native -j"${JOBS}" --target package
    cp native/*.AppImage "${OUTDIR}"
else
    cmake --build native --target installer
    cp native/companion/*.exe "${OUTDIR}"
fi
