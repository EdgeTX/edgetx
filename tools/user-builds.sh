#!/usr/bin/env bash
set -euo pipefail

cleanup() {
  stty sane 2>/dev/null || true
}

trap 'echo; echo "Cancelled."; cleanup; return 130 2>/dev/null || exit 130' INT
trap 'cleanup' EXIT

if [[ -n "${BASH_SOURCE:-}" ]]; then
  script_path="${BASH_SOURCE[0]}"
elif [[ -n "${ZSH_VERSION:-}" ]]; then
  script_path="${(%):-%N}"
else
  script_path="$0"
fi

script_dir="$(cd "$(dirname "${script_path}")" && pwd)"
root_dir="$(cd "${script_dir}/.." && pwd)"
root_cmake="${root_dir}/CMakeLists.txt"

radios_config="${script_dir}/radio-targets.tsv"
firmware_options_file="${script_dir}/radio-firmware-options.tsv"
use_fzf=1
preselect=""
saved_idx=""
saved_build_choice="2"
saved_clean_choice="n"
saved_fw_options=""
saved_use_custom_options="n"
config_loaded=0
user_builds_dir="${root_dir}/user-builds"
config_file="${user_builds_dir}/.user-builds-config"
declare -a custom_cmake_args=()
declare -a custom_option_labels=()
declare -a firmware_option_names=()
declare -a firmware_option_labels=()
last_marker_ansi=$'\033[33m(last)\033[0m'

strip_ansi() {
  # Strip ANSI escape sequences from fzf-selected lines before comparison/parsing.
  printf '%s' "$1" | sed -E $'s/\x1B\\[[0-9;]*[A-Za-z]//g'
}

hash_text() {
  local input_text="$1"
  local digest=""
  if command -v shasum >/dev/null 2>&1; then
    digest="$(printf '%s' "${input_text}" | shasum -a 256 | awk '{print $1}')"
  elif command -v sha256sum >/dev/null 2>&1; then
    digest="$(printf '%s' "${input_text}" | sha256sum | awk '{print $1}')"
  elif command -v md5 >/dev/null 2>&1; then
    digest="$(printf '%s' "${input_text}" | md5 | awk '{print $NF}')"
  else
    digest="$(printf '%s' "${input_text}" | cksum | awk '{print $1}')"
  fi
  printf '%s' "${digest}"
}

save_user_builds_config() {
  mkdir -p "${user_builds_dir}"
  {
    printf "LAST_RADIO=%s\n" "${saved_idx}"
    printf "LAST_BUILD_TYPE=%s\n" "${saved_build_choice}"
    printf "LAST_CLEAN_BUILD=%s\n" "${saved_clean_choice}"
    printf "LAST_USE_CUSTOM_OPTIONS=%s\n" "${saved_use_custom_options}"
    printf "LAST_FW_OPTIONS=%s\n" "${saved_fw_options}"
  } > "${config_file}"
}

option_matches_filter() {
  local filter_raw="$1"
  local selected_pcb="$2"
  local selected_pcbrev="$3"
  local selected_pair=""
  local token=""
  local token_upper=""
  local pcb_upper="$(printf '%s' "${selected_pcb}" | tr '[:lower:]' '[:upper:]')"
  local pcbrev_upper="$(printf '%s' "${selected_pcbrev}" | tr '[:lower:]' '[:upper:]')"

  if [[ -n "${selected_pcbrev}" ]]; then
    selected_pair="${selected_pcb}:${selected_pcbrev}"
  fi
  local pair_upper="$(printf '%s' "${selected_pair}" | tr '[:lower:]' '[:upper:]')"

  # Empty filter means global option.
  if [[ -z "${filter_raw//[[:space:]]/}" ]]; then
    return 0
  fi

  IFS=',' read -r -a filter_tokens <<< "${filter_raw}"
  for token in "${filter_tokens[@]}"; do
    token="$(printf '%s' "${token}" | tr -d '[:space:]')"
    [[ -z "${token}" ]] && continue
    token_upper="$(printf '%s' "${token}" | tr '[:lower:]' '[:upper:]')"
    if [[ "${token_upper}" == *:* ]]; then
      [[ -n "${pair_upper}" && "${token_upper}" == "${pair_upper}" ]] && return 0
    else
      [[ "${token_upper}" == "${pcb_upper}" ]] && return 0
      [[ -n "${pcbrev_upper}" && "${token_upper}" == "${pcbrev_upper}" ]] && return 0
    fi
  done

  return 1
}

fw_major="$(awk -F'"' '/set\(VERSION_MAJOR/{print $2; exit}' "${root_cmake}")"
fw_minor="$(awk -F'"' '/set\(VERSION_MINOR/{print $2; exit}' "${root_cmake}")"
fw_revision="$(awk -F'"' '/set\(VERSION_REVISION/{print $2; exit}' "${root_cmake}")"
fw_semver="${fw_major}.${fw_minor}.${fw_revision}"
if [[ -n "${EDGETX_VERSION_TAG:-}" ]]; then
  fw_version="${EDGETX_VERSION_TAG}"
else
  fw_prefix="${EDGETX_VERSION_PREFIX:-pre}"
  fw_suffix="${EDGETX_VERSION_SUFFIX:-selfbuild}"
  fw_version="${fw_prefix}-${fw_semver}-${fw_suffix}"
fi
app_version_label="${fw_semver}"
if [[ -n "${EDGETX_VERSION_SUFFIX:-}" ]]; then
  app_version_label="${app_version_label}-${EDGETX_VERSION_SUFFIX}"
elif [[ -z "${EDGETX_VERSION_TAG:-}" ]]; then
  app_version_label="${app_version_label}-selfbuild"
fi

while [[ $# -gt 0 ]]; do
  case "$1" in
    --txt)
      use_fzf=0
      ;;
    --select=*)
      preselect="${1#*=}"
      ;;
    --select)
      shift
      preselect="${1:-}"
      ;;
  esac
  shift
done

if [[ ! -f "${radios_config}" ]]; then
  echo "Missing radios config: ${radios_config}"
  echo "Generate it with:"
  echo "  tools/radio-targets.py --export-tsv tools/radio-targets.tsv"
  exit 1
fi

if [[ -f "${config_file}" ]]; then
  config_loaded=1
  config_kv_found=0
  while IFS= read -r config_line; do
    [[ -z "${config_line}" ]] && continue
    if [[ "${config_line}" == *=* ]]; then
      config_kv_found=1
      config_key="${config_line%%=*}"
      config_val="${config_line#*=}"
      case "${config_key}" in
        LAST_RADIO)
          saved_idx="${config_val}"
          ;;
        LAST_BUILD_TYPE)
          saved_build_choice="${config_val}"
          ;;
        LAST_CLEAN_BUILD)
          saved_clean_choice="${config_val}"
          ;;
        LAST_USE_CUSTOM_OPTIONS)
          saved_use_custom_options="${config_val}"
          ;;
        LAST_FW_OPTIONS)
          saved_fw_options="${config_val}"
          ;;
      esac
    fi
  done < "${config_file}"
  # Backward compatibility: old config had only radio index.
  if [[ "${config_kv_found}" -eq 0 ]]; then
    saved_idx="$(tr -d '[:space:]' < "${config_file}")"
  fi
fi

if [[ -d "${root_dir}/build" ]]; then
  if [[ "${use_fzf}" -eq 1 && -t 1 ]] && command -v fzf >/dev/null 2>&1; then
    clean_prompt="Do you want clean build? (build folder removed) "
    if [[ "${saved_clean_choice}" == "y" ]]; then
      if [[ "${config_loaded}" -eq 1 ]]; then
        clean_selection="$(printf "Yes %b\nNo\n" "${last_marker_ansi}" | fzf --ansi --height=20% --reverse --prompt="${clean_prompt}" --no-multi || true)"
      else
        clean_selection="$(printf "Yes (default)\nNo\n" | fzf --height=20% --reverse --prompt="${clean_prompt}" --no-multi || true)"
      fi
    else
      if [[ "${config_loaded}" -eq 1 ]]; then
        clean_selection="$(printf "No %b\nYes\n" "${last_marker_ansi}" | fzf --ansi --height=20% --reverse --prompt="${clean_prompt}" --no-multi || true)"
      else
        clean_selection="$(printf "No (default)\nYes\n" | fzf --height=20% --reverse --prompt="${clean_prompt}" --no-multi || true)"
      fi
    fi
    if [[ -z "${clean_selection}" ]]; then
      return 0 2>/dev/null || exit 0
    fi
    clean_selection_plain="$(strip_ansi "${clean_selection}")"
    if [[ "${clean_selection_plain}" == Yes* ]]; then
      saved_clean_choice="y"
      rm -rf "${root_dir}/build"
      echo "Removed ${root_dir}/build"
    else
      saved_clean_choice="n"
    fi
  else
    if [[ "${saved_clean_choice}" == "y" ]]; then
      printf "Do you want clean build? [Y/n]: "
    else
      printf "Do you want clean build? [y/N]: "
    fi
    read -r clean_answer
    if [[ -z "${clean_answer}" ]]; then
      clean_answer="${saved_clean_choice}"
    fi
    if [[ "${clean_answer}" =~ ^[Yy]$ ]]; then
      saved_clean_choice="y"
      rm -rf "${root_dir}/build"
      echo "Removed ${root_dir}/build"
    else
      saved_clean_choice="n"
    fi
  fi
fi

if [[ "${use_fzf}" -eq 1 && -t 1 ]] && command -v fzf >/dev/null 2>&1; then
  if [[ "${saved_build_choice}" == "1" ]]; then
    if [[ "${config_loaded}" -eq 1 ]]; then
      build_selection="$(printf "1. Firmware %b\n2. Simulator\n3. Companion\n" "${last_marker_ansi}" | fzf --ansi --height=35% --reverse --prompt="Select build type: " --no-multi || true)"
    else
      build_selection="$(printf "1. Firmware (default)\n2. Simulator\n3. Companion\n" | fzf --height=35% --reverse --prompt="Select build type: " --no-multi || true)"
    fi
  elif [[ "${saved_build_choice}" == "3" ]]; then
    if [[ "${config_loaded}" -eq 1 ]]; then
      build_selection="$(printf "3. Companion %b\n2. Simulator\n1. Firmware\n" "${last_marker_ansi}" | fzf --ansi --height=35% --reverse --prompt="Select build type: " --no-multi || true)"
    else
      build_selection="$(printf "3. Companion (default)\n2. Simulator\n1. Firmware\n" | fzf --height=35% --reverse --prompt="Select build type: " --no-multi || true)"
    fi
  else
    if [[ "${config_loaded}" -eq 1 ]]; then
      build_selection="$(printf "2. Simulator %b\n1. Firmware\n3. Companion\n" "${last_marker_ansi}" | fzf --ansi --height=35% --reverse --prompt="Select build type: " --no-multi || true)"
    else
      build_selection="$(printf "2. Simulator (default)\n1. Firmware\n3. Companion\n" | fzf --height=35% --reverse --prompt="Select build type: " --no-multi || true)"
    fi
  fi
  if [[ -z "${build_selection}" ]]; then
    return 0 2>/dev/null || exit 0
  fi
  build_selection_plain="$(strip_ansi "${build_selection}")"
  build_choice="$(echo "${build_selection_plain}" | awk '{print $1}' | sed 's/\.//')"
else
  echo "Build type:"
  echo "  1) Firmware"
  echo "  2) Simulator"
  echo "  3) Companion"
  printf "Select build type [%s]: " "${saved_build_choice}"
  read -r build_choice
  build_choice="${build_choice:-${saved_build_choice}}"
fi
saved_build_choice="${build_choice}"

case "${build_choice}" in
  1)
    build_target="firmware"
    build_label="firmware"
    ;;
  2)
    build_target="libsimulator"
    build_label="simulator"
    ;;
  3)
    build_target="libsimulator"
    build_label="companion"
    ;;
  *)
    echo "Invalid build type selection."
    exit 1
    ;;
esac

# Firmware builds require the ARM cross-compiler toolchain.
if [[ "${build_target}" == "firmware" ]]; then
  if ! command -v arm-none-eabi-gcc >/dev/null 2>&1 || ! command -v arm-none-eabi-g++ >/dev/null 2>&1; then
    echo "Missing required ARM toolchain for firmware builds:"
    echo "  - arm-none-eabi-gcc"
    echo "  - arm-none-eabi-g++"
    echo "Install the GNU Arm Embedded Toolchain and ensure those binaries are in PATH."
    exit 1
  fi

  # EdgeTX firmware build is pinned to Arm GNU Toolchain 14.2.rel1 (compiler version 14.2.1).
  required_arm_gcc_version="14.2.1"
  arm_gpp_version="$(arm-none-eabi-g++ -dumpfullversion -dumpversion 2>/dev/null || true)"
  if [[ -z "${arm_gpp_version}" ]]; then
    arm_gpp_version="$(arm-none-eabi-g++ --version 2>/dev/null | awk 'NR==1 {print $NF}')"
  fi
  if [[ "${arm_gpp_version}" != "${required_arm_gcc_version}" ]]; then
    echo "Unsupported ARM toolchain version: ${arm_gpp_version:-unknown}"
    echo "Required version: ${required_arm_gcc_version} (Arm GNU Toolchain 14.2.rel1)"
    echo "Install/update arm-none-eabi toolchain and ensure the correct version is first in PATH."
    exit 1
  fi
fi

if [[ -z "${preselect}" ]]; then
  if [[ "${saved_idx}" =~ ^[0-9]+$ ]]; then
    preselect="${saved_idx}"
  fi
fi

if [[ -n "${preselect}" && "${preselect}" =~ ^[0-9]+$ && "${saved_idx}" != "${preselect}" ]]; then
  idx="${preselect}"
elif [[ "${use_fzf}" -eq 1 && -t 1 ]] && command -v fzf >/dev/null 2>&1; then
  radio_lines="$(awk -F '\t' '
    NF >= 3 {
      disp = ""
      if ($5 != "" || $6 != "") {
        disp = " (" $5
        if ($5 != "" && $6 != "") {
          disp = disp " "
        }
        disp = disp $6 ")"
      }
      printf "%s. %s%s\n", $1, $2, disp
    }
  ' "${radios_config}")"
  if [[ -n "${preselect}" && "${preselect}" =~ ^[0-9]+$ ]]; then
    radio_lines="$(printf '%s\n' "${radio_lines}" | awk -v n="${preselect}." '
      $1 == n { selected = $0; next }
      { others[++count] = $0 }
      END {
        if (selected != "") print selected
        for (i = 1; i <= count; i++) print others[i]
      }
    ')"
    radio_lines="$(
      while IFS= read -r radio_line; do
        if [[ "${radio_line}" == "${preselect}."* ]]; then
          printf "%s %b\n" "${radio_line}" "${last_marker_ansi}"
        else
          printf "%s\n" "${radio_line}"
        fi
      done <<< "${radio_lines}"
    )"
  else
    :
  fi
  selection="$(printf '%s\n' "${radio_lines}" | fzf --ansi --height=40% --reverse --prompt="Select radio: " --no-multi || true)"
  if [[ -z "${selection}" ]]; then
    return 0 2>/dev/null || exit 0
  fi
  idx="$(echo "${selection}" | awk '{print $1}' | sed 's/\.//')"
else
  echo "Available radios:"
  radio_lines_txt="$(
    awk -F '\t' '
    NF >= 3 {
      disp = ""
      if ($5 != "" || $6 != "") {
        disp = " (" $5
        if ($5 != "" && $6 != "") {
          disp = disp " "
        }
        disp = disp $6 ")"
      }
      printf "%3d. %s%s\n", $1, $2, disp
    }
  ' "${radios_config}"
  )"
  if [[ -n "${preselect}" && "${preselect}" =~ ^[0-9]+$ ]]; then
    while IFS= read -r radio_line; do
      trimmed="$(printf '%s' "${radio_line}" | sed -E 's/^[[:space:]]+//')"
      if [[ "${trimmed}" == "${preselect}."* ]]; then
        printf "%s %b\n" "${radio_line}" "${last_marker_ansi}"
      else
        printf "%s\n" "${radio_line}"
      fi
    done <<< "${radio_lines_txt}"
  else
    printf "%s\n" "${radio_lines_txt}"
  fi
  echo
  if [[ -n "${preselect}" ]]; then
    printf "Select radio [%s]: " "${preselect}"
  else
    printf "Select radio: "
  fi
  read -r idx
  idx="${idx:-${preselect}}"
fi

if [[ -z "${idx}" ]]; then
  return 0 2>/dev/null || exit 0
fi

if [[ ! "${idx}" =~ ^[0-9]+$ ]]; then
  echo "Invalid selection."
  exit 1
fi

mkdir -p "${user_builds_dir}"
saved_idx="${idx}"

selected_row="$(awk -F '\t' -v n="${idx}" '$1 == n {print $3 "\t" $4; exit}' "${radios_config}")"
if [[ -z "${selected_row}" ]]; then
  echo "Failed to resolve selection from ${radios_config}."
  exit 1
fi
selected_radio_name="$(awk -F '\t' -v n="${idx}" '$1 == n {print $2; exit}' "${radios_config}")"

pcb="$(printf '%s\n' "${selected_row}" | awk -F '\t' '{print $1}')"
pcbrev="$(printf '%s\n' "${selected_row}" | awk -F '\t' '{print $2}')"

export EDGETX_PCB="${pcb}"
export EDGETX_PCBREV="${pcbrev:-}"

if [[ "${build_target}" == "firmware" && -f "${firmware_options_file}" ]]; then
  use_custom_options="${saved_use_custom_options}"
  if [[ "${use_fzf}" -eq 1 && -t 1 ]] && command -v fzf >/dev/null 2>&1; then
    if [[ "${saved_use_custom_options}" == "y" ]]; then
      custom_choice="$(printf "Yes %b\nNo\n" "${last_marker_ansi}" | fzf --ansi --height=20% --reverse --prompt="Use custom firmware options? " --no-multi || true)"
    else
      if [[ "${config_loaded}" -eq 1 ]]; then
        custom_choice="$(printf "No %b\nYes\n" "${last_marker_ansi}" | fzf --ansi --height=20% --reverse --prompt="Use custom firmware options? " --no-multi || true)"
      else
        custom_choice="$(printf "No (default)\nYes\n" | fzf --height=20% --reverse --prompt="Use custom firmware options? " --no-multi || true)"
      fi
    fi
    if [[ -z "${custom_choice}" ]]; then
      return 0 2>/dev/null || exit 0
    fi
    custom_choice_plain="$(strip_ansi "${custom_choice}")"
    if [[ "${custom_choice_plain}" == Yes* ]]; then
      use_custom_options="y"
    else
      use_custom_options="n"
    fi
  else
    if [[ "${saved_use_custom_options}" == "y" ]]; then
      printf "Use custom firmware options? [Y/n]: "
    else
      printf "Use custom firmware options? [y/N]: "
    fi
    read -r custom_choice_text
    if [[ -z "${custom_choice_text}" ]]; then
      custom_choice_text="${saved_use_custom_options}"
    fi
    if [[ "${custom_choice_text}" =~ ^[Yy]$ ]]; then
      use_custom_options="y"
    else
      use_custom_options="n"
    fi
  fi
  saved_use_custom_options="${use_custom_options}"

  if [[ "${use_custom_options}" != "y" ]]; then
    custom_cmake_args=()
    custom_option_labels=()
  else
  firmware_option_names=()
  firmware_option_labels=()
  firmware_option_count=0
  while IFS=$'\t' read -r option_label option_value option_filter; do
    [[ -z "${option_label}" || -z "${option_value}" ]] && continue
    if option_matches_filter "${option_filter:-}" "${EDGETX_PCB}" "${EDGETX_PCBREV}"; then
      if [[ "${option_value}" =~ ^-D([A-Za-z0-9_]+)=(ON|OFF)$ ]]; then
        option_name="${BASH_REMATCH[1]}"
      elif [[ "${option_value}" =~ ^-D([A-Za-z0-9_]+)$ ]]; then
        option_name="${BASH_REMATCH[1]}"
      elif [[ "${option_value}" =~ ^([A-Za-z0-9_]+)$ ]]; then
        option_name="${BASH_REMATCH[1]}"
      else
        continue
      fi
      option_base_label="${option_label#Enable }"
      option_base_label="${option_base_label#Disable }"
      if [[ -z "${option_base_label}" ]]; then
        option_base_label="${option_name}"
      fi
      option_exists=0
      if [[ "${firmware_option_count}" -gt 0 ]]; then
        for ((i=0; i<firmware_option_count; i++)); do
          if [[ "${firmware_option_names[$i]}" == "${option_name}" ]]; then
            option_exists=1
            break
          fi
        done
      fi
      if [[ "${option_exists}" -eq 0 ]]; then
        firmware_option_names[$firmware_option_count]="${option_name}"
        firmware_option_labels[$firmware_option_count]="${option_base_label}"
        firmware_option_count=$((firmware_option_count + 1))
      fi
    fi
  done < <(awk -F '\t' 'NF >= 2 && $1 !~ /^#/ { filter = (NF >= 3 ? $3 : ""); printf "%s\t%s\t%s\n", $1, $2, filter }' "${firmware_options_file}")

  if [[ "${firmware_option_count}" -gt 0 ]]; then
    if [[ "${use_fzf}" -eq 1 && -t 1 ]] && command -v fzf >/dev/null 2>&1; then
      declare -a firmware_option_states=()
      for ((i=0; i<firmware_option_count; i++)); do
        firmware_option_states[$i]="default"
        if [[ -n "${saved_fw_options}" ]]; then
          IFS=';' read -r -a saved_fw_entries <<< "${saved_fw_options}"
          for saved_fw_entry in "${saved_fw_entries[@]}"; do
            [[ -z "${saved_fw_entry}" ]] && continue
            saved_fw_name="${saved_fw_entry%%=*}"
            saved_fw_state="${saved_fw_entry#*=}"
            if [[ "${saved_fw_name}" == "${firmware_option_names[$i]}" && ( "${saved_fw_state}" == "on" || "${saved_fw_state}" == "off" ) ]]; then
              firmware_option_states[$i]="${saved_fw_state}"
              break
            fi
          done
        fi
      done
      options_query=""

      while true; do
        menu_file="$(mktemp)"
        for ((i=0; i<firmware_option_count; i++)); do
          printf "%s\t%s\t%s\t%s\n" "${i}" "${firmware_option_states[$i]}" "${firmware_option_labels[$i]}" "${firmware_option_names[$i]}" >> "${menu_file}"
        done
        export FIRMWARE_OPTIONS_MENU_FILE="${menu_file}"
        render_script="$(mktemp)"
        cat > "${render_script}" <<'EOF'
#!/usr/bin/env bash
set -euo pipefail
q="${FZF_QUERY:-}"
menu="${FIRMWARE_OPTIONS_MENU_FILE:-}"
[[ -f "${menu}" ]] || exit 0

printf "[Done] Continue\t__DONE__\t-1\n"
while IFS=$'\t' read -r idx state label name; do
  [[ "${state}" == "default" ]] && continue
  case "${state}" in
    on)  color=$'\033[32m'; tag="[ON]" ;;
    off) color=$'\033[31m'; tag="[OFF]" ;;
    *)   color=$'\033[90m'; tag="[Default]" ;;
  esac
  printf "%b%s%b %s\t%s\t%s\n" "${color}" "${tag}" $'\033[0m' "${label}" "${name}" "${idx}"
done < "${menu}"

q_lc="$(printf "%s" "${q}" | tr '[:upper:]' '[:lower:]')"
while IFS=$'\t' read -r idx state label name; do
  [[ "${state}" != "default" ]] && continue
  if [[ -n "${q_lc}" ]]; then
    line_lc="$(printf "%s %s" "${label}" "${name}" | tr '[:upper:]' '[:lower:]')"
    [[ "${line_lc}" == *"${q_lc}"* ]] || continue
  fi
  printf "%b%s%b %s\t%s\t%s\n" $'\033[90m' "[Default]" $'\033[0m' "${label}" "${name}" "${idx}"
done < "${menu}"
EOF
        chmod +x "${render_script}"
        bind_cmd="start:reload(${render_script}),change:reload(${render_script})"

        selection_with_query="$(
          printf '' | fzf --ansi --disabled --print-query --query="${options_query}" --height=55% --reverse --delimiter=$'\t' --with-nth=1 --prompt="Set firmware options (Enter toggles, Done to continue): " --no-multi --no-sort --bind "${bind_cmd}" || true
        )"
        rm -f "${menu_file}"
        rm -f "${render_script}"
        unset FIRMWARE_OPTIONS_MENU_FILE

        if [[ -z "${selection_with_query}" ]]; then
          return 0 2>/dev/null || exit 0
        fi

        options_query="$(printf '%s\n' "${selection_with_query}" | sed -n '1p')"
        firmware_options_selection="$(printf '%s\n' "${selection_with_query}" | sed -n '2p')"
        if [[ -z "${firmware_options_selection}" ]]; then
          return 0 2>/dev/null || exit 0
        fi

        selected_key="$(printf '%s\n' "${firmware_options_selection}" | awk -F '\t' '{print $2}')"
        selected_idx="$(printf '%s\n' "${firmware_options_selection}" | awk -F '\t' '{print $3}')"
        if [[ "${selected_key}" == "__DONE__" ]]; then
          break
        fi
        if [[ ! "${selected_idx}" =~ ^[0-9]+$ ]]; then
          continue
        fi

        current_state="${firmware_option_states[$selected_idx]}"
        if [[ "${current_state}" == "default" ]]; then
          firmware_option_states[$selected_idx]="on"
        elif [[ "${current_state}" == "on" ]]; then
          firmware_option_states[$selected_idx]="off"
        else
          firmware_option_states[$selected_idx]="default"
        fi
      done

      for ((i=0; i<firmware_option_count; i++)); do
        option_name="${firmware_option_names[$i]}"
        option_label="${firmware_option_labels[$i]}"
        option_state="${firmware_option_states[$i]}"
        if [[ "${option_state}" == "on" ]]; then
          custom_cmake_args+=( "-D${option_name}=ON" )
          custom_option_labels+=( "${option_label}=ON" )
        elif [[ "${option_state}" == "off" ]]; then
          custom_cmake_args+=( "-D${option_name}=OFF" )
          custom_option_labels+=( "${option_label}=OFF" )
        fi
      done
      saved_fw_options=""
      for ((i=0; i<firmware_option_count; i++)); do
        option_state="${firmware_option_states[$i]}"
        [[ "${option_state}" == "default" ]] && continue
        if [[ -z "${saved_fw_options}" ]]; then
          saved_fw_options="${firmware_option_names[$i]}=${option_state}"
        else
          saved_fw_options="${saved_fw_options};${firmware_option_names[$i]}=${option_state}"
        fi
      done
    else
      echo "Extra firmware options:"
      for ((i=0; i<firmware_option_count; i++)); do
        option_label="${firmware_option_labels[$i]}"
        printf "  %2d) %s\n" "$((i + 1))" "${option_label}"
      done
      printf "Select options (comma-separated numbers, Enter for none): "
      read -r firmware_options_input
      if [[ -n "${firmware_options_input}" ]]; then
        IFS=',' read -r -a firmware_option_numbers <<< "${firmware_options_input}"
        for option_number_raw in "${firmware_option_numbers[@]}"; do
          option_number="$(printf '%s' "${option_number_raw}" | tr -d '[:space:]')"
          [[ -z "${option_number}" ]] && continue
          if [[ ! "${option_number}" =~ ^[0-9]+$ ]]; then
            continue
          fi
          option_idx=$((option_number - 1))
          if [[ "${option_idx}" -lt 0 || "${option_idx}" -ge "${firmware_option_count}" ]]; then
            continue
          fi
          option_name="${firmware_option_names[$option_idx]}"
          option_label="${firmware_option_labels[$option_idx]}"
          printf "%s [d=default/on/off] [d]: " "${option_label}"
          read -r option_state_input
          option_state="$(printf '%s' "${option_state_input}" | tr '[:upper:]' '[:lower:]')"
          case "${option_state:-d}" in
            on|o)
              custom_cmake_args+=( "-D${option_name}=ON" )
              custom_option_labels+=( "${option_label}=ON" )
              ;;
            off|f)
              custom_cmake_args+=( "-D${option_name}=OFF" )
              custom_option_labels+=( "${option_label}=OFF" )
              ;;
            *)
              ;;
          esac
        done
      fi
      saved_fw_options=""
      for option_arg in "${custom_cmake_args[@]}"; do
        if [[ "${option_arg}" =~ ^-D([A-Za-z0-9_]+)=(ON|OFF)$ ]]; then
          option_name="${BASH_REMATCH[1]}"
          option_state="$(printf '%s' "${BASH_REMATCH[2]}" | tr '[:upper:]' '[:lower:]')"
          if [[ -z "${saved_fw_options}" ]]; then
            saved_fw_options="${option_name}=${option_state}"
          else
            saved_fw_options="${saved_fw_options};${option_name}=${option_state}"
          fi
        fi
      done
    fi
  fi
  fi
fi

save_user_builds_config

echo "Building: ${build_label}"
if [[ -n "${selected_radio_name}" ]]; then
  echo "Radio: ${selected_radio_name}"
else
  echo "Radio: ${model_id:-${EDGETX_PCB}}"
fi
echo "Options:"
if [[ ${#custom_option_labels[@]} -gt 0 ]]; then
  for option_label in "${custom_option_labels[@]}"; do
    option_name="${option_label%=*}"
    option_value="${option_label##*=}"
    if [[ -n "${option_name}" && -n "${option_value}" && "${option_name}" != "${option_value}" ]]; then
      echo " - ${option_name} = ${option_value}"
    else
      echo " - ${option_label}"
    fi
  done
else
  echo " - none"
fi
echo
echo "Run:"
if [[ -n "${EDGETX_PCBREV}" ]]; then
  cmake_cmd=(cmake -S . -B build -DPCB="${EDGETX_PCB}" -DPCBREV="${EDGETX_PCBREV}")
else
  cmake_cmd=(cmake -S . -B build -DPCB="${EDGETX_PCB}")
fi
if [[ ${#custom_cmake_args[@]} -gt 0 ]]; then
  cmake_cmd+=( "${custom_cmake_args[@]}" )
fi
printf "  %q " "${cmake_cmd[@]}"
echo
echo "  cmake --build build --target ${build_target}"

model_id="${EDGETX_PCB}"
if [[ -n "${EDGETX_PCBREV}" ]]; then
  model_id="${EDGETX_PCBREV}"
fi
confirm_target_label="${build_label}"
if [[ "${build_target}" == "libsimulator" ]]; then
  if [[ "${build_label}" == "companion" ]]; then
    confirm_target_label="companion library"
  else
    confirm_target_label="simulator library"
  fi
fi
confirm_prompt="Configure and build ${confirm_target_label} for ${model_id}? "

if [[ "${use_fzf}" -eq 1 && -t 1 ]] && command -v fzf >/dev/null 2>&1; then
  run_selection="$(printf "Yes (default)\nNo\n" | fzf --height=20% --reverse --prompt="${confirm_prompt}" --no-multi || true)"
  if [[ -z "${run_selection}" ]]; then
    return 0 2>/dev/null || exit 0
  fi
  run_answer="y"
  if [[ "${run_selection}" == "Yes" || "${run_selection}" == "Yes (default)" ]]; then
    run_answer="y"
  elif [[ "${run_selection}" == "No" || "${run_selection}" == "No (default)" ]]; then
    run_answer="n"
  fi
else
  printf "%s[Y/n]: " "${confirm_prompt}"
  read -r run_answer
  if [[ -z "${run_answer}" ]]; then
    run_answer="y"
  fi
fi

if [[ "${run_answer}" =~ ^[Yy]$ ]]; then
  force_build_app_executable=0
  if [[ "${build_label}" == "simulator" || "${build_label}" == "companion" ]]; then
    app_target="${build_label}"
    app_macos_dir="${root_dir}/build/native/${app_target}.app/Contents/MacOS"
    existing_app_bin=""
    if [[ -d "${app_macos_dir}" ]]; then
      existing_app_bin="$(find "${app_macos_dir}" -maxdepth 1 -type f ! -name '*.dylib' | head -n 1 || true)"
    fi

    if [[ -z "${existing_app_bin}" ]]; then
      # No existing app executable yet; build simulator lib first, then app.
      build_target="libsimulator"
      force_build_app_executable=1
    else
      # Fast path: if app executable exists, only rebuild the selected radio library.
      build_target="libsimulator"
    fi
  fi

  (cd "${root_dir}" && "${cmake_cmd[@]}")
  (cd "${root_dir}" && cmake --build build --target "${build_target}")
  if [[ ( "${build_label}" == "simulator" || "${build_label}" == "companion" ) && "${force_build_app_executable}" -eq 1 ]]; then
    (cd "${root_dir}" && cmake --build build --target "${build_label}")
  fi

  target_name="${EDGETX_PCB}"
  if [[ -n "${EDGETX_PCBREV}" ]]; then
    target_name="${target_name}-${EDGETX_PCBREV}"
  fi
  pcb_slug="$(printf '%s' "${EDGETX_PCB}" | tr '[:upper:]' '[:lower:]' | tr -cs 'a-z0-9._-' '-')"
  pcbrev_slug="$(printf '%s' "${EDGETX_PCBREV}" | tr '[:upper:]' '[:lower:]' | tr -cs 'a-z0-9._-' '-')"
  target_slug="$(printf '%s' "${target_name}" | tr '[:upper:]' '[:lower:]' | tr -cs 'a-z0-9._-' '-')"
  version_slug="$(printf '%s' "${fw_version}" | tr '[:upper:]' '[:lower:]' | tr -cs 'a-z0-9._-' '-')"
  stamp="$(date +%Y%m%d-%H%M%S)"
  mkdir -p "${user_builds_dir}"
  if [[ "${build_target}" == "firmware" ]]; then
    if [[ ${#custom_cmake_args[@]} -gt 0 ]]; then
      out_dir="${user_builds_dir}/firmwares-custom"
    else
      out_dir="${user_builds_dir}/firmwares"
    fi
  else
    out_dir="${user_builds_dir}"
  fi
  mkdir -p "${out_dir}"
  copied=0

  if [[ "${build_target}" == "firmware" ]]; then
    fw_radio_slug="${target_slug}"
    if [[ -n "${pcbrev_slug}" ]]; then
      fw_radio_slug="${pcbrev_slug}"
    fi
    fw_base_name="${fw_radio_slug}-${version_slug}"
    custom_fingerprint=""
    if [[ ${#custom_cmake_args[@]} -gt 0 ]]; then
      custom_fingerprint_source="$(printf '%s\n' "${custom_cmake_args[@]}" | LC_ALL=C sort | tr '\n' ';')"
      custom_fingerprint="$(hash_text "${custom_fingerprint_source}" | cut -c1-8)"
      fw_base_name="${fw_base_name}-cust-${custom_fingerprint}"
    fi
    if [[ -f "${root_dir}/build/arm-none-eabi/firmware.bin" ]]; then
      cp "${root_dir}/build/arm-none-eabi/firmware.bin" "${out_dir}/${fw_base_name}.bin"
      copied=1
    fi
    if [[ -f "${root_dir}/build/arm-none-eabi/firmware.uf2" ]]; then
      cp "${root_dir}/build/arm-none-eabi/firmware.uf2" "${out_dir}/${fw_base_name}.uf2"
      copied=1
    fi
    if [[ -n "${custom_fingerprint}" ]]; then
      {
        echo "build_type=firmware"
        echo "radio=${selected_radio_name:-${fw_radio_slug}}"
        echo "pcb=${EDGETX_PCB}"
        echo "pcbrev=${EDGETX_PCBREV}"
        echo "version=${fw_version}"
        echo "timestamp=$(date -u +%Y-%m-%dT%H:%M:%SZ)"
        echo "fingerprint=${custom_fingerprint}"
        printf "cmake_configure="
        printf "%q " "${cmake_cmd[@]}"
        echo
        echo "cmake_build=cmake --build build --target firmware"
        echo "custom_options:"
        for option_label in "${custom_option_labels[@]}"; do
          echo " - ${option_label}"
        done
      } > "${out_dir}/${fw_base_name}.buildinfo.txt"
    fi
  else
    shopt -s nullglob
    all_sim_libs=( "${root_dir}"/build/native/libedgetx-*-simulator.* "${root_dir}"/build/native/plugins/libedgetx-*-simulator.* )
    shopt -u nullglob

    declare -a selected_sim_libs=()
    declare -a lib_name_keys=()
    declare -a key_hits=()
    if [[ -n "${pcbrev_slug}" ]]; then
      lib_name_keys+=( "${pcbrev_slug}" )
    fi
    lib_name_keys+=( "${target_slug}" "${pcb_slug}" )

    for key in "${lib_name_keys[@]}"; do
      [[ -z "${key}" ]] && continue
      shopt -s nullglob
      key_hits=( "${root_dir}"/build/native/libedgetx-"${key}"-simulator.* "${root_dir}"/build/native/plugins/libedgetx-"${key}"-simulator.* )
      shopt -u nullglob
      if [[ ${key_hits[0]+set} ]]; then
        for lib in "${key_hits[@]}"; do
          [[ -z "${lib}" ]] && continue
          skip=0
          if [[ ${selected_sim_libs[0]+set} ]]; then
            for existing in "${selected_sim_libs[@]}"; do
              if [[ "${existing}" == "${lib}" ]]; then
                skip=1
                break
              fi
            done
          fi
          if [[ "${skip}" -eq 0 ]]; then
            selected_sim_libs+=( "${lib}" )
          fi
        done
      fi
    done

    if [[ "${#selected_sim_libs[@]}" -eq 0 ]]; then
      # Some simulator targets can build the app executable without producing
      # the flavour dylib; force one libsimulator build and rescan.
      (cd "${root_dir}" && cmake --build build --target libsimulator)
      shopt -s nullglob
      all_sim_libs=( "${root_dir}"/build/native/libedgetx-*-simulator.* "${root_dir}"/build/native/plugins/libedgetx-*-simulator.* )
      shopt -u nullglob
      for key in "${lib_name_keys[@]}"; do
        [[ -z "${key}" ]] && continue
        shopt -s nullglob
        key_hits=( "${root_dir}"/build/native/libedgetx-"${key}"-simulator.* "${root_dir}"/build/native/plugins/libedgetx-"${key}"-simulator.* )
        shopt -u nullglob
        if [[ ${key_hits[0]+set} ]]; then
          for lib in "${key_hits[@]}"; do
            [[ -z "${lib}" ]] && continue
            skip=0
            if [[ ${selected_sim_libs[0]+set} ]]; then
              for existing in "${selected_sim_libs[@]}"; do
                if [[ "${existing}" == "${lib}" ]]; then
                  skip=1
                  break
                fi
              done
            fi
            if [[ "${skip}" -eq 0 ]]; then
              selected_sim_libs+=( "${lib}" )
            fi
          done
        fi
      done
    fi

    if [[ "${#selected_sim_libs[@]}" -eq 0 && "${#all_sim_libs[@]}" -gt 0 ]]; then
      # Fallback: use the newest available simulator library when naming mismatch occurs.
      newest_lib="$(ls -t "${all_sim_libs[@]}" 2>/dev/null | head -n 1 || true)"
      if [[ -n "${newest_lib}" ]]; then
        selected_sim_libs+=( "${newest_lib}" )
      fi
    fi

    app_target="${build_label}"
    app_macos_dir="${root_dir}/build/native/${app_target}.app/Contents/MacOS"
    if [[ ! -d "${app_macos_dir}" ]]; then
      echo "${app_target}.app not found; building ${app_target} executable once..."
      (cd "${root_dir}" && cmake --build build --target "${app_target}")
    fi
    app_bin="$(find "${app_macos_dir}" -maxdepth 1 -type f ! -name '*.dylib' | head -n 1 || true)"
    if [[ -z "${app_bin}" ]]; then
      echo "${app_target}.app executable missing; building ${app_target} executable once..."
      (cd "${root_dir}" && cmake --build build --target "${app_target}")
      app_bin="$(find "${app_macos_dir}" -maxdepth 1 -type f ! -name '*.dylib' | head -n 1 || true)"
      if [[ -z "${app_bin}" ]]; then
        echo "${app_target}.app is missing its executable in ${app_macos_dir}"
        exit 1
      fi
    fi

    packed=0
    up_to_date=0
    if [[ ${selected_sim_libs[0]+set} ]]; then
      for lib in "${selected_sim_libs[@]}"; do
        [[ -z "${lib}" ]] && continue
        lib_name="$(basename "${lib}")"
        app_lib="${app_macos_dir}/${lib_name}"
        if [[ -f "${app_lib}" ]]; then
          if [[ "${lib}" -nt "${app_lib}" ]]; then
            cp "${lib}" "${app_macos_dir}/"
            packed=$((packed + 1))
          else
            up_to_date=$((up_to_date + 1))
          fi
        else
          cp "${lib}" "${app_macos_dir}/"
          packed=$((packed + 1))
        fi
      done
    fi
    if [[ "${packed}" -gt 0 || "${up_to_date}" -gt 0 ]]; then
      app_bundle="${user_builds_dir}/edgetx-${app_target}-${app_version_label}.app"
      rm -rf "${app_bundle}"
      cp -R "${root_dir}/build/native/${app_target}.app" "${app_bundle}"
      icon_src="${root_dir}/companion/src/images/iconmac.icns"
      app_res_dir="${app_bundle}/Contents/Resources"
      if [[ -f "${icon_src}" ]]; then
        mkdir -p "${app_res_dir}"
        cp "${icon_src}" "${app_res_dir}/iconmac.icns"
      fi
      if [[ "${packed}" -gt 0 ]]; then
        echo "Packed/updated ${packed} selected simulator libraries into: ${app_macos_dir}"
      else
        echo "Selected simulator library already up to date in ${app_target}.app"
      fi
      echo "${app_target}.app written to: ${app_bundle}"
      copied=1
    fi
  fi

  if [[ "${copied}" -eq 1 ]]; then
    if [[ "${build_target}" == "firmware" ]]; then
      echo "Artifacts copied to: ${out_dir}"
    fi
  else
    echo "Build succeeded, but no artifacts were found to copy."
  fi
fi
