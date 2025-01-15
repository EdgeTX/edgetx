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

#pragma once

#include "hal/i2c_driver.h"

#define TAS2505_REG(page, reg) ((page << 8) + reg)

#define TAS2505_PAGECTL TAS2505_REG(0, 0)
#define TAS2505_RESET TAS2505_REG(0, 1)
#define TAS2505_CLKMUX TAS2505_REG(0, 4)
#define TAS2505_PLLPR TAS2505_REG(0, 5)
#define TAS2505_PLLJ TAS2505_REG(0, 6)
#define TAS2505_PLLDMSB TAS2505_REG(0, 7)
#define TAS2505_PLLDLSB TAS2505_REG(0, 8)
#define TAS2505_NDAC TAS2505_REG(0, 11)
#define TAS2505_MDAC TAS2505_REG(0, 12)
#define TAS2505_DOSRMSB TAS2505_REG(0, 13)
#define TAS2505_DOSRLSB TAS2505_REG(0, 14)
#define TAS2505_IFACE1 TAS2505_REG(0, 27)
#define TAS2505_IFACE2 TAS2505_REG(0, 28)
#define TAS2505_IFACE3 TAS2505_REG(0, 29)
#define TAS2505_BCLKNDIV TAS2505_REG(0, 30)
#define TAS2505_DACFLAG1 TAS2505_REG(0, 37)
#define TAS2505_DACFLAG2 TAS2505_REG(0, 38)
#define TAS2505_STICKYFLAG1 TAS2505_REG(0, 42)
#define TAS2505_INTFLAG1 TAS2505_REG(0, 43)
#define TAS2505_STICKYFLAG2 TAS2505_REG(0, 44)
#define TAS2505_INTFLAG2 TAS2505_REG(0, 46)
#define TAS2505_DACINSTRSET TAS2505_REG(0, 60)
#define TAS2505_DACSETUP1 TAS2505_REG(0, 63)
#define TAS2505_DACSETUP2 TAS2505_REG(0, 64)
#define TAS2505_DACVOL TAS2505_REG(0, 65)
#define TAS2505_REF_POR_LDO_BGAP_CTRL TAS2505_REG(1, 1)
#define TAS2505_LDO_CTRL TAS2505_REG(1, 2)
#define TAS2505_PLAYBACKCONF1 TAS2505_REG(1, 3)
#define TAS2505_HPL_OUT TAS2505_REG(1, 9)
#define TAS2505_COMMON_MODE TAS2505_REG(1, 10)
#define TAS2505_HP_ROUTING TAS2505_REG(1, 12)
#define TAS2505_HP_DRIVER TAS2505_REG(1, 16)
#define TAS2505_HP_VOL TAS2505_REG(1, 22)
#define TAS2505_MIXER_CTRL TAS2505_REG(1, 24)
#define TAS2505_SPKAMPCTRL1 TAS2505_REG(1, 45)
#define TAS2505_SPKVOL1 TAS2505_REG(1, 46)
#define TAS2505_SPKVOL2 TAS2505_REG(1, 48)
#define TAS2505_DACANLGAINFLAG TAS2505_REG(1, 63)

#define TAS2505_SPK_DRV_MUTED (0U << 4)
#define TAS2505_SPK_DRV_6_DB (1U << 4)
#define TAS2505_SPK_DRV_12_DB (2U << 4)
#define TAS2505_SPK_DRV_18_DB (3U << 4)
#define TAS2505_SPK_DRV_24_DB (4U << 4)
#define TAS2505_SPK_DRV_32_DB (5U << 4)

#define TAS2505_HP_DRV_MIN -5
#define TAS2505_HP_DRV_MAX 29

typedef struct {
  etx_i2c_bus_t bus;
  uint16_t      addr;

  uint8_t  ndac;
  uint8_t  mdac;
  uint16_t dosr;

  int8_t dac_vol;

  int8_t  hp_drv;
  uint8_t hp_vol;

  uint8_t spk_drv;
  uint8_t spk_vol;

} tas2505_t;

int tas2505_probe(tas2505_t* dev, etx_i2c_bus_t bus, uint16_t addr);
int tas2505_init(tas2505_t* dev);
