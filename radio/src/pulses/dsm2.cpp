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

#define DSM2_SEND_BIND                     (1 << 7)
#define DSM2_SEND_RANGECHECK               (1 << 5)

#if defined(PCBSKY9X)
uint8_t  dsm2BindTimer = DSM2_BIND_TIMEOUT;
#endif

// DSM2 control bits
#define DSM2_CHANS           6
#define FRANCE_BIT           0x10
#define DSMX_BIT             0x08
#define BAD_DATA             0x47

#define BITLEN_DSM2          (8*2) //125000 Baud => 8uS per bit

#if defined(PPM_PIN_SERIAL)
void putDsm2SerialBit(uint8_t bit)
{
  extmodulePulsesData.dsm2.serialByte >>= 1;
  if (bit & 1) {
    extmodulePulsesData.dsm2.serialByte |= 0x80;
  }
  if (++extmodulePulsesData.dsm2.serialBitCount >= 8) {
    *extmodulePulsesData.dsm2.ptr++ = extmodulePulsesData.dsm2.serialByte;
    extmodulePulsesData.dsm2.serialBitCount = 0;
  }
}

void sendByteDsm2(uint8_t b)     // max 10changes 0 10 10 10 10 1
{
  putDsm2SerialBit(0);           // Start bit
  for (uint8_t i=0; i<8; i++) {  // 8 data Bits
    putDsm2SerialBit(b & 1);
    b >>= 1;
  }

  putDsm2SerialBit(1);           // Stop bit
  putDsm2SerialBit(1);           // Stop bit
}

void putDsm2Flush()
{
  for (int i=0; i<16; i++) {
    putDsm2SerialBit(1);         // 16 extra stop bits
  }
}
#else
void _send_1(uint8_t v)
{
  if (extmodulePulsesData.dsm2.index & 1)
    v += 2;
  else
    v -= 2;

  *extmodulePulsesData.dsm2.ptr++ = v - 1;
  extmodulePulsesData.dsm2.index += 1;
}

void sendByteDsm2(uint8_t b) // max 10 changes 0 10 10 10 10 1
{
  bool    lev = 0;
  uint8_t len = BITLEN_DSM2; // max val: 9*16 < 256
  for (uint8_t i=0; i<=8; i++) { // 8Bits + Stop=1
    bool nlev = b & 1; // lsb first
    if (lev == nlev) {
      len += BITLEN_DSM2;
    }
    else {
      _send_1(len);
      len  = BITLEN_DSM2;
      lev  = nlev;
    }
    b = (b>>1) | 0x80; // shift in stop bit
  }
  _send_1(len); // stop bit (len is already BITLEN_DSM2)
}

void putDsm2Flush()
{
  if (extmodulePulsesData.dsm2.index & 1)
    *extmodulePulsesData.dsm2.ptr++ = 60000;
  else
    *(extmodulePulsesData.dsm2.ptr - 1) = 60000;
}
#endif

// This is the data stream to send, prepare after 19.5 mS
// Send after 22.5 mS

void setupPulsesDSM2()
{
  uint8_t dsmDat[14];

#if defined(PPM_PIN_SERIAL)
  extmodulePulsesData.dsm2.serialByte = 0 ;
  extmodulePulsesData.dsm2.serialBitCount = 0 ;
#else
  extmodulePulsesData.dsm2.index = 0;
#endif

  extmodulePulsesData.dsm2.ptr = extmodulePulsesData.dsm2.pulses;

  switch (moduleState[EXTERNAL_MODULE].protocol) {
    case PROTOCOL_CHANNELS_DSM2_LP45:
      dsmDat[0] = 0x00;
      break;
    case PROTOCOL_CHANNELS_DSM2_DSM2:
      dsmDat[0] = 0x10;
      break;
    default: // DSMX
      dsmDat[0] = 0x10 | DSMX_BIT;
      break;
  }

#if defined(PCBSKY9X)
  if (dsm2BindTimer > 0) {
    dsm2BindTimer--;
    if (switchState(SW_DSM2_BIND)) {
      moduleState[EXTERNAL_MODULE].mode = MODULE_MODE_BIND;
      dsmDat[0] |= DSM2_SEND_BIND;
    }
  }
  else if (moduleState[EXTERNAL_MODULE].mode == MODULE_MODE_RANGECHECK) {
    dsmDat[0] |= DSM2_SEND_RANGECHECK;
  }
  else {
    moduleState[EXTERNAL_MODULE].mode = 0;
  }
#else
  if (moduleState[EXTERNAL_MODULE].mode == MODULE_MODE_BIND) {
    dsmDat[0] |= DSM2_SEND_BIND;
  }
  else if (moduleState[EXTERNAL_MODULE].mode == MODULE_MODE_RANGECHECK) {
    dsmDat[0] |= DSM2_SEND_RANGECHECK;
  }
#endif

  dsmDat[1] = g_model.header.modelId[EXTERNAL_MODULE]; // DSM2 Header second byte for model match

  for (int i=0; i<DSM2_CHANS; i++) {
    int channel = g_model.moduleData[EXTERNAL_MODULE].channelsStart+i;
    int value = channelOutputs[channel] + 2*PPM_CH_CENTER(channel) - 2*PPM_CENTER;
    uint16_t pulse = limit(0, ((value*13)>>5)+512, 1023);
    dsmDat[2+2*i] = (i<<2) | ((pulse>>8)&0x03);
    dsmDat[3+2*i] = pulse & 0xff;
  }

  for (int i=0; i<14; i++) {
    sendByteDsm2(dsmDat[i]);
  }

  putDsm2Flush();
}
