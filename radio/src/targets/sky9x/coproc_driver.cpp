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

int8_t volumeRequired ;
uint8_t coprocReadDataPending ;
uint8_t coprocWriteDataPending ;
uint8_t CoProc_appgo_pending ;
uint8_t Volume_read ;
CoprocData coprocData;
bool get_onlytemp;
#if !defined(SIMU)
// TODO not used? static uint8_t *Twi_read_address ;
#endif
static uint8_t TwiOperation ;

#define TWI_NONE          0
#define TWI_WRITE_VOL     2
#define TWI_READ_COPROC   3
#define TWI_COPROC_APPGO  4
#define TWI_WAIT_STOP     5
#define TWI_WRITE_COPROC  6

// Commands to the coprocessor bootloader/application
#define TWI_CMD_PAGEUPDATE          0x01  // TWI Command to program a flash page
#define TWI_CMD_EXECUTEAPP          0x02  // TWI Command to jump to the application program
#define TWI_CMD_SETREAD_ADDRESS     0x03  // TWI Command to set address to read from
#define TWI_CMD_WRITE_DATA          0x04  // TWI Command send data to the application
#define COPROC_RX_BUXSIZE   22
uint8_t Co_proc_status[COPROC_RX_BUXSIZE] ;
uint8_t *coprocWriteDataPtr ;
uint32_t coprocWriteDataSize ;


// This is called from an interrupt routine, or
// interrupts must be disabled while it is called
// from elsewhere.
void i2cCheck()
{
  if ( TWI0->TWI_IMR & TWI_IMR_TXCOMP ) {
    return ;    // Busy
  }

  if ( volumeRequired >= 0 ) {      // Set volume to this value
    TWI0->TWI_MMR = 0x002F0000 ;    // Device 5E (>>1) and master is writing
    TwiOperation = TWI_WRITE_VOL ;
    TWI0->TWI_THR = volumeRequired ;   // Send data
    volumeRequired = -1 ;
    TWI0->TWI_IER = TWI_IER_TXCOMP ;
    TWI0->TWI_CR = TWI_CR_STOP ;    // Stop Tx
  }
  else if (coprocReadDataPending) {
    coprocData.valid = 0 ;
    coprocReadDataPending = 0 ;
    TWI0->TWI_MMR = 0x00351000 ;    // Device 35 and master is reading
    TwiOperation = TWI_READ_COPROC ;
#ifndef SIMU
    TWI0->TWI_RPR = (uint32_t)&Co_proc_status[0] ;
#endif
    TWI0->TWI_RCR = COPROC_RX_BUXSIZE - 1 ;
    if ( TWI0->TWI_SR & TWI_SR_RXRDY ) {
      (void) TWI0->TWI_RHR ;
    }
    TWI0->TWI_PTCR = TWI_PTCR_RXTEN ; // Start transfers
    TWI0->TWI_CR = TWI_CR_START ;   // Start Rx
    TWI0->TWI_IER = TWI_IER_RXBUFF | TWI_IER_TXCOMP ;
  }
  else if ( CoProc_appgo_pending ) {
    CoProc_appgo_pending = 0 ;
    TWI0->TWI_MMR = 0x00350000 ;    // Device 35 and master is writing
    TwiOperation = TWI_COPROC_APPGO ;
    TWI0->TWI_THR = TWI_CMD_EXECUTEAPP ;  // Send appgo command
    TWI0->TWI_IER = TWI_IER_TXCOMP ;
    TWI0->TWI_CR = TWI_CR_STOP ;    // Stop Tx
  }
  else if ( coprocWriteDataPending ) {
    coprocWriteDataPending = 0 ;
    TWI0->TWI_MMR = 0x00350000 ;    // Device 35 and master is writing
    TwiOperation = TWI_WRITE_COPROC ;
#ifndef SIMU
    TWI0->TWI_TPR = (uint32_t)coprocWriteDataPtr ;
#endif
    TWI0->TWI_TCR = coprocWriteDataSize ;
    TWI0->TWI_THR = TWI_CMD_WRITE_DATA ;  // Send write command
    TWI0->TWI_PTCR = TWI_PTCR_TXTEN ; // Start data transfer
    TWI0->TWI_IER = TWI_IER_TXBUFE | TWI_IER_TXCOMP ;
  }
}

void coprocReadData(bool onlytemp)
{
  get_onlytemp = onlytemp;
  coprocReadDataPending = 1 ;
  __disable_irq() ;
  i2cCheck() ;
  __enable_irq() ;
}

void coprocWriteData(uint8_t *data, uint32_t size)
{
  coprocWriteDataPtr = data;
  coprocWriteDataSize = size;
  coprocWriteDataPending = 1;
  __disable_irq();
  i2cCheck();
  __enable_irq();
}

#if !defined(SIMU)
extern "C" void TWI0_IRQHandler()
{
  if ( TwiOperation == TWI_READ_COPROC )
  {
    if ( TWI0->TWI_SR & TWI_SR_RXBUFF )
    {
      TWI0->TWI_IDR = TWI_IDR_RXBUFF ;
      TwiOperation = TWI_WAIT_STOP ;
      TWI0->TWI_CR = TWI_CR_STOP ;  // Stop Rx
      TWI0->TWI_RCR = 1 ;           // Last byte
      return ;
    }
    else
    {
      coprocData.valid = -1 ;
    }
  }

  if ( TwiOperation == TWI_WAIT_STOP )
  {
    coprocData.valid = 1 ;
    coprocData.read = Co_proc_status[0] ;
    if ( coprocData.read & 0x80 ) {
      // Bootloader
      CoProc_appgo_pending = 1 ;  // Action application
    }
    else {
      // Got data from tiny app
      // Set the date and time
      struct gtm utm;
      if (!get_onlytemp) {
        utm.tm_sec = Co_proc_status[1] ;
        utm.tm_min = Co_proc_status[2] ;
        utm.tm_hour = Co_proc_status[3] ;
        utm.tm_mday = Co_proc_status[4] ;
        utm.tm_mon = Co_proc_status[5] - 1;
        utm.tm_year = (Co_proc_status[6] + ( Co_proc_status[7] << 8 )) - TM_YEAR_BASE;
        g_rtcTime = gmktime(&utm);
      }
      coprocData.temp = Co_proc_status[8];
      if (coprocData.temp > coprocData.maxtemp)
        coprocData.maxtemp = coprocData.temp;
    }
    TWI0->TWI_PTCR = TWI_PTCR_RXTDIS ;  // Stop transfers
    if ( TWI0->TWI_SR & TWI_SR_RXRDY ) {
      (void) TWI0->TWI_RHR ;      // Discard any rubbish data
    }
  }

//  if ( TwiOperation == TWI_WRITE_VOL )
//  {

//  }

  if ( TwiOperation == TWI_WRITE_COPROC )
  {
    if ( TWI0->TWI_SR & TWI_SR_TXBUFE )
    {
      TWI0->TWI_IDR = TWI_IDR_TXBUFE ;
      TWI0->TWI_CR = TWI_CR_STOP ;    // Stop Tx
      TWI0->TWI_PTCR = TWI_PTCR_TXTDIS ;  // Stop transfers
      TwiOperation = TWI_NONE ;
      return ;
    }
  }

  TWI0->TWI_IDR = TWI_IDR_TXCOMP | TWI_IDR_TXBUFE | TWI_IDR_RXBUFF ;
  TWI0->TWI_PTCR = TWI_PTCR_TXTDIS | TWI_PTCR_RXTDIS ; // Stop transfers
  if ( TWI0->TWI_SR & TWI_SR_NACK )
  {
  }
  TwiOperation = TWI_NONE ;
  i2cCheck() ;
}
#endif
