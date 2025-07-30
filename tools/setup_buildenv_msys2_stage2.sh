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
pacman -S --noconfirm mingw-w64-x86_64-cmake \
					mingw-w64-x86_64-python-pip \
					mingw-w64-x86_64-python-pillow \
					mingw-w64-x86_64-python-lz4 \
					mingw-w64-x86_64-libjpeg-turbo \
					mingw-w64-x86_64-zlib \
					mingw-w64-x86_64-libtiff \
					mingw-w64-x86_64-freetype \
					mingw-w64-x86_64-lcms2 \
					mingw-w64-x86_64-libwebp \
					mingw-w64-x86_64-openjpeg2 \
					mingw-w64-x86_64-libimagequant \
					mingw-w64-x86_64-libraqm \
					mingw-w64-x86_64-SDL2 \
					mingw-w64-x86_64-clang \
					mingw-w64-x86_64-nsis \
					mingw-w64-x86_64-dfu-util \
					mingw-w64-x86_64-openssl

if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
  echo "Step finished. Please check the output above and press Enter to continue or Ctrl+C to stop."
  read
fi

echo "=== Step $((STEP++)): Downloading GNU Arm Embedded Toolchain ==="
wget https://developer.arm.com/-/media/Files/downloads/gnu/14.2.rel1/binrel/arm-gnu-toolchain-14.2.rel1-mingw-w64-i686-arm-none-eabi.zip
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
  echo "Step finished. Please check the output above and press Enter to continue or Ctrl+C to stop."
  read
fi

echo "=== Step $((STEP++)): Installing GNU Arm Embedded Toolchain ==="
unzip arm-gnu-toolchain-14.2.rel1-mingw-w64-i686-arm-none-eabi.zip -d gcc-arm-none-eabi
rm arm-gnu-toolchain-14.2.rel1-mingw-w64-i686-arm-none-eabi.zip
mv gcc-arm-none-eabi/ /opt/gcc-arm-none-eabi
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

echo "=== Step $((STEP++)): Installing Python packages ==="
# Python 3.11 introduced the managed environment breakage aka PEP 668.
# since we are building a self-contained environment the simple fix is to add --break-system-packages to all pip installs
python3 -m pip install --break-system-package -U setuptools && \
python3 -m pip install --break-system-package \
    asciitree \
    jinja2 \
    pillow \
    clang==14.0.0 \
    lz4 \
	pyelftools
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
  echo "Step finished. Please check the output above and press Enter to continue or Ctrl+C to stop."
  read
fi

echo "=== Step $((STEP++)): Downloading aqt ==="
wget "https://github.com/miurahr/aqtinstall/releases/download/Continuous/aqt.exe"
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
  echo "Step finished. Please check the output above and press Enter to continue or Ctrl+C to stop."
  read
fi

echo "=== Step $((STEP++)): Installing Qt build environment ==="
./aqt.exe install-qt windows desktop 6.9.0 win64_mingw -m qtmultimedia qtserialport 
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
  echo "Step finished. Please check the output above and press Enter to continue or Ctrl+C to stop."
fi

echo "Setting up build environment has finished. You can now start building EdgeTX firmware and binaries!"
