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

#include "opentx.h"
#include "yaml_node.h"

#include "yaml_datastructs_funcs.cpp"

#if defined(PCBX10)
 #include "yaml_datastructs_x10.cpp"
#elif defined(PCBX12S)
 #include "yaml_datastructs_x12s.cpp"
#elif defined(PCBX7)
 #include "yaml_datastructs_x7.cpp"
#elif defined(PCBX9LITE)
 #include "yaml_datastructs_x9lite.cpp"
#elif defined(PCBX9LITES)
 #include "yaml_datastructs_x9lites.cpp"
#elif defined(PCBXLITE)
 #include "yaml_datastructs_xlite.cpp"
#elif defined(PCBXLITES)
 #include "yaml_datastructs_xlites.cpp"
#elif defined(PCBX9E)
 #include "yaml_datastructs_x9e.cpp"
#elif defined(PCBX9D)
 #include "yaml_datastructs_x9d.cpp"
#elif defined(PCBX9DP)
 #include "yaml_datastructs_x9dp.cpp"
#else
#error "Board not supported by YAML storage"
#endif
