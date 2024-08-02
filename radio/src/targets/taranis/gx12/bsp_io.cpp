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

#include "bsp_io.h"
#include "hal/switch_driver.h"
#include "drivers/pca95xx.h"
#include "stm32_i2c_driver.h"
#include "timers_driver.h"

#include <FreeRTOS/include/FreeRTOS.h>
#include <FreeRTOS/include/timers.h>

struct bsp_io_expander {
  pca95xx_t exp;
  uint32_t mask;
  uint32_t state;
};

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

static void _poll_switches(void *pvParameter1, uint32_t ulParameter2)
{
  (void)ulParameter2;
  bsp_io_expander* io = (bsp_io_expander*)pvParameter1;
  _read_io_expander(io); 
}

static void _io_int_handler(bsp_io_expander* io)
{
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
    xTimerPendFunctionCallFromISR(_poll_switches, (void*)io, 0,
                                  &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
  } else {
    _read_io_expander(io);
  }
}

static void _switch_int_handler() {
  _io_int_handler(&_io_switches);
}

static void _fs_switch_int_handler() {
  _io_int_handler(&_io_fs_switches);
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

  // setup expander 1 pin change interrupt
  gpio_init_int(SWITCH_INT_GPIO, GPIO_IN, GPIO_FALLING, _switch_int_handler);
  bsp_io_read_switches();

  // configure expander 2
  _init_io_expander(&_io_fs_switches, 0x3F);
  if (pca95xx_init(&_io_fs_switches.exp, I2C_Bus_2, 0x75) < 0) {
    return -1;
  }

  // setup expander 2 pin change interrupt
  gpio_init_int(FS_SWITCH_INT_GPIO, GPIO_IN, GPIO_FALLING, _fs_switch_int_handler);
  bsp_io_read_fs_switches();

  return 0;
}

uint32_t bsp_io_read_switches()
{
  return _read_io_expander(&_io_switches);
}

uint32_t bsp_io_read_fs_switches()
{
  return _read_io_expander(&_io_fs_switches);
}

void boardInitSwitches()
{
  bsp_io_init();
}

struct bsp_io_sw_def {
  uint32_t pin_high;
  uint32_t pin_low;
};

static const bsp_io_sw_def _switch_defs[] = {
  { SWITCH_A, 0 },
  { SWITCH_B_H, SWITCH_B_L },
  { SWITCH_C_H, SWITCH_C_L },
  { SWITCH_D, 0 },
  { SWITCH_E_H, SWITCH_E_L },
  { SWITCH_F_H, SWITCH_F_L },
  { SWITCH_G, 0 },
  { SWITCH_H, 0 },
};

static SwitchHwPos _get_switch_pos(uint8_t idx)
{
  SwitchHwPos pos = SWITCH_HW_UP;
  const bsp_io_sw_def* def = &_switch_defs[idx];

  uint32_t state = _io_switches.state;  
  if (!def->pin_low) {
    // 2POS switch
    if ((state & def->pin_high) == 0) {
      pos = SWITCH_HW_DOWN;
    }
  } else {
    bool hi = state & def->pin_high;
    bool lo = state & def->pin_low;

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
  uint32_t state = _io_fs_switches.state;
  if ((state & (1 << idx)) == 0) {
    return SWITCH_HW_DOWN;
  } else {
    return SWITCH_HW_UP;
  }
}

SwitchHwPos boardSwitchGetPosition(uint8_t cat, uint8_t idx)
{
  if (cat == SWITCH_PHYSICAL) {
    return _get_switch_pos(idx);
  } else if (cat == SWITCH_FUNCTION){
    return _get_fs_switch_pos(idx);
  }

  return SWITCH_HW_UP;
}
