#!/bin/bash
#
# This is a wrapper for the elf-size-report.awk script which prints a detailed report 
#   of memory segment allocations and usage in a given ELF file, based on gcc 'size' output.
# This script sets up memory segment information needed by the awk script.
# If more than one .elf file is given, then a comparison (delta memory use) report is 
#   also performed using elf-size-compare.awk.
# Currently supports some STMicro & Atmel ARM MCUs. May not work with all placement strategies.
#
# Usage:
#  [VAR=value] ./elf-memory-report.sh [options] [file.elf] [file2.elf] [...]
#
#  Where VAR is any of the CAPITALIZED variables below. More than one can be specified, 
#  and any VAR can also be set in the environment.
#    SIZE : gcc binutils 'size' command to use, default is "arm-none-eabi-size".
#    MCU  : 
#      Specifies MCU model on which to base default segment location/size parameters. Also see "--mcu=" option.
#    MEM_START_<segment> & MEM_SIZE_<segment> : (read code below for defaults)
#      Allows specifying additional/custom segment information, eg. MEM_SIZE_FLASH=768
#
#  Options can be one or more of the following:
#    --mcu=<MCU>
#      Set MCU type. Same as setting the MCU env. variable, and overrides it.
#    --skip-data
#      Some linker scripts duplicate .data in the output (eg: .data and .data_run).
#      Set this option to skip .data from being counted in RAM a 2nd time 
#      (it is always shown as being in Flash).
#    --compare-only
#      When specifying multiple elf files, only show the final comparison report.
#    --no-compare
#      Opposite and exclusive of previous option, that is, do NOT show comparison report (only size reports).
#
#  If an elf file is not specified on the command line, this script will prompt for one.
#
#  Examples:
#    MCU="STM32F439xI" MEM_SIZE_SDRAM2=8192 ./elf-memory-report.sh path/to/firmware.elf
#    ./elf-memory-report.sh --mcu ATSAM3S4C --compare-only firmware_new.elf firmware_old.elf
#    set -k && ./elf-memory-report.sh MCU=STM32F427VG --skip-data firmware.elf
#      (the -k switch allows variable assignment anywhere on the command line)
#    c:\cygwin\bin\bash -kc './elf-memory-report.sh --mcu ATSAM3S8C "c:\windows\path\with spaces\firmware.elf"'
#
# License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
# Copyright (c)2016 Maxim Paperno
# Copyright (c)2017 OpenTX
#

# -- gcc binutils 'size' command to use
: ${SIZE:="arm-none-eabi-size"}

# -- MCU type, if defined will set certain memory defaults
#    e.g. STM32F407xE or ATSAM3S4C
: ${MCU:="unknown"}

# -- process options
compare=2
skip_data=0
for i in "$@" ; do
  case $i in
    --mcu=*) MCU="${i#*=}"; shift;;
    --skip-data) skip_data=1; shift;;
    --compare-only) compare=1; shift;;
    --no-compare) compare=0; shift;;
  esac
done

# -- make sure gawk is installed
: ${AWKCMD:=""}
if gawk --version >/dev/null 2>&1; then
  [[ "$OSTYPE" == "darwin"* ]] && AWKCMD="gawk -nf"
else
  echo "GAWK is required, exiting."
  exit 1
fi

# -- defaults which may change based on MCU
default_flash_addr=0x08000000
default_flash_sz=1024  # KB
default_ccm_sz=0
default_ram1_sz=128
default_ram2_sz=0
default_ram3_sz=0
# -- Set defaults based on MCU type
if [[ $MCU != "unknown" ]] ; then
  mcu_brand=${MCU:0:2}  # ST or AT
  if [[ $mcu_brand == "ST" && ${#MCU} -ge 7 ]] ; then
    #                 e.g.: STM32F427VG
    stm_series=${MCU:6:1} # 4
    stm_ser_ex=${MCU:7:1} # 2
    stm_flash=${MCU:10:1} # G
    default_ram1_sz=112
    default_ram2_sz=16
    case $stm_flash in
      "I") default_flash_sz=2048;;
      "G") default_flash_sz=1024;;
      "E") default_flash_sz=512;;
      "C") default_flash_sz=256;;
    esac
    if (( $stm_series == 4 )) ; then
      default_ccm_sz=64
      if (( $stm_ser_ex >= 2 )) ; then
        default_ram3_sz=64
      fi
    fi
  elif [[ $mcu_brand == "AT" && ${#MCU} -ge 9 ]] ; then
    #                 e.g.: ATSAM3S4C
    atm_series=${MCU:5:4} # 3S4C
    default_flash_addr=0x00400000
    case $atm_series in
      "3S8C") default_flash_sz=512; default_ram1_sz=64;;
      "3S4C") default_flash_sz=256; default_ram1_sz=48;;
      "3S2C") default_flash_sz=128; default_ram1_sz=32;;
    esac
  else
    echo "MCU type unrecognized, exiting."
    exit 1
  fi
fi

# -- target memory locations in hex
: ${MEM_START_FLASH:=$default_flash_addr}
: ${MEM_START_CCRAM:=0x10000000}
: ${MEM_START_SRAM1:=0x20000000}
: ${MEM_START_SRAM2:=0x2001c000}
: ${MEM_START_SRAM3:=0x20020000}
: ${MEM_START_PSRAM1:=0x60000000}
: ${MEM_START_SDRAM1:=0xC0000000}
: ${MEM_START_SDRAM2:=0xD0000000}

# -- target memory sizes in KB
: ${MEM_SIZE_FLASH:=$default_flash_sz}
: ${MEM_SIZE_CCRAM:=$default_ccm_sz}
: ${MEM_SIZE_SRAM1:=$default_ram1_sz}
: ${MEM_SIZE_SRAM2:=$default_ram2_sz}
: ${MEM_SIZE_SRAM3:=$default_ram3_sz}
: ${MEM_SIZE_PSRAM1:=0}
: ${MEM_SIZE_SDRAM1:=0}
: ${MEM_SIZE_SDRAM2:=0}

# -- prompt for elf file if not on command line
if [[ -z "$@" ]] ; then
  read -p "Elf file(s): " ELF_FILES
else
  ELF_FILES=$@
fi

# -- Set arrays for awk script. The segments should be in increasing address order.
names=("Flash" "CCRAM" "SRAM1" "SRAM2" "SRAM3" "PSRAM1" "SDRAM1" "SDRAM2")
addrs=(${MEM_START_FLASH} ${MEM_START_CCRAM} ${MEM_START_SRAM1} ${MEM_START_SRAM2} ${MEM_START_SRAM3} \
  ${MEM_START_PSRAM1} ${MEM_START_SDRAM1} ${MEM_START_SDRAM2})
sizes=(${MEM_SIZE_FLASH} ${MEM_SIZE_CCRAM} ${MEM_SIZE_SRAM1} ${MEM_SIZE_SRAM2} ${MEM_SIZE_SRAM3} \
  ${MEM_SIZE_PSRAM1} ${MEM_SIZE_SDRAM1} ${MEM_SIZE_SDRAM2})

base_dir="$(dirname "$BASH_SOURCE")"
base_dir="${base_dir%%[[:space:]]}" # sometimes we get trailing \n

# -- run the ARM size command for each elf file and send output to awk script
all_output=""
file_cnt=0
for elf in "$@"; do
  if [[ ! -f "$elf" ]] ; then
    echo "ELF file \"$elf\" not found."
    continue
  fi
  #${SIZE} -A -x ${elf}
  escelf="${elf//\\//}"
  output=`${SIZE} -A -x ${elf} | ${AWKCMD} "${base_dir}/elf-size-report.awk" \
      -v mcu="${MCU}" \
      -v elf="${escelf}" \
      -v skip_data=${skip_data} \
      -v names="${names[*]}" \
      -v addrs="${addrs[*]}" \
      -v sizes="${sizes[*]}"`
  if (( compare != 1 )) ; then
    echo "$output"
    echo
  fi
  file_cnt+=1
  all_output+="${output}\n"
done
# -- if more than one elf file was given, also run a comparison report
if (( file_cnt > 1 && compare )) ; then
  echo "$all_output" | ${AWKCMD} "${base_dir}/elf-size-compare.awk"
fi

