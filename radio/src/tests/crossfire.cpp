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

#include "gtest/gtest.h"
#include "gtests.h"
#include "telemetry/telemetry.h"
#include "telemetry/crossfire.h"
#include "crc.h"

#if defined(CROSSFIRE)

uint8_t createCrossfireChannelsFrame(uint8_t moduleIdx, uint8_t * frame, int16_t * pulses, uint8_t nChannels);

// Channels 1-16 ramp from -1024, channels 17-32 are zero
static void initPulses(int16_t * pulses)
{
  memset(pulses, 0, sizeof(int16_t) * MAX_OUTPUT_CHANNELS);
  for (int i=0; i<CROSSFIRE_CHANNELS_COUNT; i++) {
    pulses[i] = -1024 + (2048 / CROSSFIRE_CHANNELS_COUNT) * i;
  }
}

// Spec-defined part of the frame (0x16 RC Channels Packed): sync byte, type,
// 16 x 11-bit channel packing. Expected bytes computed independently, not
// derived from createCrossfireChannelsFrame() itself.
TEST(Crossfire, createCrossfireChannelsFrame)
{
  MODEL_RESET();

  int16_t pulsesStart[MAX_OUTPUT_CHANNELS];
  uint8_t crossfire[CROSSFIRE_FRAME_MAXLEN];

  memset(crossfire, 0, sizeof(crossfire));
  initPulses(pulsesStart);

  createCrossfireChannelsFrame(EXTERNAL_MODULE, crossfire, pulsesStart, CROSSFIRE_CHANNELS_COUNT);

  ASSERT_EQ(crossfire[0], MODULE_ADDRESS);
  ASSERT_EQ(crossfire[2], CHANNELS_ID);

  const uint8_t expectedChannelData[22] = {
    0xAD, 0xA0, 0x88, 0x5E, 0xC0, 0x73, 0xA4, 0x56, 0x51, 0x4C, 0x6F,
    0xE0, 0x33, 0x22, 0x2B, 0x27, 0x9A, 0x57, 0xF0, 0x1A, 0x99, 0xD5
  };
  ASSERT_EQ(memcmp(&crossfire[3], expectedChannelData, sizeof(expectedChannelData)), 0);
}

// Status byte after the 0x16 payload is an ExpressLRS extension, not TBS CRSF
// spec (semantics per ExpressLRS's TXModuleEndpoint.cpp / crsf_protocol.h).
// Frame is always 25 bytes (1 ID + 22 channel data + 1 status + 1 CRC);
// bit 0 = commanded armed status (Switch mode only), bit 1 = arming mode is CH5.
TEST(Crossfire, ExpressLRSArmingExtension_CH5Mode)
{
  MODEL_RESET();

  int16_t pulsesStart[MAX_OUTPUT_CHANNELS];
  uint8_t crossfire[CROSSFIRE_FRAME_MAXLEN];

  memset(crossfire, 0, sizeof(crossfire));
  initPulses(pulsesStart);

  g_model.moduleData[EXTERNAL_MODULE].crsf.crsfArmingMode = ARMING_MODE_CH5;

  uint8_t len = createCrossfireChannelsFrame(EXTERNAL_MODULE, crossfire, pulsesStart, CROSSFIRE_CHANNELS_COUNT);

  ASSERT_EQ(len, 27);
  ASSERT_EQ(crossfire[0], MODULE_ADDRESS);
  ASSERT_EQ(crossfire[1], 25);
  ASSERT_EQ(crossfire[2], CHANNELS_ID);
  ASSERT_EQ(crossfire[25], 0x02); // bit 1: arming mode CH5

  uint8_t crc = crc8(&crossfire[2], 24);
  ASSERT_EQ(crossfire[26], crc);
}

TEST(Crossfire, ExpressLRSArmingExtension_SwitchMode)
{
  MODEL_RESET();

  int16_t pulsesStart[MAX_OUTPUT_CHANNELS];
  uint8_t crossfire[CROSSFIRE_FRAME_MAXLEN];

  memset(crossfire, 0, sizeof(crossfire));
  initPulses(pulsesStart);

  g_model.moduleData[EXTERNAL_MODULE].crsf.crsfArmingMode = ARMING_MODE_SWITCH;
  g_model.moduleData[EXTERNAL_MODULE].crsf.crsfArmingTrigger = SWSRC_NONE;

  uint8_t len = createCrossfireChannelsFrame(EXTERNAL_MODULE, crossfire, pulsesStart, CROSSFIRE_CHANNELS_COUNT);

  ASSERT_EQ(len, 27);
  ASSERT_EQ(crossfire[0], MODULE_ADDRESS);
  ASSERT_EQ(crossfire[1], 25);
  ASSERT_EQ(crossfire[2], CHANNELS_ID);
  ASSERT_EQ(crossfire[25], 0); // SWSRC_NONE -> not armed, bit 1 clear (Switch mode)

  uint8_t crc = crc8(&crossfire[2], 24);
  ASSERT_EQ(crossfire[26], crc);
}

// More than 16 channels: the 16-channel frame is followed by channels 17-32
// packed the same way, placed after the status byte.
// 1 ID + 22 channel data + 1 status + 22 channel data (17-32) + 1 CRC = 47
TEST(Crossfire, createCrossfireChannelsFrame_32Channels)
{
  MODEL_RESET();

  int16_t pulsesStart[MAX_OUTPUT_CHANNELS];
  uint8_t crossfire[CROSSFIRE_FRAME_MAXLEN];

  initPulses(pulsesStart);
  // channels 17-32 ramp down from +1024
  for (int i=0; i<CROSSFIRE_CHANNELS_COUNT; i++) {
    pulsesStart[CROSSFIRE_CHANNELS_COUNT + i] = 1024 - (2048 / CROSSFIRE_CHANNELS_COUNT) * i;
  }

  g_model.moduleData[EXTERNAL_MODULE].crsf.crsfArmingMode = ARMING_MODE_CH5;

  const uint8_t expectedChannelData[22] = {
    0xAD, 0xA0, 0x88, 0x5E, 0xC0, 0x73, 0xA4, 0x56, 0x51, 0x4C, 0x6F,
    0xE0, 0x33, 0x22, 0x2B, 0x27, 0x9A, 0x57, 0xF0, 0x1A, 0x99, 0xD5
  };
  const uint8_t expectedChannelDataHigh[22] = {
    0x13, 0x67, 0xB5, 0x91, 0xC1, 0x9B, 0xD7, 0x89, 0xB2, 0xD2, 0x88,
    0xE0, 0xD3, 0x1B, 0xC5, 0x5A, 0x75, 0x24, 0xF0, 0xE8, 0x85, 0x22
  };

  // any count above 16 selects the large frame
  for (int8_t nChannels : {CROSSFIRE_CHANNELS_COUNT + 1, MAX_OUTPUT_CHANNELS}) {
    memset(crossfire, 0, sizeof(crossfire));

    uint8_t len = createCrossfireChannelsFrame(EXTERNAL_MODULE, crossfire, pulsesStart, nChannels);

    ASSERT_EQ(len, 49) << "nChannels=" << (int)nChannels;
    ASSERT_EQ(crossfire[0], MODULE_ADDRESS);
    ASSERT_EQ(crossfire[1], 47);
    ASSERT_EQ(crossfire[2], CHANNELS_ID);
    ASSERT_EQ(memcmp(&crossfire[3], expectedChannelData, sizeof(expectedChannelData)), 0);
    ASSERT_EQ(crossfire[25], 0x02); // bit 1: arming mode CH5
    ASSERT_EQ(memcmp(&crossfire[26], expectedChannelDataHigh, sizeof(expectedChannelDataHigh)), 0);

    uint8_t crc = crc8(&crossfire[2], 46);
    ASSERT_EQ(crossfire[48], crc);
  }
}

// 16 or fewer channels keeps the standard 16-channel frame
TEST(Crossfire, createCrossfireChannelsFrame_16ChannelsOrFewer)
{
  MODEL_RESET();

  int16_t pulsesStart[MAX_OUTPUT_CHANNELS];
  uint8_t crossfire[CROSSFIRE_FRAME_MAXLEN];

  initPulses(pulsesStart);

  for (int8_t nChannels : {0, 8, CROSSFIRE_CHANNELS_COUNT}) {
    memset(crossfire, 0, sizeof(crossfire));

    uint8_t len = createCrossfireChannelsFrame(EXTERNAL_MODULE, crossfire, pulsesStart, nChannels);

    ASSERT_EQ(len, 27) << "nChannels=" << (int)nChannels;
    ASSERT_EQ(crossfire[1], 25);
    ASSERT_EQ(crossfire[26], crc8(&crossfire[2], 24));
  }
}

// The channel count comes from the module setup, not from a count of mixes
TEST(Crossfire, sentModuleChannelsFollowsModuleSetup)
{
  MODEL_RESET();

  g_model.moduleData[EXTERNAL_MODULE].type = MODULE_TYPE_CROSSFIRE;

  // a new module defaults to 16 channels, not to the 32 maximum
  g_model.moduleData[EXTERNAL_MODULE].channelsCount =
      defaultModuleChannels_M8(EXTERNAL_MODULE);
  EXPECT_EQ(sentModuleChannels(EXTERNAL_MODULE), CROSSFIRE_CHANNELS_COUNT);

  g_model.moduleData[EXTERNAL_MODULE].channelsCount =
      CROSSFIRE_MAX_CHANNELS_COUNT - 8;
  EXPECT_EQ(sentModuleChannels(EXTERNAL_MODULE), CROSSFIRE_MAX_CHANNELS_COUNT);

  // never drops below 16, whatever an older model holds
  g_model.moduleData[EXTERNAL_MODULE].channelsCount = 0;
  EXPECT_EQ(sentModuleChannels(EXTERNAL_MODULE), CROSSFIRE_CHANNELS_COUNT);

  // only 16 and 32 are selectable
  EXPECT_TRUE(isCrossfireChannelsCountAllowed(CROSSFIRE_CHANNELS_COUNT - 8));
  EXPECT_TRUE(isCrossfireChannelsCountAllowed(CROSSFIRE_MAX_CHANNELS_COUNT - 8));
  EXPECT_FALSE(isCrossfireChannelsCountAllowed(0));
  EXPECT_FALSE(isCrossfireChannelsCountAllowed(16));
}

TEST(Crossfire, crc8)
{
  uint8_t frame[] = { 0x00, 0x0C, 0x14, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x01, 0x03, 0x00, 0x00, 0x00, 0xF4 };
  uint8_t crc = crc8(&frame[2], frame[1]-1);
  ASSERT_EQ(frame[frame[1]+1], crc);
}

#if defined(HARDWARE_EXTERNAL_MODULE)
#include "pulses/crossfire.h"

struct crsf_frame_test {
  void* ctx = nullptr;

  uint8_t buffer[TELEMETRY_RX_PACKET_SIZE];
  uint8_t len = 0;

  crsf_frame_test()
  {
    ctx = CrossfireDriver.init(EXTERNAL_MODULE);
    if (!luaInputTelemetryFifo) {
      luaInputTelemetryFifo = new TelemetryQueue();
      assert(luaInputTelemetryFifo != nullptr);
    } else {
      luaInputTelemetryFifo->clear();
    }
  }

  template<unsigned Len>
  void process(uint8_t (&frame)[Len]) {
    CrossfireDriver.processFrame(ctx, frame, Len, buffer, &len);    
  }

  ~crsf_frame_test()
  {
    if (ctx != nullptr) {
      CrossfireDriver.deinit(ctx);
    }
  }
};

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
    //next trailing packet start
    0xEA, 0x0A, 0x0B
};

TEST(Crossfire, frameParser_incompleteFrames)
{
  crsf_frame_test ft;
  if (!ft.ctx) return;
  
  ft.process(incomplete_frame);
  EXPECT_EQ(ft.len, 7);
  EXPECT_EQ(ft.buffer[0], 0xEA);
  EXPECT_EQ(ft.buffer[1], 0x1F);

  ft.process(cont_frame);
  EXPECT_EQ(ft.len, 3);
  EXPECT_EQ(ft.buffer[0], 0xEA);
  EXPECT_EQ(ft.buffer[1], 0x0A);

  uint8_t* lua_buffer = luaInputTelemetryFifo->buffer();
  EXPECT_EQ(luaInputTelemetryFifo->size(), (size_t)(0x14 + 0x21 + 0x1F));

  unsigned offset = 0;
  EXPECT_EQ(lua_buffer[offset], 0x14);
  offset += 0x14;
  
  EXPECT_EQ(lua_buffer[offset], 0x21);
  offset += 0x21;

  EXPECT_EQ(lua_buffer[offset], 0x1F);
  EXPECT_EQ(lua_buffer[offset + 0x1F - 1], 0xFE);
}

static uint8_t length_error[] = {
    0x2A, 0xFE, 0x5F, 0x00,
};

static uint8_t length_error2[] = {
    // first frame
    0xEA, 0x09, 0xFF, 0x11, 0xFD, 0x05, 0x00, 0x00, 0x13, 0x01, 0x8C,
    // 2nd incomplete frame
    0xEA, 0xFE, 0x5F, 0x00,
};

static uint8_t length_error3[] = {
    // first frame
    0xEA, 0x09, 0xFF, 0x11, 0xFD, 0x05, 0x00, 0x00, 0x13, 0x01, 0x8C,
    // invalid: invalid frame start
    0x2A, 0x09, 0x5F, 0x00,
    // 2nd valid frame
    0xEA, 0x09, 0xFF, 0x11, 0xFD, 0x05, 0x00, 0x00, 0x13, 0x01, 0x8C,
};


TEST(Crossfire, frameParser_length)
{
  crsf_frame_test ft;
  if (!ft.ctx) return;

  uint8_t* lua_buffer = luaInputTelemetryFifo->buffer();

  // Check that a frame that is too big is rejected even if incomplete
  ft.process(length_error);
  EXPECT_EQ(ft.len, 0);

  // Check that a frame that is too big is rejected if positioned
  // after a complete frame
  ft.process(length_error2);
  EXPECT_EQ(ft.len, 0);

  // the first complete frame should have been processed
  EXPECT_EQ(luaInputTelemetryFifo->size(), (size_t)0x09);

  ft.process(length_error3);
  EXPECT_EQ(ft.len, 0);

  // only the first frame has been processed, as the rest
  // of the input buffer is thrown away due to length error
  EXPECT_EQ(luaInputTelemetryFifo->size(), (size_t)(0x09 + 0x09 + 0x09));

  // check all 3 frames
  unsigned offset = 0;
  for (int i = 0; i < 3; i++) {
    EXPECT_EQ(lua_buffer[offset], 0x09);
    EXPECT_EQ(lua_buffer[offset + 0x09 - 1], 0x01);
    offset += 0x09;
  }
}

static uint8_t invalid_frames[] = {
    // first frame
    0xEA, 0x14, 0xFF, 0x11, 0xFD, 0x05, 0x00, 0x00, 0x13, 0x01, 0x01,
    0x2A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x5D, 0x98, 0xB4,
    // random bytes
    0x00, 0x35, 0xA4,
    // second frame
    0xEA, 0x21, 0xFF, 0x1E, 0xFD, 0x12, 0x00, 0x00, 0x14, 0x01, 0x01,
    0x4A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x8F, 0xC2, 0x35, 0x3F, 0xFD, 0xC7, 0xD1, 0x3C, 0x4C, 0x01, 0x92,
    0x3F, 0x31,
    // invalid CRC frame
    0xEA, 0x02, 0x00, 0x01,
    // third frame
    0xEA, 0x1F, 0xFF, 0x1C, 0xFD, 0x10, 0x00,0x00, 0x15, 0x01, 0x01, 0x24, 0x00, 0x00, 0x35, 0x7D, 0xF2, 0x40,
    0xE8, 0x03, 0xE8, 0x03, 0xDC, 0x05, 0xDC, 0x05, 0xE1, 0x05, 0xE1,
    0x05, 0x2A, 0xFE, 0x5F,
};

TEST(Crossfire, frameParser_badFrames)
{
  //check if frameParser correctly skips bad frames (too long, bad CRC) and does't lose following packets 
  crsf_frame_test ft;
  if (!ft.ctx) return;

  ft.process(invalid_frames);
  EXPECT_EQ(ft.len,0);

  uint8_t* lua_buffer = luaInputTelemetryFifo->buffer();
  EXPECT_EQ(luaInputTelemetryFifo->size(), (size_t)(0x14 + 0x21 + 0x1F));

  unsigned offset = 0;
  EXPECT_EQ(lua_buffer[offset], 0x14);
  offset += 0x14;
  
  EXPECT_EQ(lua_buffer[offset], 0x21);
  offset += 0x21;

  EXPECT_EQ(lua_buffer[offset], 0x1F);
  EXPECT_EQ(lua_buffer[offset + 0x1F - 1], 0xFE);
}

static uint8_t jumboFrame1[]={
  0xEA, 0x3E, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFB
};
static uint8_t jumboFrame2[]={ //jumbo frame 2 
  0x8D, 0xEA, 0x3D, 0xFE, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x57
};

TEST(Crossfire, frameParser_multipleJumboFrames)
{
  crsf_frame_test ft;
  if (!ft.ctx) return;

  ft.process(jumboFrame1);
  EXPECT_EQ(ft.len, 63);
  EXPECT_EQ(ft.buffer[0], 0xEA);
  EXPECT_EQ(ft.buffer[1], 0x3E);

  ft.process(jumboFrame2);
  EXPECT_EQ(ft.len,0);

  uint8_t* lua_buffer = luaInputTelemetryFifo->buffer();
  EXPECT_EQ(luaInputTelemetryFifo->size(), (size_t)(62 + 61));

  unsigned offset = 0;
  EXPECT_EQ(lua_buffer[offset], 0x3E);
  EXPECT_EQ(lua_buffer[offset + 0x3E - 1], 0xFB);

  EXPECT_EQ(lua_buffer[offset], 0x3E);
  offset += 0x3E;
  
  EXPECT_EQ(lua_buffer[offset], 0x3D);
  EXPECT_EQ(lua_buffer[offset + 0x3D - 1], 0xF0);
}
#endif // HARDWARE_EXTERNAL_MODULE
#endif

