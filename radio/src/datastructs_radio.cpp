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
#endif
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

bool RadioData::hasKeyShortcut(QMPage shortcut)
{
  for (int i = 0; i < MAX_KEY_SHORTCUTS; i += 1)
    if (keyShortcuts[i].shortcut == shortcut)
      return true;
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
#endif
