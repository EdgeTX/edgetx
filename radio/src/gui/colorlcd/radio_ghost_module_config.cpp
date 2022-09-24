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
    Window(parent, rect, FORWARD_SCROLL // | FORM_FORWARD_FOCUS
           | REFRESH_ALWAYS)
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

RadioGhostModuleConfig::RadioGhostModuleConfig(uint8_t moduleIdx) :
  Page(ICON_RADIO_TOOLS),
  moduleIdx(moduleIdx)
{
  init();
  buildHeader(&header);
  buildBody(&body);
  // setFocus(SET_FOCUS_DEFAULT);
#if defined(TRIMS_EMULATE_BUTTONS)
  setTrimsAsButtons(true);  // Use trim joysticks to operate menu (e.g. on NV14)
#endif
}

void RadioGhostModuleConfig::buildHeader(Window * window)
{
  new StaticText(window, {PAGE_TITLE_LEFT, PAGE_TITLE_TOP + 10, LCD_W - PAGE_TITLE_LEFT, PAGE_LINE_HEIGHT}, "GHOST MODULE", 0, COLOR_THEME_SECONDARY1);
}

void RadioGhostModuleConfig::buildBody(FormWindow * window)
{
  new GhostModuleConfigWindow(window, {0, 0, LCD_W, LCD_H - MENU_HEADER_HEIGHT - 5});
}

#if defined(HARDWARE_KEYS)
void RadioGhostModuleConfig::onEvent(event_t event)
{
  switch (event) {
#if defined(ROTARY_ENCODER_NAVIGATION)
    case EVT_ROTARY_LEFT:
#else
    case EVT_KEY_BREAK(KEY_UP):
#endif
      reusableBuffer.ghostMenu.buttonAction = GHST_BTN_JOYUP;
      reusableBuffer.ghostMenu.menuAction = GHST_MENU_CTRL_NONE;
      moduleState[EXTERNAL_MODULE].counter = GHST_MENU_CONTROL;
      break;

#if defined(ROTARY_ENCODER_NAVIGATION)
    case EVT_ROTARY_RIGHT:
#else
    case EVT_KEY_BREAK(KEY_DOWN):
#endif
      reusableBuffer.ghostMenu.buttonAction = GHST_BTN_JOYDOWN;
      reusableBuffer.ghostMenu.menuAction = GHST_MENU_CTRL_NONE;
      moduleState[EXTERNAL_MODULE].counter = GHST_MENU_CONTROL;
      break;

    case EVT_KEY_FIRST(KEY_ENTER):
      reusableBuffer.ghostMenu.buttonAction = GHST_BTN_JOYPRESS;
      reusableBuffer.ghostMenu.menuAction = GHST_MENU_CTRL_NONE;
      moduleState[EXTERNAL_MODULE].counter = GHST_MENU_CONTROL;
      break;

    case EVT_KEY_BREAK(KEY_EXIT):
      reusableBuffer.ghostMenu.buttonAction = GHST_BTN_JOYLEFT;
      reusableBuffer.ghostMenu.menuAction = GHST_MENU_CTRL_NONE;
      moduleState[EXTERNAL_MODULE].counter = GHST_MENU_CONTROL;
      break;

    case EVT_KEY_LONG(KEY_EXIT):
      memclear(&reusableBuffer.ghostMenu, sizeof(reusableBuffer.ghostMenu));
      reusableBuffer.ghostMenu.buttonAction = GHST_BTN_NONE;
      reusableBuffer.ghostMenu.menuAction = GHST_MENU_CTRL_CLOSE;
      moduleState[EXTERNAL_MODULE].counter = GHST_MENU_CONTROL;
      RTOS_WAIT_MS(10);
      Page::onEvent(event);
#if defined(TRIMS_EMULATE_BUTTONS)
      setTrimsAsButtons(false);  // switch trims back to normal
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
    setTrimsAsButtons(false);  // switch trims back to normal
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

