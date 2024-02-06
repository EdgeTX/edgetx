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

#include "hal.h"
#include "stm32_gpio_driver.h"

void videoSwitchInit()
{
  LL_GPIO_InitTypeDef gpioInit;
  LL_GPIO_StructInit(&gpioInit);

  stm32_gpio_enable_clock(VIDEO_SWITCH_GPIO);
  
  gpioInit.Mode = LL_GPIO_MODE_OUTPUT;
  gpioInit.Speed = LL_GPIO_SPEED_FREQ_LOW;
  gpioInit.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  gpioInit.Pull = LL_GPIO_PULL_NO;

  gpioInit.Pin = VIDEO_SWITCH_GPIO_PIN;
  LL_GPIO_Init(VIDEO_SWITCH_GPIO, &gpioInit);
  LL_GPIO_SetOutputPin(VIDEO_SWITCH_GPIO, VIDEO_SWITCH_GPIO_PIN);
}

void switchToVideo()
{
  LL_GPIO_ResetOutputPin(VIDEO_SWITCH_GPIO, VIDEO_SWITCH_GPIO_PIN);
}

void switchToRadio()
{
  LL_GPIO_SetOutputPin(VIDEO_SWITCH_GPIO, VIDEO_SWITCH_GPIO_PIN);
}
