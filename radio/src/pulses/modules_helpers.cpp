/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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

#if defined(EXTERNAL_ANTENNA)
bool isExternalAntennaEnabled()
{
  switch (g_eeGeneral.antennaMode) {
    case ANTENNA_MODE_INTERNAL:
      return false;
    case ANTENNA_MODE_EXTERNAL:
      return true;
    case ANTENNA_MODE_PER_MODEL:
      switch (g_model.moduleData[INTERNAL_MODULE].pxx.antennaMode) {
        case ANTENNA_MODE_EXTERNAL:
        case ANTENNA_MODE_ASK:
          return globalData.externalAntennaEnabled;
        default:
          return false;
      }
    default:
      return globalData.externalAntennaEnabled;
  }
}
#endif
