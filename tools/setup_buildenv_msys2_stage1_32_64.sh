#! /usr/bin/env bash

## Bash script to setup EdgeTX 32 and 64 bit development environments first stage.

if [[ ! "$MSYSTEM" == "MSYS" ]]; then
  echo "ERROR: this script cannot be run in MSYS2 MinGW 64-bit console (blue icon) or"
  echo "       in MSYS2 MinGW 32-bit console (green icon)"
  echo "INFO: run in MSYS2 MSYS console (violet icon)"
  exit 1
fi

STEP_PAUSE=0
STEP=0

# Defaults
INSTALL_64BIT=1
INSTALL_32BIT=0

# == Functions ==

function log() {
  echo "=== [INFO] $*"
}

function fail() {
  echo ""
  echo "=== [ERROR] $*"
  exit 1
}

function check_command() {
  result=$1
  cli_info=$2

  if [[ $result -ne 0 ]]; then
    fail "${cli_info} (exit-code=$result)"
  else
    log "Step $STEP finished - OK"
  
    if [[ $STEP_PAUSE -eq 1 ]]; then
      echo "Step finished. Please check the output above and press Enter to continue or Ctrl+C to stop."
      read
    fi

    return 0
  fi
}

new_step() {
  log "=== Step $((++STEP)): ${1} ==="
}

end_step() {
	#	Parameters:
	#	1 - Result code
  # 2 - Message

  check_command $1 "${2}"
}

run_step() {
	#	Parameters:
	#	1 - Description
  # 2 - Command

  new_step "${1}"
  eval "${2}"
  end_step $? "${2}"
}

bool_to_text() {
  if [[ $1 -eq 1 ]]; then
    echo "Yes"
  else
    echo "No"
  fi
}

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
		-p | --pause)  STEP_PAUSE=1     ; shift   ;;
    -h | --help)   usage            ; shift   ;;
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
  Install 32-bit toolchain:  $(bool_to_text ${INSTALL_32BIT})
  Install 64-bit toolchain:  $(bool_to_text ${INSTALL_64BIT})
  Pause after each step:     $(bool_to_text ${STEP_PAUSE})
"
read -p "Press any key to continue or ctl+C to abort"

run_step "Updating MSYS2 base packages" "pacman -Suy --noconfirm"
if [[ $INSTALL_32BIT -eq 1 ]]; then run_step "Installing 32-bit toolchain" "pacman -S --noconfirm mingw-w64-i686-toolchain"; fi
if [[ $INSTALL_64BIT -eq 1 ]]; then run_step "Installing 64-bit toolchain" "pacman -S --noconfirm mingw-w64-x86_64-toolchain"; fi
run_step "Installing git and make packages" "pacman -S --noconfirm git make"

# Free up disk space
run_step "Pruning the MSYS2 package cache" "paccache -r"

echo "This stage has finished. Please close the MSYS console and continue to stage 2."
