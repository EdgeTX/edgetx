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

// Start TIMER3 for input capture
inline void start_timer3()
{
  Tc *ptc ;

  // Enable peripheral clock TC0 = bit 23 thru TC5 = bit 28
  PMC->PMC_PCER0 |= 0x04000000L ;               // Enable peripheral clock to TC3

  ptc = TC1 ;           // Tc block 1 (TC3-5)
  ptc->TC_BCR = 0 ;                       // No sync
  ptc->TC_BMR = 2 ;
  ptc->TC_CHANNEL[0].TC_CMR = 0x00000000 ;        // Capture mode
  ptc->TC_CHANNEL[0].TC_CMR = 0x00090005 ;        // 0000 0000 0000 1001 0000 0000 0000 0101, XC0, A rise, b fall
  ptc->TC_CHANNEL[0].TC_CCR = 5 ;         // Enable clock and trigger it (may only need trigger)

  configure_pins( PIO_PC23, PIN_PERIPHERAL | PIN_INPUT | PIN_PER_B | PIN_PORTC | PIN_PULLUP ) ;

  NVIC_SetPriority( TC3_IRQn, 14 ) ; // Low priority interrupt
  NVIC_EnableIRQ(TC3_IRQn) ;
  ptc->TC_CHANNEL[0].TC_IER = TC_IER0_LDRAS ;
}

// Start Timer4 to provide 0.5uS clock for input capture
void start_timer4()
{
  Tc *ptc ;
  uint32_t timer ;

  timer = Master_frequency / (2*2000000) ;                // MCK/2 and 2MHz

  // Enable peripheral clock TC0 = bit 23 thru TC5 = bit 28
  PMC->PMC_PCER0 |= 0x08000000L ;               // Enable peripheral clock to TC4

  ptc = TC1 ;           // Tc block 1 (TC3-5)
  ptc->TC_BCR = 0 ;                       // No sync
  ptc->TC_BMR = 0 ;
  ptc->TC_CHANNEL[1].TC_CMR = 0x00008000 ;        // Waveform mode
  ptc->TC_CHANNEL[1].TC_RC = timer ;
  ptc->TC_CHANNEL[1].TC_RA = timer >> 1 ;
  ptc->TC_CHANNEL[1].TC_CMR = 0x0009C000 ;        // 0000 0000 0000 1001 1100 0000 0100 0000
  // MCK/2, set @ RA, Clear @ RC waveform
  ptc->TC_CHANNEL[1].TC_CCR = 5 ;         // Enable clock and trigger it (may only need trigger)
}

// Timer3 used for PPM_IN pulse width capture. Counter running at 16MHz / 8 = 2MHz
// equating to one count every half millisecond. (2 counts = 1ms). Control channel
// count delta values thus can range from about 1600 to 4400 counts (800us to 2200us),
// corresponding to a PPM signal in the range 0.8ms to 2.2ms (1.5ms at center).
// (The timer is free-running and is thus not reset to zero at each capture interval.)
// Timer 4 generates the 2MHz clock to clock Timer 3

extern "C" void TC3_IRQHandler() //capture ppm in at 2MHz
{
  uint32_t status = TC1->TC_CHANNEL[0].TC_SR;
  if (status & TC_SR0_LDRAS) {
    uint16_t capture = TC1->TC_CHANNEL[0].TC_RA ;
    captureTrainerPulses(capture);
  }
}

void init_trainer_capture()
{
  PIOC->PIO_PER = PIO_PC22;
  start_timer4() ;
  start_timer3() ;
}

void stop_trainer_capture()
{
  PIOC->PIO_PDR = PIO_PC22;
  TC1->TC_CHANNEL[0].TC_IDR = TC_IDR0_LDRAS ;
  NVIC_DisableIRQ(TC3_IRQn) ;
}
