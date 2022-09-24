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
#include "telemetry/ghost.h"
#include "telemetry/ghost_menu.h"

uint8_t createGhostMenuControlFrame(uint8_t * frame, int16_t * pulses)
{
  uint8_t * buf = frame;

  *buf++ = getGhostModuleAddr();            // addr
  *buf++ = GHST_UL_RC_CHANS_SIZE;           // length
  const uint8_t * crc_start = buf;
  *buf++ = GHST_UL_MENU_CTRL;               // type

  // payload
  *buf++ = reusableBuffer.ghostMenu.buttonAction; // Joystick states, Up, Down, Left, Right, Press
  *buf++ = reusableBuffer.ghostMenu.menuAction;   // menu control, open, close, etc.
  for (uint8_t i = 0; i < 8; i++) {
    *buf++ = 0;   // padding to make this the same size as the pulses packet
  }

  // crc
  *buf++ = crc8(crc_start, GHST_UL_RC_CHANS_SIZE - 1);

  return buf - frame;
}

// Range for pulses (channels output) is [-1024:+1024]
uint8_t createGhostChannelsFrame(uint8_t * frame, int16_t * pulses, bool raw12bits)
{
  static uint8_t lastGhostFrameId = 0;
  uint8_t ghostUpper4Offset = 0;

  switch (lastGhostFrameId) {
    case GHST_UL_RC_CHANS_HS4_5TO8:
    case GHST_UL_RC_CHANS_HS4_12_5TO8:
      lastGhostFrameId = raw12bits ? GHST_UL_RC_CHANS_HS4_12_9TO12 : GHST_UL_RC_CHANS_HS4_9TO12;
      ghostUpper4Offset = 4;
    break;

    case GHST_UL_RC_CHANS_HS4_9TO12:
    case GHST_UL_RC_CHANS_HS4_12_9TO12:
      lastGhostFrameId = raw12bits ? GHST_UL_RC_CHANS_HS4_12_13TO16 : GHST_UL_RC_CHANS_HS4_13TO16;
      ghostUpper4Offset = 8;
      break;

    case GHST_UL_RC_CHANS_HS4_13TO16:
    case GHST_UL_RC_CHANS_HS4_12_13TO16:
      lastGhostFrameId = raw12bits ? GHST_UL_RC_CHANS_HS4_12_5TO8 : GHST_UL_RC_CHANS_HS4_5TO8;
      ghostUpper4Offset = 0;
      break;

    default:  // We don't have known previous state so init
      lastGhostFrameId = raw12bits ? GHST_UL_RC_CHANS_HS4_12_5TO8 : GHST_UL_RC_CHANS_HS4_5TO8;
      ghostUpper4Offset = 0;
    break;
  }

  uint8_t * buf = frame;
  *buf++ = getGhostModuleAddr();            // addr
  *buf++ = GHST_UL_RC_CHANS_SIZE;           // len
  uint8_t * crc_start = buf;
  *buf++ = lastGhostFrameId;                // type

  // payload
  // first 4 high speed, 12 bit channels (11 relevant bits with openTx)
  uint32_t bits = 0;
  uint8_t bitsavailable = 0;
  for (int i = 0; i < 4; i++) {
    uint32_t value;
    if(raw12bits) {
      value = limit(0, (1024 + (pulses[i] + 2 * PPM_CH_CENTER(i) - 2 * PPM_CENTER)) << 1, 0xFFF);
    } else {
      value = limit(0, GHST_RC_CTR_VAL_12BIT + (((pulses[i] + 2 * PPM_CH_CENTER(i) - 2 * PPM_CENTER) << 3) / 5), 2 * GHST_RC_CTR_VAL_12BIT);
    }
    bits |= value << bitsavailable;
    bitsavailable += GHST_CH_BITS_12;
    while (bitsavailable >= 8) {
      *buf++ = bits;
      bits >>= 8;
      bitsavailable -= 8;
    }
  }

  // second 4 lower speed, 8 bit channels
  for (int i = 4; i < 8; ++i) {
    uint8_t channelIndex = i + ghostUpper4Offset;
    uint8_t value;
    if(raw12bits) {
      value = limit(0, 128 + ((pulses[channelIndex] + 2 * PPM_CH_CENTER(channelIndex) - 2 * PPM_CENTER) >> 3), 0xFF);
    } else {
      value = limit(0, GHST_RC_CTR_VAL_8BIT + (((pulses[channelIndex] + 2 * PPM_CH_CENTER(channelIndex) - 2 * PPM_CENTER) >> 1) / 5), 2 * GHST_RC_CTR_VAL_8BIT);
    }
    *buf++ = value;
  }

  // crc
  *buf++ = crc8(crc_start, GHST_UL_RC_CHANS_SIZE - 1);

  return buf - frame;
}

void setupPulsesGhost()
{
  if (telemetryProtocol == PROTOCOL_TELEMETRY_GHOST) {

    auto &module = g_model.moduleData[EXTERNAL_MODULE];
    auto *p_data = &extmodulePulsesData.ghost;

    #if defined(LUA)
    if (outputTelemetryBuffer.destination == TELEMETRY_ENDPOINT_SPORT) {
      memcpy(p_data->pulses, outputTelemetryBuffer.data, outputTelemetryBuffer.size);
      p_data->length = outputTelemetryBuffer.size;
      outputTelemetryBuffer.reset();
    } else
    #endif
    if (moduleState[EXTERNAL_MODULE].counter == GHST_MENU_CONTROL) {
        p_data->length = createGhostMenuControlFrame(p_data->pulses, &channelOutputs[module.channelsStart]);
    } else {
        p_data->length = createGhostChannelsFrame(p_data->pulses, &channelOutputs[module.channelsStart], module.ghost.raw12bits);
    }
    moduleState[EXTERNAL_MODULE].counter = GHST_FRAME_CHANNEL;
  }
}
