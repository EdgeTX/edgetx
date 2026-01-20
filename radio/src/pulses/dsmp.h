/*
 * Copyright (C) EdgeTx
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

#pragma once

#include "dsm2.h"

struct DSMPModuleStatus {
  uint8_t   version[2] = {1, 0};  // Default Version 1.0
  tmr10ms_t lastUpdate;
  uint8_t	ch_order   = 0xFF;
  uint8_t	flags      = 0;

  inline bool isValid() const { return (bool)(get_tmr10ms() - lastUpdate < 500);}
  void getStatusString(char *statusText) const;
};

extern DSMPModuleStatus &getDSMPStatus(uint8_t module);

// Lemon RX DSMP
extern const etx_proto_driver_t DSMPDriver;
