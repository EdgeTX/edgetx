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

#include "aw9523b.h"

#define REG_INPUT     0x00
#define REG_OUTPUT    0x02
#define REG_DIRECTION 0x04
#define REG_INTERRUPT 0x06
#define REG_CONTROL   0x11
#define REG_LED_MODE  0x12

#define CTL_P0_DRIVE_MODE_OD 0
#define CTL_P0_DRIVE_MODE_PP (1<<4)

static int aw9523b_i2c_write16(aw9523b_t* dev, uint16_t reg, uint16_t value)
{
  if (!dev->addr) return -1;

  uint8_t val8 = value&0xFF;
  if (i2c_write(dev->bus, dev->addr, reg, 1, (uint8_t*)&val8, 1) < 0) {
    return -1;
  }
  val8 = (value>>8)&0xFF;
  if (i2c_write(dev->bus, dev->addr, reg+1, 1, (uint8_t*)&val8, 1) < 0) {
    return -1;
  }

  return 0;  
}

int aw9523b_init(aw9523b_t* dev, etx_i2c_bus_t bus, uint16_t addr)
{
  dev->bus = 0;
  dev->addr = 0;
  //dev->interrupt = 0;
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
  volatile uint8_t test = 0;
  uint8_t tmp = CTL_P0_DRIVE_MODE_PP;
  i2c_read(dev->bus, dev->addr, REG_CONTROL, 1, (uint8_t*)&test, 1);
  i2c_write(dev->bus, dev->addr, REG_CONTROL, 1, (uint8_t*)&tmp, 1);
  i2c_read(dev->bus, dev->addr, REG_CONTROL, 1, (uint8_t*)&test, 1);
  aw9523b_i2c_write16(dev, REG_LED_MODE, 0xFFFF); // set all pins to GPIO mode not LED mode
  aw9523b_i2c_write16(dev, REG_INTERRUPT, 0); // enable interrupts for all pins

  return 0;
}

int aw9523b_set_direction(aw9523b_t* dev, uint16_t mask, uint16_t dir)
{
  uint16_t tmp = dev->direction;
  tmp &= ~mask;
  tmp |= dir & mask;

  if (aw9523b_i2c_write16(dev, REG_DIRECTION, tmp) < 0) {
    return -1;
  }

  dev->direction = tmp;
  return 0;
}

int aw9523b_write(aw9523b_t* dev, uint16_t mask, uint16_t value)
{
  uint16_t tmp = dev->output;
  tmp &= ~mask;
  tmp |= value & mask;

  if (aw9523b_i2c_write16(dev, REG_OUTPUT, tmp) < 0) {
    return -1;
  }

  dev->output = tmp;
  return 0;
}

int aw9523b_read(aw9523b_t* dev, uint16_t mask, uint16_t* value)
{
  if (!dev->addr) return -1;
  
  uint16_t tmp;
  uint8_t tmp8;
  if (i2c_read(dev->bus, dev->addr, REG_INPUT, 1, (uint8_t*)&tmp8, 1) < 0) {
    return -1;
  }
  tmp = tmp8;
  if (i2c_read(dev->bus, dev->addr, REG_INPUT+1, 1, (uint8_t*)&tmp8, 1) < 0) {
    return -1;
  }

  tmp |= ((uint16_t)tmp)<<8;
  *value = tmp & mask;
  return 0;
}
