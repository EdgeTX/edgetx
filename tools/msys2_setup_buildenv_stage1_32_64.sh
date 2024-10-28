#! /usr/bin/env bash

# exit on first error
# set -e

## Bash script to setup EdgeTX 32 and 64 bit development environments first stage.

if [[ ! "$MSYSTEM" == "MSYS" ]]; then
  echo "ERROR: this script cannot be run in MSYS2 MinGW 64-bit console (blue icon) or"
  echo "       MSYS2 MinGW 32-bit console (green icon)"
  echo "INFO:  run in MSYS2 MSYS console (violet icon)"
  exit 1
fi

cat << EOF

**************************************************************************
*                        I M P O R T A N T                               *
*                                                                        *
*  The MSYS2 base packages MUST be updated PRIOR to running this script. *
*                                                                        *
*  If you have not done so, abort this script. Then in a MSYS2           *
*  MSYS console (violet icon) session, run the command 'pacman -Suy'     *
**************************************************************************

EOF

read -p "Press Enter key to continue or Ctrl+C to abort"

# == Include common variables and functions ==
source msys2_common_32_64.sh

# Defaults
INSTALL_64BIT=1
INSTALL_32BIT=0
INSTALL_EXTRAS=1

# == Functions ==

function usage() {
>&2 cat << EOF

Usage:
  $(basename $0) [options]

Parser command options.

Options:
  -h, --help       display help text and exit
  -p, --pause      pause after each command (default: false)
      --no-64bit   do not install 64-bit toolchain (default: install)
      --32bit      install 32-bit toolchain (default: do not install)
	    --no-extras  do not install the extra base packages (default: install)
EOF

exit 1
}

# == End functions ==

short_options=hp
long_options="help, pause, no-64bit, 32bit, no-extras"

args=$(getopt --options "$short_options" --longoptions "$long_options" -- "$@")
[[ $? -gt 0 ]] && usage

eval set -- ${args}

while true
do
	case $1 in
	  -p | --pause)       STEP_PAUSE=1     ; shift   ;;
    -h | --help)        usage            ; shift   ;;
         --no-64bit)    INSTALL_64BIT=0  ; shift   ;;
         --32bit)       INSTALL_32BIT=1  ; shift   ;;
         --no-extras)   INSTALL_EXTRAS=0 ; shift   ;;
    # -- means the end of the arguments; drop this, and break out of the while loop
    --) shift; break ;;
    *) >&2 echo Unsupported option: $1
       usage ;;
	esac
done

clear

cat << EOF

Setup MSYS2 Build Environment for EdgeTX 32 and 64 bit - Stage 1

Executing with the following options:
  Install 64-bit toolchain:  $(bool_to_text ${INSTALL_64BIT})
  Install 32-bit toolchain:  $(bool_to_text ${INSTALL_32BIT})
  Install extra packages:    $(bool_to_text ${INSTALL_EXTRAS})
  Pause after each step:     $(bool_to_text ${STEP_PAUSE})

EOF

read -p "Press Enter key to continue or Ctrl+C to abort"

[[ $INSTALL_64BIT -eq 1 ]] && run_step "Installing 64-bit toolchain" "pacman -S --noconfirm mingw-w64-x86_64-toolchain"

[[ $INSTALL_32BIT -eq 1 ]] && run_step "Installing 32-bit toolchain" "pacman -S --noconfirm mingw-w64-i686-toolchain"

[[ $INSTALL_EXTRAS -eq 1 ]] && run_step "Installing git, make and tar packages" "pacman -S --noconfirm git make tar"

echo ""
echo "Stage 1 of setting up EdgeTX build environment has finished."
echo "Please close this console and continue to Stage 2."
echo ""
