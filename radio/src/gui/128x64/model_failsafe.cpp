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

extern uint8_t g_moduleIdx;

void onFailsafeMenu(const char * result)
{
  uint8_t sub = menuVerticalPosition;
  int16_t * failsafe = &g_model.failsafeChannels[sub];
  int32_t channelValue = channelOutputs[sub];

  if (result == STR_NONE) {
    *failsafe = FAILSAFE_CHANNEL_NOPULSE;
  }
  else if (result == STR_HOLD) {
    *failsafe = FAILSAFE_CHANNEL_HOLD;
  }
  else if (result == STR_CHANNEL2FAILSAFE) {
    *failsafe = channelValue;
  }
  else if (result == STR_CHANNELS2FAILSAFE) {
    setCustomFailsafe(g_moduleIdx);
  }
  s_editMode = 0;
}

void menuModelFailsafe(event_t event)
{
  uint8_t sub = menuVerticalPosition;
  const coord_t x = 1;
  const int lim = (g_model.extendedLimits ? (512 * LIMIT_EXT_PERCENT / 100) : 512) * 2;

  SIMPLE_SUBMENU_NOTITLE(sentModuleChannels(g_moduleIdx));

  lcdDrawText(LCD_W / 2, 0, STR_FAILSAFESET, CENTERED);
  lcdInvertLine(0);

  for (uint8_t i=0; i<NUM_BODY_LINES; i++) {
    coord_t y = MENU_HEADER_HEIGHT + 1 + i * FH;
    uint8_t k = i + menuVerticalOffset;
    LcdFlags attr = (sub == k) ? INVERS : 0;

    uint8_t wbar = LCD_W - FW * 4 - FWNUM * 4;
    if (g_eeGeneral.ppmunit == PPM_PERCENT_PREC1)
      wbar -= 6;

    if (sub == k && event == EVT_KEY_LONG(KEY_ENTER)) {
      POPUP_MENU_START(onFailsafeMenu, 4, STR_NONE, STR_HOLD, STR_CHANNEL2FAILSAFE, STR_CHANNELS2FAILSAFE);
    }

    // Channel
    putsChn(0, y, k + 1, SMLSIZE);

    // Value
    LcdFlags flags = TINSIZE;
    if (attr) {
      flags |= INVERS;
      if (s_editMode > 0) {
        flags |= BLINK;
        CHECK_INCDEC_MODELVAR(event, g_model.failsafeChannels[k], -lim, +lim);
        if (g_model.failsafeChannels[k] > +lim && g_model.failsafeChannels[k] < FAILSAFE_CHANNEL_HOLD) {
          g_model.failsafeChannels[k] = 0; // switching from HOLD/NOPULSE to value edit
        }
      }
    }

    const int32_t channelValue = channelOutputs[k];
    int32_t failsafeValue = g_model.failsafeChannels[k];
    const coord_t xValue = x+LCD_W-4-wbar;
    if (failsafeValue == FAILSAFE_CHANNEL_HOLD) {
      lcdDrawText(xValue, y, STR_HOLD_UPPERCASE, RIGHT|flags);
      failsafeValue = 0;
    }
    else if (failsafeValue == FAILSAFE_CHANNEL_NOPULSE) {
      lcdDrawText(xValue, y, STR_NONE_UPPERCASE, RIGHT|flags);
      failsafeValue = 0;
    }
    else {
      if (g_eeGeneral.ppmunit == PPM_US) {
        lcdDrawNumber(xValue, y, PPM_CH_CENTER(k)+failsafeValue/2, RIGHT|flags);
      } else if (g_eeGeneral.ppmunit == PPM_PERCENT_PREC1) {
        lcdDrawNumber(xValue, y, calcRESXto1000(failsafeValue), RIGHT|PREC1|flags);
      } else {
        lcdDrawNumber(xValue, y, calcRESXto1000(failsafeValue)/10, RIGHT|flags);
      }
    }

    // Gauge
#if !(defined(PCBX7) || defined(PCBX9LITE) || defined(PCBX9LITES)) // X7/X9 LCD doesn't like too many horizontal lines
    lcdDrawRect(x+LCD_W-3-wbar, y, wbar+1, 6);
#endif
    const uint8_t lenChannel = limit<uint8_t>(1, (abs(channelValue) * wbar/2 + lim/2) / lim, wbar/2);
    const uint8_t lenFailsafe = limit<uint8_t>(1, (abs(failsafeValue) * wbar/2 + lim/2) / lim, wbar/2);
    const coord_t xChannel = (channelValue>0) ? x+LCD_W-3-wbar/2 : x+LCD_W-2-wbar/2-lenChannel;
    const coord_t xFailsafe = (failsafeValue>0) ? x+LCD_W-3-wbar/2 : x+LCD_W-2-wbar/2-lenFailsafe;
    lcdDrawHorizontalLine(xChannel, y+1, lenChannel, DOTTED, 0);
    lcdDrawHorizontalLine(xChannel, y+2, lenChannel, DOTTED, 0);
    lcdDrawSolidHorizontalLine(xFailsafe, y+3, lenFailsafe);
    lcdDrawSolidHorizontalLine(xFailsafe, y+4, lenFailsafe);
  }
}
