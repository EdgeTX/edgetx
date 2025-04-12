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

#include "hal/switch_driver.h"
#include "drivers/pca95xx.h"
#include "os/async.h"
#include "stm32_i2c_driver.h"
#include "timers_driver.h"
#include "stm32_ws2812.h"
#include "stm32_switch_driver.h"

#include <FreeRTOS/include/FreeRTOS.h>
#include <FreeRTOS/include/timers.h>

#include "myeeprom.h"

#define IO_INT_GPIO GPIO_PIN(GPIOE, 14)
#define IO_RESET_GPIO GPIO_PIN(GPIOE, 15)

extern const stm32_switch_t* boardGetSwitchDef(uint8_t idx);

struct bsp_io_expander {
  pca95xx_t exp;
  uint32_t mask;
  uint32_t state;
};

static volatile bool _poll_switches_in_queue = false;
static bsp_io_expander _io_switches;
static bsp_io_expander _io_fs_switches;

static void _init_io_expander(bsp_io_expander* io, uint32_t mask)
{
  io->mask = mask;
  io->state = 0;  
}

static uint32_t _read_io_expander(bsp_io_expander* io)
{
  uint16_t value = 0;
  if (pca95xx_read(&io->exp, io->mask, &value) == 0) {
    io->state = value;
  }
  return io->state;  
}

static void _poll_switches(void *param1, uint32_t param2)
{
  (void)param1;
  (void)param2;
  _poll_switches_in_queue = false;
  _read_io_expander(&_io_switches); 
  _read_io_expander(&_io_fs_switches); 
}

static void _io_int_handler()
{
  async_call_isr(_poll_switches, &_poll_switches_in_queue, nullptr, 0);
}

int bsp_io_init()
{
  timersInit();

  if (i2c_init(I2C_Bus_2) < 0) {
    return -1;
  }

  // configure expander 1
  _init_io_expander(&_io_switches, 0xFC3F);
  if (pca95xx_init(&_io_switches.exp, I2C_Bus_2, 0x74) < 0) {
    return -1;
  }

  // configure expander 2
  _init_io_expander(&_io_fs_switches, 0x3F);
  if (pca95xx_init(&_io_fs_switches.exp, I2C_Bus_2, 0x75) < 0) {
    return -1;
  }

  // setup expanders pin change interrupt
  gpio_init_int(IO_INT_GPIO, GPIO_IN, GPIO_FALLING, _io_int_handler);

  // setup expanders reset pin
  gpio_init(IO_RESET_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
  gpio_set(IO_RESET_GPIO);

  _read_io_expander(&_io_switches);
  _read_io_expander(&_io_fs_switches);

  return 0;
}

void boardInitSwitches()
{
  bsp_io_init();
}

static SwitchHwPos _get_switch_pos(uint8_t idx)
{
  SwitchHwPos pos = SWITCH_HW_UP;

  const stm32_switch_t* def = boardGetSwitchDef(idx);
  uint32_t state = _io_switches.state;

  if (def->isCustomSwitch) {
    if ((state & def->Pin_high) == 0) {
      return SWITCH_HW_DOWN;
    } else {
      return SWITCH_HW_UP;
    }
  }
  else if (!def->Pin_low) {
    // 2POS switch
    if ((state & def->Pin_high) == 0) {
      pos = SWITCH_HW_DOWN;
    }
  } else {
    bool hi = state & def->Pin_high;
    bool lo = state & def->Pin_low;

    if (hi && lo) {
      pos = SWITCH_HW_MID;
    } else if (!hi && lo) {
      pos = SWITCH_HW_DOWN;
    }
  }

  return pos;
}

static SwitchHwPos _get_fs_switch_pos(uint8_t idx)
{
  const stm32_switch_t* def = boardGetSwitchDef(idx);
  uint32_t state = _io_fs_switches.state;
  if ((state & def->Pin_high) == 0) {
    return SWITCH_HW_DOWN;
  } else {
    return SWITCH_HW_UP;
  }
}

SwitchHwPos boardSwitchGetPosition(uint8_t idx)
{
  if (idx < 8)
    return _get_switch_pos(idx);
  return _get_fs_switch_pos(idx);
}
