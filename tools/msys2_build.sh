#! /usr/bin/env bash

# exit on first error
# set -e
# echo
# set -x

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
source ${SCRIPT_DIR}/msys2_common.sh

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

OUTPUT_DIR_SUFFIX_CPN=companion

BUILD_OPTIONS=""
EXTRA_BUILD_OPTIONS=""
BUILD_TYPE=Release

BUILD_COMPANION=0
BUILD_FIRMWARE=0
BUILD_INSTALLER=0
BUILD_LIBSIMS=0
BUILD_SIMULATOR=0

# 'all' is required for validation
declare -a supported_radios=(all)
declare -a HWDEFS_RADIO_TYPES=()
declare -a RADIO_TYPES=()
declare -l BUILD_HWDEFS=radios

# == End initialise variables ==

# == Functions ==

function branch_version_part() {
	#	Parameters:
	#	1 - version component as defined in top level CMakeLists.txt

	local verspart

  if [ -f "${SOURCE_PATH}/CMakeLists.txt" ]; then
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

function download_file() {

	# Usage:
	# download_file [options] sourcefile

	#	Options:
	# -b  - branch if not main
	# -dd - destination directory if not current script directory
	#	-df - destination file name if not same as source filename
	#	-sd - source directory relative to repo root

	local branch=main
	local destdir
	local destfile
	local srcdir
	local srcfile

  while [[ -n $1 ]]; do
    case $1 in
      -b)  shift ; branch=${1}   ;;
      -dd) shift ; destdir=${1}  ;;
      -df) shift ; destfile=${1} ;;
      -sd) shift ; srcdir=${1}   ;;
      *)           srcfile=${1}  ;;
    esac
    shift
  done

	[ -z "${srcfile}" ] && fail "Source file not supplied"
	srcdir=$([ ! -z "${srcdir}" ] && echo "${srcdir}/")
	destdir=$([ -z "${destdir}" ] && echo "${srcdir}")
	destfile=$([ -z "${destfile}" ] && echo "${srcfile}")

  wget --no-cache -q -O ${destdir}/${destfile} https://github.com/${REPO_OWNER}/${REPO_NAME}/raw/${branch}/${srcdir}${srcfile}
  [[ $? -gt 0 ]] && fail "Unable to download ${branch}/${srcdir}${srcfile} from ${REPO_OWNER}/${REPO_NAME}"
}

function validate_radio_types() {

  for ((i = 0; i < ${#RADIO_TYPES[@]}; ++i)); do
    if [[ ! " ${supported_radios[*]} " =~ [[:space:]]${RADIO_TYPES[i]}[[:space:]] ]]; then
      fail "Unsupported radio type: '${RADIO_TYPES[i]}'"
    fi
  done
}

function validate_radio_build_options() {

  for ((i = 0; i < ${#RADIO_TYPES[@]}; ++i)); do
    unset BUILD_OPTIONS
    if ! get_target_build_options ${RADIO_TYPES[@],,}; then
      fail "No buid options for radio type: '${RADIO_TYPES[i]}'"
    fi
	done
}

function is_libsim_supported() {
	#	Parameters:
	#	1 - Radio type

  [[ " ${simulator_plugins[*]} " =~ [[:space:]]${1}[[:space:]] ]] && return 0 || return 1
}

function validate_libsims() {

  if [ "${RADIO_TYPES[0]}" != "all" ]; then
    for ((i = 0; i < ${#RADIO_TYPES[@]}; ++i)); do
      if ! is_libsim_supported ${RADIO_TYPES[i]}; then
        fail "Unsupported radio simulator: '${RADIO_TYPES[i]}'"
      fi
    done
  fi
}

function validate_option_hwdefs() {

  local valid_hwdefs=("all" "radios" "none")
  if [[ ! " ${valid_hwdefs[*]} " =~ [[:space:]]${BUILD_HWDEFS}[[:space:]] ]]; then
    fail "--hwdefs: '${BUILD_HWDEFS}'"
  fi
}

function build_output_path() {
	#	Parameters:
	#	1 - Target

  local outpath="${ROOT_DIR}/${OUTPUT_DIR}/${OUTPUT_DIR_PREFIX}"

  [[ $OUTPUT_APPEND_TARGET -eq 1 ]] && outpath+="${1}" || outpath+="output"
  echo ${outpath}
}

function create_switch_output_dir() {
	#	Parameters:
	#	1 - Target

  local crpath="$(build_output_path ${1})"

  [ ! -d ${crpath} ] && run_step "Creating output directory: ${crpath}" "mkdir -p ${crpath}"

  run_step "Switching to output directory: ${crpath}" "cd ${crpath}"
}

function delete_output_dir() {
	#	Parameters:
	#	1 - Target

  local delpath="$(build_output_path ${1})"

  if [ -d ${delpath} ]; then
    log "Deleting: ${delpath}"
    rm -rf ${delpath}
  fi
}

function set_build_options() {
	#	Parameters:
  # 1 - radio type

  unset BUILD_OPTIONS
  get_target_build_options "${1}"
  #log "Build options: ${BUILD_OPTIONS}"
}

function prep_target() {
	#	Parameters:
  # 1 - config

  local cfg="${1}"

  BUILD_COMMON_OPTIONS="--fresh -G 'MSYS Makefiles' -Wno-dev -DCMAKE_PREFIX_PATH=${QT_PATH} -DSDL2_LIBRARY_PATH=${MSYSTEM_PREFIX}/bin/ \
  -DOPENSSL_ROOT_DIR=${MSYSTEM_PREFIX}/bin/ -DCMAKE_BUILD_TYPE=${BUILD_TYPE}"

  run_step "Deleting CMake cache files" "rm -f CMakeCache.txt ${cfg}/CMakeCache.txt"
  run_step "Generating CMake build environment" "cmake ${BUILD_COMMON_OPTIONS} ${BUILD_OPTIONS} ${EXTRA_BUILD_OPTIONS} ${SOURCE_PATH}"
  run_step "Running CMake clean" "cmake --build . --target clean"
  run_step "Running CMake configure: ${cfg}" "cmake --build . --target ${cfg}-configure"
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
	#	Parameters:
  # 1 - message

  if [ ! -z ${1} ]; then
>&2 cat << EOF
""
"${1}"
""
EOF
  fi

>&2 cat << EOF
Usage:
  $(basename $0) [options] [all|<radio-type> <radio-type> ...]

Parser command options.

Options:
  -a, --all                       companion, firmware(s), all hwdefs, installer, libsim(s) and simulator
  -b, --branch <branch>           git branch to use (default: main)
      --build-opts "<options>"    in addition to default radio-type build options eg "-DTRANSLATIONS=DE"
      --build-type <type>         cmake build type (default: Release)
      --clean-all                 delete local repo and output directory (default: false))
      --clean-output              delete existing output directories before building (default: false))
      --clone                     force clone the repo from github even if exists locally (default: false)
  -c, --companion                 build Companion (default: false)
  -d, --defs <radios|all|none>    generate hardware definition files for Companion and Simulator (default: radios)
  -e, --edgetx-version <version>  sets the version of Qt to compile against (default: ${supported_edgetx_versions[0]})
      --fetch                     refresh local source directory from github (default: false)
  -f, --firmware                  build firmware (default: false)
  -h, --help                      display help text and exit
  -i, --installer                 build the installer (default: false)
  -l, --libsims                   build radio library simulator(s) (default: false)
  -m, --arm-toolchain-dir         fully qualified path to arm toolchain directory (default: Windows installer default folder)
      --no-append-target          do not append target (radio-type|companion) to build output directory name
  -o, --output-dir <path>         relative path to root directory for build output files (default: EdgeTX/edgetx/build-<target>)
                                  Note: radio-type or companion will be appended unless --no-append-target
  -p, --pause                     pause after each command (default: false)
  -q, --qt-root-dir <path>        base path for qt install files (default: ${HOME}/qt)
      --qt-version <version>      overide the version of Qt to compile against (default: ${supported_qt_versions[0]})
      --repo-name <name>          github repo name (default: edgetx). This allows using forks of EdgeTX.
      --repo-owner <owner>        github repo owner. This allows using forks of EdgeTX (default: EdgeTX}
  -r, --root-dir <path>           base path for files (default: ${HOME})
      --source-dir <path>         relative path to root directory for source files (default: EdgeTX/edgetx)
  -s, --simulator                 build Simulator (default: false)
EOF

exit 1
}

# == End functions ==

# == Parse the command line ==
short_options=ab:cd:e:fhilm:o:pq:r:s

long_options="all, branch:, clean-all, clean-output, edgetx-version:, help, output-dir:, pause, arm-toolchain-dir:, qt-root-dir:, root-dir:, source-dir:, \
build-options:, build-type:, clone, companion, fetch, firmware, installer, no-append-target, qt-version:, libsims, \
repo-name:, repo-owner:, simulator, defs:"

args=$(getopt --options "$short_options" --longoptions "$long_options" -- "$@")
[[ $? -gt 0 ]] && usage

eval set -- ${args}

## No parameters passed to script
[[ $# -eq 1 ]] && usage "No parameters passed to script"

while true
do
	case $1 in
    -a | --all)                 BUILD_FIRMWARE=1
                                BUILD_COMPANION=1
                                BUILD_SIMULATOR=1
                                BUILD_LIBSIMS=1
                                BUILD_INSTALLER=1
                                BUILD_HWDEFS=all                                ; shift   ;;
		-b | --branch)	            BRANCH_NAME="${2}"                              ; shift 2 ;;
         --clean-all)           OUTPUT_DELETE=1
                                REPO_CLONE=1                                    ; shift   ;;
         --clean-output)        OUTPUT_DELETE=1                                 ; shift   ;;
    -d | --defs)                BUILD_HWDEFS="${2}"                             ; shift 2 ;;
		-e | --edgetx-version)	    EDGETX_VERSION="${2}"
                                QT_VERSION="$(get_qt_version ${2})"             ; shift 2 ;;
		     --qt-version)	        QT_VERSION="${2}"                               ; shift 2 ;;
    -h | --help)                usage                                           ; shift   ;;
		     --repo-owner)	        REPO_OWNER="${2}"                               ; shift 2 ;;
		     --repo-name)	          REPO_NAME="${2}"                                ; shift 2 ;;
		-m | --arm-toolchain-dir)   ARM_TOOLCHAIN_DIR="${2}"                        ; shift 2 ;;
		-p | --pause)               STEP_PAUSE=1                                    ; shift   ;;
		-q | --qt-root-dir)         QT_ROOT_DIR="${2}"                              ; shift 2 ;;
		-r | --root-dir)            ROOT_DIR="${2}"                                 ; shift 2 ;;
         --source-dir)          SOURCE_DIR="${2}"
                                OUTPUT_DIR="${2}"                               ; shift 2 ;;
		-o | --output-dir)          OUTPUT_DIR="${2}"                               ; shift 2 ;;
		     --build-options)       EXTRA_BUILD_OPTIONS="${2}"                      ; shift 2 ;;
		     --build-type)          BUILD_TYPE="${2}"                               ; shift 2 ;;
         --clone)               REPO_CLONE=1                                    ; shift   ;;
         --fetch)               REPO_FETCH=1                                    ; shift   ;;
    -c | --companion)           BUILD_COMPANION=1                               ; shift   ;;
    -f | --firmware)            BUILD_FIRMWARE=1                                ; shift   ;;
    -i | --installer)           BUILD_INSTALLER=1                               ; shift   ;;
    -l | --libsims)             BUILD_LIBSIMS=1                                 ; shift   ;;
    -s | --simulator)           BUILD_SIMULATOR=1                               ; shift   ;;
         --no-append-target)    OUTPUT_APPEND_TARGET=0                          ; shift   ;;
    # -- means the end of the arguments; drop this, and break out of the while loop
    --) shift; break ;;
    *) >&2 echo Unsupported option: $1
       usage ;;
	esac
done

# remaining args should be a list of radio-types
[[ $# -eq 0 ]] && usage "No radio-types specified"
RADIO_TYPES=($@)

# == End parse command line =="

[ ! -d "$ROOT_DIR" ] && fail "Unable to find root directory $ROOT_DIR"

if [[ ${BUILD_FIRMWARE} -eq 1 && ! -d "${ARM_TOOLCHAIN_DIR}" ]] ; then
  fail "Unable to find ARM toolchain directory ${ARM_TOOLCHAIN_DIR}"
fi

SOURCE_PATH="${ROOT_DIR}/${SOURCE_DIR}"
OUTPUT_PATH="${ROOT_DIR}/${OUTPUT_DIR}/${OUTPUT_DIR_PREFIX}"

[ $OUTPUT_APPEND_TARGET -eq 1 ] && OUTPUT_PATH+="${OUTPUT_TARGET_PLACEHOLDER}" || OUTPUT_PATH+="output"

validate_edgetx_version
split_version EDGETX_VERSION

if [[ -f "${SOURCE_PATH}/CMakeLists.txt" && $REPO_CLONE -eq 0 && $REPO_FETCH -eq 0 ]]; then
  BRANCH_EDGETX_VERSION=$(branch_version)
  BRANCH_QT_VERSION=$(get_qt_version "${BRANCH_EDGETX_VERSION}")
fi

validate_qt_version
split_version QT_VERSION
check_qt_arch_support
QT_PATH="${QT_ROOT_DIR}/${QT_VERSION}"

[ ! -d "$QT_PATH" ] && fail "Unable to find Qt install directory $QT_PATH"

unset ARCHDIR

for d in $QT_PATH/*; do
  dirname="$(basename $d)"

  if [[ "${dirname}" == *_${MSYSTEM:(-2)} ]];then
    ARCHDIR="${dirname}"
    break
  fi
done

[ -z "$ARCHDIR" ] && fail "Unable to find suitable Qt architecture directory in $QT_PATH for $MSYSTEM"

QT_PATH+="/${ARCHDIR}"

# this should never be true but included as a script logic safeguard
[ ! -d "$QT_PATH" ] && fail "Unable to find Qt install directory $QT_PATH"

# Option overrides

if [ ! -d "${SOURCE_PATH}/.git" ] || [ ! -f "${SOURCE_PATH}/CMakeLists.txt" ]; then
  REPO_CLONE=1
  REPO_FETCH=0
else
  if [ $REPO_FETCH -eq 1 ]; then
    REPO_CLONE=0
  fi
  if [ $REPO_CLONE -eq 1 ]; then
    REPO_FETCH=0
  fi
fi

CONFIG_COMMON_FILE="build-common.sh"
CONFIG_COMMON_REPO_DIR="tools"
CONFIG_FIRMWARES_FILE="fw.json"

# download latest config files from repo if local do not exist or to possibly be overwritten
if [ $REPO_FETCH -eq 1 || $REPO_CLONE -eq 1 ]; then
  # supported radio simulators and build options
  download_file -sd ${CONFIG_COMMON_REPO_DIR} -dd "${SCRIPT_DIR}" "${CONFIG_COMMON_FILE}"
  CONFIG_COMMON_PATH="${SCRIPT_DIR}/${CONFIG_COMMON_FILE}"
  # supported radio firmwares
  download_file "${CONFIG_FIRMWARES_FILE}"
  CONFIG_FIRMWARES_PATH="${SCRIPT_DIR}/${CONFIG_FIRMWARES_FILE}"
else
  # use the repo versions
  CONFIG_COMMON_PATH="${SOURCE_PATH}/${CONFIG_COMMON_REPO_DIR}/${CONFIG_COMMON_FILE}"
  CONFIG_FIRMWARES_PATH="${SOURCE_PATH}/${CONFIG_FIRMWARES_FILE}"
fi

# load downloaded or local configuration files
[ ! -f ${CONFIG_COMMON_PATH} ] && fail "${CONFIG_COMMON_PATH} not found"
source "${CONFIG_COMMON_PATH}"

[ ! -f "${CONFIG_FIRMWARES_PATH}" ] && fail "${CONFIG_FIRMWARES_PATH} not found"

for radio_target in $(cat '${CONFIG_FIRMWARES_PATH}' | jq -r '.targets[] | .[1]'); do
  # remove trailing hyphen to end of string
  supported_radios+=("${radio_target%-*}")
done

# echo ${supported_radios[*]}

validate_radio_types
validate_radio_build_options
validate_libsims
validate_option_hwdefs

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
  Generate hardware defns:  ${BUILD_HWDEFS}
  Delete output dirs:       $(bool_to_text ${OUTPUT_DELETE})
  Build Companion:          $(bool_to_text ${BUILD_COMPANION})
  Build firmware:           $(bool_to_text ${BUILD_FIRMWARE})
  Build installer:          $(bool_to_text ${BUILD_INSTALLER})
  Build libsims:            $(bool_to_text ${BUILD_LIBSIMS})
  Build Simulator:          $(bool_to_text ${BUILD_SIMULATOR})
  Pause after each step:    $(bool_to_text ${STEP_PAUSE})
EOF

read -p "Press Enter key to continue or Ctrl+C to abort"

## ============== Execute ==============

if [ "${RADIO_TYPES[0]}" == "all" ]; then
  # omit the 'all' validation entry
  RADIO_TYPES=("${supported_radios[@]:1}")
fi

if [ "${BUILD_HWDEFS}" == "all" ]; then
  # omit the 'all' validation entry
  HWDEFS_RADIO_TYPES=("${supported_radios[@]:1}")
elif [ "${BUILD_HWDEFS}" == "radios" ]; then
  HWDEFS_RADIO_TYPES=("${RADIO_TYPES[@]}")
fi

if [[ $REPO_CLONE -eq 1 && -d "${SOURCE_PATH}" ]]; then
  run_step "Deleting existing source directory" "rm -rf ${SOURCE_PATH}"
fi

# tidy output directories before builds
if [ $OUTPUT_DELETE -eq 1 ]; then
  new_step "Deleting old build output directories"

  if [ $OUTPUT_APPEND_TARGET -eq 0 ]; then
    delete_output_dir
  else
    if [[ $BUILD_FIRMWARE -eq 1 || $BUILD_LIBSIMS -eq 1 ]]; then
      for ((i = 0; i < ${#RADIO_TYPES[@]}; ++i)); do
        delete_output_dir ${RADIO_TYPES[i]}
      done
    fi
    if [[ "$BUILD_HWDEFS" != "none" || $BUILD_COMPANION -eq 1 || $BUILD_SIMULATOR -eq 1 || $BUILD_INSTALLER -eq 1 ]]; then
      delete_output_dir ${OUTPUT_DIR_SUFFIX_CPN}
    fi
  fi

  end_step 0
fi

# github source
if [ $REPO_CLONE -eq 1 ]; then
  run_step "Cloning GitHub repo" \
           "git clone --recursive -b ${BRANCH_NAME} https://github.com/${REPO_OWNER}/${REPO_NAME}.git ${SOURCE_PATH}"
  run_step "Switching to source directory" "cd ${SOURCE_PATH}"
fi

if [ $REPO_FETCH -eq 1 ]; then
  run_step "Switching to source directory" "cd ${SOURCE_PATH}"
  run_step "Fetching latest commits" "git fetch --all"
  run_step "Checking out branch" "git checkout ${BRANCH_NAME}"
  run_step "Forcing reset of local to remote" "git reset --hard origin/${BRANCH_NAME}"
  run_step "Cleaning up after reset" "git clean -df"
  run_step "Updating submodules to current commits" "git submodule update --init --recursive"
fi

# (re)check Qt version suitable for branch
if [[ $REPO_CLONE -eq 1 || $REPO_FETCH -eq 1 ]]; then
  new_step "Checking required Qt version for branch"
  BRANCH_EDGETX_VERSION=$(branch_version)
  BRANCH_QT_VERSION=$(get_qt_version "${BRANCH_EDGETX_VERSION}")

  if [ "${BRANCH_QT_VERSION}" != "${QT_VERSION}" ]; then
    warn "Qt version ${QT_VERSION} branch '${BRANCH_NAME}' expects Qt version ${BRANCH_QT_VERSION}"
    warn "Press Enter to continue or Ctrl+C to abort."
    read
  fi

  end_step 0
fi

# builds
if [ $BUILD_FIRMWARE -eq 1 ]; then
  # required for compile and elf-size-report.sh
  PATH=${ARM_TOOLCHAIN_DIR}:${PATH}

  for ((i = 0; i < ${#RADIO_TYPES[@]}; ++i)); do
    log "Generating firmware: ${RADIO_TYPES[i]}"
    create_switch_output_dir ${RADIO_TYPES[i]}
    set_build_options ${RADIO_TYPES[i]}
    prep_and_build_target arm-none-eabi firmware-size

    if [ $OUTPUT_APPEND_TARGET -eq 0 ]; then
      run_step "Renaming firmware binary" "mv arm-none-eabi/firmware.bin arm-none-eabi/firmware-${RADIO_TYPES[i]}.bin"
    fi
  done
fi

if [[ "${BUILD_HWDEFS}" != "none" || $BUILD_COMPANION -eq 1 || $BUILD_SIMULATOR -eq 1 || $BUILD_LIBSIMS -eq 1 || $BUILD_INSTALLER -eq 1 ]]; then
  create_switch_output_dir ${OUTPUT_DIR_SUFFIX_CPN}
fi

if [ $BUILD_LIBSIMS -eq 1 ]; then
  for ((i = 0; i < ${#RADIO_TYPES[@]}; ++i)); do
    # not all radios may have libsim support when radio type 'all'
    if is_libsim_supported ${RADIO_TYPES[i]}; then
      log "Generating libsim: ${RADIO_TYPES[i]}"
      set_build_options ${RADIO_TYPES[i]}
      prep_and_build_target native libsimulator
    fi
  done
fi

if [ "${BUILD_HWDEFS}" == "all" ]; then
  new_step "Removing existing radio hardware definitions"
	radiodir="$(build_output_path ${OUTPUT_DIR_SUFFIX_CPN})/native/radio/src"
	if [ -d ${radiodir} ]; then
		rm -f ${radiodir}/*.json*
	fi
	unset radiodir
  end_step 0
fi

if [ "${BUILD_HWDEFS}" != "none" ]; then
  if [[ $OUTPUT_APPEND_TARGET -eq 1 || ( $BUILD_LIBSIMS -eq 0 && ( $BUILD_COMPANION -eq 1 || $BUILD_SIMULATOR -eq 1 ) ) ]]; then
    # generate hardware definition json files for inclusion as resources in Companion and Simulator
    for ((i = 0; i < ${#HWDEFS_RADIO_TYPES[@]}; ++i)); do
      log "Generating hardware definition: ${HWDEFS_RADIO_TYPES[i]}"
      set_build_options ${HWDEFS_RADIO_TYPES[i]}
      prep_and_build_target native hardware_defs
    done
  fi
fi

if [ $BUILD_COMPANION -eq 1 ]; then
  new_step "Clean Companion hardware definitions resource"
	# forces cmake to rebuild resource from latest set of json files
	cpndir="$(build_output_path ${OUTPUT_DIR_SUFFIX_CPN})/native/companion/src"
	if [ -d ${cpndir} ]; then
		rm -f ${cpndir}/hwdefs.qrc*
		rm -f ${cpndir}/qrc_hwdefs.cpp*
	fi
	unset cpndir
  end_step 0
fi

if [[ $BUILD_COMPANION -eq 1 || $BUILD_SIMULATOR -eq 1 || $BUILD_INSTALLER -eq 1 ]]; then
  # use the first radio as cmake will fail without a radio
  set_build_options ${RADIO_TYPES[0]}
  prep_target native
fi

[ $BUILD_COMPANION -eq 1 ] && build_target native companion

[ $BUILD_SIMULATOR -eq 1 ] && build_target native simulator

[ $BUILD_INSTALLER -eq 1 ] && build_target native installer

if [ $BUILD_FIRMWARE -eq 1 ]; then
  [[ OUTPUT_APPEND_TARGET -eq 0 ]] && RADIO_FIRMWARE_FILE="firmware-[radio-type].bin" || RADIO_FIRMWARE_FILE="firmware.bin"
  echo "Firmwares   : $(build_output_path '[radio-type]')/arm-none-eabi/${RADIO_FIRMWARE_FILE}"
fi

[ $BUILD_COMPANION -eq 1 ] && echo "Companion   : $(build_output_path ${OUTPUT_DIR_SUFFIX_CPN})/native/companion.exe"
[ $BUILD_SIMULATOR -eq 1 ] && echo "Simulator   : $(build_output_path ${OUTPUT_DIR_SUFFIX_CPN})/native/simulator.exe"
[ $BUILD_LIBSIMS   -eq 1 ] && echo "Libsims     : $(build_output_path ${OUTPUT_DIR_SUFFIX_CPN})/native/libedgetx-[radio-type]-simulator.dll"
[ $BUILD_INSTALLER -eq 1 ] && echo "Installer   : $(build_output_path ${OUTPUT_DIR_SUFFIX_CPN})/native/companion/companion-windows-x.x.x.exe"

echo ""
echo "Build(s) finished"
echo ""
