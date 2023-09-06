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
#include "theme.h"
#include "theme_manager.h"

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

const uint8_t mask_topleft[] = {
#include "mask_topleft.lbm"
};

const uint8_t mask_currentmenu_bg[] = {
#include "mask_currentmenu_bg.lbm"
};

const uint8_t mask_currentmenu_dot[] = {
#include "mask_currentmenu_dot.lbm"
};

const uint8_t mask_currentmenu_shadow[] = {
#include "mask_currentmenu_shadow.lbm"
};

uint16_t EdgeTxTheme::defaultColors[LCD_COLOR_COUNT] = {
  RGB(18, 94, 153),     // DEFAULT
  RGB(0, 0, 0),         // PRIMARY1
  RGB(255, 255, 255),   // PRIMARY2
  RGB(12, 63, 102),     // PRIMARY3
  RGB(18, 94, 153),     // SECONDARY1
  RGB(182, 224, 242),   // SECONDARY2
  RGB(228, 238, 242),   // SECONDARY3
  RGB(20, 161, 229),    // FOCUS
  RGB(0, 153, 9),       // EDIT
  RGB(255, 222, 0),     // ACTIVE
  RGB(224, 0, 0),       // WARNING
  RGB(140, 140, 140),   // DISABLED
  RGB(170, 85, 0)       // CUSTOM
};

EdgeTxTheme::EdgeTxTheme(): name("EdgeTX")
{
  loadColors();
}

void EdgeTxTheme::load()
{
  loadColors();
  ThemePersistance::instance()->loadDefaultTheme();

  if (!error)
    error = BitmapBuffer::load8bitMaskLZ4(error_bitmap);
  if (!busy)
    busy = BitmapBuffer::load8bitMaskLZ4(busy_bitmap);
  if (!shutdown)
    shutdown = BitmapBuffer::load8bitMaskLZ4(shutdown_bitmap);

  update();
}

void EdgeTxTheme::update()
{
  createIcons();
  loadIcons();
  if (!backgroundBitmap) {
    backgroundBitmap = BitmapBuffer::loadBitmap(getFilePath("background.png"));
  }
  initLvglTheme();
}

void EdgeTxTheme::loadColors() const
{
  TRACE("Load EdgeTX theme colors");
  memcpy(lcdColorTable, defaultColors, sizeof(defaultColors));
}

void EdgeTxTheme::createIcons()
{
  if (!iconsLoaded) {
    iconsLoaded = true;

    iconMask = new BitmapBuffer*[MENUS_ICONS_COUNT];
    for (int id = ICON_EDGETX; id != MENUS_ICONS_COUNT; id++) {
      iconMask[id] = BitmapBuffer::load8bitMaskLZ4(getBuiltinIcon((MenuIcons)id));
    }

    // Get mask with max size. Extract size from shadow LBM file.
    uint16_t* shadow = (uint16_t*)mask_currentmenu_shadow;
    currentMenuBackground = new BitmapBuffer(BMP_RGB565, shadow[0], shadow[1]);

    topleftBitmap = BitmapBuffer::load8bitMaskLZ4(mask_topleft);

    loadBuiltinBitmaps();
  }
}

void EdgeTxTheme::loadIcons() const
{
  if (currentMenuBackground) {
    currentMenuBackground->drawSolidFilledRect(
        0, 0, currentMenuBackground->width(), currentMenuBackground->height(),
        COLOR_THEME_SECONDARY1);

    currentMenuBackground->drawSolidFilledRect(
        0, MENU_HEADER_HEIGHT, currentMenuBackground->width(),
        MENU_TITLE_TOP - MENU_HEADER_HEIGHT, COLOR_THEME_SECONDARY3);

    std::unique_ptr<BitmapBuffer> background(BitmapBuffer::load8bitMaskLZ4(mask_currentmenu_bg));
    currentMenuBackground->drawMask(0, 0, background.get(), COLOR_THEME_FOCUS);

    std::unique_ptr<BitmapBuffer> shadow(BitmapBuffer::load8bitMaskLZ4(mask_currentmenu_shadow));
    currentMenuBackground->drawMask(0, 0, shadow.get(), COLOR_THEME_PRIMARY1);

    std::unique_ptr<BitmapBuffer> dot(BitmapBuffer::load8bitMaskLZ4(mask_currentmenu_dot));
    currentMenuBackground->drawMask(10, 39, dot.get(), COLOR_THEME_PRIMARY2);
  }
}

void EdgeTxTheme::setBackgroundImageFileName(const char *fileName)
{
  // ensure you delete old bitmap
  if (backgroundBitmap != nullptr)
    delete backgroundBitmap;

  strncpy(backgroundImageFileName, fileName, FF_MAX_LFN);
  backgroundImageFileName[FF_MAX_LFN] = '\0'; // ensure string termination

  // Try to load bitmap. If this fails backgroundBitmap will be NULL and default will be loaded in update() method
  backgroundBitmap = BitmapBuffer::loadBitmap(backgroundImageFileName);
}

const char * EdgeTxTheme::getFilePath(const char * filename) const
{
  static char path[FF_MAX_LFN+1] = THEMES_PATH "/";
  strcpy(path + sizeof(THEMES_PATH), name);
  int len = sizeof(THEMES_PATH) + strlen(path + sizeof(THEMES_PATH));
  path[len] = '/';
  strcpy(path+len+1, filename);
  return path;
}

const BitmapBuffer * EdgeTxTheme::getIconMask(uint8_t index) const
{
  return iconMask[index];
}

void EdgeTxTheme::drawUsbPluggedScreen(BitmapBuffer * dc) const
{
  // draw USB icon
  dc->clear(COLOR_THEME_SECONDARY3);
  uint16_t* usb_icon_hdr = (uint16_t*)_LBM_USB_PLUGGED;
  dc->drawBitmapPattern((LCD_W - usb_icon_hdr[0]) / 2,
                        (LCD_H - usb_icon_hdr[1]) / 2,
                        LBM_USB_PLUGGED, COLOR_THEME_SECONDARY1);
}

void EdgeTxTheme::drawBackground(BitmapBuffer * dc) const
{
  dc->clear(COLOR_THEME_SECONDARY3);
  if (backgroundBitmap)
    dc->drawBitmap(0, 0, backgroundBitmap);
}

void EdgeTxTheme::drawHeaderIcon(BitmapBuffer * dc, uint8_t icon) const
{
  dc->drawSolidFilledRect(0, 0, LCD_W, MENU_HEADER_HEIGHT, COLOR_THEME_SECONDARY1);

  if (topleftBitmap)
    dc->drawMask(0, 0, topleftBitmap, COLOR_THEME_FOCUS);

  if (icon == ICON_EDGETX)
    dc->drawMask(4, 10, iconMask[icon], COLOR_THEME_PRIMARY2);
  else
    dc->drawMask(5, 7, iconMask[icon], COLOR_THEME_PRIMARY2);
}

void EdgeTxTheme::drawPageHeaderBackground(BitmapBuffer *dc, uint8_t icon, const char *title) const
{
  drawHeaderIcon(dc, icon);

  dc->drawSolidFilledRect(0, MENU_HEADER_HEIGHT, LCD_W,
                          MENU_TITLE_TOP - MENU_HEADER_HEIGHT,
                          COLOR_THEME_SECONDARY3);  // the white separation line

  dc->drawSolidFilledRect(0, MENU_TITLE_TOP, LCD_W, MENU_TITLE_HEIGHT,
                          COLOR_THEME_SECONDARY1);  // the title line background
  if (title) {
    dc->drawText(MENUS_MARGIN_LEFT, MENU_TITLE_TOP + 1, title, COLOR_THEME_PRIMARY2);
  }

  drawMenuDatetime(dc, DATETIME_MIDDLE, DATETIME_LINE1, COLOR_THEME_PRIMARY2);
}

void EdgeTxTheme::drawMenuDatetime(BitmapBuffer * dc, coord_t x, coord_t y, LcdFlags color) const
{
  const TimerOptions timerOptions = {.options = SHOW_TIME};
  struct gtm t;
  gettime(&t);
  char str[10];
#if defined(TRANSLATIONS_CN) || defined(TRANSLATIONS_TW)
  sprintf(str, "%02d-%02d", t.tm_mon + 1, t.tm_mday);
#else
  sprintf(str, "%d %s", t.tm_mday, STR_MONTHS[t.tm_mon]);
#endif
  dc->drawText(x, y, str, FONT(XS)|color|CENTERED);
  getTimerString(str, getValue(MIXSRC_TX_TIME), timerOptions);
  dc->drawText(x, y + 15, str, FONT(XS)|color|CENTERED);
}

void EdgeTxTheme::drawMenuIcon(BitmapBuffer *dc, uint8_t icon, bool checked) const
{
  if (checked)
    dc->drawBitmap(0, 0, currentMenuBackground);
  dc->drawMask(2, 7, iconMask[icon], COLOR_THEME_PRIMARY2);
}

EdgeTxTheme defaultTheme;

EdgeTxTheme * EdgeTxTheme::instance()
{
  return &defaultTheme;
}
