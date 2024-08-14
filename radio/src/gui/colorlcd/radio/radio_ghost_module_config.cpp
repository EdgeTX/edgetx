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

#include "radio_ghost_module_config.h"

#include "libopenui.h"
#include "edgetx.h"
#include "telemetry/ghost.h"
#include "telemetry/ghost_menu.h"

class GhostModuleConfigWindow : public Window
{
 public:
  GhostModuleConfigWindow(Window* parent, const rect_t& rect) :
      Window(parent, rect)
  {
    constexpr coord_t yOffset = 20;
    constexpr coord_t lineSpacing = 25;
    coord_t h = getFontHeight(FONT(L));

    for (int i = 0; i < GHST_MENU_LINES; i += 1) {
      menuLines[i][0] = new StaticText(
          this, {xOffset, yOffset + i * lineSpacing, LV_SIZE_CONTENT, h}, "",
          COLOR_THEME_PRIMARY1_INDEX, FONT(L));
      etx_txt_color(menuLines[i][0]->getLvObj(), COLOR_THEME_SECONDARY1_INDEX,
                    LV_PART_MAIN);
      etx_solid_bg(menuLines[i][0]->getLvObj(), COLOR_THEME_FOCUS_INDEX,
                   LV_STATE_USER_1);
      etx_txt_color(menuLines[i][0]->getLvObj(), COLOR_THEME_SECONDARY3_INDEX,
                    LV_STATE_USER_1);

      menuLines[i][1] = new StaticText(
          this, {xOffset2, yOffset + i * lineSpacing, LV_SIZE_CONTENT, h}, "",
          COLOR_THEME_PRIMARY1_INDEX, FONT(L));
      etx_txt_color(menuLines[i][1]->getLvObj(), COLOR_THEME_SECONDARY1_INDEX,
                    LV_PART_MAIN);
      etx_solid_bg(menuLines[i][1]->getLvObj(), COLOR_THEME_FOCUS_INDEX,
                   LV_STATE_USER_1);
      etx_txt_color(menuLines[i][1]->getLvObj(), COLOR_THEME_SECONDARY3_INDEX,
                    LV_STATE_USER_1);
    }
  }

  static LAYOUT_VAL(xOffset, 140, 20)
  static LAYOUT_VAL(xOffset2, 260, 140)

 protected:
  StaticText* menuLines[GHST_MENU_LINES][2];

  void checkEvents() override
  {
    for (uint8_t i = 0; i < GHST_MENU_LINES; i += 1) {
      if (reusableBuffer.ghostMenu.line[i].splitLine) {
        menuLines[i][0]->setText(reusableBuffer.ghostMenu.line[i].menuText);
        if (reusableBuffer.ghostMenu.line[i].lineFlags &
            GHST_LINE_FLAGS_LABEL_SELECT)
          lv_obj_add_state(menuLines[i][0]->getLvObj(), LV_STATE_USER_1);

        menuLines[i][1]->setText(reusableBuffer.ghostMenu.line[i].menuText +
                                 reusableBuffer.ghostMenu.line[i].splitLine);
        if (reusableBuffer.ghostMenu.line[i].lineFlags &
            GHST_LINE_FLAGS_VALUE_SELECT)
          lv_obj_add_state(menuLines[i][1]->getLvObj(), LV_STATE_USER_1);
      } else {
        if (reusableBuffer.ghostMenu.line[i].lineFlags &
                GHST_LINE_FLAGS_VALUE_EDIT &&
            BLINK_ON_PHASE)
          menuLines[i][0]->setText("");
        else
          menuLines[i][0]->setText(reusableBuffer.ghostMenu.line[i].menuText);
        if (reusableBuffer.ghostMenu.line[i].lineFlags &
            GHST_LINE_FLAGS_LABEL_SELECT)
          lv_obj_add_state(menuLines[i][0]->getLvObj(), LV_STATE_USER_1);

        menuLines[i][1]->setText("");
      }
    }
  }
};

static void ghostmoduleconfig_cb(lv_event_t* e)
{
  RadioGhostModuleConfig* ghostmoduleconfig =
      (RadioGhostModuleConfig*)lv_event_get_user_data(e);
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
    Page(ICON_RADIO_TOOLS), moduleIdx(moduleIdx)
{
  init();
  buildHeader(header);
  buildBody(body);
  lv_obj_clear_flag(lvobj, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_clear_flag(lvobj, LV_OBJ_FLAG_CLICK_FOCUSABLE);
  lv_group_add_obj(lv_group_get_default(), lvobj);
  lv_group_set_editing(lv_group_get_default(), true);
  lv_obj_add_event_cb(lvobj, ghostmoduleconfig_cb, LV_EVENT_KEY, this);
#if defined(TRIMS_EMULATE_BUTTONS)
  setHatsAsKeys(true);  // Use trim joysticks to operate menu (e.g. on NV14)
#endif
}

void RadioGhostModuleConfig::buildHeader(Window* window)
{
  header->setTitle("GHOST MODULE");
}

void RadioGhostModuleConfig::buildBody(Window* window)
{
  window->padAll(PAD_ZERO);
  new GhostModuleConfigWindow(window,
                              {0, 0, LCD_W, LCD_H - EdgeTxStyles::MENU_HEADER_HEIGHT - 5});
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

  if (reusableBuffer.ghostMenu.menuStatus ==
      GHST_MENU_STATUS_UNOPENED) {  // Handles situation where module is
                                    // plugged after tools start
    reusableBuffer.ghostMenu.buttonAction = GHST_BTN_NONE;
    reusableBuffer.ghostMenu.menuAction = GHST_MENU_CTRL_OPEN;
    moduleState[EXTERNAL_MODULE].counter = GHST_MENU_CONTROL;
  } else if (reusableBuffer.ghostMenu.menuStatus == GHST_MENU_STATUS_CLOSING) {
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
