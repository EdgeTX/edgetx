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

#include "edgetx.h"
#include "hal/switch_driver.h"

#if defined(COLORLCD)
#include "view_main.h"
#endif

SwitchConfig ModelData::getSwitchType(uint8_t n)
{
#if defined(FUNCTION_SWITCHES)
  if (switchIsCustomSwitch(n) && cfsType(n) != SWITCH_GLOBAL)
    return cfsType(n);
#endif
  return g_eeGeneral.switchType(n);
}

void ModelData::setSwitchType(uint8_t n, SwitchConfig v) {
#if defined(FUNCTION_SWITCHES)
  if (switchIsCustomSwitch(n) && cfsType(n) != SWITCH_GLOBAL)
    cfsSetType(n,v);
#endif
  g_eeGeneral.switchSetType(n, v);
}

char* ModelData::getSwitchCustomName(uint8_t n)
{
#if defined(FUNCTION_SWITCHES)
  if (switchIsCustomSwitch(n) && cfsType(n) != SWITCH_GLOBAL)
    return g_model.cfsName(n);
#endif
  return g_eeGeneral.getSwitchCustomName(n);
}

bool ModelData::switchHasCustomName(uint8_t n)
{
#if defined(FUNCTION_SWITCHES)
  if (switchIsCustomSwitch(n) && cfsType(n) != SWITCH_GLOBAL)
    return g_model.cfsName(n)[0] != 0;
#endif
  return g_eeGeneral.switchHasCustomName(n);
}

uint8_t ModelData::getSwitchStateForWarning(uint8_t n)
{
#if defined(FUNCTION_SWITCHES)
  if (switchIsCustomSwitch(n)) {
    return cfsState(n) ? 3 : 1;
  } 
#endif
  extern swarnstate_t switches_states;
  return (switches_states >> (n * 2)) & 3;
}

char* RadioData::getSwitchCustomName(uint8_t n)
{
  return switchConfig[n].name;
}

bool RadioData::switchHasCustomName(uint8_t n)
{
  return switchConfig[n].name[0] != 0;
}

SwitchConfig RadioData::switchType(uint8_t n) {
  return (SwitchConfig)switchConfig[n].type;
}

void RadioData::switchSetType(uint8_t n, SwitchConfig v) {
  switchConfig[n].type = v;
  storageDirty(EE_GENERAL);
}

char* RadioData::switchName(uint8_t n) {
  return switchConfig[n].name;
}

#if defined(FUNCTION_SWITCHES)
uint8_t ModelData::getSwitchGroup(uint8_t n) {
  if (switchIsCustomSwitch(n) && cfsType(n) != SWITCH_GLOBAL)
    return cfsGroup(n);
  return g_eeGeneral.switchGroup(n);
}

fsStartPositionType ModelData::getSwitchStart(uint8_t n) {
  if (switchIsCustomSwitch(n) && cfsType(n) != SWITCH_GLOBAL)
    return cfsStart(n);
  return g_eeGeneral.switchStart(n);
}

void ModelData::setSwitchStart(uint8_t n, fsStartPositionType v) {
  if (switchIsCustomSwitch(n) && cfsType(n) != SWITCH_GLOBAL)
    cfsSetStart(n, v);
  g_eeGeneral.switchSetStart(n, v);
}

SwitchConfig ModelData::cfsType(uint8_t n) {
  return (SwitchConfig)customSwitches[switchGetCustomSwitchIdx(n)].type;
}

void ModelData::cfsSetType(uint8_t n, SwitchConfig v) {
  customSwitches[switchGetCustomSwitchIdx(n)].type = v;
  storageDirty(EE_MODEL);
}

char* ModelData::cfsName(uint8_t n) {
  return customSwitches[switchGetCustomSwitchIdx(n)].name;
}

uint8_t ModelData::cfsGroup(uint8_t n) {
  return customSwitches[switchGetCustomSwitchIdx(n)].group;
}

void ModelData::cfsSetGroup(uint8_t n, uint8_t v) {
  customSwitches[switchGetCustomSwitchIdx(n)].group = v;
  storageDirty(EE_MODEL);
}

fsStartPositionType ModelData::cfsStart(uint8_t n) {
  return (fsStartPositionType)customSwitches[switchGetCustomSwitchIdx(n)].start;
}

void ModelData::cfsSetStart(uint8_t n, fsStartPositionType v) {
  customSwitches[switchGetCustomSwitchIdx(n)].start = v;
  storageDirty(EE_MODEL);
}

bool ModelData::cfsState(uint8_t n) {
  return customSwitches[switchGetCustomSwitchIdx(n)].state;
}

void ModelData::cfsSetState(uint8_t n, bool v) {
  customSwitches[switchGetCustomSwitchIdx(n)].state = v;
  storageDirty(EE_MODEL);
}

bool ModelData::cfsSFState(uint8_t n) {
  return bfGet<uint8_t>(sfPushState, switchGetCustomSwitchIdx(n), 1);
}

void ModelData::cfsSetSFState(uint8_t n, bool v) {
  // Note: n = cfs index, not switch index
  sfPushState = bfSet<uint8_t>(sfPushState, v, n, 1);
}

void ModelData::cfsResetSFState() {
  sfPushState = 0;
}

#if defined(FUNCTION_SWITCHES_RGB_LEDS)
RGBLedColor& ModelData::getSwitchOnColor(uint8_t n) {
  if (switchIsCustomSwitch(n) && cfsType(n) != SWITCH_GLOBAL)
    return cfsOnColor(n);
  return g_eeGeneral.switchOnColor(n);
}

RGBLedColor& ModelData::getSwitchOffColor(uint8_t n) {
  if (switchIsCustomSwitch(n) && cfsType(n) != SWITCH_GLOBAL)
    return cfsOffColor(n);
  return g_eeGeneral.switchOffColor(n);
}

bool ModelData::getSwitchOnColorLuaOverride(uint8_t n) {
  if (switchIsCustomSwitch(n) && cfsType(n) != SWITCH_GLOBAL)
    return cfsOnColorLuaOverride(n);
  return g_eeGeneral.cfsOnColorLuaOverride(n);
}

bool ModelData::getSwitchOffColorLuaOverride(uint8_t n) {
  if (switchIsCustomSwitch(n) && cfsType(n) != SWITCH_GLOBAL)
    return cfsOffColorLuaOverride(n);
  return g_eeGeneral.cfsOffColorLuaOverride(n);
}

RGBLedColor& ModelData::cfsOnColor(uint8_t n) {
  return customSwitches[switchGetCustomSwitchIdx(n)].onColor;
}

RGBLedColor& ModelData::cfsOffColor(uint8_t n) {
  return customSwitches[switchGetCustomSwitchIdx(n)].offColor;
}

bool ModelData::cfsOnColorLuaOverride(uint8_t n) {
  return customSwitches[switchGetCustomSwitchIdx(n)].onColorLuaOverride;
}

bool ModelData::cfsOffColorLuaOverride(uint8_t n) {
  return customSwitches[switchGetCustomSwitchIdx(n)].offColorLuaOverride;
}

void ModelData::cfsSetOnColorLuaOverride(uint8_t n, bool v) {
  customSwitches[switchGetCustomSwitchIdx(n)].onColorLuaOverride = v;
  storageDirty(EE_MODEL);
}

void ModelData::cfsSetOffColorLuaOverride(uint8_t n, bool v) {
  customSwitches[switchGetCustomSwitchIdx(n)].offColorLuaOverride = v;
  storageDirty(EE_MODEL);
}
#endif

fsStartPositionType RadioData::switchStart(uint8_t n) {
  return (fsStartPositionType)switchConfig[n].start;
}

void RadioData::switchSetStart(uint8_t n, fsStartPositionType v) {
  switchConfig[n].start = v;
  storageDirty(EE_GENERAL);
}

#if defined(FUNCTION_SWITCHES_RGB_LEDS)
RGBLedColor& RadioData::switchOnColor(uint8_t n) {
  return switchConfig[n].onColor;
}

RGBLedColor& RadioData::switchOffColor(uint8_t n) {
  return switchConfig[n].offColor;
}

bool RadioData::cfsOnColorLuaOverride(uint8_t n) {
  return switchConfig[n].onColorLuaOverride;
}

bool RadioData::cfsOffColorLuaOverride(uint8_t n) {
  return switchConfig[n].offColorLuaOverride;
}

void RadioData::cfsSetOnColorLuaOverride(uint8_t n, bool v) {
  switchConfig[n].onColorLuaOverride = v;
  storageDirty(EE_GENERAL);
}

void RadioData::cfsSetOffColorLuaOverride(uint8_t n, bool v) {
  switchConfig[n].offColorLuaOverride = v;
  storageDirty(EE_GENERAL);
}
#endif
#endif

#if defined(COLORLCD)
QMPage RadioData::getKeyShortcut(event_t event)
{
  QMPage page = QM_NONE;
  switch(event) {
    case EVT_KEY_BREAK(KEY_MODEL):
      page = (QMPage)keyShortcuts[0].shortcut;
      break;
    case EVT_KEY_BREAK(KEY_SYS):
      page = (QMPage)keyShortcuts[1].shortcut;
      break;
    case EVT_KEY_BREAK(KEY_TELE):
      page = (QMPage)keyShortcuts[2].shortcut;
      break;
    case EVT_KEY_LONG(KEY_MODEL):
      page = (QMPage)keyShortcuts[3].shortcut;
      break;
    case EVT_KEY_LONG(KEY_SYS):
      page = (QMPage)keyShortcuts[4].shortcut;
      break;
    case EVT_KEY_LONG(KEY_TELE):
      page = (QMPage)keyShortcuts[5].shortcut;
      break;
    default:
      break;
  }
  if (page >= QM_UI_SCREEN1 && page <= QM_UI_SCREEN10)
    page = (QMPage)(QM_UI_SCREEN1 + ViewMain::instance()->getCurrentMainView());
  return page;
}

void RadioData::setKeyShortcut(event_t event, QMPage shortcut)
{
  switch(event) {
    case EVT_KEY_BREAK(KEY_MODEL):
      keyShortcuts[0].shortcut = shortcut;
      break;
    case EVT_KEY_BREAK(KEY_SYS):
      keyShortcuts[1].shortcut = shortcut;
      break;
    case EVT_KEY_BREAK(KEY_TELE):
      keyShortcuts[2].shortcut = shortcut;
      break;
    case EVT_KEY_LONG(KEY_MODEL):
      keyShortcuts[3].shortcut = shortcut;
      break;
    case EVT_KEY_LONG(KEY_SYS):
      keyShortcuts[4].shortcut = shortcut;
      break;
    case EVT_KEY_LONG(KEY_TELE):
      keyShortcuts[5].shortcut = shortcut;
      break;
    default:
      break;
  }
}

void RadioData::defaultKeyShortcuts()
{
  setKeyShortcut(EVT_KEY_BREAK(KEY_MODEL), QM_MODEL_SETUP);
  setKeyShortcut(EVT_KEY_LONG(KEY_MODEL), QM_MANAGE_MODELS);
  setKeyShortcut(EVT_KEY_BREAK(KEY_SYS), QM_OPEN_QUICK_MENU);
  setKeyShortcut(EVT_KEY_LONG(KEY_SYS), QM_TOOLS_APPS);
  setKeyShortcut(EVT_KEY_BREAK(KEY_TELE), QM_UI_SCREEN1);
  setKeyShortcut(EVT_KEY_LONG(KEY_TELE), QM_TOOLS_CHAN_MON);
}
#endif