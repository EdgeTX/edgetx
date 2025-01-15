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

#include "tas2505.h"
#include "stm32_i2s.h"
#include "timers_driver.h"

#include "debug.h"

#define TAS2505_NDAC_ENABLE (1U << 7)
#define TAS2505_MDAC_ENABLE (1U << 7)
#define TAS2505_MASTER_REF_ENABLE (1U << 4)
#define TAS2505_HPL_OUT_UP (1U << 5)
#define TAS2505_MIXER_P_TO_HP (1U << 2)

static int tas2505_i2c_write(tas2505_t* dev, uint16_t reg, uint8_t value)
{
  if (!dev->addr) return -1;
  return i2c_write(dev->bus, dev->addr, reg, 1, (uint8_t*)&value, sizeof(value));
}

static int tas2505_write_reg(tas2505_t* dev, uint16_t reg, uint8_t value)
{
  uint8_t page = reg >> 8;
  if (tas2505_i2c_write(dev, 0, page) < 0) return -1;
  return tas2505_i2c_write(dev, reg & 0xFF, value);
}

static void tas2505_delay(uint32_t ms)
{
  uint32_t start = timersGetMsTick();
  while (timersGetMsTick() - start < ms) {}
}

int tas2505_probe(tas2505_t* dev, etx_i2c_bus_t bus, uint16_t addr)
{
  dev->bus = 0;
  dev->addr = 0;

  if (i2c_init(bus) < 0) {
    TRACE("TAS2505 bus init error");
    return -1;
  }

  if (i2c_dev_ready(bus, addr) < 0) {
    TRACE("TAS2505 device init error");
    return -1;
  }

  dev->bus = bus;
  dev->addr = addr;

  return 0;
}

int tas2505_init(tas2505_t* dev)
{
  // software reset
  if (tas2505_write_reg(dev, TAS2505_RESET, 1) < 0) return -1;
  tas2505_delay(2);

  if (dev->hp_drv < TAS2505_HP_DRV_MIN) {
    dev->hp_drv = TAS2505_HP_DRV_MIN;
  } else if (dev->hp_drv > TAS2505_HP_DRV_MAX) {
    dev->hp_drv = TAS2505_HP_DRV_MAX;
  }

  const struct {
    uint16_t reg;
    uint8_t value;
  } params[] = {
      {TAS2505_LDO_CTRL, 0},
      {TAS2505_CLKMUX, 0},  // MCLK -> CODEC_CLKIN

      {TAS2505_NDAC, (uint8_t)(TAS2505_NDAC_ENABLE | dev->ndac)},
      {TAS2505_MDAC, (uint8_t)(TAS2505_MDAC_ENABLE | dev->mdac)},
      {TAS2505_DOSRLSB, (uint8_t)(dev->dosr & 0xFF)},
      {TAS2505_DOSRMSB, (uint8_t)(dev->dosr >> 8)},

      {TAS2505_IFACE1, 0},                // I2S, 16bit, BCLK in, WCLK in
      {TAS2505_IFACE2, 0},                // Data slot offset 00
      {TAS2505_DACINSTRSET, 0b00000010},  // PRB_P2

      {TAS2505_DACVOL, (uint8_t)dev->dac_vol},
      {TAS2505_DACSETUP1, 0b10010100},    // DAC on, left channel
      {TAS2505_DACSETUP2, 0b00000100},    // DAC unmuted

      {TAS2505_REF_POR_LDO_BGAP_CTRL, TAS2505_MASTER_REF_ENABLE},
      {TAS2505_HPL_OUT, TAS2505_HPL_OUT_UP}, // HPL output ON

      {TAS2505_COMMON_MODE, 0},           // CM 0.9V
      {TAS2505_HP_ROUTING, TAS2505_MIXER_P_TO_HP}, // enable HP
      {TAS2505_HP_DRIVER, (uint8_t)(dev->hp_drv & 0x3F)},
      {TAS2505_HP_VOL, dev->hp_vol},

      {TAS2505_SPKVOL1, dev->spk_vol},
      {TAS2505_SPKVOL2, dev->spk_drv},
      {TAS2505_SPKAMPCTRL1, 0b00000010},  // Spkr ON
  };

  for (unsigned i = 0; i < DIM(params); i++) {
    uint16_t reg = params[i].reg;
    uint8_t value = params[i].value;
    if (tas2505_write_reg(dev, reg, value) < 0) {
      return -1;
    }
  }

  return 0;
}
