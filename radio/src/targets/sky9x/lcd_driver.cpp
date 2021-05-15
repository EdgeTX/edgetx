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

// LCD i/o pins
// LCD_RES     PC27
// LCD_CS1     PC26
// LCD_E       PC12
// LCD_RnW     PC13
// LCD_A0      PC15
// LCD_D0      PC0
// LCD_D1      PC7
// LCD_D2      PC6
// LCD_D3      PC5
// LCD_D4      PC4
// LCD_D5      PC3
// LCD_D6      PC2
// LCD_D7      PC1

#define LCD_DATA  0x000000FFL
#define LCD_A0    0x00000080L
#define LCD_RnW   0x00002000L
#define LCD_E     0x00001000L
#define LCD_CS1   0x04000000L
#define LCD_RES   0x08000000L

void lcdSendCtl(uint8_t val)
{
  Pio *pioptr;

  pioptr = PIOC;
#if !defined(REVX)
    pioptr->PIO_CODR = LCD_CS1;            // Select LCD
#endif
  PIOA->PIO_CODR = LCD_A0;
#if !defined(REVX)
    pioptr->PIO_CODR = LCD_RnW;            // Write
#endif
  pioptr->PIO_ODSR = val;

  pioptr->PIO_SODR = LCD_E;                      // Start E pulse
  // Need a delay here (250nS)
  TC0->TC_CHANNEL[0].TC_CCR = 5; // Enable clock and trigger it (may only need trigger)
  while ( TC0->TC_CHANNEL[0].TC_CV < 9 )          // Value depends on MCK/2 (used 18MHz)
  {
    // Wait
  }
  pioptr->PIO_CODR = LCD_E;                      // End E pulse
  PIOA->PIO_SODR = LCD_A0;                       // Data

#if !defined(REVX)
  pioptr->PIO_SODR = LCD_CS1;                    // Deselect LCD
#endif
}

void lcdInit()
{
  Pio *pioptr;
  // /home/thus/txt/datasheets/lcd/KS0713.pdf
  // ~/txt/flieger/ST7565RV17.pdf  from http://www.glyn.de/content.asp?wdid=132&sid=

  // read the inputs, and lock the LCD lines
  lcdInputs = PIOC->PIO_PDSR; // 6 LEFT, 5 RIGHT, 4 DOWN, 3 UP ()
  lcdLock = 1;

  configure_pins( LCD_A0, PIN_ENABLE | PIN_LOW | PIN_OUTPUT | PIN_PORTA | PIN_NO_PULLUP );
  pioptr = PIOC;
#if defined(REVX)
  pioptr->PIO_PER = PIO_PC27 | PIO_PC12 | 0xFF;
  pioptr->PIO_CODR = LCD_E;
  pioptr->PIO_CODR = LCD_RnW | LCD_CS1;  // No longer needed, used elsewhere
#else
  pioptr->PIO_PER = PIO_PC27 | PIO_PC26 | PIO_PC13 | PIO_PC12 | 0xFF;
  pioptr->PIO_CODR = LCD_E | LCD_RnW | LCD_CS1;
#endif
  pioptr->PIO_SODR = LCD_RES;
  pioptr->PIO_OER = PIO_PC27 | PIO_PC26 | PIO_PC13 | PIO_PC12 | 0xFF;          // Set bits 27,26,13,12,7-0 output
  pioptr->PIO_OWER = 0x000000FFL;                // Allow write to ls 8 bits in ODSR

  TC0->TC_CHANNEL[0].TC_CCR = 5;       // Enable clock and trigger it (may only need trigger)
  pioptr->PIO_CODR = LCD_RES;            // Reset LCD
  while ( TC0->TC_CHANNEL[0].TC_CV < 500 ) ;        // > 10us, Value depends on MCK/2 (used 18MHz)
  TC0->TC_CHANNEL[0].TC_CCR = 5; // Enable clock and trigger it (may only need trigger)
  pioptr->PIO_SODR = LCD_RES;            // Remove LCD reset
  while ( TC0->TC_CHANNEL[0].TC_CV < 27000 ) ;              // 1500us, Value depends on MCK/2 (used 18MHz)
  lcdSendCtl(0xe2); //Initialize the internal functions
  lcdSendCtl(0xae); //DON = 0: display OFF
  lcdSendCtl(0xa1); //ADC = 1: reverse direction(SEG132->SEG1)
  lcdSendCtl(0xA6); //REV = 0: non-reverse display
  lcdSendCtl(0xA4); //EON = 0: normal display. non-entire
  lcdSendCtl(0xA2); // Select LCD bias=0
  lcdSendCtl(0xC0); //SHL = 0: normal direction (COM1->COM64)
  lcdSendCtl(0x2F); //Control power circuit operation VC=VR=VF=1
  lcdSendCtl(0x25); //Select int resistance ratio R2 R1 R0 =5
  lcdSendCtl(0x81); //Set reference voltage Mode
  lcdSendCtl(0x22); // 24 SV5 SV4 SV3 SV2 SV1 SV0 = 0x18

#if defined(REVX)
  // 200mS delay (only if not wdt reset)
  if ( ( ( ResetReason & RSTC_SR_RSTTYP ) != (2 << 8) ) && !globalData.unexpectedShutdown ) {
    for (uint32_t j = 0; j < 100; j += 1 ) {
      TC0->TC_CHANNEL[0].TC_CCR = 5; // Enable clock and trigger it (may only need trigger)
      while ( TC0->TC_CHANNEL[0].TC_CV < 36000 ) {
        // Value depends on MCK/2 (used 18MHz) give 2mS delay
        WDG_RESET(); // Wait
      }
    }
  }
#endif

  lcdSendCtl(0xAF); // DON = 1: display ON

  pioptr->PIO_ODR = 0x0000003AL;         // Set bits 1, 3, 4, 5 input
  pioptr->PIO_PUER = 0x0000003AL;                // Set bits 1, 3, 4, 5 with pullups
  pioptr->PIO_ODSR = 0;                                                  // Drive D0 low

  lcdLock = 0;
}

void lcdSetRefVolt(uint8_t val)
{
  Pio *pioptr;
  pioptr = PIOC;

  // read the inputs, and lock the LCD lines
  lcdInputs = PIOC->PIO_PDSR; // 6 LEFT, 5 RIGHT, 4 DOWN, 3 UP ()
  lcdLock = 1;

  pioptr->PIO_OER = 0x0C00B0FFL;         // Set bits 27,26,15,13,12,7-0 output

  lcdSendCtl(0x81);
  if ( val == 0 )
  {
    val = 0x22;
  }
  lcdSendCtl(val);

  pioptr->PIO_ODR = 0x000000FEL;         // Set bits 1, 3, 4, 5 input
  pioptr->PIO_PUER = 0x000000FEL;                // Set bits 1, 3, 4, 5 with pullups
  pioptr->PIO_ODSR = 0;                                                  // Drive D0 low

  lcdLock = 0;
}

void lcdRefresh()
{
  Pio *pioptr;
  uint8_t *p = displayBuf;
  uint32_t y;
  uint32_t x;
  uint32_t z;
  uint32_t ebit;

  ebit = LCD_E;

  pioptr = PIOA;
  pioptr->PIO_PER = 0x00000080; // Enable bit 7 (LCD-A0)
  pioptr->PIO_OER = 0x00000080;// Set bit 7 output

  // read the inputs, and lock the LCD lines
  lcdInputs = PIOC->PIO_PDSR; // 6 LEFT, 5 RIGHT, 4 DOWN, 3 UP ()
  lcdLock = 1;

  pioptr = PIOC;
  pioptr->PIO_OER = 0x0C0030FFL; // Set bits 27,26,15,13,12,7-0 output
  for (y = 0; y < 8; y++) {
    lcdSendCtl(g_eeGeneral.optrexDisplay ? 0 : 0x04);
    lcdSendCtl(0x10); //column addr 0
    lcdSendCtl(y | 0xB0); //page addr y

#if !defined(REVX)
    pioptr->PIO_CODR = LCD_CS1; // Select LCD
#endif
    PIOA->PIO_SODR = LCD_A0; // Data
#if !defined(REVX)
    pioptr->PIO_CODR = LCD_RnW; // Write
#endif

    x = *p;
    for (z = 0; z < LCD_W; z += 1) {
      pioptr->PIO_ODSR = x;
      pioptr->PIO_SODR = ebit; // Start E pulse
      // Need a delay here (250nS)
      p += 1;
      x = *p;
//                      TC0->TC_CHANNEL[0].TC_CCR = 5; // Enable clock and trigger it (may only need trigger)
//                      while ( TC0->TC_CHANNEL[0].TC_CV < 3 )          // Value depends on MCK/2 (used 6MHz)
//                      {
//                              // Wait
//                      }
      pioptr->PIO_CODR = ebit; // End E pulse
    }
#if !defined(REVX)
    pioptr->PIO_SODR = LCD_CS1; // Deselect LCD
#endif
  }

  pioptr->PIO_ODSR = 0xFF;                                       // Drive lines high
  pioptr->PIO_PUER = 0x000000FEL;        // Set bits 1, 3, 4, 5 with pullups
  pioptr->PIO_ODR = 0x000000FEL;         // Set bits 1, 3, 4, 5 input
  pioptr->PIO_ODSR = 0xFE;                                       // Drive D0 low
  lcdLock = 0;
}
