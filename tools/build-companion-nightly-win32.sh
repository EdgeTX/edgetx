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
      echo >&2 "usage: $0 [-j<jobs>|--jobs=<jobs>] SRCDIR OUTDIR VERSION_SUFFIX"
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
elif [ "$(uname)" != "Linux" ]; then
    COMMON_OPTIONS="${COMMON_OPTIONS} -DSDL_LIBRARY_PATH=/mingw32/bin/"
fi

if [ "$3" != "" ]; then
  COMMON_OPTIONS="${COMMON_OPTIONS} -DVERSION_SUFFIX=$3"
fi

rm -rf build
mkdir build
cd build

cmake ${COMMON_OPTIONS} -DPCB=X9LITE ${SRCDIR}
make -j${JOBS} libsimulator
rm CMakeCache.txt

cmake ${COMMON_OPTIONS} -DPCB=X9LITES ${SRCDIR}
make -j${JOBS} libsimulator
rm CMakeCache.txt

cmake ${COMMON_OPTIONS} -DPCB=X7 ${SRCDIR}
make -j${JOBS} libsimulator
rm CMakeCache.txt

cmake ${COMMON_OPTIONS} -DPCB=X7 -DPCBREV=ACCESS ${SRCDIR}
make -j${JOBS} libsimulator
rm CMakeCache.txt

cmake ${COMMON_OPTIONS} -DPCB=X7 -DPCBREV=T12 ${SRCDIR}
make -j${JOBS} libsimulator
rm CMakeCache.txt

cmake ${COMMON_OPTIONS} -DPCB=X7 -DPCBREV=TX12 ${SRCDIR}
make -j${JOBS} libsimulator
rm CMakeCache.txt

cmake ${COMMON_OPTIONS} -DPCB=X7 -DPCBREV=ZORRO ${SRCDIR}
make -j${JOBS} libsimulator
rm CMakeCache.txt

cmake ${COMMON_OPTIONS} -DPCB=X7 -DPCBREV=T8 ${SRCDIR}
make -j${JOBS} libsimulator
rm CMakeCache.txt

cmake ${COMMON_OPTIONS} -DPCB=X7 -DPCBREV=TLITE ${SRCDIR}
make -j${JOBS} libsimulator
rm CMakeCache.txt

cmake ${COMMON_OPTIONS} -DPCB=X7 -DPCBREV=TPRO ${SRCDIR}
make -j${JOBS} libsimulator
rm CMakeCache.txt

cmake ${COMMON_OPTIONS} -DPCB=X9D ${SRCDIR}
make -j${JOBS} libsimulator
rm CMakeCache.txt

cmake ${COMMON_OPTIONS} -DPCB=X9D+ ${SRCDIR}
make -j${JOBS} libsimulator
rm CMakeCache.txt

cmake ${COMMON_OPTIONS} -DPCB=X9D+ -DPCBREV=2019 ${SRCDIR}
make -j${JOBS} libsimulator
rm CMakeCache.txt

cmake ${COMMON_OPTIONS} -DPCB=XLITE ${SRCDIR}
make -j${JOBS} libsimulator
rm CMakeCache.txt

cmake ${COMMON_OPTIONS} -DPCB=XLITES ${SRCDIR}
make -j${JOBS} libsimulator
rm CMakeCache.txt

cmake ${COMMON_OPTIONS} -DPCB=X9E ${SRCDIR}
make -j${JOBS} libsimulator
rm CMakeCache.txt

cmake ${COMMON_OPTIONS} -DPCB=NV14 ${SRCDIR}
make -j${JOBS} libsimulator
rm CMakeCache.txt

cmake ${COMMON_OPTIONS} -DPCB=PL18 ${SRCDIR}
make -j${JOBS} libsimulator
rm CMakeCache.txt

cmake ${COMMON_OPTIONS} -DPCB=X10 ${SRCDIR}
make -j${JOBS} libsimulator
rm CMakeCache.txt

cmake ${COMMON_OPTIONS} -DPCB=X10 -DPCBREV=T16 -DINTERNAL_MODULE_MULTI=YES ${SRCDIR}
make -j${JOBS} libsimulator
rm CMakeCache.txt

cmake ${COMMON_OPTIONS} -DPCB=X10 -DPCBREV=TX16S ${SRCDIR}
make -j${JOBS} libsimulator
rm CMakeCache.txt

cmake ${COMMON_OPTIONS} -DPCB=X10 -DPCBREV=T18 ${SRCDIR}
make -j${JOBS} libsimulator
rm CMakeCache.txt

cmake ${COMMON_OPTIONS} -DPCB=X10 -DPCBREV=EXPRESS ${SRCDIR}
make -j${JOBS} libsimulator
rm CMakeCache.txt

cmake ${COMMON_OPTIONS} -DPCB=X12S ${SRCDIR}
make -j${JOBS} libsimulator

if [ "$(uname)" = "Darwin" ]; then
    make -j${JOBS} package
    cp *.dmg ${OUTDIR}
elif [ "$(uname)" = "Linux" ]; then
    make -j${JOBS} package
    cp *.AppImage ${OUTDIR}
else
    make installer
    cp companion/*.exe ${OUTDIR}
fi
