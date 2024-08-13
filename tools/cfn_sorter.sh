#!/bin/sh

rm radio/src/cfn_sort.cpp

echo "/*" >> radio/src/cfn_sort.cpp
echo " * Copyright (C) EdgeTX" >> radio/src/cfn_sort.cpp
echo " *" >> radio/src/cfn_sort.cpp
echo " * Based on code named" >> radio/src/cfn_sort.cpp
echo " *   opentx - https://github.com/opentx/opentx" >> radio/src/cfn_sort.cpp
echo " *   th9x - http://code.google.com/p/th9x" >> radio/src/cfn_sort.cpp
echo " *   er9x - http://code.google.com/p/er9x" >> radio/src/cfn_sort.cpp
echo " *   gruvin9x - http://code.google.com/p/gruvin9x" >> radio/src/cfn_sort.cpp
echo " *" >> radio/src/cfn_sort.cpp
echo " * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html" >> radio/src/cfn_sort.cpp
echo " *" >> radio/src/cfn_sort.cpp
echo " * This program is free software; you can redistribute it and/or modify" >> radio/src/cfn_sort.cpp
echo " * it under the terms of the GNU General Public License version 2 as" >> radio/src/cfn_sort.cpp
echo " * published by the Free Software Foundation." >> radio/src/cfn_sort.cpp
echo " *" >> radio/src/cfn_sort.cpp
echo " * This program is distributed in the hope that it will be useful," >> radio/src/cfn_sort.cpp
echo " * but WITHOUT ANY WARRANTY; without even the implied warranty of" >> radio/src/cfn_sort.cpp
echo " * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the" >> radio/src/cfn_sort.cpp
echo " * GNU General Public License for more details." >> radio/src/cfn_sort.cpp
echo " */" >> radio/src/cfn_sort.cpp
echo "" >> radio/src/cfn_sort.cpp
echo "// This file is auto-generated. Do not edit." >> radio/src/cfn_sort.cpp
echo "" >> radio/src/cfn_sort.cpp
echo "#include \"dataconstants.h\"" >> radio/src/cfn_sort.cpp
echo "" >> radio/src/cfn_sort.cpp
echo "Functions cfn_sorted[] = {" >> radio/src/cfn_sort.cpp
echo "#if   defined(TRANSLATIONS_CN)" >> radio/src/cfn_sort.cpp
echo "" >> radio/src/cfn_sort.cpp

gcc -std=c++11 -lstdc++ -DLNG_CN tools/cfn_sorter.cpp
./a.out >> radio/src/cfn_sort.cpp

echo "" >> radio/src/cfn_sort.cpp
echo "#elif defined(TRANSLATIONS_CZ)" >> radio/src/cfn_sort.cpp
echo "" >> radio/src/cfn_sort.cpp

gcc -std=c++11 -lstdc++ -DLNG_CZ tools/cfn_sorter.cpp
./a.out >> radio/src/cfn_sort.cpp

echo "" >> radio/src/cfn_sort.cpp
echo "#elif defined(TRANSLATIONS_DA)" >> radio/src/cfn_sort.cpp
echo "" >> radio/src/cfn_sort.cpp

gcc -std=c++11 -lstdc++ -DLNG_DA tools/cfn_sorter.cpp
./a.out >> radio/src/cfn_sort.cpp

echo "" >> radio/src/cfn_sort.cpp
echo "#elif defined(TRANSLATIONS_DE)" >> radio/src/cfn_sort.cpp
echo "" >> radio/src/cfn_sort.cpp

gcc -std=c++11 -lstdc++ -DLNG_DE tools/cfn_sorter.cpp
./a.out >> radio/src/cfn_sort.cpp

echo "" >> radio/src/cfn_sort.cpp
echo "#elif defined(TRANSLATIONS_ES)" >> radio/src/cfn_sort.cpp
echo "" >> radio/src/cfn_sort.cpp

gcc -std=c++11 -lstdc++ -DLNG_ES tools/cfn_sorter.cpp
./a.out >> radio/src/cfn_sort.cpp

echo "" >> radio/src/cfn_sort.cpp
echo "#elif defined(TRANSLATIONS_FI)" >> radio/src/cfn_sort.cpp
echo "" >> radio/src/cfn_sort.cpp

gcc -std=c++11 -lstdc++ -DLNG_FI tools/cfn_sorter.cpp
./a.out >> radio/src/cfn_sort.cpp

echo "" >> radio/src/cfn_sort.cpp
echo "#elif defined(TRANSLATIONS_FR)" >> radio/src/cfn_sort.cpp
echo "" >> radio/src/cfn_sort.cpp

gcc -std=c++11 -lstdc++ -DLNG_FR tools/cfn_sorter.cpp
./a.out >> radio/src/cfn_sort.cpp

echo "" >> radio/src/cfn_sort.cpp
echo "#elif defined(TRANSLATIONS_HE)" >> radio/src/cfn_sort.cpp
echo "" >> radio/src/cfn_sort.cpp

gcc -std=c++11 -lstdc++ -DLNG_HE tools/cfn_sorter.cpp
./a.out >> radio/src/cfn_sort.cpp

echo "" >> radio/src/cfn_sort.cpp
echo "#elif defined(TRANSLATIONS_IT)" >> radio/src/cfn_sort.cpp
echo "" >> radio/src/cfn_sort.cpp

gcc -std=c++11 -lstdc++ -DLNG_IT tools/cfn_sorter.cpp
./a.out >> radio/src/cfn_sort.cpp

echo "" >> radio/src/cfn_sort.cpp
echo "#elif defined(TRANSLATIONS_JP)" >> radio/src/cfn_sort.cpp
echo "" >> radio/src/cfn_sort.cpp

gcc -std=c++11 -lstdc++ -DLNG_JP tools/cfn_sorter.cpp
./a.out >> radio/src/cfn_sort.cpp

echo "" >> radio/src/cfn_sort.cpp
echo "#elif defined(TRANSLATIONS_NL)" >> radio/src/cfn_sort.cpp
echo "" >> radio/src/cfn_sort.cpp

gcc -std=c++11 -lstdc++ -DLNG_NL tools/cfn_sorter.cpp
./a.out >> radio/src/cfn_sort.cpp

echo "" >> radio/src/cfn_sort.cpp
echo "#elif defined(TRANSLATIONS_PL)" >> radio/src/cfn_sort.cpp
echo "" >> radio/src/cfn_sort.cpp

gcc -std=c++11 -lstdc++ -DLNG_PL tools/cfn_sorter.cpp
./a.out >> radio/src/cfn_sort.cpp

echo "" >> radio/src/cfn_sort.cpp
echo "#elif defined(TRANSLATIONS_PT)" >> radio/src/cfn_sort.cpp
echo "" >> radio/src/cfn_sort.cpp

gcc -std=c++11 -lstdc++ -DLNG_PT tools/cfn_sorter.cpp
./a.out >> radio/src/cfn_sort.cpp

echo "" >> radio/src/cfn_sort.cpp
echo "#elif defined(TRANSLATIONS_RU)" >> radio/src/cfn_sort.cpp
echo "" >> radio/src/cfn_sort.cpp

gcc -std=c++11 -lstdc++ -DLNG_RU tools/cfn_sorter.cpp
./a.out >> radio/src/cfn_sort.cpp

echo "" >> radio/src/cfn_sort.cpp
echo "#elif defined(TRANSLATIONS_SE)" >> radio/src/cfn_sort.cpp
echo "" >> radio/src/cfn_sort.cpp

gcc -std=c++11 -lstdc++ -DLNG_SE tools/cfn_sorter.cpp
./a.out >> radio/src/cfn_sort.cpp

echo "" >> radio/src/cfn_sort.cpp
echo "#elif defined(TRANSLATIONS_TW)" >> radio/src/cfn_sort.cpp
echo "" >> radio/src/cfn_sort.cpp

gcc -std=c++11 -lstdc++ -DLNG_TW tools/cfn_sorter.cpp
./a.out >> radio/src/cfn_sort.cpp

echo "" >> radio/src/cfn_sort.cpp
echo "#elif defined(TRANSLATIONS_UA)" >> radio/src/cfn_sort.cpp
echo "" >> radio/src/cfn_sort.cpp

gcc -std=c++11 -lstdc++ -DLNG_UA tools/cfn_sorter.cpp
./a.out >> radio/src/cfn_sort.cpp

echo "" >> radio/src/cfn_sort.cpp
echo "#else" >> radio/src/cfn_sort.cpp
echo "" >> radio/src/cfn_sort.cpp

gcc -std=c++11 -lstdc++ -DLNG_EN tools/cfn_sorter.cpp
./a.out >> radio/src/cfn_sort.cpp

echo "" >> radio/src/cfn_sort.cpp
echo "#endif" >> radio/src/cfn_sort.cpp
echo "};" >> radio/src/cfn_sort.cpp
echo "" >> radio/src/cfn_sort.cpp
echo "uint8_t getFuncSortIdx(uint8_t func)" >> radio/src/cfn_sort.cpp
echo "{" >> radio/src/cfn_sort.cpp
echo "  for (uint8_t i = 0; i < FUNC_MAX; i += 1)" >> radio/src/cfn_sort.cpp
echo "    if (cfn_sorted[i] == func)" >> radio/src/cfn_sort.cpp
echo "      return i;" >> radio/src/cfn_sort.cpp
echo "  return 0;" >> radio/src/cfn_sort.cpp
echo "}" >> radio/src/cfn_sort.cpp

rm a.out
