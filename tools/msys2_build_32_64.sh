#! /usr/bin/env bash

# exit on first error
set -e

## Bash script to show how to get EdgeTX source from GitHub,
## how to build firmware, Companion, Simulator, radio simulator
## library and how to create an installation package.
## Let it run as normal user in MSYS2 MinGW 64-bit console (blue icon) or 32-bit console (green icon).
##
## Note: This script works only for branches stemming from EdgeTX (v2.10 or later)

if [[ "$MSYSTEM" == "MSYS" ]]; then
  echo "ERROR: this script cannot be run in MSYS2 MSYS console (violet icon)"
  echo "INFO: run as normal user in MSYS2 MinGW 64-bit console (blue icon) or"
  echo "      in MSYS2 MinGW 32-bit console (green icon)"
  exit 1
fi

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

# == Include common variables and functions ==
. ${SCRIPT_DIR}/msys2_common_32_64.sh

# download latest supported radio simulators and build options
wget -qLO https://github.com/edgetx/edgetx/raw/main/tools/build-common.sh
. ${SCRIPT_DIR}/build-common.sh

# download latest supported radio firmwares
wget -qlO https://github.com/edgetx/edgetx/raw/main/fw.json

declare -a supported_radios=(all)

for radio_target in $(cat fw.json | jq -r '.targets[] | .[1]'); do
  supported_radios+=(${radio_target%-})
done

# == Initialise variables ==

REPO_OWNER="EdgeTX"
REPO_NAME="edgetx"
REPO_CLONE=0
REPO_FETCH=0
BRANCH_NAME="main"
BRANCH_EDGETX_VERSION=unknown
BRANCH_QT_VERSION=unknown

ARM_TOOLCHAIN_DIR="/c/Program Files (x86)/GNU Arm Embedded Toolchain/10 2020-q4-major/bin"
QT_ROOT_DIR="${HOME}/qt"
ROOT_DIR="${HOME}"
SOURCE_DIR="${REPO_OWNER}/${REPO_NAME}"

OUTPUT_DIR_PREFIX="build-"
OUTPUT_TARGET_PLACEHOLDER="<target>"
OUTPUT_DIR="${SOURCE_DIR}"
OUTPUT_APPEND_TARGET=1
OUTPUT_DELETE=0

CPN_FLDR=companion

BUILD_OPTIONS=""
EXTRA_BUILD_OPTIONS=""
BUILD_TYPE=Release

BUILD_COMPANION=0
BUILD_FIRMWARE=0
BUILD_INSTALLER=0
BUILD_LIBSIMS=0
BUILD_SIMULATOR=0
BUILD_HWDEFS=0

declare -a HWDEFS_RADIO_TYPES=()

declare -a RADIO_TYPES=()

# == End initialise variables ==

# == Functions ==

function branch_version_part() {
	#	Parameters:
	#	1 - version component as defined in top level CMakeLists.txt

	local verspart

  if [[ -f "${SOURCE_PATH}/CMakeLists.txt" ]]; then
    searchstr="set(VERSION_${1}"
    if [ $(grep -m 1 -c "${searchstr}" "${SOURCE_PATH}/CMakeLists.txt") -eq 1 ]; then
      verspart=$(grep -m 1 "${searchstr}" "${SOURCE_PATH}/CMakeLists.txt" | cut -c $(expr ${#searchstr} + 1)-)
      verspart=${verspart/\)/}
      verspart=${verspart//\"/}
      verspart=${verspart// /}
    fi
  fi

  echo ${verspart}
}

function branch_version_major() {
	echo "$(branch_version_part 'MAJOR')"
}

function branch_version_minor() {
	echo "$(branch_version_part 'MINOR')"
}

function branch_version_revision() {
	echo "$(branch_version_part 'REVISION')"
}

function branch_version() {
	echo "$(branch_version_major).$(branch_version_minor).$(branch_version_revision)"
}

function validate_radio_types() {

  for ((i = 0; i < ${#RADIO_TYPES[@]}; ++i)); do
  	local radio_found=0

    for ((j = 0; j < ${#supported_radios[@]}; ++j)); do
      if [[ "${RADIO_TYPES[i],,}" == "${supported_radios[j],,}" ]]; then
        radio_found=1
        break;
      fi
    done

    if [[ radio_found -eq 0 ]]; then
      fail "Unsupported radio type: '${RADIO_TYPES[i]}'"
    fi
	done
}

function validate_radio_build_options() {

  for ((i = 0; i < ${#RADIO_TYPES[@]}; ++i)); do
    if ! get_target_build_options ${RADIO_TYPES[@],,}; then
      fail "No buid options for radio type: '${RADIO_TYPES[i]}'"
    fi
	done
}

function validate_simulator_plugins() {

  for ((i = 0; i < ${#RADIO_TYPES[@]}; ++i)); do
  	local sim_found=0

    for ((j = 0; j < ${#simulator_plugins[@]}; ++j)); do
      if [[ "${RADIO_TYPES[i],,}" == "${simulator_plugins[j],,}" ]]; then
        sim_found=1
        break;
      fi
    done

    if [[ sim_found -eq 0 ]]; then
      fail "Unsupported radio simulator: '${RADIO_TYPES[i]}'"
    fi
	done
}

function get_all_radio_types() {
	#	Parameters:
	#	1 - variable to old the radio list

	local -a radios

  # start at 1 to skip 'all' entry
  for ((i = 1; i < ${#supported_radios[@]}; ++i));  do
		IFS='|' read -ra supported_radio <<< "${supported_radios[(i)]}"
    radio="$(trim_spaces "${supported_radio[0],,}")"
		radios+=(${radio})
	done

  eval "${1}=(${radios[@]})"
}

function build_output_path() {
	#	Parameters:
	#	1 - Target

  outpath="${ROOT_DIR}/${OUTPUT_DIR}/${OUTPUT_DIR_PREFIX}"
  if [[ $OUTPUT_APPEND_TARGET -eq 1 ]]; then
    outpath+="${1}";
  else
    outpath+="output";
  fi
  echo ${outpath}
}

function create_output_dir() {
	#	Parameters:
	#	1 - Target

  crpath="$(build_output_path ${1})"

  if [[ ! -d ${crpath} ]]; then
    run_step "Creating output directory: ${crpath}" "mkdir -p ${crpath}"
  fi

  run_step "Switching to output directory" "cd ${crpath}"
}

function delete_output_dir() {
	#	Parameters:
	#	1 - Target

  local delpath="$(build_output_path ${1})"

  if [[ -d ${delpath} ]]; then
    log "Deleting: ${delpath}"
    rm -rf ${delpath}
  fi
}

function prep_target() {
	#	Parameters:
  # 1 - config

  BUILD_COMMON_OPTIONS="--fresh -G 'MSYS Makefiles' -Wno-dev -DCMAKE_PREFIX_PATH=${QT_PATH} -DSDL2_LIBRARY_PATH=${MSYSTEM_PREFIX}/bin/ \
  -DOPENSSL_ROOT_DIR=${MSYSTEM_PREFIX}/bin/ -DCMAKE_BUILD_TYPE=${BUILD_TYPE}"

  run_step "Generating CMake build environment" "cmake ${BUILD_COMMON_OPTIONS} ${BUILD_OPTIONS} ${EXTRA_BUILD_OPTIONS} ${SOURCE_PATH}"
  run_step "Running CMake clean" "cmake --build . --target clean"
  run_step "Running CMake configure: ${1}" "cmake --build . --target ${1}-configure"
}

function build_target() {
	#	Parameters:
  # 1 - config
	#	2 - Target
  run_step "Building: $2" "cmake --build ${1} -j$(nproc) --target ${2}"
}

function prep_and_build_target() {
	#	Parameters:
  # 1 - config
	#	2 - Target
  prep_target ${1}
  build_target ${1} ${2}
}

function usage() {
>&2 cat << EOF

Usage:
  $(basename $0) [options] [all|<radio-type> <radio-type> ...]

Parser command options.

Options:
  -a, --all-targets                    build companion, firmware(s), radio libsim(s), simulator and installer
  -b, --branch <branch>                git branch use (default: main)
      --extra_build-options <options>  eg -DTRANSLATIONS=DE
                                       Note: radio options will be appended
      --build-type <type>              cmake build type default: ${BUILD_TYPE}
  -c, --clean                          delete local repo and output directory
      --clone                          force clone the repo from github even if exists locally
      --companion                      compile Companion
      --delete-output                  delete existing output directories before building
  -e, --edgetx-version <version>       sets the version of Qt to compile against (default: ${EDGETX_VERSION})
      --fetch                          refresh local source directory from github
      --firmware                       compile firmware
  -h, --help                           display help text and exit
      --hw-defs                        generate all hardware definition json files for Companion and Simulator
                                       Note: hardware definitions must be generated at least once
      --hw-defs-radio-types            generate hardware definition json files for only radio-types specified in command line
                                       Note: generating all supported radios takes a considerable amount of time
                                             recommended to generate all at least once
      --installer                      build the installer
      --libsims                        compile radio simulator dlls
  -m, --arm-toolchain-dir              fully qualified path to arm toolchain directory (default: Windows installer default folder)
      --no-append-target               do not append target (radio type|companion) to build output directory name
  -o, --output-dir <path>              relative path to root directory for build output files (default: $OUTPUT_DIR)
                                       Note: radio type will be appended unless --no-append-radio
  -p, --pause                          pause after each command (default: false)
  -q, --qt-root-dir <path>             base path for qt install files (default: $QT_ROOT_DIR)
      --qt-version <version>           overide the version of Qt to compile against (default: ${QT_VERSION})
      --repo-name <name>               github repo name (default: $REPO_NAME)
      --repo-owner <owner>             github repo owner. This allows using forks of $REPO_NAME (default: $REPO_OWNER}
  -r, --root-dir <path>                base path for files (default: $ROOT_DIR)
  -s, --source-dir <path>              relative path to root directory for source files (default: $SOURCE_DIR)
      --simulator                      compile Simulator
EOF
exit 1
}

# == End functions ==

# == Parse the command line ==
short_options=ab:ce:hm:o:pq:r:s:
long_options="all-targets, branch:, clean, edgetx-version:, help, output-dir:, pause, arm-toolchain-dir:, qt-root-dir:, root-dir:, source-dir:, \
extra_build-options:, build-type:, clone, companion, delete-output, fetch, firmware, installer, no-append-target, qt-version:, libsims, \
repo-name:, repo-owner:, simulator, hw-defs, hw-defs-radio-types"

args=$(getopt --options "$short_options" --longoptions "$long_options" -- "$@")
if [[ $? -gt 0 ]]; then
  usage
fi

eval set -- ${args}

## No parameters passed to script
if [[ $# -eq 1 ]]; then
  usage
fi

while true
do
	case $1 in
    -c | --clean)               OUTPUT_DELETE=1
                                REPO_CLONE=1                                    ; shift   ;;
    -a | --all-targets)         BUILD_FIRMWARE=1
                                BUILD_COMPANION=1
                                BUILD_SIMULATOR=1
                                BUILD_LIBSIMS=1
                                BUILD_INSTALLER=1
                                BUILD_HWDEFS=1                                  ; shift   ;;
		-b | --branch)	            BRANCH_NAME="${2}"                              ; shift 2 ;;
		-e | --edgetx-version)	    EDGETX_VERSION="${2}"
                                QT_VERSION="$(get_qt_version ${2})"             ; shift 2 ;;
		     --qt-version)	        QT_VERSION="${2}"                               ; shift 2 ;;
		-p | --pause)               STEP_PAUSE=1                                    ; shift   ;;
    -h | --help)                usage                                           ; shift   ;;
		     --repo-owner)	        REPO_OWNER="${2}"                               ; shift 2 ;;
		     --repo-name)	          REPO_NAME="${2}"                                ; shift 2 ;;
		-m | --arm-toolchain-dir)   ARM_TOOLCHAIN_DIR="${2}"                        ; shift 2 ;;
		-q | --qt-root-dir)         QT_ROOT_DIR="${2}"                              ; shift 2 ;;
		-r | --root-dir)            ROOT_DIR="${2}"                                 ; shift 2 ;;
		-s | --source-dir)          SOURCE_DIR="${2}"
                                OUTPUT_DIR="${2}"                               ; shift 2 ;;
		-o | --output-dir)          OUTPUT_DIR="${2}"                               ; shift 2 ;;
         --delete-output)       OUTPUT_DELETE=1                                 ; shift   ;;
		     --extra-build-options) EXTRA_BUILD_OPTIONS="${2}"                      ; shift 2 ;;
		     --build-type)          BUILD_TYPE="${2}"                               ; shift 2 ;;
         --clone)               REPO_CLONE=1                                    ; shift   ;;
         --fetch)               REPO_FETCH=1                                    ; shift   ;;
         --companion)           BUILD_COMPANION=1                               ; shift   ;;
         --firmware)            BUILD_FIRMWARE=1                                ; shift   ;;
         --installer)           BUILD_INSTALLER=1                               ; shift   ;;
         --libsims)             BUILD_LIBSIMS=1                                 ; shift   ;;
         --simulator)           BUILD_SIMULATOR=1                               ; shift   ;;
         --hw-defs-radio-types) BUILD_HWDEFS=2                                  ; shift   ;;
         --hw-defs)             BUILD_HWDEFS=1                                  ; shift   ;;
         --no-append-target)    OUTPUT_APPEND_TARGET=0                          ; shift   ;;
    # -- means the end of the arguments; drop this, and break out of the while loop
    --) shift; break ;;
    *) >&2 echo Unsupported option: $1
       usage ;;
	esac
done

if [[ $# -eq 0 ]]; then
  fail "No radio types specified"
  usage
fi

# == End parse command line =="

# == Validation ==

if [[ ! -d "$ROOT_DIR" ]]; then
  fail "Unable to find root directory $ROOT_DIR"
fi

if [[ ${BUILD_FIRMWARE} -eq 1 ]] && [[ ! -d "${ARM_TOOLCHAIN_DIR}" ]]; then
  fail "Unable to find ARM toolchain directory ${ARM_TOOLCHAIN_DIR}"
fi

RADIO_TYPES=($@)
validate_radio_types
validate_simulator_plugins
validate_radio_build_options

# option --hw-defs
if [[ ${BUILD_HWDEFS} -eq 1 ]]; then
    HWDEFS_RADIO_TYPES=("all")
else
  # option --hw-defs-radio-types
  if [[ ${BUILD_HWDEFS} -eq 2 ]]; then
    HWDEFS_RADIO_TYPES=(${RADIO_TYPES[@]})
  fi
fi

SOURCE_PATH="${ROOT_DIR}/${SOURCE_DIR}"
OUTPUT_PATH="${ROOT_DIR}/${OUTPUT_DIR}/${OUTPUT_DIR_PREFIX}"

if [[ $OUTPUT_APPEND_TARGET -eq 1 ]]; then
  OUTPUT_PATH+="${OUTPUT_TARGET_PLACEHOLDER}"
else
  OUTPUT_PATH+="output"
fi

validate_edgetx_version
split_version EDGETX_VERSION

if [[ -f "${SOURCE_PATH}/CMakeLists.txt" ]] && [[ $REPO_CLONE -eq 0 ]] && [[ $REPO_FETCH -eq 0 ]]; then
  BRANCH_EDGETX_VERSION=$(branch_version)
  BRANCH_QT_VERSION=$(get_qt_version "${BRANCH_EDGETX_VERSION}")
fi

validate_qt_version
split_version QT_VERSION
check_qt_arch_support
QT_PATH="${QT_ROOT_DIR}/${QT_VERSION}"

if [[ ! -d "$QT_PATH" ]]; then
  fail "Unable to find Qt install directory $QT_PATH"
fi

unset archdir

for d in $QT_PATH/*; do
  dirname="$(basename $d)"

  if [[ "${dirname}" == *_${MSYSTEM:(-2)} ]];then
    archdir="${dirname}"
    break
  fi
done

if [[ -z "$archdir" ]]; then
  fail "Unable to find suitable Qt architecture directory in $QT_PATH for $MSYSTEM"
fi

QT_PATH+="/${archdir}"

# this should never be true but included as a script logic safeguard
if [[ ! -d "$QT_PATH" ]]; then
  fail "Unable to find Qt install directory $QT_PATH"
fi

# == End validation ==

# Option overrides

if [[ ! -d "${SOURCE_PATH}/.git" ]] || [[ ! -f "${SOURCE_PATH}/CMakeLists.txt" ]]; then
  REPO_CLONE=1
  REPO_FETCH=0
else
  if [[ $REPO_FETCH -eq 1 ]]; then
    REPO_CLONE=0
  fi
  if [[ $REPO_CLONE -eq 1 ]]; then
    REPO_FETCH=0
  fi
fi

if [[ $OUTPUT_DELETE -eq 1 ]]; then
  if [[ $BUILD_COMPANION -eq 1 ]] || [[ $BUILD_SIMULATOR -eq 1 ]] || [[ $BUILD_INSTALLER -eq 1 ]]; then
    BUILD_HWDEFS=1
  fi
fi

# Display confirmation message with option to exit

cat << EOF
EdgeTX version:             ${EDGETX_VERSION}
Qt version:                 ${QT_VERSION}
Radio types:                ${RADIO_TYPES[@]}
Extra build options:        ${EXTRA_BUILD_OPTIONS}
Build type:                 ${BUILD_TYPE}
Repo owner:                 ${REPO_OWNER}
     name:                  ${REPO_NAME}
     branch:                ${BRANCH_NAME}
     branch EdgeTX version: ${BRANCH_EDGETX_VERSION}
     branch Qt version:     ${BRANCH_QT_VERSION}
     clone:                 $(bool_to_text ${REPO_CLONE})
     fetch:                 $(bool_to_text ${REPO_FETCH})
Paths:
  ARM toolchain:            ${ARM_TOOLCHAIN_DIR}
  Root:                     ${ROOT_DIR}
  Source:                   ${SOURCE_PATH}
  Output:                   ${OUTPUT_PATH}
  Qt package:               ${QT_PATH}
Options:
  Delete output dirs:       $(bool_to_text ${OUTPUT_DELETE})
  Build Companion:          $(bool_to_text ${BUILD_COMPANION})
  Build firmware:           $(bool_to_text ${BUILD_FIRMWARE})
  Build installer:          $(bool_to_text ${BUILD_INSTALLER})
  Build libsims:            $(bool_to_text ${BUILD_LIBSIMS})
  Build Simulator:          $(bool_to_text ${BUILD_SIMULATOR})
  Generate hardware defns:  $(bool_to_text ${BUILD_HWDEFS}) : ${HWDEFS_RADIO_TYPES[@]}
  Pause after each step:    $(bool_to_text ${STEP_PAUSE})
EOF

read -p "Press Enter key to continue or Ctrl+C to abort"

## ============== Execute ==============

if [[ ${RADIO_TYPES,,} == "all" ]]; then
  get_all_radio_types RADIO_TYPES
fi

if [[ ${HWDEFS_RADIO_TYPES,,} == "all" ]]; then
  get_all_radio_types HWDEFS_RADIO_TYPES
fi

if [[ $REPO_CLONE -eq 1 ]] && [[ -d ${SOURCE_PATH} ]]; then
  run_step "Deleting existing source directory" "rm -rf ${SOURCE_PATH}"
fi

# tidy output directories before builds
if [[ $OUTPUT_DELETE -eq 1 ]]; then
  new_step "Deleting old build outputs"
  if [[ $OUTPUT_APPEND_TARGET -eq 0 ]]; then
    delete_output_dir
  else
    if [[ $BUILD_FIRMWARE -eq 1 ]] || [[ $BUILD_RADIO_SIM -eq 1 ]]; then
      for ((i = 0; i < ${#RADIO_TYPES[@]}; ++i)); do
        delete_output_dir ${RADIO_TYPES[i]}
      done
    fi
    if [[ $BUILD_HWDEFS -ne 0 ]] || [[ $BUILD_COMPANION -eq 1 ]] || [[ $BUILD_SIMULATOR -eq 1 ]] || [[ $BUILD_INSTALLER -eq 1 ]]; then
      delete_output_dir ${CPN_FLDR}
    fi
  fi
  end_step 0 "Deleting old build outputs"
fi

# github source
if [[ $REPO_CLONE -eq 1 ]]; then
  run_step "Cloning GitHub repo" \
           "git clone --recursive -b ${BRANCH_NAME} https://github.com/${REPO_OWNER}/${REPO_NAME}.git ${SOURCE_PATH}"
  run_step "Switching to source directory" "cd ${SOURCE_PATH}"
fi

if [[ $REPO_FETCH -eq 1 ]]; then
  run_step "Switching to source directory" "cd ${SOURCE_PATH}"
  run_step "Fetching latest commits" "git fetch --all"
  run_step "Checking out branch" "git checkout ${BRANCH_NAME}"
  run_step "Forcing reset of local to remote" "git reset --hard origin/${BRANCH_NAME}"
  run_step "Cleaning up after reset" "git clean -df"
  run_step "Updating submodules to current commits" "git submodule update --init --recursive"
fi

# (re)check Qt version suitable for branch
if [[ $REPO_CLONE -eq 1 ]] || [[ $REPO_FETCH -eq 1 ]]; then
  new_step "Checking required Qt version for branch"
  BRANCH_EDGETX_VERSION=$(branch_version)
  BRANCH_QT_VERSION=$(get_qt_version "${BRANCH_EDGETX_VERSION}")

  if [[ "${BRANCH_QT_VERSION}" != "${QT_VERSION}" ]]; then
    warn "Qt version ${QT_VERSION} branch '${BRANCH_NAME}' expects Qt version ${BRANCH_QT_VERSION}"
    warn "Press Enter to continue or Ctrl+C to abort."
    read
  fi

  end_step 0 "Checking required Qt version for branch"
fi

# builds
if [[ $BUILD_FIRMWARE -eq 1 ]]; then
  # required for compile and elf-size-report.sh
  PATH=${ARM_TOOLCHAIN_DIR}:${PATH}

  for ((i = 0; i < ${#RADIO_TYPES[@]}; ++i)); do
    create_output_dir ${RADIO_TYPES[i]}
    get_target_build_options ${RADIO_TYPES[i]}
    prep_and_build_target arm-none-eabi firmware-size

    if [[ $OUTPUT_APPEND_TARGET -eq 0 ]]; then
      run_step "Renaming firmware binary" "mv arm-none-eabi/firmware.bin arm-none-eabi/firmware_${RADIO_TYPES[i]}.bin"
    fi
  done
fi

if [[ $BUILD_LIBSIMS -eq 1 ]]; then
  for ((i = 0; i < ${#RADIO_TYPES[@]}; ++i)); do
    create_output_dir ${RADIO_TYPES[i]}
    get_target_build_options ${RADIO_TYPES[i]}
    prep_and_build_target native libsimulator
  done
fi

if [[ $BUILD_HWDEFS -ne 0 ]] || [[ $BUILD_COMPANION -eq 1 ]] || [[ $BUILD_SIMULATOR -eq 1 ]] || [[ $BUILD_INSTALLER -eq 1 ]]; then
  create_output_dir ${CPN_FLDR}
fi

if [[ $BUILD_HWDEFS -ne 0 ]]; then
  new_step "Removing previous hw defs"
  # clean up from previous runs to force regeneration
	radiodir="$(build_output_path ${CPN_FLDR})/native/radio/src"
	if [ -d ${radiodir} ]; then
		rm -f ${radiodir}/*.json*
	fi

	# forces cmake to rebuild from latest set of json files
	cpndir="$(build_output_path ${CPN_FLDR})/native/companion/src"
	if [ -d ${cpndir} ]; then
		rm -f ${cpndir}/hwdefs.qrc*
		rm -f ${cpndir}/qrc_hwdefs.cpp*
	fi

  end_step 0 "Removing previous hw defs"

  # generate all hardware definition json files for inclusion as resources in Companion and Simulator
  for ((i = 0; i < ${#HWDEFS_RADIO_TYPES[@]}; ++i)); do
    get_target_build_options ${RADIO_TYPES[i]}
    prep_and_build_target native hardware_defs
  done
else
  if [[ $BUILD_COMPANION -eq 1 ]] || [[ $BUILD_SIMULATOR -eq 1 ]] || [[ $BUILD_INSTALLER -eq 1 ]]; then
    # just use the first radio as cmake will fail without a radio
    get_target_build_options ${RADIO_TYPES[0]}
    prep_target native
  fi
fi

if [[ $BUILD_COMPANION -eq 1 ]]; then build_target native companion; fi

if [[ $BUILD_SIMULATOR -eq 1 ]]; then build_target native simulator; fi

if [[ $BUILD_INSTALLER -eq 1 ]]; then build_target native installer; fi

echo ""
echo "Completed successfully"
echo ""

OUTPUT_PATH="${ROOT_DIR}/${OUTPUT_DIR}/${OUTPUT_DIR_PREFIX}"
if [[ $OUTPUT_APPEND_TARGET -eq 1 ]]; then OUTPUT_PATH+="${OUTPUT_TARGET_PLACEHOLDER}"; fi

if [[ $BUILD_FIRMWARE -eq 1 ]]; then
  if [[ OUTPUT_APPEND_TARGET -eq 0 ]]; then
    echo "Firmwares   : ${OUTPUT_PATH}/arm-none-eabi/firmware[-radio type].bin"
  else
    echo "Firmwares   : ${OUTPUT_PATH}/arm-none-eabi/firmware.bin"
  fi
fi

if [[ $BUILD_COMPANION -eq 1 ]]; then echo "Companion   : ${OUTPUT_PATH}/native/Release/companion.exe"; fi
if [[ $BUILD_SIMULATOR -eq 1 ]]; then echo "Simulator   : ${OUTPUT_PATH}/native/Release/simulator.exe"; fi
if [[ $BUILD_LIBSIMS   -eq 1 ]]; then echo "Libsims     : ${OUTPUT_PATH}/native/Release/libedgetx-[radio-type]-simulator.dll"; fi
if [[ $BUILD_INSTALLER -eq 1 ]]; then echo "Installer   : ${OUTPUT_PATH}/native/companion/companion-windows-x.x.x.exe"; fi
