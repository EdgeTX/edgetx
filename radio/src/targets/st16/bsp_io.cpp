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

#include "stm32_i2c_driver.h"
#include "stm32_switch_driver.h"

extern const stm32_switch_t* boardGetSwitchDef(uint8_t idx);

#define BSP_IN_I2C_BUS I2C_Bus_2
#define BSP_IN_I2C_ADDR 0x20

#define BSP_IN_MASK 0xFFFF

#define BSP_OUT_I2C_BUS I2C_Bus_2
#define BSP_OUT_I2C_ADDR 0x21

#define BSP_OUT_MASK                                               \
  (PCA95XX_PIN_0 | PCA95XX_PIN_1 | PCA95XX_PIN_2 | PCA95XX_PIN_3 | \
   PCA95XX_PIN_4 | PCA95XX_PIN_5 | PCA95XX_PIN_6 | PCA95XX_PIN_7 | \
   PCA95XX_PIN_8 | PCA95XX_PIN_9 | PCA95XX_PIN_10)

#define BSP_CHECK(x) \
  if ((x) < 0) return -1

static pca95xx_t output_exp;
static pca95xx_t input_exp;

static uint16_t inputState = 0;
static bool updateInputState = false;

void bsp_port_extender_irq_handler() { updateInputState = true; }

static void bsp_input_read()
{
  // this is a fall back, the interrupts from the port extender
  // are unreliable under some circumstances
  static int readCount = 0;
  readCount++;

  if (!updateInputState && readCount < 50) return;

  uint16_t value=0;

  readCount=0;
  updateInputState = false;

  if (pca95xx_read(&input_exp, BSP_IN_MASK, &value) < 0) return;
  inputState = value;
}

int bsp_io_init()
{
  // init outputs
  BSP_CHECK(pca95xx_init(&output_exp, BSP_OUT_I2C_BUS, BSP_OUT_I2C_ADDR));
  BSP_CHECK(pca95xx_write(&output_exp, BSP_OUT_MASK, BSP_EXT_PWR | BSP_INT_PWR));
  BSP_CHECK(pca95xx_set_direction(&output_exp, BSP_OUT_MASK, 0));

  // init inputs
  BSP_CHECK(pca95xx_init(&input_exp, BSP_IN_I2C_BUS, BSP_IN_I2C_ADDR));

  gpio_init_int(GPIO_PIN(GPIOH, 6), GPIO_IN, GPIO_FALLING,
                bsp_port_extender_irq_handler);
  updateInputState = true;
  bsp_input_read();

  return 0;
}

void bsp_output_set(uint16_t pin) { pca95xx_write(&output_exp, pin, pin); }

void bsp_output_clear(uint16_t pin) { pca95xx_write(&output_exp, pin, 0); }

uint16_t bsp_input_get()
{
  bsp_input_read();
  return inputState;
}

SwitchHwPos boardSwitchGetPosition(uint8_t idx)
{
  const stm32_switch_t* sw = boardGetSwitchDef(idx);

  if (sw->type == SWITCH_HW_ADC || sw->GPIOx_high != nullptr)
    return stm32_switch_get_position(sw);

  bool val = false;
  uint16_t pins = bsp_input_get();

  val = (pins & sw->Pin_high) != 0;

  if (sw->inverted) val = !val;

  return val ? SWITCH_HW_UP : SWITCH_HW_DOWN;
}
