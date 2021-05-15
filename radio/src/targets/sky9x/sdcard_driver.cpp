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

#define MCI_INITIAL_SPEED   400000

void init_SDcard()
{
#ifndef SIMU
  configure_pins( 0xFC000000, PIN_PERIPHERAL | PIN_INPUT | PIN_PER_C | PIN_PORTA | PIN_NO_PULLUP ) ;
  configure_pins( PIO_PB7, PIN_INPUT | PIN_PORTB | PIN_NO_PULLUP | PIN_NO_PULLDOWN ) ;

  unsigned short clkDiv;
  Hsmci *pMciHw = HSMCI ;

  /* Enable the MCI peripheral */
  PMC->PMC_PCER0 |= 1 << ID_HSMCI ;             // Enable peripheral clock to HSMCI
  pMciHw->HSMCI_CR = HSMCI_CR_SWRST;  /* Reset the MCI */
  pMciHw->HSMCI_CR = HSMCI_CR_MCIDIS | HSMCI_CR_PWSDIS;  /* Disable the MCI */
  pMciHw->HSMCI_IDR = 0xFFFFFFFF;  /* Disable all the interrupts */
  pMciHw->HSMCI_DTOR = HSMCI_DTOR_DTOCYC_Msk | HSMCI_DTOR_DTOMUL_Msk ;  /* Set the Data Timeout Register */
  pMciHw->HSMCI_CSTOR = HSMCI_CSTOR_CSTOCYC_Msk | HSMCI_CSTOR_CSTOMUL_Msk ;  /* CSTOR ? */
  /* Set the Mode Register: 400KHz for MCK = 48MHz (CLKDIV = 58) */
  clkDiv = 45; // (Master_frequency / (MCI_INITIAL_SPEED * 2)) - 1;
  pMciHw->HSMCI_MR = clkDiv | (7 << 8) ;

  /* Set the SDCard Register 1-bit, slot A */
  pMciHw->HSMCI_SDCR = HSMCI_SDCR_SDCSEL_SLOTA | HSMCI_SDCR_SDCBUS_1 ;
  /* Enable the MCI and the Power Saving */
  pMciHw->HSMCI_CR = HSMCI_CR_MCIEN/* | HSMCI_CR_PWSEN */;
  /* Configure MCI */
  pMciHw->HSMCI_CFG = HSMCI_CFG_FIFOMODE | HSMCI_CFG_FERRCTRL;
#endif
}
