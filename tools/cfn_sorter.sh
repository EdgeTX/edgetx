#!/bin/sh

set -e
# set -x

# Required locales (check your system and install if needed):

# Linux:
# apt update && apt install locales
# locale-gen zh_CN.UTF-8 cs_CZ.UTF-8 da_DK.UTF-8 de_DE.UTF-8 es_ES.UTF-8 en_US.UTF-8 fi_FI.UTF-8 fr_FR.UTF-8 he_IL.UTF-8 it_IT.UTF-8 ja_JP.UTF-8 nl_NL.UTF-8 pl_PL.UTF-8 pt_PT.UTF-8 ru_RU.UTF-8 sv_SE.UTF-8 zh_TW.UTF-8 uk_UA.UTF-8

# zh_CN.UTF-8 / zh_CN.utf8
# cs_CZ.UTF-8 / cs_CZ.utf8
# da_DK.UTF-8 / da_DK.utf8
# de_DE.UTF-8 / de_DE.utf8
# es_ES.UTF-8 / es_ES.utf8
# en_US.UTF-8 / en_US.utf8
# fi_FI.UTF-8 / fi_FI.utf8
# fr_FR.UTF-8 / fr_FR.utf8
# he_IL.UTF-8 / he_IL.utf8
# it_IT.UTF-8 / it_IT.utf8
# ja_JP.UTF-8 / ja_JP.utf8
# nl_NL.UTF-8 / nl_NL.utf8
# pl_PL.UTF-8 / pl_PL.utf8
# pt_PT.UTF-8 / pt_PT.utf8
# ru_RU.UTF-8 / ru_RU.utf8
# sv_SE.UTF-8 / sv_SE.utf8
# zh_TW.UTF-8 / zh_TW.utf8
# uk_UA.UTF-8 / uk_UA.utf8

# Determine the compiler to use
if command -v g++ >/dev/null 2>&1; then
    CXX=g++
elif command -v gcc >/dev/null 2>&1; then
    CXX=gcc
elif command -v clang++ >/dev/null 2>&1; then
    CXX=clang++
else
    echo "Neither g++, gcc nor clang++ found. Please install a C++ compiler."
    exit 1
fi

# Compile specified translation macro, append to cfn_sort.cpp
compile_and_append() {
  local lng_macro=$1
  local translation_macro=$2
  local condition=$3

  echo "Compiling ${translation_macro} (${lng_macro}) ..."

  $CXX -std=c++11 -lstdc++ -Wfatal-errors -D${lng_macro} tools/cfn_sorter.cpp
  {
    if [ "$condition" = "else" ]; then
      echo "#${condition}"
    else
      echo "#${condition} defined(${translation_macro})"
    fi
    ./a.out
  } >> radio/src/cfn_sort.cpp
}

rm radio/src/cfn_sort.cpp

# Copyright header
cat tools/copyright-header.txt > radio/src/cfn_sort.cpp

# Start of cfn_sort.cpp
echo "Compiling TRANSLATIONS_CN (LNG_CN) ..."
$CXX -std=c++11 -lstdc++ -Wfatal-errors -DLNG_CN tools/cfn_sorter.cpp
{
cat <<EOF >> radio/src/cfn_sort.cpp
// This file is auto-generated via cfn_sorter.sh. Do not edit.

#include "dataconstants.h"

Functions cfn_sorted[] = {
#if defined(TRANSLATIONS_CN)
EOF
./a.out
} >> radio/src/cfn_sort.cpp

# Languages
compile_and_append "LNG_CZ" "TRANSLATIONS_CZ" "elif"
compile_and_append "LNG_DA" "TRANSLATIONS_DA" "elif"
compile_and_append "LNG_DE" "TRANSLATIONS_DE" "elif"
compile_and_append "LNG_ES" "TRANSLATIONS_ES" "elif"
compile_and_append "LNG_FI" "TRANSLATIONS_FI" "elif"
compile_and_append "LNG_FR" "TRANSLATIONS_FR" "elif"
compile_and_append "LNG_HE" "TRANSLATIONS_HE" "elif"
compile_and_append "LNG_IT" "TRANSLATIONS_IT" "elif"
compile_and_append "LNG_JP" "TRANSLATIONS_JP" "elif"
compile_and_append "LNG_NL" "TRANSLATIONS_NL" "elif"
compile_and_append "LNG_PL" "TRANSLATIONS_PL" "elif"
compile_and_append "LNG_PT" "TRANSLATIONS_PT" "elif"
compile_and_append "LNG_RU" "TRANSLATIONS_RU" "elif"
compile_and_append "LNG_SE" "TRANSLATIONS_SE" "elif"
compile_and_append "LNG_TW" "TRANSLATIONS_TW" "elif"
compile_and_append "LNG_UA" "TRANSLATIONS_UA" "elif"

# Final else case
compile_and_append "LNG_EN" "TRANSLATIONS_EN" "else"

# End of cfn_sort.cpp
cat <<EOF >> radio/src/cfn_sort.cpp
#endif
};

uint8_t getFuncSortIdx(uint8_t func)
{
  for (uint8_t i = 0; i < FUNC_MAX; i += 1)
    if (cfn_sorted[i] == func)
      return i;
  return 0;
}
EOF

[ -f ./a.out ] && rm ./a.out
