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

#include "stm32_ws2812.h"
#include "stm32_switch_driver.h"
#include "stm32_i2c_driver.h"
#include "hal/switch_driver.h"
#include "drivers/pca95xx.h"
#include "timers_driver.h"
#include "delays_driver.h"
#include "hal.h"

#if !defined(BOOT)
#include "os/async.h"
#include "os/timer.h"
#endif

#include "debug.h"

extern const stm32_switch_t* boardGetSwitchDef(uint8_t idx);
extern bool suspendI2CTasks;

struct bsp_io_expander {
    pca95xx_t exp;
    uint32_t mask;
    uint32_t state;
};

static volatile bool _poll_switches_in_queue = false;

static bsp_io_expander _io_switches;
static bsp_io_expander _io_fs_switches;

#if !defined(BOOT)
static timer_handle_t _poll_timer = TIMER_INITIALIZER;
#endif

static void _init_io_expander(bsp_io_expander* io, uint32_t mask)
{
  io->mask = mask;
  io->state = 0;
}

static void _expanders_reset()
{
  gpio_clear(IO_RESET_GPIO);
  delay_us(1);  // Only 6ns needed according to TCA9539 datasheet
  gpio_set(IO_RESET_GPIO);
  delay_us(1);  // Chip time to reset is 400ns
}

static uint32_t _read_io_expander(bsp_io_expander* io)
{
  uint16_t value = 0;
  if (pca95xx_read(&io->exp, io->mask, &value) == 0) {
    io->state = value;
  } else {
    // Unable to read PCA, reset it
    TRACE("ERROR: resetting PCA95XX");
    _expanders_reset();
    // Re read
    if (pca95xx_read(&io->exp, io->mask, &value) == 0) {
      io->state = value;
    } else {
      // PCA reset did not work, try resetting PCA I2C Bus
      TRACE("ERROR: resetting PCA95XX I2C bus");
      stm32_i2c_deinit(IO_EXPANDER_I2C_BUS);
      bsp_io_init();
      if (pca95xx_read(&io->exp, io->mask, &value) == 0) {
        io->state = value;
      } else {
        TRACE("ERROR: Unrecoverable error on PCA95XX");
      }
    }
  }
  return io->state;
}

#if !defined(BOOT)
typedef enum {
  TRIGGERED_BY_TIMER = 0,
  TRIGGERED_BY_IRQ,
} trigger_source_t;

static void _poll_switches(void *param1, uint32_t trigger_source)
{
  if (trigger_source == TRIGGERED_BY_IRQ) {
    _poll_switches_in_queue = false;
    timer_reset(&_poll_timer);
  }

  // Suspend hardware reads when required
  if (suspendI2CTasks) return;

  _read_io_expander(&_io_switches);
  _read_io_expander(&_io_fs_switches);
}

static void _io_int_handler()
{
  async_call_isr(_poll_switches, &_poll_switches_in_queue, nullptr,
                 TRIGGERED_BY_IRQ);
}

static void _poll_cb(timer_handle_t* timer)
{
  (void)timer;
  _poll_switches(nullptr, TRIGGERED_BY_TIMER);
}

static void start_poll_timer()
{
  timer_create(&_poll_timer, _poll_cb, "portex", 100, true);
  timer_start(&_poll_timer);
}
#endif

int bsp_io_init()
{
  int i2cError = i2c_init(IO_EXPANDER_I2C_BUS);
  if (i2cError < 0) {
    TRACE("I2C INIT ERROR: %d", i2cError);
    return -1;
  }

  // setup expanders reset pin
  gpio_init(IO_RESET_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
  _expanders_reset();

  // configure expander 1
  _init_io_expander(&_io_fs_switches, IO_EXPANDER1_MASK);
  if (pca95xx_init(&_io_fs_switches.exp, IO_EXPANDER_I2C_BUS, IO_EXPANDER1_I2C_ADDR) < 0) {
    TRACE("EXP1 INIT ERROR");
    return -1;
  }

  // configure expander 2
  _init_io_expander(&_io_switches, IO_EXPANDER2_MASK);
  if (pca95xx_init(&_io_switches.exp, IO_EXPANDER_I2C_BUS, IO_EXPANDER2_I2C_ADDR) < 0) {
    TRACE("EXP2 INIT ERROR");
    return -1;
  }

#if !defined(BOOT)
  // setup expanders pin change interrupt
  gpio_init_int(IO_INT_GPIO, GPIO_IN, GPIO_FALLING, _io_int_handler);
#endif

  _read_io_expander(&_io_switches);
  _read_io_expander(&_io_fs_switches);

#if !defined(BOOT)
  start_poll_timer();
#endif

  return 0;
}

uint32_t bsp_get_fs_switches()
{
  return _io_fs_switches.state;
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
    if ((state & def->Pin_high) != 0) {
      pos = SWITCH_HW_DOWN;
    }
  } else {
    bool hi = state & def->Pin_high;
    bool lo = state & def->Pin_low;

    if(!isSwitch3Pos(idx))
    {
      // Switch not declared as 3POS installed in a 3POS HW
      if (!(hi && lo)) {
        pos = SWITCH_HW_DOWN;
      }
    } else if (hi && lo) {
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

bool boardIsCustomSwitch(uint8_t idx);

SwitchHwPos boardSwitchGetPosition(uint8_t idx)
{
  if (boardIsCustomSwitch(idx)) {
    return _get_fs_switch_pos(idx);
  } else {
    return _get_switch_pos(idx);
  }
}
