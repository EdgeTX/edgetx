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

bool ModelData::hasScreenData(int screenNum)
{
  return _screenData[screenNum] != nullptr;
}

CustomScreenData* ModelData::getScreenData(int screenNum)
{
  if (_screenData[screenNum] == nullptr) {
    _screenData[screenNum] = new CustomScreenData();
    _screenData[screenNum]->layoutData.clear();
  }

  return _screenData[screenNum];
}

const char* ModelData::getScreenLayoutId(int screenNum)
{
  return getScreenData(screenNum)->LayoutId.c_str();
}

void ModelData::setScreenLayoutId(int screenNum, const char* s)
{
  getScreenData(screenNum)->LayoutId = s;
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

void ModelData::resetScreenData()
{
  _topbarData.clear();

  for (int i = 0; i < MAX_CUSTOM_SCREENS; i += 1) {
    if (_screenData[i]) delete _screenData[i];
    _screenData[i] = nullptr;
  }
}
#endif

// User Data store. Each entry is a key/value pair.
// Can be used by Lua scripts to save configuration data.
std::vector<UserData> userData;

// Check if userData[n] exists - controls YAML writing
bool ModelData::hasUserData(int n)
{
  return (size_t)n < userData.size() && !userData[n].key.empty();
}

// Get User Data item at position 'n'
// Used when parsing YAML files
UserData* ModelData::getUserData(int n)
{
  if ((size_t)n >= userData.size())
    return nullptr;
  return &userData[n];
}

// Get User Data item with specified key
UserData* ModelData::getUserData(const char* key)
{
  if (key[0]) {
    for (int i = 0; (size_t)i < userData.size(); i += 1)
      if (userData[i].key == key)
        return &userData[i];
  }
  return nullptr;
}

// Get User Data item at position 'n', resizing the store if needed
// (handles gaps in a sparse/hand-edited YAML index sequence). No
// storageDirty() here - this also runs on plain loads, not just mutations.
UserData* ModelData::getOrCreateUserData(int n)
{
  if ((size_t)n >= MAX_USER_DATA) return nullptr;
  if ((size_t)n >= userData.size()) {
    userData.resize(n + 1);
  }
  return &userData[n];
}

// val/val_len are used verbatim (not NUL-terminated semantics), so a
// string value may contain embedded NUL bytes.
static bool setUD(const char* key, const char* val, size_t val_len, UDType typ)
{
  if (key[0] == 0) return false;

  auto ud = g_model.getUserData(key);
  if (ud == nullptr) {
    // Reuse a gap slot (see getOrCreateUserData()) before growing the store.
    for (auto& slot : userData) {
      if (slot.key.empty()) {
        slot.key = key;
        slot.value.assign(val, val_len);
        slot.type = typ;
        storageDirty(EE_MODEL);
        return true;
      }
    }
    if (userData.size() >= MAX_USER_DATA) return false;
    userData.emplace_back();
    userData.back().key = key;
    userData.back().value.assign(val, val_len);
    userData.back().type = typ;
    storageDirty(EE_MODEL);
  } else if (ud->type != typ || ud->value.compare(0, std::string::npos, val, val_len) != 0) {
    ud->value.assign(val, val_len);
    ud->type = typ;
    storageDirty(EE_MODEL);
  }
  return true;
}

// Update or add User Data item
bool ModelData::setUserData(const char* key, const char* str)
{
  return setUD(key, str, strlen(str), UD_STRING);
}

// Update or add User Data item - str may contain embedded NUL bytes.
bool ModelData::setUserData(const char* key, const char* str, size_t len)
{
  return setUD(key, str, len, UD_STRING);
}

// Update or add User Data item
bool ModelData::setUserData(const char* key, int32_t num)
{
  std::string s = std::to_string(num);
  return setUD(key, s.c_str(), s.size(), UD_INT);
}

// Update or add User Data item
bool ModelData::setUserData(const char* key, float num)
{
  // %.9g gives enough significant digits to round-trip a 32-bit float,
  // unlike std::to_string() which fixes 6 decimal places and silently
  // loses precision (e.g. very small or very large values).
  char buf[32];
  int len = snprintf(buf, sizeof(buf), "%.9g", (double)num);
  return setUD(key, buf, len, UD_FLOAT);
}

void ModelData::deleteUserData(const char* key)
{
  for (auto ud = userData.cbegin(); ud != userData.cend(); ud++) {
    if (ud->key == key) {
      userData.erase(ud);
      storageDirty(EE_MODEL);
      return;
    }
  }
}

// Clear all User Data
void ModelData::clearUserData()
{
  userData.clear();
}

int ModelData::getUserDataCount()
{
  return userData.size();
}
