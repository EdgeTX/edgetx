/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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

#include "edgetx.h"

void pwrInit()
{
  // Configure RF_power (PC17)
  configure_pins(PIO_PC17, PIN_ENABLE | PIN_INPUT | PIN_PORTC | PIN_NO_PULLUP | PIN_PULLDOWN);
  configure_pins(PIO_PA8, PIN_ENABLE | PIN_INPUT | PIN_PORTA | PIN_PULLUP); // Enable bit A8 (Soft Power)
}

bool pwrPressed()
{
#if defined(SIMU)
  return true;
#else
  return PIOC->PIO_PDSR & PIO_PC17;
#endif
}

void pwrOn()
{
}

// turn off soft power
void pwrOff()
{
  configure_pins(PIO_PA8, PIN_ENABLE | PIN_OUTPUT | PIN_LOW | PIN_PORTA | PIN_NO_PULLUP);
}
