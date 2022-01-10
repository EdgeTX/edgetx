/*
 * Copyright (C) EdgeTX
 *
 * Based on code named
 *   opentx - https://github.com/opentx/opentx
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

#include "opentx.h"


#define BITLEN_SBUS          (10*2) // 100000 Baud => 10uS per bit


/* The protocol reuse some the DSM2 definitions where they are identical */


#if defined(PPM_PIN_SERIAL)
void sendByteSbus(uint8_t b)
{
  uint8_t parity = 1;

  putDsm2SerialBit(0);           // Start bit
  for (uint8_t i=0; i<8; i++) {  // 8 data Bits
    putDsm2SerialBit(b & 1);
    parity = parity ^ (b & 1);
    b >>= 1;
  }
  putDsm2SerialBit(!parity);     // Even Parity bit

  putDsm2SerialBit(1);           // Stop bit
  putDsm2SerialBit(1);           // Stop bit
}
#else
static void _send_level(uint8_t v)
{
  /* Copied over from DSM, this looks doubious and in my logic analyzer
     output the low->high is about 2 ns late */
  if (extmodulePulsesData.dsm2.index & 1)
    v += 2;
  else
    v -= 2;

  *extmodulePulsesData.dsm2.ptr++ = v - 1;
  extmodulePulsesData.dsm2.index+=1;
}

void sendByteSbus(uint8_t b) // max 11 changes 0 10 10 10 10 P 1
{
  bool    lev = 0;
  uint8_t parity = 1;

  uint8_t len = BITLEN_SBUS; //max val: 10*20 < 256
  for (uint8_t i=0; i<=9; i++) { //8Bits + 1Parity + Stop=1
    bool nlev = b & 1; //lsb first
    parity = parity ^ (uint8_t)nlev;
    if (lev == nlev) {
      len += BITLEN_SBUS;
    }
    else {
      _send_level(len);
      len  = BITLEN_SBUS;
      lev  = nlev;
    }
    b = (b>>1) | 0x80; //shift in ones for stop bit and parity
    if (i==7)
      b = b ^ parity; // lowest bit is one from previous line
  }
  _send_level(len+ BITLEN_SBUS); // enlarge the last bit to be two stop bits long
}
#endif


#define SBUS_NORMAL_CHANS           16
#define SBUS_CHAN_BITS       11


/* Definitions from CleanFlight/BetaFlight */

#define SBUS_FLAG_CHANNEL_17        (1 << 0)
#define SBUS_FLAG_CHANNEL_18        (1 << 1)
#define SBUS_FLAG_SIGNAL_LOSS       (1 << 2)
#define SBUS_FLAG_FAILSAFE_ACTIVE   (1 << 3)
#define SBUS_FRAME_BEGIN_BYTE       0x0F

#define SBUS_CHAN_CENTER            992

inline int getChannelValue(uint8_t port, int channel)
{
  int ch = g_model.moduleData[port].channelsStart+channel;
  // We will ignore 17 and 18th if that brings us over the limit
  if (ch > 31)
    return 0;
  return channelOutputs[ch] + 2 * PPM_CH_CENTER(ch) - 2*PPM_CENTER;
}

static void sbusFlush()
{
  if (extmodulePulsesData.dsm2.index & 1)
    *extmodulePulsesData.dsm2.ptr++ = 60000;
  else
    *(extmodulePulsesData.dsm2.ptr - 1) = 60000;
}

void setupPulsesSbus()
{
#if defined(PPM_PIN_SERIAL)
  extmodulePulsesData.dsm2.serialByte = 0;
  extmodulePulsesData.dsm2.serialBitCount = 0;
#else
  extmodulePulsesData.dsm2.index = 0;
#endif

  extmodulePulsesData.dsm2.ptr = extmodulePulsesData.dsm2.pulses;

  // Sync Byte
  sendByteSbus(SBUS_FRAME_BEGIN_BYTE);

  uint32_t bits = 0;
  uint8_t bitsavailable = 0;

  // byte 1-22, channels 0..2047, limits not really clear (B
  for (int i=0; i<SBUS_NORMAL_CHANS; i++) {
    int value = getChannelValue(EXTERNAL_MODULE, i);

    value =  value*8/10 + SBUS_CHAN_CENTER;
    bits |= limit(0, value, 2047) << bitsavailable;
    bitsavailable += SBUS_CHAN_BITS;
    while (bitsavailable >= 8) {
      sendByteSbus((uint8_t) (bits & 0xff));
      bits >>= 8;
      bitsavailable -= 8;
    }
  }

  // flags
  uint8_t flags=0;
  if (getChannelValue(EXTERNAL_MODULE, 16) > 0)
    flags |= SBUS_FLAG_CHANNEL_17;
  if (getChannelValue(EXTERNAL_MODULE, 17) > 0)
    flags |= SBUS_FLAG_CHANNEL_18;

  sendByteSbus(flags);

  // last byte, always 0x0
  sendByteSbus(0x00);

  sbusFlush();
}
