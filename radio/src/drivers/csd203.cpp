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

#include "csd203.h"

#include "delays_driver.h"
#include "hal/i2c_driver.h"
#include "os/timer.h"

// CSD203 register map
#define CSD203_REG_CONFIG       0x00
#define CSD203_REG_SHUNT_V     0x01
#define CSD203_REG_BUS_V       0x02
#define CSD203_REG_POWER       0x03
#define CSD203_REG_CURRENT     0x04
#define CSD203_REG_CALIBRATION 0x05
#define CSD203_REG_MFR_ID      0xFE

#define CSD203_MFR_ID_EXPECTED 0x4153

// Calibration parameter: gain * 10000K
#define CSD203_CAL_PARAM       51200

// Config register fields (matching CSD203 datasheet)
#define CFG_RST         (1 << 15)
#define CFG_AVG_16      (2 << 9)
#define CFG_VBUS_CT_1MS (4 << 5)
#define CFG_VSHT_CT_1MS (4 << 2)
#define CFG_MODE_CONT   7

extern bool suspendI2CTasks;

static int csd203_read_reg(csd203_t* dev, uint8_t reg, uint16_t* value)
{
  uint8_t buf[2];
  if (i2c_read(dev->bus, dev->addr, reg, 1, buf, 2) < 0)
    return -1;
  *value = (buf[0] << 8) | buf[1];
  return 0;
}

static int csd203_write_reg(csd203_t* dev, uint8_t reg, uint16_t value)
{
  uint8_t buf[2] = {(uint8_t)(value >> 8), (uint8_t)(value & 0xFF)};
  return i2c_write(dev->bus, dev->addr, reg, 1, buf, 2);
}

int csd203_init(csd203_t* dev, etx_i2c_bus_t bus, uint16_t addr,
                uint16_t rshunt, uint16_t current_lsb)
{
  dev->bus = bus;
  dev->addr = addr;
  dev->initialized = false;

  uint16_t config = CFG_RST | CFG_AVG_16 | CFG_VBUS_CT_1MS |
                    CFG_VSHT_CT_1MS | CFG_MODE_CONT;
  if (csd203_write_reg(dev, CSD203_REG_CONFIG, config) < 0)
    return -1;

  uint16_t cal = CSD203_CAL_PARAM / (current_lsb * rshunt);
  if (csd203_write_reg(dev, CSD203_REG_CALIBRATION, cal) < 0)
    return -1;

  delay_ms(1);

  uint16_t mfr_id;
  if (csd203_read_reg(dev, CSD203_REG_MFR_ID, &mfr_id) < 0)
    return -1;

  if (mfr_id != CSD203_MFR_ID_EXPECTED)
    return -1;

  // Dummy current read to clear conversion-ready flag
  uint16_t dummy;
  csd203_read_reg(dev, CSD203_REG_CURRENT, &dummy);

  dev->initialized = true;
  return 0;
}

uint16_t csd203_read_voltage(csd203_t* dev)
{
  uint16_t val = 0;
  csd203_read_reg(dev, CSD203_REG_BUS_V, &val);
  return val;
}

uint16_t csd203_read_current(csd203_t* dev)
{
  uint16_t val = 0;
  csd203_read_reg(dev, CSD203_REG_CURRENT, &val);
  return val;
}

// Board-level: 3 CSD203 sensors (main, internal, external)
// Addresses: A1=VS/A0=GND (0x44), A1=VS/A0=VS (0x45), A1=GND/A0=VS (0x41)
#define CSD203_ADDR_MAIN     0x44
#define CSD203_ADDR_INTERNAL 0x45
#define CSD203_ADDR_EXTERNAL 0x41

// Shunt resistor 10 mOhm, current LSB 1 mA
#define CSD203_RSHUNT      10
#define CSD203_CURRENT_LSB 10

static csd203_t csd203_main;
static csd203_t csd203_internal;
static csd203_t csd203_external;

static etx_i2c_bus_t csd203_bus;
static uint16_t csd203_ext_vbus;
static timer_handle_t csd203_timer = TIMER_INITIALIZER;

#define CSD203_POLL_PERIOD_MS 10

static void csd203TimerCb(timer_handle_t* timer)
{
  (void)timer;
  static uint16_t step = 0;

  if (suspendI2CTasks) return;
  if (!i2c_trylock(csd203_bus)) return;

  if (step == 0 && csd203_main.initialized) {
    csd203_read_current(&csd203_main);
    csd203_read_voltage(&csd203_main);
  } else if (step == 1 && csd203_internal.initialized) {
    csd203_read_current(&csd203_internal);
    csd203_read_voltage(&csd203_internal);
  } else if (step == 2 && csd203_external.initialized) {
    csd203_read_current(&csd203_external);
    csd203_ext_vbus = (uint16_t)(csd203_read_voltage(&csd203_external) * 1.25f);
  }

  i2c_unlock(csd203_bus);
  if (++step >= 3) step = 0;
}

void csd203_start(etx_i2c_bus_t bus)
{
  csd203_bus = bus;

  if (i2c_init(bus) < 0)
    return;

  csd203_init(&csd203_main, bus, CSD203_ADDR_MAIN,
              CSD203_RSHUNT, CSD203_CURRENT_LSB);

  delay_ms(5);
  csd203_init(&csd203_internal, bus, CSD203_ADDR_INTERNAL,
              CSD203_RSHUNT, CSD203_CURRENT_LSB);

  delay_ms(5);
  csd203_init(&csd203_external, bus, CSD203_ADDR_EXTERNAL,
              CSD203_RSHUNT, CSD203_CURRENT_LSB);

  if (csd203_main.initialized || csd203_internal.initialized ||
      csd203_external.initialized) {
    timer_create(&csd203_timer, csd203TimerCb, "csd203",
                 CSD203_POLL_PERIOD_MS, true);
    timer_start(&csd203_timer);
  }
}

uint16_t getBatteryVoltage()
{
  return csd203_ext_vbus / 10;
}
