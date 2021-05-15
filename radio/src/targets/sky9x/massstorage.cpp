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

#ifndef SIMU

extern "C" {
#include "usb/device/massstorage/MSDDriver.h"
extern unsigned char MEDSdcard_Initialize(Media *media, unsigned char mciID);
}

/** LUN read/write buffer. */
unsigned char msdBuffer[MSD_BUFFER_SIZE];

unsigned int msdReadTotal=0, msdWriteTotal=0;

static void ConfigureUsbClock(void)
{
    /* Enable PLLB for USB */
    PMC->CKGR_PLLBR = CKGR_PLLBR_DIVB(1)
                    | CKGR_PLLBR_MULB(7)
                    | CKGR_PLLBR_PLLBCOUNT_Msk;
    while((PMC->PMC_SR & PMC_SR_LOCKB) == 0); // TODO  && (timeout++ < CLOCK_TIMEOUT));
    /* USB Clock uses PLLB */
    PMC->PMC_USB = PMC_USB_USBDIV(1)    /* /2   */
                 | PMC_USB_USBS;        /* PLLB */
}

/** Maximum number of LUNs which can be defined. */
#define MAX_LUNS            1

/** Media index for different disks */
#define DRV_SDMMC           0    /** SD card */

Media medias[MAX_LUNS];

/*----------------------------------------------------------------------------
 *        Local variables
 *----------------------------------------------------------------------------*/

/** Device LUNs. */
MSDLun luns[MAX_LUNS];

int itoa(int val, char* buf)
{
    const unsigned int radix = 10;

    char* p;
    unsigned int a;        //every digit
    int len;
    char* b;            //start of the digit char
    char temp;
    unsigned int u;

    p = buf;

#if 0
    if (val < 0)
    {
        *p++ = '-';
        val = 0 - val;
    }
#endif

    u = (unsigned int)val;

    b = p;

    do
    {
        a = u % radix;
        u /= radix;

        *p++ = a + '0';

    } while (u > 0);

    len = (int)(p - buf);

    *p-- = 0;

    //swap
    do
    {
        temp = *p;
        *p = *b;
        *b = temp;
        --p;
        ++b;

    } while (b < p);

    return len;
}

uint8_t write_bytes(char *str, uint32_t bytes)
{
  uint8_t len;
  if (bytes < 1024*100) {
    len = itoa(bytes / 1024, str);
    str[len] = '.';
    str[len+1] = '0' + ((bytes % 1024) / 103);
    str[len+2] = 'K';
    str[len+3] = '\0';
    len += 3;
  }
  else if (bytes < 1024*1024) {
    len = itoa(bytes / 1024, str);
    str[len] = 'K';
    str[len+1] = '\0';
    len += 1;
  }
  else if (bytes < 1024*1024*100) {
    len = itoa(bytes / (1024*1024), str);
    str[len] = '.';
    str[len+1] = '0' + ((bytes % (1024*1024)) / 104858);
    str[len+2] = 'M';
    str[len+3] = '\0';
    len += 3;
  }
  else {
    len = itoa(bytes / (1024*1024), str);
    str[len] = 'M';
    str[len+1] = '\0';
    len += 1;
  }
  return len;
}

static void MSDCallbacks_Data( unsigned char flowDirection, unsigned int dataLength,
                               unsigned int fifoNullCount, unsigned int fifoFullCount )
{
  if (flowDirection)
    msdReadTotal += dataLength;
  else
    msdWriteTotal += dataLength;

  strcpy(statusLineMsg, "USB Rd:");
  uint8_t len = write_bytes(statusLineMsg+7, msdReadTotal);
  strcpy(statusLineMsg+7+len, " Wr:");
  write_bytes(statusLineMsg+7+len+4, msdWriteTotal);
  showStatusLine();
}

void usbMassStorage()
{
  static bool initialized = false;

  if (usbPlugged() && sd_card_ready()) {
    TRACE_DEBUG("usbMassStorage\n\r");

    if (sdMounted()) {
      Card_state = SD_ST_DATA;
      audioQueue.stopSD();
      logsClose();
      f_mount(nullptr, "", 0); // unmount SD
    }

    if (!initialized) {

      ConfigureUsbClock();

      /* Initialize LUN */
      MEDSdcard_Initialize(&(medias[DRV_SDMMC]), 0);

      LUN_Init(&(luns[DRV_SDMMC]), &(medias[DRV_SDMMC]),
          msdBuffer, MSD_BUFFER_SIZE,
          0, 0, 0, 0,
          MSDCallbacks_Data);

      /* BOT driver initialization */
      MSDDriver_Initialize(luns, 1);

      // VBus_Configure();
      USBD_Connect();

      initialized = true;
    }

    /* Mass storage state machine */
    for (uint8_t i=0; i<50; i++)
      MSDDriver_StateMachine();
  }
  else {
    msdReadTotal = 0;
    msdWriteTotal = 0;
  }
}

#endif
