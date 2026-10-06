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

#include "hal/gpio.h"
#include "stm32_gpio_driver.h"

// G11P has no I/O expander: the BSP outputs are plain GPIOs.
//
// Not all ST16 BSP_* outputs exist on G11P (module power, audio reset/mute and
// the LCD CS are handled directly in board.cpp / hal_g11p.h); the remaining
// ones map to the G11P nets below.

int bsp_io_init()
{
  // Outputs
  gpio_init(CHARGE_EN_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
  gpio_init(LCD_NRST_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
  gpio_set(LCD_NRST_GPIO);

  // Inputs (digital switch / keys)
  gpio_init(GPIO_PIN(GPIOH, 4), GPIO_IN, GPIO_PIN_SPEED_LOW); // SW1
  gpio_init(GPIO_PIN(GPIOC, 7), GPIO_IN, GPIO_PIN_SPEED_LOW); // K6A
  gpio_init(GPIO_PIN(GPIOA, 8), GPIO_IN, GPIO_PIN_SPEED_LOW); // K7A

  return 0;
}

void bsp_output_set(uint16_t pin)
{
  switch (pin) {
    case BSP_CHARGE_EN:
      gpio_set(CHARGE_EN_GPIO);
      break;
    case BSP_LCD_NRST:
      gpio_set(LCD_NRST_GPIO);
      break;
    default:
      break;
  }
}

void bsp_output_clear(uint16_t pin)
{
  switch (pin) {
    case BSP_CHARGE_EN:
      gpio_clear(CHARGE_EN_GPIO);
      break;
    case BSP_LCD_NRST:
      gpio_clear(LCD_NRST_GPIO);
      break;
    default:
      break;
  }
}

uint16_t bsp_input_get()
{
  return 0;
}

SwitchHwPos bsp_get_switch_position(const stm32_switch_t *sw,
                                    SwitchCategory cat, uint8_t idx)
{
  (void)sw;
  (void)cat;
  (void)idx;
  // G11P switch/key decoding is provided by g11p_switch_driver.cpp
  return SWITCH_HW_UP;
}
