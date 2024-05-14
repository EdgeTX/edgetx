#! /usr/bin/env bash

## Bash script to provide common code for MSYS2 setup EdgeTX development toolchain and compiling binaries

# **** Note: these arrays must be kept in sync and the latest version first in sequence      ****
# ****       EdgeTX versions major.minor[.patch]                                             ****
# ****       Qt versions major.minor.patch and can repeat to keep paired with EdgeTX version ****
declare -a supported_edgetx_versions=("2.11" "2.10")
declare -a supported_qt_versions=("5.15.2" "5.15.2")

EDGETX_VERSION="${supported_edgetx_versions[0]}"
QT_VERSION="${supported_qt_versions[0]}"
STEP_PAUSE=0
STEP=0

function log() {
  echo "=== [INFO] $*"
}

function fail() {
  echo ""
  echo "=== [ERROR] $*"
  exit 1
}

function warn() {
  echo "=== [WARNING] $*"
}

function check_command() {
  #	Parameters:
  #	1 - Result code
  # 2 - Message

  local result=$1
  local cli_info=$2

  if [[ $result -ne 0 ]]; then
    fail "${cli_info} (exit-code=$result)"
  else
    log "Step $STEP: Finished - OK"

    if [[ $STEP_PAUSE -eq 1 ]]; then
      echo "Step finished. Please check the output above and press Enter key to continue or Ctrl+C to stop."
      read
    fi

    return 0
  fi
}

function new_step() {
	#	Parameters:
	#	1 - Message
  log "Step $((++STEP)): ${1}"
}

function end_step() {
	#	Parameters:
	#	1 - Result code
  # 2 - Message

  check_command $1 "${2}"
}

function run_step() {
	#	Parameters:
	#	1 - Description
  # 2 - Command

  new_step "${1}"
  eval "${2}"
  end_step $? "${2}"
}

function bool_to_text() {
	#	Parameters:
	#	1 - boolean

  if [[ $1 -eq 1 ]]; then
    echo "Yes"
  else
    echo "No"
  fi
}

function trim_spaces() {
  local var="${1}"
  # remove leading spaces
  var="${var#"${var%%[![:space:]]*}"}"
  # remove trailing spaces
  var="${var%"${var##*[![:space:]]}"}"
  echo ${var}
}

function check_qt_arch_support() {
  if [[ "$MSYSTEM" == "MINGW32" ]] && [[ $QT_VERSION_MAJOR -gt 5 ]]; then
    fail "Qt versions later than 5 do not support 32-bit architectures"
  fi
}

function get_qt_version() {
	#	Parameters:
	#	1 - EdgeTX version

  # Return dependent Qt version

  local etxarr
  local supparr
  local found=0

  IFS='.' read -ra etxarr <<< "${1}"

  for ((i = 0; i < ${#supported_edgetx_versions[@]}; ++i)); do
    found=1
    IFS='.' read -ra supparr <<< "${supported_edgetx_versions[i]}"

    for ((j = 0; j < ${#supparr[@]}; ++j)); do
      if [[ "${etxarr[j]}" != "${supparr[j]}" ]]; then
        found=0
        break
      fi
    done

    if [[ $found -eq 1 ]]; then
      echo ${supported_qt_versions[i]}
      return 0
    fi
  done

  echo ""
  return 1
}

function split_version() {
	#	Parameters:
	#	1 - version variable in format major.minor[.patch]

  # Output 3 variables based on version variable parameter

  local vers
  local arr
  eval "vers=\${$1}"
  IFS='.' read -ra arr <<< "${vers}"

  if [[ ${#arr[@]} -gt 0 ]]; then eval "${1}_MAJOR=${arr[0]}"; fi
  if [[ ${#arr[@]} -gt 1 ]]; then eval "${1}_MINOR=${arr[1]}"; fi
  if [[ ${#arr[@]} -gt 2 ]]; then eval "${1}_PATCH=${arr[2]}"; else eval "${1}_PATCH=0"; fi
}

function validate_version() {
	#	Parameters:
	#	1 - product
  # 2 - version to check
  # 3 - version array

  local prod="${1}"
  local checkver="${2}"
  shift 2
  local arr=("$@")

  for arrver in "${arr}"
  do
    if [[ "${checkver}" == "${arrver}" ]]; then
      return 0
    fi
  done

  fail "Unsupported ${prod} version ${checkver}"
}

function validate_edgetx_version() {
  validate_version "EdgeTX" "${EDGETX_VERSION}" "${supported_edgetx_versions[@]}"
}

function validate_qt_version() {
  validate_version "Qt" "${QT_VERSION}" "${supported_qt_versions[@]}"
}
