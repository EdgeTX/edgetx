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

#include "pca95xx.h"

#define REG_INPUT     0x00
#define REG_OUTPUT    0x02
#define REG_POLARITY  0x04
#define REG_DIRECTION 0x06

static int pca95xx_i2c_write16(pca95xx_t* dev, uint16_t reg, uint16_t value)
{
  if (!dev->addr) return -1;

  if (i2c_write(dev->bus, dev->addr, reg, 1, (uint8_t*)&value, sizeof(value)) < 0) {
    return -1;
  }

  return 0;  
}

int pca95xx_init(pca95xx_t* dev, etx_i2c_bus_t bus, uint16_t addr)
{
  dev->bus = 0;
  dev->addr = 0;
  dev->polarity = 0;
  dev->direction = 0xFFFF;
  dev->output = 0;

  if (i2c_init(bus) < 0) {
    return -1;
  }

  if (i2c_dev_ready(bus, addr) < 0) {
    return -1;
  }

  dev->bus = bus;
  dev->addr = addr;
  return 0;
}

int pca95xx_set_direction(pca95xx_t* dev, uint16_t mask, uint16_t dir)
{
  uint16_t tmp = dev->direction;
  tmp &= ~mask;
  tmp |= dir & mask;

  if (pca95xx_i2c_write16(dev, REG_DIRECTION, tmp) < 0) {
    return -1;
  }

  dev->direction = tmp;
  return 0;
}

int pca95xx_set_polarity(pca95xx_t* dev, uint16_t mask, uint16_t dir)
{
  uint16_t tmp = dev->polarity;
  tmp &= ~mask;
  tmp |= dir & mask;

  if (pca95xx_i2c_write16(dev, REG_POLARITY, tmp) < 0) {
    return -1;
  }

  dev->polarity = tmp;
  return 0;
}

int pca95xx_write(pca95xx_t* dev, uint16_t mask, uint16_t value)
{
  uint16_t tmp = dev->output;
  tmp &= ~mask;
  tmp |= value & mask;

  if (pca95xx_i2c_write16(dev, REG_OUTPUT, tmp) < 0) {
    return -1;
  }

  dev->output = tmp;
  return 0;
}

int pca95xx_read(pca95xx_t* dev, uint16_t mask, uint16_t* value)
{
  if (!dev->addr) return -1;
  
  uint16_t tmp;
  if (i2c_read(dev->bus, dev->addr, REG_INPUT, 1, (uint8_t*)&tmp, 2) < 0) {
    return -1;
  }

  *value = tmp & mask;
  return 0;
}
