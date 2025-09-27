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

#include "hal/gpio.h"
#include "stm32_gpio.h"

#include "board.h"
#include "hal/usb_driver.h"

void usbChargerInit()
{
  gpio_init(UCHARGER_GPIO, GPIO_IN_PU, GPIO_PIN_SPEED_LOW);
#if defined(UCHARGER_PW)
  gpio_init(UCHARGER_PW, GPIO_OUT, GPIO_PIN_SPEED_LOW);
  gpio_set(UCHARGER_PW);
  //gpio_clear(UCHARGER_PW);  0=DISABLE 1=ENABLE
#endif
#if defined(UCHARGER_EN)
  gpio_init(UCHARGER_EN, GPIO_OUT, GPIO_PIN_SPEED_LOW);
  //gpio_set(UCHARGER_EN);
  gpio_clear(UCHARGER_EN);  //cherge EN  0=ENANLE 1=DISABLE
#endif
#if defined(USB_GPIO_SEL)
  gpio_init(USB_GPIO_SEL, GPIO_OUT, GPIO_PIN_SPEED_LOW);
  gpio_set(USB_GPIO_SEL);

  //gpio_clear(USB_GPIO_SEL); // USB HUB SELECT 0->H7 MCU  1->CHARGE MCU
#endif
}

bool usbChargerLed()
{
  return (gpio_read(UCHARGER_GPIO) && usbPlugged());
}
