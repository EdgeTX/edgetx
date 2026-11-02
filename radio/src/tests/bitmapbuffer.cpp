/*
 * Copyright (C) EdgeTX
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

#if defined(COLORLCD)

#include <vector>

#include "bitmapbuffer.h"

extern void lv_stb_init();

namespace
{

class InspectableBitmapBuffer : public BitmapBuffer
{
 public:
  using BitmapBuffer::BitmapBuffer;
  const pixel_t* dataEnd() const { return data_end; }
};

TEST(BitmapBuffer, RejectsAllocationDimensionOverflow)
{
  for (auto format : {BMP_RGB565, BMP_ARGB4444}) {
    for (const auto& size : {std::pair<uint16_t, uint16_t>{65535, 65535},
                             {65535, 32769}, {32769, 65535}}) {
      InspectableBitmapBuffer image(format, size.first, size.second);
      EXPECT_EQ(image.getData(), nullptr);
      EXPECT_EQ(image.dataEnd(), nullptr);
      EXPECT_FALSE(image.resizeToLVGL(1, 1));
    }
  }
}

TEST(BitmapBuffer, OwnedStorageKeepsItsBoundsAndOwnership)
{
  for (auto format : {BMP_RGB565, BMP_ARGB4444}) {
    InspectableBitmapBuffer image(format, 3, 2);
    auto data = image.getData();
    ASSERT_NE(data, nullptr);
    EXPECT_EQ(image.dataEnd(), data + 6);
    image.setData(nullptr);
    EXPECT_EQ(image.getData(), data);
    EXPECT_EQ(image.dataEnd(), data + 6);
  }
}

TEST(BitmapBuffer, NullBorrowedStorageHasNoEndPointer)
{
  InspectableBitmapBuffer image(BMP_RGB565, 65535, 65535, nullptr);
  EXPECT_EQ(image.getData(), nullptr);
  EXPECT_EQ(image.dataEnd(), nullptr);
  EXPECT_FALSE(image.resizeToLVGL(1, 1));
}

TEST(BitmapBuffer, ReplacingBorrowedStorageUpdatesItsBounds)
{
  pixel_t first[6] = {};
  pixel_t second[6] = {};
  InspectableBitmapBuffer image(BMP_RGB565, 3, 2, first);
  EXPECT_EQ(image.getData(), first);
  EXPECT_EQ(image.dataEnd(), first + 6);
  image.setData(second);
  EXPECT_EQ(image.getData(), second);
  EXPECT_EQ(image.dataEnd(), second + 6);
  image.setData(nullptr);
  EXPECT_EQ(image.getData(), nullptr);
  EXPECT_EQ(image.dataEnd(), nullptr);
  EXPECT_FALSE(image.resizeToLVGL(1, 1));
  image.setData(first);
  EXPECT_EQ(image.getData(), first);
  EXPECT_EQ(image.dataEnd(), first + 6);
}

class TemporaryImageFile
{
 public:
  explicit TemporaryImageFile(const char* filename) :
      filename(filename), source(std::string("A/") + filename)
  {
  }

  ~TemporaryImageFile()
  {
    if (created) EXPECT_EQ(f_unlink(filename), FR_OK);
  }

  ::testing::AssertionResult write(const std::vector<uint8_t>& bytes)
  {
    FIL handle;
    auto opened = f_open(&handle, filename, FA_CREATE_NEW | FA_WRITE);
    if (opened != FR_OK)
      return ::testing::AssertionFailure() << "f_open: " << opened;
    created = true;
    UINT written = 0;
    auto result = f_write(&handle, bytes.data(), bytes.size(), &written);
    auto closed = f_close(&handle);
    if (result != FR_OK || written != bytes.size() || closed != FR_OK)
      return ::testing::AssertionFailure()
             << "f_write: " << result << ", bytes: " << written
             << ", f_close: " << closed;
    return ::testing::AssertionSuccess();
  }

  const char* const filename;
  const std::string source;

 private:
  bool created = false;
};

std::vector<uint8_t> makeBitmap(unsigned width, unsigned height)
{
  unsigned rowSize = (width * 3 + 3) & ~3u;
  unsigned pixelBytes = rowSize * height;
  std::vector<uint8_t> bytes(54 + pixelBytes);
  auto put32 = [&](unsigned offset, unsigned value) {
    for (unsigned i = 0; i < 4; ++i) bytes[offset + i] = value >> (i * 8);
  };
  bytes[0] = 'B';
  bytes[1] = 'M';
  put32(2, bytes.size());
  put32(10, 54);
  put32(14, 40);
  put32(18, width);
  put32(22, height);
  bytes[26] = 1;
  bytes[28] = 24;
  put32(34, pixelBytes);
  return bytes;
}

class BitmapFilenameDecoder : public ::testing::Test
{
 protected:
  static lv_img_decoder_t* decoder;

  static void SetUpTestSuite()
  {
    lv_stb_init();
    lv_img_decoder_dsc_t decoded = {};
    ASSERT_EQ(lv_img_decoder_open(&decoded,
                                 "A/images/color/bitmap_320x240.png",
                                 lv_color_make(0, 0, 0), 0),
              LV_RES_OK);
    decoder = decoded.decoder;
    lv_img_decoder_close(&decoded);
    ASSERT_NE(decoder, nullptr);
  }

  static void TearDownTestSuite()
  {
    if (decoder) lv_img_decoder_delete(decoder);
    decoder = nullptr;
  }
};

lv_img_decoder_t* BitmapFilenameDecoder::decoder = nullptr;

TEST_F(BitmapFilenameDecoder, HeaderDimensionsMustFitLvgl)
{
  constexpr unsigned maxDimension = BitmapBuffer::MAX_LVGL_IMAGE_DIMENSION;
  for (const auto& size :
       {std::pair<unsigned, unsigned>{1, 1}, {maxDimension, 1},
        {1, maxDimension}, {maxDimension + 1, 1}, {1, maxDimension + 1},
        {0, 1}, {1, 0}}) {
    SCOPED_TRACE(::testing::Message() << size.first << "x" << size.second);
    TemporaryImageFile file("images/color/bitmap_decoder_dimensions.bmp");
    ASSERT_TRUE(file.write(makeBitmap(size.first, size.second)));
    lv_img_header_t header = {};
    auto result = lv_img_decoder_get_info(file.source.c_str(), &header);
    bool valid = size.first > 0 && size.second > 0 &&
                 size.first <= maxDimension && size.second <= maxDimension;
    EXPECT_EQ(result, valid ? LV_RES_OK : LV_RES_INV);
    if (valid && result == LV_RES_OK) {
      EXPECT_EQ(header.w, size.first);
      EXPECT_EQ(header.h, size.second);
      EXPECT_EQ(header.cf, LV_IMG_CF_TRUE_COLOR);
    }
  }
}

TEST_F(BitmapFilenameDecoder, OpenRechecksDecodedDimensions)
{
  constexpr unsigned maxDimension = BitmapBuffer::MAX_LVGL_IMAGE_DIMENSION;
  for (const auto& size :
       {std::pair<unsigned, unsigned>{1, 1}, {maxDimension, 1},
        {1, maxDimension}, {maxDimension + 1, 1}, {1, maxDimension + 1}}) {
    SCOPED_TRACE(::testing::Message() << size.first << "x" << size.second);
    TemporaryImageFile file("images/color/bitmap_decoder_open_dimensions.bmp");
    ASSERT_TRUE(file.write(makeBitmap(size.first, size.second)));
    lv_img_decoder_dsc_t decoded = {};
    decoded.src = file.source.c_str();
    decoded.src_type = LV_IMG_SRC_FILE;
    // Bypass info_cb so its header checks cannot hide missing open_cb checks.
    auto result = decoder->open_cb(decoder, &decoded);
    bool valid = size.first <= maxDimension && size.second <= maxDimension;
    EXPECT_EQ(result, valid ? LV_RES_OK : LV_RES_INV);
    if (valid) {
      EXPECT_NE(decoded.img_data, nullptr);
    } else {
      EXPECT_EQ(decoded.img_data, nullptr);
    }
    decoder->close_cb(decoder, &decoded);
    EXPECT_EQ(decoded.img_data, nullptr);
  }
}

TEST_F(BitmapFilenameDecoder, GrayscalePngPreservesColorAndAlpha)
{
  TemporaryImageFile file("images/color/bitmap_decoder_grayscale_alpha.png");
  ASSERT_TRUE(file.write({
      0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d,
      0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x01,
      0x08, 0x04, 0x00, 0x00, 0x00, 0x5e, 0x2b, 0xb7, 0x01, 0x00, 0x00, 0x00,
      0x0d, 0x49, 0x44, 0x41, 0x54, 0x78, 0x9c, 0x63, 0x28, 0x67, 0xb8, 0xfb,
      0x1f, 0x00, 0x04, 0x9a, 0x02, 0x54, 0xd6, 0x57, 0x3f, 0x9f, 0x00, 0x00,
      0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82}));
  lv_img_header_t header = {};
  ASSERT_EQ(lv_img_decoder_get_info(file.source.c_str(), &header), LV_RES_OK);
  EXPECT_EQ(header.cf, LV_IMG_CF_TRUE_COLOR_ALPHA);
  EXPECT_EQ(header.w, 2);
  EXPECT_EQ(header.h, 1);
  lv_img_decoder_dsc_t decoded = {};
  ASSERT_EQ(lv_img_decoder_open(&decoded, file.source.c_str(),
                                lv_color_make(0, 0, 0), 0),
            LV_RES_OK);
  EXPECT_EQ(decoded.header.cf, LV_IMG_CF_TRUE_COLOR_ALPHA);
  EXPECT_EQ(decoded.header.w, 2);
  EXPECT_EQ(decoded.header.h, 1);
  EXPECT_NE(decoded.img_data, nullptr);
  if (decoded.img_data) {
    pixel_t first = RGB(0x77, 0x77, 0x77);
    pixel_t second = RGB(0xDD, 0xDD, 0xDD);
    const uint8_t expected[] = {
        uint8_t(first), uint8_t(first >> 8), 0,
        uint8_t(second), uint8_t(second >> 8), 255};
    EXPECT_EQ(memcmp(decoded.img_data, expected, sizeof(expected)), 0);
  }
  lv_img_decoder_close(&decoded);
  EXPECT_EQ(decoded.img_data, nullptr);
}

}  // namespace

#endif
