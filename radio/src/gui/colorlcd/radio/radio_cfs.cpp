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

#if defined(FUNCTION_SWITCHES)

#include "radio_cfs.h"

#include "edgetx.h"
#include "hal/rgbleds.h"
#include "strhelpers.h"
#include "switches.h"
#include "color_picker.h"

#define SET_DIRTY() storageDirty(EE_GENERAL)

static const lv_coord_t cfs_line_col_dsc1[] = {LV_GRID_CONTENT,
    LV_GRID_TEMPLATE_LAST};

#if defined(FUNCTION_SWITCHES_RGB_LEDS)
static const lv_coord_t cfs_line_col_dsc2[] = {
  LV_GRID_FR(13), LV_GRID_FR(11), LV_GRID_FR(11),
  LV_GRID_FR(12), LV_GRID_FR(10), LV_GRID_FR(7), LV_GRID_FR(7),
  LV_GRID_TEMPLATE_LAST};
#else
static const lv_coord_t cfs_line_col_dsc2[] = {
  LV_GRID_FR(10), LV_GRID_FR(10), LV_GRID_FR(10),
  LV_GRID_FR(12), LV_GRID_FR(8),  LV_GRID_TEMPLATE_LAST};
#endif

static const lv_coord_t cfs_line_row_dsc[] = {LV_GRID_CONTENT,
   LV_GRID_TEMPLATE_LAST};

extern const char* _fct_sw_start[];
extern const char* edgetx_fs_manual_url;

class RadioFunctionSwitch : public Window
{
 public:
  RadioFunctionSwitch(Window* parent, uint8_t sw) :
      Window(parent, {0, 0, LCD_W - PAD_SMALL * 2, ROW_H}), switchIndex(sw)
  {
    padAll(PAD_TINY);

    std::string s(STR_CHAR_SWITCH);
    s += switchGetDefaultName(switchIndex);

    new StaticText(this, {PAD_LARGE, PAD_MEDIUM, SW_W, EdgeTxStyles::STD_FONT_HEIGHT}, s);

    new ModelTextEdit(this, {NM_X, 0, NM_W, 0},
                      g_eeGeneral.switchName(switchIndex), LEN_SWITCH_NAME);

    typeChoice = new Choice(
        this, {TP_X, 0, TP_W, 0}, STR_SWTYPES, SWITCH_NONE, SWITCH_GLOBAL,
        [=]() { return g_eeGeneral.switchType(switchIndex); },
        [=](int val) {
            g_eeGeneral.switchSetType(switchIndex, (SwitchConfig)val);
          if (val == SWITCH_NONE) {
#if defined(FUNCTION_SWITCHES_RGB_LEDS)
            if (g_model.getSwitchType(switchIndex) == SWITCH_NONE)
              fsLedRGB(switchGetCustomSwitchIdx(switchIndex), 0);
#endif
          } else if (val == SWITCH_TOGGLE) {
            g_eeGeneral.switchSetStart(switchIndex, FS_START_PREVIOUS);
            setFSLogicalState(switchIndex, 0);
            startChoice->setValue(startChoice->getIntValue());
          }
          SET_DIRTY();
        });
    typeChoice->setAvailableHandler([=](int typ) -> bool {
      if (typ == SWITCH_3POS || typ == SWITCH_GLOBAL) return false;
      int group = g_eeGeneral.switchGroup(switchIndex);
      if (group > 0 && g_model.cfsGroupAlwaysOn(group) && typ == SWITCH_TOGGLE)
        return false;
      return true;
    });

    startChoice = new Choice(
        this, {ST_X, 0, ST_W, 0}, _fct_sw_start, 0, 2,
        [=]() { return g_eeGeneral.switchStart(switchIndex); },
        [=](int val) {
            g_eeGeneral.switchSetStart(switchIndex, (fsStartPositionType)val);
          SET_DIRTY();
        });

#if defined(FUNCTION_SWITCHES_RGB_LEDS)
#if NARROW_LAYOUT
    new StaticText(this, rect_t{C1_X - C1_W - PAD_TINY, C1_Y + PAD_SMALL, C1_W, 0}, STR_OFF, COLOR_THEME_PRIMARY1_INDEX, FONT(XS) | RIGHT);
    new StaticText(this, rect_t{C2_X - C2_W - PAD_TINY, C2_Y + PAD_SMALL, C2_W, 0}, STR_ON_ONE_SWITCHES[0], COLOR_THEME_PRIMARY1_INDEX, FONT(XS) | RIGHT);
#endif

    offValue = g_eeGeneral.switchOffColor(switchIndex);
    onValue = g_eeGeneral.switchOnColor(switchIndex);

    offColor = new ColorPicker(
        this, {C1_X, 0, C1_W, 0},
        [=]() -> int {  // getValue
          return g_eeGeneral.switchOffColor(switchIndex).getColor() | RGB888_FLAG;
        },
        [=](int newValue) {  // setValue
            g_eeGeneral.switchOffColor(switchIndex) = offValue;
            g_eeGeneral.switchOnColor(switchIndex) = onValue;

          // Convert color index to RGB
          newValue = color32ToRGB(newValue);
          g_eeGeneral.switchOffColor(switchIndex).setColor(newValue);

          offValue = g_eeGeneral.switchOffColor(switchIndex);
          SET_DIRTY();
        },
        [=](int newValue) { previewColor(newValue); }, ETX_RGB888);

    onColor = new ColorPicker(
        this, {C2_X, 0, C2_W, 0},
        [=]() -> int {  // getValue
          return g_eeGeneral.switchOnColor(switchIndex).getColor() | RGB888_FLAG;
        },
        [=](int newValue) {  // setValue
            g_eeGeneral.switchOffColor(switchIndex) = offValue;
            g_eeGeneral.switchOnColor(switchIndex) = onValue;

          // Convert color index to RGB
          newValue = color32ToRGB(newValue);
          g_eeGeneral.switchOnColor(switchIndex).setColor(newValue);

          onValue = g_eeGeneral.switchOnColor(switchIndex);
          SET_DIRTY();
        },
        [=](int newValue) { previewColor(newValue); }, ETX_RGB888);

    overrideLabel = new StaticText(this, {GR_X, C1_Y + EdgeTxStyles::UI_ELEMENT_HEIGHT + PAD_LARGE, GR_W + ST_W - PAD_LARGE, 0},
                                   STR_LUA_OVERRIDE, COLOR_THEME_PRIMARY1_INDEX, FONT(XS) | RIGHT);
    offOverride = new ToggleSwitch(this, {C1_X - PAD_MEDIUM * 2, C1_Y + EdgeTxStyles::UI_ELEMENT_HEIGHT + PAD_OUTLINE, 0, 0},
                                  [=]() { return g_model.cfsOffColorLuaOverride(switchIndex); },
                                  [=](bool v) { g_model.cfsSetOffColorLuaOverride(switchIndex, v); });
    onOverride = new ToggleSwitch(this, {C2_X, C1_Y + EdgeTxStyles::UI_ELEMENT_HEIGHT + PAD_OUTLINE, 0, 0},
                                  [=]() { return g_model.cfsOnColorLuaOverride(switchIndex); },
                                  [=](bool v) { g_model.cfsSetOnColorLuaOverride(switchIndex, v); });
#endif //FUNCTION_SWITCHES_RGB_LEDS

    setState();
  }

#if defined(FUNCTION_SWITCHES_RGB_LEDS)
  static LAYOUT_VAL_SCALED(SW_W, 70)
  static constexpr coord_t NM_X = SW_W + PAD_SMALL;
  static LAYOUT_VAL_SCALED(NM_W, 60)
  static constexpr coord_t TP_X = NM_X + NM_W + PAD_SMALL;
  static LAYOUT_VAL_SCALED(TP_W, 78)
  static constexpr coord_t GR_X = TP_X + TP_W + PAD_SMALL;
  static LAYOUT_VAL_SCALED(GR_W, 84)
  static constexpr coord_t ST_X = GR_X + GR_W + PAD_SMALL;
  static LAYOUT_VAL_SCALED(ST_W, 60)
#if NARROW_LAYOUT
  static constexpr coord_t ROW_H = EdgeTxStyles::UI_ELEMENT_HEIGHT * 3 + PAD_OUTLINE * 4;
  static constexpr coord_t C1_X = TP_X;
  static constexpr coord_t C1_Y = EdgeTxStyles::UI_ELEMENT_HEIGHT + PAD_OUTLINE;
  static LAYOUT_VAL_SCALED(C1_W, 40)
  static constexpr coord_t C2_X = GR_X;
  static constexpr coord_t C2_Y = EdgeTxStyles::UI_ELEMENT_HEIGHT + PAD_OUTLINE;
  static LAYOUT_VAL_SCALED(C2_W, 40)
#else
  static constexpr coord_t ROW_H = EdgeTxStyles::UI_ELEMENT_HEIGHT * 2 + PAD_OUTLINE * 3;
  static constexpr coord_t C1_X = ST_X + ST_W + PAD_SMALL;
  static constexpr coord_t C1_Y = 0;
  static LAYOUT_VAL_SCALED(C1_W, 40)
  static constexpr coord_t C2_X = C1_X + C1_W + PAD_SMALL;
  static constexpr coord_t C2_Y = 0;
  static LAYOUT_VAL_SCALED(C2_W, 40)
#endif
#else
  static constexpr coord_t ROW_H = EdgeTxStyles::UI_ELEMENT_HEIGHT + PAD_OUTLINE * 2;
  static constexpr coord_t SW_W = (LCD_W - PAD_SMALL * 2 - PAD_TINY * 4) / 5;
  static constexpr coord_t NM_X = SW_W + PAD_TINY;
  static LAYOUT_VAL_SCALED(NM_W, 80)
  static constexpr coord_t TP_X = NM_X + SW_W + PAD_TINY;
  static LAYOUT_VAL_SCALED(TP_W, 86)
  static constexpr coord_t GR_X = TP_X + SW_W + PAD_TINY;
  static LAYOUT_VAL_SCALED(GR_W, 94)
  static constexpr coord_t ST_X = GR_X + SW_W + PAD_LARGE * 2 + PAD_SMALL;
  static LAYOUT_VAL_SCALED(ST_W, 70)
#endif

 protected:
  uint8_t switchIndex;
  Choice* typeChoice = nullptr;
  Choice* startChoice = nullptr;
#if defined(FUNCTION_SWITCHES_RGB_LEDS)
  ColorPicker* offColor = nullptr;
  ColorPicker* onColor = nullptr;
  RGBLedColor offValue;
  RGBLedColor onValue;
  StaticText* overrideLabel = nullptr;
  ToggleSwitch* onOverride = nullptr;
  ToggleSwitch* offOverride = nullptr;
#endif
  int lastType = -1;

#if defined(FUNCTION_SWITCHES_RGB_LEDS)
  void previewColor(int newValue)
  {
    // Convert color index to RGB
    newValue = color32ToRGB(newValue);
    if (g_model.cfsState(switchIndex)) {
        g_eeGeneral.switchOnColor(switchIndex).setColor(newValue);
    } else {
        g_eeGeneral.switchOffColor(switchIndex).setColor(newValue);
    }
  }
#endif

  void setState()
  {
    uint8_t typ = g_eeGeneral.switchType(switchIndex);
    startChoice->show(typ == SWITCH_2POS && g_eeGeneral.switchGroup(switchIndex) == 0);
#if defined(FUNCTION_SWITCHES_RGB_LEDS)
    offColor->show(typ != SWITCH_NONE && typ != SWITCH_GLOBAL);
    onColor->show(typ != SWITCH_NONE && typ != SWITCH_GLOBAL);
    overrideLabel->show(typ != SWITCH_NONE && typ != SWITCH_GLOBAL);
    onOverride->show(typ != SWITCH_NONE && typ != SWITCH_GLOBAL);
    offOverride->show(typ != SWITCH_NONE && typ != SWITCH_GLOBAL);
    if (typ != SWITCH_NONE && typ != SWITCH_GLOBAL)
      setHeight(ROW_H);
    else
      setHeight(ROW_H - EdgeTxStyles::UI_ELEMENT_HEIGHT - PAD_OUTLINE);
#endif
  }

  void checkEvents() override
  {
    setState();
    if (lastType != (int)g_eeGeneral.switchType(switchIndex)) {
      lastType = g_eeGeneral.switchType(switchIndex);
      typeChoice->setValue(lastType);
    }
    Window::checkEvents();
  }
};

RadioFunctionSwitches::RadioFunctionSwitches() : Page(ICON_RADIO_HARDWARE)
{
  header->setTitle(STR_HARDWARE);
  header->setTitle2(STR_MENU_FSWITCH);

  body->padAll(PAD_TINY);
  body->setFlexLayout(LV_FLEX_FLOW_COLUMN, PAD_ZERO);

  FlexGridLayout grid1(cfs_line_col_dsc1, cfs_line_row_dsc, PAD_TINY);
  FlexGridLayout grid2(cfs_line_col_dsc2, cfs_line_row_dsc, PAD_TINY);

  auto line = body->newLine(grid2);
  new StaticText(line, rect_t{}, STR_SWITCHES);
  new StaticText(line, rect_t{}, STR_NAME, COLOR_THEME_PRIMARY1_INDEX, FONT(XS));
  new StaticText(line, rect_t{}, STR_SWITCH_TYPE,
                 COLOR_THEME_PRIMARY1_INDEX, FONT(XS));
  grid2.nextColumn(); 
  // new StaticText(line, rect_t{}, STR_GROUP, COLOR_THEME_PRIMARY1_INDEX, FONT(XS));
  startupHeader = new StaticText(line, rect_t{}, STR_SWITCH_STARTUP,
                 COLOR_THEME_PRIMARY1_INDEX, FONT(XS));
#if defined(FUNCTION_SWITCHES_RGB_LEDS)
  new StaticText(line, rect_t{}, STR_OFF, COLOR_THEME_PRIMARY1_INDEX, FONT(XS));
  new StaticText(line, rect_t{}, STR_ON_ONE_SWITCHES[0], COLOR_THEME_PRIMARY1_INDEX, FONT(XS));
#endif

  for (uint8_t i = 0; i < switchGetMaxSwitches(); i += 1) {
    if (switchIsCustomSwitch(i))
      new RadioFunctionSwitch(body, i);
  }

  setState();

  line = body->newLine(grid1);

  new StaticText(line, rect_t{}, STR_MORE_INFO);

  line = body->newLine(grid1);
  line->padBottom(PAD_LARGE);
  line->padLeft((width() - 150) / 2);

  auto qr = lv_qrcode_create(line->getLvObj(), 150,
                             makeLvColor(COLOR_THEME_SECONDARY1),
                             makeLvColor(COLOR_THEME_SECONDARY3));
  lv_qrcode_update(qr, edgetx_fs_manual_url, strlen(edgetx_fs_manual_url));
}

void RadioFunctionSwitches::setState()
{
  // int cnt = 0;
  // for (int i = 1; i <= 3; i += 1) {
  //   cnt += getSwitchCountInFSGroup(i);
  // }
  // startupHeader->show(cnt != NUM_FUNCTIONS_SWITCHES);
}

void RadioFunctionSwitches::checkEvents()
{
  setState();
  Page::checkEvents();
}

#endif
