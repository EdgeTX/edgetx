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
#include "watchdog_driver.h"

void pwrInit()
{
#if defined(INTMODULE_BOOTCMD_GPIO)
  gpio_init(INTMODULE_BOOTCMD_GPIO, GPIO_OUT);
  gpio_write(INTMODULE_BOOTCMD_GPIO, INTMODULE_BOOTCMD_DEFAULT);
#endif

  // Internal module power
  gpio_init(INTMODULE_PWR_GPIO, GPIO_OUT);
  INTERNAL_MODULE_OFF();

  // External module power
  gpio_init(EXTMODULE_PWR_GPIO, GPIO_OUT);
  EXTERNAL_MODULE_PWR_OFF();

  // PWR switch
  gpio_init(PWR_SWITCH_GPIO, GPIO_IN_PU);

#if defined(PWR_EXTRA_SWITCH_GPIO)
  // PWR Extra switch
  gpio_init(PWR_EXTRA_SWITCH_GPIO, GPIO_IN_PU);
#endif

#if defined(PCBREV_HARDCODED)
  hardwareOptions.pcbrev = PCBREV_HARDCODED;
#elif defined(PCBREV_GPIO)
  #if defined(PCBREV_GPIO_PULL_DOWN)
    gpio_init(PCBREV_GPIO, GPIO_IN_PD);
  #endif
  gpio_init(PCBREV_GPIO, GPIO_IN_PU);
  hardwareOptions.pcbrev = PCBREV_VALUE();
#elif defined(PCBREV_GPIO_1) && defined(PCBREV_GPIO_2)
  gpio_init(PCBREV_GPIO_1, GPIO_IN_PU);
  gpio_init(PCBREV_GPIO_2, GPIO_IN_PU);
  hardwareOptions.pcbrev = PCBREV_VALUE();
#endif
}

void pwrOn()
{
  gpio_init(PWR_ON_GPIO, GPIO_OUT);
  gpio_set(PWR_ON_GPIO);
}

void pwrOff()
{
  gpio_clear(PWR_ON_GPIO);
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
#else
  return !gpio_read(PWR_SWITCH_GPIO);
#endif
}

bool pwrOffPressed()
{
#if defined(PWR_BUTTON_PRESS)
  return pwrPressed();
#else
  return !pwrPressed();
#endif
}

void pwrResetHandler()
{
  if (WAS_RESET_BY_WATCHDOG_OR_SOFTWARE()) {
    pwrOn();
  }
}
