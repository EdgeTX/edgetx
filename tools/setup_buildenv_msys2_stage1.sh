#! /usr/bin/env bash

## Bash script to setup EdgeTX development environment first stage.
## Let it run as normal user in MSYS2 MSYS console (violet icon).

PAUSEAFTEREACHLINE="false"
STEP=1
# Parse argument(s)
for arg in "$@"
do
	if [[ $arg == "--pause" ]]; then
		PAUSEAFTEREACHLINE="true"
	fi
done

echo "=== Step $((STEP++)): Updating MSYS2 base packages ==="
pacman -Su --noconfirm
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
  echo "Step finished. Please check the output above and press Enter to continue or Ctrl+C to stop."
  read
fi

echo "=== Step $((STEP++)): Installing system wide packages ==="
pacman -S --noconfirm git make mingw-w64-x86_64-toolchain
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
  echo "Step finished. Please check the output above and press Enter to continue or Ctrl+C to stop."
fi

echo "This stage has finished. Please close the MSYS console and continue to stage 2 in MinGW 64-bit console (blue icon)."
