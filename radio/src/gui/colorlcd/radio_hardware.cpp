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
#include "opentx.h"
#include "radio_calibration.h"
#include "radio_diaganas.h"
#include "radio_diagkeys.h"

#if defined(BLUETOOTH)
#include "hw_bluetooth.h"
#endif

#define SET_DIRTY() storageDirty(EE_GENERAL)

static const lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(2),
                                     LV_GRID_TEMPLATE_LAST};
static const lv_coord_t row_dsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT,
                                     LV_GRID_TEMPLATE_LAST};

static Window* hbox(Window* parent)
{
  auto box = new Window(parent, rect_t{});
  box->padAll(PAD_TINY);
  box->setFlexLayout(LV_FLEX_FLOW_ROW, PAD_SMALL);
  lv_obj_set_style_grid_cell_x_align(box->getLvObj(), LV_GRID_ALIGN_STRETCH, 0);
  lv_obj_set_flex_align(box->getLvObj(), LV_FLEX_ALIGN_START,
                        LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_SPACE_AROUND);

  return box;
}

RadioHardwarePage::RadioHardwarePage() :
    PageTab(STR_HARDWARE, ICON_RADIO_HARDWARE)
{
}

void RadioHardwarePage::checkEvents() { enableVBatBridge(); }

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

void RadioHardwarePage::build(Window* window)
{
  window->setFlexLayout(LV_FLEX_FLOW_COLUMN, 0);

  FlexGridLayout grid(col_dsc, row_dsc, PAD_TINY);

  // TODO: sub-title?

  // Batt meter range - Range 3.0v to 16v
  auto line = window->newLine(grid);
  new StaticText(line, rect_t{}, STR_BATTERY_RANGE);

  auto box = hbox(line);
  auto batMin = new NumberEdit(
      box, rect_t{0, 0, NUM_EDIT_W, 0}, -60 + 90, g_eeGeneral.vBatMax + 29 + 90,
      GET_SET_WITH_OFFSET(g_eeGeneral.vBatMin, 90), PREC1);
  batMin->setSuffix("V");
  new StaticText(box, rect_t{}, "-");
  auto batMax = new NumberEdit(
      box, rect_t{0, 0, NUM_EDIT_W, 0}, g_eeGeneral.vBatMin - 29 + 120, 40 + 120,
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

  // Bat calibration
  line = window->newLine(grid);
  new StaticText(line, rect_t{}, STR_BATT_CALIB);
  box = hbox(line);
  new BatCalEdit(box, rect_t{0, 0, NUM_EDIT_W, 0});

  // RTC Batt check enable
  line = window->newLine(grid);
  new StaticText(line, rect_t{}, STR_RTC_CHECK);

  box = hbox(line);
  new ToggleSwitch(box, rect_t{},
                   GET_SET_INVERTED(g_eeGeneral.disableRtcWarning));

  // RTC Batt display
  new StaticText(box, rect_t{}, STR_VALUE);
  new DynamicNumber<uint16_t>(
      box, rect_t{}, [] { return getRTCBatteryVoltage(); },
      COLOR_THEME_PRIMARY1 | PREC2, nullptr, "V");

  // ADC filter
  line = window->newLine(grid);
  new StaticText(line, rect_t{}, STR_JITTER_FILTER);
  box = hbox(line);
  new ToggleSwitch(box, rect_t{}, GET_SET_INVERTED(g_eeGeneral.noJitterFilter));

#if defined(AUDIO_MUTE_GPIO)
  // Mute audio
  line = window->newLine(grid);
  new StaticText(line, rect_t{}, STR_AUDIO_MUTE);
  box = hbox(line);
  new ToggleSwitch(box, rect_t{}, GET_SET_DEFAULT(g_eeGeneral.audioMuteEnable));
#endif

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
  new Subtitle(window, STR_INPUTS);

  box = new Window(window, rect_t{});
  box->setFlexLayout(LV_FLEX_FLOW_ROW_WRAP, PAD_MEDIUM);
  lv_obj_set_style_flex_main_place(box->getLvObj(), LV_FLEX_ALIGN_SPACE_EVENLY,
                                   0);
  box->padAll(PAD_MEDIUM);

  new TextButton(box, rect_t{0, 0, 100, 0}, STR_CALIBRATION, [=]() -> uint8_t {
    new RadioCalibrationPage();
    return 0;
  });

  // Sticks
  makeHWInputButton<HWSticks>(box, STR_STICKS);

  // Pots & Sliders
  makeHWInputButton<HWPots>(box, STR_POTS);

  // Switches
  makeHWInputButton<HWSwitches>(box, STR_SWITCHES);

  // Debugs
  new Subtitle(window, STR_DEBUG);

  box = new Window(window, rect_t{});
  box->setFlexLayout(LV_FLEX_FLOW_ROW_WRAP, PAD_MEDIUM);
  lv_obj_set_style_flex_main_place(box->getLvObj(), LV_FLEX_ALIGN_SPACE_EVENLY,
                                   0);
  box->padAll(PAD_MEDIUM);

  new TextButton(box, rect_t{0, 0, 100, 0}, STR_ANALOGS_BTN, [=]() -> uint8_t {
    new RadioAnalogsDiagsViewPageGroup();
    return 0;
  });

  new TextButton(box, rect_t{0, 0, 100, 0}, STR_KEYS_BTN, [=]() -> uint8_t {
    new RadioKeyDiagsPage();
    return 0;
  });
}
