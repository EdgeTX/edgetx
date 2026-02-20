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
#include "hal/adc_driver.h"
#include "hal/switch_driver.h"
#include "tasks/mixer_task.h"

#if defined(COLORLCD)
#include "radio_tools.h"
#include "quick_menu_def.h"
#include "view_main.h"
#endif

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
int RadioData::getKeyShortcutNum(event_t event)
{
  switch(event) {
    case EVT_KEY_BREAK(KEY_MODEL):  return 0;
    case EVT_KEY_BREAK(KEY_SYS):    return 1;
    case EVT_KEY_BREAK(KEY_TELE):   return 2;
    case EVT_KEY_LONG(KEY_MODEL):   return 3;
    case EVT_KEY_LONG(KEY_SYS):     return 4;
    case EVT_KEY_LONG(KEY_TELE):    return 5;
    default:
      return -1;
  }
}

event_t RadioData::getKeyShortcutEvent(int n)
{
  switch(n) {
    case 0: return EVT_KEY_BREAK(KEY_MODEL);
    case 1: return EVT_KEY_BREAK(KEY_SYS);
    case 2: return EVT_KEY_BREAK(KEY_TELE);
    case 3: return EVT_KEY_LONG(KEY_MODEL);
    case 4: return EVT_KEY_LONG(KEY_SYS);
    case 5: return EVT_KEY_LONG(KEY_TELE);
    default:
      return 0;
  }
}

QMPage RadioData::getKeyShortcut(event_t event)
{
  QMPage page = QM_NONE;
#if VERSION_MAJOR == 2
  switch(event) {
    case EVT_KEY_BREAK(KEY_MODEL):
      page = QM_MODEL_SETUP;
      break;
    case EVT_KEY_BREAK(KEY_SYS):
      page = QM_TOOLS_APPS;
      break;
    case EVT_KEY_BREAK(KEY_TELE):
      page = QM_UI_SCREEN1;
      break;
    case EVT_KEY_LONG(KEY_MODEL):
      page = QM_MANAGE_MODELS;
      break;
    case EVT_KEY_LONG(KEY_SYS):
      page = QM_RADIO_SETUP;
      break;
    case EVT_KEY_LONG(KEY_TELE):
      page = QM_TOOLS_CHAN_MON;
      break;
    default:
      break;
  }
#else
  int n = getKeyShortcutNum(event);
  if (n >= 0) page = (QMPage)keyShortcuts[n].shortcut;
#endif
  if (page >= QM_UI_SCREEN1 && page <= QM_UI_SCREEN10)
    page = (QMPage)(QM_UI_SCREEN1 + ViewMain::instance()->getCurrentMainView());
  return page;
}

void RadioData::setKeyShortcut(event_t event, QMPage shortcut)
{
  int n = getKeyShortcutNum(event);
  if (n >= 0) keyShortcuts[n].shortcut = shortcut;
}

bool RadioData::hasKeyShortcut(QMPage shortcut, event_t event)
{
  // Returns true if the shortcut is defined on any other key except 'event' key
  for (int i = 0; i < MAX_KEY_SHORTCUTS; i += 1) {
    auto ev = getKeyShortcutEvent(i);
    if (shortcut < QM_APP) {
      if (keyShortcuts[i].shortcut == shortcut)
        return ev != event;
    } else {
      if (keyShortcuts[i].shortcut == QM_APP) {
        int idx = getLuaToolId(getKeyToolName(ev)) + QM_APP;
        if (idx == shortcut)
          return ev != event;
      }
    }
  }
  return false;
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

static std::string _keyToolNames[MAX_KEY_SHORTCUTS];

void RadioData::setKeyToolName(event_t event, const std::string name)
{
  int n = getKeyShortcutNum(event);
  if (n >= 0) _keyToolNames[n] = name;
}

const std::string RadioData::getKeyToolName(event_t event)
{
  int n = getKeyShortcutNum(event);
  if (n >= 0) return _keyToolNames[n];
  return "";
}

static std::string _favoriteToolNames[MAX_QM_FAVORITES];

void RadioData::setFavoriteToolName(int fav, const std::string name)
{
  _favoriteToolNames[fav] = name;
}

const std::string RadioData::getFavoriteToolName(int fav)
{
  return _favoriteToolNames[fav];
}
#endif
