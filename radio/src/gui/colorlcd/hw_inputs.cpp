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

#include "analogs.h"
#include "hal/adc_driver.h"
#include "hal/switch_driver.h"
#include "opentx.h"
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
static const lv_coord_t col_three_dsc[] = {LV_GRID_FR(8), LV_GRID_FR(12), LV_GRID_FR(20),
                                         LV_GRID_TEMPLATE_LAST};

#if LCD_W > LCD_H
static const lv_coord_t pots_col_dsc[] = {LV_GRID_FR(2), LV_GRID_FR(2), LV_GRID_FR(5),
                                          LV_GRID_FR(2), LV_GRID_TEMPLATE_LAST};
#else
static const lv_coord_t pots_col_dsc[] = {LV_GRID_FR(22), LV_GRID_FR(48), LV_GRID_FR(16),
                                          LV_GRID_TEMPLATE_LAST};
#endif

static const lv_coord_t row_dsc[] = {LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};

HWSticks::HWSticks(Window* parent) : FormWindow(parent, rect_t{})
{
  FlexGridLayout grid(col_two_dsc, row_dsc, 2);
  setFlexLayout();

  auto max_sticks = adcGetMaxInputs(ADC_INPUT_MAIN);
  for (int i = 0; i < max_sticks; i++) {
    auto line = newLine(&grid);
    new StaticText(line, rect_t{}, analogGetCanonicalName(ADC_INPUT_MAIN, i), 0,
                   COLOR_THEME_PRIMARY1);
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

HWPots::HWPots(Window* parent) : FormWindow(parent, rect_t{})
{
  FlexGridLayout grid(pots_col_dsc, row_dsc, 2);
  setFlexLayout();

  potsChanged = false;

  setCloseHandler([=]() {
    if (potsChanged) {
      deleteCustomScreens();
      loadCustomScreens();
    }
  });

  auto max_pots = adcGetMaxInputs(ADC_INPUT_FLEX);
  for (int i = 0; i < max_pots; i++) {
    // TODO: check initialised ADC inputs instead!

    // Display EX3 & EX4 (= last two pots) only when FlySky gimbals are present
    // TODO: use input disabled mask instead
    // #if !defined(SIMU) && defined(RADIO_FAMILY_T16)
    //     if (!globalData.flyskygimbals && (i >= (NUM_POTS - 2))) continue;
    // #endif
    auto line = newLine(&grid);
    new StaticText(line, rect_t{}, adcGetInputLabel(ADC_INPUT_FLEX, i), 0,
                   COLOR_THEME_PRIMARY1);

#if LCD_H > LCD_W
    line = newLine(&grid);
#endif

    new HWInputEdit(line, (char*)analogGetCustomLabel(ADC_INPUT_FLEX, i),
                    LEN_ANA_NAME);
    auto pot = new Choice(
        line, rect_t{}, STR_POTTYPES, FLEX_NONE, FLEX_SWITCH,
        [=]() -> int { return getPotType(i); },
        [=](int newValue) {
          setPotType(i, newValue);
          switchFixFlexConfig();
          potsChanged = true;
          SET_DIRTY();
        });
    pot->setAvailableHandler([=](int val) { return isPotTypeAvailable(val); });

    new ToggleSwitch(
        line, rect_t{}, [=]() -> uint8_t { return (uint8_t)getPotInversion(i); },
        [=](int8_t newValue) {
          setPotInversion(i, newValue);
          SET_DIRTY();
        });
  }
}

class SwitchDynamicLabel : public StaticText
{
 public:
  SwitchDynamicLabel(Window* parent, uint8_t index) :
      StaticText(parent, rect_t{}, "", 0, COLOR_THEME_PRIMARY1), index(index)
  {
    checkEvents();
  }

  std::string label()
  {
    std::string str(switchGetCanonicalName(index));
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

static void flex_channel_changed(lv_event_t* e)
{
  auto target = lv_event_get_target(e);
  auto channel = (Choice*)lv_obj_get_user_data(target);

  auto sw_cfg = (Choice*)lv_event_get_user_data(e);
  lv_obj_t* sw_cfg_obj = sw_cfg->getLvObj();

  if (channel->getIntValue() < 0) {
    lv_obj_add_flag(sw_cfg_obj, LV_OBJ_FLAG_HIDDEN);
    sw_cfg->setValue(0);
  } else {
    lv_obj_clear_flag(sw_cfg_obj, LV_OBJ_FLAG_HIDDEN);
  }
}

HWSwitches::HWSwitches(Window* parent) : FormWindow(parent, rect_t{})
{
  FlexGridLayout grid(col_three_dsc, row_dsc, 2);
  setFlexLayout();

  auto max_switches = switchGetMaxSwitches();
  for (int i = 0; i < max_switches; i++) {
    auto line = newLine(&grid);
    new SwitchDynamicLabel(line, i);
    new HWInputEdit(line, (char*)switchGetCustomName(i), LEN_SWITCH_NAME);

    auto box = new FormWindow(line, rect_t{});
    box->setFlexLayout(LV_FLEX_FLOW_ROW, lv_dpx(4));
    box->setWidth(lv_pct(45));

    Choice* channel = nullptr;
    if (switchIsFlex(i)) {
      channel = new Choice(
          box, rect_t{}, -1, adcGetMaxInputs(ADC_INPUT_FLEX) - 1,
          [=]() -> int { return switchGetFlexConfig(i); },
          [=](int newValue) { switchConfigFlex(i, newValue); });
      channel->setAvailableHandler(
          [=](int val) { return val < 0 || switchIsFlexInputAvailable(i, val); });
      channel->setTextHandler([=](int val) -> std::string {
        if (val < 0) return STR_NONE;
        return adcGetInputLabel(ADC_INPUT_FLEX, val);
      });
    }

    auto sw_cfg = new Choice(
        box, rect_t{}, STR_SWTYPES, SWITCH_NONE, switchGetMaxType(i),
        [=]() -> int { return SWITCH_CONFIG(i); },
        [=](int newValue) {
          swconfig_t mask = (swconfig_t)SWITCH_CONFIG_MASK(i);
          g_eeGeneral.switchConfig =
              (g_eeGeneral.switchConfig & ~mask) |
              ((swconfig_t(newValue) & SW_CFG_MASK) << (SW_CFG_BITS * i));
          SET_DIRTY();
        });

    if (channel) {
      lv_obj_t* obj = channel->getLvObj();
      lv_obj_add_event_cb(obj, flex_channel_changed, LV_EVENT_VALUE_CHANGED, sw_cfg);
      lv_event_send(obj, LV_EVENT_VALUE_CHANGED, NULL);
    }
  }
}

template <class T>
HWInputDialog<T>::HWInputDialog(const char* title) :
    Dialog(Layer::back(), std::string(), rect_t{})
{
  setCloseWhenClickOutside(true);
  if (title) content->setTitle(title);
  new T(&content->form);
#if LCD_W > LCD_H
  content->setWidth(LCD_W * 0.8);
#else
  content->setWidth(LCD_W * 0.95);
#endif
  content->updateSize();
}

template struct HWInputDialog<HWSticks>;
template struct HWInputDialog<HWPots>;
template struct HWInputDialog<HWSwitches>;
