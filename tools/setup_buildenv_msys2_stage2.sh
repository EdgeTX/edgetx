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
pacman -S --noconfirm mingw-w64-x86_64-cmake mingw-w64-x86_64-python-pip mingw-w64-x86_64-python-pillow mingw-w64-x86_64-python-lz4 mingw-w64-x86_64-libjpeg-turbo mingw-w64-x86_64-zlib mingw-w64-x86_64-libtiff mingw-w64-x86_64-freetype mingw-w64-x86_64-lcms2 mingw-w64-x86_64-libwebp mingw-w64-x86_64-openjpeg2 mingw-w64-x86_64-libimagequant mingw-w64-x86_64-libraqm mingw-w64-x86_64-SDL2 mingw-w64-x86_64-nsis mingw-w64-x86_64-clang mingw-w64-x86_64-openssl unzip wget
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
  echo "Step finished. Please check the output above and press Enter to continue or Ctrl+C to stop."
  read
fi

echo "=== Step $((STEP++)): Downloading ARM gcc toolchain ==="
wget https://developer.arm.com/-/media/Files/downloads/gnu/13.2.rel1/binrel/arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-arm-none-eabi.zip
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
  echo "Step finished. Please check the output above and press Enter to continue or Ctrl+C to stop."
  read
fi

echo "=== Step $((STEP++)): Installing ARM gcc toolchain ==="
unzip arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-arm-none-eabi.zip
rm arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-arm-none-eabi.zip
mv arm-gnu-toolchain-13.2.Rel1-mingw-w64-i686-arm-none-eabi /opt/gcc-arm-none-eabi
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
  echo "Step finished. Please check the output above and press Enter to continue or Ctrl+C to stop."
  read
fi

echo "=== Step $((STEP++)): Adding GNU Arm Embedded Toolchain to PATH of current user ==="
echo 'export PATH="/opt/gcc-arm-none-eabi/bin:$PATH"' >> ~/.bashrc
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
  echo "Step finished. Please press Enter to continue or Ctrl+C to stop."
  read
fi

echo "=== Step $((STEP++)): Installing Python clang ==="
python -m pip install clang
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
  echo "Step finished. Please check the output above and press Enter to continue or Ctrl+C to stop."
  read
fi

echo "=== Step $((STEP++)): Installing Python jinja2 ==="
python -m pip install jinja2
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
  echo "Step finished. Please check the output above and press Enter to continue or Ctrl+C to stop."
  read
fi

echo "=== Step $((STEP++)): Installing Python setuptools and wheel ==="
python -m pip install setuptools wheel
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
  echo "Step finished. Please check the output above and press Enter to continue or Ctrl+C to stop."
  read
fi

echo "=== Step $((STEP++)): Fetching and installing Python package bcj-cffi ==="
git clone --recursive https://github.com/miurahr/bcj-cffi.git
cd bcj-cffi
python setup.py install
cd ..
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
  echo "Step finished. Please check the output above and press Enter to continue or Ctrl+C to stop."
  read
fi

echo "=== Step $((STEP++)): Fetching and installing Python package pyppmd ==="
git clone --recursive https://github.com/miurahr/pyppmd.git
cd pyppmd
python setup.py install
cd ..
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
  echo "Step finished. Please check the output above and press Enter to continue or Ctrl+C to stop."
  read
fi

echo "=== Step $((STEP++)): Installing Python package py7zr in version 0.16.1 ==="
python -m pip install py7zr==0.16.1
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
  echo "Step finished. Please check the output above and press Enter to continue or Ctrl+C to stop."
  read
fi

echo "=== Step $((STEP++)): Installing Python package aqtinstall in version 1.2.5 ==="
python -m pip install aqtinstall==1.2.5
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
  echo "Step finished. Please check the output above and press Enter to continue or Ctrl+C to stop."
  read
fi

echo "=== Step $((STEP++)): Installing Qt build environment ==="
python -m aqt install 5.12.9 windows desktop win64_mingw73
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
  echo "Step finished. Please check the output above and press Enter to continue or Ctrl+C to stop."
fi

echo "Setting up build environment has finished. You can now start building EdgeTX firmware and binaries!"
