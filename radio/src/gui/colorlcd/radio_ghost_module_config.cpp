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

#include "opentx.h"
#include "radio_ghost_module_config.h"
#include "libopenui.h"
#include "telemetry/ghost.h"
#include "telemetry/ghost_menu.h"

class GhostModuleConfigWindow: public Window
{
  public:
    GhostModuleConfigWindow(Window * parent, const rect_t & rect) :
    Window(parent, rect, REFRESH_ALWAYS)
    {
      // setFocus(SET_FOCUS_DEFAULT);
    }

    void paint(BitmapBuffer * dc) override
    {
#if LCD_H > LCD_W
      constexpr coord_t xOffset = 20;
      constexpr coord_t xOffset2 = 140;
#else
      constexpr coord_t xOffset = 140;
      constexpr coord_t xOffset2 = 260;
#endif
      constexpr coord_t yOffset = 20;
      constexpr coord_t lineSpacing = 25;

      for (uint8_t line = 0; line < GHST_MENU_LINES; line++) {
        if (reusableBuffer.ghostMenu.line[line].splitLine) {
          if (reusableBuffer.ghostMenu.line[line].lineFlags & GHST_LINE_FLAGS_LABEL_SELECT) {
            dc->drawSolidFilledRect(xOffset, yOffset + line * lineSpacing, getTextWidth(reusableBuffer.ghostMenu.line[line].menuText, 0,FONT(L)), getFontHeight(FONT(L)), FONT(L) | COLOR_THEME_FOCUS);
            dc->drawText(xOffset, yOffset + line * lineSpacing, reusableBuffer.ghostMenu.line[line].menuText, FONT(L) | COLOR_THEME_SECONDARY3);
          }
          else {
            dc->drawText(xOffset, yOffset + line * lineSpacing, reusableBuffer.ghostMenu.line[line].menuText, FONT(L));
          }

          if (reusableBuffer.ghostMenu.line[line].lineFlags & GHST_LINE_FLAGS_VALUE_SELECT) {
            dc->drawSolidFilledRect(xOffset, yOffset + line * lineSpacing, getTextWidth( &reusableBuffer.ghostMenu.line[line].menuText[reusableBuffer.ghostMenu.line[line].splitLine], 0,FONT(L)), getFontHeight(0), COLOR_THEME_FOCUS);
            dc->drawText(xOffset, yOffset + line * lineSpacing,  &reusableBuffer.ghostMenu.line[line].menuText[reusableBuffer.ghostMenu.line[line].splitLine], FONT(L) | COLOR_THEME_SECONDARY3);
          }
          else {
            dc->drawText(xOffset2, yOffset + line * lineSpacing, &reusableBuffer.ghostMenu.line[line].menuText[reusableBuffer.ghostMenu.line[line].splitLine], FONT(L) | COLOR_THEME_SECONDARY1);
          }
        }
        else {
          if (reusableBuffer.ghostMenu.line[line].lineFlags & GHST_LINE_FLAGS_LABEL_SELECT) {
            dc->drawSolidFilledRect(xOffset, yOffset + line * lineSpacing, getTextWidth(reusableBuffer.ghostMenu.line[line].menuText, 0, FONT(L)), getFontHeight(FONT(L)), COLOR_THEME_FOCUS);
            dc->drawText(xOffset, yOffset + line * lineSpacing, reusableBuffer.ghostMenu.line[line].menuText, FONT(L) | COLOR_THEME_SECONDARY3);
          }
          else if (reusableBuffer.ghostMenu.line[line].lineFlags & GHST_LINE_FLAGS_VALUE_EDIT) {
            if (BLINK_ON_PHASE) {
              dc->drawText(xOffset, yOffset + line * lineSpacing, reusableBuffer.ghostMenu.line[line].menuText, FONT(L));
            }
          }
          else {
            dc->drawText(xOffset, yOffset + line * lineSpacing, reusableBuffer.ghostMenu.line[line].menuText, FONT(L) | COLOR_THEME_SECONDARY1);
          }
        }
      }
    }
  protected:
};

static void ghostmoduleconfig_cb(lv_event_t* e)
{
  RadioGhostModuleConfig* ghostmoduleconfig = (RadioGhostModuleConfig*)lv_event_get_user_data(e);
  if (!ghostmoduleconfig || ghostmoduleconfig->deleted()) return;

  uint32_t key = lv_event_get_key(e);

  switch (key) {
    case LV_KEY_LEFT:
      reusableBuffer.ghostMenu.buttonAction = GHST_BTN_JOYUP;
      reusableBuffer.ghostMenu.menuAction = GHST_MENU_CTRL_NONE;
      moduleState[EXTERNAL_MODULE].counter = GHST_MENU_CONTROL;
      break;
    case LV_KEY_RIGHT:
      reusableBuffer.ghostMenu.buttonAction = GHST_BTN_JOYDOWN;
      reusableBuffer.ghostMenu.menuAction = GHST_MENU_CTRL_NONE;
      moduleState[EXTERNAL_MODULE].counter = GHST_MENU_CONTROL;
      break;

    case LV_KEY_ENTER:
      reusableBuffer.ghostMenu.buttonAction = GHST_BTN_JOYPRESS;
      reusableBuffer.ghostMenu.menuAction = GHST_MENU_CTRL_NONE;
      moduleState[EXTERNAL_MODULE].counter = GHST_MENU_CONTROL;
      break;
  }
}

#if defined(HARDWARE_KEYS) && !defined(PCBPL18)
void RadioGhostModuleConfig::onCancel()
{
  reusableBuffer.ghostMenu.buttonAction = GHST_BTN_JOYLEFT;
  reusableBuffer.ghostMenu.menuAction = GHST_MENU_CTRL_NONE;
  moduleState[EXTERNAL_MODULE].counter = GHST_MENU_CONTROL;
}
#endif

RadioGhostModuleConfig::RadioGhostModuleConfig(uint8_t moduleIdx) :
  Page(ICON_RADIO_TOOLS),
  moduleIdx(moduleIdx)
{
  init();
  buildHeader(&header);
  buildBody(&body);
  lv_obj_clear_flag(lvobj, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_clear_flag(lvobj, LV_OBJ_FLAG_CLICK_FOCUSABLE);
  lv_group_add_obj(lv_group_get_default(), lvobj);
  lv_group_set_editing(lv_group_get_default(), true);
  lv_obj_add_event_cb(lvobj, ghostmoduleconfig_cb, LV_EVENT_KEY, this);
#if defined(TRIMS_EMULATE_BUTTONS)
  setHatsAsKeys(true);  // Use trim joysticks to operate menu (e.g. on NV14)
#endif
}

void RadioGhostModuleConfig::buildHeader(Window * window)
{
  header.setTitle("GHOST MODULE");
}

void RadioGhostModuleConfig::buildBody(FormWindow * window)
{
  new GhostModuleConfigWindow(window, {0, 0, LCD_W, LCD_H - MENU_HEADER_HEIGHT - 5});
}

#if defined(HARDWARE_KEYS) && !defined(PCBPL18)
void RadioGhostModuleConfig::onEvent(event_t event)
{
  switch (event) {
    case EVT_KEY_LONG(KEY_EXIT):
      memclear(&reusableBuffer.ghostMenu, sizeof(reusableBuffer.ghostMenu));
      reusableBuffer.ghostMenu.buttonAction = GHST_BTN_NONE;
      reusableBuffer.ghostMenu.menuAction = GHST_MENU_CTRL_CLOSE;
      moduleState[EXTERNAL_MODULE].counter = GHST_MENU_CONTROL;
      RTOS_WAIT_MS(10);
      Page::onEvent(event);
#if defined(TRIMS_EMULATE_BUTTONS)
      setHatsAsKeys(false);  // switch trims back to normal
#endif
      break;
  }
}

void RadioGhostModuleConfig::checkEvents()
{
  Page::checkEvents();

  if (reusableBuffer.ghostMenu.menuStatus == GHST_MENU_STATUS_UNOPENED) { // Handles situation where module is plugged after tools start
    reusableBuffer.ghostMenu.buttonAction = GHST_BTN_NONE;
    reusableBuffer.ghostMenu.menuAction = GHST_MENU_CTRL_OPEN;
    moduleState[EXTERNAL_MODULE].counter = GHST_MENU_CONTROL;
  }
  else if (reusableBuffer.ghostMenu.menuStatus == GHST_MENU_STATUS_CLOSING) {
    RTOS_WAIT_MS(10);
    deleteLater();
#if defined(TRIMS_EMULATE_BUTTONS)
    setHatsAsKeys(false);  // switch trims back to normal
#endif
  }
}
#endif

void RadioGhostModuleConfig::init()
{
  memclear(&reusableBuffer.ghostMenu, sizeof(reusableBuffer.ghostMenu));
  reusableBuffer.ghostMenu.buttonAction = GHST_BTN_NONE;
  reusableBuffer.ghostMenu.menuAction = GHST_MENU_CTRL_OPEN;
  moduleState[EXTERNAL_MODULE].counter = GHST_MENU_CONTROL;
}

