#! /usr/bin/env bash

## Bash script to setup EdgeTX development environment first stage.
## Let it run as normal user in MSYS2 MSYS console (violet icon).

pacman -Su --noconfirm

pacman -S --noconfirm git make mingw-w64-x86_64-toolchain

echo "This stage has finished. Please close the MSYS console and continue to stage 2 in MinGW 64-bit console (blue icon)."
