/*
 * Copyright (C) EdgeTX
 *
 * PNG encoder backend for Widget Studio LCD capture (simuCaptureArm /
 * simuLcdNotify, see simulib.h).  Deliberately kept OUT of simulib.cpp: that
 * file lives in the simu_drivers object library, which is also linked into
 * gtests-radio and wasi-module, and stb_image_write's single-definition
 * pattern would collide with radio/src/tests/lcd_480x272.cpp's own copy.
 * This file is compiled only into the interactive `simu` executable and owns
 * the PNG encoder plus the native frame-ready callback.
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 */

#include "simulib.h"

#if defined(WIDGET_STUDIO) && !defined(__wasm__)

#include "simulcd.h"

#include <vector>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

namespace {

bool captureDump(const char* path)
{
#if defined(COLORLCD)
  const int w = LCD_W;
  const int h = LCD_H;
  std::vector<uint8_t> rgb(w * h * 3);
  const pixel_t* src = simuLcdBuf;
  for (int i = 0; i < w * h; i++) {
    uint16_t p = src[i];
    uint8_t r5 = (p >> 11) & 0x1F;
    uint8_t g6 = (p >> 5) & 0x3F;
    uint8_t b5 = p & 0x1F;
    rgb[i * 3 + 0] = (r5 << 3) | (r5 >> 2);
    rgb[i * 3 + 1] = (g6 << 2) | (g6 >> 4);
    rgb[i * 3 + 2] = (b5 << 3) | (b5 >> 2);
  }
  return stbi_write_png(path, w, h, 3, rgb.data(), w * 3) != 0;
#else
  (void)path;
  return false;
#endif
}

}  // namespace

void simuLcdNotify()
{
  std::string path;
  if (simuConsumeCapturePath(path)) captureDump(path.c_str());
}

#endif  // WIDGET_STUDIO && !__wasm__
