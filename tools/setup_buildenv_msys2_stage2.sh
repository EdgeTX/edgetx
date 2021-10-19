#! /usr/bin/env bash

## Bash script to setup EdgeTX development environment second stage.
## Let it run as normal user in MSYS2 MinGW 64-bit console (blue icon).

pacman -S --noconfirm mingw-w64-x86_64-cmake mingw-w64-x86_64-python-pip mingw-w64-x86_64-arm-none-eabi-gcc mingw-w64-x86_64-libjpeg-turbo mingw-w64-x86_64-zlib mingw-w64-x86_64-libtiff mingw-w64-x86_64-freetype mingw-w64-x86_64-lcms2 mingw-w64-x86_64-libwebp mingw-w64-x86_64-openjpeg2 mingw-w64-x86_64-libimagequant mingw-w64-x86_64-libraqm mingw-w64-x86_64-SDL mingw-w64-x86_64-nsis

pip install wheel

wget https://raw.githubusercontent.com/EdgeTX/edgetx/main/tools/setup_buildenv_msys2_py_requirements.txt

pip install -r setup_buildenv_msys2_py_requirements.txt

rm setup_buildenv_msys2_py_requirements.txt

aqt install 5.12.9 windows desktop win64_mingw73

echo "Setting up build environment has finished. You can now start building EdgeTX firmware and binaries!"
