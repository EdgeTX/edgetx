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

#include "colors.h"

#include <math.h>

#include <algorithm>

const uint16_t defaultColors[] = {
    RGB(0, 0, 0),        // PRIMARY1
    RGB(255, 255, 255),  // PRIMARY2
    RGB(12, 63, 102),    // PRIMARY3
    RGB(18, 94, 153),    // SECONDARY1
    RGB(182, 224, 242),  // SECONDARY2
    RGB(228, 238, 242),  // SECONDARY3
    RGB(20, 161, 229),   // FOCUS
    RGB(0, 153, 9),      // EDIT
    RGB(255, 222, 0),    // ACTIVE
    RGB(224, 0, 0),      // WARNING
    RGB(140, 140, 140),  // DISABLED
    RGB(170, 85, 0),     // CUSTOM
    RGB(0, 0, 0),        // FIXED BLACK
    RGB(255, 255, 255),  // FIXED WHITE
    RGB(128, 128, 128),  // FIXED GREY
};

// Needs to be initialised at compile time so widget color options work
uint16_t lcdColorTable[] = {
    RGB(0, 0, 0),        // PRIMARY1
    RGB(255, 255, 255),  // PRIMARY2
    RGB(12, 63, 102),    // PRIMARY3
    RGB(18, 94, 153),    // SECONDARY1
    RGB(182, 224, 242),  // SECONDARY2
    RGB(228, 238, 242),  // SECONDARY3
    RGB(20, 161, 229),   // FOCUS
    RGB(0, 153, 9),      // EDIT
    RGB(255, 222, 0),    // ACTIVE
    RGB(224, 0, 0),      // WARNING
    RGB(140, 140, 140),  // DISABLED
    RGB(170, 85, 0),     // CUSTOM
    RGB(0, 0, 0),        // FIXED BLACK
    RGB(255, 255, 255),  // FIXED WHITE
    RGB(128, 128, 128),  // FIXED GREY
};

uint32_t HSVtoRGB(float H, float S, float V)
{
  if (H > 360 || H < 0 || S > 100 || S < 0 || V > 100 || V < 0) {
    return 0;
  }

  float s = S / 100;
  float v = V / 100;
  float C = s * v;
  float X = C * (1 - abs(fmod(H / 60.0, 2) - 1));
  float m = v - C;
  float r, g, b;

  if (H >= 0 && H < 60) {
    r = C, g = X, b = 0;
  } else if (H >= 60 && H < 120) {
    r = X, g = C, b = 0;
  } else if (H >= 120 && H < 180) {
    r = 0, g = C, b = X;
  } else if (H >= 180 && H < 240) {
    r = 0, g = X, b = C;
  } else if (H >= 240 && H < 300) {
    r = X, g = 0, b = C;
  } else {
    r = C, g = 0, b = X;
  }
  int R = (r + m) * 255;
  int G = (g + m) * 255;
  int B = (b + m) * 255;

  return RGB(R, G, B);
}

void RGBtoHSV(uint8_t R, uint8_t G, uint8_t B, float &fH, float &fS, float &fV)
{
  float fR = (float)R / 255;
  float fG = (float)G / 255;
  float fB = (float)B / 255;
  float fCMax = std::max(std::max(fR, fG), fB);
  float fCMin = std::min(std::min(fR, fG), fB);
  float fDelta = fCMax - fCMin;

  if (fDelta > 0) {
    if (fCMax == fR) {
      fH = 60 * (fmod(((fG - fB) / fDelta), 6));
    } else if (fCMax == fG) {
      fH = 60 * (((fB - fR) / fDelta) + 2);
    } else if (fCMax == fB) {
      fH = 60 * (((fR - fG) / fDelta) + 4);
    }

    if (fCMax > 0) {
      fS = fDelta / fCMax;
    } else {
      fS = 0;
    }

    fV = fCMax;
  } else {
    fH = 0;
    fS = 0;
    fV = fCMax;
  }

  if (fH < 0) {
    fH = 360 + fH;
  }
}

// TODO: work out how to remove this function
LcdColorIndex indexFromColor(uint32_t lcdFlags)
{
  uint16_t color = COLOR_VAL(lcdFlags);

  for (uint8_t i = 0; i < TOTAL_COLOR_COUNT; i += 1)
    if (lcdColorTable[i] == color) return (LcdColorIndex)i;

  return CUSTOM_COLOR_INDEX;
}

// Return flags with RGB color value instead of indexed theme color
LcdFlags colorToRGB(LcdFlags colorFlags)
{
  // RGB or indexed color?
  if (colorFlags & RGB_FLAG)
    return colorFlags;

  return (colorFlags & 0xFFFF) | COLOR(COLOR_VAL(colorFlags)) | RGB_FLAG;
}

/**
 * Helper function to translate a colorFlags value to a lv_color_t suitable
 * for passing to an lv_obj function
 * @param colorFlags a textFlags value.  This value will contain the color shifted by 16 bits.
 */
lv_color_t makeLvColor(uint32_t colorFlags)
{
  auto color = COLOR_VAL(colorFlags);
  return lv_color_make(GET_RED(color), GET_GREEN(color), GET_BLUE(color));
}
