#! /usr/bin/env bash

## Bash script to setup EdgeTX development environment second stage.
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

echo "=== Step $((STEP++)): Installing packages ==="
pacman -S --noconfirm mingw-w64-x86_64-cmake mingw-w64-x86_64-python-pip mingw-w64-x86_64-arm-none-eabi-gcc mingw-w64-x86_64-libjpeg-turbo mingw-w64-x86_64-zlib mingw-w64-x86_64-libtiff mingw-w64-x86_64-freetype mingw-w64-x86_64-lcms2 mingw-w64-x86_64-libwebp mingw-w64-x86_64-openjpeg2 mingw-w64-x86_64-libimagequant mingw-w64-x86_64-libraqm mingw-w64-x86_64-SDL mingw-w64-x86_64-nsis
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
  echo "Step finished. Please check the output above and press Enter to continue or Ctrl+C to stop."
  read
fi

echo "=== Step $((STEP++)): Installing Python wheel ==="
pip install wheel
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
  echo "Step finished. Please check the output above and press Enter to continue or Ctrl+C to stop."
  read
fi

echo "=== Step $((STEP++)): Fetching Python package requirements ==="
wget https://raw.githubusercontent.com/EdgeTX/edgetx/main/tools/setup_buildenv_msys2_py_requirements.txt
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
  echo "Step finished. Please check the output above and press Enter to continue or Ctrl+C to stop."
  read
fi

echo "=== Step $((STEP++)): Installing Python packages ==="
pip install -r setup_buildenv_msys2_py_requirements.txt
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
  echo "Step finished. Please check the output above and press Enter to continue or Ctrl+C to stop."
  read
fi

echo "=== Step $((STEP++)): Deleting Python package requirements file ==="
rm setup_buildenv_msys2_py_requirements.txt
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
  echo "Step finished. Please check the output above and press Enter to continue or Ctrl+C to stop."
  read
fi

echo "=== Step $((STEP++)): Installing Qt build environment ==="
aqt install 5.12.9 windows desktop win64_mingw73
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
  echo "Step finished. Please check the output above and press Enter to continue or Ctrl+C to stop."
fi

echo "Setting up build environment has finished. You can now start building EdgeTX firmware and binaries!"
