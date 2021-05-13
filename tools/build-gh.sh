#!/bin/bash

# Stops on first error, echo on
set -e
set -x

# Allow variable core usage, default uses all cpu cores, to set 8 cores for example : commit-tests.sh -j8
num_cpus=$(nproc)
: "${CORES:=$num_cpus}"
# Default build treats warnings as errors, set -Wno-error to override, e.g.: commit-tests.sh -Wno-error
#: "${WERROR:=1}"
# A board name to build for, or ALL
: "${FLAVOR:=ALL}"

for i in "$@"
do
case $i in
    --jobs=*)
      CORES="${i#*=}"
      shift
      ;;
    -j*)
      CORES="${i#*j}"
      shift
      ;;
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

if [ "$(uname)" = "Darwin" ]; then
    SCRIPT=$(python -c 'import os,sys;print(os.path.realpath(sys.argv[1]))' "$0")
else
    SCRIPT=$(readlink -f "$0")
fi

if [[ -n ${GCC_ARM} ]] ; then
  export PATH=${GCC_ARM}:$PATH
fi

: ${SRCDIR:=$(dirname "$SCRIPT")/..}

: ${COMMON_OPTIONS:="-DCMAKE_BUILD_TYPE=Debug -DTRACE_SIMPGMSPACE=NO -DVERBOSE_CMAKELISTS=YES -DCMAKE_RULE_MESSAGES=OFF -Wno-dev "}
if (( $WERROR )); then COMMON_OPTIONS+=" -DWARNINGS_AS_ERRORS=YES -DMULTIMODULE=YES"; fi

: ${EXTRA_OPTIONS:=" $EXTRA_OPTIONS "}

COMMON_OPTIONS+=${EXTRA_OPTIONS}

: ${FIRMARE_TARGET:="firmware-size"}

mkdir build || true
cd build

# wipe clean in case of serial builds in the same dir
rm -rf ./* || true

BUILD_OPTIONS=${COMMON_OPTIONS}

GIT_SHA_SHORT=`git rev-parse --short HEAD`
#GIT_TAG=`git describe --tags`

target_name=`echo "$FLAVOR" | tr '[:upper:]' '[:lower:]'`
fw_name="${target_name}-${GIT_SHA_SHORT}.bin"

echo "Building ${fw_name}"

case $FLAVOR in

    X9LITE)
        BUILD_OPTIONS+=" -DPCB=X9LITE"
        ;;
    X9LITES)
        BUILD_OPTIONS+=" -DPCB=X9LITES"
        ;;
    X7)
        BUILD_OPTIONS+=" -DPCB=X7"
        ;;
    T12)
        BUILD_OPTIONS+=" -DPCB=X7 -DPCBREV=T12 -DINTERNAL_MODULE_MULTI=ON"
        ;;
    TX12)
        BUILD_OPTIONS+=" -DPCB=X7 -DPCBREV=TX12"
        ;;
    T8)
        BUILD_OPTIONS+=" -DPCB=X7 -DPCBREV=T8"
        ;;
    TLITE)
        BUILD_OPTIONS+=" -DPCB=X7 -DPCBREV=TLITE"
        ;;
    XLITE)
        BUILD_OPTIONS+=" -DPCB=XLITE"
        ;;
    XLITES)
        BUILD_OPTIONS+=" -DPCB=XLITES"
        ;;
    X9DP)
        BUILD_OPTIONS+=" -DPCB=X9D+"
        ;;
    X9DP2019)
        BUILD_OPTIONS+=" -DPCB=X9D+ -DPCBREV=2019"
        ;;
    X9E)
        BUILD_OPTIONS+=" -DPCB=X9E"
        ;;
    X10)
        BUILD_OPTIONS+=" -DPCB=X10"
        ;;
    X12S)
        BUILD_OPTIONS+=" -DPCB=X12S"
        ;;
    T16)
        BUILD_OPTIONS+=" -DPCB=X10 -DPCBREV=T16"
        ;;
    T18)
        BUILD_OPTIONS+="-DPCB=X10 -DPCBREV=T18"
        ;;
    TX16S)
        BUILD_OPTIONS+=" -DPCB=X10 -DPCBREV=TX16S"
        ;;
esac

cmake ${BUILD_OPTIONS} "${SRCDIR}"
make -j"${CORES}" ${FIRMARE_TARGET}

mv firmware.bin "../${fw_name}"

