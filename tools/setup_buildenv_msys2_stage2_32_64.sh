#! /usr/bin/env bash

## Bash script to setup EdgeTX development environment second stage.

if [[ "$MSYSTEM" == "MSYS" ]]; then
  echo "ERROR: this script cannot be run in MSYS2 MSYS console (violet icon)"
  echo "INFO: run as normal user in MSYS2 MinGW 64-bit console (blue icon) or"
  echo "      in MSYS2 MinGW 32-bit console (green icon)"
  exit 1
fi

# == Initialise variables ==
STEP_PAUSE=0
STEP=0

# EdgeTX v2.9 and earlier require 5.12.9
# EdgeTX v2.10 and later require 5.15.2
declare -a supported_qt_versions=("5.12.9" "5.15.2")
QT_ONLY=0
QT_VERSION="${supported_qt_versions[((${#supported_qt_versions[@]} - 1))]}"
# == End initialise variables ==

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
  -h, --help                  display help text and exit
  -p, --pause                 pause after each command (default: false)
  -q, --qtversion <version>   version of Qt to compile against (default: latest)
      --qt-only              skip to qt version install (the download often fails)
EOF
exit 1
}

# == End functions ==

short_options=hpq:
long_options="help, pause, qtversion:, qt-only"

args=$(getopt --options "$short_options" --longoptions "$long_options" -- "$@")
if [[ $? -gt 0 ]]; then
  usage
fi

eval set -- ${args}

while true
do
	case $1 in
		-p | --pause)        STEP_PAUSE=1     ; shift    ;;
    -h | --help)         usage            ; shift    ;;
    -q | --qtversion)    QT_VERSION=${2}  ; shift 2  ;;
    --qt-only)           QT_ONLY=1        ; shift    ;;
    # -- means the end of the arguments; drop this, and break out of the while loop
    --) shift; break ;;
    *) >&2 echo Unsupported option: $1
       usage ;;
	esac
done

# == Validation ==

for qtver in "${supported_qt_versions[@]}"
do
  if [[ "$QT_VERSION" == "$qtver" ]]; then
    qtversionvalid=1
    break
  fi
done

if [[ -z "${qtversionvalid}" ]]; then
  fail "Unsupported Qt version ${QT_VERSION}"
fi

IFS='.' read -ra versarray <<< ${QT_VERSION}
if [[ ${#versarray[@]} -ne 3 ]]; then
	fail "Invalid Qt version: ${QT_VERSION}"
fi

QT_VERS_MAJOR="${versarray[0]}"
QT_VERS_MINOR="${versarray[1]}"
QT_VERS_PATCH="${versarray[2]}"

if [[ "$MSYSTEM" == "MINGW32" ]] && [[ $QT_VERS_MAJOR -gt 5 ]]; then
  fail "Cannot compile 32-bit for Qt versions greater than 5"
fi

# == End validation ==

echo "
Execute with the following:
  Qt version:            ${QT_VERSION}
  Qt only:               $(bool_to_text ${QT_ONLY})
  Pause after each step: $(bool_to_text ${STEP_PAUSE})
"
read -p "Press any key to continue or ctl+C to abort"

# == Execute ==

# Notes:
#   6/11/23 openssl and zlib installed in the base dev toolchain
#           psutil is a dependency of aqtinstall but fails to install via dependencies so explicitly install python-psutil above

if [[ $QT_ONLY -eq 0 ]]; then
  new_step "Installing packages"

  pacman -S --noconfirm \
  ${MINGW_PACKAGE_PREFIX}-cmake \
  ${MINGW_PACKAGE_PREFIX}-python-pip \
  ${MINGW_PACKAGE_PREFIX}-python-pillow \
  ${MINGW_PACKAGE_PREFIX}-python-lz4 \
  ${MINGW_PACKAGE_PREFIX}-python-psutil \
  ${MINGW_PACKAGE_PREFIX}-libjpeg-turbo \
  ${MINGW_PACKAGE_PREFIX}-libtiff \
  ${MINGW_PACKAGE_PREFIX}-freetype \
  ${MINGW_PACKAGE_PREFIX}-lcms2 \
  ${MINGW_PACKAGE_PREFIX}-libwebp \
  ${MINGW_PACKAGE_PREFIX}-openjpeg2 \
  ${MINGW_PACKAGE_PREFIX}-libimagequant \
  ${MINGW_PACKAGE_PREFIX}-libraqm \
  ${MINGW_PACKAGE_PREFIX}-SDL2 \
  ${MINGW_PACKAGE_PREFIX}-clang \
  ${MINGW_PACKAGE_PREFIX}-nsis \
  ${MINGW_PACKAGE_PREFIX}-dfu-util

  end_step $? "pacman -S --noconfirm <packages>"

  # 6/11/23 package only available for 64-bit
  if [[ "$MSYSTEM" == "MINGW64" ]]; then run_step "Installing arm-none-eabi-gcc" "pacman -S --noconfirm	${MINGW_PACKAGE_PREFIX}-arm-none-eabi-gcc"; fi

  run_step "Upgrading pip" "python -m pip install -U pip"
  run_step "Installing Python clang" "python -m pip install clang"
  run_step "Installing Python setuptools and wheel" "python -m pip install setuptools wheel"
  run_step "Installing Python jinja2" "python -m pip install jinja2"

  # From https://github.com/miurahr/aqtinstall Install section
  # When you want to use it on MSYS2/Mingw64 environment, you need to set environmental variable export SETUPTOOLS_USE_DISTUTILS=stdlib,
  # because of setuptools package on mingw wrongly raise error VC6.0 is not supported
  export SETUPTOOLS_USE_DISTUTILS=stdlib

  # this will also install dependencies and see note above re psutil issue
  run_step "Installing Python package aqtinstall" "python -m pip install aqtinstall"
fi

# if modules use syntax -m MODULE [MODULE]
QT_MODULES=
# supported versions of dependencies
QT_TOOLS=
QT_HOST=windows
QT_TARGET=desktop
QT_ARCH=
QT_INSTALL_DIR=qt

new_step "Searching for available Qt ${QT_VERSION} architectures"

qtarchlist="$(python -m aqt list-qt ${QT_HOST} ${QT_TARGET} --arch ${QT_VERSION})"

IFS=' ' read -ra qtarcharray <<< ${qtarchlist}

for arch in "${qtarcharray[@]}"; do
  if [[ "${arch}" == win${MSYSTEM:(-2)}_mingw* ]];then
    QT_ARCH=${arch}
    break
  fi
done

if [[ -z "$QT_ARCH" ]]; then
  end_step 1 "Unable to find suitable Qt architecture in Qt ${QT_VERSION} for ${MSYSTEM}"
else
  end_step 0 ""
fi

run_step "Installing Qt ${QT_VERSION}" "python -m aqt install-qt --outputdir ${QT_INSTALL_DIR} ${QT_HOST} ${QT_TARGET} ${QT_VERSION} ${QT_ARCH} ${QT_MODULES}"

echo "Setting up build environment has finished. You can now start building EdgeTX firmware and binaries!"
