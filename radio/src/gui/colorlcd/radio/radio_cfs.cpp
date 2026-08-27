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

#include "choice.h"
#include "color_picker.h"
#include "edgetx.h"
#include "hal/rgbleds.h"
#include "strhelpers.h"
#include "switches.h"
#include "textedit.h"
#include "toggleswitch.h"

#define SET_DIRTY() storageDirty(EE_GENERAL)

extern const char* _fct_sw_start[];
extern const char* edgetx_fs_manual_url;

//-----------------------------------------------------------------------------

class RadioFunctionSwitch : public FunctionSwitchBase
{
 public:
  RadioFunctionSwitch(Window* parent, uint8_t sw) : FunctionSwitchBase(parent, sw)
  {
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
    offValue = g_eeGeneral.switchOffColor(switchIndex);
    onValue = g_eeGeneral.switchOnColor(switchIndex);

    offColor = new ColorPicker(
        this, {C1_X, C1_Y, C1_W, 0},
        [=]() -> int {  // getValue
          activeSwitch = switchIndex;
          return g_eeGeneral.switchOffColor(switchIndex).getColor() | RGB888_FLAG;
        },
        [=](int newValue) {  // setValue
            g_eeGeneral.switchOffColor(switchIndex) = offValue;
            g_eeGeneral.switchOnColor(switchIndex) = onValue;

          // Convert color index to RGB
          newValue = color32ToRGB(newValue);
          g_eeGeneral.switchOffColor(switchIndex).setColor(newValue);

          offValue = g_eeGeneral.switchOffColor(switchIndex);
          setFSEditOverride(-1, 0);
          SET_DIRTY();
        }, ETX_RGB888);

    onColor = new ColorPicker(
        this, {C2_X, C2_Y, C2_W, 0},
        [=]() -> int {  // getValue
          activeSwitch = switchIndex;
          return g_eeGeneral.switchOnColor(switchIndex).getColor() | RGB888_FLAG;
        },
        [=](int newValue) {  // setValue
            g_eeGeneral.switchOffColor(switchIndex) = offValue;
            g_eeGeneral.switchOnColor(switchIndex) = onValue;

          // Convert color index to RGB
          newValue = color32ToRGB(newValue);
          g_eeGeneral.switchOnColor(switchIndex).setColor(newValue);

          onValue = g_eeGeneral.switchOnColor(switchIndex);
          setFSEditOverride(-1, 0);
          SET_DIRTY();
        }, ETX_RGB888);

    offOverride = new ToggleSwitch(this, {OVROFF_X, C1_Y + EdgeTxStyles::UI_ELEMENT_HEIGHT + PAD_OUTLINE, 0, 0},
                                  [=]() { return g_eeGeneral.cfsOffColorLuaOverride(switchIndex); },
                                  [=](bool v) { g_eeGeneral.cfsSetOffColorLuaOverride(switchIndex, v); });
    onOverride = new ToggleSwitch(this, {C2_X, C1_Y + EdgeTxStyles::UI_ELEMENT_HEIGHT + PAD_OUTLINE, 0, 0},
                                  [=]() { return g_eeGeneral.cfsOnColorLuaOverride(switchIndex); },
                                  [=](bool v) { g_eeGeneral.cfsSetOnColorLuaOverride(switchIndex, v); });
#endif //FUNCTION_SWITCHES_RGB_LEDS

    setState();
  }

 protected:

  void setState()
  {
    uint8_t typ = g_eeGeneral.switchType(switchIndex);
    startChoice->show(typ == SWITCH_2POS && g_eeGeneral.switchGroup(switchIndex) == 0);
    setLEDState(typ);
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

//-----------------------------------------------------------------------------

RadioFunctionSwitches::RadioFunctionSwitches() : FunctionSwitchesBase(ICON_RADIO_HARDWARE, STR_HARDWARE)
{
  for (uint8_t i = 0; i < switchGetMaxSwitches(); i += 1) {
    if (switchIsCustomSwitch(i))
      new RadioFunctionSwitch(body, i);
  }

  addQRCode();

  setState();
}

void RadioFunctionSwitches::setState()
{
  bool showStartHeader = false;
  for (uint8_t i = 0; i < switchGetMaxSwitches(); i += 1) {
    if (switchIsCustomSwitch(i) && g_eeGeneral.switchType(i) == SWITCH_2POS) {
      showStartHeader = true;
      break;
    }
  }
  startupHeader->show(showStartHeader);
}

#endif

//-----------------------------------------------------------------------------
