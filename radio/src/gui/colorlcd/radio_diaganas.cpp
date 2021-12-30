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
#include "radio_diaganas.h"
#include "libopenui.h"

#if defined(IMU_LSM6DS33)
#include "imu_lsm6ds33.h"
#endif

#if defined(RADIO_FAMILY_T16) || defined(PCBNV14)
#include "../../targets/horus/flyskyHallStick_driver.h"
#endif

constexpr coord_t ANA_OFFSET = 150;

class RadioAnalogsDiagsWindow: public Window {
  public:
    RadioAnalogsDiagsWindow(Window * parent, const rect_t & rect):
      Window(parent, rect)
    {
    }

    void checkEvents() override
    {
      // will always force a full monitor window refresh
      invalidate();
    }

    void paint(BitmapBuffer * dc) override
    {
#if !defined(SIMU) && (defined(RADIO_FAMILY_T16) || defined(PCBNV14))
        if (globalData.flyskygimbals)
        {
            for (uint8_t i = 0; i < FLYSKY_HALL_CHANNEL_COUNT; i++) {
  #if LCD_W > LCD_H
                coord_t y = 1 + (i / 2) * FH;
                uint8_t x = i & 1 ? LCD_W / 2 + 10 : 10;
  #else
                coord_t y = 1 + i * FH;
                uint8_t x = 10;
  #endif
                dc->drawNumber(x, y, i + 1, LEADING0 | LEFT | COLOR_THEME_PRIMARY1, 2);
                dc->drawText(x + 2 * 15 - 2, y, ":", COLOR_THEME_PRIMARY1);
                dc->drawNumber(x + 3 * 15 - 1, y, hall_raw_values[i], LEFT | COLOR_THEME_PRIMARY1);
                dc->drawNumber(x + ANA_OFFSET, y, (int16_t) calibratedAnalogs[CONVERT_MODE(i)] * 25 / 256, RIGHT | COLOR_THEME_PRIMARY1);
            }

            for (uint8_t i = FLYSKY_HALL_CHANNEL_COUNT; i < NUM_STICKS + NUM_POTS + NUM_SLIDERS; i++) {
  #if LCD_W > LCD_H
                coord_t y = 1 + (i / 2) * FH;
                uint8_t x = i & 1 ? LCD_W / 2 + 10 : 10;
  #else
                coord_t y = 1 + i * FH;
                uint8_t x = 10;
  #endif
                dc->drawNumber(x, y, i + 1, LEADING0 | LEFT | COLOR_THEME_PRIMARY1, 2);
                dc->drawText(x + 2 * 15 - 2, y, ":",  COLOR_THEME_PRIMARY1);
                dc->drawNumber(x + 3 * 15 -1, y, anaIn(i), LEFT | COLOR_THEME_PRIMARY1);
                dc->drawNumber(x + ANA_OFFSET, y, (int16_t) calibratedAnalogs[CONVERT_MODE(i)] * 25 / 256, RIGHT | COLOR_THEME_PRIMARY1);
            }
        }
#endif

        if (!globalData.flyskygimbals) // Also Simulator
        {
            for (uint8_t i = 0; i < NUM_STICKS + NUM_POTS + NUM_SLIDERS; i++) {
#if LCD_W > LCD_H
                coord_t y = 1 + (i / 2) * FH;
                uint8_t x = i & 1 ? LCD_W / 2 + 10 : 10;
#else
                coord_t y = 1 + i * FH;
                uint8_t x = 10;
#endif
                dc->drawNumber(x, y, i + 1, LEADING0 | LEFT | COLOR_THEME_PRIMARY1, 2);
                dc->drawText(x + 2 * 15 - 2, y, ":", COLOR_THEME_PRIMARY1);
                dc->drawNumber(x + 3 * 15 -1, y, anaIn(i), LEFT | COLOR_THEME_PRIMARY1);
                dc->drawNumber(x + ANA_OFFSET, y, (int16_t) calibratedAnalogs[CONVERT_MODE(i)] * 25 / 256, RIGHT | COLOR_THEME_PRIMARY1);
            }
        }

#if !defined(SIMU) && defined(IMU_LSM6DS33)
      coord_t yimu = MENU_CONTENT_TOP + 3 * FH;
      coord_t ximu = MENUS_MARGIN_LEFT;
      char imudata[80];
      sprintf(imudata, "IMU temp.: %.2f deg.C, Gyro XYZ [rad/s]: %.2f, %.2f, %.2f",
              IMUoutput.fTemperatureDegC,
              IMUoutput.fGyroXradps, IMUoutput.fGyroYradps, IMUoutput.fGyroZradps);
      dc->drawText(ximu, yimu, imudata);
      yimu = MENU_CONTENT_TOP + 4 * FH;
      sprintf(imudata, "Linear acceleration XYZ [m/s^2]: %.2f %.2f %.2f",
                IMUoutput.fAccX, IMUoutput.fAccY, IMUoutput.fAccZ);
      dc->drawText(ximu, yimu, imudata);
#endif

#if defined(HARDWARE_TOUCH)
      TouchState rawTouchState = getInternalTouchState();
      constexpr coord_t y = MENU_CONTENT_TOP + 5 * FH;

      if (rawTouchState.event != TE_NONE && rawTouchState.event != TE_SLIDE_END) {
        coord_t x = dc->drawText(MENUS_MARGIN_LEFT, y, STR_TOUCH_PANEL);
        x = dc->drawNumber(x + 5, y, rawTouchState.x);
        x = dc->drawText(x, y, ":");
        dc->drawNumber(x, y, rawTouchState.y);
        dc->drawLine(rawTouchState.x - 10, rawTouchState.y - 8 - parent->top(), rawTouchState.x + 10, rawTouchState.y + 8 - parent->top(), SOLID, 0);
        dc->drawLine(rawTouchState.x - 10, rawTouchState.y + 8 - parent->top(), rawTouchState.x + 10, rawTouchState.y - 8- parent->top(), SOLID, 0);
      }
#if !defined(SIMU) && !defined(PCBNV14)
      constexpr coord_t y1 = MENU_CONTENT_TOP + 6 * FH;
      coord_t x1 = MENUS_MARGIN_LEFT;
      x1 = dc->drawText(x1, y1, "Touch GT911 FW ver:") + 8;
      x1 = dc->drawNumber(x1, y1, touchGT911fwver, LEFT, 4) + 16;
      x1 = dc->drawText(x1, y1, "TSI2CEvents:") + 4;
      dc->drawNumber(x1, y1, touchGT911hiccups, LEFT, 5);
#endif
#endif
    };

  protected:
};

void RadioAnalogsDiagsPage::buildHeader(Window * window)
{
  new StaticText(window, {PAGE_TITLE_LEFT, PAGE_TITLE_TOP + 10, LCD_W - PAGE_TITLE_LEFT, PAGE_LINE_HEIGHT}, STR_MENU_RADIO_ANALOGS, 0, COLOR_THEME_PRIMARY2);
}

void RadioAnalogsDiagsPage::buildBody(Window * window)
{
  new RadioAnalogsDiagsWindow(window, {10, 10, window->width() - 10, window->height() - 10});
}

RadioAnalogsDiagsPage::RadioAnalogsDiagsPage():
  Page(ICON_MODEL_SETUP)
{
  buildHeader(&header);
  buildBody(&body);
  setFocus(SET_FOCUS_DEFAULT);
}
