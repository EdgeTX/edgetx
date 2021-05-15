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

void rotaryEncoderInit()
{
  configure_pins(PIO_PC19 | PIO_PC21, PIN_ENABLE | PIN_INPUT | PIN_PORTC | PIN_PULLUP);        // 19 and 21 are rotary encoder
  configure_pins(PIO_PB6, PIN_ENABLE | PIN_INPUT | PIN_PORTB | PIN_PULLUP);            // rotary encoder switch
  PIOC->PIO_IER = PIO_PC19 | PIO_PC21;
  NVIC_EnableIRQ(PIOC_IRQn);
}

void rotaryEncoderEnd()
{
  NVIC_DisableIRQ(PIOC_IRQn);
  PIOC->PIO_IDR = PIO_PC19 | PIO_PC21;
}

volatile uint32_t Rotary_position;
extern "C" void PIOC_IRQHandler()
{
  uint32_t dummy;

  dummy = PIOC->PIO_ISR;                 // Read and clear status register
  (void) dummy;                          // Discard value - prevents compiler warning

  dummy = PIOC->PIO_PDSR;                // Read Rotary encoder (PC19, PC21)
  dummy >>= 19;
  dummy &= 0x05;                 // pick out the three bits
  if (dummy != (Rotary_position & 0x05)) {
    if ((Rotary_position & 0x01) ^ ((dummy & 0x04) >> 2))
      rotencValue -= 1;
    else
      rotencValue += 1;

    Rotary_position &= ~0x45;
    Rotary_position |= dummy;
  }
}
