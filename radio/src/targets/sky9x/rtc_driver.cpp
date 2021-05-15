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

#if defined(REVX)
// TODO change this, not the right place
#define TWI_NONE          0
#define TWI_WRITE_VOL     2
#define TWI_WAIT_COMP             7
#define TWI_READ_RTC              8
#define TWI_WRITE_RTC             9
#define TWI_WAIT_RTCSTOP        10
#define TWI_WRITE_MFP     11
int8_t volumeRequired ;
int8_t Rtc_read_pending;
uint8_t Rtc_write_pending ;
int8_t Rtc_valid ;
uint8_t MFPsetting = 0 ;
uint8_t CALsetting = 0 ;
uint8_t *Twi_read_address ;
uint8_t TwiOperation ;
#define RTC_RX_BUXSIZE                  10
#define RTC_SIZE                        7
uint8_t Rtc_status[RTC_RX_BUXSIZE] ;
uint8_t *Rtc_write_ptr ;
uint32_t Rtc_write_count ;
uint8_t RtcConfig[8] ;          // For initial config and writing to RTC
// 0x80, 0, 0, 0x08, 0, 0, 0, 0x80

uint32_t fromBCD(uint8_t bcd_value)
{
  return (((bcd_value & 0xF0) * 10) >> 4) + (bcd_value & 0x0F);
}

uint32_t toBCD(uint32_t value)
{
  div_t qr = div(value, 10);
  return (qr.quot << 4) + qr.rem;
}

// This is called from an interrupt routine, or
// interrupts must be disabled while it is called
// from elsewhere.
void i2cCheck()
{
  if ( TWI0->TWI_IMR & TWI_IMR_TXCOMP ) {
    return ;                // Busy
  }

  if ( volumeRequired >= 0 )                             // Set volume to this value
  {
            TWI0->TWI_MMR = 0x002F0000 ;            // Device 5E (>>1) and master is writing
            TwiOperation = TWI_WRITE_VOL ;
            TWI0->TWI_THR = volumeRequired ;               // Send data
            volumeRequired = -1 ;
            TWI0->TWI_IER = TWI_IER_TXCOMP ;
            TWI0->TWI_CR = TWI_CR_STOP ;            // Stop Tx
  }
  else if ( Rtc_read_pending )
  {
          Rtc_valid = 0 ;
          Rtc_read_pending = 0 ;
          TWI0->TWI_MMR = 0x006F1100 ;            // Device 6F and master is reading, 1 byte addr
          TWI0->TWI_IADR = 0 ;
          TwiOperation = TWI_READ_RTC ;
#ifndef SIMU
          TWI0->TWI_RPR = (uint32_t)&Rtc_status[0] ;
#endif
          TWI0->TWI_RCR = RTC_SIZE - 1 ;
          if ( TWI0->TWI_SR & TWI_SR_RXRDY )
          {
                  (void) TWI0->TWI_RHR ;
          }

          TWI0->TWI_PTCR = TWI_PTCR_RXTEN ;       // Start transfers
          TWI0->TWI_CR = TWI_CR_START ;           // Start Rx
          TWI0->TWI_IER = TWI_IER_RXBUFF | TWI_IER_TXCOMP ;
  }
  else if ( Rtc_write_pending )
  {
          if ( Rtc_write_pending & (2|4) )
          {
                  TWI0->TWI_MMR = 0x006F0100 ;            // Device 6F and master is writing, 1 byte addr
                  TwiOperation = TWI_WRITE_MFP ;
                  if ( Rtc_write_pending & 2 )
                  {
                          TWI0->TWI_IADR = 7 ;
                          TWI0->TWI_THR = MFPsetting ;    // Send data
                          Rtc_write_pending &= ~2 ;
                  }
                  else
                  {
                          TWI0->TWI_IADR = 8 ;
                          TWI0->TWI_THR = CALsetting ;    // Send data
                          Rtc_write_pending &= ~4 ;
                  }
                  TWI0->TWI_IER = TWI_IER_TXCOMP ;
                  TWI0->TWI_CR = TWI_CR_STOP ;            // Stop Tx
          }
          else
          {
                  Rtc_write_pending &= ~1 ;
                  TWI0->TWI_MMR = 0x006F0100 ;            // Device 6F and master is writing, 1 byte addr
                  TWI0->TWI_IADR = 0 ;
                  TwiOperation = TWI_WRITE_RTC ;
#ifndef SIMU
                  TWI0->TWI_TPR = (uint32_t)Rtc_write_ptr+1 ;
#endif
                  TWI0->TWI_TCR = Rtc_write_count-1 ;
                  TWI0->TWI_THR = *Rtc_write_ptr ;        // First byte
                  TWI0->TWI_PTCR = TWI_PTCR_TXTEN ;       // Start data transfer
                  TWI0->TWI_IER = TWI_IER_TXBUFE | TWI_IER_TXCOMP ;
          }
  }
}

void writeRTC(uint8_t *ptr)
{
  uint32_t year;
  RtcConfig[0] = 0x80 | toBCD(*ptr++);
  RtcConfig[1] = toBCD(*ptr++);
  RtcConfig[2] = toBCD(*ptr++);
  RtcConfig[3] = 0x08;
  RtcConfig[4] = toBCD(*ptr++);
  RtcConfig[5] = toBCD( *ptr++);
  year = *ptr++;
  year |= *ptr << 8;
  RtcConfig[6] = toBCD(year - 2000);
  RtcConfig[7] = MFPsetting;
  Rtc_write_ptr = RtcConfig;
  Rtc_write_count = 8;
  __disable_irq();
  Rtc_write_pending |= 1;
  i2cCheck();
  __enable_irq();
}

void readRTC()
{
  Rtc_read_pending = 1 ;
  __disable_irq() ;
  i2cCheck() ;
  __enable_irq() ;
}

// TODO should be inside an i2c_driver.cpp
#ifndef SIMU
extern "C" void TWI0_IRQHandler()
{
        uint32_t status ;
        status = TWI0->TWI_SR ;         // Read only once, some bits cleared on read

        if ( TwiOperation == TWI_READ_RTC )
        {
                if ( status & TWI_SR_RXBUFF )
                {
                        TWI0->TWI_IDR = TWI_IDR_RXBUFF ;
                        TwiOperation = TWI_WAIT_RTCSTOP ;
                        TWI0->TWI_CR = TWI_CR_STOP ;    // Stop Rx
                        TWI0->TWI_RCR = 1 ;                                             // Last byte
                        return ;
                }
                else
                {
                        // must be TXCOMP, prob. NAK in data
                        if ( TWI0->TWI_RCR > 0 )
                        {
                                Rtc_valid = -1 ;
                                TWI0->TWI_CR = TWI_CR_STOP ;    // Stop Rx
                        }
                }
        }

        if ( TwiOperation == TWI_WAIT_RTCSTOP )
        {
                Rtc_valid = 1 ;
                // Set the date and time
                struct gtm utm;

                utm.tm_sec = fromBCD( Rtc_status[0] & 0x7F ) ;
                utm.tm_min = fromBCD( Rtc_status[1] & 0x7F ) ;
                utm.tm_hour = fromBCD( Rtc_status[2] & 0x3F ) ;
                utm.tm_mday = fromBCD( Rtc_status[4] & 0x3F ) ;
                utm.tm_mon = fromBCD( Rtc_status[5] & 0x1F ) - 1;
                utm.tm_year = fromBCD( Rtc_status[6] ) + 100 ;
                g_rtcTime = gmktime(&utm);

                TWI0->TWI_PTCR = TWI_PTCR_RXTDIS ;      // Stop transfers
                if ( status & TWI_SR_RXRDY )
                {
                        (void) TWI0->TWI_RHR ;                  // Discard any rubbish data
                }
        }

        if ( status & TWI_SR_NACK )
        {
                TWI0->TWI_CR = TWI_CR_STOP ;            // Stop Tx
        }

        TWI0->TWI_IDR = TWI_IDR_TXCOMP | TWI_IDR_TXBUFE | TWI_IDR_RXBUFF ;
        TWI0->TWI_PTCR = TWI_PTCR_TXTDIS | TWI_PTCR_RXTDIS ;    // Stop transfers
        if ( ( status & TWI_SR_TXCOMP ) == 0 )
        {
                TWI0->TWI_IER = TWI_IER_TXCOMP ;
                TwiOperation = TWI_WAIT_COMP ;
                return ;
        }

        TwiOperation = TWI_NONE ;
        i2cCheck() ;
}
#endif
#endif

struct t_i2cTime
{
  uint8_t setCode ;
  uint8_t Time[7] ;
} I2CTime ;

void rtcSetTime(const struct gtm * t)
{
  g_ms100 = 0; // start of next second begins now

  I2CTime.setCode = 0x74 ;    // Tiny SET TIME CODE command
  I2CTime.Time[0] = t->tm_sec ;
  I2CTime.Time[1] = t->tm_min ;
  I2CTime.Time[2] = t->tm_hour ;
  I2CTime.Time[3] = t->tm_mday ;
  I2CTime.Time[4] = t->tm_mon+1 ;
  I2CTime.Time[5] = (uint8_t) (t->tm_year+TM_YEAR_BASE);
  I2CTime.Time[6] = (t->tm_year+TM_YEAR_BASE) >> 8;
#if defined(REVX)
  writeRTC((uint8_t *)&I2CTime.Time[0]) ;
#elif defined(COPROCESSOR)
  coprocWriteData((uint8_t *) &I2CTime, 8);
#endif
}

void rtcInit()
{
#if defined(REVX)
  readRTC();
#elif defined(COPROCESSOR)
  coprocReadData();
#endif
}

#if defined(REVX)
void writeMFP()
{
  __disable_irq();
  Rtc_write_pending |= 2;
  i2cCheck();
  __enable_irq();
}

void setMFP()
{
  MFPsetting = 0x80;
  writeMFP();
}

void clearMFP()
{
  MFPsetting = 0;
  writeMFP();
}
#endif
