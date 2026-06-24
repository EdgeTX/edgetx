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

#if defined(V16_CHARGE_UI)
#include "edgetx.h"
#endif

void usbChargerInit()
{
  gpio_init(USB_CHARGER_GPIO, GPIO_IN_PU, GPIO_PIN_SPEED_LOW);
  gpio_init(USB_USBDet_GPIO, GPIO_IN_PU, GPIO_PIN_SPEED_LOW);
}

bool usbCableConnected()
{
  return gpio_read(USB_USBDet_GPIO) != 0;
}

bool usbChargerLed()
{
#if defined(V16_CHARGE_UI)
  // Charge-status GPIO can pulse during CV phase: latch on immediately,
  // require sustained inactive before clearing.
  static bool debounced = false;
  static tmr10ms_t inactiveSince = 0;
#endif

  if (!gpio_read(USB_USBDet_GPIO)) {
#if defined(V16_CHARGE_UI)
    debounced = false;
    inactiveSince = 0;
#endif
    return 0;
  }
#if defined(V16_CHARGE_UI)
  const bool charging = !gpio_read(USB_CHARGER_GPIO);

  if (charging) {
    debounced = true;
    inactiveSince = 0;
  } else if (debounced) {
    if (inactiveSince == 0) {
      inactiveSince = get_tmr10ms();
    } else if ((tmr10ms_t)(get_tmr10ms() - inactiveSince) >= 150) {
      debounced = false;
      inactiveSince = 0;
    }
  }

  return debounced;
#else
  return !gpio_read(USB_CHARGER_GPIO);
#endif
}
