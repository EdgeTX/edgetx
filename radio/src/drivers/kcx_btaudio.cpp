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
#include "stm32_gpio.h"
#include "os/sleep.h"

void btAudioInit()
{
#if defined(BTAUDIO_POWER_GPIO)
  gpio_init(BTAUDIO_POWER_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
  gpio_set(BTAUDIO_POWER_GPIO);
#endif
  gpio_init(BTAUDIO_LINKED_GPIO, GPIO_IN, GPIO_PIN_SPEED_LOW);
  gpio_init(BTAUDIO_CONNECT_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
  gpio_set(BTAUDIO_CONNECT_GPIO);
}

bool btAudioLinked()
{
  return gpio_read(BTAUDIO_LINKED_GPIO) != 0;
}

void btAudioConnect()
{
  gpio_clear(BTAUDIO_CONNECT_GPIO);
  sleep_ms(100);
  gpio_set(BTAUDIO_CONNECT_GPIO);
}
