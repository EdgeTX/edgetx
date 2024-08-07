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

void menuModelFailsafe(event_t event)
{
  const coord_t barH = (LCD_H - FH) / 8 - 1;
  const int lim = (g_model.extendedLimits ? (512 * LIMIT_EXT_PERCENT / 100) : 512) * 2;
  const uint8_t channelStart = g_model.moduleData[g_moduleIdx].channelsStart;
  uint8_t cols = 1;
  uint8_t colW = LCD_W;

  switch(event) {
    case EVT_ENTRY:
      reusableBuffer.modelFailsafe.maxNameLen = 4;
      break;

    case EVT_KEY_LONG(KEY_ENTER):
      killEvents(event);
      event = 0;

      if (menuVerticalPosition < sentModuleChannels(g_moduleIdx)) {
        if (s_editMode) {
          g_model.failsafeChannels[menuVerticalPosition] = channelOutputs[menuVerticalPosition+channelStart];
          s_editMode = 0;
        }
        else {
          int16_t * failsafe = &g_model.failsafeChannels[menuVerticalPosition];
          if (*failsafe < FAILSAFE_CHANNEL_HOLD)
            *failsafe = FAILSAFE_CHANNEL_HOLD;
          else if (*failsafe == FAILSAFE_CHANNEL_HOLD)
            *failsafe = FAILSAFE_CHANNEL_NOPULSE;
          else
            *failsafe = 0;
        }
      }
      else {
        // "Outputs => Failsafe" menu item
        setCustomFailsafe(g_moduleIdx);
      }

      storageDirty(EE_MODEL);
      AUDIO_WARNING1();
      SEND_FAILSAFE_NOW(g_moduleIdx);
      break;
  }

  SIMPLE_SUBMENU_NOTITLE(sentModuleChannels(g_moduleIdx) + 1);
  SET_SCROLLBAR_X(0);

  if (sentModuleChannels(g_moduleIdx) > 8) {
    cols = 2;
    colW = LCD_W / cols - 1;
    // Column separator
    if (menuVerticalPosition >= sentModuleChannels(g_moduleIdx)) {
      lcdDrawSolidVerticalLine(colW, FH, LCD_H - (FH*2 + FH/2 + 2));
    }
    else {
      lcdDrawSolidVerticalLine(colW, FH, LCD_H - FH);
    }
  }

  lcdDrawText(LCD_W / 2, 0, STR_FAILSAFESET, CENTERED);
  lcdInvertLine(0);

  coord_t x = colW;
  for (uint8_t col = 0; col < cols; col++) {

    coord_t y = FH + 1;
    uint8_t line = (menuVerticalPosition >= sentModuleChannels(g_moduleIdx) ? 2 : 0);
    uint8_t ch = line + col*8;

    for (; line < 8; line++) {
      const int32_t channelValue = channelOutputs[ch+channelStart];
      int32_t failsafeValue = g_model.failsafeChannels[8*col+line];
      uint8_t lenLabel = ZLEN(g_model.limitData[ch+channelStart].name);
      uint8_t barW = colW - FW * reusableBuffer.modelFailsafe.maxNameLen - FWNUM * 3;  // default bar width

      if (g_eeGeneral.ppmunit == PPM_PERCENT_PREC1) {
        barW -= FWNUM + 1;
      }
      barW += (barW % 2);

      // Channel name if present, number if not
      if (lenLabel > 0) {
        if (lenLabel > reusableBuffer.modelFailsafe.maxNameLen)
          reusableBuffer.modelFailsafe.maxNameLen = lenLabel;
        lcdDrawSizedText(x - colW, y, g_model.limitData[ch+channelStart].name, sizeof(g_model.limitData[ch+channelStart].name), ZCHAR | SMLSIZE);
      }
      else {
        putsChn(x - colW, y, ch+1, SMLSIZE);
      }

      // Value
      LcdFlags flags = TINSIZE;
      if (menuVerticalPosition == ch) {
        flags |= INVERS;
        if (s_editMode) {
          if (failsafeValue == FAILSAFE_CHANNEL_HOLD || failsafeValue == FAILSAFE_CHANNEL_NOPULSE) {
            s_editMode = 0;
          }
          else {
            flags |= BLINK;
            CHECK_INCDEC_MODELVAR(event, g_model.failsafeChannels[8*col+line], -lim, +lim);
          }
        }
      }

      const coord_t xValue = x - barW;
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
          lcdDrawNumber(xValue, y, PPM_CH_CENTER(ch)+failsafeValue/2, RIGHT|flags);
        } else if (g_eeGeneral.ppmunit == PPM_PERCENT_PREC1) {
          lcdDrawNumber(xValue, y, calcRESXto1000(failsafeValue), RIGHT|PREC1|flags);
        } else {
          lcdDrawNumber(xValue, y, calcRESXto1000(failsafeValue)/10, RIGHT|flags);
        }
      }

      // Gauge
      lcdDrawRect(x - barW, y, barW - 1, barH);
      barW = barW / 2 - 1;
      const coord_t lenChannel  = limit<uint8_t>(1, (abs(channelValue)  * barW + lim / 2) / lim, barW);
      const coord_t lenFailsafe = limit<uint8_t>(1, (abs(failsafeValue) * barW + lim / 2) / lim, barW);
      const coord_t barX = x - barW - 2;
      const coord_t xChannel  = (channelValue >= 0) ? barX : barX - lenChannel + 1;
      const coord_t xFailsafe = (failsafeValue > 0) ? barX : barX - lenFailsafe + 1;
      lcdDrawHorizontalLine(xChannel, y+1, lenChannel, DOTTED, 0);
      lcdDrawHorizontalLine(xChannel, y+2, lenChannel, DOTTED, 0);
      lcdDrawSolidHorizontalLine(xFailsafe, y+3, lenFailsafe);
      lcdDrawSolidHorizontalLine(xFailsafe, y+4, lenFailsafe);

      if (++ch >= sentModuleChannels(g_moduleIdx))
        break;

      y += barH + 1;

    }  // channels

    x += colW + 2;

  }  // columns

  if (menuVerticalPosition >= sentModuleChannels(g_moduleIdx)) {
    // Outputs => Failsafe
    lcdDrawText(CENTER_OFS, LCD_H - (FH + 1), STR_CHANNELS2FAILSAFE, INVERS);
  }
}
