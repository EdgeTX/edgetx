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
#include "libopenui.h"

//#include "checkbox.h"
//#include "modal_window.h"

extern OpenTxTheme * defaultTheme;
const BitmapBuffer * OpenTxTheme::error = nullptr;
const BitmapBuffer * OpenTxTheme::busy = nullptr;
const BitmapBuffer * OpenTxTheme::shutdown = nullptr;

constexpr coord_t LBM_USB_PLUGGED_W = 211;
constexpr coord_t LBM_USB_PLUGGED_H = 110;

const uint8_t _LBM_USB_PLUGGED[] = {
#include "mask_usb_symbol.lbm"
};
STATIC_LZ4_BITMAP(LBM_USB_PLUGGED);

const uint8_t error_bitmap[] = {
#include "mask_error.lbm"
};

const uint8_t busy_bitmap[] = {
#include "mask_busy.lbm"
};

const uint8_t shutdown_bitmap[] = {
#include "mask_shutdown.lbm"
};

std::list<OpenTxTheme *> & getRegisteredThemes()
{
  static std::list<OpenTxTheme *> themes;
  return themes;
}

void registerTheme(OpenTxTheme * theme)
{
  TRACE("register theme %s", theme->getName());
  getRegisteredThemes().push_back(theme);
}

void OpenTxTheme::init() const
{
  memset(&g_eeGeneral.themeData, 0, sizeof(OpenTxTheme::PersistentData));
  if (options) {
    int i = 0;
    for (const ZoneOption * option = options; option->name; option++, i++) {
      // TODO compiler bug? The CPU freezes ... g_eeGeneral.themeData.options[i] = &option->deflt;
      memcpy(&g_eeGeneral.themeData.options[i].value, &option->deflt, sizeof(ZoneOptionValue));
      g_eeGeneral.themeData.options[i].type = zoneValueEnumFromType(option->type);
    }
  }
}

void OpenTxTheme::load() const
{
  if (!error)
    error = BitmapBuffer::load8bitMaskLZ4(error_bitmap);
  if (!busy)
    busy = BitmapBuffer::load8bitMaskLZ4(busy_bitmap);
  if (!shutdown)
    shutdown = BitmapBuffer::load8bitMaskLZ4(shutdown_bitmap);
}

ZoneOptionValue * OpenTxTheme::getOptionValue(unsigned int index) const
{
  return &g_eeGeneral.themeData.options[index].value;
}

const char * OpenTxTheme::getFilePath(const char * filename) const
{
  static char path[FF_MAX_LFN+1] = THEMES_PATH "/";
  strcpy(path + sizeof(THEMES_PATH), getName());
  int len = sizeof(THEMES_PATH) + strlen(path + sizeof(THEMES_PATH));
  path[len] = '/';
  strcpy(path+len+1, filename);
  return path;
}

void OpenTxTheme::drawThumb(BitmapBuffer * dc, coord_t x, coord_t y, uint32_t flags)
{
  #define THUMB_WIDTH   51
  #define THUMB_HEIGHT  31
  if (!thumb) {
    thumb = BitmapBuffer::loadBitmap(getFilePath("thumb.bmp"));
  }
  lcd->drawBitmap(x, y, thumb);
  if (flags == COLOR_THEME_PRIMARY3) {
    dc->drawFilledRect(x, y, THUMB_WIDTH, THUMB_HEIGHT, SOLID, COLOR_THEME_PRIMARY1);
  }
}

void OpenTxTheme::drawBackground(BitmapBuffer * dc) const
{
  dc->drawSolidFilledRect(0, 0, LCD_W, LCD_H, COLOR_THEME_SECONDARY3);
}

//void OpenTxTheme::drawMessageBox(const char *title, const char *text,
//                                 const char *action, uint32_t type) const
//{
//  //if (flags & MESSAGEBOX_TYPE_ALERT) {
//    drawBackground();
//    lcdDrawFilledRect(0, POPUP_Y, LCD_W, POPUP_H, SOLID, COLOR_THEME_PRIMARY2 |
//    OPACITY(8));
//  //}
//
//  if (type == WARNING_TYPE_ALERT || type == WARNING_TYPE_ASTERISK)
//    lcd->drawBitmap(POPUP_X-80, POPUP_Y+12, asterisk);
//  else if (type == WARNING_TYPE_INFO)
//    lcd->drawBitmap(POPUP_X-80, POPUP_Y+12, busy);
//  else
//    lcd->drawBitmap(POPUP_X-80, POPUP_Y+12, question);
//
//  if (type == WARNING_TYPE_ALERT) {
//#if defined(TRANSLATIONS_FR) || defined(TRANSLATIONS_IT) ||
//defined(TRANSLATIONS_CZ)
//    lcdDrawText(WARNING_LINE_X, WARNING_LINE_Y, STR_WARNING,
//    COLOR_THEME_WARNING|FONT(XL)); lcdDrawText(WARNING_LINE_X, WARNING_LINE_Y+28,
//    title, COLOR_THEME_WARNING|FONT(XL));
//#else
//    lcdDrawText(WARNING_LINE_X, WARNING_LINE_Y, title, COLOR_THEME_WARNING|FONT(XL));
//    lcdDrawText(WARNING_LINE_X, WARNING_LINE_Y+28, STR_WARNING,
//    COLOR_THEME_WARNING|FONT(XL));
//#endif
//  }
//  else if (title) {
//    lcdDrawText(WARNING_LINE_X, WARNING_LINE_Y, title, COLOR_THEME_WARNING|FONT(XL));
//  }
//
//  if (text) {
//    lcdDrawText(WARNING_LINE_X, WARNING_INFOLINE_Y, text);
//  }
//
//  if (action) {
//    lcdDrawText(WARNING_LINE_X, WARNING_INFOLINE_Y+24, action);
//  }
//}

void OpenTxTheme::drawCheckBox(BitmapBuffer *dc, bool checked, coord_t x,
                               coord_t y, bool focus) const
{
  dc->drawSolidFilledRect(x, y, 16, 16, COLOR_THEME_PRIMARY2);
  if (focus) {
    dc->drawSolidRect(x, y, 16, 16, 2, COLOR_THEME_FOCUS);
  }
  else {
    dc->drawSolidRect(x, y, 16, 16, 1, COLOR_THEME_SECONDARY2);
  }
  if (checked) {
    dc->drawSolidFilledRect(x + 3, y + 3, 10, 10, COLOR_THEME_FOCUS);
  }
}

void OpenTxTheme::drawUsbPluggedScreen(BitmapBuffer * dc) const
{
  // draw USB icon
  dc->clear(COLOR_THEME_SECONDARY3);
  dc->drawBitmapPattern((LCD_W - LBM_USB_PLUGGED_W) / 2,
                        (LCD_H - LBM_USB_PLUGGED_H) / 2,
                        LBM_USB_PLUGGED, COLOR_THEME_SECONDARY1);
}


OpenTxTheme * getTheme(const char * name)
{
  std::list<OpenTxTheme *>::const_iterator it = getRegisteredThemes().cbegin();
  for (; it != getRegisteredThemes().cend(); ++it) {
    if (!strcmp(name, (*it)->getName())) {
      return (*it);
    }
  }
  return nullptr;
}

void loadTheme(OpenTxTheme * newTheme)
{
  TRACE("load theme %s", newTheme->getName());
  theme = newTheme;
  newTheme->load();
}

void loadTheme()
{
  char name[THEME_NAME_LEN + 1];
  memset(name, 0, sizeof(name));
  strncpy(name, g_eeGeneral.themeName, THEME_NAME_LEN);
  OpenTxTheme * newTheme = getTheme(name);
  if (newTheme)
    loadTheme(newTheme);
  else
    loadTheme(defaultTheme);
}

MenuWindowContent * createMenuWindow(Menu * menu)
{
  return new MenuWindowContent(menu);
}

DialogWindowContent * createDialogWindow(Dialog * dialog, const rect_t & rect)
{
  TRACE("createDialogWindow [%d, %d, %d, %d]", rect.x, rect.y, rect.w, rect.h);
  return new DialogWindowContent(dialog, rect);
}
