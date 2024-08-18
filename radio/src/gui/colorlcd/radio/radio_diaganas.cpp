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

#include "radio_diaganas.h"

#include "hal/adc_driver.h"
#include "libopenui.h"
#include "edgetx.h"
#include "etx_lv_theme.h"

// #if defined(IMU_LSM6DS33)
// #include "imu_lsm6ds33.h"
// #endif

#define STATSDEPTH 8  // ideally a value of power of 2

#if !PORTRAIT_LCD

static const lv_coord_t col_dsc[] = {
    LV_GRID_FR(30), LV_GRID_FR(30), LV_GRID_FR(40),       LV_GRID_FR(40),
    LV_GRID_FR(40), LV_GRID_FR(30), LV_GRID_FR(30),       LV_GRID_FR(40),
    LV_GRID_FR(40), LV_GRID_FR(40), LV_GRID_TEMPLATE_LAST};

#else

static const lv_coord_t col_dsc[] = {LV_GRID_FR(30), LV_GRID_FR(30),
                                     LV_GRID_FR(40), LV_GRID_FR(40),
                                     LV_GRID_FR(40), LV_GRID_TEMPLATE_LAST};

#endif

static const lv_coord_t row_dsc[] = {LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};

class AnaViewWindow : public Window
{
 public:
  AnaViewWindow(Window* parent) :
      Window(parent, {0, 0, parent->width(), parent->height()}),
      grid(col_dsc, row_dsc, PAD_ZERO)
  {
    parent->padAll(PAD_ZERO);
    padAll(PAD_SMALL);
    padLeft(PAD_LARGE);
    padRight(PAD_LARGE);
    setFlexLayout();

    line = newLine(grid);
  }

  virtual void build()
  {
    char s[10];

    auto pot_offset = adcGetInputOffset(ADC_INPUT_FLEX);
    auto max_inputs =
        adcGetMaxInputs(ADC_INPUT_MAIN) + adcGetMaxInputs(ADC_INPUT_FLEX);

    for (uint8_t i = 0; i < max_inputs; i++) {
      if (i >= pot_offset && (POT_CONFIG(i - pot_offset) == FLEX_NONE))
        continue;

#if !PORTRAIT_LCD
      if ((i & 1) == 0) line = newLine(grid);
#else
      line = newLine(grid);
#endif

      lv_obj_set_style_pad_column(line->getLvObj(), 8, 0);
      if (((adcGetInputMask() & (1 << i)) != 0) && i < adcGetMaxInputs(ADC_INPUT_MAIN))
        sprintf(s, "D%d :", i + 1);
      else
        sprintf(s, "%02d :", i + 1);

      new StaticText(line, rect_t{}, s);

      auto lbl = new DynamicText(
          line, rect_t{},
          [=]() {
            return std::to_string((int16_t)calibratedAnalogs[i] * 25 / 256);
          });
      etx_obj_add_style(lbl->getLvObj(), styles->text_align_right, LV_PART_MAIN);

      lbl = new DynamicText(
          line, rect_t{}, [=]() { return std::to_string((int16_t)column3(i)); });
      etx_obj_add_style(lbl->getLvObj(), styles->text_align_right, LV_PART_MAIN);

      if (column4size() > 0) {
        lbl = new DynamicText(
            line, rect_t{},
            [=]() {
              return std::string(column4prefix()) +
                     std::to_string((int16_t)column4(i));
            });
        etx_obj_add_style(lbl->getLvObj(), (column4size() == 2) ? styles->text_align_left : styles->text_align_right, LV_PART_MAIN);
#if !PORTRAIT_LCD
        lv_obj_set_grid_cell(lbl->getLvObj(), LV_GRID_ALIGN_STRETCH,
                             3 + (i & 1) * 5, column4size(),
                             LV_GRID_ALIGN_CENTER, 0, 1);
#else
        lv_obj_set_grid_cell(lbl->getLvObj(), LV_GRID_ALIGN_STRETCH, 3,
                             column4size(), LV_GRID_ALIGN_CENTER, 0, 1);
#endif
      } else {
        grid.nextCell();
      }

      if (column5size() > 0) {
        lbl = new DynamicText(
            line, rect_t{},
            [=]() { return std::to_string((int16_t)column5(i)); });
        etx_obj_add_style(lbl->getLvObj(), styles->text_align_left, LV_PART_MAIN);
      } else {
        grid.nextCell();
      }
    }
  }

  void checkEvents() override { Window::checkEvents(); }

 protected:
  FlexGridLayout grid;
  FormLine* line = nullptr;

  virtual int16_t column3(int i) { return 0; }
  virtual int16_t column4(int i) { return 0; }
  virtual int column4size() { return 0; }
  virtual const char* column4prefix() { return ""; }
  virtual int16_t column5(int i) { return 0; }
  virtual int column5size() { return 0; }
};

class AnaCalibratedViewWindow : public AnaViewWindow
{
 public:
  AnaCalibratedViewWindow(Window* parent) : AnaViewWindow(parent) {}

  void build() override
  {
    AnaViewWindow::build();

#if defined(HARDWARE_TOUCH)
    touchLines[0] = lv_line_create(parent->getParent()->getLvObj());
    etx_obj_add_style(touchLines[0], styles->div_line_edit, LV_PART_MAIN);
    lv_obj_add_flag(touchLines[0], LV_OBJ_FLAG_HIDDEN);
    touchLines[1] = lv_line_create(parent->getParent()->getLvObj());
    etx_obj_add_style(touchLines[1], styles->div_line_edit, LV_PART_MAIN);
    lv_obj_add_flag(touchLines[1], LV_OBJ_FLAG_HIDDEN);

    line = newLine(grid);
#if PORTRAIT_LCD
    line->padTop(20);
#else
    line->padTop(PAD_TINY);
#endif

    auto lbl = new DynamicText(
        line, rect_t{},
        [=]() {
          TouchState rawTouchState = getInternalTouchState();
          if (rawTouchState.event != TE_NONE &&
              rawTouchState.event != TE_SLIDE_END)
            return std::string(STR_TOUCH_PANEL) + " " +
                   std::to_string(rawTouchState.x) + " : " +
                   std::to_string(rawTouchState.y);
          return std::string("");
        });
    lv_obj_set_grid_cell(lbl->getLvObj(), LV_GRID_ALIGN_STRETCH, 0, 5,
                         LV_GRID_ALIGN_CENTER, 0, 1);

#if !defined(SIMU) && defined(TP_GT911)
    line = newLine(grid);
    auto lbl2 = new StaticText(
        line, rect_t{},
        std::string("Touch GT911 FW ver: ") + std::to_string(touchGT911fwver),
        COLOR_THEME_PRIMARY1_INDEX);
    lv_obj_set_grid_cell(lbl2->getLvObj(), LV_GRID_ALIGN_STRETCH, 0, 5,
                         LV_GRID_ALIGN_CENTER, 0, 1);

#if PORTRAIT_LCD
    line = newLine(grid);
#endif
    lbl2 = new StaticText(line, rect_t{},
                          "TSI2CEvents: " + std::to_string(touchGT911hiccups));
    lv_obj_set_grid_cell(lbl2->getLvObj(), LV_GRID_ALIGN_STRETCH,
                         TSI2CEventsCol, 5, LV_GRID_ALIGN_CENTER, 0, 1);
#endif
#endif  // defined(HARDWARE_TOUCH)

    setHeight(parent->height());
  }

#if defined(HARDWARE_TOUCH)
  void checkEvents() override
  {
    AnaViewWindow::checkEvents();

    TouchState rawTouchState = getInternalTouchState();
    if (rawTouchState.event != TE_NONE && rawTouchState.event != TE_SLIDE_END) {
      touchPts[0] = {(lv_coord_t)(rawTouchState.x - 10), (lv_coord_t)(rawTouchState.y - 8)};
      touchPts[1] = {(lv_coord_t)(rawTouchState.x + 10), (lv_coord_t)(rawTouchState.y + 8)};
      touchPts[2] = {(lv_coord_t)(rawTouchState.x - 10), (lv_coord_t)(rawTouchState.y + 8)};
      touchPts[3] = {(lv_coord_t)(rawTouchState.x + 10), (lv_coord_t)(rawTouchState.y - 8)};

      lv_line_set_points(touchLines[0], &touchPts[0], 2);
      lv_line_set_points(touchLines[1], &touchPts[2], 2);
      lv_obj_clear_flag(touchLines[0], LV_OBJ_FLAG_HIDDEN);
      lv_obj_clear_flag(touchLines[1], LV_OBJ_FLAG_HIDDEN);
    } else {
      lv_obj_add_flag(touchLines[0], LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(touchLines[1], LV_OBJ_FLAG_HIDDEN);
    }
  }

  void deleteLater(bool detach, bool trash) override
  {
    if (!deleted()) {
      // Attached to parent->parent window
      lv_obj_del(touchLines[0]);
      lv_obj_del(touchLines[1]);
      AnaViewWindow::deleteLater(detach, trash);
    }
  }
#endif

  static LAYOUT_VAL(TSI2CEventsCol, 5, 0)

 protected:
#if defined(HARDWARE_TOUCH)
  lv_point_t touchPts[4];
  lv_obj_t* touchLines[2];
#endif
  int16_t column3(int i) override { return anaIn(i); }
};

class AnaFilteredDevViewWindow : public AnaViewWindow
{
 protected:
  class Stats
  {
   protected:
    int16_t buffer[STATSDEPTH] = {0};
    uint8_t writePos = 0;
    uint8_t filledElems = 0;

    int16_t intSqrt(int16_t val)
    {
      if (val < 0) return 0;
      if (val <= 1) return val;

      // Try iteratively until i^2 >= val
      int i = 1, retval = 1;
      while (retval <= val) {
        i++;
        retval = i * i;
      }
      return i - 1;
    }
    int16_t meanVal()
    {
      if (filledElems) {
        int sum = 0;
        for (uint8_t i = 0; i < filledElems; i++) sum += buffer[i];
        return (int16_t)(sum / filledElems);
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
      if (filledElems < STATSDEPTH) filledElems++;
    }

    uint16_t maxDev()
    {
      if (filledElems) {
        uint16_t ret = 0;
        for (uint8_t i = 0; i < filledElems; i++)
          ret = max<uint16_t>(ret, abs(buffer[i] - meanVal()));
        return ret;
      } else
        return 0;
    }

    int16_t stdDev()
    {
      if (filledElems) {
        int16_t mean = meanVal();
        int calc = 0;
        for (uint8_t i = 0; i < filledElems; i++)
          calc += (buffer[i] - mean) * (buffer[i] - mean);
        calc = intSqrt((int16_t)(calc / filledElems));
        return (int16_t)calc;
      } else
        return 0;
    }
  };

  Stats stats[MAX_CALIB_ANALOG_INPUTS];

  int16_t column3(int i) override { return anaIn_diag(i); }

  const char* column4prefix() override { return "+/- "; }
  int column4size() override { return 2; }

  int16_t column4(int i) override { return stats[i].maxDev(); }

 public:
  AnaFilteredDevViewWindow(Window* parent) : AnaViewWindow(parent)
  {
    auto max_inputs =
        adcGetMaxInputs(ADC_INPUT_MAIN) + adcGetMaxInputs(ADC_INPUT_FLEX);

    for (uint8_t i = 0; i < max_inputs; i++) stats[i].clear();
  }

  void checkEvents() override
  {
    auto max_inputs =
        adcGetMaxInputs(ADC_INPUT_MAIN) + adcGetMaxInputs(ADC_INPUT_FLEX);

    for (uint8_t i = 0; i < max_inputs; i++) {
      stats[i].write(getAnalogValue(i));
    }
    AnaViewWindow::checkEvents();
  }
};

class AnaUnfilteredRawViewWindow : public AnaViewWindow
{
 public:
  AnaUnfilteredRawViewWindow(Window* parent) : AnaViewWindow(parent) {}

 protected:
  int16_t column3(int i) override { return getAnalogValue(i); }
};

class AnaMinMaxViewWindow : public AnaViewWindow
{
 protected:
  class MinMax
  {
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
      if (value < minvalue) minvalue = value;
      if (value > maxvalue) maxvalue = value;
    }

    int16_t MinVal() { return minvalue; }

    int16_t MaxVal() { return maxvalue; }

    uint16_t Range()
    {
      if (maxvalue > minvalue)
        return (uint16_t)(maxvalue - minvalue);
      else
        return 0;
    }
  };

  MinMax minmax[MAX_CALIB_ANALOG_INPUTS];

  int16_t column3(int i) override { return minmax[i].MinVal(); }

  int column4size() override { return 1; }

  int16_t column4(int i) override { return minmax[i].MaxVal(); }

  int column5size() override { return 1; }

  int16_t column5(int i) override { return minmax[i].Range(); }

 public:
  AnaMinMaxViewWindow(Window* parent) : AnaViewWindow(parent)
  {
    auto max_inputs =
        adcGetMaxInputs(ADC_INPUT_MAIN) + adcGetMaxInputs(ADC_INPUT_FLEX);

    for (uint8_t i = 0; i < max_inputs; i++) minmax[i].clear();
  }

  void build() override
  {
    line = newLine(grid);
    auto ttl =
        new StaticText(line, rect_t{}, STR_ANADIAGS_MOVE);
    lv_obj_set_grid_cell(ttl->getLvObj(), LV_GRID_ALIGN_STRETCH, 0, GRIDCOLS,
                         LV_GRID_ALIGN_CENTER, 0, 1);

    AnaViewWindow::build();
  }

  void checkEvents() override
  {
    auto max_inputs =
        adcGetMaxInputs(ADC_INPUT_MAIN) + adcGetMaxInputs(ADC_INPUT_FLEX);

    for (uint8_t i = 0; i < max_inputs; i++) {
      minmax[i].write(getAnalogValue(i));
    }
    AnaViewWindow::checkEvents();
  }

  static LAYOUT_VAL(GRIDCOLS, 10, 5)
};

class AnaCalibratedViewPage : public PageTab
{
 public:
  AnaCalibratedViewPage() : PageTab(STR_ANADIAGS_CALIB, ICON_STATS_ANALOGS) {}

 protected:
  void build(Window* window) override
  {
    (new AnaCalibratedViewWindow(window))->build();
  }
};

class AnaFilteredDevViewPage : public PageTab
{
 public:
  AnaFilteredDevViewPage() :
      PageTab(STR_ANADIAGS_FILTRAWDEV, ICON_STATS_THROTTLE_GRAPH)
  {
  }

 protected:
  void build(Window* window) override
  {
    (new AnaFilteredDevViewWindow(window))->build();
  }
};

class AnaUnfilteredRawViewPage : public PageTab
{
 public:
  AnaUnfilteredRawViewPage() :
      PageTab(STR_ANADIAGS_UNFILTRAW, ICON_RADIO_HARDWARE)
  {
  }

 protected:
  void build(Window* window) override
  {
    (new AnaUnfilteredRawViewWindow(window))->build();
  }
};

class AnaMinMaxViewPage : public PageTab
{
 public:
  AnaMinMaxViewPage() : PageTab(STR_ANADIAGS_MINMAX, ICON_RADIO_CALIBRATION) {}

 protected:
  void build(Window* window) override
  {
    (new AnaMinMaxViewWindow(window))->build();
  }
};

RadioAnalogsDiagsViewPageGroup::RadioAnalogsDiagsViewPageGroup() :
    TabsGroup(ICON_STATS)
{
  addTab(new AnaCalibratedViewPage());
  addTab(new AnaFilteredDevViewPage());
  addTab(new AnaUnfilteredRawViewPage());
  addTab(new AnaMinMaxViewPage());
}
