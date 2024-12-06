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

#include "edgetx.h"
#include "yaml_node.h"

// Warning: this file must be kept in sync with the CMakeLists.txt
//          in the same directory.

#include "yaml_inputs.inc"
#include "yaml_datastructs_funcs.cpp"

#if defined(PCBX10) || defined(PCBX12S)
 #if defined(RADIO_T15)
  #include "yaml_datastructs_t15.cpp"
 #elif defined(RADIO_F16)
  #include "yaml_datastructs_f16.cpp"
 #else
  #include "yaml_datastructs_x10.cpp"
 #endif
#elif defined(PCBPL18)
 #include "yaml_datastructs_pl18.cpp"
#elif defined(PCBNV14)
 #include "yaml_datastructs_nv14.cpp"
#elif defined(PCBX7)
 #if defined(RADIO_TPRO) || defined(RADIO_TPROV2) || defined(RADIO_BUMBLEBEE)
  #include "yaml_datastructs_tpro.cpp"
 #elif defined(RADIO_FAMILY_T20)
  #include "yaml_datastructs_t20.cpp"
 #elif defined(RADIO_COMMANDO8) || defined(RADIO_LR3PRO) || defined(RADIO_T8) || defined(RADIO_T12) || defined(RADIO_TLITE)
  #include "yaml_datastructs_xlite.cpp"
 #else
  #include "yaml_datastructs_128x64.cpp"
 #endif
#elif defined(PCBX9LITE)
 #include "yaml_datastructs_128x64.cpp"
#elif defined(PCBXLITE) && !defined(PCBXLITES)
 #include "yaml_datastructs_xlite.cpp"
#elif defined(PCBXLITES)
 #include "yaml_datastructs_xlites.cpp"
#elif defined(PCBX9E)
 #include "yaml_datastructs_x9e.cpp"
#elif defined(PCBX9D) || defined(PCBX9DP)
 #if PCBREV < 2019
  #include "yaml_datastructs_x9d.cpp"
 #else
  #include "yaml_datastructs_x9dp2019.cpp"
 #endif
#else
#error "Board not supported by YAML storage"
#endif

static_assert(MAX_STR > MAX_RADIODATA_MODELDATA_PARTIALMODEL_STR_LEN,
              "MAX_STR > MAX_RADIODATA_MODELDATA_PARTIALMODEL_STR_LEN");
