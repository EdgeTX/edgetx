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

  #if defined(USB_CHARGE_CONTROL)
void usbChargerEnableCharge(bool enable)
{
  if (enable)
    gpio_clear(UCHARGER_EN_GPIO);
  else
    gpio_set(UCHARGER_EN_GPIO);
}
#endif

void usbChargerInit()
{
  gpio_init(UCHARGER_GPIO, GPIO_IN_PU, GPIO_PIN_SPEED_LOW);
  gpio_init(UCHARGER_EN_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
  gpio_clear(UCHARGER_EN_GPIO);
}

bool usbChargerLed()
{
#if defined(USB_CHARGE_CONTROL)
  // When charging is disabled via the "USB SD/Joystick/VCP charge" setting,
  // the charger-enable pin is driven high to hold the charger off, so it is
  // not charging.
  if (gpio_read(UCHARGER_EN_GPIO))
    return false;
#endif
  return (gpio_read(UCHARGER_GPIO) && usbPlugged());
}
