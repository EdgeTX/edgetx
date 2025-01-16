/*
 * Copyright (C) EdgeTX
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

#include "seesaw.h"

#define STATUS_HW_ID   0x0001
#define STATUS_VERSION 0x0002

#define IS_VALID_HW_ID(id) \
  ((id >= 0x84 && id <= 0x89) || (id == 0x55))

#define GPIO_DIRSET_BULK 0x0102
#define GPIO_DIRCLR_BULK 0x0103
#define GPIO_BULK        0x0104
#define GPIO_BULK_SET    0x0105
#define GPIO_BULK_CLR    0x0106
#define GPIO_BULK_TOGGLE 0x0107
#define GPIO_PULLENSET   0x010B
#define GPIO_PULLENCLR   0x010C


int seesaw_read_hw_id(etx_i2c_bus_t bus, uint16_t addr)
{
  uint8_t hw_id = 0;
  if (i2c_read(bus, addr, STATUS_HW_ID, 2, &hw_id, sizeof(hw_id)) < 0) {
    return -1;
  }
  return hw_id;
}

int seesaw_read_version(etx_i2c_bus_t bus, uint16_t addr, uint32_t* version)
{
  return i2c_read(bus, addr, STATUS_VERSION, 2, (uint8_t*)version,
                  sizeof(uint32_t));
}

int seesaw_write_32bit_be(seesaw_t* dev, uint16_t reg, uint32_t value)
{
  uint8_t buffer[4];
  buffer[0] = value >> 24;
  buffer[1] = (value >> 16) & 0xFF;
  buffer[2] = (value >> 8) & 0xFF;
  buffer[3] = value & 0xFF;
  
  return i2c_write(dev->bus, dev->addr, reg, 2, buffer, sizeof(buffer));
}

int seesaw_read_32bit_be(seesaw_t* dev, uint16_t reg, uint32_t* value)
{
  uint8_t buffer[4] = {0};
  if (i2c_read(dev->bus, dev->addr, reg, 2, buffer, sizeof(buffer)) < 0) return -1;

  uint32_t tmp = 0;
  tmp |= buffer[0] << 24;
  tmp |= buffer[1] << 16;
  tmp |= buffer[2] << 8;
  tmp |= buffer[3];
  *value = tmp;

  return 0;
}

int seesaw_init(seesaw_t* dev, etx_i2c_bus_t bus, uint16_t addr)
{
  dev->bus = 0;
  dev->addr = 0;
  // dev->polarity = 0;
  // dev->direction = 0xFFFF;
  // dev->output = 0;

  if (i2c_init(bus) < 0) {
    return -1;
  }

  if (i2c_dev_ready(bus, addr) < 0) {
    return -1;
  }

  uint8_t hw_id = seesaw_read_hw_id(bus, addr);
  if (!IS_VALID_HW_ID(hw_id)) {
    return -1;
  }

  uint32_t version = 0;
  if (seesaw_read_version(bus, addr, &version) < 0) {
    return -1;
  }
  // uint32_t pid = version >> 16;

  dev->bus = bus;
  dev->addr = addr;
  return 0;
}

int seesaw_pin_mode(seesaw_t* dev, uint32_t mask, seesaw_input_mode mode)
{
  if (!dev->addr) return -1;

  switch (mode) {
    case SEESAW_OUTPUT:
      return seesaw_write_32bit_be(dev, GPIO_DIRSET_BULK, mask);

    case SEESAW_INPUT:
      if (seesaw_write_32bit_be(dev, GPIO_DIRCLR_BULK, mask) == 0 &&
          seesaw_write_32bit_be(dev, GPIO_PULLENCLR, mask) == 0)
        return 0;

    case SEESAW_INPUT_PULLUP:
      if (seesaw_write_32bit_be(dev, GPIO_DIRCLR_BULK, mask) == 0 &&
          seesaw_write_32bit_be(dev, GPIO_PULLENSET, mask) == 0 &&
          seesaw_write_32bit_be(dev, GPIO_BULK_SET, mask) == 0)
        return 0;

    case SEESAW_INPUT_PULLDOWN:
      if (seesaw_write_32bit_be(dev, GPIO_DIRCLR_BULK, mask) == 0 &&
          seesaw_write_32bit_be(dev, GPIO_PULLENSET, mask) == 0 &&
          seesaw_write_32bit_be(dev, GPIO_BULK_CLR, mask) == 0)
        return 0;
  }

  return -1;
}

int seesaw_digital_read(seesaw_t* dev, uint32_t pins, uint32_t* value)
{
  if (!dev->addr) return -1;
  return seesaw_read_32bit_be(dev, GPIO_BULK, value);  
}
