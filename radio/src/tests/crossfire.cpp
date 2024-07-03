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

#include "pulses/crossfire.h"

static uint8_t telemetry_rx_buffer[TELEMETRY_RX_PACKET_SIZE];

static uint8_t incomplete_frame[] = {
    // first frame
    0xEA, 0x14, 0xFF, 0x11, 0xFD, 0x05, 0x00, 0x00, 0x13, 0x01, 0x01,
    0x2A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x5D, 0x98, 0xB4,
    // second frame
    0xEA, 0x21, 0xFF, 0x1E, 0xFD, 0x12, 0x00, 0x00, 0x14, 0x01, 0x01,
    0x4A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x8F, 0xC2, 0x35, 0x3F, 0xFD, 0xC7, 0xD1, 0x3C, 0x4C, 0x01, 0x92,
    0x3F, 0x31,
    // incomplete third frame
    0xEA, 0x1F, 0xFF, 0x1C, 0xFD, 0x10, 0x00,
};

static uint8_t cont_frame[] = {
    0x00, 0x15, 0x01, 0x01, 0x24, 0x00, 0x00, 0x35, 0x7D, 0xF2, 0x40,
    0xE8, 0x03, 0xE8, 0x03, 0xDC, 0x05, 0xDC, 0x05, 0xE1, 0x05, 0xE1,
    0x05, 0x2A, 0xFE, 0x5F,
};

static void init_lua_fifo() {
  if (!luaInputTelemetryFifo) {
    luaInputTelemetryFifo = new Fifo<uint8_t, LUA_TELEMETRY_INPUT_FIFO_SIZE>();
    ASSERT_TRUE(luaInputTelemetryFifo != 0);
  } else {
    luaInputTelemetryFifo->clear();
  }
}

TEST(Crossfire, frameParser)
{
  void* ctx = CrossfireDriver.init(0);
  init_lua_fifo();

  uint8_t len = 0;
  CrossfireDriver.processFrame(ctx, incomplete_frame, sizeof(incomplete_frame),
                               telemetry_rx_buffer, &len);
  EXPECT_EQ(len, 7);
  EXPECT_EQ(telemetry_rx_buffer[0], 0xEA);
  EXPECT_EQ(telemetry_rx_buffer[1], 0x1F);

  CrossfireDriver.processFrame(ctx, cont_frame, sizeof(cont_frame),
                               telemetry_rx_buffer, &len);
  EXPECT_EQ(len, 0);

  uint8_t* lua_buffer = luaInputTelemetryFifo->buffer();
  EXPECT_EQ(luaInputTelemetryFifo->size(), 0x14 + 0x21 + 0x1F);

  unsigned offset = 0;
  EXPECT_EQ(lua_buffer[offset], 0x14);
  offset += 0x14;
  
  EXPECT_EQ(lua_buffer[offset], 0x21);
  offset += 0x21;

  EXPECT_EQ(lua_buffer[offset], 0x1F);
  EXPECT_EQ(lua_buffer[offset + 0x1F - 1], 0xFE);

  CrossfireDriver.deinit(ctx);
}

static uint8_t length_error[] = {
    0x2A, 0xFE, 0x5F, 0x00,
};

static uint8_t length_error2[] = {
    // first frame
    0xEA, 0x09, 0xFF, 0x11, 0xFD, 0x05, 0x00, 0x00, 0x13, 0x01, 0x8C,
    // 2nd incomplete frame
    0x2A, 0xFE, 0x5F, 0x00,
};

TEST(Crossfire, frameParser_fail)
{
  void* ctx = CrossfireDriver.init(0);
  init_lua_fifo();

  uint8_t len = 0;
  // Check that a frame that is too big is rejected even if incomplete
  CrossfireDriver.processFrame(ctx, length_error, sizeof(length_error),
                               telemetry_rx_buffer, &len);
  EXPECT_EQ(len, 0);

  // Check that a frame that is too big is rejected if positioned
  // after a complete frame
  CrossfireDriver.processFrame(ctx, length_error2, sizeof(length_error2),
                               telemetry_rx_buffer, &len);
  EXPECT_EQ(len, 0);

  CrossfireDriver.deinit(ctx);
}

#endif

