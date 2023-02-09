#! /usr/bin/env bash

## Bash script to show how to get EdgeTX source from GitHub,
## how to build TX16S firmware, Companion, Simulator, radio simulator
## library and how to create an installation package.
## Let it run as normal user in MSYS2 MinGW 64-bit console (blue icon).

# -----------------------------------------------------------------------------
export BRANCH_NAME="main"  # main|2.8|2.9|...
export RADIO_TYPE="TX16S"  # TX16S|X10|X10EXPRESS|X12S|XLITE|XLITES|X9LITE|X9LITES|X9D|X9D+|X9D+2019|X9E|X7|X7ACCESS|T16|T18||T12|TX12|COMMANDO8
PAUSEAFTEREACHLINE="false" # true|false
# -----------------------------------------------------------------------------



export SOURCE_DIR="edgetx_${BRANCH_NAME}"

function log() {
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

# Parse argument(s)
for arg in "$@"
do
	if [[ $arg == "--pause" ]]; then
		PAUSEAFTEREACHLINE="true"
	fi
done

STEP=1
echo "=== Step $((STEP++)): Creating a directory for EdgeTX ==="
mkdir -p ~/edgetx && cd ~/edgetx
check_command $? "mkdir -p ~/edgetx && cd ~/edgetx"
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
  echo "Step finished. Please check the output above and press Enter to continue or Ctrl+C to stop."
  read
fi


if [ ! -d "${SOURCE_DIR}" ]; then
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
fi
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
  echo "Step finished. Please check the output above and press Enter to continue or Ctrl+C to stop."
  read
fi

echo "=== Step $((STEP++)): Creating build output directory ==="
mkdir -p build-output && cd build-output
check_command $? "mkdir -p build-output && cd build-output"
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
  echo "Step finished. Please check the output above and press Enter to continue or Ctrl+C to stop."
  read
fi

echo "=== Step $((STEP++)): Running CMake for ${RADIO_TYPE} as an example ==="
cmake -G "MSYS Makefiles" -Wno-dev -DCMAKE_PREFIX_PATH=$HOME/5.12.9/mingw73_64 -DSDL_LIBRARY_PATH=/mingw64/bin/ -DPCB=X10 -DPCBREV=${RADIO_TYPE} -DDEFAULT_MODE=2 -DGVARS=YES -DPPM_UNIT=US -DHELI=NO -DLUA=YES -DCMAKE_BUILD_TYPE=Release ../
check_command $? "cmake -G MSYS Makefiles -Wno-dev -DCMAKE_PREFIX_PATH=$HOME/5.12.9/mingw73_64 -DSDL_LIBRARY_PATH=/mingw64/bin/ -DPCB=X10 -DPCBREV=${RADIO_TYPE} -DDEFAULT_MODE=2 -DGVARS=YES -DPPM_UNIT=US -DHELI=NO -DLUA=YES -DCMAKE_BUILD_TYPE=Release ../"
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
mv arm-none-eabi/firmware.bin arm-none-eabi/fw_${SOURCE_DIR}_${RADIO_TYPE}_lua-ppmus-mode2_release.bin
check_command $? "mv arm-none-eabi/firmware.bin arm-none-eabi/fw_${SOURCE_DIR}_${RADIO_TYPE}_lua-ppmus-mode2_release.bin"
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

echo "=== Step $((STEP++)): Building radio simulator library ==="
make -C native -j`nproc` libsimulator
check_command $? "make -C native -j`nproc` libsimulator"
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

echo "Finished. You can find the ${RADIO_TYPE} firmware in ~/edgetx/${SOURCE_DIR}/build-output/arm-none-eabi"
echo "and Companion installer packet in ~/edgetx/${SOURCE_DIR}/build-output/native/companion"
