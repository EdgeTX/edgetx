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

#include "pxx.h"
#include "hal/module_port.h"

bool pxxClearSPort()
{
  auto sport_module = modulePortGetModuleForPort(ETX_MOD_PORT_SPORT);
  if (sport_module >= 0) {
    // verify S.PORT is not used for TX (otherwise fail early)
    auto mod_st = modulePortGetState(sport_module);
    if (mod_st && mod_st->tx.port &&
        mod_st->tx.port->port == ETX_MOD_PORT_SPORT) {
      return false;
    }
  } else {
    // S.PORT soft-serial cannot be used for sending
    sport_module = modulePortGetModuleForPort(ETX_MOD_PORT_SPORT_INV);
  }

  if (sport_module >= 0) {
    auto mod_st = modulePortGetState(sport_module);
    if (mod_st) modulePortDeInitRxPort(mod_st);
  }

  return true;
}
