#! /usr/bin/env bash

## Bash script to show how to get EdgeTX source from GitHub (main branch),
## how to build TX16S firmware, Companion, Simulator, radio simulator
## library and how to create an installation package.
## Let it run as normal user in MSYS2 MinGW 64-bit console (blue icon).

PAUSEAFTEREACHLINE="false"
STEP=1
# Parse argument(s)
for arg in "$@"
do
	if [[ $arg == "--pause" ]]; then
		PAUSEAFTEREACHLINE="true"
	fi
done

echo "=== Step $((STEP++)): Creating a directory for EdgeTX ==="
mkdir ~/edgetx && cd ~/edgetx
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
  echo "Step finished. Please check the output above and press Enter to continue or Ctrl+C to stop."
  read
fi

echo "=== Step $((STEP++)): Fetching EdgeTX source tree (main branch) from GitHub ==="
git clone --recursive -b main https://github.com/EdgeTX/edgetx.git edgetx_main
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
  echo "Step finished. Please check the output above and press Enter to continue or Ctrl+C to stop."
  read
fi

echo "=== Step $((STEP++)): Creating build output directory ==="
cd edgetx_main && mkdir build-output && cd build-output
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
  echo "Step finished. Please check the output above and press Enter to continue or Ctrl+C to stop."
  read
fi

echo "=== Step $((STEP++)): Running CMake for TX16S as an example ==="
cmake -G "MSYS Makefiles" -DCMAKE_PREFIX_PATH=$HOME/5.12.9/mingw73_64 -DSDL_LIBRARY_PATH=/mingw64/bin/ -DPCB=X10 -DPCBREV=TX16S -DDEFAULT_MODE=2 -DGVARS=YES -DPPM_UNIT=US -DHELI=NO -DLUA=YES -DINTERNAL_GPS=YES -DCMAKE_BUILD_TYPE=Release ../
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
  echo "Step finished. Please check the output above and press Enter to continue or Ctrl+C to stop."
  read
fi

echo "=== Step $((STEP++)): Building firmware binary ==="
make -j`nproc` firmware
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
  echo "Step finished. Please check the output above and press Enter to continue or Ctrl+C to stop."
  read
fi

echo "=== Step $((STEP++)): Renaming firmware binary ==="
mv firmware.bin fw_edgetx_main_tx16s_lua-ppmus-mode2-gps_release.bin
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
  echo "Step finished. Please check the output above and press Enter to continue or Ctrl+C to stop."
  read
fi

echo "=== Step $((STEP++)): Building Companion ==="
make -j`nproc` companion
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
  echo "Step finished. Please check the output above and press Enter to continue or Ctrl+C to stop."
  read
fi

echo "=== Step $((STEP++)): Building Simulator ==="
make -j`nproc` simulator
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
  echo "Step finished. Please check the output above and press Enter to continue or Ctrl+C to stop."
  read
fi

echo "=== Step $((STEP++)): Building radio simulator library ==="
make -j`nproc` libsimulator
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
  echo "Step finished. Please check the output above and press Enter to continue or Ctrl+C to stop."
  read
fi

echo "=== Step $((STEP++)): Making an installer ==="
make installer
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
  echo "Step finished. Please check the output above and press Enter to continue or Ctrl+C to stop."
fi

echo "Finished. You can find the TX16S firmware in ~/edgetx/edgetx_main/build-output"
echo "and Companion installer packet in ~/edgetx/edgetx_main/build-output/companion"
