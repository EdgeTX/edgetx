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

#include "radio_hardware.h"

#include "hal/adc_driver.h"
#include "hw_extmodule.h"
#include "hw_inputs.h"
#include "hw_intmodule.h"
#include "hw_serial.h"
#include "libopenui.h"
#include "edgetx.h"
#include "radio_calibration.h"
#include "radio_diaganas.h"
#include "radio_diagkeys.h"
#include "radio_setup.h"

#if defined(FUNCTION_SWITCHES)
#include "radio_diagcustswitches.h"
#endif

#if defined(BLUETOOTH)
#include "hw_bluetooth.h"
#endif

#define SET_DIRTY() storageDirty(EE_GENERAL)

#if PORTRAIT_LCD
static const lv_coord_t col_dsc[] = {LV_GRID_FR(13), LV_GRID_FR(19),
                                     LV_GRID_TEMPLATE_LAST};
static const lv_coord_t row_dsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT,
                                     LV_GRID_TEMPLATE_LAST};
#else
static const lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(2),
                                     LV_GRID_TEMPLATE_LAST};
static const lv_coord_t row_dsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT,
                                     LV_GRID_TEMPLATE_LAST};
#endif

RadioHardwarePage::RadioHardwarePage() :
    PageTab(STR_HARDWARE, ICON_RADIO_HARDWARE, PAD_TINY)
{
  enableVBatBridge();
}

void RadioHardwarePage::checkEvents() { enableVBatBridge(); }

void RadioHardwarePage::cleanup()
{
  disableVBatBridge();
}

class BatCalEdit : public NumberEdit
{
 public:
  BatCalEdit(Window* parent, const rect_t& rect) :
      NumberEdit(parent, rect, -127, 127,
                 GET_SET_DEFAULT(g_eeGeneral.txVoltageCalibration))
  {
    setDisplayHandler([](int32_t value) {
      return formatNumberAsString(getBatteryVoltage(), PREC2, 0, nullptr, "V");
    });
    lastBatVolts = getBatteryVoltage();
  }

 protected:
  uint16_t lastBatVolts = 0;

  void checkEvents() override
  {
    if (getBatteryVoltage() != lastBatVolts) {
      lastBatVolts = getBatteryVoltage();
      invalidate();
    }
  }
};

static SetupLineDef setupLines[] = {
  {
    // Batt meter range - Range 3.0v to 16v
    STR_BATTERY_RANGE,
    [](Window* parent, coord_t x, coord_t y) {
      auto batMin = new NumberEdit(
          parent, {x, y, RadioHardwarePage::NUM_EDIT_W, 0}, -60 + 90, g_eeGeneral.vBatMax + 29 + 90,
          GET_SET_WITH_OFFSET(g_eeGeneral.vBatMin, 90), PREC1);
      batMin->setSuffix("V");
      new StaticText(parent, {x + RadioHardwarePage::NUM_EDIT_W + PAD_SMALL, y + PAD_SMALL + 1, PAD_LARGE, EdgeTxStyles::PAGE_LINE_HEIGHT}, "-");
      auto batMax = new NumberEdit(
          parent, {x + RadioHardwarePage::NUM_EDIT_W + PAD_LARGE + PAD_SMALL, y, RadioHardwarePage::NUM_EDIT_W, 0}, g_eeGeneral.vBatMin - 29 + 120, 40 + 120,
          GET_SET_WITH_OFFSET(g_eeGeneral.vBatMax, 120), PREC1);
      batMax->setSuffix("V");

      batMin->setSetValueHandler([=](int32_t newValue) {
        g_eeGeneral.vBatMin = newValue - 90;
        SET_DIRTY();
        batMax->setMin(g_eeGeneral.vBatMin - 29 + 120);
      });

      batMax->setSetValueHandler([=](int32_t newValue) {
        g_eeGeneral.vBatMax = newValue - 120;
        SET_DIRTY();
        batMin->setMax(g_eeGeneral.vBatMax + 29 + 90);
      });
    }
  },
  {
    // Bat calibration
    STR_BATT_CALIB,
    [](Window* parent, coord_t x, coord_t y) {
      new BatCalEdit(parent, {x, y, RadioHardwarePage::NUM_EDIT_W, 0});
    }
  },
  {
    // RTC Batt check enable
    STR_RTC_CHECK,
    [](Window* parent, coord_t x, coord_t y) {
      new ToggleSwitch(parent, {x, y, 0, 0},
                       GET_SET_INVERTED(g_eeGeneral.disableRtcWarning));

      // RTC Batt display
      new DynamicNumber<uint16_t>(
          parent,
          {x + ToggleSwitch::TOGGLE_W + PAD_SMALL, y + PAD_SMALL + 1, 0, 0},
          [] { return getRTCBatteryVoltage(); }, COLOR_THEME_PRIMARY1_INDEX, PREC2,
          "", "V");
    }
  },
  {
    // ADC filter
    STR_JITTER_FILTER,
    [](Window* parent, coord_t x, coord_t y) {
      new ToggleSwitch(parent, {x, y, 0, 0}, GET_SET_INVERTED(g_eeGeneral.noJitterFilter));
    }
  },
#if defined(AUDIO_MUTE_GPIO)
  {
    // Mute audio
    STR_AUDIO_MUTE,
    [](Window* parent, coord_t x, coord_t y) {
      new ToggleSwitch(parent, {x, y, 0, 0}, GET_SET_DEFAULT(g_eeGeneral.audioMuteEnable));
    }
  },
#endif
};

void RadioHardwarePage::build(Window* window)
{
  window->setFlexLayout(LV_FLEX_FLOW_COLUMN, PAD_TINY);

  SetupLine::showLines(window, 0, SubPage::EDT_X, padding, setupLines, DIM(setupLines));

  FlexGridLayout grid(col_dsc, row_dsc, PAD_TINY);

  FormLine* line;

#if defined(HARDWARE_INTERNAL_MODULE)
  new Subtitle(window, STR_INTERNALRF);
  line = window->newLine(grid);
  line->padLeft(PAD_SMALL);
  new InternalModuleWindow(line, grid);
#endif

#if defined(HARDWARE_EXTERNAL_MODULE)
  new Subtitle(window, STR_EXTERNALRF);
  line = window->newLine(grid);
  line->padLeft(PAD_SMALL);
  new ExternalModuleWindow(line, grid);
#endif

#if defined(BLUETOOTH)
  new Subtitle(window, STR_BLUETOOTH);
  line = window->newLine(grid);
  line->padLeft(PAD_SMALL);
  new BluetoothConfigWindow(window, grid);
#endif

  new Subtitle(window, STR_AUX_SERIAL_MODE);
  new SerialConfigWindow(window, grid);

  // Calibration
  new SetupButtonGroup(window, {0, 0, LCD_W - padding * 2, 0}, STR_INPUTS, BTN_COLS, PAD_ZERO, {
    {STR_CALIBRATION, []() { new RadioCalibrationPage(); }},
    {STR_STICKS, []() { new HWInputDialog<HWSticks>(STR_STICKS); }},
    {STR_POTS, []() { new HWInputDialog<HWPots>(STR_POTS); }},
    {STR_SWITCHES, []() { new HWInputDialog<HWSwitches>(STR_SWITCHES); }},
  });

  // Debugs
  new SetupButtonGroup(window, {0, 0, LCD_W - padding * 2, 0}, STR_DEBUG, FS_BTN_COLS, PAD_ZERO, {
    {STR_ANALOGS_BTN, []() { new RadioAnalogsDiagsViewPageGroup(); }},
    {STR_KEYS_BTN, []() { new RadioKeyDiagsPage(); }},
#if defined(FUNCTION_SWITCHES)
    {STR_FS_BTN, []() { new RadioCustSwitchesDiagsPage(); }},  
#endif    
  });
}
