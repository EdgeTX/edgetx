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
#include "delays_driver.h"
#include "stm32_ws2812.h"

#include "os/async.h"
#include "os/timer.h"

#include "myeeprom.h"

struct bsp_io_expander {
  pca95xx_t exp;
  uint32_t mask;
  uint32_t state;
};

static volatile bool _poll_switches_in_queue = false;

static bsp_io_expander _io_switches;
static bsp_io_expander _io_fs_switches;

static timer_handle_t _poll_timer = TIMER_INITIALIZER;

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
  } else {
    gpio_clear(IO_RESET_GPIO);
    TRACE("PCA95 was reset");
    delay_us(1);  // Only 6ns are needed according to PCA datasheet, but lets be safe
    gpio_set(IO_RESET_GPIO);
  }
  return io->state;  
}

typedef enum {
  TRIGGERED_BY_TIMER = 0,
  TRIGGERED_BY_IRQ,  
} trigger_source_t;

static void _poll_switches(void *param1, uint32_t trigger_source)
{
  (void)param1;

  if (trigger_source == TRIGGERED_BY_IRQ) {
    _poll_switches_in_queue = false;
    timer_reset(&_poll_timer);
  }

  bsp_io_read_switches();
  bsp_io_read_fs_switches();
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

int bsp_io_init()
{
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

  bsp_io_read_switches();
  bsp_io_read_fs_switches();

  start_poll_timer();

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

static constexpr uint32_t RGB_OFFSET = (1 << 16); // first after bspio pins
static uint16_t soft2POSLogicalState = 0xFFFF;

static const bsp_io_sw_def _switch_defs[] = {
  { SWITCH_A, RGB_OFFSET + 7 },
  { SWITCH_B_H, SWITCH_B_L },
  { SWITCH_C_H, SWITCH_C_L },
  { SWITCH_D, RGB_OFFSET + 6 },
  { SWITCH_E_H, SWITCH_E_L },
  { SWITCH_F_H, SWITCH_F_L },
  { SWITCH_G, 0 },
  { SWITCH_H, 0 },
};

static SwitchHwPos _get_switch_pos(uint8_t idx)
{
  static uint32_t oldState = 0;
  SwitchHwPos pos = SWITCH_HW_UP;
  const bsp_io_sw_def* def = &_switch_defs[idx];

  uint32_t state = _io_switches.state;

  if (def->pin_low > RGB_OFFSET) {
    // Potential soft 2pos
    if ((SWITCH_CONFIG(idx) == SWITCH_TOGGLE)) {
      if ((state & def->pin_high) == 0) {
        pos = SWITCH_HW_DOWN;
      }
    }
    else {
      if (((state & def->pin_high) == 0) && ((state & def->pin_high) != (oldState & def->pin_high))) {
        soft2POSLogicalState ^= def->pin_high;
      }
      if ((soft2POSLogicalState & def->pin_high) == 0) {
        pos = SWITCH_HW_DOWN;
      }
      else {
        pos = SWITCH_HW_UP;
      }

    }

    if (pos == SWITCH_HW_UP) {
      ws2812_set_color(def->pin_low - RGB_OFFSET, 0x0, 0x0, 0x0);
    } else {
      ws2812_set_color(def->pin_low - RGB_OFFSET, 0xFF, 0xFF, 0xFF);
    }
  }
  else if (!def->pin_low) {
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

  if (idx == switchGetMaxSwitches() - 1)
    oldState = state;

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

SwitchHwPos boardSwitchGetPosition(SwitchCategory cat, uint8_t idx)
{
  if (cat == SWITCH_PHYSICAL) {
    return _get_switch_pos(idx);
  } else if (cat == SWITCH_FUNCTION){
    return _get_fs_switch_pos(idx);
  }

  return SWITCH_HW_UP;
}
