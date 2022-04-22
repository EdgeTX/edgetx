/*
 * Copyright (C) EdgeTX
 *
 * Based on code named
 *   opentx - https://github.com/opentx/opentx
 *   th9x - http://code.google.com/p/th9x
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <storage/yaml/yaml_node.h>
#include <storage/yaml/yaml_bits.h>
#include <storage/yaml/yaml_tree_walker.h>

#include <algorithm>

#include <debug.h>
#include <string.h>

#include "../../../strhelpers.h"
#include "../../../switches.h"
#include "../../../timers.h"
#include "../../../pulses/multi.h"

#if STORAGE_CONVERSIONS < 221

namespace yaml_conv_220 {

#include "yaml_datastructs_funcs.cpp"

#if defined(PCBX12S)
 #include "yaml_datastructs_x12s.cpp"
#elif defined(PCBX10)
 #include "yaml_datastructs_x10.cpp"
#elif defined(PCBNV14)
 #include "yaml_datastructs_nv14.cpp"
#elif defined(PCBX7)
 #if defined(RADIO_T12)
  #include "yaml_datastructs_t12.cpp"
 #elif defined(RADIO_TLITE)
  #include "yaml_datastructs_tlite.cpp"
 #elif defined(RADIO_TX12)
  #include "yaml_datastructs_tx12.cpp"
#elif defined(RADIO_TX12MK2)
  #include "yaml_datastructs_tx12mk2.cpp"
 #elif defined(RADIO_ZORRO)
  #include "yaml_datastructs_zorro.cpp"
 #elif defined(RADIO_T8)
  #include "yaml_datastructs_t8.cpp"
 #else
  #include "yaml_datastructs_x7.cpp"
 #endif
#elif defined(PCBX9LITE) && !defined(PCBX9LITES)
 #include "yaml_datastructs_x9lite.cpp"
#elif defined(PCBX9LITES)
 #include "yaml_datastructs_x9lites.cpp"
#elif defined(PCBXLITE) && !defined(PCBXLITES)
 #include "yaml_datastructs_xlite.cpp"
#elif defined(PCBXLITES)
 #include "yaml_datastructs_xlites.cpp"
#elif defined(PCBX9E)
 #include "yaml_datastructs_x9e.cpp"
#elif defined(PCBX9D) || defined(PCBX9DP)
 #include "yaml_datastructs_x9d.cpp"
#else
#error "Board not supported by YAML storage"
#endif

static_assert(MAX_STR > MAX_RADIODATA_MODELDATA_PARTIALMODEL_STR_LEN,
              "MAX_STR > MAX_RADIODATA_MODELDATA_PARTIALMODEL_STR_LEN");

};

#else

namespace yaml_conv_220 {
#include "yaml_datastructs_funcs.cpp"
};

#endif // STORAGE_CONVERSIONS
