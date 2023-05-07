#!/bin/bash

# Stops on first error, echo on
set -e
set -x

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
    COMMON_OPTIONS="${COMMON_OPTIONS} -DCMAKE_OSX_DEPLOYMENT_TARGET='10.9'"
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
                              x7 x7-access
                              t8 t12 tx12 tx12mk2
                              zorro commando8 boxer
                              tlite tpro lr3pro
                              x9d x9dp x9dp2019 x9e
                              xlite xlites
                              nv14
                              x10 x10-access x12s
                              t16 t18 tx16s)

for plugin in "${simulator_plugins[@]}"
do
    BUILD_OPTIONS="${COMMON_OPTIONS} "

    echo "Building ${plugin}"
    case $plugin in
        x9lite)
            BUILD_OPTIONS+="-DPCB=X9LITE"
            ;;
        x9lites)
            BUILD_OPTIONS+="-DPCB=X9LITES"
            ;;
        x7)
            BUILD_OPTIONS+="-DPCB=X7"
            ;;
        x7-access)
            BUILD_OPTIONS+="-DPCB=X7 -DPCBREV=ACCESS -DPXX1=YES"
            ;;
        t12)
            BUILD_OPTIONS+="-DPCB=X7 -DPCBREV=T12 -DINTERNAL_MODULE_MULTI=ON"
            ;;
        tx12)
            BUILD_OPTIONS+="-DPCB=X7 -DPCBREV=TX12"
            ;;
        tx12mk2)
            BUILD_OPTIONS+="-DPCB=X7 -DPCBREV=TX12MK2"
            ;;
        t8)
            BUILD_OPTIONS+="-DPCB=X7 -DPCBREV=T8"
            ;;
        zorro)
            BUILD_OPTIONS+="-DPCB=X7 -DPCBREV=ZORRO"
            ;;
        boxer)
            BUILD_OPTIONS+="-DPCB=X7 -DPCBREV=BOXER"
            ;;
        lr3pro)
            BUILD_OPTIONS+="-DPCB=X7 -DPCBREV=LR3PRO"
            ;;
        tlite)
            BUILD_OPTIONS+="-DPCB=X7 -DPCBREV=TLITE"
            ;;
        tlitef4)
            BUILD_OPTIONS+="-DPCB=X7 -DPCBREV=TLITEF4"
            ;;
        tpro)
            BUILD_OPTIONS+="-DPCB=X7 -DPCBREV=TPRO"
            ;;
        xlite)
            BUILD_OPTIONS+="-DPCB=XLITE"
            ;;
        xlites)
            BUILD_OPTIONS+="-DPCB=XLITES"
            ;;
        x9d)
            BUILD_OPTIONS+="-DPCB=X9D"
            ;;
        x9dp)
            BUILD_OPTIONS+="-DPCB=X9D+"
            ;;
        x9dp2019)
            BUILD_OPTIONS+="-DPCB=X9D+ -DPCBREV=2019"
            ;;
        x9e)
            BUILD_OPTIONS+="-DPCB=X9E"
            ;;
        x10)
            BUILD_OPTIONS+="-DPCB=X10"
            ;;
        x10-access)
            BUILD_OPTIONS+="-DPCB=X10 -DPCBREV=EXPRESS -DPXX1=YES"
            ;;
        x12s)
            BUILD_OPTIONS+="-DPCB=X12S"
            ;;
        t16)
            BUILD_OPTIONS+="-DPCB=X10 -DPCBREV=T16 -DINTERNAL_MODULE_MULTI=ON"
            ;;
        t18)
            BUILD_OPTIONS+="-DPCB=X10 -DPCBREV=T18"
            ;;
        tx16s)
            BUILD_OPTIONS+="-DPCB=X10 -DPCBREV=TX16S"
            ;;
        nv14)
            BUILD_OPTIONS+="-DPCB=NV14"
            ;;
        commando8)
            BUILD_OPTIONS+="-DPCB=X7 -DPCBREV=COMMANDO8"
            ;;
        *)
            echo "Unknown target: $target_name"
            exit 1
            ;;
    esac

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
