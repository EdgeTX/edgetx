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

#include "edgetx.h"

constexpr coord_t CHANNEL_NAME_OFFSET = 0;
constexpr coord_t CHANNEL_VALUE_OFFSET = CHANNEL_NAME_OFFSET + 41;
constexpr coord_t CHANNEL_BAR_WIDTH = 70;

static bool mixersView = false;

void menuChannelsViewCommon(event_t event)
{
  uint8_t ch;

  switch (event) {
    case EVT_KEY_BREAK(KEY_ENTER):
      mixersView = !mixersView;
      break;
  }

  ch = 8 * (g_eeGeneral.view / ALTERNATE_VIEW);

  coord_t vx = CHANNEL_VALUE_OFFSET;
  coord_t bw = CHANNEL_BAR_WIDTH;
  if (g_eeGeneral.ppmunit == PPM_PERCENT_PREC1) {
    vx += 8;
    bw -= 8;
  }

  // Screen title
  lcdDrawText(LCD_W / 2, 0, mixersView ? STR_MIXERS_MONITOR : STR_CHANNELS_MONITOR, CENTERED);
  lcdInvertLine(0);

  // Channels
  for (uint8_t line = 0; line < 8; line++) {
    LimitData * ld = limitAddress(ch);
    const uint8_t y = 9 + line * 7;
    const int32_t val = mixersView ? ex_chans[ch] : channelOutputs[ch];
    const uint8_t lenLabel = ZLEN(g_model.limitData[ch].name);

    // Channel name if present, number if not
    if (lenLabel > 0) {
      lcdDrawSizedText(CHANNEL_NAME_OFFSET, y, g_model.limitData[ch].name, sizeof(g_model.limitData[ch].name), SMLSIZE);
    } else {
      putsChn(CHANNEL_NAME_OFFSET, y, ch + 1, SMLSIZE);
    }

    // Value
    if (g_eeGeneral.ppmunit == PPM_US) {
      lcdDrawNumber(vx, y + 1, PPM_CH_CENTER(ch) + val / 2, TINSIZE | RIGHT);
    } else if (g_eeGeneral.ppmunit == PPM_PERCENT_PREC1) {
      lcdDrawNumber(vx, y + 1, calcRESXto1000(val), PREC1 | TINSIZE | RIGHT);
    } else {
      lcdDrawNumber(vx, y + 1, calcRESXto1000(val) / 10, TINSIZE | RIGHT);
    }

    // Gauge
    drawGauge(vx, y, bw, 6, val, RESX);

    if (!mixersView) {
      int phase = (g_blinkTmr10ms >> 6) & 3;
      if (phase == 3) phase = 0;

      // Properties
#if defined(OVERRIDE_CHANNEL_FUNCTION)
      if (phase == 1) {
        if (safetyCh[ch] != OVERRIDE_CHANNEL_UNDEFINED)
          lcdDrawText(LCD_W + 1, y + 1, "OVR", TINSIZE | RIGHT);
        else
          phase = 0;
      }
#endif
      if (phase == 2) {
        if (ld && ld->revert)
          lcdDrawText(LCD_W + 1, y + 1, "INV", TINSIZE | RIGHT);
        else
          phase = 0;
      }

      if (phase == 0) {
        if (g_eeGeneral.ppmunit == PPM_US)
          lcdDrawNumber(LCD_W + 1, y + 1, calcRESXto1000(val) / 10, TINSIZE | RIGHT);
        else
          lcdDrawNumber(LCD_W + 1, y + 1, PPM_CH_CENTER(ch) + val / 2, TINSIZE | RIGHT);
      }
    }

    ++ch;
  }
}

void menuChannelsView(event_t event)
{
  switch (event) {
    case EVT_KEY_BREAK(KEY_EXIT):
      popMenu();
      break;

    case EVT_KEY_NEXT_PAGE:
      g_eeGeneral.view = (g_eeGeneral.view + (4 * ALTERNATE_VIEW) + ALTERNATE_VIEW) % (4 * ALTERNATE_VIEW);
      break;

    case EVT_KEY_PREVIOUS_PAGE:
      g_eeGeneral.view = (g_eeGeneral.view + (4 * ALTERNATE_VIEW) - ALTERNATE_VIEW) % (4 * ALTERNATE_VIEW);
      break;
  }

  menuChannelsViewCommon(event);
}
