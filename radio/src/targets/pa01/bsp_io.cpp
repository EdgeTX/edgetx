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

#include "bsp_io.h"
#include "hal.h"

#include "drivers/aw9523b.h"
#include "stm32_i2c_driver.h"
#include "stm32_switch_driver.h"

#define BSP_I2C_BUS I2C_Bus_1
#define BSP_I2C_ADDR 0x5b

#define BSP_IN_MASK                                                    \
  (AW9523B_PIN_0 | AW9523B_PIN_1 | AW9523B_PIN_2 | AW9523B_PIN_3)

#define BSP_OUT_MASK                                                   \
  (AW9523B_PIN_7 | AW9523B_PIN_8 | AW9523B_PIN_9 | AW9523B_PIN_10 |    \
   AW9523B_PIN_11 | AW9523B_PIN_12 | AW9523B_PIN_13 | AW9523B_PIN_14 | \
   AW9523B_PIN_15)

#define BSP_CHECK(x) if ((x) < 0) return -1

static aw9523b_t i2c_exp;

static uint16_t inputState = 0;
static bool shouldReadKeys = false;

static void _io_int_handler()
{
  shouldReadKeys = true;
}

bool bsp_get_shouldReadKeys()
{
  bool tmp = shouldReadKeys;
  shouldReadKeys = false;
  return tmp;
}

static volatile bool errorOccurs = false;
static void bsp_input_read()
{
  uint16_t value;
  if (aw9523b_read(&i2c_exp, BSP_IN_MASK, &value) < 0) {
    errorOccurs = true;
    return;
  }
  inputState = value;
}

int bsp_io_init()
{
  // init outputs
  BSP_CHECK(aw9523b_init(&i2c_exp, BSP_I2C_BUS, BSP_I2C_ADDR));
  BSP_CHECK(aw9523b_write(&i2c_exp, BSP_OUT_MASK,
    BSP_KEY_OUT1 | BSP_KEY_OUT2 | BSP_KEY_OUT3 | BSP_KEY_OUT4));
  BSP_CHECK(aw9523b_set_direction(&i2c_exp, 0xFFFF, BSP_IN_MASK));

  // setup expanders pin change interrupt
  gpio_init_int(IO_INT_GPIO, GPIO_IN, GPIO_FALLING, _io_int_handler);
  bsp_output_clear(BSP_U6_SELECT);
  return 0;
}

void bsp_output_set(uint16_t pin) { aw9523b_write(&i2c_exp, pin, pin); }

void bsp_output_set(uint16_t mask, uint16_t pin) { aw9523b_write(&i2c_exp, mask, pin); }

void bsp_output_clear(uint16_t pin) { aw9523b_write(&i2c_exp, pin, 0); }

uint16_t bsp_input_get()
{
  bsp_input_read();
  return inputState;
}

SwitchHwPos boardSwitchGetPosition(uint8_t idx)
{
  if (idx < 6) {
    extern const stm32_switch_t* boardGetSwitchDef(uint8_t idx);
    const stm32_switch_t* sw = boardGetSwitchDef(idx);
    return stm32_switch_get_position(sw);
  }

  return getFctKeyState(idx - 6) ? SWITCH_HW_DOWN : SWITCH_HW_UP;
}
