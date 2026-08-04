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

#include <vector>

#include "gtests.h"
#include "gui/gui_common.h"
#include "sbus.h"
#include "serial.h"
#include "trainer.h"

// Tests for the SBUS byte-stream framer used by ports without idle-line
// detection (USB-VCP). The framer has to recover 25 byte frames from a stream
// chunked at arbitrary boundaries.

namespace {

constexpr uint8_t SBUS_FRAME_SIZE = 25;
constexpr uint8_t SBUS_START = 0x0F;
constexpr uint16_t SBUS_CENTER = 992;

// Distinct value per channel, so channel ordering is checked and not just
// channel content. Kept inside the 11 bit range.
uint16_t testChannelValue(int ch) { return 200 + ch * 100; }

// Expected trainer value for a raw SBUS channel value, per sbusProcessFrame()
int16_t expectedPulse(uint16_t raw) { return ((int32_t)raw - SBUS_CENTER) * 5 / 8; }

std::vector<uint8_t> buildFrame(uint8_t flags = 0x00, uint8_t endByte = 0x00)
{
  std::vector<uint8_t> frame;
  frame.push_back(SBUS_START);

  uint32_t bits = 0;
  uint8_t bitsAvailable = 0;
  for (int ch = 0; ch < 16; ch++) {
    bits |= (uint32_t)(testChannelValue(ch) & 0x7FF) << bitsAvailable;
    bitsAvailable += 11;
    while (bitsAvailable >= 8) {
      frame.push_back(bits & 0xFF);
      bits >>= 8;
      bitsAvailable -= 8;
    }
  }

  frame.push_back(flags);
  frame.push_back(endByte);
  return frame;
}

void append(std::vector<uint8_t>& dst, const std::vector<uint8_t>& src)
{
  dst.insert(dst.end(), src.begin(), src.end());
}

// Feed a stream to the framer, split into fixed size chunks. chunkSize == 0
// means "one single chunk", which is what a small USB packet looks like.
void feed(std::vector<uint8_t> stream, size_t chunkSize = 0)
{
  if (chunkSize == 0) chunkSize = stream.size();

  size_t offset = 0;
  while (offset < stream.size()) {
    size_t n = std::min(chunkSize, stream.size() - offset);
    sbusStreamReceiveData(stream.data() + offset, n);
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

class SbusStreamTest : public ::testing::Test
{
 protected:
  void SetUp() override
  {
    sbusStreamStop();  // clears any partial frame left by a previous test
    sbusAuxSetEnabled(true);
    memset(trainerInput, 0, sizeof(trainerInput));
    trainerSetTimer(0);
  }

  void TearDown() override { sbusAuxSetEnabled(false); }
};

}  // namespace

TEST_F(SbusStreamTest, singleFrame)
{
  feed(buildFrame());

  expectTestChannels();
  EXPECT_TRUE(isTrainerValid());
}

// A 25 byte frame is routinely split across USB packets. Every possible split
// point has to work, including one byte at a time.
TEST_F(SbusStreamTest, splitAcrossChunkBoundaries)
{
  for (size_t chunkSize = 1; chunkSize <= SBUS_FRAME_SIZE; chunkSize++) {
    sbusStreamStop();
    memset(trainerInput, 0, sizeof(trainerInput));

    feed(buildFrame(), chunkSize);

    for (int ch = 0; ch < 16; ch++) {
      EXPECT_EQ(trainerInput[ch], expectedPulse(testChannelValue(ch)))
          << "chunk size " << chunkSize << ", channel " << ch;
    }
  }
}

// Several frames may arrive in a single USB packet.
TEST_F(SbusStreamTest, backToBackFramesInOneChunk)
{
  std::vector<uint8_t> stream;
  append(stream, buildFrame());
  append(stream, buildFrame());
  append(stream, buildFrame());

  feed(stream);

  expectTestChannels();
  EXPECT_TRUE(isTrainerValid());
}

// Bytes already in flight when the framer attaches are not a frame start.
TEST_F(SbusStreamTest, garbagePrefixIsSkipped)
{
  std::vector<uint8_t> stream = {0x11, 0x22, 0x33, 0xFF, 0xAB};
  append(stream, buildFrame());

  feed(stream);

  expectTestChannels();
  EXPECT_TRUE(isTrainerValid());
}

// A frame whose end byte is wrong must be rejected, and the framer must
// re-sync rather than discard everything it holds.
TEST_F(SbusStreamTest, invalidEndByteIsRejectedThenResyncs)
{
  auto bad = buildFrame(0x00, 0xFF);

  feed(bad);
  EXPECT_FALSE(isTrainerValid()) << "frame with a bad end byte was accepted";

  feed(buildFrame());
  expectTestChannels();
  EXPECT_TRUE(isTrainerValid());
}

// Garbage injected mid-stream (the --garbage case of the PC-side test tool):
// a truncated frame followed by good ones must recover.
TEST_F(SbusStreamTest, resyncAfterTruncatedFrame)
{
  auto frame = buildFrame();

  std::vector<uint8_t> stream;
  append(stream, frame);
  // half a frame, then noise, then two clean frames
  stream.insert(stream.end(), frame.begin(), frame.begin() + 12);
  for (uint8_t b : {0x55, 0xAA, 0x37, 0x91}) stream.push_back(b);
  append(stream, frame);
  append(stream, frame);

  feed(stream, 7);  // awkward chunking on top

  expectTestChannels();
  EXPECT_TRUE(isTrainerValid());
}

// The re-sync path has to find a start byte held *inside* the buffer, not just
// drop the buffer: here a stray 0x0F precedes the real frame closely enough
// that the real frame is already partly buffered when the false frame fails.
TEST_F(SbusStreamTest, resyncFindsStartByteInsideBuffer)
{
  std::vector<uint8_t> stream = {SBUS_START, 0x01, 0x02};
  append(stream, buildFrame());

  feed(stream);

  expectTestChannels();
  EXPECT_TRUE(isTrainerValid());
}

// Some implementations carry frame flags in the last byte.
TEST_F(SbusStreamTest, acceptsEndByteVariants)
{
  for (uint8_t endByte : {0x00, 0x04, 0x14, 0x24}) {
    sbusStreamStop();
    memset(trainerInput, 0, sizeof(trainerInput));
    trainerSetTimer(0);

    feed(buildFrame(0x00, endByte));

    EXPECT_TRUE(isTrainerValid()) << "end byte 0x" << std::hex << (int)endByte;
    EXPECT_EQ(trainerInput[0], expectedPulse(testChannelValue(0)));
  }
}

// Failsafe / frame-lost frames must not drive the trainer channels.
TEST_F(SbusStreamTest, failsafeAndFrameLostFramesAreIgnored)
{
  feed(buildFrame(1 << 3));  // failsafe
  EXPECT_FALSE(isTrainerValid());
  EXPECT_EQ(trainerInput[0], 0);

  feed(buildFrame(1 << 2));  // frame lost
  EXPECT_FALSE(isTrainerValid());
  EXPECT_EQ(trainerInput[0], 0);

  // ... but the stream is still in sync afterwards
  feed(buildFrame());
  expectTestChannels();
  EXPECT_TRUE(isTrainerValid());
}

// Nothing may reach the trainer channels unless the model actually asked for
// serial trainer input.
TEST_F(SbusStreamTest, ignoredWhenSerialTrainerDisabled)
{
  sbusAuxSetEnabled(false);

  feed(buildFrame());

  EXPECT_FALSE(isTrainerValid());
  for (int ch = 0; ch < 16; ch++) EXPECT_EQ(trainerInput[ch], 0) << "channel " << ch;
}

// Link loss: when frames stop arriving the trainer input has to go stale, so
// the sticks take back control. The timer is decremented from per10ms().
TEST_F(SbusStreamTest, inputGoesStaleWhenFramesStop)
{
  feed(buildFrame());
  ASSERT_TRUE(isTrainerValid());

  // 1s worth of per10ms() ticks with no data
  for (int i = 0; i < 100; i++) {
    EXPECT_TRUE(isTrainerValid()) << "went stale early, at tick " << i;
    trainerDecTimer();
  }

  EXPECT_FALSE(isTrainerValid());
}

#if defined(USB_SERIAL)
// The option has to be offered for USB-VCP in SYS -> Hardware -> Serial Port.
// USB CDC carries no line polarity, so both SBUS trainer modes would behave
// identically here; only the one presented to the user as plain SBUS is
// offered, which is UART_MODE_SBUS_TRAINER_INV (normal SBUS is inverted
// serial, so it is the MCU-inverting mode that reads "SBUS Trainer").
TEST(SbusVcpMenu, sbusTrainerIsOfferedOnVcp)
{
  EXPECT_TRUE(isSerialModeAvailable(SP_VCP, UART_MODE_SBUS_TRAINER_INV));
  EXPECT_FALSE(isSerialModeAvailable(SP_VCP, UART_MODE_SBUS_TRAINER));

  // Both must still be offered on the AUX UARTs
  EXPECT_TRUE(isSerialModeAvailable(SP_AUX1, UART_MODE_SBUS_TRAINER));
}
#endif

// A partial frame left over when the port is released must not be completed by
// whatever the next user of the port sends.
TEST_F(SbusStreamTest, partialFrameIsDroppedOnStop)
{
  auto frame = buildFrame();
  std::vector<uint8_t> partial(frame.begin(), frame.begin() + 20);
  feed(partial);

  sbusStreamStop();

  // the remaining 5 bytes must not complete a frame
  std::vector<uint8_t> rest(frame.begin() + 20, frame.end());
  feed(rest);

  EXPECT_FALSE(isTrainerValid());
}
