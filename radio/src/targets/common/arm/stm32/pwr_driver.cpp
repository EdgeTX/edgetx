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
#include "hal/abnormal_reboot.h"

void pwrInit()
{
#if defined(SD_PRESENT_GPIO)
  gpio_init(SD_PRESENT_GPIO, GPIO_IN_PU, GPIO_PIN_SPEED_LOW);
#endif

#if defined(INTMODULE_BOOTCMD_GPIO)
  gpio_init(INTMODULE_BOOTCMD_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
  gpio_write(INTMODULE_BOOTCMD_GPIO, INTMODULE_BOOTCMD_DEFAULT);
#endif

  // Internal module power
#if defined(HARDWARE_INTERNAL_MODULE)
  gpio_init(INTMODULE_PWR_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
  INTERNAL_MODULE_OFF();
#endif

  // External module power
#if defined(HARDWARE_EXTERNAL_MODULE)
  gpio_init(EXTMODULE_PWR_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
  EXTERNAL_MODULE_PWR_OFF();
#endif

  // PWR switch
#if defined(PWR_SWITCH_GPIO)
  gpio_init(PWR_SWITCH_GPIO, GPIO_IN_PU, GPIO_PIN_SPEED_LOW);
#endif

#if defined(PWR_EXTRA_SWITCH_GPIO)
  // PWR Extra switch
  gpio_init(PWR_EXTRA_SWITCH_GPIO, GPIO_IN_PU, GPIO_PIN_SPEED_LOW);
#endif

#if defined(PCBREV_HARDCODED)
  hardwareOptions.pcbrev = PCBREV_HARDCODED;
#elif defined(PCBREV_GPIO)
  #if defined(PCBREV_GPIO_PULL_DOWN)
    gpio_init(PCBREV_GPIO, GPIO_IN_PD, GPIO_PIN_SPEED_LOW);
  #else
    gpio_init(PCBREV_GPIO, GPIO_IN_PU, GPIO_PIN_SPEED_LOW);
  #endif
  hardwareOptions.pcbrev = PCBREV_VALUE();
#elif defined(PCBREV_GPIO_1) && defined(PCBREV_GPIO_2)
  gpio_init(PCBREV_GPIO_1, GPIO_IN_PU, GPIO_PIN_SPEED_LOW);
  #if defined(PCBREV_TOUCH_GPIO)
    #if defined(PCBREV_TOUCH_GPIO_PULL_UP)
      gpio_init(PCBREV_GPIO_2, GPIO_IN_PU, GPIO_PIN_SPEED_LOW);
    #else
      gpio_init(PCBREV_TOUCH_GPIO, GPIO_IN_PD, GPIO_PIN_SPEED_LOW);
    #endif
  #endif

  hardwareOptions.pcbrev = PCBREV_VALUE();
#endif

  // Aux serial port power
#if defined(AUX_SERIAL_PWR_GPIO)
  gpio_init(AUX_SERIAL_PWR_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
#endif
#if defined(AUX2_SERIAL_PWR_GPIO)
  gpio_init(AUX2_SERIAL_PWR_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
#endif
}

void pwrOn()
{
#if defined(PWR_ON_GPIO)
  gpio_init(PWR_ON_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
  gpio_set(PWR_ON_GPIO);
#endif
}

void pwrOff()
{
#if defined(PWR_ON_GPIO)
  gpio_clear(PWR_ON_GPIO);
#endif
}

#if defined(PWR_EXTRA_SWITCH_GPIO)
bool pwrForcePressed()
{
  return !gpio_read(PWR_SWITCH_GPIO) && !gpio_read(PWR_EXTRA_SWITCH_GPIO);
}
#endif

bool pwrPressed()
{
#if defined(PWR_EXTRA_SWITCH_GPIO)
  return !gpio_read(PWR_SWITCH_GPIO) && !gpio_read(PWR_EXTRA_SWITCH_GPIO);
#elif defined(PWR_SWITCH_GPIO)
  return !gpio_read(PWR_SWITCH_GPIO);
#else
  return true;
#endif
}

bool pwrOffPressed()
{
#if defined(PWR_BUTTON_PRESS)
  return pwrPressed();
#elif defined(PWR_SWITCH_GPIO)
  return !pwrPressed();
#else
  return false;
#endif
}

void pwrResetHandler()
{
  if (WAS_RESET_BY_WATCHDOG_OR_SOFTWARE()) {
    pwrOn();
  }
}
