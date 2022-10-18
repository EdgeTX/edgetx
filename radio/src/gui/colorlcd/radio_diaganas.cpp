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
#include "hal/adc_driver.h"

// #if defined(IMU_LSM6DS33)
// #include "imu_lsm6ds33.h"
// #endif

#define STATSDEPTH 8 // ideally a value of power of 2

#if LCD_W > LCD_H

#define GRIDCOLS 10
#define TSI2CEventsCol 5

static const lv_coord_t col_dsc[] = {LV_GRID_FR(30), LV_GRID_FR(30), LV_GRID_FR(40), LV_GRID_FR(40), LV_GRID_FR(40),
                                     LV_GRID_FR(30), LV_GRID_FR(30), LV_GRID_FR(40), LV_GRID_FR(40), LV_GRID_FR(40),
                                     LV_GRID_TEMPLATE_LAST};

#else

#define GRIDCOLS 5
#define TSI2CEventsCol 0

static const lv_coord_t col_dsc[] = {LV_GRID_FR(30), LV_GRID_FR(30), LV_GRID_FR(40), LV_GRID_FR(40), LV_GRID_FR(40),
                                     LV_GRID_TEMPLATE_LAST};

#endif

static const lv_coord_t row_dsc[] = {LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};

class AnaViewWindow: public FormWindow {
  public:
    AnaViewWindow(Window * parent):
      FormWindow(parent, {0, 0, parent->width(), parent->height()})
    {
      parent->padAll(0);
      padAll(4);
      padLeft(10);
      padRight(10);
      setFlexLayout();

      grid = new FlexGridLayout(col_dsc, row_dsc, 0);
      line = newLine(grid);
    }
    
    virtual void build()
    {
      char s[10];

      auto max_inputs = adcGetMaxInputs(ADC_INPUT_MAIN)
        + adcGetMaxInputs(ADC_INPUT_POT);

      for (uint8_t i = 0; i < max_inputs; i++) {
#if LCD_W > LCD_H
        if ((i & 1) == 0)
          line = newLine(grid);
#else
        line = newLine(grid);
#endif

        lv_obj_set_style_pad_column(line->getLvObj(), 8, 0);

        sprintf(s, "%02d :", i + 1);
        new StaticText(line, rect_t{}, s, COLOR_THEME_PRIMARY1);

        auto lbl = new DynamicText(line, rect_t{}, [=]() {
          return std::to_string((int16_t)calibratedAnalogs[i] * 25 / 256);
        }, COLOR_THEME_PRIMARY1);
        lv_obj_set_style_text_align(lbl->getLvObj(), LV_TEXT_ALIGN_RIGHT, 0);

        lbl = new DynamicText(line, rect_t{}, [=]() {
          return std::to_string((int16_t)column3(i));
        }, COLOR_THEME_PRIMARY1);
        lv_obj_set_style_text_align(lbl->getLvObj(), LV_TEXT_ALIGN_RIGHT, 0);

        if (column4size() > 0) {
          lbl = new DynamicText(line, rect_t{}, [=]() {
            return std::string(column4prefix()) + std::to_string((int16_t)column4(i));
          }, COLOR_THEME_PRIMARY1);
          lv_obj_set_style_text_align(lbl->getLvObj(), (column4size() == 2) ? LV_TEXT_ALIGN_LEFT : LV_TEXT_ALIGN_RIGHT, 0);
#if LCD_W > LCD_H
          lv_obj_set_grid_cell(lbl->getLvObj(), LV_GRID_ALIGN_STRETCH, 3 + (i & 1) * 5, column4size(), LV_GRID_ALIGN_CENTER, 0, 1);
#else
          lv_obj_set_grid_cell(lbl->getLvObj(), LV_GRID_ALIGN_STRETCH, 3, column4size(), LV_GRID_ALIGN_CENTER, 0, 1);
#endif
        } else {
          grid->nextCell();
        }
        
        if (column5size() > 0) {
          lbl = new DynamicText(line, rect_t{}, [=]() {
            return std::to_string((int16_t)column5(i));
          }, COLOR_THEME_PRIMARY1);
          lv_obj_set_style_text_align(lbl->getLvObj(), LV_TEXT_ALIGN_LEFT, 0);
        } else {
          grid->nextCell();
        }
      }
    }

    void deleteLater(bool detach = true, bool trash = true) override
    {
      if (grid){
        delete grid;
        grid = nullptr;
      }
    }
    
    void checkEvents() override
    {
      Window::checkEvents();
    }

  protected:
    FlexGridLayout* grid = nullptr;
    FormWindow::Line* line = nullptr;

    virtual int16_t column3(int i) { return 0; }
    virtual int16_t column4(int i) { return 0; }
    virtual int column4size() { return 0; }
    virtual const char* column4prefix() { return ""; }
    virtual int16_t column5(int i) { return 0; }
    virtual int column5size() { return 0; }
};

class AnaCalibratedViewWindow: public AnaViewWindow {
  public:
    AnaCalibratedViewWindow(Window * parent):
      AnaViewWindow(parent)
    {
    }
    
    void build() override
    {
      AnaViewWindow::build();

#if defined(HARDWARE_TOUCH)
      line = newLine(grid);
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
      line = newLine(grid);
      auto lbl2 = new StaticText(line, rect_t{}, std::string("Touch GT911 FW ver: ") + std::to_string(touchGT911fwver), COLOR_THEME_PRIMARY1);
      lv_obj_set_grid_cell(lbl2->getLvObj(), LV_GRID_ALIGN_STRETCH, 0, 5, LV_GRID_ALIGN_CENTER, 0, 1);

#if LCD_H > LCD_W
      line = newLine(grid);
#endif
      lbl2 = new StaticText(line, rect_t{}, "TSI2CEvents: " + std::to_string(touchGT911hiccups), COLOR_THEME_PRIMARY1);
      lv_obj_set_grid_cell(lbl2->getLvObj(), LV_GRID_ALIGN_STRETCH, TSI2CEventsCol, 5, LV_GRID_ALIGN_CENTER, 0, 1);
#endif
#endif // defined(HARDWARE_TOUCH)
    }

#if defined(HARDWARE_TOUCH)
    void checkEvents() override
    {
      AnaViewWindow::checkEvents();
      // will always force a full monitor window refresh
      invalidate();
    }

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
    int16_t column3(int i) override
    {
      return anaIn(i);
    }
};

class AnaFilteredDevViewWindow: public AnaViewWindow {
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

    Stats stats[MAX_CALIB_ANALOG_INPUTS];

    int16_t column3(int i) override
    {
      return anaIn_diag(i);
    }

    const char* column4prefix() override { return "+/- "; }
    int column4size() override { return 2; }

    int16_t column4(int i) override
    {
      return stats[i].maxDev();
    }

  public:
    AnaFilteredDevViewWindow(Window * parent):
      AnaViewWindow(parent)
    {
      auto max_inputs = adcGetMaxInputs(ADC_INPUT_MAIN)
        + adcGetMaxInputs(ADC_INPUT_POT);

      for (uint8_t i = 0; i < max_inputs; i++)
        stats[i].clear();
    }

    void checkEvents() override
    {
      auto max_inputs = adcGetMaxInputs(ADC_INPUT_MAIN)
        + adcGetMaxInputs(ADC_INPUT_POT);

      for (uint8_t i = 0; i < max_inputs; i++) {
        stats[i].write(getAnalogValue(i));
      }
      AnaViewWindow::checkEvents();
    }
};

class AnaUnfilteredRawViewWindow: public AnaViewWindow {
  public:
    AnaUnfilteredRawViewWindow(Window * parent):
      AnaViewWindow(parent)
    {
    }

  protected:
    int16_t column3(int i) override
    {
      return getAnalogValue(i);
    }
};

class AnaMinMaxViewWindow: public AnaViewWindow {
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

    MinMax minmax[MAX_CALIB_ANALOG_INPUTS];

    int16_t column3(int i) override
    {
      return minmax[i].MinVal();
    }

    int column4size() override { return 1; }

    int16_t column4(int i) override
    {
      return minmax[i].MaxVal();
    }

    int column5size() override { return 1; }

    int16_t column5(int i) override
    {
      return minmax[i].Range();
    }

  public:
    AnaMinMaxViewWindow(Window * parent):
      AnaViewWindow(parent)
    {
      auto max_inputs = adcGetMaxInputs(ADC_INPUT_MAIN)
        + adcGetMaxInputs(ADC_INPUT_POT);
      
      for (uint8_t i = 0; i < max_inputs; i++)
        minmax[i].clear();
    }
    
    void build() override
    {
      line = newLine(grid);
      auto ttl = new StaticText(line, rect_t{}, STR_ANADIAGS_MOVE, COLOR_THEME_PRIMARY1);
      lv_obj_set_grid_cell(ttl->getLvObj(), LV_GRID_ALIGN_STRETCH, 0, GRIDCOLS, LV_GRID_ALIGN_CENTER, 0, 1);

      AnaViewWindow::build();
    }

    void checkEvents() override
    {
      auto max_inputs = adcGetMaxInputs(ADC_INPUT_MAIN)
        + adcGetMaxInputs(ADC_INPUT_POT);
      
      for (uint8_t i = 0; i < max_inputs; i++) {
        minmax[i].write(getAnalogValue(i));
      }
      AnaViewWindow::checkEvents();
    }
};

class AnaCalibratedViewPage : public PageTab
{
 public:
  AnaCalibratedViewPage() : PageTab(STR_ANADIAGS_CALIB, ICON_STATS_ANALOGS) {}

 protected:
  void build(FormWindow* window) override {
    (new AnaCalibratedViewWindow(window))->build();
  }
};

class AnaFilteredDevViewPage : public PageTab
{
 public:
  AnaFilteredDevViewPage() : PageTab(STR_ANADIAGS_FILTRAWDEV, ICON_STATS_THROTTLE_GRAPH) {}

 protected:
  void build(FormWindow* window) override {
    (new AnaFilteredDevViewWindow(window))->build();
  }
};

class AnaUnfilteredRawViewPage : public PageTab
{
 public:
  AnaUnfilteredRawViewPage() : PageTab(STR_ANADIAGS_UNFILTRAW, ICON_RADIO_HARDWARE) {}

 protected:
  void build(FormWindow* window) override {
    (new AnaUnfilteredRawViewWindow(window))->build();
  }
};

class AnaMinMaxViewPage : public PageTab
{
 public:
  AnaMinMaxViewPage() : PageTab(STR_ANADIAGS_MINMAX, ICON_RADIO_CALIBRATION) {}

 protected:
  void build(FormWindow* window) override {
    (new AnaMinMaxViewWindow(window))->build();
  }
};

RadioAnalogsDiagsViewPageGroup::RadioAnalogsDiagsViewPageGroup() : TabsGroup(ICON_STATS)
{
  addTab(new AnaCalibratedViewPage());
  addTab(new AnaFilteredDevViewPage());
  addTab(new AnaUnfilteredRawViewPage());
  addTab(new AnaMinMaxViewPage());
}
