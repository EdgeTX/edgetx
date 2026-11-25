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

// CHG_ON (active high) switches USB VBUS through to the charger board.
// While the radio is on, driving it low stops charging. The charger board
// still charges when the radio is off.

#if defined(USB_CHARGE_CONTROL)
void usbChargerEnableCharge(bool enable)
{
  if (enable)
    gpio_set(USB_CHARGER_GPIO);
  else
    gpio_clear(USB_CHARGER_GPIO);
}
#endif

void usbChargerInit()
{
  gpio_init(USB_CHARGER_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
  gpio_set(USB_CHARGER_GPIO);
}

bool usbChargerLed()
{
  // No charge-status signal reaches the MCU (the charger's status only
  // drives its own LED), so report "charging" whenever USB is present and
  // the charger input is enabled. A full battery cannot be detected.
  return gpio_read(USB_CHARGER_GPIO) && usbPlugged();
}
