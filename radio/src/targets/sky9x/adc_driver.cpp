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

uint16_t adcValues[NUM_ANALOGS];

#if defined(FRSKY_STICKS)
const char ana_direction[NUM_ANALOGS] = {1, 1, 0, 1 ,0 ,1 ,0, 0, 0};
#endif


// Settings for mode register ADC_MR
// USEQ off - silicon problem, doesn't work
// TRANSFER = 3
// TRACKTIM = 15 (16 clock periods)
// ANACH = 1
// SETTLING = 6 (not used if ANACH = 0)
// STARTUP = 6 (96 clock periods)
// PRESCAL = 9.0 MHz clock (between 1 and 20MHz)
// FREERUN = 0
// FWUP = 0
// SLEEP = 0
// LOWRES = 0
// TRGSEL = 0
// TRGEN = 0 (software trigger only)
void adcInit()
{
  Adc *padc ;
  uint32_t timer ;

  timer = ( Master_frequency / (3600000*2) ) << 8 ;
  // Enable peripheral clock ADC = bit 29
  PMC->PMC_PCER0 |= 0x20000000L ;               // Enable peripheral clock to ADC
  padc = ADC ;
  padc->ADC_MR = 0x3FB60000 | timer ;  // 0011 1111 1011 0110 xxxx xxxx 0000 0000
  padc->ADC_ACR = ADC_ACR_TSON ;                        // Turn on temp sensor
  padc->ADC_CHER = 0x0000E33E ;  // channels 1,2,3,4,5,8,9,13,14,15
  padc->ADC_CGR = 0 ;  // Gain = 1, all channels
  padc->ADC_COR = 0 ;  // Single ended, 0 offset, all channels
}

// Read 8 (9 for REVB) ADC channels
// Documented bug, must do them 1 by 1
void adcSingleRead()
{
  Adc *padc;
  uint32_t y;
  uint32_t x;

  for (uint8_t i=0; i<4; i++)
  padc = ADC;
  y = padc->ADC_ISR; // Clear EOC flags
  for (y = NUM_ANALOGS+1; --y > 0;) {
    padc->ADC_CR = 2; // Start conversion
    x = 0;
    while ((padc->ADC_ISR & 0x01000000) == 0) {
      // wait for DRDY flag
      if (++x > 1000000) {
        break; // Software timeout
      }
    }
    x = padc->ADC_LCDR; // Clear DRSY flag
  }
  // Next bit may be done using the PDC
  adcValues[0] = ADC->ADC_CDR1;
  adcValues[1] = ADC->ADC_CDR2;
  adcValues[2] = ADC->ADC_CDR3;
  adcValues[3] = ADC->ADC_CDR4;
  adcValues[4] = ADC->ADC_CDR5;
  adcValues[5] = ADC->ADC_CDR9;
  adcValues[6] = ADC->ADC_CDR13;
  adcValues[7] = ADC->ADC_CDR14;
  adcValues[8] = ADC->ADC_CDR8 ;

  // adc direction correct
#if defined(FRSKY_STICKS)
  uint32_t i ;
  for (i=0; i<NUM_ANALOGS; i++) {
    if (ana_direction[i]) {
      adcValues[i] = 4096-adcValues[i];
    }
  }  
#endif
}

void adcRead()
{
  uint16_t temp[NUM_ANALOGS] = { 0 };

  for (int i=0; i<4; i++) {
    adcSingleRead();
    for (uint8_t x=0; x<NUM_ANALOGS; x++) {
      uint16_t val = adcValues[x];
#if defined(JITTER_MEASURE)
      if (JITTER_MEASURE_ACTIVE()) {
        rawJitter[x].measure(val);
      }
#endif
      temp[x] += val;
    }
  }

  for (uint8_t x=0; x<NUM_ANALOGS; x++) {
    adcValues[x] = temp[x] >> 2;
  }
}


#if !defined(SIMU)
uint16_t getAnalogValue(uint8_t index)
{
#if defined(PCBSKY9X)
  static const uint8_t mapping[] = {1,5,7,0,4,6,2,3};
  index = mapping[index];
#endif
  return adcValues[index];
}
#endif // #if !defined(SIMU)
