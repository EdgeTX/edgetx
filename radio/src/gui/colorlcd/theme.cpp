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

#include "theme.h"

#include "libopenui.h"
#include "theme_manager.h"
#include "topbar_impl.h"

EdgeTxTheme* EdgeTxTheme::_instance = nullptr;

EdgeTxTheme* EdgeTxTheme::instance()
{
  if (!_instance) _instance = new EdgeTxTheme();
  return _instance;
}

EdgeTxTheme::EdgeTxTheme() {}

void EdgeTxTheme::load() { ThemePersistance::instance()->loadDefaultTheme(); }

void EdgeTxTheme::update()
{
  initLvglTheme();
  if (!backgroundBitmap) {
    backgroundBitmap = BitmapBuffer::loadBitmap(
        THEMES_PATH "/EdgeTX/background.png", BMP_RGB565);
  }
  if (backgroundBitmap) MainWindow::instance()->setBackground(backgroundBitmap);
}

void EdgeTxTheme::setBackgroundImageFileName(const char* fileName)
{
  // ensure you delete old bitmap
  if (backgroundBitmap != nullptr) delete backgroundBitmap;

  strncpy(backgroundImageFileName, fileName, FF_MAX_LFN);
  backgroundImageFileName[FF_MAX_LFN] = '\0';  // ensure string termination

  // Try to load bitmap. If this fails backgroundBitmap will be NULL and default
  // will be loaded in update() method
  backgroundBitmap =
      BitmapBuffer::loadBitmap(backgroundImageFileName, BMP_RGB565);
}

HeaderDateTime::HeaderDateTime(lv_obj_t* parent, coord_t x, coord_t y)
{
  date = lv_label_create(parent);
  lv_obj_set_pos(date, x, y);
  lv_obj_set_size(date, HDR_DATE_WIDTH, HDR_DATE_HEIGHT);
  lv_obj_set_style_text_align(date, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
  etx_txt_color(date, COLOR_THEME_PRIMARY2_INDEX);
  etx_font(date, FONT_XS_INDEX);

  time = lv_label_create(parent);
  lv_obj_set_pos(time, x, y + HDR_DATE_LINE2);
  lv_obj_set_size(time, HDR_DATE_WIDTH, HDR_DATE_HEIGHT);
  lv_obj_set_style_text_align(time, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
  etx_txt_color(time, COLOR_THEME_PRIMARY2_INDEX);
  etx_font(time, FONT_XS_INDEX);

  update();
}

void HeaderDateTime::update()
{
  const TimerOptions timerOptions = {.options = SHOW_TIME};
  struct gtm t;
  gettime(&t);

  if (t.tm_min != lastMinute) {
    char str[10];
#if defined(TRANSLATIONS_CN) || defined(TRANSLATIONS_TW)
    sprintf(str, "%02d-%02d", t.tm_mon + 1, t.tm_mday);
#else
    sprintf(str, "%d %s", t.tm_mday, STR_MONTHS[t.tm_mon]);
#endif
    lv_label_set_text(date, str);

    getTimerString(str, getValue(MIXSRC_TX_TIME), timerOptions);
    lv_label_set_text(time, str);

    lastMinute = t.tm_min;
  }
}

void HeaderDateTime::setColor(uint32_t color)
{
  lv_obj_set_style_text_color(date, makeLvColor(color), LV_PART_MAIN);
  lv_obj_set_style_text_color(time, makeLvColor(color), LV_PART_MAIN);
}

HeaderIcon::HeaderIcon(Window* parent, EdgeTxIcon icon) :
  StaticIcon(parent, 0, 0, ICON_TOPLEFT_BG, COLOR_THEME_FOCUS)
{
  (new StaticIcon(this, 0, 0, icon, COLOR_THEME_PRIMARY2))->center(width(), height());
}

UsbSDConnected::UsbSDConnected() :
    Window(MainWindow::instance(), {0, 0, LCD_W, LCD_H})
{
  setWindowFlag(OPAQUE);

  etx_solid_bg(lvobj, COLOR_THEME_PRIMARY1_INDEX);
  dateTime = new HeaderDateTime(lvobj, LCD_W - TopBar::HDR_DATE_XO, HDR_DATE_Y);

  auto icon = new StaticIcon(this, 0, 0, ICON_USB_PLUGGED, COLOR_THEME_PRIMARY2);
  lv_obj_center(icon->getLvObj());
}

void UsbSDConnected::checkEvents()
{
  dateTime->update();
}
