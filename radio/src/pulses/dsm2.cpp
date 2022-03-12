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

#define DSM2_SEND_BIND                     (1 << 7)
#define DSM2_SEND_RANGECHECK               (1 << 5)


// DSM2 control bits
#define DSM2_CHANS           6
#define FRANCE_BIT           0x10
#define DSMX_BIT             0x08
#define BAD_DATA             0x47

#define BITLEN_DSM2          (8*2) //125000 Baud => 8uS per bit

static void reset_dsm2_buffer()
{
  extmodulePulsesData.dsm2.index = 0;
  extmodulePulsesData.dsm2.ptr = extmodulePulsesData.dsm2.pulses;
}

static void _send_1(uint8_t v)
{
  *extmodulePulsesData.dsm2.ptr++ = v - 1;
  extmodulePulsesData.dsm2.index += 1;
}

static void _sendByte(uint8_t b, uint8_t bit_len) // max 10 changes 0 10 10 10 10 1
{
  bool    lev = 0;
  uint8_t len = bit_len; // max val: 9*16 < 256
  for (uint8_t i=0; i<=8; i++) { // 8Bits + Stop=1
    bool nlev = b & 1; // lsb first
    if (lev == nlev) {
      len += bit_len;
    }
    else {
      _send_1(len);
      len  = bit_len;
      lev  = nlev;
    }
    b = (b>>1) | 0x80; // shift in stop bit
  }
  _send_1(len); // stop bit (len is already bit_len)
}

static void sendByteDsm2(uint8_t b)
{
  _sendByte(b, BITLEN_DSM2);
}

void putDsm2Flush()
{
  if (extmodulePulsesData.dsm2.index & 1)
    *extmodulePulsesData.dsm2.ptr++ = 255;
  else
    *(extmodulePulsesData.dsm2.ptr - 1) = 255;
}

// This is the data stream to send, prepare after 19.5 mS
// Send after 22.5 mS

void setupPulsesDSM2()
{
  uint8_t dsmDat[14];

  reset_dsm2_buffer();

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

  if (moduleState[EXTERNAL_MODULE].mode == MODULE_MODE_BIND) {
    dsmDat[0] |= DSM2_SEND_BIND;
  }
  else if (moduleState[EXTERNAL_MODULE].mode == MODULE_MODE_RANGECHECK) {
    dsmDat[0] |= DSM2_SEND_RANGECHECK;
  }

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

#define BITLEN_DSMP (17)

static void sendByteDSMP(uint8_t b)
{
  _sendByte(b, BITLEN_DSMP);
}

void setupPulsesLemonDSMP()
{
  static uint8_t pass = 0;

  reset_dsm2_buffer();

  const auto& md = g_model.moduleData[EXTERNAL_MODULE];
  // TODO: sanetize 'channels'
  auto channels = md.getChannelsCount();
  auto flags = md.dsmp.flags;

  // Force setup packet in Bind mode.
  auto module_mode = getModuleMode(EXTERNAL_MODULE);

  sendByteDSMP( 0xAA );
  sendByteDSMP( pass );

  // Setup packet
  if (pass == 0) {

    if (module_mode == MODULE_MODE_BIND) {
      flags = DSM2_SEND_BIND | (1 << 6 /* AUTO */);
      channels = 12;
    }
    sendByteDSMP( flags );

    uint8_t pwr = 7;
    if (module_mode == MODULE_MODE_RANGECHECK) {
      pwr = 4;
    }
    sendByteDSMP( pwr );    
    sendByteDSMP( channels );

    // Model number
    sendByteDSMP( 1 );

    // Send only 1 single Setup packet
    pass = 1;

  } else {

    uint8_t start_channel = 0;
    if (pass == 2) {
      start_channel += 7;
    }

    // Send channels
    for (int i=0; i<7; i++) {

      uint8_t channel = start_channel + i;
      if (channel < channels) {
        
        int value = channelOutputs[channel] + 2*PPM_CH_CENTER(channel) - 2*PPM_CENTER;
        uint16_t pulse;

        // Use 11-bit ?
        if (flags & (1 << 2)) {
          pulse = limit(0, ((value*349)>>9)+1024, 2047) | (channel << 11);
        } else {
          pulse = limit(0, ((value*13)>>5)+512, 1023) | (channel << 10);
        }

        sendByteDSMP( pulse >> 8 );
        sendByteDSMP( pulse & 0xFF );
      } else {
        // Outside of announced number of channels:
        // -> send invalid values
        sendByteDSMP( 0xFF );
        sendByteDSMP( 0xFF );
      }
    }
  }

  putDsm2Flush();

  if (++pass > 2) pass = 1;
  if (channels < 8) pass = 1;

  if (module_mode == MODULE_MODE_BIND) {
    // bind packet is setup
    pass = 0;
  }
  else if (--moduleState[EXTERNAL_MODULE].counter == 0) {
    // every 100th packet is setup
    pass = 0;
    moduleState[EXTERNAL_MODULE].counter = 100;
  }
}
