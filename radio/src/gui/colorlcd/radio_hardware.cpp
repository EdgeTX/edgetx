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
#include "radio_calibration.h"
#include "radio_diagkeys.h"
#include "radio_diaganas.h"
#include "opentx.h"
#include "libopenui.h"
#include "hal/adc_driver.h"
#include "aux_serial_driver.h"
#include "hw_intmodule.h"
#include "hw_serial.h"
#include "hw_inputs.h"

#define SET_DIRTY() storageDirty(EE_GENERAL)

static const lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(2),
                                     LV_GRID_TEMPLATE_LAST};
static const lv_coord_t row_dsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT,
                                     LV_GRID_TEMPLATE_LAST};

RadioHardwarePage::RadioHardwarePage():
  PageTab(STR_HARDWARE, ICON_RADIO_HARDWARE)
{
}

#if defined(BLUETOOTH)
class ModeChoice : public Choice
{
 public:
  ModeChoice(Window *parent, const rect_t &rect, const char **values, int vmin,
             int vmax, std::function<int()> getValue,
             std::function<void(int)> setValue = nullptr,
             bool *menuOpen = nullptr) :
      Choice(parent, rect, values, vmin, vmax, getValue, setValue),
      menuOpen(menuOpen)
  {
  }

 protected:
  void openMenu()
  {
    if (menuOpen) *menuOpen = true;
    Choice::openMenu();
  }

 private:
  bool *menuOpen;
};

class BluetoothConfigWindow : public FormGroup
{
 public:
  BluetoothConfigWindow(Window *parent, const rect_t &rect) :
      FormGroup(parent, rect, FORWARD_SCROLL)
  {
    update();
  }

  void checkEvents() override
  {
    FormGroup::checkEvents();
    if (!rte) return;
    if (bluetooth.state != lastbluetoothstate) {
      lastbluetoothstate = bluetooth.state;
      if (!(modechoiceopen || rte->hasFocus())) update();
    }
  }

  void update()
  {
    clear();

    setFlexLayout();
    FlexGridLayout grid(col_dsc, row_dsc, 2);
    lv_obj_set_style_pad_left(lvobj, lv_dpx(8), 0);

    auto line = newLine(&grid);
    new StaticText(line, rect_t{}, STR_MODE, 0, COLOR_THEME_PRIMARY1);

    modechoiceopen = false;
    btMode = new ModeChoice(
        line, rect_t{}, STR_BLUETOOTH_MODES, BLUETOOTH_OFF, BLUETOOTH_TRAINER,
        GET_DEFAULT(g_eeGeneral.bluetoothMode),
        [=](int32_t newValue) {
          g_eeGeneral.bluetoothMode = newValue;
          update();
          modechoiceopen = false;
          SET_DIRTY();
        },
        &modechoiceopen);

    if (g_eeGeneral.bluetoothMode != BLUETOOTH_OFF) {
      // Pin code (displayed for information only, not editable)
      if (g_eeGeneral.bluetoothMode == BLUETOOTH_TELEMETRY) {
        line = newLine(&grid);
        auto label = new StaticText(line, rect_t{}, STR_BLUETOOTH_PIN_CODE, 0,
                                    COLOR_THEME_PRIMARY1);
        lv_obj_set_style_pad_left(label->getLvObj(), lv_dpx(8), LV_PART_MAIN);
        new StaticText(line, rect_t{}, "000000", 0, COLOR_THEME_PRIMARY1);
      }

      // Local MAC
      line = newLine(&grid);
      auto label = new StaticText(line, rect_t{}, STR_BLUETOOTH_LOCAL_ADDR, 0,
                                  COLOR_THEME_PRIMARY1);
      lv_obj_set_style_pad_left(label->getLvObj(), lv_dpx(8), LV_PART_MAIN);
      new StaticText(
          line, rect_t{},
          bluetooth.localAddr[0] == '\0' ? "---" : bluetooth.localAddr, 0,
          COLOR_THEME_PRIMARY1);

      // Remote MAC
      line = newLine(&grid);
      label = new StaticText(line, rect_t{}, STR_BLUETOOTH_DIST_ADDR, 0,
                             COLOR_THEME_PRIMARY1);
      lv_obj_set_style_pad_left(label->getLvObj(), lv_dpx(8), LV_PART_MAIN);
      new StaticText(
          line, rect_t{},
          bluetooth.distantAddr[0] == '\0' ? "---" : bluetooth.distantAddr, 0,
          COLOR_THEME_PRIMARY1);

      // BT radio name
      line = newLine(&grid);
      label = new StaticText(line, rect_t{}, STR_NAME, 0, COLOR_THEME_PRIMARY1);
      lv_obj_set_style_pad_left(label->getLvObj(), lv_dpx(8), LV_PART_MAIN);
      rte = new RadioTextEdit(line, rect_t{}, g_eeGeneral.bluetoothName,
                              LEN_BLUETOOTH_NAME);
    }
  }

 protected:
  Choice *btMode = nullptr;
  RadioTextEdit *rte = nullptr;

 private:
  bool modechoiceopen = false;
  uint8_t lastbluetoothstate = BLUETOOTH_STATE_OFF;
};
#endif

void RadioHardwarePage::build(FormWindow * window)
{
  window->setFlexLayout();
  FlexGridLayout grid(col_dsc, row_dsc, 2);
  lv_obj_set_style_pad_all(window->getLvObj(), lv_dpx(8), 0);

  // TODO: sub-title?

  // Batt meter range - Range 3.0v to 16v
  auto line = window->newLine(&grid);
  new StaticText(line, rect_t{}, STR_BATTERY_RANGE, 0, COLOR_THEME_PRIMARY1);

  auto box = new FormGroup(line, rect_t{});
  box->setFlexLayout(LV_FLEX_FLOW_ROW, lv_dpx(4));

  auto batMin =
      new NumberEdit(box, rect_t{}, -60 + 90, g_eeGeneral.vBatMax + 29 + 90,
                     GET_SET_WITH_OFFSET(g_eeGeneral.vBatMin, 90), 0, PREC1);
  batMin->setSuffix("V");
  new StaticText(box, rect_t{}, "-");
  auto batMax =
      new NumberEdit(box, rect_t{}, g_eeGeneral.vBatMin - 29 + 120, 40 + 120,
                     GET_SET_WITH_OFFSET(g_eeGeneral.vBatMax, 120), 0, PREC1);
  batMax->setSuffix("V");

  batMin->setSetValueHandler([=](int32_t newValue) {
    g_eeGeneral.vBatMin = newValue - 90;
    SET_DIRTY();
    batMax->setMin(g_eeGeneral.vBatMin - 29 + 120);
    batMax->invalidate();
  });

  batMax->setSetValueHandler([=](int32_t newValue) {
    g_eeGeneral.vBatMax = newValue - 120;
    SET_DIRTY();
    batMin->setMax(g_eeGeneral.vBatMax + 29 + 90);
    batMin->invalidate();
  });

  // Bat calibration
  line = window->newLine(&grid);
  new StaticText(line, rect_t{}, STR_BATT_CALIB, 0, COLOR_THEME_PRIMARY1);
  auto batCal =
      new NumberEdit(line, rect_t{}, -127, 127,
                     GET_SET_DEFAULT(g_eeGeneral.txVoltageCalibration));
  batCal->setDisplayHandler([](int32_t value) {
    return formatNumberAsString(getBatteryVoltage(), PREC2, 0, nullptr, "V");
  });
  batCal->setWindowFlags(REFRESH_ALWAYS);

  // RTC Batt check enable
  line = window->newLine(&grid);
  new StaticText(line, rect_t{}, STR_RTC_CHECK, 0, COLOR_THEME_PRIMARY1);

  box = new FormGroup(line, rect_t{});
  box->setFlexLayout(LV_FLEX_FLOW_ROW, lv_dpx(8));
  lv_obj_set_style_flex_cross_place(box->getLvObj(), LV_FLEX_ALIGN_CENTER, 0);
  new CheckBox(box, rect_t{}, GET_SET_INVERTED(g_eeGeneral.disableRtcWarning ));

  // RTC Batt display
  new StaticText(box, rect_t{}, STR_VALUE, 0, COLOR_THEME_PRIMARY1);
  new DynamicNumber<uint16_t>(box, rect_t{}, [] {
      return getRTCBatteryVoltage();
  }, COLOR_THEME_PRIMARY1 | PREC2, nullptr, "V");

  // ADC filter
  line = window->newLine(&grid);
  new StaticText(line, rect_t{}, STR_JITTER_FILTER, 0, COLOR_THEME_PRIMARY1);
  new CheckBox(line, rect_t{}, GET_SET_INVERTED(g_eeGeneral.noJitterFilter));

#if defined(HARDWARE_INTERNAL_MODULE)
  new Subtitle(window, rect_t{}, TR_INTERNALRF, 0, COLOR_THEME_PRIMARY1);
  auto mod = new InternalModuleWindow(window, rect_t{});
  mod->padLeft(lv_dpx(8));
#endif

#if defined(BLUETOOTH)
  new Subtitle(window, rect_t{}, STR_BLUETOOTH, 0, COLOR_THEME_PRIMARY1);
  new BluetoothConfigWindow(window, rect_t{});
#endif

  new Subtitle(window, rect_t{}, STR_AUX_SERIAL_MODE, 0, COLOR_THEME_PRIMARY1);
  auto serial = new SerialConfigWindow(window, rect_t{});
  serial->padLeft(lv_dpx(8));

  // Calibration
  new Subtitle(window, rect_t{}, STR_INPUTS, 0, COLOR_THEME_PRIMARY1);

  box = new FormGroup(window, rect_t{});
  box->setFlexLayout(LV_FLEX_FLOW_ROW_WRAP, lv_dpx(8));
  lv_obj_set_style_flex_main_place(box->getLvObj(), LV_FLEX_ALIGN_SPACE_EVENLY, 0);
  box->padRow(lv_dpx(8));
  box->padAll(lv_dpx(8));
  
  auto calib = new TextButton(box, rect_t{}, STR_CALIBRATION);
  calib->setPressHandler([=]() -> uint8_t {
      new RadioCalibrationPage();
      return 0;
  });
  lv_obj_set_style_min_width(calib->getLvObj(), LV_DPI_DEF, 0);

  // Sticks
  auto btn = makeHWInputButton<HWSticks>(box, STR_STICKS);
  lv_obj_set_style_min_width(btn->getLvObj(), LV_DPI_DEF, 0);

  // Pots
  btn = makeHWInputButton<HWPots>(box, STR_POTS);
  lv_obj_set_style_min_width(btn->getLvObj(), LV_DPI_DEF, 0);

  // Sliders
#if (NUM_SLIDERS > 0)
  btn = makeHWInputButton<HWSliders>(box, STR_SLIDERS);
  lv_obj_set_style_min_width(btn->getLvObj(), LV_DPI_DEF, 0);
#endif

  // Switches
  btn = makeHWInputButton<HWSwitches>(box, STR_SWITCHES);
  lv_obj_set_style_min_width(btn->getLvObj(), LV_DPI_DEF, 0);
  
  // Debugs
  new Subtitle(window, rect_t{}, STR_DEBUG, 0, COLOR_THEME_PRIMARY1);

  box = new FormGroup(window, rect_t{});
  box->setFlexLayout(LV_FLEX_FLOW_ROW_WRAP, lv_dpx(8));
  lv_obj_set_style_flex_main_place(box->getLvObj(), LV_FLEX_ALIGN_SPACE_EVENLY, 0);
  box->padRow(lv_dpx(8));
  box->padAll(lv_dpx(8));

  btn = new TextButton(box, rect_t{}, STR_ANALOGS_BTN, [=]() -> uint8_t {
    new RadioAnalogsDiagsViewPageGroup();
    return 0;
  });
  lv_obj_set_style_min_width(btn->getLvObj(), LV_DPI_DEF, 0);

  btn = new TextButton(box, rect_t{}, STR_KEYS_BTN, [=]() -> uint8_t {
    new RadioKeyDiagsPage();
    return 0;
  });
  lv_obj_set_style_min_width(btn->getLvObj(), LV_DPI_DEF, 0);
}
