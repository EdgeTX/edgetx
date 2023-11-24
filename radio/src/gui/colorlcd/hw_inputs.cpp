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
  HWInputEdit(Window* parent, char* name, size_t len, coord_t x = 0,
              coord_t y = 0) :
      RadioTextEdit(parent, rect_t{x, y, 64, 32}, name, len)
  {
  }
};

static const lv_coord_t col_two_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(2),
                                         LV_GRID_TEMPLATE_LAST};
static const lv_coord_t col_three_dsc[] = {
    LV_GRID_FR(8), LV_GRID_FR(12), LV_GRID_FR(20), LV_GRID_TEMPLATE_LAST};

#if LCD_W > LCD_H
static const lv_coord_t pots_col_dsc[] = {LV_GRID_FR(2), LV_GRID_FR(2),
                                          LV_GRID_FR(5), LV_GRID_FR(2),
                                          LV_GRID_TEMPLATE_LAST};
#else
static const lv_coord_t pots_col_dsc[] = {LV_GRID_FR(13), LV_GRID_FR(7),
                                          LV_GRID_TEMPLATE_LAST};
#endif

static const lv_coord_t row_dsc[] = {LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};

HWSticks::HWSticks(Window* parent) : Window(parent, rect_t{})
{
  FlexGridLayout grid(col_two_dsc, row_dsc, PAD_TINY);
  setFlexLayout();

  auto max_sticks = adcGetMaxInputs(ADC_INPUT_MAIN);
  for (int i = 0; i < max_sticks; i++) {
    auto line = newLine(grid);
    new StaticText(line, rect_t{}, analogGetCanonicalName(ADC_INPUT_MAIN, i));
    new HWInputEdit(line, (char*)analogGetCustomLabel(ADC_INPUT_MAIN, i),
                    LEN_ANA_NAME);
  }

#if defined(STICK_DEAD_ZONE)
  auto line = newLine(grid);
  new StaticText(line, rect_t{}, STR_DEAD_ZONE);
  auto dz = new Choice(line, rect_t{}, 0, 7,
                       GET_SET_DEFAULT(g_eeGeneral.stickDeadZone));
  dz->setTextHandler([](uint8_t value) {
    return std::to_string(value ? 2 << (value - 1) : 0);
  });
#endif
}

// Absolute layout for Pots popup - due to performance issues with lv_textarea
// in a flex layout
#if LCD_W > LCD_H
#define P_LBL_X 0
#define P_LBL_W ((coord_t)((DIALOG_DEFAULT_WIDTH - 30) * 2 / 11))
#define P_NM_X (P_LBL_X + P_LBL_W + 6)
#define P_TYP_X (P_NM_X + 70)
#define P_TYP_W 160
#define P_INV_X (P_TYP_X + P_TYP_W + 6)
#define P_INV_W 52
#define P_Y(i) (i * 36 + 2)
#define P_OFST_Y 0
#else
#define P_LBL_X 0
#define P_LBL_W ((coord_t)((DIALOG_DEFAULT_WIDTH - 18) * 13 / 21))
#define P_NM_X (P_LBL_X + P_LBL_W + 6)
#define P_TYP_X 0
#define P_TYP_W P_LBL_W
#define P_INV_X (P_TYP_X + P_TYP_W + 6)
#define P_INV_W 52
#define P_Y(i) (i * 72 + 2)
#define P_OFST_Y 36
#endif

HWPots::HWPots(Window* parent) :
    Window(parent, rect_t{0, 0, DIALOG_DEFAULT_WIDTH - 12, LV_SIZE_CONTENT})
{
  potsChanged = false;

  setCloseHandler([=]() {
    if (potsChanged) {
      LayoutFactory::deleteCustomScreens();
      LayoutFactory::loadCustomScreens();
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
    new StaticText(this, rect_t{P_LBL_X, P_Y(i) + 6, P_LBL_W, 32},
                   adcGetInputLabel(ADC_INPUT_FLEX, i));

    new HWInputEdit(this, (char*)analogGetCustomLabel(ADC_INPUT_FLEX, i),
                    LEN_ANA_NAME, P_NM_X, P_Y(i));

    auto pot = new Choice(
        this, rect_t{P_TYP_X, P_Y(i) + P_OFST_Y, P_TYP_W, 32}, STR_POTTYPES,
        FLEX_NONE, FLEX_SWITCH, [=]() -> int { return getPotType(i); },
        [=](int newValue) {
          setPotType(i, newValue);
          switchFixFlexConfig();
          potsChanged = true;
          SET_DIRTY();
        });
    pot->setAvailableHandler([=](int val) { return isPotTypeAvailable(val); });

    new ToggleSwitch(
        this, rect_t{P_INV_X, P_Y(i) + P_OFST_Y, P_INV_W, 32},
        [=]() -> uint8_t { return (uint8_t)getPotInversion(i); },
        [=](int8_t newValue) {
          setPotInversion(i, newValue);
          SET_DIRTY();
        });
  }
}

// Absolute layout for Switches popup - due to performance issues with
// lv_textarea in a flex layout
#if LCD_W > LCD_H
#define SW_CTRL_W 86
#else
#define SW_CTRL_W 75
#endif

class SwitchDynamicLabel : public StaticText
{
 public:
  SwitchDynamicLabel(Window* parent, uint8_t index, coord_t x, coord_t y) :
      StaticText(parent, rect_t{x, y, SW_CTRL_W, 32}, ""),
      index(index)
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

HWSwitches::HWSwitches(Window* parent) :
    Window(parent, rect_t{0, 0, DIALOG_DEFAULT_WIDTH - 12, LV_SIZE_CONTENT})
{
  auto max_switches = switchGetMaxSwitches();
  for (int i = 0; i < max_switches; i++) {
    new SwitchDynamicLabel(this, i, 2, i * 36 + 2);
    new HWInputEdit(this, (char*)switchGetCustomName(i), LEN_SWITCH_NAME,
                    SW_CTRL_W + 8, i * 36 + 2);

    coord_t x = SW_CTRL_W * 2 + 14;
    Choice* channel = nullptr;
    if (switchIsFlex(i)) {
      channel = new Choice(
          this, rect_t{x, i * 36 + 2, SW_CTRL_W, 32}, -1,
          adcGetMaxInputs(ADC_INPUT_FLEX) - 1,
          [=]() -> int { return switchGetFlexConfig(i); },
          [=](int newValue) { switchConfigFlex(i, newValue); });
      channel->setAvailableHandler([=](int val) {
        return val < 0 || switchIsFlexInputAvailable(i, val);
      });
      channel->setTextHandler([=](int val) -> std::string {
        if (val < 0) return STR_NONE;
        return adcGetInputLabel(ADC_INPUT_FLEX, val);
      });
      x += SW_CTRL_W + 6;
    }

    auto sw_cfg = new Choice(
        this, rect_t{x, i * 36 + 2, SW_CTRL_W, 32}, STR_SWTYPES, SWITCH_NONE,
        switchGetMaxType(i), [=]() -> int { return SWITCH_CONFIG(i); },
        [=](int newValue) {
          swconfig_t mask = (swconfig_t)SWITCH_CONFIG_MASK(i);
          g_eeGeneral.switchConfig =
              (g_eeGeneral.switchConfig & ~mask) |
              ((swconfig_t(newValue) & SW_CFG_MASK) << (SW_CFG_BITS * i));
          SET_DIRTY();
        });

    if (channel) {
      lv_obj_t* obj = channel->getLvObj();
      lv_obj_add_event_cb(obj, flex_channel_changed, LV_EVENT_VALUE_CHANGED,
                          sw_cfg);
      lv_event_send(obj, LV_EVENT_VALUE_CHANGED, NULL);
    }
  }
}

template <class T>
HWInputDialog<T>::HWInputDialog(const char* title) :
    BaseDialog(Layer::back(), title, true)
{
  new T(form);
}

template struct HWInputDialog<HWSticks>;
template struct HWInputDialog<HWPots>;
template struct HWInputDialog<HWSwitches>;
