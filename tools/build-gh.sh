#!/bin/bash

# Stops on first error, echo on
set -e
set -x

# Allow variable core usage
# default uses all cpu cores
#
if [ -f /usr/bin/nproc ]; then
    num_cpus=$(nproc)
elif [ -f /usr/sbin/sysctl ]; then
    num_cpus=$(sysctl -n hw.logicalcpu)
else
    num_cpus=2
fi
: "${CORES:=$num_cpus}"

# If no build target, exit
#: "${FLAVOR:=ALL}"

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

# Add GCC_ARM to PATH
if [[ -n ${GCC_ARM} ]] ; then
  export PATH=${GCC_ARM}:$PATH
fi

: "${SRCDIR:=$(dirname "$(pwd)/$0")/..}"

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

: "${BUILD_TYPE:=Release}"
: "${COMMON_OPTIONS:="-DCMAKE_BUILD_TYPE=$BUILD_TYPE -DCMAKE_RULE_MESSAGES=OFF -Wno-dev "}"
: "${EXTRA_OPTIONS:="$EXTRA_OPTIONS"}"

COMMON_OPTIONS+=${EXTRA_OPTIONS}

: "${FIRMARE_TARGET:="firmware-size"}"

# workaround for GH repo owner
git config --global --add safe.directory "$(pwd)"

# wipe build directory clean
rm -rf build && mkdir -p build && cd build

GIT_SHA_SHORT=$(git rev-parse --short HEAD)

target_names=$(echo "$FLAVOR" | tr '[:upper:]' '[:lower:]' | tr ';' '\n')

for target_name in $target_names
do
    fw_name="${target_name}-${GIT_SHA_SHORT}.bin"
    BUILD_OPTIONS=${COMMON_OPTIONS}

    echo "Building ${fw_name}"
    case $target_name in

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
        tlite)
            BUILD_OPTIONS+="-DPCB=X7 -DPCBREV=TLITE"
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
        x9e-hall)
            BUILD_OPTIONS+="-DPCB=X9E -DSTICKS=HORUS"
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
    esac

    cmake ${BUILD_OPTIONS} "${SRCDIR}"
    cmake --build . --target arm-none-eabi-configure
    cmake --build arm-none-eabi -j"${CORES}" --target ${FIRMARE_TARGET}

    rm -f CMakeCache.txt arm-none-eabi/CMakeCache.txt
    mv arm-none-eabi/firmware.bin "../${fw_name}"
done
