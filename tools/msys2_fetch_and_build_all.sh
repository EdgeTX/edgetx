#! /usr/bin/env bash

## Bash script to show how to get EdgeTX source from GitHub,
## how to build TX16S firmware, Companion, Simulator, radio simulator
## library and how to create an installation package.
## Let it run as normal user in MSYS2 MinGW 64-bit console (blue icon).
##
## Note: This script is tested to work properly only for the branch it stems from.

# -----------------------------------------------------------------------------
export BRANCH_NAME="main"  # main|2.9|...
export RADIO_TYPE="tx16s"  # tx16s|x10|x10express|x12s|x9d|x9dp|x9lite|x9lites|x7|x7access|t12|t12max|tx12|tx12mk2|mt12|gx12|boxer|t8|zorro|pocket|tlite|tpro|t20|t20v2|t14|lr3pro|xlite|xlites|x9dp2019|x9e|x9e-hall|t15|t16|t18|nv14|commando8

export BUILD_OPTIONS="-DDEFAULT_MODE=2 -DGVARS=YES"

echo "Building ${fw_name}"
case $RADIO_TYPE in
    x9lite)
        BUILD_OPTIONS+=" -DPCB=X9LITE"
        ;;
    x9lites)
        BUILD_OPTIONS+=" -DPCB=X9LITES"
        ;;
    x7)
        BUILD_OPTIONS+=" -DPCB=X7"
        ;;
    x7access)
        BUILD_OPTIONS+=" -DPCB=X7 -DPCBREV=ACCESS -DPXX1=YES"
        ;;
    t12)
        BUILD_OPTIONS+=" -DPCB=X7 -DPCBREV=T12 -DINTERNAL_MODULE_MULTI=ON"
        ;;
    mt12)
        BUILD_OPTIONS+=" -DPCB=X7 -DPCBREV=MT12 -DINTERNAL_MODULE_MULTI=ON"
        ;;
    gx12)
        BUILD_OPTIONS+=" -DPCB=X7 -DPCBREV=GX12 -DINTERNAL_MODULE_MULTI=ON"
        ;;
    tx12)
        BUILD_OPTIONS+=" -DPCB=X7 -DPCBREV=TX12"
        ;;
    tx12mk2)
        BUILD_OPTIONS+=" -DPCB=X7 -DPCBREV=TX12MK2"
        ;;
    boxer)
        BUILD_OPTIONS+=" -DPCB=X7 -DPCBREV=BOXER"
        ;;
    t8)
        BUILD_OPTIONS+=" -DPCB=X7 -DPCBREV=T8"
        ;;
    zorro)
        BUILD_OPTIONS+=" -DPCB=X7 -DPCBREV=ZORRO"
        ;;
    pocket)
        BUILD_OPTIONS+=" -DPCB=X7 -DPCBREV=POCKET"
        ;;
    tlite)
        BUILD_OPTIONS+=" -DPCB=X7 -DPCBREV=TLITE"
        ;;
    tpro)
        BUILD_OPTIONS+=" -DPCB=X7 -DPCBREV=TPRO"
        ;;
    t20)
        BUILD_OPTIONS+=" -DPCB=X7 -DPCBREV=T20"
        ;;
    t20v2)
        BUILD_OPTIONS+=" -DPCB=X7 -DPCBREV=T20V2"
        ;;
    t12max)
        BUILD_OPTIONS+=" -DPCB=X7 -DPCBREV=T12MAX"
        ;;
    t14)
        BUILD_OPTIONS+=" -DPCB=X7 -DPCBREV=T14"
        ;;
    lr3pro)
        BUILD_OPTIONS+=" -DPCB=X7 -DPCBREV=LR3PRO"
        ;;
    xlite)
        BUILD_OPTIONS+=" -DPCB=XLITE"
        ;;
    xlites)
        BUILD_OPTIONS+=" -DPCB=XLITES"
        ;;
    x9d)
        BUILD_OPTIONS+=" -DPCB=X9D"
        ;;
    x9dp)
        BUILD_OPTIONS+=" -DPCB=X9D+"
        ;;
    x9dp2019)
        BUILD_OPTIONS+=" -DPCB=X9D+ -DPCBREV=2019"
        ;;
    x9e)
        BUILD_OPTIONS+=" -DPCB=X9E"
        ;;
    x9e-hall)
        BUILD_OPTIONS+=" -DPCB=X9E -DSTICKS=HORUS"
        ;;
    x10)
        BUILD_OPTIONS+=" -DPCB=X10"
        ;;
    x10express)
        BUILD_OPTIONS+=" -DPCB=X10 -DPCBREV=EXPRESS -DPXX1=YES"
        ;;
    x12s)
        BUILD_OPTIONS+=" -DPCB=X12S"
        ;;
    t15)
        BUILD_OPTIONS+=" -DPCB=X10 -DPCBREV=T15 -DINTERNAL_MODULE_CRSF=ON"
        ;;
    t16)
        BUILD_OPTIONS+=" -DPCB=X10 -DPCBREV=T16 -DINTERNAL_MODULE_MULTI=ON"
        ;;
    t18)
        BUILD_OPTIONS+=" -DPCB=X10 -DPCBREV=T18"
        ;;
    tx16s)
        BUILD_OPTIONS+=" -DPCB=X10 -DPCBREV=TX16S"
        ;;
    nv14)
        BUILD_OPTIONS+=" -DPCB=NV14"
        ;;
    commando8)
        BUILD_OPTIONS+=" -DPCB=X7 -DPCBREV=COMMANDO8"
        ;;
    *)
        echo "Unknown target: $RADIO_TYPE"
        exit 1
        ;;
esac

PAUSEAFTEREACHLINE="false" # true|false
# -----------------------------------------------------------------------------

export PROJ_DIR="${HOME}/edgetx"
export SOURCE_DIR="${HOME}/edgetx/edgetx_${BRANCH_NAME}"
export BUILD_OUTPUT_DIR="${SOURCE_DIR}/build-output-${RADIO_TYPE}"

function log() {
    echo ""
    echo "=== [INFO] $*"
}
function fail() {
    echo "=== [ERROR] $*"
    exit 1
}
function check_command() {
    result=$1
    cli_info=$2
    if [[ $result -ne 0 ]]; then
        fail "${cli_info} (exit-code=$result)"
    else
        log "${cli_info} - OK"
        return 0
    fi
}

echo "RADIO_TYPE: ${RADIO_TYPE}"
echo "BRANCH_NAME: ${BRANCH_NAME}"
echo "SOURCE_DIR: ${SOURCE_DIR}"
echo "BUILD_OUTPUT_DIR: ${BUILD_OUTPUT_DIR}"
echo "BUILD_OPTIONS: ${BUILD_OPTIONS}"

# Parse argument(s)
for arg in "$@"
do
	if [[ $arg == "--pause" ]]; then
		PAUSEAFTEREACHLINE="true"
	fi
done

STEP=1
echo "=== Step $((STEP++)): Creating a directory for EdgeTX ==="
mkdir -p ${PROJ_DIR} && cd ${PROJ_DIR}
check_command $? "mkdir -p ${PROJ_DIR} && cd ${PROJ_DIR}"
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
  echo "Step finished. Please check the output above and press Enter to continue or Ctrl+C to stop."
  read
fi

if [[ ! -d "${SOURCE_DIR}" ]]; then
  echo "=== Step $((STEP++)): Fetching EdgeTX source tree (${BRANCH_NAME} branch) from GitHub ==="
  git clone --recursive -b ${BRANCH_NAME} https://github.com/EdgeTX/edgetx.git ${SOURCE_DIR}
  check_command $? "git clone --recursive -b ${BRANCH_NAME} https://github.com/EdgeTX/edgetx.git ${SOURCE_DIR}"
  cd ${SOURCE_DIR}
else
  echo "=== Step $((STEP++)): updating EdgeTX source tree (${BRANCH_NAME} branch) from GitHub ==="
  cd ${SOURCE_DIR}
  check_command $? "cd ${SOURCE_DIR}"
  git checkout ${BRANCH_NAME}
  check_command $? "git checkout ${BRANCH_NAME}"
  git pull
  check_command $? "git pull"
  git submodule update --init --recursive
  check_command $? "git submodule update --init --recursive"
fi
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
  echo "Step finished. Please check the output above and press Enter to continue or Ctrl+C to stop."
  read
fi

echo "=== Step $((STEP++)): Creating build output directory ==="
[[ -d ${BUILD_OUTPUT_DIR} ]] && rm -rf ${BUILD_OUTPUT_DIR}
mkdir -p ${BUILD_OUTPUT_DIR} && cd ${BUILD_OUTPUT_DIR}
check_command $? "mkdir -p ${BUILD_OUTPUT_DIR} && cd ${BUILD_OUTPUT_DIR}"
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
  echo "Step finished. Please check the output above and press Enter to continue or Ctrl+C to stop."
  read
fi

echo "=== Step $((STEP++)): Running CMake for ${RADIO_TYPE} as an example ==="
cmake -G "MSYS Makefiles" -Wno-dev -DCMAKE_PREFIX_PATH=$HOME/5.12.9/mingw73_64 -DSDL2_LIBRARY_PATH=/mingw64/bin/ ${BUILD_OPTIONS} -DCMAKE_BUILD_TYPE=Release ../
check_command $? "cmake -G MSYS Makefiles -Wno-dev -DCMAKE_PREFIX_PATH=$HOME/5.12.9/mingw73_64 -DSDL2_LIBRARY_PATH=/mingw64/bin/ ${BUILD_OPTIONS} -DCMAKE_BUILD_TYPE=Release ../"
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
  echo "Step finished. Please check the output above and press Enter to continue or Ctrl+C to stop."
  read
fi

echo "=== Step $((STEP++)): Running Make configure ==="
make configure
check_command $? "make configure"
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
  echo "Step finished. Please check the output above and press Enter to continue or Ctrl+C to stop."
  read
fi

echo "=== Step $((STEP++)): Building firmware binary ==="
make -C arm-none-eabi -j`nproc` firmware
check_command $? "make -C arm-none-eabi -j`nproc` firmware"
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
  echo "Step finished. Please check the output above and press Enter to continue or Ctrl+C to stop."
  read
fi

echo "=== Step $((STEP++)): Renaming firmware binary ==="
mv arm-none-eabi/firmware.bin arm-none-eabi/fw_${RADIO_TYPE}_release.bin
check_command $? "mv arm-none-eabi/firmware.bin arm-none-eabi/fw_${RADIO_TYPE}_release.bin"
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
  echo "Step finished. Please check the output above and press Enter to continue or Ctrl+C to stop."
  read
fi

echo "=== Step $((STEP++)): Building radio simulator library ==="
make -C native -j`nproc` libsimulator
check_command $? "make -C native -j`nproc` libsimulator"
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
  echo "Step finished. Please check the output above and press Enter to continue or Ctrl+C to stop."
  read
fi

echo "=== Step $((STEP++)): Building Companion ==="
make -C native -j`nproc` companion
check_command $? "make -C native -j`nproc` companion"
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
  echo "Step finished. Please check the output above and press Enter to continue or Ctrl+C to stop."
  read
fi

echo "=== Step $((STEP++)): Building Simulator ==="
make -C native -j`nproc` simulator
check_command $? "make -C native -j`nproc` simulator"
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
  echo "Step finished. Please check the output above and press Enter to continue or Ctrl+C to stop."
  read
fi

echo "=== Step $((STEP++)): Making an installer ==="
make -C native installer
check_command $? "make -C native installer"
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
  echo "Step finished. Please check the output above and press Enter to continue or Ctrl+C to stop."
fi

echo "Done. \n\n"
echo "RADIO_TYPE=${RADIO_TYPE}"
echo ""
echo "firmware (${RADIO_TYPE}): ${BUILD_OUTPUT_DIR}/arm-none-eabi/fw_${RADIO_TYPE}_release.bin"
echo "Companion installer: ${BUILD_OUTPUT_DIR}/native/companion/companion-windows-x.x.x.exe"
echo "Companion          : ${BUILD_OUTPUT_DIR}/native/Release/companion.exe"
echo "Simulator          : ${BUILD_OUTPUT_DIR}/native/Release/simulator.exe"
echo "Simulator library  : ${BUILD_OUTPUT_DIR}/native/Release/libedgetx-${RADIO_TYPE}-simulator.dll"
