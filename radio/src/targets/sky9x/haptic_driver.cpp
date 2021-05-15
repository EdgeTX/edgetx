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

void hapticOff()
{
  PWM->PWM_DIS = PWM_DIS_CHID2 ;                                          // Disable channel 2
  PWM->PWM_OOV &= ~0x00040000 ;   // Force low
  PWM->PWM_OSS |= 0x00040000 ;    // Force low
}

// pwmPercent 0-100
void hapticOn( uint32_t pwmPercent )
{
  Pwm *pwmptr ;

  pwmptr = PWM ;

  if (pwmPercent > 100) {
    pwmPercent = 100 ;
  }

  pwmptr->PWM_CH_NUM[2].PWM_CDTYUPD = pwmPercent ;                // Duty
  pwmptr->PWM_ENA = PWM_ENA_CHID2 ;                                               // Enable channel 2
  pwmptr->PWM_OSC = 0x00040000 ;  // Enable output
}
