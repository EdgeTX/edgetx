#! /usr/bin/env bash

## Bash script to show how to get EdgeTX source from GitHub,
## how to build firmware, Companion, Simulator, radio simulator
## library and how to create an installation package.
## Let it run as normal user in MSYS2 MinGW 64-bit console (blue icon) or 32-bit console (green icon).
##
## Note: This script works only for branches stemming from EdgeTX (v2.9 or later)

# == Initialise variables ==
STEP=0
STEP_PAUSE=0 # false
declare -a supported_qt_versions=("5.12.9" "5.15.2")

REPO="edgetx"		    # eg edgetx | myrepo (for a fork of edgetx)
BRANCH_NAME="main"  # eg main | 2.9 | fix-issue-1234
QT_VERSION="${supported_qt_versions[((${#supported_qt_versions[@]} - 1))]}"

# 0 - false 1 = true
PROJECT_ROOT_DIR=${HOME}
BUILD_OPTIONS=""
REPO_INIT=1
REPO_FETCH=1
BUILD_DIR_INIT=1
BUILD_COMPANION=1
BUILD_SIMULATOR=1
BUILD_RADIO_SIM=1
BUILD_INSTALLER=1

# 6/11/23 there is no arm package available for 32-bit
if [[ "$MSYSTEM" == "MINGW32" ]]; then
  BUILD_FIRMWARE=0
else
  BUILD_FIRMWARE=1
fi

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
  $(basename $0) [options] <radio-type>

Parser command options.

Options:
  -b, --branch <branch>       git branch use (default: main)
  -h, --help                  display help text and exit
  -p, --pause                 pause after each command (default: false)
  -q, --qtversion <version>   version of Qt to compile against (default: 5.15.2)
  -r, --repo <repo>           github repo to use (default: edgetx}
      --project-root-dir      project root directory (default: \$HOME)
      --extra-build-options   added to standard options eg -DTRANSLATIONS=DE
      --no-repo-init          do not delete local source directory
      --no-repo-fetch         do not clone or refresh the local source dirctory from github
      --no-build-dir-init     do not delete the build directory (use with caution!)
      --no-compile            do not compile or create install components
      --no-companion          do not compile Companion
      --no-firmware           do not compile firmware
      --no-installer          do not create the installer
      --no-radio-sim          do not compile radio libsim dll
      --no-simulator          do not compile Simulator
EOF
exit 1
}

# == End functions ==

# == Parse the command line ==
short_options=hpb:q:r:
long_options="help, pause, branch:, qtversion:, repo:, project-root-dir:, extra-build-options:, no-compile, no-repo-init, no-repo-fetch, no-build-dir-init, no-companion, no-firmware, no-installer, no-radio-sim, no-simulator"

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
		-b | --branch)	        BRANCH_NAME=${2}       ; shift 2 ;;
		-q | --qtversion)	      QT_VERSION=${2}        ; shift 2 ;;
		-r | --repo)	          REPO=${2}              ; shift 2 ;;
		-p | --pause)           STEP_PAUSE=1           ; shift   ;;
    -h | --help)            usage                  ; shift   ;;
		--project-root-dir)     PROJECT_ROOT_DIR=${2}  ; shift 2 ;;
		--extra-build-options)  BUILD_OPTIONS=${2}     ; shift 2 ;;
    --no-compile)           BUILD_FIRMWARE=0
                            BUILD_COMPANION=0
                            BUILD_SIMULATOR=0
                            BUILD_RADIO_SIM=0
                            BUILD_INSTALLER=0      ; shift   ;;
    --no-repo-init)         REPO_INIT=0            ; shift   ;;
    --no-repo-fetch)        REPO_FETCH=0           ; shift   ;;
    --no-build-dir-init)    BUILD_DIR_INIT=0       ; shift   ;;
    --no-companion)         BUILD_COMPANION=0      ; shift   ;;
    --no-installer)         BUILD_INSTALLER=0      ; shift   ;;
    --no-firmware)          BUILD_FIRMWARE=0       ; shift   ;;
    --no-radio-sim)         BUILD_RADIO_SIM=0      ; shift   ;;
    --no-simulator)         BUILD_SIMULATOR=0      ; shift   ;;
    # -- means the end of the arguments; drop this, and break out of the while loop
    --) shift; break ;;
    *) >&2 echo Unsupported option: $1
       usage ;;
	esac
done

if [[ $# -ne 1 ]]; then
  fail "Only one radio type permitted"
  usage
fi

RADIO_TYPE=$@

# maintain logical sequence
if [[ $REPO_FETCH -eq 0 ]]; then REPO_INIT=0; fi
if [[ $REPO_INIT -eq 1 ]]; then REPO_FETCH=1; fi
# do not risk artifacts from previous runs
if [[ $REPO_FETCH -eq 1 ]]; then BUILD_DIR_INIT=1; fi

# == End parse command line =="

# == Validation ==

BUILD_OPTIONS+=" -DDEFAULT_MODE=2 -DGVARS=YES"

case $RADIO_TYPE in
    x9lite)     BUILD_OPTIONS+=" -DPCB=X9LITE" ;;
    x9lites)    BUILD_OPTIONS+=" -DPCB=X9LITES" ;;
    x7)         BUILD_OPTIONS+=" -DPCB=X7" ;;
    x7-access)  BUILD_OPTIONS+=" -DPCB=X7 -DPCBREV=ACCESS -DPXX1=YES" ;;
    t8)         BUILD_OPTIONS+=" -DPCB=X7 -DPCBREV=T8" ;;
    t12)        BUILD_OPTIONS+=" -DPCB=X7 -DPCBREV=T12 -DINTERNAL_MODULE_MULTI=ON" ;;
    tx12)       BUILD_OPTIONS+=" -DPCB=X7 -DPCBREV=TX12" ;;
    tx12mk2)    BUILD_OPTIONS+=" -DPCB=X7 -DPCBREV=TX12MK2" ;;
    zorro)      BUILD_OPTIONS+=" -DPCB=X7 -DPCBREV=ZORRO" ;;
    commando8)  BUILD_OPTIONS+=" -DPCB=X7 -DPCBREV=COMMANDO8" ;;
    boxer)      BUILD_OPTIONS+=" -DPCB=X7 -DPCBREV=BOXER" ;;
    tlite)      BUILD_OPTIONS+=" -DPCB=X7 -DPCBREV=TLITE" ;;
    tpro)       BUILD_OPTIONS+=" -DPCB=X7 -DPCBREV=TPRO" ;;
    lr3pro)     BUILD_OPTIONS+=" -DPCB=X7 -DPCBREV=LR3PRO" ;;
    xlite)      BUILD_OPTIONS+=" -DPCB=XLITE" ;;
    xlites)     BUILD_OPTIONS+=" -DPCB=XLITES" ;;
    nv14)       BUILD_OPTIONS+=" -DPCB=NV14" ;;
    x9d)        BUILD_OPTIONS+=" -DPCB=X9D" ;;
    x9dp)       BUILD_OPTIONS+=" -DPCB=X9D+" ;;
    x9dp2019)   BUILD_OPTIONS+=" -DPCB=X9D+ -DPCBREV=2019" ;;
    x9e)        BUILD_OPTIONS+=" -DPCB=X9E" ;;
    x9e-hall)   BUILD_OPTIONS+=" -DPCB=X9E -DSTICKS=HORUS" ;;
    x10)        BUILD_OPTIONS+=" -DPCB=X10" ;;
    x10-access) BUILD_OPTIONS+=" -DPCB=X10 -DPCBREV=EXPRESS -DPXX1=YES" ;;
    x12s)       BUILD_OPTIONS+=" -DPCB=X12S" ;;
    t16)        BUILD_OPTIONS+=" -DPCB=X10 -DPCBREV=T16 -DINTERNAL_MODULE_MULTI=ON" ;;
    t18)        BUILD_OPTIONS+=" -DPCB=X10 -DPCBREV=T18" ;;
    tx16s)      BUILD_OPTIONS+=" -DPCB=X10 -DPCBREV=TX16S" ;;
    *)          fail "Unknown radio type: $RADIO_TYPE" ;;
esac

# Validate Qt
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
	fail "Qt version: ${QT_VERSION} invalid format. Format expected major.minor.patch"
fi

QT_VERS_MAJOR="${versarray[0]}"
QT_VERS_MINOR="${versarray[1]}"
QT_VERS_PATCH="${versarray[2]}"

if [[ "$MSYSTEM" == "MINGW32" ]] && [[ $QT_VERS_MAJOR -gt 5 ]]; then
  fail "Cannot compile 32-bit binaries for Qt versions later than 5.15"
fi

QT_BASE="${HOME}/qt"
QT_PATH="${QT_BASE}/${QT_VERSION}"

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

# this should never be true but included as a safeguard
if [[ ! -d "$QT_PATH" ]]; then
  fail "Unable to find Qt install directory $QT_PATH"
fi

if [[ ! -d "$PROJECT_ROOT_DIR" ]]; then
  fail "Unable to find project root directory $PROJECT_ROOT_DIR"
fi

# End validate Qt

# == End validation ==

PROJECT_DIR="${PROJECT_ROOT_DIR}/$REPO"
SOURCE_DIR="${PROJECT_DIR}/edgetx"
BUILD_OUTPUT_DIR="${PROJECT_DIR}/build-output"

echo "
Execute with the following:
Repo:                    ${REPO}
Branch:                  ${BRANCH_NAME}
Source directory:        ${SOURCE_DIR}
Build output directory:  ${BUILD_OUTPUT_DIR}
Qt path:                 ${QT_PATH}
Radio:
  Type:                  ${RADIO_TYPE}
  Options:               ${BUILD_OPTIONS}
Pause after each step:   $(bool_to_text ${STEP_PAUSE})
Processes:
  Repo initialise:       $(bool_to_text ${REPO_INIT})
  Repo fetch:            $(bool_to_text ${REPO_FETCH})
  Build directory init:  $(bool_to_text ${BUILD_DIR_INIT})
  Build firmware:        $(bool_to_text ${BUILD_FIRMWARE})
  Build Companion:       $(bool_to_text ${BUILD_COMPANION})
  Build Simulator:       $(bool_to_text ${BUILD_SIMULATOR})
  Build radio libsim:    $(bool_to_text ${BUILD_RADIO_SIM})
  Build installer:       $(bool_to_text ${BUILD_INSTALLER})
"
read -p "Press any key to continue or ctl+C to abort"

## ============== Execute ==============

if [[ $REPO_INIT -eq 1 ]] && [[ -d ${SOURCE_DIR} ]]; then
  run_step "Deleting existing source directory" "rm -rf ${SOURCE_DIR}"
fi

if [[ $REPO_FETCH -eq 1 ]]; then
  if [[ ! -d "${PROJECT_DIR}" ]]; then
    run_step "Creating project directory for ${REPO}" "mkdir ${PROJECT_DIR}"
    run_step "Switching to project directory" "cd ${PROJECT_DIR}"
  fi

  if [[ ! -d "${SOURCE_DIR}/.git" ]]; then
    run_step "Cloning GitHub repo ${REPO} branch ${BRANCH_NAME}" \
             "git clone --recursive -b ${BRANCH_NAME} https://github.com/${REPO}/edgetx.git ${SOURCE_DIR} && cd ${SOURCE_DIR}"
  else
    run_step "Switching to source directory ${SOURCE_DIR}" "cd ${SOURCE_DIR}"
    run_step "Fetching latest commits from ${REPO}" "git fetch --all"
    run_step "Checking out branch ${BRANCH_NAME}" "git checkout ${BRANCH_NAME}"
    run_step "Forcing reset of local to remote" "git reset --hard origin/${BRANCH_NAME}"
    run_step "Cleaning up after reset" "git clean -df"
  fi

  run_step "Updating submodules to current commits" "git submodule update --init --recursive"
else
  run_step "Switching to source directory" "cd ${SOURCE_DIR}"
  run_step "Checking out branch ${BRANCH_NAME}" "git checkout ${BRANCH_NAME}"
fi

if [[ $BUILD_DIR_INIT -eq 1 ]]; then
  if [[ -d ${BUILD_OUTPUT_DIR} ]]; then
    run_step "Deleting previous build output directory" "rm -rf ${BUILD_OUTPUT_DIR}"
  fi

  run_step "Creating build output directory" "mkdir ${BUILD_OUTPUT_DIR}"
fi

run_step "Switching to ${BUILD_OUTPUT_DIR}" "cd ${BUILD_OUTPUT_DIR}"

run_step "Running CMake" \
"cmake -G 'MSYS Makefiles' -Wno-dev -DCMAKE_PREFIX_PATH=${QT_PATH} -DSDL2_LIBRARY_PATH=${MSYSTEM_PREFIX}/bin/ -DLIBSSL1_ROOT_DIR=${MSYSTEM_PREFIX}/bin/ ${BUILD_OPTIONS} -DCMAKE_BUILD_TYPE=Release ${SOURCE_DIR}"

if [[ $BUILD_FIRMWARE -eq 1 ]]; then
  run_step "Running Make configure arm-none-eab" "make arm-none-eabi-configure"
  run_step "Building firmware binary" "make -C arm-none-eabi -j$(nproc) firmware"
  run_step "Renaming firmware binary" "mv arm-none-eabi/firmware.bin arm-none-eabi/fw_${RADIO_TYPE}_release.bin"
fi

if [[ $BUILD_COMPANION -eq 1 ]] || [[ $BUILD_SIMULATOR -eq 1 ]] || [[ $BUILD_RADIO_SIM -eq 1 ]] || [[ $BUILD_INSTALLER -eq 1 ]]; then
  run_step "Running Make configure native" "make native-configure"
  MAKENATIVECMD="make -C native -j$(nproc)"
fi

if [[ $BUILD_COMPANION -eq 1 ]]; then run_step "Building Companion" "${MAKENATIVECMD} companion"; fi
if [[ $BUILD_SIMULATOR -eq 1 ]]; then run_step "Building Simulator" "${MAKENATIVECMD} simulator"; fi
if [[ $BUILD_RADIO_SIM -eq 1 ]]; then run_step "Building radio libsim" "${MAKENATIVECMD} libsimulator"; fi
if [[ $BUILD_INSTALLER -eq 1 ]]; then run_step "Making the installer" "${MAKENATIVECMD} installer"; fi

echo ""
echo "Completed successfully"
echo ""
if [[ $BUILD_FIRMWARE -eq 1 ]];  then echo "firmware            : ${BUILD_OUTPUT_DIR}/arm-none-eabi/fw_${RADIO_TYPE}_release.bin"; fi
if [[ $BUILD_COMPANION -eq 1 ]]; then echo "Companion           : ${BUILD_OUTPUT_DIR}/native/Release/companion.exe"; fi
if [[ $BUILD_SIMULATOR -eq 1 ]]; then echo "Simulator           : ${BUILD_OUTPUT_DIR}/native/Release/simulator.exe"; fi
if [[ $BUILD_RADIO_SIM -eq 1 ]]; then echo "Radio libsim        : ${BUILD_OUTPUT_DIR}/native/Release/libedgetx-${RADIO_TYPE}-simulator.dll"; fi
if [[ $BUILD_INSTALLER -eq 1 ]]; then echo "Companion installer : ${BUILD_OUTPUT_DIR}/native/companion/companion-windows-x.x.x.exe"; fi
