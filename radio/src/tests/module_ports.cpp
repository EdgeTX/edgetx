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

#include "gtests.h"
#include "hal/module_port.h"

TEST(ports, softserialFallback)
{
  modulePortInit();

  const etx_serial_init serialCfg = {
    .baudrate = 57600,
    .encoding = ETX_Encoding_8N1,
    .direction = ETX_Dir_RX,
    .polarity = ETX_Pol_Inverted,
  };

  bool has_softserial = modulePortFind(EXTERNAL_MODULE, ETX_MOD_TYPE_SERIAL,
				       ETX_MOD_PORT_SPORT_INV, ETX_Pol_Inverted);
  if (has_softserial) {
    auto mod_st = modulePortInitSerial(EXTERNAL_MODULE, ETX_MOD_PORT_SPORT,
                                       &serialCfg, false);
    EXPECT_TRUE(mod_st == nullptr);

    mod_st = modulePortInitSerial(EXTERNAL_MODULE, ETX_MOD_PORT_SPORT,
                                  &serialCfg, true);
    EXPECT_TRUE(mod_st != nullptr);
    if (!mod_st) return;

    modulePortDeInit(mod_st);
  }
}
