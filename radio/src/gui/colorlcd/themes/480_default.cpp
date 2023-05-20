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
#include "tabsgroup.h"
#include "bitmaps.h"
#include "theme_manager.h"

#include <memory>
using std::unique_ptr;

const ZoneOption OPTIONS_THEME_DEFAULT[] = {
  { STR_BACKGROUND_COLOR, ZoneOption::Color, OPTION_VALUE_UNSIGNED(COLOR_THEME_PRIMARY2) },
  { STR_MAIN_COLOR, ZoneOption::Color, OPTION_VALUE_UNSIGNED(COLOR_THEME_WARNING) },
  { nullptr, ZoneOption::Bool }
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

class Theme480: public EdgeTxTheme
{
  public:
    Theme480():
      EdgeTxTheme("EdgeTX", OPTIONS_THEME_DEFAULT)
    {
      loadColors();
    }

    void loadColors() const
    {
      TRACE("Load EdgeTX theme colors");

      lcdColorTable[DEFAULT_COLOR_INDEX] = RGB(18, 94, 153);

      lcdColorTable[COLOR_THEME_PRIMARY1_INDEX] = RGB(0, 0, 0);
      lcdColorTable[COLOR_THEME_PRIMARY2_INDEX] = RGB(255, 255, 255);
      lcdColorTable[COLOR_THEME_PRIMARY3_INDEX] = RGB(12, 63, 102);
      lcdColorTable[COLOR_THEME_SECONDARY1_INDEX] = RGB(18, 94, 153);
      lcdColorTable[COLOR_THEME_SECONDARY2_INDEX] = RGB(182, 224, 242);
      lcdColorTable[COLOR_THEME_SECONDARY3_INDEX] = RGB(228, 238, 242);
      lcdColorTable[COLOR_THEME_FOCUS_INDEX] = RGB(20, 161, 229);
      lcdColorTable[COLOR_THEME_EDIT_INDEX] = RGB(0, 153, 9);
      lcdColorTable[COLOR_THEME_ACTIVE_INDEX] = RGB(255, 222, 0);
      lcdColorTable[COLOR_THEME_WARNING_INDEX] = RGB(224, 0, 0);
      lcdColorTable[COLOR_THEME_DISABLED_INDEX] = RGB(140, 140, 140);
      lcdColorTable[CUSTOM_COLOR_INDEX] = RGB(170, 85, 0);
    }

    void loadIcons() const
    {
      if (iconsLoaded)
        return;

      iconsLoaded = true;

      for (int id = ICON_OPENTX; id != MENUS_ICONS_COUNT; id++) {
        iconMask[id] = BitmapBuffer::load8bitMaskLZ4(getBuiltinIcon((MenuIcons)id));
      }

      // Get mask with max size
      unique_ptr<BitmapBuffer> shadow(BitmapBuffer::load8bitMaskLZ4(mask_currentmenu_shadow));

      currentMenuBackground = new BitmapBuffer(BMP_RGB565, shadow->width(), shadow->height());

      if (currentMenuBackground) {

        currentMenuBackground->drawSolidFilledRect(
            0, 0, currentMenuBackground->width(), MENU_HEADER_HEIGHT,
            COLOR_THEME_SECONDARY1);

        currentMenuBackground->drawSolidFilledRect(
            0, MENU_HEADER_HEIGHT, currentMenuBackground->width(),
            MENU_TITLE_TOP - MENU_HEADER_HEIGHT, COLOR_THEME_SECONDARY3);

        currentMenuBackground->drawSolidFilledRect(
            0, MENU_TITLE_TOP, currentMenuBackground->width(),
            currentMenuBackground->height() - MENU_TITLE_TOP, COLOR_THEME_SECONDARY1);

        unique_ptr<BitmapBuffer> background(BitmapBuffer::load8bitMaskLZ4(mask_currentmenu_bg));
        currentMenuBackground->drawMask(0, 0, background.get(), COLOR_THEME_FOCUS);

        unique_ptr<BitmapBuffer> shadow(BitmapBuffer::load8bitMaskLZ4(mask_currentmenu_shadow));
        currentMenuBackground->drawMask(0, 0, shadow.get(), COLOR_THEME_PRIMARY1);

        unique_ptr<BitmapBuffer> dot(BitmapBuffer::load8bitMaskLZ4(mask_currentmenu_dot));
        currentMenuBackground->drawMask(10, 39, dot.get(), COLOR_THEME_PRIMARY2);
      }

      topleftBitmap = BitmapBuffer::load8bitMaskLZ4(mask_topleft);

      loadBuiltinBitmaps();
    }

    void setBackgroundImageFileName(const char *fileName) override
    {
      // ensure you delete old bitmap
      if (backgroundBitmap != nullptr)
        delete backgroundBitmap;
      EdgeTxTheme::setBackgroundImageFileName(fileName);  // set the filename
      backgroundBitmap = BitmapBuffer::loadBitmap(backgroundImageFileName);
    }

    void load() const override
    {
      loadColors();
      ThemePersistance::instance()->loadDefaultTheme();
      EdgeTxTheme::load();
      if (!backgroundBitmap) {
        backgroundBitmap = BitmapBuffer::loadBitmap(getFilePath("background.png"));
      }
      update();
    }

    void update(bool reload = true) const override
    {
      loadIcons();
      initLvglTheme();
    }

    void drawBackground(BitmapBuffer * dc) const override
    {
      dc->clear(COLOR_THEME_SECONDARY3);
      if (backgroundBitmap)
        dc->drawBitmap(0, 0, backgroundBitmap);
    }

    void drawTopLeftBitmap(BitmapBuffer * dc) const override
    {
      if (topleftBitmap) {
        dc->drawMask(0, 0, topleftBitmap, COLOR_THEME_FOCUS);
        dc->drawMask(4, 10, iconMask[ICON_OPENTX], COLOR_THEME_PRIMARY2);
      }
    }

    void drawPageHeaderBackground(BitmapBuffer *dc, uint8_t icon,
                                  const char *title) const override
    {
      dc->drawSolidFilledRect(0, 0, LCD_W, MENU_HEADER_HEIGHT, COLOR_THEME_SECONDARY1);
      if (topleftBitmap)
        dc->drawMask(0, 0, topleftBitmap, COLOR_THEME_FOCUS);

      if (icon == ICON_OPENTX)
        dc->drawMask(4, 10, iconMask[ICON_OPENTX], COLOR_THEME_PRIMARY2);
      else
        dc->drawMask(5, 7, iconMask[icon], COLOR_THEME_PRIMARY2);

      dc->drawSolidFilledRect(0, MENU_HEADER_HEIGHT, LCD_W,
                              MENU_TITLE_TOP - MENU_HEADER_HEIGHT,
                              COLOR_THEME_SECONDARY3);  // the white separation line

      dc->drawSolidFilledRect(0, MENU_TITLE_TOP, LCD_W, MENU_TITLE_HEIGHT,
                              COLOR_THEME_SECONDARY1);  // the title line background
      if (title) {
        dc->drawText(MENUS_MARGIN_LEFT, MENU_TITLE_TOP + 1, title, COLOR_THEME_PRIMARY2);
      }

      drawMenuDatetime(dc);
    }

    const BitmapBuffer * getIconMask(uint8_t index) const override
    {
      return iconMask[index];
    }

    void drawMenuIcon(BitmapBuffer *dc, uint8_t icon, bool checked) const override
    {
      if (checked)
        dc->drawBitmap(0, 0, currentMenuBackground);
      dc->drawMask(2, 7, iconMask[icon], COLOR_THEME_PRIMARY2);
    }

    void drawMenuDatetime(BitmapBuffer * dc) const
    {
      //dc->drawSolidVerticalLine(DATETIME_SEPARATOR_X, 7, 31, COLOR_THEME_PRIMARY2);
      const TimerOptions timerOptions = {.options = SHOW_TIME};
      struct gtm t;
      gettime(&t);
      char str[10];
#if defined(TRANSLATIONS_CN) || defined(TRANSLATIONS_TW)
      sprintf(str, "%02d-%02d", t.tm_mon + 1, t.tm_mday);
#else
      sprintf(str, "%d %s", t.tm_mday, STR_MONTHS[t.tm_mon]);
#endif
      dc->drawText(DATETIME_MIDDLE, DATETIME_LINE1, str, FONT(XS)|COLOR_THEME_PRIMARY2|CENTERED);
      getTimerString(str, getValue(MIXSRC_TX_TIME), timerOptions);
      dc->drawText(DATETIME_MIDDLE, DATETIME_LINE2, str, FONT(XS)|COLOR_THEME_PRIMARY2|CENTERED);
    }

    void drawProgressBar(BitmapBuffer *dc, coord_t x, coord_t y, coord_t w,
                         coord_t h, int value, int total) const override
    {
      dc->drawSolidRect(x, y, w, h, 1, COLOR_THEME_SECONDARY1);
      if (value > 0) {
        int width = (w * value) / total;
        dc->drawSolidFilledRect(x + 2, y + 2, width - 4, h - 4, COLOR_THEME_FOCUS);
      }
    }

  protected:
    static bool iconsLoaded;
    static const BitmapBuffer * backgroundBitmap;
    static BitmapBuffer * topleftBitmap;
    static BitmapBuffer * iconMask[MENUS_ICONS_COUNT];
    static BitmapBuffer * currentMenuBackground;
};

bool Theme480::iconsLoaded = false;

const BitmapBuffer * Theme480::backgroundBitmap = nullptr;
BitmapBuffer * Theme480::topleftBitmap = nullptr;
BitmapBuffer * Theme480::iconMask[MENUS_ICONS_COUNT] = { nullptr };
BitmapBuffer * Theme480::currentMenuBackground = nullptr;

Theme480 theme480;

#if LCD_W == 480  || LCD_H == 480
EdgeTxTheme * defaultTheme = &theme480;
EdgeTxTheme * theme = &theme480;
#endif
