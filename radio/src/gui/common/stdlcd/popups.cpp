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

#include "opentx.h"

const char * warningText = nullptr;
const char * warningInfoText;
uint8_t      warningInfoLength;
uint8_t      warningType;
uint8_t      warningResult = 0;
uint8_t      warningInfoFlags = 0;

uint8_t      popupMenuOffsetType = MENU_OFFSET_INTERNAL;
void         (* popupFunc)(event_t event) = nullptr;

const char * popupMenuItems[POPUP_MENU_MAX_LINES];
uint8_t      popupMenuSelectedItem = 0;
uint16_t     popupMenuItemsCount = 0;
uint16_t     popupMenuOffset = 0;
void         (* popupMenuHandler)(const char * result);
const char * popupMenuTitle = nullptr;

void drawMessageBoxBackground(coord_t top, coord_t height)
{
  // white background
  lcdDrawFilledRect(MESSAGEBOX_X - 1, top - 1, MESSAGEBOX_W + 2, height + 2, SOLID, ERASE);

  // border
  lcdDrawRect(MESSAGEBOX_X, top, MESSAGEBOX_W, height, SOLID, FORCE);
}

void drawMessageBox(const char * title)
{
  // background + border
  drawMessageBoxBackground(MESSAGEBOX_Y, 48);

  // title
  lcdDrawSizedText(WARNING_LINE_X, WARNING_LINE_Y, title, WARNING_LINE_LEN);

  // could be a place for a warningInfoText
}

void showMessageBox(const char * title)
{
  drawMessageBox(title);
  lcdRefresh();
}

const char * runPopupMenu(event_t event)
{
  const char * result = nullptr;

  uint8_t display_count = min<uint8_t>(popupMenuItemsCount, MENU_MAX_DISPLAY_LINES);
  uint8_t y = LCD_H / 2 - (popupMenuTitle ? 0 : 3) - (display_count * FH / 2);

  // white background
  lcdDrawFilledRect(MENU_X - 1, popupMenuTitle ? y - FH - 3 : y - 1, MENU_W + 2, display_count * (FH + 1) + (popupMenuTitle ? FH + 6 : 4), SOLID, ERASE);

  // title
  if (popupMenuTitle) {
    lcdDrawText(MENU_X + 2, y - FH, popupMenuTitle, BOLD);
    lcdDrawRect(MENU_X, y - FH - 2, lcdLastRightPos - MENU_X + 2, FH + 3);
  }

  // border
  lcdDrawRect(MENU_X, y, MENU_W, display_count * (FH + 1) + 2, SOLID, FORCE);

  // items
  for (uint8_t i=0; i<display_count; i++) {
    lcdDrawText(MENU_X+6, i*(FH+1) + y + 2, popupMenuItems[i+(popupMenuOffsetType == MENU_OFFSET_INTERNAL ? popupMenuOffset : 0)], 0);
    if (i == popupMenuSelectedItem) lcdDrawSolidFilledRect(MENU_X+1, i*(FH+1) + y + 1, MENU_W-2, 9);
  }

  // scrollbar
  if (popupMenuItemsCount > display_count) {
    drawVerticalScrollbar(MENU_X+MENU_W-1, y+1, MENU_MAX_DISPLAY_LINES * (FH+1), popupMenuOffset, popupMenuItemsCount, display_count);
  }

  event_t eventTemp = event;

#if defined(ROTARY_ENCODER_NAVIGATION) && !defined(COLORLCD)
  if (g_eeGeneral.rotEncMode >= ROTARY_ENCODER_MODE_INVERT_BOTH) {
    if (eventTemp == EVT_ROTARY_LEFT) {
      eventTemp = EVT_ROTARY_RIGHT;
    } else if (eventTemp == EVT_ROTARY_RIGHT) {
      eventTemp = EVT_ROTARY_LEFT;
    }
  }
#endif

  switch (eventTemp) {
#if defined(ROTARY_ENCODER_NAVIGATION)
    CASE_EVT_ROTARY_LEFT
#endif
    case EVT_KEY_FIRST(KEY_UP):
    case EVT_KEY_REPT(KEY_UP):
      if (popupMenuSelectedItem > 0) {
        popupMenuSelectedItem--;
      }
#if defined(SDCARD)
      else if (popupMenuOffset > 0) {
        popupMenuOffset--;
        result = STR_UPDATE_LIST;
      }
#endif
      else {
        popupMenuSelectedItem = min<uint8_t>(display_count, MENU_MAX_DISPLAY_LINES) - 1;
#if defined(SDCARD)
        if (popupMenuItemsCount > MENU_MAX_DISPLAY_LINES) {
          popupMenuOffset = popupMenuItemsCount - display_count;
          result = STR_UPDATE_LIST;
        }
#endif
      }
      break;

#if defined(ROTARY_ENCODER_NAVIGATION)
    CASE_EVT_ROTARY_RIGHT
#endif
    case EVT_KEY_FIRST(KEY_DOWN):
    case EVT_KEY_REPT(KEY_DOWN):
      if (popupMenuSelectedItem < display_count - 1 && popupMenuOffset + popupMenuSelectedItem + 1 < popupMenuItemsCount) {
        popupMenuSelectedItem++;
      }
#if defined(SDCARD)
      else if (popupMenuItemsCount > popupMenuOffset + display_count) {
        popupMenuOffset++;
        result = STR_UPDATE_LIST;
      }
#endif
      else {
        popupMenuSelectedItem = 0;
#if defined(SDCARD)
        if (popupMenuOffset) {
          popupMenuOffset = 0;
          result = STR_UPDATE_LIST;
        }
#endif
      }
      break;

    case EVT_KEY_BREAK(KEY_ENTER):
      result = popupMenuItems[popupMenuSelectedItem + (popupMenuOffsetType == MENU_OFFSET_INTERNAL ? popupMenuOffset : 0)];
      popupMenuItemsCount = 0;
      popupMenuSelectedItem = 0;
      popupMenuOffset = 0;
      popupMenuTitle = nullptr;
      break;

    case EVT_KEY_BREAK(KEY_EXIT):
      result = STR_EXIT;
      popupMenuItemsCount = 0;
      popupMenuSelectedItem = 0;
      popupMenuOffset = 0;
      popupMenuTitle = nullptr;
      break;
  }

  return result;
}

void runPopupWarning(event_t event)
{
  warningResult = false;

  drawMessageBox(warningText);

  if (warningInfoText) {
    lcdDrawSizedText(WARNING_LINE_X, WARNING_LINE_Y+FH, warningInfoText, warningInfoLength, warningInfoFlags);
  }

  switch (warningType) {
    case WARNING_TYPE_WAIT:
      return;

    case WARNING_TYPE_INFO:
      lcdDrawText(WARNING_LINE_X, WARNING_LINE_Y+2*FH+2, STR_OK);
      break;

    case WARNING_TYPE_ASTERISK:
      lcdDrawText(WARNING_LINE_X, WARNING_LINE_Y+2*FH+2, BUTTON(TR_EXIT));
      break;

    default:
      lcdDrawText(WARNING_LINE_X, WARNING_LINE_Y+2*FH+2, STR_POPUPS_ENTER_EXIT);
      break;
  }


  switch (event) {
    case EVT_KEY_BREAK(KEY_ENTER):
      if (warningType == WARNING_TYPE_ASTERISK)
        // key ignored, the user has to press [EXIT]
        break;

      if (warningType == WARNING_TYPE_CONFIRM) {
        warningType = WARNING_TYPE_ASTERISK;
        warningText = nullptr;
        if (popupMenuHandler)
          popupMenuHandler(STR_OK);
        else
          warningResult = true;
        break;
      }
      // no break

    case EVT_KEY_BREAK(KEY_EXIT):
      if (warningType == WARNING_TYPE_CONFIRM) {
        if (popupMenuHandler)
          popupMenuHandler(STR_EXIT);
      }
      warningText = nullptr;
      warningType = WARNING_TYPE_ASTERISK;
      break;
  }
}

void showAlertBox(const char * title, const char * text, const char * action , uint8_t sound)
{
  drawAlertBox(title, text, action);
  AUDIO_ERROR_MESSAGE(sound);
  lcdRefresh();
  lcdSetContrast();
  waitKeysReleased();
  resetBacklightTimeout();
  checkBacklight();
}

void drawProgressScreen(const char * title, const char * message, int num, int den)
{
  lcdClear();
  if (title) {
    lcdDrawText(LCD_W / 2 - getTextWidth(title) / 2, 2*FH, title);
  }
  if (message) {
    lcdDrawText(4, 5*FH, message);
  }
  lcdDrawRect(4, 6*FH+4, LCD_W-8, 7);
  if (num > 0 && den > 0) {
    int width = ((LCD_W-12)*num)/den;
    lcdDrawSolidHorizontalLine(6, 6*FH+6, width, FORCE);
    lcdDrawSolidHorizontalLine(6, 6*FH+7, width, FORCE);
    lcdDrawSolidHorizontalLine(6, 6*FH+8, width, FORCE);
  }
  lcdRefresh();
}
