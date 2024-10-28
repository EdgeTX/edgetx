#! /usr/bin/env bash

# exit on first error
# set -e

## Bash script to setup EdgeTX development environment second stage.

if [[ "$MSYSTEM" == "MSYS" ]]; then
  echo "ERROR: this script cannot be run in MSYS2 MSYS console (violet icon)"
  echo "INFO: run as normal user in MSYS2 MinGW 64-bit console (blue icon) or"
  echo "      in MSYS2 MinGW 32-bit console (green icon)"
  exit 1
fi

# == Include common variables and functions ==
source msys2_common_32_64.sh

# == Initialise variables ==
INSTALL_PACKAGES=1
QT_INST=1
QT_BINS=1
DOWNLOAD_DIR="${HOME}"
ARM_INST=1
INSTALL_ARM=1

# == Functions ==

function usage() {
>&2 cat << EOF

Usage:
  $(basename $0) [options]

Parser command options.

Options:
  -d, --download-dir <directory>   directory for the arm installer (default: ${DOWNLOAD_DIR})
  -e, --edgetx-version <version>   installs the dependent Qt version (default: ${EDGETX_VERSION})
  -h, --help                       display help text and exit
      --no-arm                     do not download or install arm toolchain
      --no-arm-installer           do not download arm toolchain installer
      --no-install-arm             do not run arm toolchain installer
      --no-install-packages        do not install packages
      --no-qt                      do not install Qt installer or binaries
      --no-qt-installer            do not install Qt binaries installer
	                                 Note: common for 32bit and 64bit so only needs to be performed once
      --no-qt-bins                 do not download Qt binaries
  -p, --pause                      pause after each command (default: false)
  -q, --qt-version <version>       version of Qt to install (default: ${QT_VERSION})
                                   Note: Overrides EdgeTX dependent version. Allows installing multiple versions.
EOF
exit 1
}

# == End functions ==

short_options=d:,e:,h,p,q:
long_options="download-dir:, edgetx-version:, help, no-arm, no-arm-installer, no-install-arm, no-install-packages, no-qt, no-qt-installer, no-qt-bins, pause, qt-version:"

args=$(getopt --options "$short_options" --longoptions "$long_options" -- "$@")
[[ $? -gt 0 ]] && usage

eval set -- ${args}

while true
do
	case $1 in
    -d | --download-dir)          DOWNLOAD_DIR="${2}"                 ; shift 2 ;;
		-e | --edgetx-version)        EDGETX_VERSION="${2}"
                                  QT_VERSION="$(get_qt_version ${2})" ; shift 2 ;;
    -h | --help)                  usage                               ; shift   ;;
         --no-arm-installer)      ARM_INST=0                          ; shift   ;;
         --no-install-arm)        INSTALL_ARM=0                       ; shift   ;;
         --no-arm)                ARM_INST=0                          ;
                                  INSTALL_ARM=0                       ; shift   ;;
         --no-install-packages)   INSTALL_PACKAGES=0                  ; shift   ;;
         --no-qt-installer)       QT_INST=0                           ; shift   ;;
         --no-qt-bins)            QT_BINS=0                           ; shift   ;;
         --no-qt)                 QT_INST=0                           ;
                                  QT_BINS=0                           ; shift   ;;
    -p | --pause)                 STEP_PAUSE=1                        ; shift   ;;
    -q | --qt-version)            QT_VERSION="${2}"                   ; shift 2 ;;
    # -- means the end of the arguments; drop this, and break out of the while loop
    --) shift; break ;;
    *) >&2 echo Unsupported option: $1
       usage ;;
	esac
done

# == Validation ==

if [[ $ARM_INST -eq 1 ]] && [[ ! -d ${DOWNLOAD_DIR} ]]; then
  fail "ARM installer download directory ${DOWNLOAD_DIR} does not exist"
fi

validate_edgetx_version
split_version EDGETX_VERSION

validate_qt_version
split_version QT_VERSION
check_qt_arch_support

# == End validation ==

clear

cat << EOF

Setup MSYS2 Build Environment for EdgeTX 32 and 64 bit - Stage 2

Executing with the following options:
  EdgeTX version:          ${EDGETX_VERSION}
  Qt version:              ${QT_VERSION}
  Install packages:        $(bool_to_text ${INSTALL_PACKAGES})
  Install Qt installer:    $(bool_to_text ${QT_INST})
  Download Qt binaries:    $(bool_to_text ${QT_BINS})
  ARM download directory:  ${DOWNLOAD_DIR}
  Download ARM installer   $(bool_to_text ${ARM_INST})
  Install ARM toolchain    $(bool_to_text ${INSTALL_ARM})
  Pause after each step:   $(bool_to_text ${STEP_PAUSE})
EOF

read -p "Press Enter Key to continue or Ctrl+C to abort"

# == Execute ==

if [[ $INSTALL_PACKAGES -eq 1 ]]; then
  new_step "Installing packages"

  pacman -S --noconfirm \
  ${MINGW_PACKAGE_PREFIX}-cmake \
  ${MINGW_PACKAGE_PREFIX}-python-pip \
  ${MINGW_PACKAGE_PREFIX}-python-pillow \
  ${MINGW_PACKAGE_PREFIX}-libjpeg-turbo \
  ${MINGW_PACKAGE_PREFIX}-zlib \
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
  ${MINGW_PACKAGE_PREFIX}-openssl \
  ${MINGW_PACKAGE_PREFIX}-wget \
  ${MINGW_PACKAGE_PREFIX}-python-psutil \
  ${MINGW_PACKAGE_PREFIX}-jq

  end_step $? "pacman -S --noconfirm <packages>"

  new_step "Installing dfu-util"

  if [[ "${MINGW_PACKAGE_PREFIX}" == "mingw-w64-x86_64" ]]; then
    pacman -S --noconfirm ${MINGW_PACKAGE_PREFIX}-dfu-util
  else
    wget -qO- https://dfu-util.sourceforge.net/releases/dfu-util-0.11-binaries.tar.xz | tar -xJ
    cp dfu-util-0.11-binaries/win32/dfu-util-static.exe /mingw32/bin/dfu-util.exe
    cp dfu-util-0.11-binaries/win32/libusb-1.0.dll /mingw32/bin/libusb-1.0.dll
    cp dfu-util-0.11-binaries/win32/libusb-1.0.dll.a /mingw32/bin/libusb-1.0.dll.a
  fi

  end_step $? "Installing dfu-util"

  run_step "Upgrading pip" "python -m pip install -U pip"
  # wheel required for aqtinstall
  run_step "Installing Python setuptools and wheel" "python -m pip install setuptools wheel"

  if [[ "${MINGW_PACKAGE_PREFIX}" == "mingw-w64-x86_64" ]]; then
	run_step "Installing lz4" "pacman -S --noconfirm ${MINGW_PACKAGE_PREFIX}-python-lz4"
  else
	run_step "Installing lz4" "python -m pip install lz4"
  fi

  run_step "Installing Python clang and jinja2" "python -m pip install clang jinja2"
fi

if [[ $QT_INST -eq 1 ]]; then
  # Notes:
  #   this will also install dependencies
  #   Nov 2023 psutil is a dependency of aqtinstall but fails to install via dependencies so explicitly install python-psutil
  # Nov 2023
  # From https://github.com/miurahr/aqtinstall Install section
  # When you want to use it on MSYS2/Mingw64 environment, you need to set environmental variable export SETUPTOOLS_USE_DISTUTILS=stdlib,
  # because of setuptools package on mingw wrongly raise error VC6.0 is not supported
  export SETUPTOOLS_USE_DISTUTILS=stdlib

  run_step "Installing Python package aqtinstall" "python -m pip install aqtinstall"
fi

if [[ $QT_BINS -eq 1 ]]; then
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

  run_step "Installing Qt ${QT_VERSION}" \
  "python -m aqt install-qt --outputdir ${QT_INSTALL_DIR} ${QT_HOST} ${QT_TARGET} ${QT_VERSION} ${QT_ARCH} ${QT_MODULES}"
fi

if [[ $ARM_INST -eq 1 ]]; then
  # Note: the version needs to be kept in sync with EdgeTX/build-edgetx/dev/Dockerfile

  DOWNLOAD_FILE="gcc-arm-none-eabi-10-2020-q4-major-win32.exe"

  run_step "Downloading ARM toolchain installer" \
  "wget -c -O ${DOWNLOAD_DIR}/${DOWNLOAD_FILE} --progress=bar:force:noscroll --no-check-certificate \
  'https://developer.arm.com/-/media/Files/downloads/gnu-rm/10-2020q4/${DOWNLOAD_FILE}?revision=9a4bce5a-7577-4b4f-910d-4585f55d35e8&rev=9a4bce5a75774b4f910d4585f55d35e8&hash=9770A44FEA9E9CDAC0DD9A009190CC8B'"

  if [[ $INSTALL_ARM -eq 1 ]]; then
    echo "Lauched ARM toolchain installer"
	  cmd //c ${DOWNLOAD_DIR}/${DOWNLOAD_FILE}
  fi
fi

echo ""
echo "Stage 2 of setting up EdgeTX build environment has finished"
echo ""
