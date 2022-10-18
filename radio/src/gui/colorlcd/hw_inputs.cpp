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

#include "hw_inputs.h"
#include "opentx.h"

#include "hal/adc_driver.h"
#include "hal/switch_driver.h"
#include "analogs.h"
#include "switches.h"

#define SET_DIRTY() storageDirty(EE_GENERAL)

struct HWInputEdit : public RadioTextEdit {
  HWInputEdit(Window* parent, char* name, size_t len) :
      RadioTextEdit(parent, rect_t{}, name, len)
  {
    setWidth(LV_DPI_DEF / 2);
  }
};

static const lv_coord_t col_two_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(2),
                                         LV_GRID_TEMPLATE_LAST};

static const lv_coord_t row_dsc[] = {LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};

HWSticks::HWSticks(Window* parent) : FormGroup(parent, rect_t{})
{
  FlexGridLayout grid(col_two_dsc, row_dsc, 2);
  setFlexLayout();

  auto max_sticks = adcGetMaxInputs(ADC_INPUT_MAIN);
  for (int i = 0; i < max_sticks; i++) {
    auto line = newLine(&grid);
    new StaticText(line, rect_t{}, analogGetCanonicalName(ADC_INPUT_MAIN, i),
                   0, COLOR_THEME_PRIMARY1);
    new HWInputEdit(line, (char*)analogGetCustomLabel(ADC_INPUT_MAIN, i),
                    LEN_ANA_NAME);
  }

#if defined(STICK_DEAD_ZONE)
  auto line = newLine(&grid);
  new StaticText(line, rect_t{}, STR_DEAD_ZONE, 0, COLOR_THEME_PRIMARY1);
  auto dz = new Choice(line, rect_t{}, 0, 7,
                       GET_SET_DEFAULT(g_eeGeneral.stickDeadZone));
  dz->setTextHandler([](uint8_t value) {
    return std::to_string(value ? 2 << (value - 1) : 0);
  });
#endif
}

HWPots::HWPots(Window* parent) : FormGroup(parent, rect_t{})
{
  FlexGridLayout grid(col_two_dsc, row_dsc, 2);
  setFlexLayout();

  auto max_pots = adcGetMaxInputs(ADC_INPUT_POT);
  for (int i = 0; i < max_pots; i++) {
    // TODO: check initialised ADC inputs instead!

    // Display EX3 & EX4 (= last two pots) only when FlySky gimbals are present
    // TODO: use input disabled mask instead
// #if !defined(SIMU) && defined(RADIO_FAMILY_T16)
//     if (!globalData.flyskygimbals && (i >= (NUM_POTS - 2))) continue;
// #endif
    auto line = newLine(&grid);
    new StaticText(line, rect_t{}, analogGetCanonicalName(ADC_INPUT_POT, i), 0,
                   COLOR_THEME_PRIMARY1);

    auto box = new FormGroup(line, rect_t{});
    box->setFlexLayout(LV_FLEX_FLOW_ROW, lv_dpx(4));

    auto box_obj = box->getLvObj();
    lv_obj_set_style_flex_cross_place(box_obj, LV_FLEX_ALIGN_CENTER, 0);

    new HWInputEdit(box, (char*)analogGetCustomLabel(ADC_INPUT_POT, i), LEN_ANA_NAME);
    new Choice(
        box, rect_t{}, STR_POTTYPES, POT_NONE, POT_SLIDER_WITH_DETENT,
        [=]() -> int {
          return bfGet<potconfig_t>(g_eeGeneral.potsConfig, POT_CFG_BITS * i,
                                 POT_CFG_BITS);
        },
        [=](int newValue) {
          g_eeGeneral.potsConfig = bfSet<potconfig_t>(
              g_eeGeneral.potsConfig, newValue, POT_CFG_BITS * i, POT_CFG_BITS);
          SET_DIRTY();
        });
  }
}

class SwitchDynamicLabel : public StaticText
{
 public:
  SwitchDynamicLabel(Window* parent, uint8_t index) :
      StaticText(parent, rect_t{}, "", 0, COLOR_THEME_PRIMARY1),
      index(index)
  {
    checkEvents();
  }

  std::string label()
  {
    std::string str(switchGetName(index));
    return str + getSwitchPositionSymbol(lastpos);
  }

  uint8_t position()
  {
    auto value = getValue(MIXSRC_FIRST_SWITCH + index);
    if (value > 0)
      return 2;
    else if (value < 0)
      return 0;
    else
      return 1;
  }

  void checkEvents() override
  {
    uint8_t newpos = position();
    if (newpos != lastpos) {
      lastpos = newpos;
      setText(label());
    }
  }

 protected:
  uint8_t index;
  uint8_t lastpos = 0xff;
};

HWSwitches::HWSwitches(Window* parent) : FormGroup(parent, rect_t{})
{
  FlexGridLayout grid(col_two_dsc, row_dsc, 2);
  setFlexLayout();

  for (int i = 0; i < switchGetMaxSwitches(); i++) {
    auto line = newLine(&grid);
    new SwitchDynamicLabel(line, i);

    auto box = new FormGroup(line, rect_t{});
    box->setFlexLayout(LV_FLEX_FLOW_ROW, lv_dpx(4));

    auto box_obj = box->getLvObj();
    lv_obj_set_style_flex_cross_place(box_obj, LV_FLEX_ALIGN_CENTER, 0);

    new HWInputEdit(box, (char*)switchGetCustomName(i), LEN_SWITCH_NAME);
    new Choice(
        box, rect_t{}, STR_SWTYPES, SWITCH_NONE, switchGetMaxType(i),
        [=]() -> int { return SWITCH_CONFIG(i); },
        [=](int newValue) {
          swconfig_t mask = (swconfig_t)SWITCH_CONFIG_MASK(i);
          g_eeGeneral.switchConfig =
              (g_eeGeneral.switchConfig & ~mask) |
              ((swconfig_t(newValue) & SW_CFG_MASK) << (SW_CFG_BITS * i));
          SET_DIRTY();
        });
  }
}

template <class T>
HWInputDialog<T>::HWInputDialog(const char* title) :
    Dialog(Layer::back(), std::string(), rect_t{})
{
  setCloseWhenClickOutside(true);
  if (title) content->setTitle(title);
  new T(&content->form);
  content->setWidth(LCD_W * 0.8);
  content->updateSize();
}

template struct HWInputDialog<HWSticks>;
template struct HWInputDialog<HWPots>;
template struct HWInputDialog<HWSwitches>;
