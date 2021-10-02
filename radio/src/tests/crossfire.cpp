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

#include "gtests.h"

#if defined(CROSSFIRE)
uint8_t createCrossfireChannelsFrame(uint8_t * frame, int16_t * pulses);
TEST(Crossfire, createCrossfireChannelsFrame)
{
  int16_t pulsesStart[MAX_TRAINER_CHANNELS];
  uint8_t crossfire[CROSSFIRE_FRAME_MAXLEN];

  memset(crossfire, 0, sizeof(crossfire));
  for (int i=0; i<MAX_TRAINER_CHANNELS; i++) {
    pulsesStart[i] = -1024 + (2048 / MAX_TRAINER_CHANNELS) * i;
  }

  createCrossfireChannelsFrame(crossfire, pulsesStart);

  // TODO check
}

TEST(Crossfire, crc8)
{
  uint8_t frame[] = { 0x00, 0x0C, 0x14, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x01, 0x03, 0x00, 0x00, 0x00, 0xF4 };
  uint8_t crc = crc8(&frame[2], frame[1]-1);
  ASSERT_EQ(frame[frame[1]+1], crc);
}
#endif

