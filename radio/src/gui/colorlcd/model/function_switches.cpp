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

#include "function_switches.h"

#include "edgetx.h"
#include "strhelpers.h"
#include "switches.h"
#include "color_picker.h"

#if defined(FUNCTION_SWITCHES_RGB_LEDS)
   static constexpr coord_t LH = 72;
#else
   static constexpr coord_t LH = 36;
#endif

#define SET_DIRTY() storageDirty(EE_MODEL)

static const lv_coord_t line_col_dsc1[] = {LV_GRID_CONTENT,
                                           LV_GRID_TEMPLATE_LAST};

static const lv_coord_t line_col_dsc2[] = {
    LV_GRID_FR(10), LV_GRID_FR(10), LV_GRID_FR(10),
    LV_GRID_FR(12), LV_GRID_FR(8),  LV_GRID_TEMPLATE_LAST};

static const lv_coord_t line_row_dsc[] = {LV_GRID_CONTENT,
                                          LV_GRID_TEMPLATE_LAST};

static const char* _fct_sw_start[] = {STR_CHAR_DOWN, STR_CHAR_UP, STR_LAST};

const std::string edgetx_fs_manual_url =
    "https://edgetx.gitbook.io/edgetx-user-manual/b-and-w-radios/model-select/"
    "setup#function-switches";

class FunctionSwitch : public Window
{
 public:
  FunctionSwitch(Window* parent, uint8_t sw) :
      Window(parent, {0, 0, LCD_W - PAD_SMALL * 2, EdgeTxStyles::UI_ELEMENT_HEIGHT + PAD_TINY_GAP * 2}), switchIndex(sw)
  {
    padAll(PAD_TINY_GAP);

    std::string s(STR_CHAR_SWITCH);
    s += switchGetName(switchIndex + switchGetMaxSwitches());

    new StaticText(this, {PAD_LARGE, PAD_MEDIUM, SW_W, EdgeTxStyles::PAGE_LINE_HEIGHT}, s);

    new ModelTextEdit(this, {NM_X, 0, NM_W, 0},
                      g_model.switchNames[switchIndex], LEN_SWITCH_NAME);

    typeChoice = new Choice(
        this, {TP_X, 0, TP_W, 0}, STR_SWTYPES, SWITCH_NONE, SWITCH_2POS,
        [=]() { return FSWITCH_CONFIG(switchIndex); },
        [=](int val) {
          FSWITCH_SET_CONFIG(switchIndex, val);
          if (val == SWITCH_TOGGLE) {
            FSWITCH_SET_STARTUP(switchIndex, FS_START_PREVIOUS);
            setFSLogicalState(switchIndex, 0);
            startChoice->setValue(startChoice->getIntValue());
          }
          SET_DIRTY();
        });
    typeChoice->setAvailableHandler([=](int typ) -> bool {
      int group = FSWITCH_GROUP(switchIndex);
      if (group > 0 && IS_FSWITCH_GROUP_ON(group) && typ == SWITCH_TOGGLE)
        return false;
      return true;
    });

    groupChoice = new Choice(
        this, {GR_X, 0, GR_W, 0}, STR_FUNCTION_SWITCH_GROUPS, 0, 3,
        [=]() { return FSWITCH_GROUP(switchIndex); },
        [=](int group) {
          int oldGroup = FSWITCH_GROUP(switchIndex);
          if (groupHasSwitchOn(group)) setFSLogicalState(switchIndex, 0);
          FSWITCH_SET_GROUP(switchIndex, group);
          if (group > 0) {
            FSWITCH_SET_STARTUP(switchIndex, groupDefaultSwitch(group) == -1
                                                 ? FS_START_PREVIOUS
                                                 : FS_START_OFF);
            if (FSWITCH_CONFIG(switchIndex) == SWITCH_TOGGLE &&
                IS_FSWITCH_GROUP_ON(group))
              FSWITCH_SET_CONFIG(switchIndex, SWITCH_2POS);
            setGroupSwitchState(group, switchIndex);
          } else {
            FSWITCH_SET_STARTUP(switchIndex, FS_START_PREVIOUS);
          }
          setGroupSwitchState(oldGroup);
          SET_DIRTY();
        });
    groupChoice->setAvailableHandler([=](int group) -> bool {
      if (FSWITCH_CONFIG(switchIndex) == SWITCH_TOGGLE && group &&
          IS_FSWITCH_GROUP_ON(group))
        return false;
      return true;
    });

    startChoice = new Choice(
        this, {ST_X, 0, ST_W, 0}, _fct_sw_start, 0, 2,
        [=]() { return FSWITCH_STARTUP(switchIndex); },
        [=](int val) {
          FSWITCH_SET_STARTUP(switchIndex, val);
          SET_DIRTY();
        });

#if defined(FUNCTION_SWITCHES_RGB_LEDS)
   new StaticText(this, {8 , 6 + 36, SW_W, EdgeTxStyles::PAGE_LINE_HEIGHT}, STR_FS_OFF_COLOR, COLOR_THEME_PRIMARY1);

   auto offcolor = new ColorPicker(
        this, rect_t{},
        [=]() -> int {  // getValue
          return g_model.functionSwitchLedOFFColor[switchIndex];
        },
        [=](int newValue) {  // setValue
          g_model.functionSwitchLedOFFColor[switchIndex] =
            (uint32_t)newValue;
          TRACE("Color: %x", g_model.functionSwitchLedOFFColor[switchIndex]);
          SET_DIRTY();
      });
    offcolor->setPos(NM_X, 36);

    new StaticText(this, {TP_X, 6 + 36, SW_W, EdgeTxStyles::PAGE_LINE_HEIGHT}, STR_FS_ON_COLOR, COLOR_THEME_PRIMARY1);

    auto oncolor = new ColorPicker(
        this, rect_t{},
        [=]() -> int {  // getValue
          return g_model.functionSwitchLedONColor[switchIndex];
        },
        [=](int newValue) {  // setValue
          g_model.functionSwitchLedONColor[switchIndex] =
            (uint32_t)newValue;
          SET_DIRTY();
      });
    oncolor->setPos(GR_X, 36);
#endif //FUNCTION_SWITCHES_RGB_LEDS

    setState();
  }

  static constexpr coord_t SW_W = (LCD_W - 4 * 2 - 2 * 4) / 5;
  static constexpr coord_t NM_X = SW_W + 2;
  static constexpr coord_t NM_W = 80;
  static constexpr coord_t TP_X = NM_X + SW_W + 2;
  static constexpr coord_t TP_W = 86;
  static constexpr coord_t GR_X = TP_X + SW_W + 2;
  static constexpr coord_t GR_W = 94;
  static constexpr coord_t ST_X = GR_X + SW_W + 20;
  static constexpr coord_t ST_W = 70;

 protected:
  uint8_t switchIndex;
  Choice* typeChoice = nullptr;
  Choice* groupChoice = nullptr;
  Choice* startChoice = nullptr;
  int lastType = -1;

  void setState()
  {
    startChoice->show(FSWITCH_CONFIG(switchIndex) == SWITCH_2POS && FSWITCH_GROUP(switchIndex) == 0);
    groupChoice->show(FSWITCH_CONFIG(switchIndex) != SWITCH_NONE);
  }

  void checkEvents() override
  {
    setState();
    if (lastType != (int)FSWITCH_CONFIG(switchIndex)) {
      lastType = FSWITCH_CONFIG(switchIndex);
      typeChoice->setValue(lastType);
    }
    Window::checkEvents();
  }
};

class SwitchGroup : public Window
{
 public:
  SwitchGroup(Window* parent, uint8_t group) :
      Window(parent, {0, 0, LCD_W - PAD_SMALL * 2, EdgeTxStyles::UI_ELEMENT_HEIGHT + PAD_TINY_GAP * 2}), groupIndex(group)
  {
    padAll(PAD_TINY_GAP);

    new StaticText(this, {0, PAD_MEDIUM, NM_W, EdgeTxStyles::PAGE_LINE_HEIGHT},
                   STR_FUNCTION_SWITCH_GROUPS[groupIndex]);

    auto btn = new TextButton(
        this, {AO_X, 0, AO_W, 0}, STR_GROUP_ALWAYS_ON, [=]() -> int8_t {
          int groupAlwaysOn = IS_FSWITCH_GROUP_ON(groupIndex);
          groupAlwaysOn ^= 1;
          SET_FSWITCH_GROUP_ON(groupIndex, groupAlwaysOn);
          setGroupSwitchState(groupIndex);
          startChoice->setValue(startChoice->getIntValue());
          SET_DIRTY();
          return groupAlwaysOn;
        });
    btn->check(IS_FSWITCH_GROUP_ON(groupIndex));

    new StaticText(this, {SL_X, PAD_MEDIUM, SL_W, EdgeTxStyles::PAGE_LINE_HEIGHT}, STR_SWITCH_STARTUP);

    startChoice = new Choice(
        this, {ST_X, 0, ST_W, 0}, STR_FSSWITCHES, 0,
        NUM_FUNCTIONS_SWITCHES + 1,
        [=]() { return groupDefaultSwitch(groupIndex) + 1; },
        [=](int sw) {
          for (int i = 0; i < NUM_FUNCTIONS_SWITCHES; i += 1) {
            if (FSWITCH_GROUP(i) == groupIndex) {
              FSWITCH_SET_STARTUP(i, sw ? FS_START_OFF : FS_START_PREVIOUS);
            }
          }
          if (sw > 0 && sw <= NUM_FUNCTIONS_SWITCHES) {
            FSWITCH_SET_STARTUP(sw - 1, FS_START_ON);
          }
          SET_DIRTY();
        });
    startChoice->setAvailableHandler([=](int sw) -> bool {
      return (sw == 0) ||
             (sw == NUM_FUNCTIONS_SWITCHES + 1 &&
              !IS_FSWITCH_GROUP_ON(groupIndex)) ||
             (FSWITCH_GROUP(sw - 1) == groupIndex);
    });
  }

  void refresh()
  {
    startChoice->setValue(groupDefaultSwitch(groupIndex) + 1);
  }

  static constexpr coord_t NM_W = 100;
  static constexpr coord_t AO_X = NM_W + 2;
  static constexpr coord_t AO_W = 100;
  static constexpr coord_t SL_X = AO_X + AO_W + 30;
  static constexpr coord_t SL_W = 100;
  static constexpr coord_t ST_X = SL_X + SL_W + 2;
  static constexpr coord_t ST_W = 80;

 protected:
  uint8_t groupIndex;
  Choice* startChoice;
};

ModelFunctionSwitches::ModelFunctionSwitches() : Page(ICON_MODEL_SETUP)
{
  header->setTitle(STR_MENU_MODEL_SETUP);
  header->setTitle2(STR_MENU_FSWITCH);

  body->padAll(PAD_TINY);
  body->setFlexLayout(LV_FLEX_FLOW_COLUMN, PAD_ZERO);

  FlexGridLayout grid1(line_col_dsc1, line_row_dsc, PAD_TINY);
  FlexGridLayout grid2(line_col_dsc2, line_row_dsc, PAD_TINY);

  auto line = body->newLine(grid2);
  new StaticText(line, rect_t{}, STR_SWITCHES);
  new StaticText(line, rect_t{}, STR_NAME, COLOR_THEME_PRIMARY1_INDEX, FONT(XS));
  new StaticText(line, rect_t{}, STR_SWITCH_TYPE,
                 COLOR_THEME_PRIMARY1_INDEX, FONT(XS));
  new StaticText(line, rect_t{}, STR_GROUP, COLOR_THEME_PRIMARY1_INDEX, FONT(XS));
  startupHeader = new StaticText(line, rect_t{}, STR_SWITCH_STARTUP,
                 COLOR_THEME_PRIMARY1_INDEX, FONT(XS));

  for (uint8_t i = 0; i < NUM_FUNCTIONS_SWITCHES; i += 1) {
    new FunctionSwitch(body, i);
  }

  for (uint8_t i = 1; i <= 3; i += 1) {
    groupLines[i - 1] = new SwitchGroup(body, i);
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
  lv_qrcode_update(qr, edgetx_fs_manual_url.c_str(),
                   edgetx_fs_manual_url.length());
}

void ModelFunctionSwitches::setState()
{
  int cnt = 0;
  for (int i = 0; i < 3; i += 1) {
    cnt += getSwitchCountInFSGroup(i + 1);
    groupLines[i]->show(firstSwitchInGroup(i + 1) >= 0);
    groupLines[i]->refresh();
  }
  startupHeader->show(cnt != NUM_FUNCTIONS_SWITCHES);
}

void ModelFunctionSwitches::checkEvents()
{
  setState();
  Page::checkEvents();
}

#endif
