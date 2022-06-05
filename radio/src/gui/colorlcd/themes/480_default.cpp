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

class Theme480: public OpenTxTheme
{
  public:
    Theme480():
      OpenTxTheme("EdgeTX", OPTIONS_THEME_DEFAULT)
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

    void loadMenuIcon(uint8_t index, const uint8_t * lbm, bool reload) const
    {
      BitmapBuffer * mask;
      
      if (reload) {
        mask = BitmapBuffer::load8bitMaskLZ4(lbm);
        if (mask) {
          delete iconMask[index];
          iconMask[index] = mask;

          delete menuIconNormal[index];
          menuIconNormal[index] = new BitmapBuffer(BMP_RGB565, mask->width(), mask->height());

          delete menuIconSelected[index];
          menuIconSelected[index] = new BitmapBuffer(BMP_RGB565, mask->width(), mask->height());
        }
      } else
        mask = iconMask[index];
      
      if (menuIconNormal[index]) {
        menuIconNormal[index]->clear(COLOR_THEME_SECONDARY1);
        menuIconNormal[index]->drawMask(0, 0, mask, COLOR_THEME_PRIMARY2);
      }
  
      if (menuIconSelected[index]) {
        menuIconSelected[index]->clear(COLOR_THEME_FOCUS);
        menuIconSelected[index]->drawMask(0, 0, mask, COLOR_THEME_PRIMARY2);
      }
    }

    void loadIcons(bool reload) const
    {
      for (int id = ICON_OPENTX; id != MENUS_ICONS_COUNT; id++) {
        MenuIcons icon_id = (MenuIcons)id;
        loadMenuIcon(icon_id, getBuiltinIcon(icon_id), reload);
      }

      unique_ptr<BitmapBuffer> background(BitmapBuffer::load8bitMaskLZ4(mask_currentmenu_bg));
      unique_ptr<BitmapBuffer> shadow(BitmapBuffer::load8bitMaskLZ4(mask_currentmenu_shadow));
      unique_ptr<BitmapBuffer> dot(BitmapBuffer::load8bitMaskLZ4(mask_currentmenu_dot));
      unique_ptr<BitmapBuffer> topleft(BitmapBuffer::load8bitMaskLZ4(mask_topleft));

      if (!currentMenuBackground) {
        // TODO: get rid of fixed size!!!
        currentMenuBackground = new BitmapBuffer(BMP_RGB565, 36, 53);
      }

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

        currentMenuBackground->drawMask(0, 0, background.get(),
                                        COLOR_THEME_FOCUS);

        currentMenuBackground->drawMask(0, 0, shadow.get(), COLOR_THEME_PRIMARY1);
        currentMenuBackground->drawMask(10, 39, dot.get(), COLOR_THEME_PRIMARY2);
      }

      if (!topleftBitmap) {
        topleftBitmap = new BitmapBuffer(BMP_RGB565, topleft->width(), topleft->height());
      }

      if (topleftBitmap) {
        topleftBitmap->clear(COLOR_THEME_SECONDARY1);
        topleftBitmap->drawMask(0, 0, topleft.get(), COLOR_THEME_FOCUS);
      }
    }

    void setBackgroundImageFileName(const char *fileName) override
    {
      // ensure you delete old bitmap
      if (strcmp(backgroundImageFileName, fileName) != 0 && backgroundBitmap != nullptr)
        delete backgroundBitmap;
      OpenTxTheme::setBackgroundImageFileName(fileName);  // set the filename
      backgroundBitmap = BitmapBuffer::loadBitmap(backgroundImageFileName);
    }

    void load() const override
    {
      loadColors();
      ThemePersistance::instance()->loadDefaultTheme();
      OpenTxTheme::load();
      if (!backgroundBitmap) {
        backgroundBitmap = BitmapBuffer::loadBitmap(getFilePath("background.png"));
      }
      update();
    }

    void update(bool reload = true) const override
    {
      loadIcons(reload);
      if (reload) loadBuiltinBitmaps();
      initLvglTheme();
    }

    void drawBackground(BitmapBuffer * dc) const override
    {
      if (backgroundBitmap) {
        dc->clear(COLOR_THEME_SECONDARY3);
        dc->drawBitmap(0, 0, backgroundBitmap);
      } else {
        dc->drawSolidFilledRect(0, 0, LCD_W, LCD_H, COLOR_THEME_SECONDARY3);
      }
    }

    void drawTopLeftBitmap(BitmapBuffer * dc) const override
    {
      if (topleftBitmap) {
        dc->drawBitmap(0, 0, topleftBitmap);
        dc->drawBitmap(4, 10, menuIconSelected[ICON_OPENTX]);
      }
    }

    void drawPageHeaderBackground(BitmapBuffer *dc, uint8_t icon,
                                  const char *title) const override
    {
      if (topleftBitmap) {
        dc->drawBitmap(0, 0, topleftBitmap);
        uint16_t width = topleftBitmap->width();
        dc->drawSolidFilledRect(width, 0, LCD_W - width, MENU_HEADER_HEIGHT, COLOR_THEME_SECONDARY1);
      }
      else {
        dc->drawSolidFilledRect(0, 0, LCD_W, MENU_HEADER_HEIGHT, COLOR_THEME_SECONDARY1);
      }

      if (icon == ICON_OPENTX)
        dc->drawBitmap(4, 10, menuIconSelected[ICON_OPENTX]);
      else
        dc->drawBitmap(5, 7, menuIconSelected[icon]);

      dc->drawSolidFilledRect(0, MENU_HEADER_HEIGHT, LCD_W,
                              MENU_TITLE_TOP - MENU_HEADER_HEIGHT,
                              COLOR_THEME_SECONDARY3);  // the white separation line

      dc->drawSolidFilledRect(0, MENU_TITLE_TOP, LCD_W, MENU_TITLE_HEIGHT,
                              COLOR_THEME_SECONDARY1);  // the title line background
      if (title) {
        dc->drawText(MENUS_MARGIN_LEFT, MENU_TITLE_TOP + 3, title, COLOR_THEME_PRIMARY2);
      }

      drawMenuDatetime(dc);
    }

    const BitmapBuffer * getIconMask(uint8_t index) const override
    {
      return iconMask[index];
    }

    const BitmapBuffer * getIcon(uint8_t index, IconState state) const override
    {
      return state == STATE_DEFAULT ? menuIconNormal[index] : menuIconSelected[index];
    }

    void drawCurrentMenuBackground(BitmapBuffer *dc) const override
    {
      dc->drawBitmap(0, 0,
                     currentMenuBackground);
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
      const char * const STR_MONTHS[] = TR_MONTHS;
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
    static const BitmapBuffer * backgroundBitmap;
    static BitmapBuffer * topleftBitmap;
    static BitmapBuffer * menuIconNormal[MENUS_ICONS_COUNT];
    static BitmapBuffer * menuIconSelected[MENUS_ICONS_COUNT];
    static BitmapBuffer * iconMask[MENUS_ICONS_COUNT];
    static BitmapBuffer * currentMenuBackground;
};

const BitmapBuffer * Theme480::backgroundBitmap = nullptr;
BitmapBuffer * Theme480::topleftBitmap = nullptr;
BitmapBuffer * Theme480::iconMask[MENUS_ICONS_COUNT] = { nullptr };
BitmapBuffer * Theme480::menuIconNormal[MENUS_ICONS_COUNT] = { nullptr };
BitmapBuffer * Theme480::menuIconSelected[MENUS_ICONS_COUNT] = { nullptr };
BitmapBuffer * Theme480::currentMenuBackground = nullptr;

Theme480 Theme480;

#if LCD_W == 480  || LCD_H == 480
OpenTxTheme * defaultTheme = &Theme480;
Theme * theme = &Theme480;
#endif
