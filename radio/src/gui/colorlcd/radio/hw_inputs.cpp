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
#include "edgetx.h"
#include "switches.h"

#define SET_DIRTY() storageDirty(EE_GENERAL)

struct HWInputEdit : public RadioTextEdit {
  HWInputEdit(Window* parent, char* name, size_t len, coord_t x = 0,
              coord_t y = 0) :
      RadioTextEdit(parent, rect_t{x, y, HW_INP_W, EdgeTxStyles::UI_ELEMENT_HEIGHT}, name,
                    len)
  {
  }

  static LAYOUT_VAL(HW_INP_W, 64, 64)
};

static const lv_coord_t col_two_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(2),
                                         LV_GRID_TEMPLATE_LAST};
static const lv_coord_t col_three_dsc[] = {
    LV_GRID_FR(8), LV_GRID_FR(12), LV_GRID_FR(20), LV_GRID_TEMPLATE_LAST};

#if !PORTRAIT_LCD
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
  padAll(PAD_TINY);

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

HWPots::HWPots(Window* parent) :
    Window(parent, rect_t{0, 0, DIALOG_DEFAULT_WIDTH - 12, LV_SIZE_CONTENT})
{
  padAll(PAD_TINY);

  potsChanged = false;

  setCloseHandler([=]() {
    if (potsChanged) {
      LayoutFactory::deleteCustomScreens();
      LayoutFactory::loadCustomScreens();
    }
  });

  new StaticText(this, {P_NM_X, -2, 0, 0}, STR_NAME, COLOR_THEME_PRIMARY1_INDEX, FONT(XS));
  new StaticText(this, {P_TYP_X, -2, 0, 0}, STR_TYPE, COLOR_THEME_PRIMARY1_INDEX, FONT(XS));
  new StaticText(this, {P_INV_X, -2, 0, 0}, STR_MENU_INVERT, COLOR_THEME_PRIMARY1_INDEX, FONT(XS));

  coord_t yo = EdgeTxStyles::PAGE_LINE_HEIGHT - 2;

  auto max_pots = adcGetMaxInputs(ADC_INPUT_FLEX);
  for (int i = 0; i < max_pots; i++) {
    // TODO: check initialised ADC inputs instead!

    // Display EX3 & EX4 (= last two pots) only when FlySky gimbals are present
    // TODO: use input disabled mask instead
    // #if !defined(SIMU) && defined(RADIO_FAMILY_T16)
    //     if (!globalData.flyskygimbals && (i >= (NUM_POTS - 2))) continue;
    // #endif
    new StaticText(this,
                   rect_t{P_LBL_X, P_Y(i) + yo + PAD_MEDIUM, P_LBL_W, 0},
                   adcGetInputLabel(ADC_INPUT_FLEX, i));

    new HWInputEdit(this, (char*)analogGetCustomLabel(ADC_INPUT_FLEX, i),
                    LEN_ANA_NAME, P_NM_X, P_Y(i) + yo);

    auto pot = new Choice(
        this, rect_t{P_TYP_X, P_Y(i) + P_OFST_Y + yo, P_TYP_W, 0},
        STR_POTTYPES, FLEX_NONE, FLEX_SWITCH,
        [=]() -> int { return getPotType(i); },
        [=](int newValue) {
          setPotType(i, newValue);
          switchFixFlexConfig();
          potsChanged = true;
          invertToggles[i]->show(newValue != FLEX_MULTIPOS);
          if (newValue == FLEX_MULTIPOS) {
            setPotInversion(i, 0);
            invertToggles[i]->update();
          }
          SET_DIRTY();
        });
    pot->setAvailableHandler([=](int val) { return isPotTypeAvailable(val); });

    auto tgl = new ToggleSwitch(
          this, rect_t{P_INV_X, P_Y(i) + P_OFST_Y + yo, P_INV_W, 0},
          [=]() -> uint8_t { return (uint8_t)getPotInversion(i); },
          [=](int8_t newValue) {
            setPotInversion(i, newValue);
            SET_DIRTY();
          });
    tgl->show(!IS_POT_MULTIPOS(i));
    if (IS_POT_MULTIPOS(i) && getPotInversion(i)) {
      setPotInversion(i, 0);
      SET_DIRTY();
    }
    invertToggles.push_back(tgl);
  }
}

class SwitchDynamicLabel : public StaticText
{
 public:
  SwitchDynamicLabel(Window* parent, uint8_t index, coord_t x, coord_t y, coord_t w) :
      StaticText(parent, rect_t{x, y, w, 0}, ""),
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

class HWSwitch
{
 public:
  HWSwitch(Window* parent, int swnum, coord_t y)
  {
    new SwitchDynamicLabel(parent, swnum, PAD_TINY, y + PAD_MEDIUM, SW_CTRL_W);
    new HWInputEdit(parent, (char*)switchGetCustomName(swnum), LEN_SWITCH_NAME,
                    SW_CTRL_W + 8, y);

    coord_t x = SW_CTRL_W * PAD_TINY + 14;

    if (switchIsFlex(swnum)) {
      channel = new Choice(
          parent, {x, y, SW_CTRL_W, 0}, -1,
          adcGetMaxInputs(ADC_INPUT_FLEX) - 1,
          [=]() -> int { return switchGetFlexConfig(swnum); },
          [=](int newValue) {
            switchConfigFlex(swnum, newValue);
            updateLayout();
          });
      channel->setAvailableHandler([=](int val) {
        return val < 0 || switchIsFlexInputAvailable(swnum, val);
      });
      channel->setTextHandler([=](int val) -> std::string {
        if (val < 0) return STR_NONE;
        return adcGetInputLabel(ADC_INPUT_FLEX, val);
      });
      x += SW_CTRL_W + 6;
    }

    sw_cfg = new Choice(
        parent, {x, y, SW_CTRL_W, 0},
        STR_SWTYPES, SWITCH_NONE, switchGetMaxType(swnum),
        [=]() -> int { return SWITCH_CONFIG(swnum); },
        [=](int newValue) {
          swconfig_t mask = (swconfig_t)SWITCH_CONFIG_MASK(swnum);
          g_eeGeneral.switchConfig =
              (g_eeGeneral.switchConfig & ~mask) |
              ((swconfig_t(newValue) & SW_CFG_MASK) << (SW_CFG_BITS * swnum));
          SET_DIRTY();
        });

    updateLayout();
  }

  void updateLayout()
  {
    if (channel) {
      sw_cfg->show(channel->getIntValue() >= 0);
      if (channel->getIntValue() < 0)
        sw_cfg->setValue(0);
    }
  }

  static LAYOUT_VAL(SW_CTRL_W, 86, 75)
  static LAYOUT_VAL(SW_CTRL_H, 36, 36)

 protected:
  Choice* channel = nullptr;
  Choice* sw_cfg = nullptr;
};

HWSwitches::HWSwitches(Window* parent) :
    Window(parent, rect_t{0, 0, DIALOG_DEFAULT_WIDTH - 12, LV_SIZE_CONTENT})
{
  padAll(PAD_TINY);

  auto max_switches = switchGetMaxSwitches();
  for (int i = 0; i < max_switches; i++) {
    new HWSwitch(this, i, i * HWSwitch::SW_CTRL_H + PAD_TINY);
  }
}

template <class T>
HWInputDialog<T>::HWInputDialog(const char* title) :
    BaseDialog(title, true)
{
  new T(form);
}

template struct HWInputDialog<HWSticks>;
template struct HWInputDialog<HWPots>;
template struct HWInputDialog<HWSwitches>;
