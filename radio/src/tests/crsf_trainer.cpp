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

#if defined(CROSSFIRE)

#include <vector>

#include "crc.h"
#include "crsf_trainer.h"
#include "gtests.h"
#include "gui/gui_common.h"
#include "serial.h"
#include "telemetry/crossfire.h"
#include "trainer.h"

// Tests for the CRSF trainer frame assembler used on USB-VCP. CDC delivers
// arbitrarily chunked buffers with no idle line, so frames have to be
// recovered from the byte stream itself.

namespace {

constexpr uint8_t CRSF_CH_BITS = 11;
constexpr uint16_t CRSF_CH_CENTER = 0x3E0;

// Distinct value per channel, so channel ordering is checked too
uint16_t testChannelValue(int ch) { return 200 + ch * 100; }

int16_t expectedPulse(uint16_t raw)
{
  return ((int32_t)raw - CRSF_CH_CENTER) * 5 / 8;
}

// Build an RC channels packed frame: [addr][len][type][22 bytes][crc]
std::vector<uint8_t> buildChannelsFrame(uint8_t addr = UART_SYNC)
{
  std::vector<uint8_t> payload;
  uint32_t bits = 0;
  uint8_t bitsAvailable = 0;
  for (int ch = 0; ch < 16; ch++) {
    bits |= (uint32_t)(testChannelValue(ch) & 0x7FF) << bitsAvailable;
    bitsAvailable += CRSF_CH_BITS;
    while (bitsAvailable >= 8) {
      payload.push_back(bits & 0xFF);
      bits >>= 8;
      bitsAvailable -= 8;
    }
  }

  std::vector<uint8_t> frame;
  frame.push_back(addr);
  frame.push_back((uint8_t)(payload.size() + 2));  // type + payload + crc
  frame.push_back(CHANNELS_ID);
  frame.insert(frame.end(), payload.begin(), payload.end());
  frame.push_back(crc8(frame.data() + 2, frame.size() - 2));
  return frame;
}

// A well-formed frame of some other type, which must be consumed but ignored
std::vector<uint8_t> buildOtherFrame()
{
  std::vector<uint8_t> frame = {UART_SYNC, 4, LINK_ID, 0x11, 0x22};
  frame.push_back(crc8(frame.data() + 2, frame.size() - 2));
  return frame;
}

void append(std::vector<uint8_t>& dst, const std::vector<uint8_t>& src)
{
  dst.insert(dst.end(), src.begin(), src.end());
}

void feed(std::vector<uint8_t> stream, size_t chunkSize = 0)
{
  if (chunkSize == 0) chunkSize = stream.size();
  size_t offset = 0;
  while (offset < stream.size()) {
    size_t n = std::min(chunkSize, stream.size() - offset);
    crsfTrainerReceiveData(stream.data() + offset, n);
    offset += n;
  }
}

void expectTestChannels()
{
  for (int ch = 0; ch < 16; ch++) {
    EXPECT_EQ(trainerInput[ch], expectedPulse(testChannelValue(ch)))
        << "channel " << ch;
  }
}

class CrsfTrainerTest : public ::testing::Test
{
 protected:
  void SetUp() override
  {
    crsfTrainerStop();  // clears any partial frame from a previous test
    memset(&g_model, 0, sizeof(g_model));
    g_model.trainerData.mode = TRAINER_MODE_CRSF;
    memset(trainerInput, 0, sizeof(trainerInput));
    trainerSetTimer(0);
  }

  void TearDown() override { g_model.trainerData.mode = TRAINER_MODE_OFF; }
};

}  // namespace

TEST_F(CrsfTrainerTest, singleFrame)
{
  feed(buildChannelsFrame());

  expectTestChannels();
  EXPECT_TRUE(isTrainerValid());
}

// Frames get split across USB packets at arbitrary points.
TEST_F(CrsfTrainerTest, splitAcrossChunkBoundaries)
{
  auto frame = buildChannelsFrame();
  for (size_t chunkSize = 1; chunkSize <= frame.size(); chunkSize++) {
    crsfTrainerStop();
    memset(trainerInput, 0, sizeof(trainerInput));

    feed(frame, chunkSize);

    for (int ch = 0; ch < 16; ch++) {
      EXPECT_EQ(trainerInput[ch], expectedPulse(testChannelValue(ch)))
          << "chunk size " << chunkSize << ", channel " << ch;
    }
  }
}

TEST_F(CrsfTrainerTest, backToBackFramesInOneChunk)
{
  std::vector<uint8_t> stream;
  append(stream, buildChannelsFrame());
  append(stream, buildChannelsFrame());
  append(stream, buildChannelsFrame());

  feed(stream);

  expectTestChannels();
  EXPECT_TRUE(isTrainerValid());
}

TEST_F(CrsfTrainerTest, garbagePrefixIsSkipped)
{
  std::vector<uint8_t> stream = {0x11, 0x22, 0x33, 0x44};
  append(stream, buildChannelsFrame());

  feed(stream);

  expectTestChannels();
  EXPECT_TRUE(isTrainerValid());
}

// The whole point of CRSF over SBUS: a corrupted frame is detected, not
// silently applied to the trainer channels.
TEST_F(CrsfTrainerTest, badCrcIsRejected)
{
  auto frame = buildChannelsFrame();
  frame[frame.size() - 1] ^= 0xFF;  // corrupt the CRC

  feed(frame);

  EXPECT_FALSE(isTrainerValid()) << "frame with a bad CRC was accepted";
  for (int ch = 0; ch < 16; ch++) EXPECT_EQ(trainerInput[ch], 0);
}

// Corruption of the payload is caught by the CRC as well.
TEST_F(CrsfTrainerTest, corruptedPayloadIsRejected)
{
  auto frame = buildChannelsFrame();
  frame[5] ^= 0xFF;

  feed(frame);

  EXPECT_FALSE(isTrainerValid());
}

// A corrupted frame must be dropped whole, leaving the stream aligned so the
// very next frame is decoded.
//
// Note the test channel values put 0xC8 (UART_SYNC) inside the payload -- ch0
// is 200 -- so a byte-wise re-sync here would latch onto that payload byte,
// read a bogus length from the next one, and swallow the following good frame
// while waiting for it to complete.
TEST_F(CrsfTrainerTest, corruptedFrameIsDroppedAndStreamStaysAligned)
{
  auto bad = buildChannelsFrame();
  bad[bad.size() - 1] ^= 0xFF;

  feed(bad);
  ASSERT_FALSE(isTrainerValid());

  feed(buildChannelsFrame());
  expectTestChannels();
  EXPECT_TRUE(isTrainerValid());
}

// Truncated frame followed by good ones, with awkward chunking on top.
TEST_F(CrsfTrainerTest, resyncAfterTruncatedFrame)
{
  auto frame = buildChannelsFrame();

  std::vector<uint8_t> stream;
  append(stream, frame);
  stream.insert(stream.end(), frame.begin(), frame.begin() + 12);
  for (uint8_t b : {0x55, 0xAA, 0x37}) stream.push_back(b);
  append(stream, frame);
  append(stream, frame);

  feed(stream, 7);

  expectTestChannels();
  EXPECT_TRUE(isTrainerValid());
}

// A bogus length byte must not wedge the assembler.
TEST_F(CrsfTrainerTest, insaneLengthIsRejected)
{
  std::vector<uint8_t> stream = {UART_SYNC, 0x00, 0x00};
  append(stream, buildChannelsFrame());

  feed(stream);

  expectTestChannels();
  EXPECT_TRUE(isTrainerValid());
}

// Non-channel frames share the stream; they must be consumed without
// disturbing alignment, and without touching the trainer channels.
TEST_F(CrsfTrainerTest, otherFrameTypesAreIgnoredButConsumed)
{
  feed(buildOtherFrame());
  EXPECT_FALSE(isTrainerValid());

  std::vector<uint8_t> stream;
  append(stream, buildOtherFrame());
  append(stream, buildChannelsFrame());
  append(stream, buildOtherFrame());

  feed(stream);

  expectTestChannels();
  EXPECT_TRUE(isTrainerValid());
}

// Both address bytes are accepted as a frame start.
TEST_F(CrsfTrainerTest, acceptsBothAddressBytes)
{
  feed(buildChannelsFrame(RADIO_ADDRESS));
  EXPECT_TRUE(isTrainerValid());
  expectTestChannels();
}

// Nothing may reach the trainer channels unless the model asked for CRSF
// trainer input.
TEST_F(CrsfTrainerTest, ignoredWhenTrainerModeIsNotCrsf)
{
  g_model.trainerData.mode = TRAINER_MODE_OFF;

  feed(buildChannelsFrame());

  EXPECT_FALSE(isTrainerValid());
  for (int ch = 0; ch < 16; ch++) EXPECT_EQ(trainerInput[ch], 0) << "channel " << ch;
}

// Link loss: frames stop, trainer input goes stale, sticks take over.
TEST_F(CrsfTrainerTest, inputGoesStaleWhenFramesStop)
{
  feed(buildChannelsFrame());
  ASSERT_TRUE(isTrainerValid());

  for (int i = 0; i < 100; i++) {
    EXPECT_TRUE(isTrainerValid()) << "went stale early, at tick " << i;
    trainerDecTimer();
  }

  EXPECT_FALSE(isTrainerValid());
}

// A partial frame left over when the port is released must not be completed
// by whatever the next user of the port sends.
TEST_F(CrsfTrainerTest, partialFrameIsDroppedOnStop)
{
  auto frame = buildChannelsFrame();
  std::vector<uint8_t> partial(frame.begin(), frame.begin() + 20);
  feed(partial);

  crsfTrainerStop();

  std::vector<uint8_t> rest(frame.begin() + 20, frame.end());
  feed(rest);

  EXPECT_FALSE(isTrainerValid());
}

#if defined(USB_SERIAL)
// The option has to be offered on USB-VCP, and only there: the mode is driven
// from a receive callback, which the STM32 USART driver does not provide.
TEST(CrsfTrainerMenu, crsfTrainerIsOfferedOnVcpOnly)
{
  EXPECT_TRUE(isSerialModeAvailable(SP_VCP, UART_MODE_CRSF_TRAINER));
  EXPECT_FALSE(isSerialModeAvailable(SP_AUX1, UART_MODE_CRSF_TRAINER));
}

// Selecting the serial mode has to make TRAINER_MODE_CRSF selectable too.
// Without this, the port can be configured but the trainer mode that consumes
// it stays hidden unless an ELRS module happens to be enabled -- which is not
// the case when the frames come from a PC over USB.
TEST(CrsfTrainerMenu, crsfTrainerModeAvailableWhenPortIsConfigured)
{
  auto restore = g_eeGeneral.serialPort;

  serialSetMode(SP_VCP, UART_MODE_NONE);
  bool withoutPort = isTrainerModeAvailable(TRAINER_MODE_CRSF);

  serialSetMode(SP_VCP, UART_MODE_CRSF_TRAINER);
  EXPECT_TRUE(isTrainerModeAvailable(TRAINER_MODE_CRSF))
      << "CRSF trainer mode hidden even though USB-VCP is set to CRSF Trainer";

  // Sanity: the port is what made the difference here, not something else
  EXPECT_FALSE(withoutPort);

  g_eeGeneral.serialPort = restore;
}
#endif

#endif  // CROSSFIRE
