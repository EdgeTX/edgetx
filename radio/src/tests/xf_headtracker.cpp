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

#if defined(XF_HEADTRACKER)

// Recorded from a real headtracker: CH4-CH6 at 1570.9, 1394.0 and 1961.3 us
static const uint8_t recordedFrame[XFHT_FRAME_LENGTH] = {
    0xE7, 0x9B, 0xA0, 0x8C, 0xA0, 0x8C, 0xA0, 0x8C, 0x45, 0x93,
    0xB0, 0x82, 0xDF, 0xB7, 0x80, 0xBB, 0x92, 0xB8, 0x8A, 0xA5,
};

static void feed(const uint8_t* data, size_t len)
{
  for (size_t i = 0; i < len; i++) xfhtParseByte(data[i]);
}

TEST(XfHeadtracker, recordedFrame)
{
  g_tmr10ms = 1000;
  feed(recordedFrame, sizeof(recordedFrame));
  EXPECT_EQ(get_xfht_value(0), 145);
  EXPECT_EQ(get_xfht_value(1), -217);
  EXPECT_EQ(get_xfht_value(2), 944);
  EXPECT_EQ(get_xfht_value(3), 0);
}

TEST(XfHeadtracker, badCrcIgnored)
{
  g_tmr10ms = 2000;
  feed(recordedFrame, sizeof(recordedFrame));

  uint8_t frame[XFHT_FRAME_LENGTH];
  memcpy(frame, recordedFrame, sizeof(frame));
  frame[8] = 0x00;  // roll changed, CRC left as it was
  g_tmr10ms = 2010;
  feed(frame, sizeof(frame));
  EXPECT_EQ(get_xfht_value(0), 145);
}

TEST(XfHeadtracker, resyncAfterGarbage)
{
  g_tmr10ms = 3000;
  feed(recordedFrame, sizeof(recordedFrame));
  g_tmr10ms = 3040;

  const uint8_t garbage[] = {0x12, 0xE7, 0xE7, 0x00, 0x9B, 0xE7};
  feed(garbage, sizeof(garbage));
  feed(recordedFrame, sizeof(recordedFrame));
  // past the first frame's timeout: only a frame decoded after the garbage
  // keeps the values
  g_tmr10ms = 3080;
  EXPECT_EQ(get_xfht_value(2), 944);
}

TEST(XfHeadtracker, falseHeaderInData)
{
  // CH7 at 1663.0 us is E7 9B on the wire: joining the stream there, the
  // header search alone would stay in step with it
  static const uint8_t frame[XFHT_FRAME_LENGTH] = {
      0xE7, 0x9B, 0xA0, 0x8C, 0xA0, 0x8C, 0xA0, 0x8C, 0x45, 0x93,
      0xB0, 0x82, 0xDF, 0xB7, 0xE7, 0x9B, 0x92, 0xB8, 0x04, 0x9C,
  };
  g_tmr10ms = 5000;
  feed(frame + 14, sizeof(frame) - 14);
  for (int i = 1; i <= 3; i++) {
    g_tmr10ms = 5000 + 4 * i;  // 25 Hz
    feed(frame, sizeof(frame));
  }
  EXPECT_EQ(get_xfht_value(0), 145);
  EXPECT_EQ(get_xfht_value(2), 944);
}

TEST(XfHeadtracker, lostLinkCentres)
{
  g_tmr10ms = 4000;
  feed(recordedFrame, sizeof(recordedFrame));
  g_tmr10ms = 4000 + XFHT_TIMEOUT_MS / 10;
  EXPECT_EQ(get_xfht_value(0), 145);
  g_tmr10ms = 4000 + XFHT_TIMEOUT_MS / 10 + 1;
  EXPECT_EQ(get_xfht_value(0), 0);
  EXPECT_EQ(get_xfht_value(1), 0);
  EXPECT_EQ(get_xfht_value(2), 0);
}

#endif
