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
#include "tasks/mixer_task.h"
#include "hal/adc_driver.h"

//-----------------------------------------------------------------------------

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
    return cfsName(n);
#endif
  return g_eeGeneral.getSwitchCustomName(n);
}

bool ModelData::switchHasCustomName(uint8_t n)
{
#if defined(FUNCTION_SWITCHES)
  if (switchIsCustomSwitch(n) && cfsType(n) != SWITCH_GLOBAL)
    return cfsName(n)[0] != 0;
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
#endif

#if defined(COLORLCD)
static TopBarPersistentData _topbarData;
static CustomScreenData* _screenData[MAX_CUSTOM_SCREENS];

CustomScreenData* ModelData::getScreenData(int screenNum)
{
  if (_screenData[screenNum] == nullptr) {
    _screenData[screenNum] = new CustomScreenData();
    _screenData[screenNum]->LayoutId[0] = 0;
    _screenData[screenNum]->layoutData.clear();
  }

  return _screenData[screenNum];
}

const char* ModelData::getScreenLayoutId(int screenNum)
{
  return getScreenData(screenNum)->LayoutId;
}

void ModelData::setScreenLayoutId(int screenNum, const char* s)
{
  strAppend(getScreenData(screenNum)->LayoutId, s, LAYOUT_ID_LEN);
}

TopBarPersistentData* ModelData::getTopbarData()
{
  return &_topbarData;
}

LayoutPersistentData* ModelData::getScreenLayoutData(int screenNum)
{
  return &getScreenData(screenNum)->layoutData;
}

WidgetPersistentData* ModelData::getWidgetData(int screenNum, int zoneNum)
{
  if (screenNum == -1)
    return getTopbarData()->getWidgetData(zoneNum);
  else
    return getScreenLayoutData(screenNum)->getWidgetData(zoneNum);
}

void ModelData::removeScreenLayout(int idx)
{
  if (_screenData[idx]) delete _screenData[idx];

  for (; idx < MAX_CUSTOM_SCREENS - 1; idx += 1)
    _screenData[idx] = _screenData[idx + 1];

  _screenData[idx] = nullptr;
}

void ModelData::initScreenData()
{
  _topbarData.clear();

  for (int i = 0; i < MAX_CUSTOM_SCREENS; i += 1) {
    if (_screenData[i]) delete _screenData[i];
    _screenData[i] = nullptr;
  }
}
#endif
