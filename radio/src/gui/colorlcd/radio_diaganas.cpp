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
#include "../../hal/adc_driver.h"

#if defined(IMU_LSM6DS33)
#include "imu_lsm6ds33.h"
#endif

#if defined(RADIO_FAMILY_T16) || defined(PCBNV14) || defined(PCBPL18)
#include "../../targets/horus/flyskyHallStick_driver.h"
#endif

#define STATSDEPTH 8 // ideally a value of power of 2

constexpr coord_t CA_X_OFFSET = 70;
constexpr coord_t VALUE_X_OFFSET = 120;
constexpr coord_t DEV_X_OFFSET = 125;
constexpr coord_t MIN_X_OFFSET = 115;
constexpr coord_t MAX_X_OFFSET = 165;
constexpr coord_t RANGE_X_OFFSET = 170;

RadioAnalogsDiagsViewPageGroup::RadioAnalogsDiagsViewPageGroup() : TabsGroup(ICON_STATS)
{
  addTab(new AnaCalibratedViewPage());
  addTab(new AnaFilteredDevViewPage());
  addTab(new AnaUnfilteredRawViewPage());
  addTab(new AnaMinMaxViewPage());
}

class AnaCalibratedViewWindow: public Window {
  public:
    AnaCalibratedViewWindow(Window * parent, const rect_t & rect):
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
#if !defined(SIMU) && (defined(RADIO_FAMILY_T16) || defined(PCBNV14) || defined(PCBPL18))
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
                dc->drawNumber(x + CA_X_OFFSET, y, (int16_t) calibratedAnalogs[CONVERT_MODE(i)] * 25 / 256, RIGHT | COLOR_THEME_PRIMARY1);
                dc->drawNumber(x + VALUE_X_OFFSET, y, hall_raw_values[i], RIGHT | COLOR_THEME_PRIMARY1);
            }

            //for (uint8_t i = FLYSKY_HALL_CHANNEL_COUNT; i < NUM_STICKS + NUM_POTS + NUM_SLIDERS; i++) {
            for (uint8_t i = FLYSKY_HALL_CHANNEL_COUNT; i < NUM_CALIBRATED_ANALOGS; i++) {
                #if LCD_W > LCD_H
                  coord_t y = 1 + (i / 2) * FH;
                  uint8_t x = i & 1 ? LCD_W / 2 + 10 : 10;
                #else
                  coord_t y = 1 + i * FH;
                  uint8_t x = 10;
                #endif
                dc->drawNumber(x, y, i + 1, LEADING0 | LEFT | COLOR_THEME_PRIMARY1, 2);
                dc->drawText(x + 2 * 15 - 2, y, ":",  COLOR_THEME_PRIMARY1);
                dc->drawNumber(x + CA_X_OFFSET, y, (int16_t) calibratedAnalogs[CONVERT_MODE(i)] * 25 / 256, RIGHT | COLOR_THEME_PRIMARY1);
                dc->drawNumber(x + VALUE_X_OFFSET, y, anaIn(i), RIGHT | COLOR_THEME_PRIMARY1);
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
                dc->drawNumber(x + CA_X_OFFSET, y, (int16_t) calibratedAnalogs[CONVERT_MODE(i)] * 25 / 256, RIGHT | COLOR_THEME_PRIMARY1);
                dc->drawNumber(x + VALUE_X_OFFSET, y, anaIn(i), RIGHT | COLOR_THEME_PRIMARY1);
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
      constexpr coord_t y = MENU_CONTENT_TOP + 4 * FH;

      if (rawTouchState.event != TE_NONE && rawTouchState.event != TE_SLIDE_END) {
        coord_t x = dc->drawText(MENUS_MARGIN_LEFT, y, STR_TOUCH_PANEL);
        x = dc->drawNumber(x + 5, y, rawTouchState.x);
        x = dc->drawText(x, y, ":");
        dc->drawNumber(x, y, rawTouchState.y);
        dc->drawLine(rawTouchState.x - 10, rawTouchState.y - 8 - parent->top(), rawTouchState.x + 10, rawTouchState.y + 8 - parent->top(), SOLID, 0);
        dc->drawLine(rawTouchState.x - 10, rawTouchState.y + 8 - parent->top(), rawTouchState.x + 10, rawTouchState.y - 8- parent->top(), SOLID, 0);
      }
#if !defined(SIMU)
#if !defined(PCBNV14) && !defined(PCBPL18)
      constexpr coord_t y1 = MENU_CONTENT_TOP + 5 * FH;
      coord_t x1 = MENUS_MARGIN_LEFT;
      x1 = dc->drawText(x1, y1, "Touch Chip FW ver:") + 8;
      x1 = dc->drawNumber(x1, y1, touchICfwver, LEFT, 4) + 16;
#elif defined(PCBPL18)
      constexpr coord_t y1 = MENU_CONTENT_TOP + NUM_CALIBRATED_ANALOGS * FH;
      coord_t x1 = MENUS_MARGIN_LEFT;
      x1 = dc->drawText(x1, y1, "Touch Chip FW ver:") + 8;
      x1 = dc->drawNumber(x1, y1, touchICfwver >> 8, LEFT, 0);
      x1 = dc->drawText(x1, y1, ".");
      x1 = dc->drawNumber(x1, y1, touchICfwver & 0xFF, LEFT, 0) + 16;
#endif
        x1 = dc->drawText(x1, y1, "TSI2CEvents:") + 4;
      dc->drawNumber(x1, y1, touchI2Chiccups, LEFT, 5);
#endif
#endif
    };

  protected:
};

class AnaFilteredDevViewWindow: public Window {
  protected:
    class Stats {
      protected:
        int16_t buffer[STATSDEPTH] = {0};
        uint8_t writePos = 0;
        uint8_t filledElems = 0;

        int16_t intSqrt(int16_t val)
        {
          if (val < 0)
              return 0;
          if (val <= 1)
              return val;

          // Try iteratively until i^2 >= val
          int i = 1, retval = 1;
          while (retval <= val)
          {
            i++;
            retval = i * i;
          }
          return i - 1;
        }
        int16_t meanVal()
        {
          if (filledElems)
          {
              int sum = 0;
              for (uint8_t i=0; i<filledElems; i++)
                  sum += buffer[i];
              return (int16_t)(sum/filledElems);
          } else
              return 0;
        }

      public:
        Stats() {}

        void clear()
        {
          writePos = 0;
          filledElems = 0;
        }

        // Always accept new data, discard old
        void write(int16_t value)
        {
          buffer[writePos] = value;
          writePos = (writePos + 1) % STATSDEPTH;
          if (filledElems < STATSDEPTH)
            filledElems++;
        }

        uint16_t maxDev()
        {
          if (filledElems)
          {
              uint16_t ret = 0;
              for (uint8_t i=0; i<filledElems; i++)
                  ret = max<uint16_t>(ret, abs(buffer[i] - meanVal()));
              return ret;
          } else
              return 0;
        }

        int16_t stdDev()
        {
          if (filledElems)
          {
              int16_t mean=meanVal();
              int calc = 0;
              for (uint8_t i=0; i<filledElems; i++)
                  calc += (buffer[i] - mean)*(buffer[i] - mean);
              calc = intSqrt((int16_t)(calc/filledElems));
              return (int16_t)calc;
          } else
              return 0;
        }
    };
    bool ClearStats = true;

  public:
    AnaFilteredDevViewWindow(Window * parent, const rect_t & rect):
      Window(parent, rect) {}

    void checkEvents() override
    {
      // will always force a full monitor window refresh
      invalidate();
    }

    void paint(BitmapBuffer * dc) override
    {
        static Stats stats[NUM_STICKS+NUM_POTS+NUM_SLIDERS];

        if (ClearStats)
        {
            for (uint8_t i = 0; i < NUM_STICKS + NUM_POTS + NUM_SLIDERS; i++) {
                stats[i].clear();
            }
            ClearStats = false;
        }

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
                  dc->drawNumber(x + CA_X_OFFSET, y, (int16_t) calibratedAnalogs[CONVERT_MODE(i)] * 25 / 256, RIGHT | COLOR_THEME_PRIMARY1);
                  stats[i].write(hall_raw_values[i]);
                  dc->drawNumber(x + VALUE_X_OFFSET, y, hall_raw_values[i], RIGHT | COLOR_THEME_PRIMARY1); // no need to use calculated mean for FlySky - the output is stable enough to display directly
                  dc->drawNumber(dc->drawText(x + DEV_X_OFFSET, y, " +/- ", COLOR_THEME_PRIMARY1), y, stats[i].maxDev(), LEFT | COLOR_THEME_PRIMARY1);
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
                  dc->drawNumber(x + CA_X_OFFSET, y, (int16_t) calibratedAnalogs[CONVERT_MODE(i)] * 25 / 256, RIGHT | COLOR_THEME_PRIMARY1);
                  stats[i].write(getAnalogValue(i));
                  extern uint32_t s_anaFilt[NUM_ANALOGS];
                  dc->drawNumber(x + VALUE_X_OFFSET, y, s_anaFilt[i]/JITTER_ALPHA, RIGHT | COLOR_THEME_PRIMARY1); // use integrated filter
                  dc->drawNumber(dc->drawText(x + DEV_X_OFFSET, y, " +/- ", COLOR_THEME_PRIMARY1), y, stats[i].maxDev(), LEFT | COLOR_THEME_PRIMARY1);
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
                  dc->drawNumber(x + CA_X_OFFSET, y, (int16_t) calibratedAnalogs[CONVERT_MODE(i)] * 25 / 256, RIGHT | COLOR_THEME_PRIMARY1);
                  stats[i].write(getAnalogValue(i));
                  #if !defined(SIMU)
                    extern uint32_t s_anaFilt[NUM_ANALOGS];
                    dc->drawNumber(x + VALUE_X_OFFSET, y, s_anaFilt[i]/JITTER_ALPHA, RIGHT | COLOR_THEME_PRIMARY1); // use integrated filter
                  #else
                    dc->drawNumber(x + VALUE_X_OFFSET, y, anaIn(i), RIGHT | COLOR_THEME_PRIMARY1); // for simu, can use directly the input values without filtering
                  #endif
                  dc->drawNumber(dc->drawText(x + DEV_X_OFFSET, y, " +/- ", COLOR_THEME_PRIMARY1), y, stats[i].maxDev(), LEFT | COLOR_THEME_PRIMARY1);
              }
          }
    };
};

class AnaUnfilteredRawViewWindow: public Window {
  public:
    AnaUnfilteredRawViewWindow(Window * parent, const rect_t & rect):
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
                dc->drawNumber(x + CA_X_OFFSET, y, (int16_t) calibratedAnalogs[CONVERT_MODE(i)] * 25 / 256, RIGHT | COLOR_THEME_PRIMARY1);
                dc->drawNumber(x + VALUE_X_OFFSET, y, hall_raw_values[i], RIGHT | COLOR_THEME_PRIMARY1);
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
                dc->drawNumber(x + CA_X_OFFSET, y, (int16_t) calibratedAnalogs[CONVERT_MODE(i)] * 25 / 256, RIGHT | COLOR_THEME_PRIMARY1);
                dc->drawNumber(x + VALUE_X_OFFSET, y, getAnalogValue(i), RIGHT | COLOR_THEME_PRIMARY1);
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
                dc->drawNumber(x + CA_X_OFFSET, y, (int16_t) calibratedAnalogs[CONVERT_MODE(i)] * 25 / 256, RIGHT | COLOR_THEME_PRIMARY1);
                dc->drawNumber(x + VALUE_X_OFFSET, y, getAnalogValue(i), RIGHT | COLOR_THEME_PRIMARY1);
            }
        }
    };

  protected:
};

class AnaMinMaxViewWindow: public Window {
  protected:
    class MinMax {
      protected:
        bool noElements = true;
        int16_t minvalue = INT16_MAX;
        int16_t maxvalue = INT16_MIN;

      public:
        MinMax() {}

        void clear()
        {
          minvalue = INT16_MAX;
          maxvalue = INT16_MIN;
        }

        // Always accept new data, discard old
        void write(int16_t value)
        {
          if (value < minvalue)
              minvalue = value;
          if (value > maxvalue)
              maxvalue = value;
        }

        int16_t MinVal()
        {
            return minvalue;
        }

        int16_t MaxVal()
        {
            return maxvalue;
        }

        uint16_t Range()
        {
            if (maxvalue > minvalue)
                return (uint16_t)(maxvalue - minvalue);
            else
                return 0;
        }
    };
    bool ClearStats = true;

  public:
    AnaMinMaxViewWindow(Window * parent, const rect_t & rect):
      Window(parent, rect) {}

    void checkEvents() override
    {
      // will always force a full monitor window refresh
      invalidate();
    }

    void paint(BitmapBuffer * dc) override
    {
        static MinMax minmax[NUM_STICKS+NUM_POTS+NUM_SLIDERS];

        if (ClearStats)
        {
            for (uint8_t i = 0; i < NUM_STICKS + NUM_POTS + NUM_SLIDERS; i++) {
                minmax[i].clear();
            }
            ClearStats = false;
        }
        dc->drawText(10, 1, STR_ANADIAGS_MOVE, COLOR_THEME_PRIMARY1);

#if !defined(SIMU) && (defined(RADIO_FAMILY_T16) || defined(PCBNV14))
          if (globalData.flyskygimbals)
          {
              for (uint8_t i = 0; i < FLYSKY_HALL_CHANNEL_COUNT; i++) {
                  #if LCD_W > LCD_H
                    coord_t y = 1 + (i / 2 + 1) * FH;
                    uint8_t x = i & 1 ? LCD_W / 2 + 10 : 10;
                  #else
                    coord_t y = 1 + (i + 1) * FH;
                    uint8_t x = 10;
                  #endif
                  dc->drawNumber(x, y, i + 1, LEADING0 | LEFT | COLOR_THEME_PRIMARY1, 2);
                  dc->drawText(x + 2 * 15 - 2, y, ":", COLOR_THEME_PRIMARY1);
                  dc->drawNumber(x + CA_X_OFFSET, y, (int16_t) calibratedAnalogs[CONVERT_MODE(i)] * 25 / 256, RIGHT | COLOR_THEME_PRIMARY1);
                  minmax[i].write(hall_raw_values[i]);
                  dc->drawNumber(x + MIN_X_OFFSET, y, minmax[i].MinVal(), RIGHT | COLOR_THEME_PRIMARY1);
                  dc->drawNumber(x + MAX_X_OFFSET, y, minmax[i].MaxVal(), RIGHT | COLOR_THEME_PRIMARY1);
                  dc->drawNumber(x + RANGE_X_OFFSET, y, minmax[i].Range(), LEFT | COLOR_THEME_PRIMARY1);
              }

              for (uint8_t i = FLYSKY_HALL_CHANNEL_COUNT; i < NUM_STICKS + NUM_POTS + NUM_SLIDERS; i++) {
                  #if LCD_W > LCD_H
                    coord_t y = 1 + (i / 2 + 1) * FH;
                    uint8_t x = i & 1 ? LCD_W / 2 + 10 : 10;
                  #else
                    coord_t y = 1 + (i + 1) * FH;
                    uint8_t x = 10;
                  #endif
                  dc->drawNumber(x, y, i + 1, LEADING0 | LEFT | COLOR_THEME_PRIMARY1, 2);
                  dc->drawText(x + 2 * 15 - 2, y, ":",  COLOR_THEME_PRIMARY1);
                  dc->drawNumber(x + CA_X_OFFSET, y, (int16_t) calibratedAnalogs[CONVERT_MODE(i)] * 25 / 256, RIGHT | COLOR_THEME_PRIMARY1);
                  minmax[i].write(getAnalogValue(i));
                  dc->drawNumber(x + MIN_X_OFFSET, y, minmax[i].MinVal(), RIGHT | COLOR_THEME_PRIMARY1);
                  dc->drawNumber(x + MAX_X_OFFSET, y, minmax[i].MaxVal(), RIGHT | COLOR_THEME_PRIMARY1);
                  dc->drawNumber(x + RANGE_X_OFFSET, y, minmax[i].Range(), LEFT | COLOR_THEME_PRIMARY1);
              }
          }
#endif

          if (!globalData.flyskygimbals) // Also Simulator
          {
              for (uint8_t i = 0; i < NUM_STICKS + NUM_POTS + NUM_SLIDERS; i++) {
                  #if LCD_W > LCD_H
                    coord_t y = 1 + (i / 2 + 1) * FH;
                    uint8_t x = i & 1 ? LCD_W / 2 + 10 : 10;
                  #else
                    coord_t y = 1 + (i + 1) * FH;
                    uint8_t x = 10;
                  #endif
                  dc->drawNumber(x, y, i + 1, LEADING0 | LEFT | COLOR_THEME_PRIMARY1, 2);
                  dc->drawText(x + 2 * 15 - 2, y, ":", COLOR_THEME_PRIMARY1);
                  dc->drawNumber(x + CA_X_OFFSET, y, (int16_t) calibratedAnalogs[CONVERT_MODE(i)] * 25 / 256, RIGHT | COLOR_THEME_PRIMARY1);
                  minmax[i].write(getAnalogValue(i));
                  dc->drawNumber(x + MIN_X_OFFSET, y, minmax[i].MinVal(), RIGHT | COLOR_THEME_PRIMARY1);
                  dc->drawNumber(x + MAX_X_OFFSET, y, minmax[i].MaxVal(), RIGHT | COLOR_THEME_PRIMARY1);
                  dc->drawNumber(x + RANGE_X_OFFSET, y, minmax[i].Range(), LEFT | COLOR_THEME_PRIMARY1);
              }
          }
    };
};

void AnaCalibratedViewPage::build(FormWindow *window)
{
    new AnaCalibratedViewWindow(window, {10, 10, window->width() - 10, window->height() - 10});
}

void AnaFilteredDevViewPage::build(FormWindow *window)
{
    new AnaFilteredDevViewWindow(window, {10, 10, window->width() - 10, window->height() - 10});
}

void AnaUnfilteredRawViewPage::build(FormWindow *window)
{
    new AnaUnfilteredRawViewWindow(window, {10, 10, window->width() - 10, window->height() - 10});
}

void AnaMinMaxViewPage::build(FormWindow *window)
{
    new AnaMinMaxViewWindow(window, {10, 10, window->width() - 10, window->height() - 10});
}
