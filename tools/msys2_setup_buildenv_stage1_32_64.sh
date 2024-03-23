#! /usr/bin/env bash

## Bash script to setup EdgeTX 32 and 64 bit development environments first stage.

if [[ ! "$MSYSTEM" == "MSYS" ]]; then
  echo "ERROR: this script cannot be run in MSYS2 MinGW 64-bit console (blue icon) or"
  echo "       MSYS2 MinGW 32-bit console (green icon)"
  echo "INFO:  run in MSYS2 MSYS console (violet icon)"
  exit 1
fi

# == Include common variables and functions ==
source msys2_common_32_64.sh

# Defaults
INSTALL_64BIT=1
INSTALL_32BIT=0

# == Functions ==

function usage() {
>&2 cat << EOF

Usage:
  $(basename $0) [options]

Parser command options.

Options:
  -h, --help       display help text and exit
  -p, --pause      pause after each command (default: false)
      --no-64bit   do not install 64-bit toolchain
      --32bit      install 32-bit toolchain
EOF
exit 1
}

# == End functions ==

short_options=hp
long_options="help, pause, no-64bit, 32bit"

args=$(getopt --options "$short_options" --longoptions "$long_options" -- "$@")
if [[ $? -gt 0 ]]; then
  usage
fi

eval set -- ${args}

while true
do
	case $1 in
		-p | --pause)       STEP_PAUSE=1     ; shift   ;;
    -h | --help)        usage            ; shift   ;;
         --no-64bit)    INSTALL_64BIT=0  ; shift   ;;
         --32bit)       INSTALL_32BIT=1  ; shift   ;;
    # -- means the end of the arguments; drop this, and break out of the while loop
    --) shift; break ;;
    *) >&2 echo Unsupported option: $1
       usage ;;
	esac
done

echo "
Execute with the following:
  Install 64-bit toolchain:  $(bool_to_text ${INSTALL_64BIT})
  Install 32-bit toolchain:  $(bool_to_text ${INSTALL_32BIT})
  Pause after each step:     $(bool_to_text ${STEP_PAUSE})

IMPORTANT: the MSYS2 base packages MUST be updated prior to running stage 1.
           Run the command 'pacman -Suy' in MSYS2 MSYS console (violet icon).
"
read -p "Press any key to continue or ctrl+C to abort"

if [[ $INSTALL_64BIT -eq 1 ]]; then
  run_step "Installing 64-bit toolchain" "pacman -S --noconfirm mingw-w64-x86_64-toolchain"
fi

if [[ $INSTALL_32BIT -eq 1 ]]; then
  run_step "Installing 32-bit toolchain" "pacman -S --noconfirm mingw-w64-i686-toolchain"
fi

if [[ $INSTALL_64BIT -eq 1 ]] || [[ $INSTALL_32BIT -eq 1 ]]; then
  run_step "Installing git and make packages" "pacman -S --noconfirm git make"
fi

echo "This stage has finished. Please close the MSYS console and continue to stage 2."
