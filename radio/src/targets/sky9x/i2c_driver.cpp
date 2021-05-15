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
#include "twi.h"

#define I2C_CLK_FAST  400000

uint32_t fromBCD(uint8_t bcd_value)
{
  return (((bcd_value & 0xF0) * 10) >> 4) + (bcd_value & 0x0F);
}

uint8_t toBCD(uint32_t value)
{
  div_t qr = div(value, 10);
  return (qr.quot << 4) + qr.rem;
}

twi_packet getPacket(uint8_t chip, uint8_t* addr, uint8_t addr_len, uint8_t *buffer, uint32_t length)
{
  twi_packet packet;
  packet.chip = chip;
  for (uint8_t i = 0; i < addr_len; ++i)
    packet.addr[i] = addr[i];
  packet.addr_length = addr_len;
  packet.buffer = buffer;
  packet.length = length;
  return packet;
}

bool i2cInit()
{
  //Enable Peripheral Clock
  PMC->PMC_PCER0 |= 0x00080000L;
  //Enable TWI PIOs
  Pio *pioptr;
  pioptr = PIOA;
  pioptr->PIO_ABCDSR[0] &= ~0x00000018;        // Peripheral A
  pioptr->PIO_ABCDSR[1] &= ~0x00000018;        // Peripheral A
  pioptr->PIO_PDR = 0x00000018;                // Assign to peripheral

  twi_enable_master_mode(TWI0);
  twi_options_t opt;
  opt.master_clk = Master_frequency;
  opt.speed = I2C_CLK_FAST;
  return twi_master_init(TWI0, &opt) == TWI_SUCCESS;
}

void i2cCheck()
{
}

uint32_t i2cReadBuffer(uint8_t chip, uint8_t* addr, uint8_t addr_len, uint8_t *buffer, uint32_t length)
{
  twi_packet packet(getPacket(chip, addr, addr_len, buffer, length));
  return twi_master_read(TWI0, &packet);
}

uint32_t i2cWriteBuffer(uint8_t chip, uint8_t* addr, uint8_t addr_len, uint8_t *buffer, uint32_t length)
{
  twi_packet packet(getPacket(chip, addr, addr_len, buffer, length));
  return twi_master_write(TWI0, &packet);
}
