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

// #if defined(IMU_LSM6DS33)
// #include "imu_lsm6ds33.h"
// #endif

#if defined(FLYSKY_GIMBAL)
  #include "flysky_gimbal_driver.h"
#endif

#define STATSDEPTH 8 // ideally a value of power of 2

#if LCD_W > LCD_H

static const lv_coord_t col_dsc[] = {LV_GRID_FR(30), LV_GRID_FR(30), LV_GRID_FR(40), LV_GRID_FR(40), LV_GRID_FR(40),
                                     LV_GRID_FR(30), LV_GRID_FR(30), LV_GRID_FR(40), LV_GRID_FR(40), LV_GRID_FR(40),
                                     LV_GRID_TEMPLATE_LAST};

#else

static const lv_coord_t col_dsc[] = {LV_GRID_FR(30), LV_GRID_FR(30), LV_GRID_FR(40), LV_GRID_FR(40), LV_GRID_FR(40),
                                     LV_GRID_TEMPLATE_LAST};

#endif

static const lv_coord_t row_dsc[] = {LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};

class AnaCalibratedViewWindow: public FormWindow {
  public:
    AnaCalibratedViewWindow(Window * parent, const rect_t & rect):
      FormWindow(parent, rect)
    {
      padAll(4);
      padLeft(10);
      padRight(10);
      setFlexLayout();
      FlexGridLayout grid(col_dsc, row_dsc, 0);

      char s[10];
      auto line = newLine(&grid);

      for (uint8_t i = 0; i < NUM_STICKS + NUM_POTS + NUM_SLIDERS; i++) {
        lv_obj_set_style_pad_column(line->getLvObj(), 8, 0);
        sprintf(s, "%02d :", i + 1);
        new StaticText(line, rect_t{}, s, COLOR_THEME_PRIMARY1);
        auto lbl = new DynamicText(line, rect_t{}, [=]() {
          return std::to_string((int16_t)calibratedAnalogs[CONVERT_MODE(i)] * 25 / 256);
        }, COLOR_THEME_PRIMARY1);
        lv_obj_set_style_text_align(lbl->getLvObj(), LV_TEXT_ALIGN_RIGHT, 0);
        lbl = new DynamicText(line, rect_t{}, [=]() {
#if !defined(SIMU) && defined(FLYSKY_GIMBAL)
          if (globalData.flyskygimbals && (i < FLYSKY_HALL_CHANNEL_COUNT))
            return std::to_string(hall_raw_values[i]);
          else
            return std::to_string((int16_t)anaIn(i));
#else
          return std::to_string((int16_t)anaIn(i));
#endif
        }, COLOR_THEME_PRIMARY1);
        lv_obj_set_style_text_align(lbl->getLvObj(), LV_TEXT_ALIGN_RIGHT, 0);
        grid.nextCell();
        grid.nextCell();
#if LCD_W > LCD_H
        if ((i & 1) == 1)
          line = newLine(&grid);
#else
        line = newLine(&grid);
#endif
      }

#if defined(HARDWARE_TOUCH)
      line = newLine(&grid);
#if LCD_H > LCD_W
      line->padTop(20);
#else
      line->padTop(2);
#endif

      auto lbl = new DynamicText(line, rect_t{}, [=]() {
        TouchState rawTouchState = getInternalTouchState();
        if (rawTouchState.event != TE_NONE && rawTouchState.event != TE_SLIDE_END)
          return std::string(STR_TOUCH_PANEL) + " " + std::to_string(rawTouchState.x) + " : " + std::to_string(rawTouchState.y);
        return std::string("");
      }, COLOR_THEME_PRIMARY1);
      lv_obj_set_grid_cell(lbl->getLvObj(), LV_GRID_ALIGN_STRETCH, 0, 5, LV_GRID_ALIGN_CENTER, 0, 1);

#if !defined(SIMU) && !defined(PCBNV14)
      line = newLine(&grid);
      auto lbl2 = new StaticText(line, rect_t{}, std::string("Touch GT911 FW ver: ") + std::to_string(touchGT911fwver), COLOR_THEME_PRIMARY1);
      lv_obj_set_grid_cell(lbl2->getLvObj(), LV_GRID_ALIGN_STRETCH, 0, 5, LV_GRID_ALIGN_CENTER, 0, 1);

#if LCD_H > LCD_W
      line = newLine(&grid);
      lbl2 = new StaticText(line, rect_t{}, "TSI2CEvents: " + std::to_string(touchGT911hiccups), COLOR_THEME_PRIMARY1);
      lv_obj_set_grid_cell(lbl2->getLvObj(), LV_GRID_ALIGN_STRETCH, 0, 5, LV_GRID_ALIGN_CENTER, 0, 1);
#else
      lbl2 = new StaticText(line, rect_t{}, "TSI2CEvents: " + std::to_string(touchGT911hiccups), COLOR_THEME_PRIMARY1);
      lv_obj_set_grid_cell(lbl2->getLvObj(), LV_GRID_ALIGN_STRETCH, 5, 5, LV_GRID_ALIGN_CENTER, 0, 1);
#endif
#endif
#endif
    }

    void checkEvents() override
    {
      Window::checkEvents();
#if defined(HARDWARE_TOUCH)
      // will always force a full monitor window refresh
      invalidate();
#endif
    }

#if defined(HARDWARE_TOUCH)
    void paint(BitmapBuffer * dc) override
    {
      TouchState rawTouchState = getInternalTouchState();
      if (rawTouchState.event != TE_NONE && rawTouchState.event != TE_SLIDE_END) {
        dc->drawLine(rawTouchState.x - 10, rawTouchState.y - 8 - 68, rawTouchState.x + 10, rawTouchState.y + 8 - 68, SOLID, 0);
        dc->drawLine(rawTouchState.x - 10, rawTouchState.y + 8 - 68, rawTouchState.x + 10, rawTouchState.y - 8 - 68, SOLID, 0);
      }
    };
#endif

  protected:
};

class AnaFilteredDevViewWindow: public FormWindow {
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

    Stats stats[NUM_STICKS+NUM_POTS+NUM_SLIDERS];

  public:
    AnaFilteredDevViewWindow(Window * parent, const rect_t & rect):
      FormWindow(parent, rect)
    {
      padAll(4);
      padLeft(10);
      padRight(10);
      setFlexLayout();
      FlexGridLayout grid(col_dsc, row_dsc, 0);

      for (uint8_t i = 0; i < NUM_STICKS + NUM_POTS + NUM_SLIDERS; i++)
        stats[i].clear();

      extern uint32_t s_anaFilt[NUM_ANALOGS];

      char s[10];
      auto line = newLine(&grid);

      for (uint8_t i = 0; i < NUM_STICKS + NUM_POTS + NUM_SLIDERS; i++) {
        lv_obj_set_style_pad_column(line->getLvObj(), 8, 0);
        sprintf(s, "%02d :", i + 1);
        new StaticText(line, rect_t{}, s, COLOR_THEME_PRIMARY1);
        auto lbl = new DynamicText(line, rect_t{}, [=]() {
          return std::to_string((int16_t)calibratedAnalogs[CONVERT_MODE(i)] * 25 / 256);
        }, COLOR_THEME_PRIMARY1);
        lv_obj_set_style_text_align(lbl->getLvObj(), LV_TEXT_ALIGN_RIGHT, 0);
        lbl = new DynamicText(line, rect_t{}, [=]() {
#if !defined(SIMU) && defined(FLYSKY_GIMBAL)
          if (globalData.flyskygimbals && (i < FLYSKY_HALL_CHANNEL_COUNT))
            return std::to_string(hall_raw_values[i]);
          else
            return std::to_string((int16_t)s_anaFilt[i]/JITTER_ALPHA);
#else
#if !defined(SIMU)
          return std::to_string((int16_t)s_anaFilt[i]/JITTER_ALPHA);
#else
          return std::to_string((int16_t)anaIn(i));
#endif
#endif
        }, COLOR_THEME_PRIMARY1);
        lv_obj_set_style_text_align(lbl->getLvObj(), LV_TEXT_ALIGN_RIGHT, 0);
        lbl = new DynamicText(line, rect_t{}, [=]() {
          return std::string("+/- ") + std::to_string(stats[i].maxDev());
        }, COLOR_THEME_PRIMARY1);
        lv_obj_set_style_text_align(lbl->getLvObj(), LV_TEXT_ALIGN_LEFT, 0);
#if LCD_W > LCD_H
        lv_obj_set_grid_cell(lbl->getLvObj(), LV_GRID_ALIGN_STRETCH, 3 + (i & 1) * 5, 2, LV_GRID_ALIGN_CENTER, 0, 1);
        grid.nextCell();
        if ((i & 1) == 1)
          line = newLine(&grid);
#else
        lv_obj_set_grid_cell(lbl->getLvObj(), LV_GRID_ALIGN_STRETCH, 3, 2, LV_GRID_ALIGN_CENTER, 0, 1);
        grid.nextCell();
        line = newLine(&grid);
#endif
      }
    }

    void checkEvents() override
    {
      for (uint8_t i = 0; i < NUM_STICKS + NUM_POTS + NUM_SLIDERS; i++) {
#if !defined(SIMU) && defined(FLYSKY_GIMBAL)
        if (globalData.flyskygimbals && (i < FLYSKY_HALL_CHANNEL_COUNT))
          stats[i].write(hall_raw_values[i]);
        else
          stats[i].write(getAnalogValue(i));
#else
        stats[i].write(getAnalogValue(i));
#endif
      }
      Window::checkEvents();
    }
};

class AnaUnfilteredRawViewWindow: public FormWindow {
  public:
    AnaUnfilteredRawViewWindow(Window * parent, const rect_t & rect):
      FormWindow(parent, rect)
    {
      padAll(4);
      padLeft(10);
      padRight(10);
      setFlexLayout();
      FlexGridLayout grid(col_dsc, row_dsc, 0);

      char s[10];
      auto line = newLine(&grid);

      for (uint8_t i = 0; i < NUM_STICKS + NUM_POTS + NUM_SLIDERS; i++) {
        lv_obj_set_style_pad_column(line->getLvObj(), 8, 0);
        sprintf(s, "%02d :", i + 1);
        new StaticText(line, rect_t{}, s, COLOR_THEME_PRIMARY1);
        auto lbl = new DynamicText(line, rect_t{}, [=]() {
          return std::to_string((int16_t)calibratedAnalogs[CONVERT_MODE(i)] * 25 / 256);
        }, COLOR_THEME_PRIMARY1);
        lv_obj_set_style_text_align(lbl->getLvObj(), LV_TEXT_ALIGN_RIGHT, 0);
        lbl = new DynamicText(line, rect_t{}, [=]() {
#if !defined(SIMU) && defined(FLYSKY_GIMBAL)
          if (globalData.flyskygimbals && (i < FLYSKY_HALL_CHANNEL_COUNT))
            return std::to_string(hall_raw_values[i]);
          else
            return std::to_string((int16_t)getAnalogValue(i));
#else
          return std::to_string((int16_t)getAnalogValue(i));
#endif
        }, COLOR_THEME_PRIMARY1);
        lv_obj_set_style_text_align(lbl->getLvObj(), LV_TEXT_ALIGN_RIGHT, 0);
        grid.nextCell();
        grid.nextCell();
#if LCD_W > LCD_H
        if ((i & 1) == 1)
          line = newLine(&grid);
#else
        line = newLine(&grid);
#endif
      }
    }
};

class AnaMinMaxViewWindow: public FormWindow {
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

    MinMax minmax[NUM_STICKS+NUM_POTS+NUM_SLIDERS];

  public:
    AnaMinMaxViewWindow(Window * parent, const rect_t & rect):
      FormWindow(parent, rect)
    {
      padAll(4);
      padLeft(10);
      padRight(10);
      setFlexLayout();
      FlexGridLayout grid(col_dsc, row_dsc, 0);

      for (uint8_t i = 0; i < NUM_STICKS + NUM_POTS + NUM_SLIDERS; i++)
        minmax[i].clear();

      char s[10];
      auto line = newLine(&grid);
      auto ttl = new StaticText(line, rect_t{}, STR_ANADIAGS_MOVE, COLOR_THEME_PRIMARY1);
      lv_obj_set_grid_cell(ttl->getLvObj(), LV_GRID_ALIGN_STRETCH, 0, 6, LV_GRID_ALIGN_CENTER, 0, 1);

      for (uint8_t i = 0; i < NUM_STICKS + NUM_POTS + NUM_SLIDERS; i++) {
#if LCD_W > LCD_H
        if ((i & 1) == 0)
          line = newLine(&grid);
#else
        line = newLine(&grid);
#endif
        lv_obj_set_style_pad_column(line->getLvObj(), 8, 0);
        sprintf(s, "%02d :", i + 1);
        new StaticText(line, rect_t{}, s, COLOR_THEME_PRIMARY1);
        auto lbl = new DynamicText(line, rect_t{}, [=]() {
          return std::to_string((int16_t)calibratedAnalogs[CONVERT_MODE(i)] * 25 / 256);
        }, COLOR_THEME_PRIMARY1);
        lv_obj_set_style_text_align(lbl->getLvObj(), LV_TEXT_ALIGN_RIGHT, 0);
        lbl = new DynamicText(line, rect_t{}, [=]() {
          return std::to_string((int16_t)minmax[i].MinVal());
        }, COLOR_THEME_PRIMARY1);
        lv_obj_set_style_text_align(lbl->getLvObj(), LV_TEXT_ALIGN_RIGHT, 0);
        lbl = new DynamicText(line, rect_t{}, [=]() {
          return std::to_string((int16_t)minmax[i].MaxVal());
        }, COLOR_THEME_PRIMARY1);
        lv_obj_set_style_text_align(lbl->getLvObj(), LV_TEXT_ALIGN_RIGHT, 0);
        lbl = new DynamicText(line, rect_t{}, [=]() {
          return std::to_string((int16_t)minmax[i].Range());
        }, COLOR_THEME_PRIMARY1);
        lv_obj_set_style_text_align(lbl->getLvObj(), LV_TEXT_ALIGN_LEFT, 0);
      }
    }

    void checkEvents() override
    {
      for (uint8_t i = 0; i < NUM_STICKS + NUM_POTS + NUM_SLIDERS; i++) {
#if !defined(SIMU) && defined(FLYSKY_GIMBAL)
        if (globalData.flyskygimbals && (i < FLYSKY_HALL_CHANNEL_COUNT))
          minmax[i].write(hall_raw_values[i]);
        else
          minmax[i].write(getAnalogValue(i));
#else
        minmax[i].write(getAnalogValue(i));
#endif
      }
      Window::checkEvents();
    }
};

class AnaCalibratedViewPage : public PageTab
{
 public:
  AnaCalibratedViewPage() : PageTab(STR_ANADIAGS_CALIB, ICON_STATS_ANALOGS) {}

 protected:
  void build(FormWindow* window) override {
    window->padAll(0);
    new AnaCalibratedViewWindow(window, {0, 0, window->width(), window->height()});
  }
};

class AnaFilteredDevViewPage : public PageTab
{
 public:
  AnaFilteredDevViewPage() : PageTab(STR_ANADIAGS_FILTRAWDEV, ICON_STATS_THROTTLE_GRAPH) {}

 protected:
  void build(FormWindow* window) override {
    window->padAll(0);
    new AnaFilteredDevViewWindow(window, {0, 0, window->width(), window->height()});
  }
};

class AnaUnfilteredRawViewPage : public PageTab
{
 public:
  AnaUnfilteredRawViewPage() : PageTab(STR_ANADIAGS_UNFILTRAW, ICON_RADIO_HARDWARE) {}

 protected:
  void build(FormWindow* window) override {
    window->padAll(0);
    new AnaUnfilteredRawViewWindow(window, {0, 0, window->width(), window->height()});
  }
};

class AnaMinMaxViewPage : public PageTab
{
 public:
  AnaMinMaxViewPage() : PageTab(STR_ANADIAGS_MINMAX, ICON_RADIO_CALIBRATION) {}

 protected:
  void build(FormWindow* window) override {
    window->padAll(0);
    new AnaMinMaxViewWindow(window, {0, 0, window->width(), window->height()});
  }
};

RadioAnalogsDiagsViewPageGroup::RadioAnalogsDiagsViewPageGroup() : TabsGroup(ICON_STATS)
{
  addTab(new AnaCalibratedViewPage());
  addTab(new AnaFilteredDevViewPage());
  addTab(new AnaUnfilteredRawViewPage());
  addTab(new AnaMinMaxViewPage());
}
